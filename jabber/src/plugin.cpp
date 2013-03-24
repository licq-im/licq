/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq developers <licq-dev@googlegroups.com>
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

#include <boost/foreach.hpp>

#include "client.h"
#include "owner.h"
#include "plugin.h"
#include "sessionmanager.h"
#include "user.h"
#include "vcard.h"

#include <licq/contactlist/usermanager.h>
#include <licq/event.h>
#include <licq/logging/log.h>
#include <licq/oneventmanager.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/protocolsignal.h>
#include <licq/statistics.h>
#include <licq/userevents.h>

using namespace LicqJabber;

using Licq::OnEventData;
using Licq::gOnEventManager;
using Licq::gLog;
using std::string;

Plugin::Plugin()
  : myClient(NULL)
{
  gLog.debug("Using gloox version %s", gloox::GLOOX_VERSION.c_str());
}

Plugin::~Plugin()
{
  delete myClient;
}

int Plugin::run()
{
  myMainLoop.addRawFile(getReadPipe(), this);
  myMainLoop.run();
  return 0;
}

void Plugin::rawFileEvent(int fd, int /*revents*/)
{
  char ch;
  ::read(fd, &ch, sizeof(ch));

  switch (ch)
  {
    case PipeSignal:
      processSignal(popSignal().get());
      break;
    case PipeShutdown:
      doLogoff();
      myMainLoop.quit();
      break;
    default:
      gLog.error("Unknown command %c", ch);
      break;
  }
}

void Plugin::processSignal(const Licq::ProtocolSignal* signal)
{
  assert(signal != NULL);

  switch (signal->signal())
  {
    case Licq::ProtocolSignal::SignalLogon:
      doLogon(dynamic_cast<const Licq::ProtoLogonSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalLogoff:
      doLogoff();
      break;
    case Licq::ProtocolSignal::SignalChangeStatus:
      doChangeStatus(
          dynamic_cast<const Licq::ProtoChangeStatusSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalAddUser:
      doAddUser(dynamic_cast<const Licq::ProtoAddUserSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalRemoveUser:
      doRemoveUser(dynamic_cast<const Licq::ProtoRemoveUserSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalRenameUser:
      doRenameUser(dynamic_cast<const Licq::ProtoRenameUserSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalChangeUserGroups:
      doChangeUserGroups(
          dynamic_cast<const Licq::ProtoChangeUserGroupsSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalSendMessage:
      doSendMessage(dynamic_cast<const Licq::ProtoSendMessageSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalNotifyTyping:
      doNotifyTyping(
          dynamic_cast<const Licq::ProtoTypingNotificationSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalGrantAuth:
      doGrantAuth(dynamic_cast<const Licq::ProtoGrantAuthSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalRefuseAuth:
      doRefuseAuth(dynamic_cast<const Licq::ProtoRefuseAuthSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalRequestInfo:
      doGetInfo(dynamic_cast<const Licq::ProtoRequestInfo*>(signal));
      break;
    case Licq::ProtocolSignal::SignalUpdateInfo:
      doUpdateInfo(dynamic_cast<const Licq::ProtoUpdateInfoSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalRequestPicture:
      doGetPicture(dynamic_cast<const Licq::ProtoRequestPicture*>(signal));
      break;
    case Licq::ProtocolSignal::SignalRequestAuth:
      doRequestAuth(dynamic_cast<const Licq::ProtoRequestAuthSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalRenameGroup:
      doRenameGroup(dynamic_cast<const Licq::ProtoRenameGroupSignal*>(signal));
      break;
    default:
      gLog.error("Unknown signal %u", signal->signal());
      /* Unsupported action, if it has an eventId, cancel it */
      if (signal->eventId() != 0)
        Licq::gPluginManager.pushPluginEvent(
            new Licq::Event(signal, Licq::Event::ResultUnsupported));
      break;
  }
}

void Plugin::doLogon(const Licq::ProtoLogonSignal* signal)
{
  unsigned status = signal->status();
  if (status == User::OfflineStatus)
    return;

  string username;
  string password;
  string host;
  int port;
  string resource;
  gloox::TLSPolicy tlsPolicy;
  {
    OwnerReadGuard owner(signal->userId());
    if (!owner.isLocked())
    {
      gLog.error("No owner set");
      return;
    }

    username = owner->accountId();
    password = owner->password();
    host = owner->serverHost();
    port = owner->serverPort();
    resource = owner->resource();
    tlsPolicy = owner->tlsPolicy();
  }

  if (myClient == NULL)
    myClient = new Client(myMainLoop, signal->userId(), username, password,
                          host, port, resource, tlsPolicy);
  else
    myClient->setPassword(password);

  if (!myClient->isConnected())
  {
    if (!myClient->connect(status))
    {
      delete myClient;
      myClient = NULL;
      return;
    }
  }
}

void Plugin::doChangeStatus(const Licq::ProtoChangeStatusSignal* signal)
{
  assert(myClient != NULL);
  myClient->changeStatus(signal->status());
}

void Plugin::doLogoff()
{
  if (myClient == NULL)
    return;

  delete myClient;
  myClient = NULL;
}

void Plugin::doSendMessage(const Licq::ProtoSendMessageSignal* signal)
{
  assert(myClient != NULL);

  bool isUrgent = (signal->flags() & Licq::ProtocolSignal::SendUrgent);

  myClient->getSessionManager()->sendMessage(
      signal->userId().accountId(), signal->message(), isUrgent);

  Licq::EventMsg* message = new Licq::EventMsg(
      signal->message().c_str(), Licq::EventMsg::TimeNow,
      Licq::EventMsg::FlagSender);

  Licq::Event* event =
      new Licq::Event(signal, Licq::Event::ResultAcked, message);
  event->myCommand = Licq::Event::CommandMessage;

  if (event->m_pUserEvent)
  {
    UserWriteGuard user(signal->userId());
    if (user.isLocked())
    {
      event->m_pUserEvent->AddToHistory(*user, false);
      user->SetLastSentEvent();
      gOnEventManager.performOnEvent(OnEventData::OnEventMsgSent, *user);
    }
    Licq::gStatistics.increase(Licq::Statistics::EventsSentCounter);
  }

  Licq::gPluginManager.pushPluginEvent(event);
}

void Plugin::doNotifyTyping(const Licq::ProtoTypingNotificationSignal* signal)
{
  assert(myClient != NULL);

  myClient->getSessionManager()->notifyTyping(
      signal->userId().accountId(), signal->active());
}

void Plugin::doGetInfo(const Licq::ProtoRequestInfo* signal)
{
  assert(myClient != NULL);
  myClient->getVCard(signal->userId().accountId());

  Licq::gPluginManager.pushPluginEvent(new Licq::Event(signal));
}

void Plugin::doUpdateInfo(const Licq::ProtoUpdateInfoSignal* signal)
{
  assert(myClient != NULL);
  OwnerReadGuard owner(signal->userId());
  if (!owner.isLocked())
  {
    gLog.error("No owner set");
    return;
  }

  UserToVCard vcard(*owner);
  myClient->setOwnerVCard(vcard);

  Licq::gPluginManager.pushPluginEvent(new Licq::Event(signal));
}

void Plugin::doGetPicture(const Licq::ProtoRequestPicture* signal)
{
  assert(myClient != NULL);
  myClient->getVCard(signal->userId().accountId());

  Licq::gPluginManager.pushPluginEvent(new Licq::Event(signal));
}

void Plugin::doAddUser(const Licq::ProtoAddUserSignal* signal)
{
  assert(myClient != NULL);
  const Licq::UserId userId = signal->userId();
  gloox::StringList groupNames;
  getUserGroups(userId, groupNames);
  myClient->addUser(userId.accountId(), groupNames, true);
}

void Plugin::doChangeUserGroups(
    const Licq::ProtoChangeUserGroupsSignal* signal)
{
  assert(myClient != NULL);
  const Licq::UserId userId = signal->userId();
  gloox::StringList groupNames;
  getUserGroups(userId, groupNames);
  myClient->changeUserGroups(userId.accountId(), groupNames);
}

void Plugin::doRemoveUser(const Licq::ProtoRemoveUserSignal* signal)
{
  assert(myClient != NULL);
  myClient->removeUser(signal->userId().accountId());
  Licq::gUserManager.removeLocalUser(signal->userId());
}

void Plugin::doRenameUser(const Licq::ProtoRenameUserSignal* signal)
{
  assert(myClient != NULL);
  string newName;
  {
    UserReadGuard u(signal->userId());
    if (!u.isLocked())
      return;
    newName = u->getAlias();
  }

  myClient->renameUser(signal->userId().accountId(), newName);
}

void Plugin::doGrantAuth(const Licq::ProtoGrantAuthSignal* signal)
{
  assert(myClient != NULL);
  myClient->grantAuthorization(signal->userId().accountId());

  Licq::gPluginManager.pushPluginEvent(new Licq::Event(signal));
}

void Plugin::doRefuseAuth(const Licq::ProtoRefuseAuthSignal* signal)
{
  assert(myClient != NULL);
  myClient->refuseAuthorization(signal->userId().accountId());

  Licq::gPluginManager.pushPluginEvent(new Licq::Event(signal));
}

void Plugin::doRequestAuth(const Licq::ProtoRequestAuthSignal* signal)
{
  assert(myClient != NULL);
  myClient->requestAuthorization(
      signal->userId().accountId(), signal->message());
}

void Plugin::doRenameGroup(const Licq::ProtoRenameGroupSignal* signal)
{
  Licq::UserListGuard userList(signal->userId());
  BOOST_FOREACH(Licq::User* licqUser, **userList)
  {
    Licq::UserReadGuard user(licqUser);

    if (!user->isInGroup(signal->groupId()))
      continue;

    // User is member of renamed group, get complete group list and update
    // server
    gloox::StringList groupNames;
    const Licq::UserGroupList& groups = user->GetGroups();
    BOOST_FOREACH(int groupId, groups)
    {
      string groupName = Licq::gUserManager.GetGroupNameFromGroup(groupId);
      if (!groupName.empty())
        groupNames.push_back(groupName);
    }
    myClient->changeUserGroups(user->id().accountId(), groupNames);
  }
}

void Plugin::getUserGroups(const Licq::UserId& userId,
                           gloox::StringList& retGroupNames)
{
  UserReadGuard user(userId);
  if (!user.isLocked())
    return;

  const Licq::UserGroupList& groups = user->GetGroups();
  BOOST_FOREACH(int groupId, groups)
  {
    string groupName = Licq::gUserManager.GetGroupNameFromGroup(groupId);
    if (!groupName.empty())
      retGroupNames.push_back(groupName);
  }
}
