/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq Developers <licq-dev@googlegroups.com>
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

#include "client.h"
#include "handler.h"
#include "jabber.h"

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/daemon.h>
#include <licq/log.h>
#include <licq_events.h>

#include <sys/select.h>

using Licq::gLog;
using std::string;

Jabber::Jabber() :
  myHandler(NULL),
  myDoRun(false),
  myClient(NULL)
{
  myHandler = new Handler();
}

Jabber::~Jabber()
{
  delete myClient;
  delete myHandler;
}

int Jabber::run(int pipe)
{
  fd_set readFds;

  myDoRun = (pipe != -1);
  while (myDoRun)
  {
    FD_ZERO(&readFds);
    FD_SET(pipe, &readFds);
    int nfds = pipe + 1;

    int sock = -1;
    if (myClient != NULL)
    {
      sock = myClient->getSocket();
      if (sock != -1)
      {
        FD_SET(sock, &readFds);
        if (sock > pipe)
          nfds = sock + 1;
      }
    }

    if (::select(nfds, &readFds, NULL, NULL, NULL) > 0)
    {
      if (sock != -1 && FD_ISSET(sock, &readFds))
        myClient->recv();
      if (FD_ISSET(pipe, &readFds))
        processPipe(pipe);
    }
  }

  return 0;
}

void Jabber::processPipe(int pipe)
{
  char ch;
  ::read(pipe, &ch, sizeof(ch));

  switch (ch)
  {
    case PLUGIN_SIGNAL:
    {
      LicqProtoSignal* signal = Licq::gDaemon->PopProtoSignal();
      processSignal(signal);
      delete signal;
    }
    break;
    case PLUGIN_SHUTDOWN:
      myDoRun = false;
      break;
    default:
      gLog.error("Unkown command %c", ch);
      break;
  }
}

void Jabber::processSignal(LicqProtoSignal* signal)
{
  assert(signal != NULL);

  gLog.info("Got signal %u", signal->type());
  switch (signal->type())
  {
    case PROTOxLOGON:
      doLogon(static_cast<LicqProtoLogonSignal*>(signal));
      break;
    case PROTOxCHANGE_STATUS:
      doChangeStatus(static_cast<LicqProtoChangeStatusSignal*>(signal));
      break;
    case PROTOxLOGOFF:
      doLogoff();
      break;
    case PROTOxSENDxMSG:
      doSendMessage(static_cast<LicqProtoSendMessageSignal*>(signal));
      break;
    case PROTOxREQUESTxINFO:
      doGetInfo(static_cast<LicqProtoRequestInfo*>(signal));
      break;
    case PROTOxADD_USER:
      doAddUser(static_cast<LicqProtoAddUserSignal*>(signal));
      break;
    case PROTOxCHANGE_USER_GROUPS:
      doChangeUserGroups(static_cast<LicqProtoChangeUserGroupsSignal*>(signal));
      break;
    case PROTOxREM_USER:
      doRemoveUser(static_cast<LicqProtoRemoveUserSignal*>(signal));
      break;
    case PROTOxRENAME_USER:
      doRenameUser(static_cast<LicqProtoRenameUserSignal*>(signal));
      break;
    case PROTOxSENDxTYPING_NOTIFICATION:
    case PROTOxSENDxGRANTxAUTH:
    case PROTOxSENDxREFUSExAUTH:
    case PROTOxUPDATExINFO:
    case PROTOxREQUESTxPICTURE:
    case PROTOxBLOCKxUSER:
    case PROTOxUNBLOCKxUSER:
    case PROTOxACCEPTxUSER:
    case PROTOxUNACCEPTxUSER:
    case PROTOxIGNORExUSER:
    case PROTOxUNIGNORExUSER:
    case PROTOxSENDxFILE:
    case PROTOxSENDxCHAT:
    case PROTOxCANCELxEVENT:
    case PROTOxSENDxEVENTxREPLY:
    case PROTOxOPENEDxWINDOW:
    case PROTOxCLOSEDxWINDOW:
    case PROTOxOPENxSECURE:
    case PROTOxCLOSExSECURE:
    default:
      gLog.info("Unkown signal %u", signal->type());
      break;
  }
}

void Jabber::doLogon(LicqProtoLogonSignal* signal)
{
  unsigned status = signal->status();
  if (status == Licq::User::OfflineStatus)
    return;

  string username;
  string password;
  {
    Licq::OwnerReadGuard owner(JABBER_PPID);
    if (!owner.isLocked())
    {
      gLog.error("No owner set");
      return;
    }

    username = owner->accountId();
    password = owner->password();
  }

  myHandler->setStatus(status);

  if (myClient == NULL)
    myClient = new Client(*myHandler, username, password);
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

void Jabber::doChangeStatus(LicqProtoChangeStatusSignal* signal)
{
  assert(myClient != NULL);
  myClient->changeStatus(signal->status());
}

void Jabber::doLogoff()
{
  if (myClient == NULL)
    return;

  delete myClient;
  myClient = NULL;
}

void Jabber::doSendMessage(LicqProtoSendMessageSignal* signal)
{
  assert(myClient != NULL);
  myClient->sendMessage(signal->userId().accountId(), signal->message());

  CEventMsg* message = new CEventMsg(signal->message().c_str(), 0, TIME_NOW, 0);
  message->m_eDir = D_SENDER;

  LicqEvent* event = new LicqEvent(signal->eventId(), 0, NULL, CONNECT_SERVER,
                                   signal->userId(), message);
  event->thread_plugin = signal->callerThread();
  event->m_eResult = EVENT_ACKED;

  if (event->m_pUserEvent)
  {
    Licq::UserWriteGuard user(signal->userId());
    event->m_pUserEvent->AddToHistory(*user, D_SENDER);
  }

  Licq::gDaemon->PushPluginEvent(event);
}

void Jabber::doGetInfo(LicqProtoRequestInfo* signal)
{
  assert(myClient != NULL);
  myClient->getVCard(signal->userId().accountId());
}

void Jabber::doAddUser(LicqProtoAddUserSignal* signal)
{
  assert(myClient != NULL);
  myClient->addUser(signal->userId().accountId());
}

void Jabber::doChangeUserGroups(LicqProtoChangeUserGroupsSignal* signal)
{
  assert(myClient != NULL);
  myClient->changeUserGroups(signal->userId().accountId(),
      signal->groups());
}

void Jabber::doRemoveUser(LicqProtoRemoveUserSignal* signal)
{
  assert(myClient != NULL);
  myClient->removeUser(signal->userId().accountId());
}

void Jabber::doRenameUser(LicqProtoRenameUserSignal* signal)
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
