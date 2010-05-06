/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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


namespace LicqDaemon
{

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


  // From Licq::UserManager

  Licq::User* fetchUser(const Licq::UserId& userId, unsigned short lockType = LOCK_R,
      bool addUser = false, bool* retWasAdded = NULL);
  void addOwner(const Licq::UserId& userId);
  void RemoveOwner(unsigned long);

  Licq::Owner* FetchOwner(unsigned long ppid, unsigned short lockType);
  void DropOwner(const Licq::Owner* owner);
  bool userExists(const Licq::UserId& userId);
  Licq::UserId ownerUserId(unsigned long ppid);
  std::string OwnerId(unsigned long ppid);
  bool isOwner(const Licq::UserId& userId);
  void DropUser(const Licq::User* user);
  unsigned long icqOwnerUin();
  void notifyUserUpdated(const Licq::UserId& userId, unsigned long subSignal);
  bool addUser(const Licq::UserId& userId, bool permanent = true,
      bool addToServer = true, unsigned short groupId = 0);
  bool makeUserPermanent(const Licq::UserId& userId, bool addToServer = true, int groupId = 0);
  void removeUser(const Licq::UserId& userId, bool removeFromServer = true);
  Licq::UserMap* LockUserList(unsigned short lockType = LOCK_R);
  void UnlockUserList();
  Licq::GroupMap* LockGroupList(unsigned short lockType = LOCK_R);
  void UnlockGroupList();
  Licq::OwnerMap* LockOwnerList(unsigned short lockType = LOCK_R);
  void UnlockOwnerList();
  Licq::Group* FetchGroup(int groupId, unsigned short lockType = LOCK_R);
  void DropGroup(const Licq::Group* group);
  bool groupExists(int groupId);
  int AddGroup(const std::string& name, unsigned short icqGroupId = 0);
  void RemoveGroup(int groupId);
  bool RenameGroup(int groupId, const std::string& name, bool sendUpdate = true);
  void SaveGroups();
  void ModifyGroupSorting(int groupId, int newIndex);
  void ModifyGroupID(const std::string& name, unsigned short icqGroupId);
  void ModifyGroupID(int groupId, unsigned short icqGroupId);
  unsigned short GetIDFromGroup(const std::string& name);
  unsigned short GetIDFromGroup(int groupId);
  int GetGroupFromID(unsigned short icqGroupId);
  int GetGroupFromName(const std::string& name);
  std::string GetGroupNameFromGroup(int groupId);
  void setUserInGroup(const Licq::UserId& userId, int groupId,
      bool inGroup, bool updateServer = true);
  void userStatusChanged(const Licq::UserId& userId, unsigned newStatus);
  void ownerStatusChanged(unsigned long protocolId, unsigned newStatus);
  void SaveAllUsers();
  const char* DefaultUserEncoding();
  void SetDefaultUserEncoding(const char* defaultEncoding);
  unsigned short NumUsers();
  unsigned short NumOwners();
  unsigned int NumGroups();

private:
  Licq::ReadWriteMutex myGroupListMutex;
  Licq::ReadWriteMutex myUserListMutex;
  Licq::ReadWriteMutex myOwnerListMutex;

  Licq::GroupMap myGroups;
  Licq::UserMap myUsers;
  Licq::OwnerMap myOwners;
  unsigned short m_nUserListLockType;
  unsigned short myGroupListLockType;
  unsigned short m_nOwnerListLockType;
  bool m_bAllowSave;
  char* m_szDefaultEncoding;
};

extern UserManager gUserManager;

} // namespace LicqDaemon

#endif
