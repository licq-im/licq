#ifndef LICQ_CONTACTLIST_USERMANAGER_H
#define LICQ_CONTACTLIST_USERMANAGER_H

#include <boost/noncopyable.hpp>
#include <list>
#include <map>
#include <string>

#include "group.h"
#include "owner.h"
#include "user.h"

class CICQDaemon;


/*---------------------------------------------------------------------------
 * FOR_EACH_USER
 *
 * Macros to iterate through the entire list of users.  "pUser" will be a
 * pointer to the current user.
 *-------------------------------------------------------------------------*/
#include <boost/foreach.hpp>

#define FOR_EACH_USER_START(x)                           \
  {                                                      \
    Licq::UserListGuard _ul_;                            \
    BOOST_FOREACH(Licq::User* pUser, **_ul_)             \
    {                                                    \
      pUser->Lock(x);                                    \
      {

#define FOR_EACH_PROTO_USER_START(x, y)                  \
  {                                                      \
    Licq::UserListGuard _ul_(x);                         \
    BOOST_FOREACH(Licq::User* pUser, **_ul_)             \
    {                                                    \
      pUser->Lock(y);                                    \
      {

#define FOR_EACH_OWNER_START(x)                           \
  {                                                      \
    Licq::OwnerListGuard _ol_;                           \
    BOOST_FOREACH(Licq::Owner* pOwner, **_ol_)           \
    {                                                    \
      pOwner->Lock(x);                                   \
      {

#define FOR_EACH_OWNER_END                                \
      }                                                   \
      pOwner->Unlock();                                   \
    }                                                     \
  }                                                       \

#define FOR_EACH_OWNER_BREAK                              \
        {                                                 \
          pOwner->Unlock();                               \
          break;                                          \
        }

#define FOR_EACH_USER_END                \
      }                                  \
      pUser->Unlock();                   \
    }                                    \
  }

#define FOR_EACH_PROTO_USER_END FOR_EACH_USER_END

#define FOR_EACH_USER_BREAK              \
        {                                \
          pUser->Unlock();               \
          break;                         \
        }

#define FOR_EACH_PROTO_USER_BREAK FOR_EACH_USER_BREAK

#define FOR_EACH_USER_CONTINUE           \
        {                                \
          pUser->Unlock();               \
          continue;                      \
        }

#define FOR_EACH_PROTO_USER_CONTINUE FOR_EACH_USER_CONTINUE

#define FOR_EACH_GROUP_START(x)                          \
  {                                                      \
    Licq::GroupListGuard _gl_(false);                    \
    BOOST_FOREACH(Licq::Group* pGroup, **_gl_)           \
    {                                                    \
      pGroup->Lock(x);                                   \
      {

#define FOR_EACH_GROUP_START_SORTED(x)                                  \
  {                                                      \
    Licq::GroupListGuard _gl_;                           \
    BOOST_FOREACH(Licq::Group* pGroup, **_gl_)           \
    {                                                    \
      pGroup->Lock(x);                                   \
      {

#define FOR_EACH_GROUP_CONTINUE          \
        {                                \
          pGroup->Unlock();              \
          continue;                      \
        }

#define FOR_EACH_GROUP_BREAK             \
        {                                \
          pGroup->Unlock();              \
          break;                         \
        }

#define FOR_EACH_GROUP_END               \
      }                                  \
      pGroup->Unlock();                  \
    }                                    \
  }


namespace Licq
{
// Lists for public API
typedef std::list<User*> UserList;
typedef std::list<Owner*> OwnerList;
typedef std::list<Group*> GroupList;

// Maps for internal user manager data
typedef std::map<UserId, class Licq::User*> UserMap;
typedef std::map<int, Licq::Group*> GroupMap;
typedef std::map<unsigned long, class Licq::Owner*> OwnerMap;

class UserManager : private boost::noncopyable
{
public:
  /**
   * Find and lock an user object
   *
   * @param userId User id
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @param addUser True if user should be added (as temporary) if not found
   * @param retWasAdded If not null, will be set to true if user was added
   * @return The locked user object if user exist or was created, otherwise NULL
   */
  virtual User* fetchUser(const UserId& userId, unsigned short lockType = LOCK_R,
      bool addUser = false, bool* retWasAdded = NULL) = 0;

  // For protocol plugins
  virtual void addOwner(const UserId& userId) = 0;

  void AddOwner(const char* accountId, unsigned long protocolId)
  { if (accountId != 0) addOwner(UserId(accountId, protocolId)); }

  virtual void RemoveOwner(unsigned long) = 0;

  User* FetchUser(const char* idstring, unsigned long ppid, unsigned short lockType)
  { return idstring == NULL ? NULL : fetchUser(User::makeUserId(idstring, ppid), lockType); }

  virtual Owner* FetchOwner(unsigned long ppid, unsigned short lockType) = 0;

  /**
   * Find and lock an owner object based on userId
   *
   * Note: Currently this is just a convenience wrapper but if/when Licq
   *   starts supporting multiple owners per protocol this call will be needed
   *   to be able to get any owner.
   *
   * @param userId User id of owner
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @return The locked owner object if owner exists, otherwise NULL
   */
  Owner* fetchOwner(const UserId& userId, unsigned short lockType)
  { return FetchOwner(User::getUserProtocolId(userId), lockType); }

  /**
   * Release owner lock
   */
  virtual void DropOwner(const Owner* owner) = 0;

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

  // Get account id for an owner
  virtual std::string OwnerId(unsigned long ppid) = 0;

  /**
   * Check if user is an owner
   *
   * @param userId Id of user to check
   * @return True if user id is valid and user is an owner
   */
  virtual bool isOwner(const UserId& userId) = 0;

  // ICQ Protocol only (from original Licq)
  virtual void DropUser(const User* user) = 0;

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
  virtual void notifyUserUpdated(const Licq::UserId& userId, unsigned long subSignal) = 0;

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
   * Lock user list for access
   * call UnlockUserList when lock is no longer needed
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @return Map of all users indexed by user id
   */
  virtual UserMap* LockUserList(unsigned short lockType = LOCK_R) = 0;

  /**
   * Release user list lock
   */
  virtual void UnlockUserList() = 0;

  /**
   * Lock group list for access
   * Call UnlockGroupList when lock is no longer needed
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @return Map of all user groups indexed by group ids
   */
  virtual GroupMap* LockGroupList(unsigned short lockType = LOCK_R) = 0;

  /**
   * Release group list lock
   */
  virtual void UnlockGroupList() = 0;

  /**
   * Lock owner list for access
   * Call UnlockOwnerList when lock is no longer needed
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @return Map of all owners indexed by protocol instance id
   */
  virtual OwnerMap* LockOwnerList(unsigned short lockType = LOCK_R) = 0;

  /**
   * Release owner list lock
   */
  virtual void UnlockOwnerList() = 0;

  /**
   * Find and lock a group
   * After use, the lock must be released by calling DropGroup()
   *
   * @param groupId Id of group to fetch
   * @param lockType Type of lock to get
   * @return The group if found no NULL if groupId was invalid
   */
  virtual Group* FetchGroup(int groupId, unsigned short lockType = LOCK_R) = 0;

  /**
   * Release the lock for a group preivously returned by FetchGroup()
   *
   * @param group The group to unlock
   */
  virtual void DropGroup(const Group* group) = 0;

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
   * Save user group list to configuration file
   * Note: This function assumes that user group list is already locked.
   */
  virtual void SaveGroups() = 0;

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
   * @param name Name of group to change
   * @param icqGroupId ICQ server group id to set
   */
  virtual void ModifyGroupID(const std::string& name, unsigned short icqGroupId) = 0;

  /**
   * Change ICQ server group id for a user group
   *
   * @param groupId Id of group to change
   * @param icqGroupId ICQ server group id to set
   */
  virtual void ModifyGroupID(int groupId, unsigned short icqGroupId) = 0;

  /**
   * Get ICQ group id from group name
   *
   * @param name Group name
   * @return Id for ICQ server group or 0 if not found
   */
  virtual unsigned short GetIDFromGroup(const std::string& name) = 0;

  /**
   * Get ICQ group id from group
   *
   * @param groupId Group
   * @return Id for iCQ server group or 0 if groupId was invalid
   */
  virtual unsigned short GetIDFromGroup(int groupId) = 0;

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
  virtual const char* DefaultUserEncoding() = 0;
  virtual void SetDefaultUserEncoding(const char* defaultEncoding) = 0;

  virtual unsigned short NumUsers() = 0;
  virtual unsigned short NumOwners() = 0;

protected:
  virtual ~UserManager() { /* Empty */ }
};


extern UserManager& gUserManager;

/**
 * Read mutex guard for Licq::User
 */
class UserReadGuard : public ReadMutexGuard<User>
{
public:
  /**
   * Constructor, will fetch and lock a user based on user id
   * Note: Always check that the user was actually fetched before using
   *
   * @param userId Id of user to fetch
   * @param addUser True if user should be added (as temporary) if not found
   * @param retWasAdded If not null, will be set to true if user was added
   */
  UserReadGuard(const UserId& userId, bool addUser = false, bool* retWasAdded = NULL)
    : ReadMutexGuard<User>(gUserManager.fetchUser(userId, LOCK_R, addUser, retWasAdded))
  { }

  // Derived constructors
  UserReadGuard(User* user, bool locked = true)
    : ReadMutexGuard<User>(user, locked)
  { }
  UserReadGuard(ReadMutexGuard<User>* guard)
    : ReadMutexGuard<User>(guard)
  { }
};

/**
 * Write mutex guard for Licq::User
 */
class UserWriteGuard : public WriteMutexGuard<User>
{
public:
  /**
   * Constructor, will fetch and lock a user based on user id
   * Note: Always check that the user was actually fetched before using
   *
   * @param userId Id of user to fetch
   * @param addUser True if user should be added (as temporary) if not found
   * @param retWasAdded If not null, will be set to true if user was added
   */
  UserWriteGuard(const UserId& userId, bool addUser = false, bool* retWasAdded = NULL)
    : WriteMutexGuard<User>(gUserManager.fetchUser(userId, LOCK_W, addUser, retWasAdded))
  { }

  // Derived constructors
  UserWriteGuard(User* user, bool locked = true)
    : WriteMutexGuard<User>(user, locked)
  { }
  UserWriteGuard(WriteMutexGuard<User>* guard)
    : WriteMutexGuard<User>(guard)
  { }
};

/**
 * Read mutex guard for Licq::Owner
 */
class OwnerReadGuard : public ReadMutexGuard<Owner>
{
public:
  /**
   * Constructor, will fetch and lock an owner based on user id
   * Note: Always check that the owner was actually fetched before using
   *
   * @param userId Id of owner to fetch
   */
  OwnerReadGuard(const UserId& userId)
    : ReadMutexGuard<Owner>(gUserManager.fetchOwner(userId, LOCK_R))
  { }

  /**
   * Constructor, will fetch and lock an owner based on protocolId
   * Note: Always check that the owner was actually fetched before using
   *
   * @param userId Id of owner to fetch
   */
  OwnerReadGuard(unsigned long protocolId)
    : ReadMutexGuard<Owner>(gUserManager.FetchOwner(protocolId, LOCK_R))
  { }

  // Derived constructors
  OwnerReadGuard(Owner* owner, bool locked = true)
    : ReadMutexGuard<Owner>(owner, locked)
  { }
  OwnerReadGuard(ReadMutexGuard<Owner>* guard)
    : ReadMutexGuard<Owner>(guard)
  { }
};

/**
 * Write mutex guard for Licq::Owner
 */
class OwnerWriteGuard : public WriteMutexGuard<Owner>
{
public:
  /**
   * Constructor, will fetch and lock an owner based on user id
   * Note: Always check that the owner was actually fetched before using
   *
   * @param userId Id of owner to fetch
   */
  OwnerWriteGuard(const UserId& userId)
    : WriteMutexGuard<Owner>(gUserManager.fetchOwner(userId, LOCK_W))
  { }

  /**
   * Constructor, will fetch and lock an owner based on protocolId
   * Note: Always check that the owner was actually fetched before using
   *
   * @param userId Id of owner to fetch
   */
  OwnerWriteGuard(unsigned long protocolId)
    : WriteMutexGuard<Owner>(gUserManager.FetchOwner(protocolId, LOCK_W))
  { }

  // Derived constructors
  OwnerWriteGuard(Owner* owner, bool locked = true)
    : WriteMutexGuard<Owner>(owner, locked)
  { }
  OwnerWriteGuard(WriteMutexGuard<Owner>* guard)
    : WriteMutexGuard<Owner>(guard)
  { }
};

/**
 * Read mutex guard for Licq::Group
 */
class GroupReadGuard : public ReadMutexGuard<Group>
{
public:
  /**
   * Constructor, will fetch and lock a group based on group id
   * Note: Always check that the group was actually fetched before using
   *
   * @param groupId Id of group to fetch
   */
  GroupReadGuard(int groupId)
    : ReadMutexGuard<Group>(gUserManager.FetchGroup(groupId, LOCK_R))
  { }

  // Derived constructors
  GroupReadGuard(Group* group, bool locked = true)
    : ReadMutexGuard<Group>(group, locked)
  { }
  GroupReadGuard(ReadMutexGuard<Group>* guard)
    : ReadMutexGuard<Group>(guard)
  { }
};

/**
 * Write mutex guard for Licq::Group
 */
class GroupWriteGuard : public WriteMutexGuard<Group>
{
public:
  /**
   * Constructor, will fetch and lock a group based on group id
   * Note: Always check that the group was actually fetched before using
   *
   * @param groupId Id of group to fetch
   */
  GroupWriteGuard(int groupId)
    : WriteMutexGuard<Group>(gUserManager.FetchGroup(groupId, LOCK_W))
  { }

  // Derived constructors
  GroupWriteGuard(Group* group, bool locked = true)
    : WriteMutexGuard<Group>(group, locked)
  { }
  GroupWriteGuard(WriteMutexGuard<Group>* guard)
    : WriteMutexGuard<Group>(guard)
  { }
};

/**
 * Mutex guard for accessing the user list
 * The user list will be locked by creating a UserListGuard and unlocked when
 * the guard is destroyed. This class also acts as a wrapper to hide the
 * internals of the user manager.
 *
 * Note that the users in the list are not locked and any access beyond
 * reading the id requires the user to be locked by the caller.
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
