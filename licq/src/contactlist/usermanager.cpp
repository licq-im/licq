/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2012 Licq developers <licq-dev@googlegroups.com>
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

#include <licq/logging/log.h>
#include <licq/daemon.h>
#include <licq/inifile.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/protocolsignal.h>

#include "../gettext.h"
#include "../icq/icq.h"
#include "../protocolmanager.h"
#include "group.h"
#include "owner.h"
#include "user.h"

using std::list;
using std::string;
using Licq::GroupListGuard;
using Licq::GroupReadGuard;
using Licq::GroupWriteGuard;
using Licq::OwnerListGuard;
using Licq::OwnerReadGuard;
using Licq::OwnerWriteGuard;
using Licq::PluginSignal;
using Licq::UserListGuard;
using Licq::UserId;
using Licq::UserGroupList;
using Licq::UserReadGuard;
using Licq::UserWriteGuard;
using Licq::gDaemon;
using Licq::gLog;
using Licq::gPluginManager;
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
  Owner* o = new Owner(userId);

  myOwnerListMutex.lockWrite();
  myOwners[userId.protocolId()] = o;
  myOwnerListMutex.unlockWrite();

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
  Licq::IniFile licqConf("licq.conf");
  licqConf.loadFile();

  unsigned nOwners;
  licqConf.setSection("owners");
  licqConf.get("NumOfOwners", nOwners, 0);

  m_bAllowSave = false;

  //TODO Check for loaded plugins before the owner, so we can see
  //which owner(s) to load
  myOwnerListMutex.lockWrite();
  for (unsigned short i = 1; i <= nOwners; i++)
  {
    char sOwnerIDKey[20], sOwnerPPIDKey[20];
    string accountId, ppidStr;
    sprintf(sOwnerIDKey, "Owner%d.Id", i);
    licqConf.get(sOwnerIDKey, accountId);
    sprintf(sOwnerPPIDKey, "Owner%d.PPID", i);
    licqConf.get(sOwnerPPIDKey, ppidStr);
    unsigned long protocolId = (ppidStr[0] << 24) | (ppidStr[1] << 16) | (ppidStr[2] << 8) | (ppidStr[3]);

    UserId ownerId(accountId, protocolId);
    Owner* o = new Owner(ownerId);

    myOwners[protocolId] = o;
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
    unsigned icqGroupId = 0;
    sprintf(key, "Group%d.name", i);
    licqConf.get(key, groupName);

    sprintf(key, "Group%d.id", i);
    licqConf.get(key, groupId, 0);

    sprintf(key, "Group%d.Sorting", i);
    bool newConfig = licqConf.get(key, sortIndex, i-1);

    sprintf(key, "Group%d.IcqServerId", i);
    licqConf.get(key, icqGroupId, 0);

    // Sorting and IcqServerId did not exist in older versions.
    // If they are missing, assume that we are reading an old configuration
    // where id parameter is ICQ server side group id.
    if (!newConfig)
    {
      icqGroupId = groupId;
      groupId = i;
    }

    Group* newGroup = new Group(groupId, groupName);
    newGroup->setSortIndex(sortIndex);

    list<string> serverIdKeys;
    sprintf(key, "Group%d.ServerId.", i);
    licqConf.getKeyList(serverIdKeys, key);
    BOOST_FOREACH(const string& serverIdKey, serverIdKeys)
    {
      size_t keylen = serverIdKey.size();
      unsigned long protocolId = serverIdKey[keylen-4] << 24 |
          serverIdKey[keylen-3] << 16 | serverIdKey[keylen-2] << 8 |
          serverIdKey[keylen-1];
      unsigned long serverId;
      licqConf.get(serverIdKey, serverId, 0);
      newGroup->setServerId(protocolId, serverId);
    }

    // ServerId per protocol didn't exist in 1.3.x and older.
    // This will preserve ICQ group ids when reading old config.
    if (serverIdKeys.empty() && icqGroupId != 0)
      newGroup->setServerId(LICQ_PPID, icqGroupId);

    myGroups[groupId] = newGroup;
  }
  m_bAllowSave = true;
  myGroupListMutex.unlockWrite();

  licqConf.setSection("network");
  licqConf.get("DefaultUserEncoding", myDefaultEncoding, "");

  // Load users from users.conf
  Licq::IniFile usersConf("users.conf");
  usersConf.loadFile();

  unsigned nUsers;
  usersConf.setSection("users");
  usersConf.get("NumOfUsers", nUsers);
  gLog.info(tr("Loading %d users"), nUsers);

  // TODO: We need to only load users of protocol plugins that are loaded!
  myUserListMutex.lockWrite();
  for (unsigned short i = 1; i<= nUsers; i++)
  {
    string userFile;
    char sUserKey[20];
    sprintf(sUserKey, "User%d", i);
    if (!usersConf.get(sUserKey, userFile, ""))
    {
      gLog.warning(tr("Skipping user %i, empty key"), i);
      continue;
    }
    size_t sz = userFile.rfind('.');
    if (sz == string::npos)
    {
      gLog.error(tr("Fatal error reading protocol information for User%d with ID '%s'\n"
          "Please check \"%s/users.conf\""), i,
          userFile.c_str(), gDaemon.baseDir().c_str());
      exit(1);
    }
    string accountId = userFile.substr(0, sz);
    unsigned long protocolId = (userFile[sz+1] << 24) | (userFile[sz+2] << 16) | (userFile[sz+3] << 8) | userFile[sz+4];

    UserId userId(accountId, protocolId);
    User* u = new User(userId);
    u->AddToContactList();
    myUsers[userId] = u;
  }
  myUserListMutex.unlockWrite();

  return true;
}

void UserManager::saveUserList() const
{
  Licq::IniFile usersConf("users.conf");
  usersConf.loadFile();
  usersConf.setSection("users");

  int count = 0;
  for (UserMap::const_iterator i = myUsers.begin(); i != myUsers.end(); ++i)
  {
    i->second->lockRead();
    bool temporary = i->second->NotInList();
    string accountId = i->second->accountId();
    unsigned long ppid = i->second->protocolId();
    i->second->unlockRead();

    // Only save users that's been permanently added
    if (temporary)
      continue;
    ++count;

    char key[20];
    sprintf(key, "User%i", count);

    usersConf.set(key, accountId + "." + Licq::protocolId_toString(ppid));
  }
  usersConf.set("NumOfUsers", count);
  usersConf.writeFile();
}

bool UserManager::addUser(const UserId& uid,
    bool permanent, bool addToServer, unsigned short groupId)
{
  if (!uid.isValid())
    return false;

  if (isOwner(uid))
    return false;

  myUserListMutex.lockWrite();

  // Make sure user isn't already in the list
  UserMap::const_iterator iter = myUsers.find(uid);
  if (iter != myUsers.end())
  {
    myUserListMutex.unlockWrite();
    return false;
  }

  User* pUser = new User(uid, !permanent);
  pUser->lockWrite();

  if (permanent)
  {
    // Set this user to be on the contact list
    pUser->AddToContactList();
    //pUser->SetEnableSave(true);
    pUser->save(User::SaveAll);
  }

  // Store the user in the lookup map
  myUsers[uid] = pUser;

  pUser->unlockWrite();

  if (permanent)
    saveUserList();

  myUserListMutex.unlockWrite();

  // Notify plugins that user was added
  // Send this before adding user to server side as protocol code may generate updated signals
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListUserAdded, uid, groupId));

  // Set initial group membership but let add signal below update the server
  if (groupId != 0)
    setUserInGroup(uid, groupId, true, false);

  // Add user to server side list
  if (permanent && addToServer)
    gProtocolManager.addUser(uid, groupId);

  return true;
}

bool UserManager::makeUserPermanent(const UserId& userId, bool addToServer,
    int groupId)
{
  if (!userId.isValid())
    return false;

  if (isOwner(userId))
    return false;

  {
    Licq::UserWriteGuard user(userId);
    if (!user.isLocked())
      return false;

    // Check if user is already permanent
    if (!user->NotInList())
      return false;

    dynamic_cast<User*>(*user)->SetPermanent();
  }

  // Save local user list to disk
  saveUserList();

  // Add user to server side list
  if (addToServer)
    gProtocolManager.addUser(userId, groupId);

  // Set initial group membership, also sets server group for user
  if (groupId != 0)
    setUserInGroup(userId, groupId, true, addToServer);

  return true;
}

void UserManager::removeUser(const UserId& userId, bool removeFromServer)
{
  // Remove the user from the server side list first
  if (removeFromServer)
    gProtocolManager.removeUser(userId);

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
    u->RemoveFiles();
    saveUserList();
  }
  myUserListMutex.unlockWrite();
  u->unlockWrite();
  delete u;

  // Notify plugins about the removed user
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListUserRemoved, userId));
}

// Need to call CICQDaemon::SaveConf() after this
void UserManager::RemoveOwner(unsigned long ppid)
{
  // List should only be locked when not holding any user lock to avoid
  // deadlock, so we cannot call FetchOwner here.
  myOwnerListMutex.lockWrite();
  OwnerMap::iterator iter = myOwners.find(ppid);
  if (iter == myOwners.end())
  {
    myOwnerListMutex.unlockWrite();
    return;
  }

  Owner* o = iter->second;
  o->lockWrite();

  // Don't allow removing an owner that is online
  if (o->status() != Owner::OfflineStatus)
  {
    myOwnerListMutex.unlockWrite();
    o->unlockWrite();
    return;
  }

  myOwners.erase(iter);
  o->RemoveFiles();
  UserId id = o->id();
  myOwnerListMutex.unlockWrite();
  o->unlockWrite();
  delete o;

  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListOwnerRemoved, id));
}

Licq::User* UserManager::fetchUser(const UserId& userId,
    bool writeLock, bool addUser, bool* retWasAdded)
{
  if (retWasAdded != NULL)
    *retWasAdded = false;

  User* user = NULL;

  if (!userId.isValid())
    return NULL;

  // Check for an owner first
  myOwnerListMutex.lockRead();
  OwnerMap::iterator iter_o = myOwners.find(userId.protocolId());
  if (iter_o != myOwners.end())
  {
    if (iter_o->second->id() == userId)
    {
      user = iter_o->second;
      if (writeLock)
        user->lockWrite();
      else
        user->lockRead();
    }
  }
  myOwnerListMutex.unlockRead();

  if (user != NULL)
    return user;

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
      user = new User(userId, true);

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

  myOwnerListMutex.lockRead();
  OwnerMap::iterator iter_o = myOwners.find(userId.protocolId());
  if (iter_o != myOwners.end())
  {
    if (iter_o->second->id() == userId)
      exists = true;
  }
  myOwnerListMutex.unlockRead();
  if (exists)
    return true;

  myUserListMutex.lockRead();
  UserMap::const_iterator iter = myUsers.find(userId);
  if (iter != myUsers.end())
    exists = true;
  myUserListMutex.unlockRead();
  return exists;
}

UserId UserManager::ownerUserId(unsigned long ppid)
{
  Licq::OwnerReadGuard owner(ppid);
  if (!owner.isLocked())
    return UserId();

  return owner->id();
}

bool UserManager::isOwner(const UserId& userId)
{
  bool exists = false;

  myOwnerListMutex.lockRead();
  OwnerMap::iterator iter_o = myOwners.find(userId.protocolId());
  if (iter_o != myOwners.end())
  {
    if (iter_o->second->id() == userId)
      exists = true;
  }
  myOwnerListMutex.unlockRead();

  return exists;
}

unsigned long UserManager::icqOwnerUin()
{
  return strtoul(ownerUserId(LICQ_PPID).accountId().c_str(), (char**)NULL, 10);
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

int UserManager::AddGroup(const string& name, unsigned short icqGroupId)
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
  newGroup->setServerId(LICQ_PPID, icqGroupId);
  newGroup->setSortIndex(myGroups.size());
  myGroups[gid] = newGroup;

  SaveGroups();
  myGroupListMutex.unlockWrite();

  bool icqOnline = false;
  {
    Licq::OwnerReadGuard icqOwner(LICQ_PPID);
    if (icqOwner.isLocked())
      icqOnline = icqOwner->isOnline();
  }

  if (icqGroupId == 0 && icqOnline)
    gIcqProtocol.icqAddGroup(name);
  else
    gLog.info(tr("Added group %s (%u) to list from server"),
        name.c_str(), icqGroupId);

  // Send signal to let plugins know of the new group
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupAdded, UserId(), gid));

  return gid;
}

void UserManager::RemoveGroup(int groupId)
{
  // Must be called when there are no locks on group or group list
  gIcqProtocol.icqRemoveGroup(groupId);

  // TODO: Notify other protocols as well
  //       For signals, include id, name and serverId for protocol as signal
  //       may be processed after group is gone.

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
  group->unlockWrite();
  delete group;


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
      if (u->removeFromGroup(groupId))
        notifyUserUpdated(u->id(), PluginSignal::UserGroups);
    }
  }

  // Send signal to let plugins know of the removed group
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupRemoved, UserId(), groupId));

  // Send signal to let plugins know that sorting indexes may have changed
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupsReordered));
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

bool UserManager::RenameGroup(int groupId, const string& name, bool sendUpdate)
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
  unsigned short icqGroupId = group->serverId(LICQ_PPID);
  group->unlockWrite();

  myGroupListMutex.lockRead();
  SaveGroups();
  myGroupListMutex.unlockRead();

  // If we rename a group on logon, don't send the rename packet
  if (sendUpdate)
    gIcqProtocol.icqRenameGroup(name, icqGroupId);

  // Send signal to let plugins know the group has changed
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupChanged, UserId(), groupId));

  return true;
}

void UserManager::SaveGroups()
{
  if (!m_bAllowSave) return;

  // Load the group info from licq.conf
  Licq::IniFile licqConf("licq.conf");
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

  licqConf.writeFile();
}

int UserManager::GetGroupFromID(unsigned short icqGroupId)
{
  myGroupListMutex.lockRead();
  GroupMap::const_iterator iter;
  int groupId = 0;
  for (iter = myGroups.begin(); iter != myGroups.end(); ++iter)
  {
    iter->second->lockRead();
    if (iter->second->serverId(LICQ_PPID) == icqGroupId)
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

void UserManager::ModifyGroupID(int groupId, unsigned short icqGroupId)
{
  Group* group = fetchGroup(groupId, true);
  if (group == NULL)
    return;

  group->setServerId(LICQ_PPID, icqGroupId);
  group->unlockWrite();

  myGroupListMutex.lockRead();
  SaveGroups();
  myGroupListMutex.unlockRead();
}

unsigned short UserManager::GenerateSID()
{
  bool bCheckGroup, bDone;
  int nSID;
  unsigned short nOwnerPDINFO;

  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    nOwnerPDINFO = o->GetPDINFO();
  }

  // Generate a SID
  srand(time(NULL));
  nSID = 1+(int)(65535.0*rand()/(RAND_MAX+1.0));

  nSID &= 0x7FFF; // server limit it looks like

  // Make sure we have a unique number - a map would be better
  do
  {
    bDone = true;
    bCheckGroup = true;

    if (nSID == 0) nSID++;
    if (nSID == nOwnerPDINFO) nSID++;

    {
      Licq::UserListGuard userList(LICQ_PPID);
      BOOST_FOREACH(const Licq::User* user, **userList)
      {
        Licq::UserReadGuard u(user);

        if (u->GetSID() == nSID  || u->GetInvisibleSID() == nSID ||
          u->GetVisibleSID() == nSID)
        {
          if (nSID == 0x7FFF)
            nSID = 1;
          else
            nSID++;
          bDone = false;	// Restart
          bCheckGroup = false;	// Don't waste time now
          break;
        }
      }
    }

    if (bCheckGroup)
    {
      // Check our groups too!
      Licq::GroupListGuard groupList;
      BOOST_FOREACH(const Licq::Group* group, **groupList)
      {
        Licq::GroupReadGuard g(group);

        unsigned short icqGroupId = g->serverId(LICQ_PPID);
        if (icqGroupId == nSID)
        {
          if (nSID == 0x7FFF)
            nSID = 1;
          else
            nSID++;
          bDone = false;
          break;
        }
      }
    }

  } while (!bDone);

  return nSID;
}

Licq::Owner* UserManager::fetchOwner(unsigned long protocolId, bool writeLock)
{
  Owner* o = NULL;

  myOwnerListMutex.lockRead();
  OwnerMap::iterator iter = myOwners.find(protocolId);
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

bool UserManager::UpdateUsersInGroups()
{
  bool bDid = false;

  Licq::UserListGuard userList(LICQ_PPID);
  BOOST_FOREACH(Licq::User* user, **userList)
  {
    Licq::UserWriteGuard u(user);

    unsigned short nGSID = u->GetGSID();
    if (nGSID)
    {
      int nInGroup = gUserManager.GetGroupFromID(nGSID);
      if (nInGroup != 0)
      {
        u->addToGroup(nInGroup);
        bDid = true;
      }
    }
  }

  return bDid;
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
  if (groupId == 0)
    return;

  int gsid;

  {
    Licq::UserWriteGuard u(userId);
    if (!u.isLocked())
      return;

    gsid = u->GetGSID();

    // Don't remove user from local group if member of the same server group
    if (!inGroup && u->GetSID() != 0 && GetGroupFromID(gsid) == groupId)
      return;

    // Update user object
    u->setInGroup(groupId, inGroup);
  }

  // Notify server
  if (updateServer)
  {
    bool ownerIsOnline = false;
    {
      OwnerReadGuard owner(userId.protocolId());
      ownerIsOnline = (owner.isLocked() && owner->isOnline());
    }

    if (ownerIsOnline)
    {
      if (userId.protocolId() == LICQ_PPID)
      {
        if (inGroup) // Server group can only be changed, not removed
          gIcqProtocol.icqChangeGroup(userId, groupId, gsid);
      }
      else
        gPluginManager.pushProtocolSignal(new Licq::ProtoChangeUserGroupsSignal(userId), userId.protocolId());
    }
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

OwnerReadGuard::OwnerReadGuard(unsigned long protocolId)
  : ReadMutexGuard<Owner>(LicqDaemon::gUserManager.fetchOwner(protocolId, false), true)
{
  // Empty
}

OwnerWriteGuard::OwnerWriteGuard(const UserId& userId)
  : WriteMutexGuard<Owner>(LicqDaemon::gUserManager.fetchOwner(userId, true), true)
{
  // Empty
}

OwnerWriteGuard::OwnerWriteGuard(unsigned long protocolId)
  : WriteMutexGuard<Owner>(LicqDaemon::gUserManager.fetchOwner(protocolId, true), true)
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

UserListGuard::~UserListGuard()
{
  LicqDaemon::gUserManager.unlockUserList();
}

OwnerListGuard::OwnerListGuard()
{
  const OwnerMap& ownerMap = LicqDaemon::gUserManager.lockOwnerList();

  for (OwnerMap::const_iterator i = ownerMap.begin(); i != ownerMap.end(); ++i)
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
