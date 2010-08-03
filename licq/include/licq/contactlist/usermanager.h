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

#ifndef LICQ_CONTACTLIST_USERMANAGER_H
#define LICQ_CONTACTLIST_USERMANAGER_H

#include <boost/noncopyable.hpp>
#include <list>
#include <string>

namespace Licq
{
class Group;
class Owner;
class User;
class UserId;

typedef std::list<User*> UserList;
typedef std::list<Owner*> OwnerList;
typedef std::list<Group*> GroupList;

class UserManager : private boost::noncopyable
{
public:
  virtual void addOwner(const UserId& userId) = 0;

  virtual void RemoveOwner(unsigned long) = 0;

  /**
   * Check if a user id is in the list
   *
   * @param userId User id to check
   * @return True if user id is in list, otherwise false
   */
  virtual bool userExists(const UserId& userId) = 0;

  /**
   * Get user id for an owner
   *
   * @param ppid Protocol id
   * @return User id of owner or empty string if no such owner exists
   */
  virtual UserId ownerUserId(unsigned long ppid) = 0;

  /**
   * Check if user is an owner
   *
   * @param userId Id of user to check
   * @return True if user id is valid and user is an owner
   */
  virtual bool isOwner(const UserId& userId) = 0;

  /**
   * Convenience function to get icq owner as an unsigned long
   * Only meant to be used internally for icq protocol functions
   *
   * @return Icq owner
   */
  virtual unsigned long icqOwnerUin() = 0;

  /**
   * Notify plugins about changes for a user
   *
   * @param userId User that has changed
   * @param subSignal Sub signal for changes
   */
  virtual void notifyUserUpdated(const UserId& userId, unsigned long subSignal) = 0;

  /**
   * Add a user to the contact list
   *
   * @param userId User to add
   * @param permanent True if user should be added permanently to list and saved to disk
   * @param addToServer True if server should be notified (ignored for temporary users)
   * @param groupId Initial group to place user in or zero for no group
   * @return false if user id is invalid or user is already in list, otherwise true
   */
  virtual bool addUser(const UserId& userId, bool permanent = true,
      bool addToServer = true, unsigned short groupId = 0) = 0;

  /**
   * Add a temporary user to the list
   * Will change the user to permanent and optionally add to server side list
   *
   * @param userId User to add
   * @param addToServer True if server should be notified
   * @param groupId Initial group to place user in or zero for no group
   * @return True if user exists and was temporary, otherwise false
   */
  virtual bool makeUserPermanent(const UserId& userId, bool addToServer = true, int groupId = 0) = 0;

  /**
   * Remove a user from the list
   *
   * @param userId Id of user to remove
   */
  virtual void removeUser(const UserId& userId, bool removeFromServer = true) = 0;

  /**
   * Check if a group id is valid
   *
   * @param groupId Id of user group to check for
   * @return True if the group exists
   */
  virtual bool groupExists(int groupId) = 0;

  /**
   * Add a user group
   *
   * @param name Group name, must be unique
   * @param icqGroupId ICQ server group id
   * @return Id of new group or zero if group could not be created
   */
  virtual int AddGroup(const std::string& name, unsigned short icqGroupId = 0) = 0;

  /**
   * Remove a user group
   *
   * @param groupId Id of group to remove
   */
  virtual void RemoveGroup(int groupId) = 0;

  /**
   * Rename a user group
   *
   * @param groupId Id of group to rename
   * @param name New group name, must be unique
   * @param sendUpdate True if server group should be updated
   * @return True if group was successfully renamed
   */
  virtual bool RenameGroup(int groupId, const std::string& name, bool sendUpdate = true) = 0;

  /**
   * Get number of user groups
   *
   * @return Number of user groups
   */
  virtual unsigned int NumGroups() = 0;

  /**
   * Move sorting position for a group
   * Sorting position for other groups may also be changed to make sure all
   * groups have unique sorting indexes.
   *
   * @param groupId Id of group to move
   * @param newIndex New sorting index where 0 is the top position
   */
  virtual void ModifyGroupSorting(int groupId, int newIndex) = 0;

  /**
   * Change ICQ server group id for a user group
   *
   * @param groupId Id of group to change
   * @param icqGroupId ICQ server group id to set
   */
  virtual void ModifyGroupID(int groupId, unsigned short icqGroupId) = 0;

  /**
   * Get group id from ICQ server group id
   *
   * @param icqGroupId ICQ server group id
   * @return Id for group or 0 if not found
   */
  virtual int GetGroupFromID(unsigned short icqGroupId) = 0;

  /**
   * Find id for group with a given name
   *
   * @param name Name of the group
   * @return Id for the group or 0 if there is no group with that name
   */
  virtual int GetGroupFromName(const std::string& name) = 0;

  /**
   * Get group name from the given group ID
   *
   * @param groupId Internal group ID
   * @return Group name
   */
  virtual std::string GetGroupNameFromGroup(int groupId) = 0;

  /**
   * Set user group membership and (optionally) update server
   *
   * @param userId Id of user
   * @param groupId Id of user group
   * @param inGroup True to add user to group or false to remove
   * @param updateServer True if server list should be updated
   */
  virtual void setUserInGroup(const UserId& userId, int groupId,
      bool inGroup, bool updateServer = true) = 0;

  /**
   * Add user to a group and update server group
   *
   * @param userId User id
   * @param groupId Group id
   */
  void addUserToGroup(const UserId& userId, int groupId)
  { setUserInGroup(userId, groupId, true); }

  /**
   * Remove user from a group
   *
   * @param userId User id
   * @param groupId Group id
   */
  void removeUserFromGroup(const UserId& userId, int groupId)
  { setUserInGroup(userId, groupId, false); }

  /**
   * Convenience function to change status for a user (or owner) and signal plugins
   * This function is used by protocol plugins to report status changes
   *
   * @Param userId User to change status for
   * @param newStatus New status for user/owner
   */
  virtual void userStatusChanged(const UserId& userId, unsigned newStatus) = 0;

  /**
   * Convenience function to change status for an owner and signal plugins
   * This function is used by protocol plugins to report status changes
   *
   * @Param userId User to change status for
   * @param newStatus New status for owner
   */
  virtual void ownerStatusChanged(unsigned long protocolId, unsigned newStatus) = 0;

  virtual void SaveAllUsers() = 0;
  virtual const std::string& defaultUserEncoding() = 0;
  virtual void setDefaultUserEncoding(const std::string& defaultEncoding) = 0;

  virtual unsigned short NumUsers() = 0;
  virtual unsigned short NumOwners() = 0;

protected:
  virtual ~UserManager() { /* Empty */ }
};


extern UserManager& gUserManager;


/**
 * Mutex guard for accessing the user list
 * The user list will be locked by creating a UserListGuard and unlocked when
 * the guard is destroyed. This class also acts as a wrapper to hide the
 * internals of the user manager.
 *
 * Note that the users in the list are not locked and any access beyond
 * reading the id requires the user to be locked by the caller.
 *
 * Usage example: Iterate over all ICQ users with read-only access
 *     Licq::UserListGuard userList;
 *     BOOST_FOREACH(const Licq::User* user, **userList)
 *     {
 *       Licq::UserReadGuard u(user);
 *       ...
 *     }
 */
class UserListGuard : private boost::noncopyable
{
public:
  /**
   * Constructor
   * Will read-lock the user list in user manager
   *
   * @param protocolId Protocol id to get users for or zero to get all
   */
  UserListGuard(unsigned long protocolId = 0);

  /**
   * Destructor, will release lock on user list
   */
  ~UserListGuard();

  // Access operators
  const UserList* operator*() const { return &myUserList; }
  const UserList* operator->() const { return &myUserList; }

private:
  UserList myUserList;
};

/**
 * Mutex guard for accessing the owner list
 * The owner list will be locked by creating a OwnerListGuard and unlocked when
 * the guard is destroyed. This class also acts as a wrapper to hide the
 * internals of the user manager.
 *
 * Note that the owners in the list are not locked and any access beyond
 * reading the id requires the owner to be locked by the caller.
 *
 * Usage example: Iterate over all owners with write access
 *     Licq::OwnerListGuard ownerList;
 *     BOOST_FOREACH(Licq::Owner* owner, **ownerList)
 *     {
 *       Licq::OwnerWriteGuard o(owner);
 *       ...
 *     }
 */
class OwnerListGuard : private boost::noncopyable
{
public:
  /**
   * Constructor
   * Will read-lock the owner list in user manager
   */
  OwnerListGuard();

  /**
   * Destructor, will release lock on group list
   */
  ~OwnerListGuard();

  // Access operators
  const OwnerList* operator*() const { return &myOwnerList; }
  const OwnerList* operator->() const { return &myOwnerList; }

private:
  OwnerList myOwnerList;
};

/**
 * Mutex guard for accessing the group list
 * The group list will be locked by creating a GroupListGuard and unlocked when
 * the guard is destroyed. This class also acts as a wrapper to hide the
 * internals of the user manager.
 *
 * Note that the groups in the list are not locked and any access beyond
 * reading the id requires the group to be locked by the caller.
 *
 * Usage example: Iterate over all groups (unsorted) with read-only access
 *     Licq::GroupListGuard groupList(false);
 *     BOOST_FOREACH(const Licq::Group* group, **groupList)
 *     {
 *       Licq::GroupReadGuard g(group);
 *       ...
 *     }
 */
class GroupListGuard : private boost::noncopyable
{
public:
  /**
   * Constructor
   * Will read-lock the group list in user manager
   *
   * @param sorted True if list should be sorted
   */
  GroupListGuard(bool sorted = true);

  /**
   * Destructor, will release lock on group list
   */
  ~GroupListGuard();

  // Access operators
  const GroupList* operator*() const { return &myGroupList; }
  const GroupList* operator->() const { return &myGroupList; }

private:
  GroupList myGroupList;
};


} // namespace Licq

#endif // LICQ_CONTACTLIST_USERMANAGER_H
