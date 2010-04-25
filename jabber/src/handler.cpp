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

#include "handler.h"
#include "jabber.h"

#include <licq/log.h>
#include <licq/oneventmanager.h>
#include <licq_icqd.h>
#include <licq_user.h>

using std::string;
using Licq::OnEventManager;
using Licq::gOnEventManager;

#define TRACE() Licq::gLog.info("In Handler::%s()", __func__)

Handler::Handler() :
  myStatus(Licq::User::OfflineStatus),
  myNextConvoId(1)
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

  gLicqDaemon->changeUserStatus(gUserManager.ownerUserId(JABBER_PPID), myStatus);

  LicqSignal* result = new LicqSignal(SIGNAL_LOGON, 0,
                                      USERID_NONE, JABBER_PPID);
  gLicqDaemon->pushPluginSignal(result);
}

void Handler::onChangeStatus(unsigned status)
{
  TRACE();

  gLicqDaemon->changeUserStatus(gUserManager.ownerUserId(JABBER_PPID), status);
}

void Handler::onDisconnect()
{
  TRACE();

  FOR_EACH_PROTO_USER_START(JABBER_PPID, LOCK_W)
  {
    if (pUser->isOnline())
      gLicqDaemon->changeUserStatus(pUser, Licq::User::OfflineStatus);
  }
  FOR_EACH_PROTO_USER_END;

  gLicqDaemon->changeUserStatus(gUserManager.ownerUserId(JABBER_PPID), Licq::User::OfflineStatus);

  LicqSignal* result = new LicqSignal(SIGNAL_LOGOFF, 0,
      gUserManager.ownerUserId(JABBER_PPID));
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

void Handler::onUserStatusChange(const string& id, unsigned status)
{
  TRACE();

  gLicqDaemon->changeUserStatus(Licq::UserId(id, JABBER_PPID), status);
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

  if (myConvoIds.find(from) == myConvoIds.end())
    myConvoIds[from] = myNextConvoId++;

  CEventMsg* event = new CEventMsg(
      message.c_str(), ICQ_CMDxRCV_SYSxMSGxOFFLINE, ::time(0),
      0, myConvoIds[from]);

  UserId userId = LicqUser::makeUserId(from, JABBER_PPID);
  LicqUser* user = gUserManager.fetchUser(userId, LOCK_W, true);

  if (user)
    user->setIsTyping(false);
  if (gLicqDaemon->AddUserEvent(user, event))
    gOnEventManager.performOnEvent(OnEventManager::OnEventMessage, user);
  gUserManager.DropUser(user);
}

std::string Handler::getStatusMessage(unsigned status)
{
  if ((status & Licq::User::MessageStatuses) == 0)
    return string();

  Licq::OwnerReadGuard o(JABBER_PPID);
  if (!o.isLocked())
    return string();

  return o->AutoResponse();
}
