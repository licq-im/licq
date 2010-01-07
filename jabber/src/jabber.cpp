/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Erik Johansson <erijo@licq.org>
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

#include <licq_log.h>
#include <licq_icqd.h>
#include <licq_user.h>

#include <sys/select.h>

Jabber::Jabber(CICQDaemon* daemon) :
  myDaemon(daemon),
  myHandler(NULL),
  myDoRun(false),
  myClient(NULL)
{
  myHandler = new Handler(daemon);
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
      FD_SET(sock, &readFds);
      if (sock > pipe)
        nfds = sock + 1;
    }

    if (::select(nfds, &readFds, NULL, NULL, NULL) > 0)
    {
      if (myClient != NULL && FD_ISSET(sock, &readFds))
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
      LicqProtoSignal* signal = myDaemon->PopProtoSignal();
      processSignal(signal);
      delete signal;
    }
    break;
    case PLUGIN_SHUTDOWN:
      myDoRun = false;
      break;
    default:
      gLog.Error("%sUnkown command %c\n", L_JABBERxSTR, ch);
      break;
  }
}

void Jabber::processSignal(LicqProtoSignal* signal)
{
  assert(signal != NULL);

  gLog.Info("%sGot signal %u\n", L_JABBERxSTR, signal->type());
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
    default:
      gLog.Info("%sUnkown signal %u\n", L_JABBERxSTR, signal->type());
      break;
  }
}

void Jabber::doLogon(LicqProtoLogonSignal* signal)
{
  unsigned long status = signal->status();
  if (status == ICQ_STATUS_OFFLINE)
    return;

  if (myClient == NULL)
  {
    const LicqOwner* owner = gUserManager.FetchOwner(JABBER_PPID, LOCK_R);
    if (owner == NULL)
    {
      gLog.Error("%sNo owner set\n", L_JABBERxSTR);
      return;
    }

    std::string username = owner->IdString();
    std::string password = owner->Password();
    gUserManager.DropOwner(owner);

    myHandler->setStatus(status);
    myClient = new Client(*myHandler, username, password);
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
  myHandler->onChangeStatus(signal->status());
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
  (void)signal;
}

void Jabber::doGetInfo(LicqProtoRequestInfo* signal)
{
  myClient->getVCard(LicqUser::getUserAccountId(signal->userId()));
}
