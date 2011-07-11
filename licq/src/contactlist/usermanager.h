/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq developers
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

#ifndef LICQDAEMON_USERMANAGER_H
#define LICQDAEMON_USERMANAGER_H

#include <licq/contactlist/usermanager.h>

#include <map>

#include <licq/thread/readwritemutex.h>
#include <licq/userid.h>


namespace LicqDaemon
{
class Group;
class Owner;
class User;

typedef std::map<Licq::UserId, User*> UserMap;
typedef std::map<int, Group*> GroupMap;
typedef std::map<unsigned long, Owner*> OwnerMap;

class UserManager : public Licq::UserManager
{
public:
  UserManager();
  ~UserManager();

  /**
   * Shut down the user manager
   */
  void shutdown();

  bool Load();
  unsigned short GenerateSID();
  bool UpdateUsersInGroups();

  /**
   * Save user list to configuration file
   * Note: This function assumes that the user list is already locked.
   */
  void saveUserList() const;

  /**
   * Fetch and lock the user list map
   *
   * @return The internal map with all users
   */
  const UserMap& lockUserList();

  /**
   * Free lock on user list map
   */
  void unlockUserList();

  /**
   * Fetch and lock the owner list map
   *
   * @return The internal map with all owners
   */
  const OwnerMap& lockOwnerList();

  /**
   * Free lock on group list map
   */
  void unlockOwnerList();

  /**
   * Fetch and lock the group list map
   *
   * @return The internal map with all groups
   */
  const GroupMap& lockGroupList();

  /**
   * Free lock on group list map
   */
  void unlockGroupList();

  /**
   * Fetch and lock a group
   *
   * @param groupId Id of group to get
   * @param writeLock True to lock group for writing, false for read lock
   * @return Requested group if exist, otherwise NULL
   */
  Group* fetchGroup(int groupId, bool writeLock = false);

  /**
   * Find and lock an user object
   *
   * @param userId User id
   * @param writeLock True to lock user for writing, false for read lock
   * @param addUser True if user should be added (as temporary) if not found
   * @param retWasAdded If not null, will be set to true if user was added
   * @return The locked user object if user exist or was created, otherwise NULL
   */
  Licq::User* fetchUser(const Licq::UserId& userId, bool writeLock = false,
      bool addUser = false, bool* retWasAdded = NULL);

  /**
   * Fetch and lock an owner object based on protocolId
   *
   * @param protocolId Protocol to get owner for
   * @param writeLock True to lock owner for writing, false for read lock
   * @return The locked owner object if owner exists, otherwise NULL
   */
  Licq::Owner* fetchOwner(unsigned long protocolId, bool writeLock = false);

  /**
   * Find and lock an owner object based on userId
   *
   * Note: Currently this is just a convenience wrapper but if/when Licq
   *   starts supporting multiple owners per protocol this call will be needed
   *   to be able to get any owner.
   *
   * @param userId User id of owner
   * @param writeLock True to lock owner for writing, false for read lock
   * @return The locked owner object if owner exists, otherwise NULL
   */
  Licq::Owner* fetchOwner(const Licq::UserId& userId, bool writeLock = false)
  { return fetchOwner(userId.protocolId(), writeLock); }

  // From Licq::UserManager
  void addOwner(const Licq::UserId& userId);
  void RemoveOwner(unsigned long);
  bool userExists(const Licq::UserId& userId);
  Licq::UserId ownerUserId(unsigned long ppid);
  bool isOwner(const Licq::UserId& userId);
  unsigned long icqOwnerUin();
  void notifyUserUpdated(const Licq::UserId& userId, unsigned long subSignal);
  bool addUser(const Licq::UserId& userId, bool permanent = true,
      bool addToServer = true, unsigned short groupId = 0);
  bool makeUserPermanent(const Licq::UserId& userId, bool addToServer = true, int groupId = 0);
  void removeUser(const Licq::UserId& userId, bool removeFromServer = true);
  bool groupExists(int groupId);
  int AddGroup(const std::string& name, unsigned short icqGroupId = 0);
  void RemoveGroup(int groupId);
  bool RenameGroup(int groupId, const std::string& name, bool sendUpdate = true);
  void ModifyGroupSorting(int groupId, int newIndex);
  void ModifyGroupID(int groupId, unsigned short icqGroupId);
  int GetGroupFromID(unsigned short icqGroupId);
  int GetGroupFromName(const std::string& name);
  std::string GetGroupNameFromGroup(int groupId);
  void setUserInGroup(const Licq::UserId& userId, int groupId,
      bool inGroup, bool updateServer = true);
  void SaveAllUsers();
  const std::string& defaultUserEncoding();
  void setDefaultUserEncoding(const std::string& defaultEncoding);
  unsigned short NumUsers();
  unsigned short NumOwners();
  unsigned int NumGroups();

private:
  void SaveGroups();

  Licq::ReadWriteMutex myGroupListMutex;
  Licq::ReadWriteMutex myUserListMutex;
  Licq::ReadWriteMutex myOwnerListMutex;

  GroupMap myGroups;
  UserMap myUsers;
  OwnerMap myOwners;
  bool m_bAllowSave;
  std::string myDefaultEncoding;
};

extern UserManager gUserManager;

} // namespace LicqDaemon

#endif
