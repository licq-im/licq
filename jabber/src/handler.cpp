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

#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/icqdefines.h>
#include <licq/log.h>
#include <licq/oneventmanager.h>
#include <licq/pluginsignal.h>
#include <licq/userevents.h>

using std::string;
using Licq::OnEventManager;
using Licq::UserId;
using Licq::gOnEventManager;
using Licq::gUserManager;

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

  gUserManager.ownerStatusChanged(JABBER_PPID, myStatus);

  Licq::gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalLogon,
      0, UserId() , JABBER_PPID));
}

void Handler::onChangeStatus(unsigned status)
{
  TRACE();

  gUserManager.ownerStatusChanged(JABBER_PPID, status);
}

void Handler::onDisconnect()
{
  TRACE();

  FOR_EACH_PROTO_USER_START(JABBER_PPID, LOCK_W)
  {
    if (pUser->isOnline())
      pUser->statusChanged(Licq::User::OfflineStatus);
  }
  FOR_EACH_PROTO_USER_END;

  gUserManager.ownerStatusChanged(JABBER_PPID, Licq::User::OfflineStatus);

  Licq::gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalLogoff,
      Licq::PluginSignal::LogoffRequested, gUserManager.ownerUserId(JABBER_PPID)));
}

void Handler::onUserAdded(const std::string& id,
                          const std::string& name,
                          const std::list<std::string>& groups)
{
  TRACE();

  UserId userId(id, JABBER_PPID);
  bool wasAdded = false;
  if (!gUserManager.userExists(userId))
  {
    gUserManager.addUser(userId, true, false);
    wasAdded = true;
  }
  Licq::UserWriteGuard user(userId);
  assert(user.isLocked());
  if (wasAdded)
    user->setAlias(name);

  Licq::UserGroupList glist;
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

  user->setUserEncoding("UTF-8");
  if (!user->KeepAliasOnUpdate())
    user->setAlias(name);

  // Remove this line when SetGroups call above saves contact groups itself.
  user->SaveLicqInfo();

  Licq::gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
      Licq::PluginSignal::UserBasic, userId));
  Licq::gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
      Licq::PluginSignal::UserGroups, userId));
}

void Handler::onUserRemoved(const std::string& id)
{
  TRACE();

  gUserManager.removeUser(UserId(id, JABBER_PPID), false);
}

void Handler::onUserStatusChange(const string& id, unsigned status)
{
  TRACE();

  gUserManager.userStatusChanged(Licq::UserId(id, JABBER_PPID), status);
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

void Handler::onUserAuthorizationRequest(const std::string& id,
                                         const std::string& message)
{
  TRACE();

  Licq::EventAuthRequest* event = new Licq::EventAuthRequest(
      UserId(id, JABBER_PPID),
      std::string(), // alias
      std::string(), std::string(), // first and last name
      std::string(), // email
      message,
      ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);

  Licq::OwnerWriteGuard owner(JABBER_PPID);
  if (Licq::gDaemon.addUserEvent(*owner, event))
  {
    event->AddToHistory(*owner, true);
    gOnEventManager.performOnEvent(OnEventManager::OnEventSysMsg, *owner);
  }
}

void Handler::onMessage(const std::string& from, const std::string& message)
{
  TRACE();

  if (myConvoIds.find(from) == myConvoIds.end())
    myConvoIds[from] = myNextConvoId++;

  Licq::EventMsg* event = new Licq::EventMsg(
      message.c_str(), ICQ_CMDxRCV_SYSxMSGxOFFLINE, ::time(0),
      0, myConvoIds[from]);

  Licq::UserWriteGuard user(UserId(from, JABBER_PPID), true);

  if (user.isLocked())
    user->setIsTyping(false);
  if (Licq::gDaemon.addUserEvent(*user, event))
    gOnEventManager.performOnEvent(OnEventManager::OnEventMessage, *user);
}

std::string Handler::getStatusMessage(unsigned status)
{
  if ((status & Licq::User::MessageStatuses) == 0)
    return string();

  Licq::OwnerReadGuard o(JABBER_PPID);
  if (!o.isLocked())
    return string();

  return o->autoResponse();
}
