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

void Handler::onUserAdded(const std::string& id,
                          const std::string& name,
                          const std::list<std::string>& groups)
{
  TRACE();

  UserId userId = LicqUser::makeUserId(id, JABBER_PPID);

  LicqUser* user = gUserManager.fetchUser(userId, LOCK_W);
  if (user == NULL)
  {
    gUserManager.addUser(userId, true, false);
    user = gUserManager.fetchUser(userId, LOCK_W);
    assert(user != NULL);
    user->setAlias(name);
  }

  UserGroupList glist;
  for (std::list<std::string>::const_iterator it = groups.begin();
      it != groups.end(); ++it)
  {
    int groupId = gUserManager.GetGroupFromName(*it);
    if (groupId == 0)
      continue; // TODO: Implement virtual group creation in the daemon
    glist.insert(groupId);
  }
  user->SetGroups(glist);

  user->SetUserEncoding("UTF-8");
  if (!user->KeepAliasOnUpdate())
    user->setAlias(name);

  // Remove this line when SetGroups call above saves contact groups itself.
  user->SaveLicqInfo();

  gUserManager.DropUser(user);

  myDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER,
                                            USER_GENERAL, userId));
}

void Handler::onUserRemoved(const std::string& /*item*/)
{
  TRACE();
}

void Handler::onUserStatusChange(const std::string& id, const unsigned long newStatus)
{
  TRACE();

  UserId userId = LicqUser::makeUserId(id, JABBER_PPID);

  LicqUser* user = gUserManager.fetchUser(userId, LOCK_W);
  assert(user != NULL);
  user->SetStatus(newStatus);

  myDaemon->pushPluginSignal(
      new LicqSignal(SIGNAL_UPDATExUSER, USER_STATUS, user->id(), JABBER_PPID));

  gUserManager.DropUser(user);
}

void Handler::onMessage(const std::string& from, const std::string& message)
{
  TRACE();

  
  CEventMsg* event = new CEventMsg(
      message.c_str(), ICQ_CMDxRCV_SYSxMSGxOFFLINE, ::time(0), 0);

  UserId userId = LicqUser::makeUserId(from, JABBER_PPID);
  LicqUser* user = gUserManager.fetchUser(userId, LOCK_W);

  if (user)
    user->SetTyping(0);
  if (myDaemon->AddUserEvent(user, event))
    myDaemon->m_xOnEventManager.Do(ON_EVENT_MSG, user);
  gUserManager.DropUser(user);
}
