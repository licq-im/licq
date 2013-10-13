/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2013 Licq developers <licq-dev@googlegroups.com>
 *
 * Please refer to the COPYRIGHT file distributed with this source
 * distribution for the names of the individual contributors.
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "plugin.h"

#include <boost/foreach.hpp>
#include <cstring>
#include <list>
#include <poll.h>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/logging/log.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/userevents.h>

#include "dbusinterface.h"

using namespace LicqDbus;

Plugin::Plugin()
  : myConn(NULL)
{
  // Empty
}

Plugin::~Plugin()
{
  // Empty
}

int Plugin::run()
{
  setSignalMask(Licq::PluginSignal::SignalUser);

  myMainLoop.addRawFile(getReadPipe(), this);

  myConn = new DbusInterface(myMainLoop, this);
  myConn->connect();

  // Timer to try reconnecting if connection is lost
  myMainLoop.addTimeout(60*1000, this, 0, false);

  myMainLoop.run();
  Licq::gLog.info("D-Bus plugin shutting down");

  myMainLoop.removeCallback(this);

  // Send shutdown signal to clients before disconnecting from D-Bus
  myConn->sendSignal("/org/licq/Core", "org.licq.Core", "Shutdown", NULL);
  myConn->flush();

  myConn->disconnect();
  delete myConn;
  myConn = NULL;

  return 0;
}

bool Plugin::isEnabled() const
{
  return true;
}

void Plugin::timeoutEvent(int /*id*/)
{
  // Try to reconnect to message bus if not connected
  myConn->connect();
}

void Plugin::rawFileEvent(int /*id*/, int fd, int revents)
{
  if (revents & (POLLERR|POLLHUP|POLLNVAL))
    myMainLoop.quit();

  if ((revents & POLLIN) == 0)
    return;

  char ch;
  ::read(fd, &ch, sizeof(char));

  switch (ch)
  {
    case PipeSignal:
      processSignal(popSignal().get());
      break;

    case PipeEvent:
      popEvent();
      break;

    case PipeShutdown:
      myMainLoop.quit();
      break;

    case PipeEnable:
    case PipeDisable:
      break;

    default:
      Licq::gLog.error("Unknown notification from Licq core: %c", ch);
  }
}

std::string Plugin::protocolIdToString(unsigned long protocolId)
{
  switch (protocolId)
  {
    case ICQ_PPID: return "ICQ";
    case MSN_PPID: return "MSN";
    case JABBER_PPID: return "Jabber";
    default:
      return Licq::protocolId_toString(protocolId);
  }
}

std::string Plugin::userIdToObjectPath(const Licq::UserId& userId)
{
  std::string s("/org/licq/ContactList/");
  s += protocolIdToString(userId.protocolId());
  if (userId.isOwner())
  {
    s += '/';
    s += DbusInterface::encodeObjectPathPart(userId.ownerId().accountId());
  }
  s += '/';
  s += DbusInterface::encodeObjectPathPart(userId.accountId());
  return s;
}

Licq::UserId Plugin::objectPathToUserId(const std::string& object)
{
  if (object.compare(0, 22, "/org/licq/ContactList/") != 0)
    return Licq::UserId();

  size_t p1 = object.find('/', 23);
  if (p1 == std::string::npos)
    // Missing two parameters
    return Licq::UserId();

  unsigned long protocolId = Licq::protocolId_fromString(object.substr(22, p1-22));
  if (protocolId == 0)
    // Unknown protocol
    return Licq::UserId();

  size_t p2 = object.find('/', p1+1);
  if (p2 == std::string::npos)
  {
    // Owner
    std::string accountId(DbusInterface::decodeObjectPathPart(object.substr(p1+1)));
    return Licq::UserId(protocolId, accountId);
  }

  if (object.find('/', p2+1) != std::string::npos)
    // Too many levels
    return Licq::UserId();

  std::string ownerAccountId(DbusInterface::decodeObjectPathPart(object.substr(p1+1, p2-p1-1)));
  std::string accountId(DbusInterface::decodeObjectPathPart(object.substr(p2+1)));
  if (accountId == ownerAccountId)
    // Owners shouldn't have the last level
    return Licq::UserId();

  return Licq::UserId(Licq::UserId(protocolId, ownerAccountId), accountId);
}

void Plugin::processSignal(const Licq::PluginSignal* sig)
{
  assert(sig != NULL);

  switch (sig->signal())
  {
    case Licq::PluginSignal::SignalUser:
    {
      Licq::UserReadGuard user(sig->userId());
      if (!user.isLocked())
        break;

      std::string object(userIdToObjectPath(sig->userId()));
      std::string iface(sig->userId().isOwner() ? "org.licq.Account" : "org.licq.Contact");

      switch (sig->subSignal())
      {
        case Licq::PluginSignal::UserStatus:
          // Note: The statusString() returns a translated string for displaying
          //       The numeric status should be used for any client logic
          myConn->sendSignal(object, iface, "Status", "us", user->status(), user->statusString().c_str());
          break;

        case Licq::PluginSignal::UserEvents:
        {
          if (sig->argument() == 0)
            // Auto response check
            break;

          // Get a display name to include in signals
          std::string name = user->getAlias();
          if (name.empty())
            name = user->getFullName();
          if (name.empty())
            name = user->accountId();

          // Signal number of unread messages for this contact
          myConn->sendSignal(object, iface, "NumUnread", "us", user->NewMessages(), name.c_str());

          // Signal total number of unread messages for all contacts
          myConn->sendSignal("/org/licq/ContactList", "org.licq.ContactList",
              "NumUnread", "u", Licq::User::getNumUserEvents());

          const Licq::UserEvent* ue = user->EventPeekId(sig->argument());
          if (ue != NULL)
          {
            // New message received
            myConn->sendSignal(object, iface, "ReceivedEvent", "ss", ue->text().c_str(), name.c_str());
          }
          break;
        }
      }
      break;
    }

    default:
      break;
  }
}

void Plugin::dbusConnected()
{
  if (!myConn->requestName("org.licq"))
    Licq::gLog.warning("Failed to claim name on message bus");

  // Signal clients that we are up and running
  myConn->sendSignal("/org/licq/Core", "org.licq.Core", "Started", NULL);
}

int Plugin::dbusMethod(const char* path, const char* iface, const char* member,
      DBusMessage* msgref, DBusMessageIter* argref, const char* fmt)
{
  if (strcmp(iface, "org.licq.Core") == 0)
  {
    if (strcmp(path, "/org/licq/Core") != 0)
      return DbusInterface::ErrorUnknownObject;

    if (strcmp(member, "GetVersion") == 0)
    {
      myConn->sendReply(msgref, "s", Licq::gDaemon.Version());
      return DbusInterface::MethodReplied;
    }

    if (strcmp(member, "Shutdown") == 0)
    {
      myConn->sendReply(msgref, NULL);
      myConn->flush();
      Licq::gDaemon.Shutdown();
      return DbusInterface::MethodReplied;
    }

    return DbusInterface::ErrorUnknownMethod;
  }

  if (strcmp(iface, "org.licq.ContactList") == 0)
  {
    if (strcmp(path, "/org/licq/ContactList") != 0)
      return DbusInterface::ErrorUnknownObject;

    if (strcmp(member, "GetAccounts") == 0)
    {
      std::list<std::string> owners;
      {
        Licq::OwnerListGuard ownerList;
        BOOST_FOREACH(const Licq::Owner* o, **ownerList)
          owners.push_back(protocolIdToString(o->protocolId()) + '/' + o->accountId());
      }
      myConn->sendReply(msgref, "as", &owners);
      return DbusInterface::MethodReplied;
    }

    return DbusInterface::ErrorUnknownMethod;
  }

  if (strcmp(iface, "org.licq.Account") == 0 || strcmp(iface, "org.licq.Contact") == 0)
  {
    Licq::UserId userId(objectPathToUserId(path));
    if (!userId.isValid())
      return DbusInterface::ErrorUnknownObject;

    // If object path is owner iface must also be and vice versa
    if ((strcmp(iface, "org.licq.Account") == 0) != userId.isOwner())
      return DbusInterface::ErrorUnknownObject;

    if (strcmp(member, "GetContacts") == 0 && userId.isOwner())
    {
      std::list<std::string> contacts;
      {
        Licq::UserListGuard userList(userId);
        BOOST_FOREACH(const Licq::User* u, **userList)
          contacts.push_back(u->accountId());
      }
      myConn->sendReply(msgref, "as", &contacts);
      return DbusInterface::MethodReplied;
    }

    if (strcmp(member, "SetStatus") == 0 && userId.isOwner())
    {
      // Make sure owner exists
      if (!Licq::gUserManager.userExists(userId))
        return DbusInterface::ErrorUnknownObject;

      unsigned status;
      if (strcmp(fmt, "s") == 0)
      {
        const char* strStatus;
        myConn->getNextMessageParamValue(argref, &strStatus);
        if (!Licq::User::stringToStatus(strStatus, status))
          return DbusInterface::ErrorInvalidArgs;
      }
      else if (strcmp(fmt, "u") == 0)
        myConn->getNextMessageParamValue(argref, &status);
      else
        return DbusInterface::ErrorInvalidSignature;

      Licq::gProtocolManager.setStatus(userId, status);

      myConn->sendReply(msgref, NULL);
      return DbusInterface::MethodReplied;
    }

    // Only read access functions below
    Licq::UserReadGuard u(userId);
    if (!u.isLocked())
      return DbusInterface::ErrorUnknownObject;

    if (strcmp(member, "GetName") == 0)
    {
      myConn->sendReply(msgref, "ssss", u->getAlias().c_str(), u->getFirstName().c_str(),
          u->getLastName().c_str(), u->getEmail().c_str());
      return DbusInterface::MethodReplied;
    }

    if (strcmp(member, "GetStatus") == 0)
    {
      myConn->sendReply(msgref, "us", u->status(), u->statusString().c_str());
      return DbusInterface::MethodReplied;
    }

    return DbusInterface::ErrorUnknownMethod;
  }

  return DbusInterface::ErrorUnknownInterface;
}

std::string Plugin::dbusIntrospect(const char* path)
{
  const char* p = path;

  if (strcmp(path, "/") == 0)
    return "<node name=\"org\">";
  if (strcmp(path, "/org") == 0)
    return "<node name=\"licq\">";

  if (strncmp(path, "/org/licq", 9) != 0)
    return "";
  p = path + 9;
  if (p[0] == '\0')
    return "<node name=\"Core\"/><node name=\"ContactList\"/>";

  if (strcmp(p, "/Core") == 0)
    return "<interface name=\"org.licq.Core\">"
        "<method name=\"GetVersion\"><arg type=\"s\" direction=\"out\"/></method>"
        "<method name=\"Shutdown\"/>"
        "<signal name=\"Started\"/>"
        "<signal name=\"Shutdown\"/>"
        "</interface>";

  // Everything below is for ContactList tree
  if (strncmp(p, "/ContactList", 12) != 0)
    return "";
  p += 12;

  if (p[0] == '\0')
  {
    std::string s("<interface name=\"org.licq.ContactList\">"
          "<method name=\"GetAccounts\">"
            "<arg type=\"as\" direction=\"out\"/>"
          "</method>"
        "</interface>");
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(const Licq::Owner* o, **ownerList)
      s += "<node name=\"" + protocolIdToString(o->protocolId()) + "\"/>";
    return s;
  }

  if (*(p++) != '/')
    return "";
Licq::gLog.info("PROTOCOL: '%s'\n", p);
  unsigned long protocolId = Licq::protocolId_fromString(p);
  if (protocolId != 0)
  {
    std::string s;
    Licq::OwnerListGuard ownerList(protocolId);
    BOOST_FOREACH(const Licq::Owner* o, **ownerList)
      s += "<node name=\"" + DbusInterface::encodeObjectPathPart(o->accountId()) + "\"/>";
    return s;
  }

  Licq::UserId userId(objectPathToUserId(path));
  if (!userId.isValid())
    return "";

  std::string s;

  if (userId.isOwner())
    s = "<interface name=\"org.licq.Account\">"
          "<method name=\"SetStatus\">"
            "<arg name=\"StatusBits\" type=\"u\" direction=\"in\"/>"
          "</method>"
          "<method name=\"GetContacts\">"
            "<arg name=\"Contacts\" type=\"as\" direction=\"out\"/>"
          "</method>";
  else
    s = "<interface name=\"org.licq.Contact\">";

  s.append("<method name=\"GetName\">"
          "<arg name=\"Alias\" type=\"s\" direction=\"out\"/>"
          "<arg name=\"FirstName\" type=\"s\" direction=\"out\"/>"
          "<arg name=\"LastName\" type=\"s\" direction=\"out\"/>"
          "<arg name=\"Email\" type=\"s\" direction=\"out\"/>"
        "</method>"
        "<method name=\"GetStatus\">"
          "<arg name=\"StatusBits\" type=\"u\" direction=\"out\"/>"
          "<arg name=\"StatusText\" type=\"s\" direction=\"out\"/>"
        "</method>"
        "<signal name=\"Status\">"
          "<arg name=\"StatusBits\" type=\"u\"/>"
          "<arg name=\"StatusText\" type=\"s\"/>"
        "</signal>"
        "<signal name=\"NumUnread\">"
          "<arg name=\"NumUnread\" type=\"u\"/>"
          "<arg name=\"DisplayName\" type=\"s\"/>"
        "</signal>"
        "<signal name=\"ReceivedEvent\">"
          "<arg name=\"MessageText\" type=\"s\"/>"
          "<arg name=\"DisplayName\" type=\"s\"/>"
        "</signal>"
      "</interface>");

  if (userId.isOwner())
  {
    Licq::UserListGuard userList(userId);
    BOOST_FOREACH(const Licq::User* u, **userList)
      s += "<node name=\"" + DbusInterface::encodeObjectPathPart(u->accountId()) + "\"/>";
  }

  return s;
}
