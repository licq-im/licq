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

#include "handler.h"
#include "jabber.h"

#include <licq_log.h>
#include <licq_icqd.h>

#define TRACE() gLog.Info("%sIn Handler::%s()\n", L_JABBERxSTR, __func__)

Handler::Handler(CICQDaemon* daemon) :
  myDaemon(daemon),
  myStatus(ICQ_STATUS_OFFLINE)
{
  // Empty
}

Handler::~Handler()
{
  // Empty
}

void Handler::onConnect()
{
  TRACE();

  LicqOwner* owner = gUserManager.FetchOwner(JABBER_PPID, LOCK_W);
  myDaemon->ChangeUserStatus(owner, myStatus);
  gUserManager.DropOwner(owner);

  LicqSignal* result = new LicqSignal(SIGNAL_LOGON, 0,
                                      USERID_NONE, JABBER_PPID);
  myDaemon->pushPluginSignal(result);
}

void Handler::onChangeStatus(unsigned long status)
{
  TRACE();

  LicqOwner* owner = gUserManager.FetchOwner(JABBER_PPID, LOCK_W);
  myDaemon->ChangeUserStatus(owner, status);

  LicqSignal* result = new LicqSignal(SIGNAL_UPDATExUSER, USER_STATUS,
                                      owner->id(), JABBER_PPID);
  gUserManager.DropOwner(owner);

  myDaemon->pushPluginSignal(result);
}

void Handler::onDisconnect()
{
  TRACE();

  FOR_EACH_PROTO_USER_START(JABBER_PPID, LOCK_W)
  {
    if (!pUser->StatusOffline())
      myDaemon->ChangeUserStatus(pUser, ICQ_STATUS_OFFLINE);
  }
  FOR_EACH_PROTO_USER_END;

  LicqOwner* owner = gUserManager.FetchOwner(JABBER_PPID, LOCK_W);
  myDaemon->ChangeUserStatus(owner, ICQ_STATUS_OFFLINE);
  gUserManager.DropOwner(owner);

  LicqSignal* result = new LicqSignal(SIGNAL_LOGOFF, 0,
                                      USERID_NONE, JABBER_PPID);
  myDaemon->pushPluginSignal(result);
}
