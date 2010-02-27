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

#include <licq/log.h>
#include <licq_icqd.h>
#include <licq_user.h>

#define TRACE() Licq::gLog.info("In Handler::%s()", __func__)

Handler::Handler() :
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
  gLicqDaemon->ChangeUserStatus(owner, myStatus);
  gUserManager.DropOwner(owner);

  LicqSignal* result = new LicqSignal(SIGNAL_LOGON, 0,
                                      USERID_NONE, JABBER_PPID);
  gLicqDaemon->pushPluginSignal(result);
}

void Handler::onChangeStatus(unsigned long status)
{
  TRACE();

  LicqOwner* owner = gUserManager.FetchOwner(JABBER_PPID, LOCK_W);
  gLicqDaemon->ChangeUserStatus(owner, status);

  LicqSignal* result = new LicqSignal(SIGNAL_UPDATExUSER, USER_STATUS,
                                      owner->id(), JABBER_PPID);
  gUserManager.DropOwner(owner);

  gLicqDaemon->pushPluginSignal(result);
}

void Handler::onDisconnect()
{
  TRACE();

  FOR_EACH_PROTO_USER_START(JABBER_PPID, LOCK_W)
  {
    if (!pUser->StatusOffline())
      gLicqDaemon->ChangeUserStatus(pUser, ICQ_STATUS_OFFLINE);
  }
  FOR_EACH_PROTO_USER_END;

  LicqOwner* owner = gUserManager.FetchOwner(JABBER_PPID, LOCK_W);
  gLicqDaemon->ChangeUserStatus(owner, ICQ_STATUS_OFFLINE);
  gUserManager.DropOwner(owner);

  LicqSignal* result = new LicqSignal(SIGNAL_LOGOFF, 0,
                                      USERID_NONE, JABBER_PPID);
  gLicqDaemon->pushPluginSignal(result);
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
      groupId = gUserManager.AddGroup(*it);
    if (groupId == 0)
      continue;
    glist.insert(groupId);
  }
  user->SetGroups(glist);

  user->SetUserEncoding("UTF-8");
  if (!user->KeepAliasOnUpdate())
    user->setAlias(name);

  // Remove this line when SetGroups call above saves contact groups itself.
  user->SaveLicqInfo();

  gUserManager.DropUser(user);

  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_BASIC, userId));
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_GROUPS, userId));
}

void Handler::onUserRemoved(const std::string& id)
{
  TRACE();

  UserId userId = LicqUser::makeUserId(id, JABBER_PPID);
  gUserManager.removeUser(userId, false);
}

void Handler::onUserStatusChange(const std::string& id, const unsigned long newStatus)
{
  TRACE();

  UserId userId = LicqUser::makeUserId(id, JABBER_PPID);

  LicqUser* user = gUserManager.fetchUser(userId, LOCK_W);
  assert(user != NULL);
  user->SetStatus(newStatus);

  gLicqDaemon->pushPluginSignal(
      new LicqSignal(SIGNAL_UPDATExUSER, USER_STATUS, user->id(), JABBER_PPID));

  gUserManager.DropUser(user);
}

void Handler::onRosterReceived(const std::set<std::string>& ids)
{
  TRACE();

  std::list<UserId> todel;
  std::list<UserId>::const_iterator it;

  FOR_EACH_PROTO_USER_START(JABBER_PPID, LOCK_R)
  {
    if (ids.count(pUser->accountId()) == 0)
      todel.push_back(pUser->id());
  }
  FOR_EACH_PROTO_USER_END;

  for (it = todel.begin(); it != todel.end(); ++it)
    gUserManager.removeUser(*it, false);
}

void Handler::onMessage(const std::string& from, const std::string& message)
{
  TRACE();
  
  CEventMsg* event = new CEventMsg(
      message.c_str(), ICQ_CMDxRCV_SYSxMSGxOFFLINE, ::time(0), 0);

  UserId userId = LicqUser::makeUserId(from, JABBER_PPID);
  LicqUser* user = gUserManager.fetchUser(userId, LOCK_W, true);

  if (user)
    user->SetTyping(0);
  if (gLicqDaemon->AddUserEvent(user, event))
    gLicqDaemon->m_xOnEventManager.Do(ON_EVENT_MSG, user);
  gUserManager.DropUser(user);
}

std::string Handler::getStatusMessage(unsigned long status)
{
  std::string msg = std::string();
  LicqOwner* owner = NULL;

  if ((status & ~ICQ_STATUS_FxFLAGS) != ICQ_STATUS_ONLINE)
    owner = gUserManager.FetchOwner(JABBER_PPID, LOCK_R);

  if (owner != NULL)
  {
    msg = owner->AutoResponse();
    gUserManager.DropOwner(owner);
  }

  return msg;
}
