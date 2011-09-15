/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq Developers <licq-dev@googlegroups.com>
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
#include "handler.h"
#include "plugin.h"
#include "pluginversion.h"
#include "sessionmanager.h"
#include "vcard.h"

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/event.h>
#include <licq/logging/log.h>
#include <licq/oneventmanager.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/protocolsignal.h>
#include <licq/statistics.h>
#include <licq/userevents.h>

#include <sys/select.h>

using namespace Jabber;

using Licq::OnEventData;
using Licq::gOnEventManager;
using Licq::gLog;
using std::string;

const time_t PING_TIMEOUT = 60;

const char* const JabberConfigFile = "licq_jabber.conf";

Plugin::Plugin(Params& p) :
    Licq::ProtocolPlugin(p),
    myConfig(JabberConfigFile),
  myHandler(NULL),
  myDoRun(false),
  myClient(NULL)
{
  gLog.debug("Using gloox version %s", gloox::GLOOX_VERSION.c_str());
  myHandler = new Handler();
}

Plugin::~Plugin()
{
  delete myClient;
  delete myHandler;
}

string Plugin::name() const
{
  return "Jabber";
}

string Plugin::version() const
{
  return PLUGIN_VERSION_STRING;
}

string Plugin::configFile() const
{
  return JabberConfigFile;
}

unsigned long Plugin::protocolId() const
{
  return JABBER_PPID;
}

unsigned long Plugin::capabilities() const
{
  return Licq::ProtocolPlugin::CanSendMsg
      | Licq::ProtocolPlugin::CanHoldStatusMsg
      | Licq::ProtocolPlugin::CanSendAuth
      | Licq::ProtocolPlugin::CanSendAuthReq;
}

string Plugin::defaultServerHost() const
{
  return string();
}

int Plugin::defaultServerPort() const
{
  return 5222;
}

bool Plugin::init(int, char**)
{
  return true;
}

int Plugin::run()
{
  int pipe = getReadPipe();

  fd_set readFds;

  time_t lastPing = 0;
  struct timeval pingTimeout;

  myDoRun = (pipe != -1);
  while (myDoRun)
  {
    FD_ZERO(&readFds);
    FD_SET(pipe, &readFds);
    int nfds = pipe + 1;
    struct timeval* timeout = NULL;

    const time_t now = ::time(NULL);
    if (lastPing == 0)
      lastPing = now;

    int sock = -1;
    if (myClient != NULL)
    {
      sock = myClient->getSocket();
      if (sock != -1)
      {
        FD_SET(sock, &readFds);
        if (sock > pipe)
          nfds = sock + 1;

        if (lastPing + PING_TIMEOUT <= now)
        {
          myClient->ping();
          lastPing = now;
          pingTimeout.tv_sec = PING_TIMEOUT;
        }
        else
          pingTimeout.tv_sec = std::min(PING_TIMEOUT, now - lastPing);

        pingTimeout.tv_usec = 0;
        timeout = &pingTimeout;
      }
    }
    else
      lastPing = 0;

    if (::select(nfds, &readFds, NULL, NULL, timeout) > 0)
    {
      if (sock != -1 && FD_ISSET(sock, &readFds))
        myClient->recv();
      if (FD_ISSET(pipe, &readFds))
        processPipe(pipe);
    }
  }

  return 0;
}

void Plugin::destructor()
{
  delete this;
}

void Plugin::processPipe(int pipe)
{
  char ch;
  ::read(pipe, &ch, sizeof(ch));

  switch (ch)
  {
    case Licq::ProtocolPlugin::PipeSignal:
    {
      Licq::ProtocolSignal* signal = popSignal();
      processSignal(signal);
      delete signal;
      break;
    }
    case Licq::ProtocolPlugin::PipeShutdown:
      doLogoff();
      myDoRun = false;
      break;
    default:
      gLog.error("Unknown command %c", ch);
      break;
  }
}

void Plugin::processSignal(Licq::ProtocolSignal* signal)
{
  assert(signal != NULL);

  switch (signal->signal())
  {
    case Licq::ProtocolSignal::SignalLogon:
      doLogon(static_cast<Licq::ProtoLogonSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalLogoff:
      doLogoff();
      break;
    case Licq::ProtocolSignal::SignalChangeStatus:
      doChangeStatus(static_cast<Licq::ProtoChangeStatusSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalAddUser:
      doAddUser(static_cast<Licq::ProtoAddUserSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalRemoveUser:
      doRemoveUser(static_cast<Licq::ProtoRemoveUserSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalRenameUser:
      doRenameUser(static_cast<Licq::ProtoRenameUserSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalChangeUserGroups:
      doChangeUserGroups(
          static_cast<Licq::ProtoChangeUserGroupsSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalSendMessage:
      doSendMessage(static_cast<Licq::ProtoSendMessageSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalNotifyTyping:
      doNotifyTyping(static_cast<Licq::ProtoTypingNotificationSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalGrantAuth:
      doGrantAuth(static_cast<Licq::ProtoGrantAuthSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalRefuseAuth:
      doRefuseAuth(static_cast<Licq::ProtoRefuseAuthSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalRequestInfo:
      doGetInfo(static_cast<Licq::ProtoRequestInfo*>(signal));
      break;
    case Licq::ProtocolSignal::SignalUpdateInfo:
      doUpdateInfo(static_cast<Licq::ProtoUpdateInfoSignal*>(signal));
      break;
    case Licq::ProtocolSignal::SignalRequestPicture:
      gLog.info("SignalRequestPicture not implemented");
      break;
    case Licq::ProtocolSignal::SignalBlockUser:
      gLog.info("SignalBlockUser not implemented");
      break;
    case Licq::ProtocolSignal::SignalUnblockUser:
      gLog.info("SignalUnblockUser not implemented");
      break;
    case Licq::ProtocolSignal::SignalAcceptUser:
      gLog.info("SignalAcceptUser not implemented");
      break;
    case Licq::ProtocolSignal::SignalUnacceptUser:
      gLog.info("SignalUnacceptUser not implemented");
      break;
    case Licq::ProtocolSignal::SignalIgnoreUser:
      gLog.info("SignalIgnoreUser not implemented");
      break;
    case Licq::ProtocolSignal::SignalUnignoreUser:
      gLog.info("SignalUnignoreUser not implemented");
      break;
    case Licq::ProtocolSignal::SignalSendFile:
      gLog.info("SignalSendFile not implemented");
      break;
    case Licq::ProtocolSignal::SignalSendChat:
      gLog.info("SignalSendChat not implemented");
      break;
    case Licq::ProtocolSignal::SignalCancelEvent:
      gLog.info("SignalCancelEvent not implemented");
      break;
    case Licq::ProtocolSignal::SignalSendReply:
      gLog.info("SignalSendReply not implemented");
      break;
    case Licq::ProtocolSignal::SignalOpenedWindow:
      gLog.info("SignalOpenedWindow not implemented");
      break;
    case Licq::ProtocolSignal::SignalClosedWindow:
      gLog.info("SignalClosedWindow not implemented");
      break;
    case Licq::ProtocolSignal::SignalOpenSecure:
      gLog.info("SignalOpenSecure not implemented");
      break;
    case Licq::ProtocolSignal::SignalCloseSecure:
      gLog.info("SignalCloseSecure not implemented");
      break;
    case Licq::ProtocolSignal::SignalRequestAuth:
      doRequestAuth(static_cast<Licq::ProtoRequestAuthSignal*>(signal));
      break;
    default:
      gLog.error("Unknown signal %u", signal->signal());
      break;
  }
}

void Plugin::doLogon(Licq::ProtoLogonSignal* signal)
{
  unsigned status = signal->status();
  if (status == Licq::User::OfflineStatus)
    return;

  string username;
  string password;
  string host;
  int port;
  {
    Licq::OwnerReadGuard owner(JABBER_PPID);
    if (!owner.isLocked())
    {
      gLog.error("No owner set");
      return;
    }

    username = owner->accountId();
    password = owner->password();
    host = owner->serverHost();
    port = owner->serverPort();
  }

  if (myClient == NULL)
    myClient = new Client(myConfig, *myHandler, username, password, host, port);
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

void Plugin::doChangeStatus(Licq::ProtoChangeStatusSignal* signal)
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

void Plugin::doSendMessage(Licq::ProtoSendMessageSignal* signal)
{
  assert(myClient != NULL);

  bool isUrgent = (signal->flags() & Licq::ProtocolSignal::SendUrgent);

  myClient->getSessionManager()->sendMessage(
      signal->userId().accountId(), signal->message(), isUrgent);

  Licq::EventMsg* message = new Licq::EventMsg(
      signal->message().c_str(), Licq::EventMsg::TimeNow, Licq::EventMsg::FlagSender);

  Licq::Event* event = new Licq::Event(signal->eventId(), 0, NULL,
      Licq::Event::ConnectServer, signal->userId(), message);
  event->myCommand = Licq::Event::CommandMessage;
  event->thread_plugin = signal->callerThread();
  event->m_eResult = Licq::Event::ResultAcked;

  if (event->m_pUserEvent)
  {
    Licq::UserWriteGuard user(signal->userId());
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

void Plugin::doNotifyTyping(Licq::ProtoTypingNotificationSignal* signal)
{
  assert(myClient != NULL);

  myClient->getSessionManager()->notifyTyping(
      signal->userId().accountId(), signal->active());
}

void Plugin::doGetInfo(Licq::ProtoRequestInfo* signal)
{
  assert(myClient != NULL);
  myClient->getVCard(signal->userId().accountId());
}

void Plugin::doUpdateInfo(Licq::ProtoUpdateInfoSignal* /*signal*/)
{
  assert(myClient != NULL);
  Licq::OwnerReadGuard owner(JABBER_PPID);
  if (!owner.isLocked())
  {
    gLog.error("No owner set");
    return;
  }

  UserToVCard vcard(*owner);
  myClient->setOwnerVCard(vcard);
}

void Plugin::doAddUser(Licq::ProtoAddUserSignal* signal)
{
  assert(myClient != NULL);
  const Licq::UserId userId = signal->userId();
  gloox::StringList groupNames;
  getUserGroups(userId, groupNames);
  myClient->addUser(userId.accountId(), groupNames, true);
}

void Plugin::doChangeUserGroups(Licq::ProtoChangeUserGroupsSignal* signal)
{
  assert(myClient != NULL);
  const Licq::UserId userId = signal->userId();
  gloox::StringList groupNames;
  getUserGroups(userId, groupNames);
  myClient->changeUserGroups(userId.accountId(), groupNames);
}

void Plugin::getUserGroups(const Licq::UserId& userId, gloox::StringList& retGroupNames)
{
  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return;

  const Licq::UserGroupList groups = u->GetGroups();
  BOOST_FOREACH(int groupId, groups)
  {
    string groupName = Licq::gUserManager.GetGroupNameFromGroup(groupId);
    if (!groupName.empty())
      retGroupNames.push_back(groupName);
  }
}

void Plugin::doRemoveUser(Licq::ProtoRemoveUserSignal* signal)
{
  assert(myClient != NULL);
  myClient->removeUser(signal->userId().accountId());
}

void Plugin::doRenameUser(Licq::ProtoRenameUserSignal* signal)
{
  assert(myClient != NULL);
  string newName;
  {
    Licq::UserReadGuard u(signal->userId());
    if (!u.isLocked())
      return;
    newName = u->getAlias();
  }

  myClient->renameUser(signal->userId().accountId(), newName);
}

void Plugin::doGrantAuth(Licq::ProtoGrantAuthSignal* signal)
{
  assert(myClient != NULL);
  myClient->grantAuthorization(signal->userId().accountId());
}

void Plugin::doRefuseAuth(Licq::ProtoRefuseAuthSignal* signal)
{
  assert(myClient != NULL);
  myClient->refuseAuthorization(signal->userId().accountId());
}

void Plugin::doRequestAuth(Licq::ProtoRequestAuthSignal* signal)
{
  assert(myClient != NULL);
  myClient->requestAuthorization(
      signal->userId().accountId(), signal->message());
}
