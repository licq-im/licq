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
#include "vcard.h"

#include <boost/foreach.hpp>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/icqdefines.h>
#include <licq/logging/log.h>
#include <licq/oneventmanager.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/socket.h>
#include <licq/userevents.h>

using namespace Jabber;

using Licq::OnEventData;
using Licq::UserId;
using Licq::gOnEventManager;
using Licq::gUserManager;
using std::string;

#define TRACE() Licq::gLog.debug("In Handler::%s()", __func__)

Handler::Handler() :
  myNextConvoId(1)
{
  // Empty
}

void Handler::onConnect(const string& ip, int port, unsigned status)
{
  TRACE();

  Licq::OwnerWriteGuard owner(JABBER_PPID);
  if (owner.isLocked())
  {
    owner->statusChanged(status);
    owner->SetIpPort(Licq::INetSocket::ipToInt(ip), port);
    owner->SetTimezone(Licq::User::SystemTimezone());
  }

  Licq::gDaemon.pushPluginSignal(
      new Licq::PluginSignal(Licq::PluginSignal::SignalLogon,
                             0, UserId(), JABBER_PPID));
}

void Handler::onChangeStatus(unsigned status)
{
  TRACE();

  gUserManager.ownerStatusChanged(JABBER_PPID, status);
}

void Handler::onDisconnect(bool authError)
{
  TRACE();

  {
    Licq::UserListGuard userList(JABBER_PPID);
    BOOST_FOREACH(Licq::User* user, **userList)
    {
      Licq::UserWriteGuard u(user);
      if (u->isOnline())
        u->statusChanged(Licq::User::OfflineStatus);
    }
  }

  gUserManager.ownerStatusChanged(JABBER_PPID, Licq::User::OfflineStatus);

  Licq::gDaemon.pushPluginSignal(
      new Licq::PluginSignal(Licq::PluginSignal::SignalLogoff,
                             authError ?
                             Licq::PluginSignal::LogoffPassword :
                             Licq::PluginSignal::LogoffRequested,
                             gUserManager.ownerUserId(JABBER_PPID)));
}

void Handler::onUserAdded(
    const string& id, const string& name, const std::list<string>& groups)
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
  for (std::list<string>::const_iterator it = groups.begin();
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

  Licq::gDaemon.pushPluginSignal(
      new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                             Licq::PluginSignal::UserBasic, userId));
  Licq::gDaemon.pushPluginSignal(
      new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                             Licq::PluginSignal::UserGroups, userId));

  // Request user info if this is a new user
  if (wasAdded)
    Licq::gProtocolManager.requestUserInfo(userId);
}

void Handler::onUserRemoved(const string& id)
{
  TRACE();

  gUserManager.removeUser(UserId(id, JABBER_PPID), false);
}

void Handler::onUserStatusChange(const string& id, unsigned status)
{
  TRACE();

  Licq::UserWriteGuard user(Licq::UserId(id, JABBER_PPID));
  if (user.isLocked())
  {
    user->SetSendServer(true);
    user->statusChanged(status);

    if (status == Licq::User::OnlineStatus)
      gOnEventManager.performOnEvent(OnEventData::OnEventOnline, *user);
  }
}

void Handler::onUserInfo(const string& id, const VCardToUser& wrapper)
{
  TRACE();

  bool updated = false;
  Licq::UserId userId(id, JABBER_PPID);
  if (gUserManager.isOwner(userId))
  {
    Licq::OwnerWriteGuard owner(userId);
    if (owner.isLocked())
      updated = wrapper.updateUser(*owner);
  }
  else
  {
    Licq::UserWriteGuard user(userId);
    if (user.isLocked())
      updated = wrapper.updateUser(*user);
  }

  if (updated)
  {
    Licq::gDaemon.pushPluginSignal(
        new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                               Licq::PluginSignal::UserBasic, userId));
  }
}

void Handler::onRosterReceived(const std::set<string>& ids)
{
  TRACE();

  std::list<UserId> todel;
  std::list<UserId>::const_iterator it;

  {
    Licq::UserListGuard userList(JABBER_PPID);
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      if (ids.count(user->accountId()) == 0)
        todel.push_back(user->id());
    }
  }

  for (it = todel.begin(); it != todel.end(); ++it)
    gUserManager.removeUser(*it, false);
}

void Handler::onUserAuthorizationRequest(
    const string& id, const string& message)
{
  TRACE();

  Licq::EventAuthRequest* event = new Licq::EventAuthRequest(
      UserId(id, JABBER_PPID),
      string(), // alias
      string(), string(), // first and last name
      string(), // email
      message,
      ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);

  Licq::OwnerWriteGuard owner(JABBER_PPID);
  if (Licq::gDaemon.addUserEvent(*owner, event))
  {
    event->AddToHistory(*owner, true);
    gOnEventManager.performOnEvent(OnEventData::OnEventSysMsg, *owner);
  }
}

void Handler::onMessage(const string& from, const string& message, time_t sent,
                        bool urgent)
{
  TRACE();

  Licq::EventMsg* event = new Licq::EventMsg(
      message.c_str(), ICQ_CMDxRCV_SYSxMSGxOFFLINE, sent,
      urgent ? unsigned(Licq::UserEvent::FlagUrgent) : 0,
      getConvoId(from));

  Licq::UserWriteGuard user(UserId(from, JABBER_PPID), true);

  if (user.isLocked())
    user->setIsTyping(false);
  if (Licq::gDaemon.addUserEvent(*user, event))
    gOnEventManager.performOnEvent(OnEventData::OnEventMessage, *user);
}

void Handler::onNotifyTyping(const string& from, bool active)
{
  TRACE();

  Licq::UserWriteGuard user(UserId(from, JABBER_PPID));
  if (user.isLocked())
  {
    user->setIsTyping(active);

    Licq::gDaemon.pushPluginSignal(
        new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                               Licq::PluginSignal::UserTyping,
                               user->id(),
                               getConvoId(from)));
  }
}

string Handler::getStatusMessage(unsigned status)
{
  if ((status & Licq::User::MessageStatuses) == 0)
    return string();

  Licq::OwnerReadGuard o(JABBER_PPID);
  if (!o.isLocked())
    return string();

  return o->autoResponse();
}

unsigned long Handler::getConvoId(const string& from)
{
  std::map<string, unsigned long>::iterator it = myConvoIds.find(from);
  if (it == myConvoIds.end())
    it = myConvoIds.insert(std::make_pair(from, myNextConvoId++)).first;
  return it->second;
}
