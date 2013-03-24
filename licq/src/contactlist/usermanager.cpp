/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "usermanager.h"

#include <boost/foreach.hpp>
#include <cstdio> // sprintf

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/inifile.h>
#include <licq/logging/log.h>
#include <licq/pluginsignal.h>
#include <licq/protocolsignal.h>

#include "../daemon.h"
#include "../gettext.h"
#include "../plugin/pluginmanager.h"
#include "../protocolmanager.h"
#include "group.h"
#include "user.h"

using std::list;
using std::string;
using Licq::GroupListGuard;
using Licq::GroupReadGuard;
using Licq::GroupWriteGuard;
using Licq::Owner;
using Licq::OwnerListGuard;
using Licq::OwnerReadGuard;
using Licq::OwnerWriteGuard;
using Licq::PluginSignal;
using Licq::User;
using Licq::UserListGuard;
using Licq::UserId;
using Licq::UserGroupList;
using Licq::UserReadGuard;
using Licq::UserWriteGuard;
using Licq::gLog;
using namespace LicqDaemon;

// Declare global UserManager (internal for daemon)
LicqDaemon::UserManager LicqDaemon::gUserManager;

// Initialize global Licq::UserManager to refer to the internal UserManager
Licq::UserManager& Licq::gUserManager(LicqDaemon::gUserManager);


UserManager::UserManager()
{
  // Set up the basic all users and new users group
  myGroupListMutex.setName("grouplist");
  myUserListMutex.setName("userlist");
  myOwnerListMutex.setName("ownerlist");
}


UserManager::~UserManager()
{
  // Empty
}

void UserManager::shutdown()
{
  UserMap::iterator iter;
  for (iter = myUsers.begin(); iter != myUsers.end(); ++iter)
    delete iter->second;
  myUsers.clear();

  GroupMap::iterator g_iter;
  for (g_iter = myGroups.begin(); g_iter != myGroups.end(); ++g_iter)
    delete g_iter->second;
  myGroups.clear();

  OwnerMap::iterator o_iter;
  for (o_iter = myOwners.begin(); o_iter != myOwners.end(); ++o_iter)
    delete o_iter->second;
  myOwners.clear();
}

void UserManager::addOwner(const UserId& userId)
{
  if (!userId.isOwner())
    return;

  myOwnerListMutex.lockWrite();

  // Make sure owner isn't already in configuration
  if (myConfiguredOwners.count(userId) > 0)
  {
    myOwnerListMutex.unlockWrite();
    return;
  }

  for (OwnerMap::const_iterator i = myOwners.begin(); i != myOwners.end(); ++i)
  {
    if (i->first.protocolId() == userId.protocolId() &&
        (i->second->protocolCapabilities() & Licq::ProtocolPlugin::CanMultipleOwners) == 0)
    {
      // We already have another owner for this protocol, and multiple owners isn't supported
      myOwnerListMutex.unlockWrite();
      return;
    }
  }

  Owner* o = createOwner(userId);

  myConfiguredOwners.insert(userId);
  myOwners[userId] = o;

  myOwnerListMutex.unlockWrite();

  saveOwnerList();

  // Create section for this owner in users.conf
  myUserListMutex.lockRead();
  saveUserList(userId);
  myUserListMutex.unlockRead();

  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListOwnerAdded, userId));
}

/*---------------------------------------------------------------------------
 * UserManager::Load
 *-------------------------------------------------------------------------*/
bool UserManager::Load()
{
  gLog.info(tr("Loading user configuration"));

  // Load the group info from licq.conf
  Licq::IniFile& licqConf(gDaemon.getLicqConf());

  unsigned nOwners;
  licqConf.setSection("owners");
  licqConf.get("NumOfOwners", nOwners, 0);

  // Make a map of owners to easier migrate server ids for groups
  std::map<unsigned long, std::string> oldOwners;

  myOwnerListMutex.lockWrite();
  for (unsigned short i = 1; i <= nOwners; i++)
  {
    char sOwnerIDKey[20], sOwnerPPIDKey[20];
    string accountId, ppidStr;
    sprintf(sOwnerIDKey, "Owner%d.Id", i);
    licqConf.get(sOwnerIDKey, accountId);
    sprintf(sOwnerPPIDKey, "Owner%d.Protocol", i);
    licqConf.get(sOwnerPPIDKey, ppidStr);
    unsigned long protocolId = Licq::protocolId_fromString(ppidStr);
    myConfiguredOwners.insert(UserId(protocolId, accountId));
    oldOwners[protocolId] = accountId;
  }
  myOwnerListMutex.unlockWrite();

  unsigned int nGroups;
  licqConf.setSection("groups");
  licqConf.get("NumOfGroups", nGroups);

  myGroupListMutex.lockWrite();
  m_bAllowSave = false;
  for (unsigned int i = 1; i <= nGroups; i++)
  {
    char key[40];
    string groupName;
    int groupId, sortIndex;
    sprintf(key, "Group%d.name", i);
    licqConf.get(key, groupName);
    sprintf(key, "Group%d.Sorting", i);
    licqConf.get(key, sortIndex, i-1);
    sprintf(key, "Group%d.id", i);
    licqConf.get(key, groupId, 0);

    Group* newGroup = new Group(groupId, groupName);
    newGroup->setSortIndex(sortIndex);

    list<string> serverIdKeys;
    sprintf(key, "Group%d.ServerId.", i);
    licqConf.getKeyList(serverIdKeys, key);
    BOOST_FOREACH(const string& serverIdKey, serverIdKeys)
    {
      unsigned long serverId;
      licqConf.get(serverIdKey, serverId, 0);

      string id = serverIdKey.substr(strlen(key));
      unsigned long protocolId = Licq::protocolId_fromString(id.substr(0, 4));
      string accountId;
      if (id.size() <= 4)
      {
        // Old (pre 1.8.0) config with only protocol id
        // Get first (and hopefully only) owner
        std::map<unsigned long, std::string>::const_iterator iter = oldOwners.find(protocolId);
        if (iter != oldOwners.end())
        {
          accountId = iter->second;

          // Update configuration with full owner id in key
          licqConf.unset(serverIdKey);
          licqConf.set(serverIdKey + accountId, serverId);
        }
      }
      else
      {
        accountId = id.substr(4);
      }

      if (!accountId.empty())
        newGroup->setServerId(Licq::UserId(protocolId, accountId), serverId);
    }

    myGroups[groupId] = newGroup;
  }
  m_bAllowSave = true;
  myGroupListMutex.unlockWrite();

  licqConf.setSection("network");
  licqConf.get("DefaultUserEncoding", myDefaultEncoding, "");

  licqConf.writeFile();
  gDaemon.releaseLicqConf();

  return true;
}

void UserManager::loadUserList(const UserId& ownerId)
{
  // Load users from users.conf
  Licq::IniFile usersConf("users.conf");
  usersConf.loadFile();

  string ppidStr = Licq::protocolId_toString(ownerId.protocolId());

  myUserListMutex.lockWrite();

  usersConf.setSection(ownerId.accountId() + "." + ppidStr);
  int numUsers;
  usersConf.get("NumUsers", numUsers);
  gLog.info(tr("Loading %i users for %s"), numUsers, ownerId.toString().c_str());

  for (int i = 1; i <= numUsers; ++i)
  {
    char key[20];
    sprintf(key, "User%i", i);
    string accountId;
    usersConf.get(key, accountId);
    if (accountId.empty() || accountId == ownerId.accountId())
    {
      gLog.warning(tr("Skipping user %i, invalid key"), i);
      continue;
    }
    UserId userId(ownerId, accountId);
    User* u = createUser(userId);
    u->myPrivate->addToContactList();
    myUsers[userId] = u;
  }
  myUserListMutex.unlockWrite();
}

void UserManager::loadProtocol(unsigned long protocolId)
{
  myOwnerListMutex.lockWrite();
  BOOST_FOREACH(const UserId& ownerId, myConfiguredOwners)
  {
    if (ownerId.protocolId() != protocolId)
      continue;

    // Load owner
    Owner* o = createOwner(ownerId);
    myOwners[ownerId] = o;

    // Notify plugins that the owner has been added
    gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
        PluginSignal::ListOwnerAdded, ownerId));

    // Load all users for owner
    loadUserList(ownerId);

    if ((o->protocolCapabilities() & Licq::ProtocolPlugin::CanMultipleOwners) == 0)
      // This protocol only supports one owner per protocol, don't load any more
      break;
  }
  myOwnerListMutex.unlockWrite();

  // Notify plugins that the users have been added
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListInvalidate));
}

void UserManager::unloadProtocol(unsigned long protocolId)
{
  // Delete all user objects using this protocol
  myUserListMutex.lockWrite();
  for (UserMap::iterator i = myUsers.begin(); i != myUsers.end(); )
  {
    if (i->first.protocolId() != protocolId)
    {
      ++i;
      continue;
    }

    User* u = i->second;
    u->lockWrite();
    myUsers.erase(i++);
    u->unlockWrite();
    delete u;
  }
  myUserListMutex.unlockWrite();

  // Delete owner object for this protocol
  myOwnerListMutex.lockWrite();
  for (OwnerMap::iterator i = myOwners.begin(); i != myOwners.end(); )
  {
    if (i->first.protocolId() != protocolId)
    {
      ++i;
      continue;
    }

    Owner* o = i->second;
    UserId ownerId = o->id();
    o->lockWrite();
    myOwners.erase(i++);
    o->unlockWrite();
    delete o;

    // Notify plugins that an owner has been removed
    gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
        PluginSignal::ListOwnerRemoved, ownerId));
  }
  myOwnerListMutex.unlockWrite();

  // Notify plugins that the users have been removed
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListInvalidate));
}

void UserManager::saveOwnerList()
{
  myOwnerListMutex.lockRead();

  Licq::IniFile& licqConf(gDaemon.getLicqConf());

  licqConf.setSection("owners");
  licqConf.set("NumOfOwners", (unsigned long)myOwners.size());

  int count = 0;
  {
    BOOST_FOREACH(const UserId& ownerId, myConfiguredOwners)
    {
      ++count;

      char key[64];
      snprintf(key, sizeof(key), "Owner%d.Id", count);
      licqConf.set(key, ownerId.accountId());

      snprintf(key, sizeof(key), "Owner%d.Protocol", count);
      licqConf.set(key, Licq::protocolId_toString(ownerId.protocolId()));
    }
  }

  // Don't leave old higher numbered owners in config
  char key[14];
  do
  {
    ++count;
    sprintf(key, "Owner%d.", count);
  }
  while (licqConf.unset(string(key) + "Id") | licqConf.unset(string(key) + "Protocol"));

  licqConf.writeFile();
  gDaemon.releaseLicqConf();

  myOwnerListMutex.unlockRead();
}

void UserManager::saveUserList(const UserId& ownerId)
{
  Licq::IniFile usersConf("users.conf");
  usersConf.loadFile();
  string ppidStr = Licq::protocolId_toString(ownerId.protocolId());
  usersConf.setSection(ownerId.accountId() + "." + ppidStr);

  int count = 0;
  for (UserMap::const_iterator i = myUsers.begin(); i != myUsers.end(); ++i)
  {
    if (i->first.ownerId() != ownerId)
      continue;

    i->second->lockRead();
    bool temporary = i->second->NotInList();
    i->second->unlockRead();

    // Only save users for this owner that's been permanently added
    if (temporary)
      continue;
    ++count;

    char key[20];
    sprintf(key, "User%i", count);
    usersConf.set(key, i->first.accountId());
  }
  usersConf.set("NumUsers", count);

  // Don't leave old higher numbered users in config
  char key[20];
  do
  {
    ++count;
    sprintf(key, "User%i", count);
  }
  while (usersConf.unset(key));

  usersConf.writeFile();
}

bool UserManager::allowUnloadProtocol(unsigned long protocolId)
{
  myOwnerListMutex.lockRead();
  for (OwnerMap::const_iterator i = myOwners.begin(); i != myOwners.end(); ++i)
  {
    if (i->first.protocolId() != protocolId)
      continue;

    // If owner is online, don't allow unloading the protocol
    i->second->lockRead();
    bool isOnline = i->second->isOnline();
    i->second->unlockRead();
    if (isOnline)
    {
      myOwnerListMutex.unlockRead();
      gLog.warning(tr("Protocol must be offline to be unloaded"));
      return false;
    }
  }
  myOwnerListMutex.unlockRead();
  return true;
}

bool UserManager::addUser(const UserId& uid,
    bool permanent, bool addToServer, unsigned short groupId)
{
  if (!uid.isValid())
    return false;

  if (uid.isOwner())
    return false;

  myUserListMutex.lockWrite();

  User* user;
  bool created;
  UserMap::const_iterator iter = myUsers.find(uid);
  if (iter != myUsers.end())
  {
    user = iter->second;
    created = false;

    // If user already is in list only continue if it should be made permanent
    if (!user->NotInList() || !permanent)
    {
      myUserListMutex.unlockWrite();
      return false;
    }
  }
  else
  {
    user = createUser(uid, !permanent);
    created = true;
  }

  user->lockWrite();

  if (permanent)
  {
    // Set this user to be on the contact list
    if (created)
    {
      user->myPrivate->addToContactList();
      user->save(User::SaveAll);
    }
    else
    {
      user->myPrivate->setPermanent();
    }
  }

  // Store the user in the lookup map
  if (created)
    myUsers[uid] = user;

  user->unlockWrite();

  if (permanent)
    saveUserList(uid.ownerId());

  myUserListMutex.unlockWrite();

  // Notify plugins that user was added
  // Send this before adding user to server side as protocol code may generate updated signals
  if (created)
    gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
        PluginSignal::ListUserAdded, uid, groupId));
  else if (permanent)
    gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
        PluginSignal::UserSettings, uid, 0));

  // Set initial group membership but let add signal below update the server
  if (groupId != 0)
    setUserInGroup(uid, groupId, true, false);

  // Add user to server side list
  if (permanent && addToServer)
    gProtocolManager.addUser(uid);

  return true;
}

void UserManager::removeUser(const UserId& userId)
{
  {
    // Only allow removing a user if the protocol is available
    OwnerReadGuard owner(userId.ownerId());
    if (!owner.isLocked() || !owner->isOnline())
      return;
  }

  // Remove the user from the server side list first in case protocol needs any
  // data from the user object. The protocol will call removeLocalUser() when
  // it is finished.
  gPluginManager.pushProtocolSignal(new Licq::ProtoRemoveUserSignal(userId));
}

void UserManager::removeLocalUser(const UserId& userId)
{
  // List should only be locked when not holding any user lock to avoid
  // deadlock, so we cannot call fetchUser here.
  myUserListMutex.lockWrite();
  UserMap::iterator iter = myUsers.find(userId);
  if (iter == myUsers.end())
  {
    myUserListMutex.unlockWrite();
    return;
  }

  User* u = iter->second;
  u->lockWrite();
  myUsers.erase(iter);
  if (!u->NotInList())
  {
    u->myPrivate->removeFiles();

    saveUserList(userId.ownerId());
  }
  myUserListMutex.unlockWrite();
  u->unlockWrite();
  delete u;

  // Notify plugins about the removed user
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListUserRemoved, userId));
}

void UserManager::writeToUserHistory(Licq::User* user, const string& text)
{
  user->myPrivate->writeToHistory(text);
}

bool UserManager::removeOwner(const Licq::UserId& userId)
{
  // List should only be locked when not holding any user lock to avoid
  // deadlock, so we cannot call FetchOwner here.
  myOwnerListMutex.lockWrite();
  OwnerMap::iterator iter = myOwners.find(userId);
  if (iter == myOwners.end())
  {
    myOwnerListMutex.unlockWrite();
    return false;
  }

  Owner* o = iter->second;
  o->lockWrite();

  // Don't allow removing an owner that is online
  if (o->status() != Owner::OfflineStatus)
  {
    myOwnerListMutex.unlockWrite();
    o->unlockWrite();
    return false;
  }

  myOwners.erase(iter);
  o->myPrivate->removeFiles();
  myConfiguredOwners.erase(userId);
  myOwnerListMutex.unlockWrite();
  o->unlockWrite();
  delete o;

  saveOwnerList();

  myGroupListMutex.lockWrite();
  for (GroupMap::iterator group = myGroups.begin(); group != myGroups.end(); ++group)
  {
    group->second->lockRead();
    group->second->unsetServerId(userId);
    group->second->unlockRead();
  }
  SaveGroups();
  myGroupListMutex.unlockWrite();

  // Remove user list section from users.conf
  Licq::IniFile usersConf("users.conf");
  usersConf.loadFile();
  string ppidStr = Licq::protocolId_toString(userId.protocolId());
  usersConf.removeSection(userId.accountId() + "." + ppidStr);
  usersConf.writeFile();

  gPluginManager.shutdownProtocolInstance(userId);

  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListOwnerRemoved, userId));

  return true;
}

Licq::User* UserManager::fetchUser(const UserId& userId,
    bool writeLock, bool addUser, bool* retWasAdded)
{
  if (retWasAdded != NULL)
    *retWasAdded = false;

  User* user = NULL;

  if (!userId.isValid())
    return NULL;

  if (userId.isOwner())
  {
    myOwnerListMutex.lockRead();
    OwnerMap::const_iterator iter_o = myOwners.find(userId);
    if (iter_o != myOwners.end())
    {
      user = iter_o->second;
      if (writeLock)
        user->lockWrite();
      else
        user->lockRead();
    }
    myOwnerListMutex.unlockRead();
    return user;
  }

  if (addUser)
    myUserListMutex.lockWrite();
  else
    myUserListMutex.lockRead();

  UserMap::const_iterator iter = myUsers.find(userId);
  if (iter != myUsers.end())
    user = iter->second;

    // If allowed by caller, add user if it wasn't found in list
    if (user == NULL && addUser)
    {
      // Create a temporary user
      user = createUser(userId, true);

      // Store the user in the lookup map
      myUsers[userId] = user;

      // Notify plugins that we added user to list
      gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
          PluginSignal::ListUserAdded, userId));

      if (retWasAdded != NULL)
        *retWasAdded = true;
    }

    // Lock the user and release the lock on the list
    if (user != NULL)
  {
    if (writeLock)
      user->lockWrite();
    else
      user->lockRead();
  }

  if (addUser)
    myUserListMutex.unlockWrite();
  else
    myUserListMutex.unlockRead();

  return user;
}

bool UserManager::userExists(const UserId& userId)
{
  bool exists = false;

  if (userId.isOwner())
  {
    myOwnerListMutex.lockRead();
    OwnerMap::const_iterator iter_o = myOwners.find(userId);
    if (iter_o != myOwners.end())
      exists = true;
    myOwnerListMutex.unlockRead();
  }
  else
  {
    myUserListMutex.lockRead();
    UserMap::const_iterator iter = myUsers.find(userId);
    if (iter != myUsers.end())
      exists = true;
    myUserListMutex.unlockRead();
  }
  return exists;
}

User* UserManager::createUser(const UserId& userId, bool temporary)
{
  assert(!userId.isOwner());

  User* u = gPluginManager.createProtocolUser(userId, temporary);

  if (u == NULL)
    // Protocol hasn't subclassed the user class, create from Licq::User instead
    u = new User(userId, temporary);

  return u;
}

Owner* UserManager::createOwner(const UserId& ownerId)
{
  assert(ownerId.isOwner());

  Owner* o = gPluginManager.createProtocolOwner(ownerId);

  if (o == NULL)
    // Protocol hasn't subclassed the owner class, create from Licq::Owner instead
    o = new Owner(ownerId);

  return o;
}

void UserManager::notifyUserUpdated(const UserId& userId, unsigned long subSignal)
{
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser, subSignal, userId));
}

Group* UserManager::fetchGroup(int group, bool writeLock)
{
  myGroupListMutex.lockRead();
  GroupMap::const_iterator iter = myGroups.find(group);
  Group* g = NULL;
  if (iter != myGroups.end())
  {
    g = iter->second;
    if (writeLock)
      g->lockWrite();
    else
      g->lockRead();
  }
  myGroupListMutex.unlockRead();
  return g;
}

bool UserManager::groupExists(int groupId)
{
  myGroupListMutex.lockRead();
  GroupMap::const_iterator iter = myGroups.find(groupId);
  bool found = (iter != myGroups.end());
  myGroupListMutex.unlockRead();
  return found;
}

int UserManager::AddGroup(const string& name)
{
  if (name.empty())
    return 0;

  if (GetGroupFromName(name) != 0)
  {
    // Don't allow a duplicate name
    gLog.warning(tr("Group %s is already in list"), name.c_str());
    return 0;
  }

  myGroupListMutex.lockWrite();

  // Find first free group id
  int gid;
  for (gid = 1; myGroups.count(gid) != 0 ; ++gid)
    ;

  Group* newGroup = new Group(gid, name);
  newGroup->setSortIndex(myGroups.size());
  myGroups[gid] = newGroup;

  SaveGroups();
  myGroupListMutex.unlockWrite();

  // Send signal to let plugins know of the new group
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupAdded, UserId(), gid));

  return gid;
}

void UserManager::RemoveGroup(int groupId)
{
  // List should only be locked when not holding any group lock to avoid
  // deadlock, so we cannot call fetchGroup here.
  myGroupListMutex.lockWrite();
  GroupMap::iterator iter = myGroups.find(groupId);
  if (iter == myGroups.end())
  {
    myGroupListMutex.unlockWrite();
    return;
  }
  Group* group = iter->second;

  group->lockWrite();
  int sortIndex = group->sortIndex();

  // Erase the group
  myGroups.erase(iter);

  // Decrease sorting index for higher groups so we don't leave a gap
  for (iter = myGroups.begin(); iter != myGroups.end(); ++iter)
  {
    iter->second->lockWrite();
    int si = iter->second->sortIndex();
    if (si > sortIndex)
      iter->second->setSortIndex(si - 1);
    iter->second->unlockWrite();
  }

  SaveGroups();
  myGroupListMutex.unlockWrite();

  // Remove group from users
  {
    Licq::UserListGuard userList;
    BOOST_FOREACH(Licq::User* user, **userList)
    {
      Licq::UserWriteGuard u(user);
      bool notify = false;
      bool changeServer = false;

      if (u->removeFromGroup(groupId))
      {
        if (u->serverGroup() == -1)
          changeServer = true;
        notify = true;
      }

/*
      if (u->serverGroup() == groupId)
      {
        int newServerGroup = TODO find another group
        u->setServerGroup(newServerGroup);
        changeServer = true;
        notify = true;
      }
*/

      if (changeServer)
        gPluginManager.pushProtocolSignal(new Licq::ProtoChangeUserGroupsSignal(u->id()));

      if (notify)
        notifyUserUpdated(u->id(), PluginSignal::UserGroups);
    }
  }

  // Remove group from protocols
  myOwnerListMutex.lockRead();
  for (OwnerMap::const_iterator i = myOwners.begin(); i != myOwners.end(); ++i)
  {
    i->second->lockRead();
    bool isOnline = i->second->isOnline();
    i->second->unlockRead();
    if (!isOnline)
      continue;

    // Group object is no long reachable so include protocol specific server id and name in signal
    gPluginManager.pushProtocolSignal(new Licq::ProtoRemoveGroupSignal(
        i->first, groupId, group->serverId(i->first), group->name()));
  }
  myOwnerListMutex.unlockRead();

  // Send signal to let plugins know of the removed group
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupRemoved, UserId(), groupId));

  // Send signal to let plugins know that sorting indexes may have changed
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupsReordered));

  // Delete the group
  group->unlockWrite();
  delete group;
}

void UserManager::ModifyGroupSorting(int groupId, int newIndex)
{
  Group* group = fetchGroup(groupId);
  if (group == NULL)
    return;

  if (newIndex < 0)
    newIndex = 0;
  if (static_cast<unsigned int>(newIndex) >= NumGroups())
    newIndex = NumGroups() - 1;

  int oldIndex = group->sortIndex();
  group->unlockRead();

  myGroupListMutex.lockRead();

  // Move all groups between new and old position one step
  for (GroupMap::iterator i = myGroups.begin(); i != myGroups.end(); ++i)
  {
    i->second->lockWrite();
    int si = i->second->sortIndex();
    if (newIndex < oldIndex && si >= newIndex && si < oldIndex)
      i->second->setSortIndex(si + 1);
    else if (newIndex > oldIndex && si > oldIndex && si <= newIndex)
      i->second->setSortIndex(si - 1);
    i->second->unlockWrite();
  }

  group->lockWrite();
  group->setSortIndex(newIndex);
  group->unlockWrite();

  SaveGroups();
  myGroupListMutex.unlockRead();

  // Send signal to let plugins know that sorting indexes have changed
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupsReordered));
}

bool UserManager::RenameGroup(int groupId, const string& name, const Licq::UserId& skipOwnerId)
{
  int foundGroupId = GetGroupFromName(name);

  if (foundGroupId == groupId)
    // Name isn't changed so nothing to do here
    return true;

  if (foundGroupId != 0)
  {
    // Don't allow a duplicate name
    gLog.warning(tr("Group name %s is already in list"), name.c_str());
    return false;
  }

  Group* group = fetchGroup(groupId, true);
  if (group == NULL)
  {
    gLog.warning(tr("Renaming request for invalid group %u"), groupId);
    return false;
  }

  group->setName(name);
  group->unlockWrite();

  myGroupListMutex.lockRead();
  SaveGroups();
  myGroupListMutex.unlockRead();

  myOwnerListMutex.lockRead();
  for (OwnerMap::const_iterator i = myOwners.begin(); i != myOwners.end(); ++i)
  {
    // Don't notify the protocol that called us
    if (i->first == skipOwnerId)
      continue;

    i->second->lockRead();
    bool isOnline = i->second->isOnline();
    i->second->unlockRead();
    if (!isOnline)
      continue;

    gPluginManager.pushProtocolSignal(new Licq::ProtoRenameGroupSignal(i->first, groupId));
  }
  myOwnerListMutex.unlockRead();

  // Send signal to let plugins know the group has changed
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupChanged, UserId(), groupId));

  return true;
}

void UserManager::SaveGroups()
{
  if (!m_bAllowSave) return;

  // Load the group info from licq.conf
  Licq::IniFile& licqConf(gDaemon.getLicqConf());
  licqConf.loadFile();

  licqConf.setSection("groups");
  GroupMap::size_type count = myGroups.size();
  licqConf.set("NumOfGroups", static_cast<unsigned int>(count));

  int i = 1;
  for (GroupMap::iterator group = myGroups.begin(); group != myGroups.end(); ++group)
  {
    group->second->lockRead();
    group->second->save(licqConf, i);
    group->second->unlockRead();
    ++i;
  }

  // Don't leave old higher numbered groups in config
  while (true)
  {
    char key[32];
    sprintf(key, "Group%d.", i);
    list<string> groupKeys;
    licqConf.getKeyList(groupKeys, key);
    if (groupKeys.empty())
      break;
    BOOST_FOREACH(const string& key, groupKeys)
      licqConf.unset(key);
    ++i;
  }

  licqConf.writeFile();
  gDaemon.releaseLicqConf();
}

int UserManager::getGroupFromServerId(const Licq::UserId& ownerId, unsigned long serverId)
{
  myGroupListMutex.lockRead();
  GroupMap::const_iterator iter;
  int groupId = 0;
  for (iter = myGroups.begin(); iter != myGroups.end(); ++iter)
  {
    iter->second->lockRead();
    if (iter->second->serverId(ownerId) == serverId)
      groupId = iter->first;
    iter->second->unlockRead();
  }
  myGroupListMutex.unlockRead();

  return groupId;
}

int UserManager::GetGroupFromName(const string& name)
{
  myGroupListMutex.lockRead();
  GroupMap::const_iterator iter;
  int id = 0;
  for (iter = myGroups.begin(); iter != myGroups.end(); ++iter)
  {
    iter->second->lockRead();
    if (iter->second->name() == name)
      id = iter->first;
    iter->second->unlockRead();
  }
  myGroupListMutex.unlockRead();

  return id;
}

std::string UserManager::GetGroupNameFromGroup(int groupId)
{
  myGroupListMutex.lockRead();
  GroupMap::const_iterator iter;
  std::string name;
  for (iter = myGroups.begin(); iter != myGroups.end(); ++iter)
  {
    iter->second->lockRead();
    if (iter->second->id() == groupId)
      name = iter->second->name();
    iter->second->unlockRead();
  }
  myGroupListMutex.unlockRead();

  return name;
}

void UserManager::setGroupServerId(int groupId, const Licq::UserId& ownerId,
    unsigned long serverId)
{
  Group* group = fetchGroup(groupId, true);
  if (group == NULL)
    return;

  group->setServerId(ownerId, serverId);
  group->unlockWrite();

  myGroupListMutex.lockRead();
  SaveGroups();
  myGroupListMutex.unlockRead();
}

Licq::Owner* UserManager::fetchOwner(const UserId& userId, bool writeLock)
{
  Owner* o = NULL;

  myOwnerListMutex.lockRead();
  OwnerMap::const_iterator iter = myOwners.find(userId);
  if (iter != myOwners.end())
  {
    o = iter->second;
    if (writeLock)
      o->lockWrite();
    else
      o->lockRead();
  }
  myOwnerListMutex.unlockRead();

  return o;
}

void UserManager::SaveAllUsers()
{
  Licq::UserListGuard userList;
  BOOST_FOREACH(Licq::User* user, **userList)
  {
    Licq::UserWriteGuard u(user);
    if (!u->NotInList())
      u->save(Licq::User::SaveAll);
  }
}

unsigned short UserManager::NumUsers()
{
  //myUserListMutex.lockRead();
  unsigned short n = myUsers.size();
  //myUserListMutex.unlockRead();
  return n;
}

unsigned short UserManager::NumOwners()
{
  unsigned short n = myOwners.size();
  return n;
}

unsigned int UserManager::NumGroups()
{
  //myGroupListMutex.lockRead();
  unsigned int n = myGroups.size();
  //myGroupListMutex.unlockRead();
  return n;
}

const UserMap& UserManager::lockUserList()
{
  myUserListMutex.lockRead();
  return myUsers;
}

void UserManager::unlockUserList()
{
  myUserListMutex.unlockRead();
}

const GroupMap& UserManager::lockGroupList()
{
  myGroupListMutex.lockRead();
  return myGroups;
}

void UserManager::unlockGroupList()
{
  myGroupListMutex.unlockRead();
}

const OwnerMap& UserManager::lockOwnerList()
{
  myOwnerListMutex.lockRead();
  return myOwners;
}

void UserManager::unlockOwnerList()
{
  myOwnerListMutex.unlockRead();
}

void UserManager::setUserInGroup(const UserId& userId, int groupId,
      bool inGroup, bool updateServer)
{
  // User group 0 is invalid
  if (groupId <= 0)
    return;

  int serverGroup;

  {
    Licq::UserWriteGuard u(userId);
    if (!u.isLocked())
      return;

    serverGroup = u->serverGroup();

    // Don't remove user from local group if member of the same server group
    if (!inGroup && serverGroup == groupId)
      return;

    // Update user object
    u->setInGroup(groupId, inGroup);

    // Server group is set by protocol after performing the actual change
  }

  // Notify server
  if (serverGroup == -1 || updateServer)
  {
    bool ownerIsOnline = false;
    {
      OwnerReadGuard owner(userId.ownerId());
      ownerIsOnline = (owner.isLocked() && owner->isOnline());
    }

    if (ownerIsOnline)
      gPluginManager.pushProtocolSignal(new Licq::ProtoChangeUserGroupsSignal(userId));
  }

  // Notify plugins
  notifyUserUpdated(userId, PluginSignal::UserGroups);
}

const string& UserManager::defaultUserEncoding()
{
  return myDefaultEncoding;
}

void UserManager::setDefaultUserEncoding(const string& defaultEncoding)
{
  myDefaultEncoding = defaultEncoding;
}


UserReadGuard::UserReadGuard(const UserId& userId, bool addUser, bool* retWasAdded)
  : ReadMutexGuard<User>(LicqDaemon::gUserManager.fetchUser(userId, false, addUser, retWasAdded), true)
{
  // Empty
}

UserWriteGuard::UserWriteGuard(const UserId& userId, bool addUser, bool* retWasAdded)
  : WriteMutexGuard<User>(LicqDaemon::gUserManager.fetchUser(userId, true, addUser, retWasAdded), true)
{
  // Empty
}

OwnerReadGuard::OwnerReadGuard(const UserId& userId)
  : ReadMutexGuard<Owner>(LicqDaemon::gUserManager.fetchOwner(userId, false), true)
{
  // Empty
}

OwnerWriteGuard::OwnerWriteGuard(const UserId& userId)
  : WriteMutexGuard<Owner>(LicqDaemon::gUserManager.fetchOwner(userId, true), true)
{
  // Empty
}

GroupReadGuard::GroupReadGuard(int groupId)
  : ReadMutexGuard<Group>(LicqDaemon::gUserManager.fetchGroup(groupId), true)
{
  // Empty
}

GroupWriteGuard::GroupWriteGuard(int groupId)
  : WriteMutexGuard<Group>(LicqDaemon::gUserManager.fetchGroup(groupId, true), true)
{
  // Empty
}


UserListGuard::UserListGuard(unsigned long protocolId)
{
  const UserMap& userMap = LicqDaemon::gUserManager.lockUserList();

  for (UserMap::const_iterator i = userMap.begin(); i != userMap.end(); ++i)
    if (protocolId == 0 || i->first.protocolId() == protocolId)
      myUserList.push_back(i->second);
}

UserListGuard::UserListGuard(const Licq::UserId& ownerId)
{
  const UserMap& userMap = LicqDaemon::gUserManager.lockUserList();

  for (UserMap::const_iterator i = userMap.begin(); i != userMap.end(); ++i)
    if (!ownerId.isValid() || i->first.ownerId() == ownerId)
      myUserList.push_back(i->second);
}

UserListGuard::~UserListGuard()
{
  LicqDaemon::gUserManager.unlockUserList();
}

OwnerListGuard::OwnerListGuard(unsigned long protocolId)
{
  const OwnerMap& ownerMap = LicqDaemon::gUserManager.lockOwnerList();

  for (OwnerMap::const_iterator i = ownerMap.begin(); i != ownerMap.end(); ++i)
    if (protocolId == 0 || i->first.protocolId() == protocolId)
      myOwnerList.push_back(i->second);
}

OwnerListGuard::~OwnerListGuard()
{
  LicqDaemon::gUserManager.unlockOwnerList();
}

GroupListGuard::GroupListGuard(bool sorted)
{
  const GroupMap& groupMap = LicqDaemon::gUserManager.lockGroupList();

  for (GroupMap::const_iterator i = groupMap.begin(); i != groupMap.end(); ++i)
    myGroupList.push_back(i->second);

  if (sorted)
    myGroupList.sort(compare_groups);
}

GroupListGuard::~GroupListGuard()
{
  LicqDaemon::gUserManager.unlockGroupList();
}
