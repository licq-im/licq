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

#include "handler.h"
#include "owner.h"
#include "user.h"
#include "vcard.h"

#include <boost/foreach.hpp>

#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/logging/log.h>
#include <licq/oneventmanager.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/socket.h>
#include <licq/userevents.h>

#define TRACE_FORMAT "Handler::%s: "
#define TRACE_ARGS __func__
#include "debug.h"

using namespace LicqJabber;

using Licq::OnEventData;
using Licq::UserId;
using Licq::gOnEventManager;
using Licq::gUserManager;
using std::string;

Handler::Handler(const Licq::UserId& ownerId)
  : myOwnerId(ownerId)
{
  // Empty
}

void Handler::onConnect(const string& ip, int port, unsigned status)
{
  TRACE();

  {
    OwnerWriteGuard owner(myOwnerId);
    if (owner.isLocked())
    {
      owner->statusChanged(status);
      owner->SetIpPort(Licq::INetSocket::ipToInt(ip), port);
      owner->setTimezone(User::systemTimezone());
    }
  }

  Licq::gPluginManager.pushPluginSignal(
      new Licq::PluginSignal(Licq::PluginSignal::SignalLogon,
      0, myOwnerId));
}

void Handler::onChangeStatus(unsigned status)
{
  TRACE();

  OwnerWriteGuard owner(myOwnerId);
  if (owner.isLocked())
    owner->statusChanged(status);
}

void Handler::onDisconnect(bool authError)
{
  TRACE();

  {
    Licq::UserListGuard userList(myOwnerId);
    BOOST_FOREACH(Licq::User* licqUser, **userList)
    {
      Licq::UserWriteGuard user(licqUser);
      if (user->isOnline())
        user->statusChanged(User::OfflineStatus);
    }
  }

  {
    OwnerWriteGuard owner(myOwnerId);
    if (owner.isLocked())
      owner->statusChanged(User::OfflineStatus);
  }

  Licq::gPluginManager.pushPluginSignal(
      new Licq::PluginSignal(Licq::PluginSignal::SignalLogoff,
                             authError ?
                             Licq::PluginSignal::LogoffPassword :
                             Licq::PluginSignal::LogoffRequested,
                             myOwnerId));
}

void Handler::onUserAdded(
    const string& id, const string& name, const std::list<string>& groups,
    bool awaitingAuthorization)
{
  TRACE("%s (%s)", id.c_str(), name.c_str());

  UserId userId(myOwnerId, id);
  bool wasAdded = false;
  if (!gUserManager.userExists(userId))
  {
    gUserManager.addUser(userId, true, false);
    wasAdded = true;
  }
  UserWriteGuard user(userId);
  assert(user.isLocked());

  user->SetEnableSave(false);

  if (wasAdded || !user->KeepAliasOnUpdate())
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
  user->SetAwaitingAuth(awaitingAuthorization);
  user->SetSendServer(true);

  user->SetEnableSave(true);
  user->save(User::SaveUserInfo | User::SaveLicqInfo);

  Licq::gPluginManager.pushPluginSignal(
      new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                             Licq::PluginSignal::UserBasic, userId));
  Licq::gPluginManager.pushPluginSignal(
      new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                             Licq::PluginSignal::UserGroups, userId));

  // Request user info if this is a new user
  if (wasAdded)
    Licq::gProtocolManager.requestUserInfo(userId);
}

void Handler::onUserRemoved(const string& id)
{
  TRACE();

  Licq::gUserManager.removeLocalUser(UserId(myOwnerId, id));
}

void Handler::onUserStatusChange(
    const string& id, unsigned status, const string& msg,
    const string& photoHash)
{
  TRACE();

  bool refreshInfo = false;

  Licq::UserId userId(myOwnerId, id);
  UserWriteGuard user(userId);
  if (user.isLocked())
  {
    user->SetSendServer(true);
    user->setAutoResponse(msg);
    user->statusChanged(status);

    if (!photoHash.empty() && photoHash != user->pictureSha1())
    {
      Licq::gLog.debug("New picture SHA1 for %s; requesting new VCard",
                       userId.accountId().c_str());
      refreshInfo = true;
    }
  }

  if (refreshInfo)
    Licq::gProtocolManager.requestUserInfo(userId);
}

void Handler::onUserInfo(const string& id, const VCardToUser& wrapper)
{
  TRACE();

  bool aliasUpdated = false;
  int saveGroup = 0;
  Licq::UserId userId(myOwnerId, id);
  if (userId.isOwner())
  {
    OwnerWriteGuard owner(userId);
    if (owner.isLocked())
    {
      const string oldAlias = owner->getAlias();
      saveGroup = wrapper.updateUser(*owner);
      aliasUpdated = owner->getAlias() != oldAlias;
    }
  }
  else
  {
    UserWriteGuard user(userId);
    if (user.isLocked())
    {
      const string oldAlias = user->getAlias();
      saveGroup = wrapper.updateUser(*user);
      aliasUpdated = user->getAlias() != oldAlias;
    }
  }

  if (saveGroup != 0)
  {
    if (saveGroup & User::SaveUserInfo)
      Licq::gPluginManager.pushPluginSignal(
          new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                                 Licq::PluginSignal::UserBasic, userId));

    if (saveGroup & User::SavePictureInfo)
      Licq::gPluginManager.pushPluginSignal(
          new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                                 Licq::PluginSignal::UserPicture, userId));
  }

  if (aliasUpdated)
    Licq::gProtocolManager.updateUserAlias(userId);
}

void Handler::onRosterReceived(const std::set<string>& ids)
{
  TRACE();

  std::list<UserId> todel;
  std::list<UserId>::const_iterator it;

  {
    Licq::UserListGuard userList(myOwnerId);
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      if (ids.count(user->accountId()) == 0)
        todel.push_back(user->id());
    }
  }

  for (it = todel.begin(); it != todel.end(); ++it)
    Licq::gUserManager.removeLocalUser(*it);
}

void Handler::onUserAuthorizationRequest(
    const string& id, const string& message)
{
  TRACE();

  Licq::EventAuthRequest* event = new Licq::EventAuthRequest(
      UserId(myOwnerId, id),
      string(), // alias
      string(), string(), // first and last name
      string(), // email
      message,
      time(0), 0);

  OwnerWriteGuard owner(myOwnerId);
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
      message.c_str(), sent,
      urgent ? unsigned(Licq::UserEvent::FlagUrgent) : 0);

  Licq::UserWriteGuard user(UserId(myOwnerId, from), true);

  if (user.isLocked())
    user->setIsTyping(false);

  if (Licq::gDaemon.addUserEvent(*user, event))
    gOnEventManager.performOnEvent(OnEventData::OnEventMessage, *user);
}

void Handler::onNotifyTyping(const string& from, bool active)
{
  TRACE();

  UserWriteGuard user(UserId(myOwnerId, from));
  if (user.isLocked())
  {
    user->setIsTyping(active);

    Licq::gPluginManager.pushPluginSignal(
        new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                               Licq::PluginSignal::UserTyping,
                               user->id()));
  }
}

string Handler::getStatusMessage(unsigned status)
{
  if ((status & User::MessageStatuses) == 0)
    return string();

  OwnerReadGuard o(myOwnerId);
  if (!o.isLocked())
    return string();

  return o->autoResponse();
}
