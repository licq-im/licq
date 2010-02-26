#ifndef LICQ_CONTACTLIST_USERMANAGER_H
#define LICQ_CONTACTLIST_USERMANAGER_H

#include <list>
#include <string>

#include "licq/contactlist/group.h"
#include "licq/contactlist/owner.h"
#include "licq/contactlist/user.h"

class CICQDaemon;

namespace Licq
{

/*---------------------------------------------------------------------------
 * FOR_EACH_USER
 *
 * Macros to iterate through the entire list of users.  "pUser" will be a
 * pointer to the current user.
 *-------------------------------------------------------------------------*/
#define FOR_EACH_USER_START(x)                           \
  {                                                      \
    Licq::User* pUser;                                   \
    const Licq::UserMap* _ul_ = Licq::gUserManager.LockUserList(LOCK_R); \
    for (UserMap::const_iterator _i_ = _ul_->begin();    \
         _i_ != _ul_->end(); _i_++)                      \
    {                                                    \
      pUser = _i_->second;                               \
      pUser->Lock(x);                                    \
      {

#define FOR_EACH_PROTO_USER_START(x, y)                  \
  {                                                      \
    Licq::User* pUser;                                   \
    const Licq::UserMap* _ul_ = Licq::gUserManager.LockUserList(LOCK_R); \
    for (UserMap::const_iterator _i_ = _ul_->begin();    \
         _i_ != _ul_->end(); _i_++)                      \
    {                                                    \
      pUser = _i_->second;                               \
      if (pUser->ppid() != x)                            \
        continue;                                        \
      pUser->Lock(y);                                    \
      {

#define FOR_EACH_OWNER_START(x)                           \
  {                                                       \
    Licq::OwnerMap* _ol_ = Licq::gUserManager.LockOwnerList(LOCK_R); \
    for (OwnerMap::const_iterator _i_ = _ol_->begin();    \
         _i_ != _ol_->end(); _i_++)                       \
    {                                                     \
      Licq::Owner* pOwner = _i_->second;                  \
      pOwner->Lock(x);                                    \
      {

#define FOR_EACH_OWNER_END                                \
      }                                                   \
      pOwner->Unlock();                                   \
    }                                                     \
    Licq::gUserManager.UnlockOwnerList();                 \
  }                                                       \

#define FOR_EACH_OWNER_BREAK                              \
        {                                                 \
          Licq::gUserManager.DropOwner(pOwner->PPID());   \
          break;                                          \
        }

#define FOR_EACH_USER_END                \
      }                                  \
      pUser->Unlock();                   \
    }                                    \
    Licq::gUserManager.UnlockUserList(); \
  }

#define FOR_EACH_PROTO_USER_END FOR_EACH_USER_END

#define FOR_EACH_USER_BREAK              \
        {                                \
          Licq::gUserManager.DropUser(pUser); \
          break;                         \
        }

#define FOR_EACH_PROTO_USER_BREAK        \
        {                                \
          Licq::gUserManager.DropUser(pUser); \
          break;                         \
        }

#define FOR_EACH_PROTO_USER_CONTINUE     \
        {                                \
          Licq::gUserManager.DropUser(pUser); \
          continue;                      \
        }

#define FOR_EACH_USER_CONTINUE           \
        {                                \
          Licq::gUserManager.DropUser(pUser); \
          continue;                      \
        }

#define FOR_EACH_GROUP_START(x)                          \
  {                                                      \
    Licq::Group* pGroup;                                 \
    Licq::GroupMap* _gl_ = Licq::gUserManager.LockGroupList(LOCK_R); \
    for (GroupMap::iterator _i_ = _gl_->begin();         \
         _i_ != _gl_->end(); ++_i_)                      \
    {                                                    \
      pGroup = _i_->second;                              \
      pGroup->Lock(x);                                   \
      {

#define FOR_EACH_GROUP_START_SORTED(x)                                  \
  {                                                                     \
    Licq::Group* pGroup;                                                \
    std::list<Licq::Group*> _sortedGroups_;                             \
    FOR_EACH_GROUP_START(LOCK_R)                                        \
      _sortedGroups_.push_back(pGroup);                                 \
      }                                                                 \
      pGroup->Unlock();                                                 \
     }                                                                  \
    }                                                                   \
    _sortedGroups_.sort(Licq::compare_groups);                          \
    for (std::list<Licq::Group*>::iterator _i_ = _sortedGroups_.begin(); \
        _i_ != _sortedGroups_.end(); ++_i_)                             \
    {                                                                   \
      pGroup = *_i_;                                                    \
      pGroup->Lock(x);                                                  \
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
    Licq::gUserManager.UnlockGroupList(); \
  }




#define FOR_EACH_PROTO_ID_START(x)                       \
  {                                                      \
    char *szId;                                          \
    Licq::UserMap* _ul_ = Licq::gUserManager.LockUserList(LOCK_R); \
    for (UserMap::const_iterator _i_ = _ul_->begin();    \
         _i_ != _ul_->end(); _i_++)                      \
    {                                                    \
      if (_i_->first.second != x)                        \
        continue;                                        \
      szId = (*_i_)->IdString();                         \
      {

#define FOR_EACH_PROTO_ID_BREAK          \
        {                                \
          break;                         \
        }

#define FOR_EACH_PROTO_ID_CONTINUE       \
        {                                \
          continue;                      \
        }

/**
 * The amount of registered system groups, excluding the 'All Users' group.
 *
 * @deprecated Scheduled for removal, use NUM_GROUPS_SYSTEM_ALL instead.
 */
const int NUM_GROUPS_SYSTEM     = NUM_GROUPS_SYSTEM_ALL - 1;

extern const char *GroupsSystemNames[NUM_GROUPS_SYSTEM_ALL];


class UserManager
{
public:
  UserManager();
  ~UserManager();
  bool Load();

  /**
   * Find and lock an user object
   *
   * @param userId User id
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @param addUser True if user should be added (as temporary) if not found
   * @param retWasAdded If not null, will be set to true if user was added
   * @return The locked user object if user exist or was created, otherwise NULL
   */
  User* fetchUser(const UserId& userId, unsigned short lockType = LOCK_R,
      bool addUser = false, bool* retWasAdded = NULL);

  // For protocol plugins
  void AddOwner(const char *, unsigned long);
  void RemoveOwner(unsigned long);

  User* FetchUser(const char* idstring, unsigned long ppid, unsigned short lockType)
  { return idstring == NULL ? NULL : fetchUser(User::makeUserId(idstring, ppid), lockType); }

  Owner* FetchOwner(unsigned long ppid, unsigned short lockType);

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
  void DropOwner(const Owner* owner);

  /**
   * Check if a user id is in the list
   *
   * @param userId User id to check
   * @return True if user id is in list, otherwise false
   */
  bool userExists(const UserId& userId);

  /**
   * Get user id for an owner
   *
   * @param ppid Protocol id
   * @return User id of owner or empty string if no such owner exists
   */
  UserId ownerUserId(unsigned long ppid);

  // Get account id for an owner
  std::string OwnerId(unsigned long ppid);

  /**
   * Check if user is an owner
   *
   * @param userId Id of user to check
   * @return True if user id is valid and user is an owner
   */
  bool isOwner(const UserId& userId);

  // ICQ Protocol only (from original Licq)
  void DropUser(const User* user);

  /**
   * Convenience function to get icq owner as an unsigned long
   * Only meant to be used internally for icq protocol functions
   *
   * @return Icq owner
   */
  unsigned long icqOwnerUin();

  /**
   * Notify plugins about changes for a user
   *
   * @param userId User that has changed
   * @param subSignal Sub signal for changes
   */
  void notifyUserUpdated(const Licq::UserId& userId, unsigned long subSignal);

  /**
   * Add a user to the contact list
   *
   * @param userId User to add
   * @param permanent True if user should be added permanently to list and saved to disk
   * @param addToServer True if server should be notified (ignored for temporary users)
   * @param groupId Initial group to place user in or zero for no group
   * @return false if user id is invalid or user is already in list, otherwise true
   */
  bool addUser(const UserId& userId, bool permanent = true,
      bool addToServer = true, unsigned short groupId = 0);

  /**
   * Add a temporary user to the list
   * Will change the user to permanent and optionally add to server side list
   *
   * @param userId User to add
   * @param addToServer True if server should be notified
   * @param groupId Initial group to place user in or zero for no group
   * @return True if user exists and was temporary, otherwise false
   */
  bool makeUserPermanent(const UserId& userId, bool addToServer = true, int groupId = 0);

  /**
   * Remove a user from the list
   *
   * @param userId Id of user to remove
   */
  void removeUser(const UserId& userId, bool removeFromServer = true);

  /**
   * Lock user list for access
   * call UnlockUserList when lock is no longer needed
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @return Map of all users indexed by user id
   */
  UserMap* LockUserList(unsigned short lockType = LOCK_R);

  /**
   * Release user list lock
   */
  void UnlockUserList();

  /**
   * Save user list to configuration file
   * Note: This function assumes that the user list is already locked.
   */
  void saveUserList() const;

  /**
   * Lock group list for access
   * Call UnlockGroupList when lock is no longer needed
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @return Map of all user groups indexed by group ids
   */
  GroupMap* LockGroupList(unsigned short lockType = LOCK_R);

  /**
   * Release group list lock
   */
  void UnlockGroupList();

  /**
   * Lock owner list for access
   * Call UnlockOwnerList when lock is no longer needed
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @return Map of all owners indexed by protocol instance id
   */
  OwnerMap* LockOwnerList(unsigned short lockType = LOCK_R);

  /**
   * Release owner list lock
   */
  void UnlockOwnerList();

  /**
   * Find and lock a group
   * After use, the lock must be released by calling DropGroup()
   *
   * @param groupId Id of group to fetch
   * @param lockType Type of lock to get
   * @return The group if found no NULL if groupId was invalid
   */
  Group* FetchGroup(int groupId, unsigned short lockType = LOCK_R);

  /**
   * Release the lock for a group preivously returned by FetchGroup()
   *
   * @param group The group to unlock
   */
  void DropGroup(const Group* group);

  /**
   * Check if a group id is valid
   *
   * @param gtype Group type
   * @param groupId Id of group to check for
   * @return True if the group exists
   */
  bool groupExists(GroupType gtype, int groupId);

  /**
   * Add a user group
   *
   * @param name Group name, must be unique
   * @param icqGroupId ICQ server group id
   * @return Id of new group or zero if group could not be created
   */
  int AddGroup(const std::string& name, unsigned short icqGroupId = 0);

  /**
   * Remove a user group
   *
   * @param groupId Id of group to remove
   */
  void RemoveGroup(int groupId);

  /**
   * Rename a user group
   *
   * @param groupId Id of group to rename
   * @param name New group name, must be unique
   * @param sendUpdate True if server group should be updated
   * @return True if group was successfully renamed
   */
  bool RenameGroup(int groupId, const std::string& name, bool sendUpdate = true);

  /**
   * Get number of user groups
   *
   * @return Number of user groups
   */
  unsigned int NumGroups();

  /**
   * Save user group list to configuration file
   * Note: This function assumes that user group list is already locked.
   */
  void SaveGroups();

  /**
   * Move sorting position for a group
   * Sorting position for other groups may also be changed to make sure all
   * groups have unique sorting indexes.
   *
   * @param groupId Id of group to move
   * @param newIndex New sorting index where 0 is the top position
   */
  void ModifyGroupSorting(int groupId, int newIndex);

  /**
   * Change ICQ server group id for a user group
   *
   * @param name Name of group to change
   * @param icqGroupId ICQ server group id to set
   */
  void ModifyGroupID(const std::string& name, unsigned short icqGroupId);

  /**
   * Change ICQ server group id for a user group
   *
   * @param groupId Id of group to change
   * @param icqGroupId ICQ server group id to set
   */
  void ModifyGroupID(int groupId, unsigned short icqGroupId);

  /**
   * Get ICQ group id from group name
   *
   * @param name Group name
   * @return Id for ICQ server group or 0 if not found
   */
  unsigned short GetIDFromGroup(const std::string& name);

  /**
   * Get ICQ group id from group
   *
   * @param groupId Group
   * @return Id for iCQ server group or 0 if groupId was invalid
   */
  unsigned short GetIDFromGroup(int groupId);

  /**
   * Get group id from ICQ server group id
   *
   * @param icqGroupId ICQ server group id
   * @return Id for group or 0 if not found
   */
  int GetGroupFromID(unsigned short icqGroupId);

  /**
   * Find id for group with a given name
   *
   * @param name Name of the group
   * @return Id for the group or 0 if there is no group with that name
   */
  int GetGroupFromName(const std::string& name);

  /**
   * Get group name from the given group ID
   *
   * @param groupId Internal group ID
   * @return Group name
   */
  std::string GetGroupNameFromGroup(int groupId);

  unsigned short GenerateSID();

  /**
   * Set user group membership and (optionally) update server
   *
   * @param userId User id
   * @param groupType Group type
   * @param groupId Group id
   * @param inGroup True to add user to group or false to remove
   * @param updateServer True if server list should be updated
   */
  void setUserInGroup(const UserId& userId, GroupType groupType,
      int groupId, bool inGroup, bool updateServer = true);

  /**
   * Add user to a group and update server group
   *
   * @param userId User id
   * @param groupId Group id
   */
  void addUserToGroup(const UserId& userId, int groupId)
  { setUserInGroup(userId, GROUPS_USER, groupId, true, true); }

  /**
   * Remove user from a group
   *
   * @param userId User id
   * @param groupId Group id
   */
  void removeUserFromGroup(const UserId& userId, int groupId)
  { setUserInGroup(userId, GROUPS_USER, groupId, false); }

  void SaveAllUsers();

  char* DefaultUserEncoding() { return m_szDefaultEncoding; }
  void SetDefaultUserEncoding(const char* defaultEncoding);

  bool UpdateUsersInGroups();

  unsigned short NumUsers();
  unsigned short NumOwners();

protected:
  ReadWriteMutex myGroupListMutex;
  ReadWriteMutex myUserListMutex;
  ReadWriteMutex myOwnerListMutex;

  GroupMap myGroups;
  UserMap myUsers;
  OwnerMap myOwners;
  unsigned short m_nUserListLockType;
  unsigned short myGroupListLockType;
  unsigned short m_nOwnerListLockType;
  bool m_bAllowSave;
  char* m_szDefaultEncoding;

  friend class ::CICQDaemon;
};


extern class UserManager gUserManager;

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

} // namespace Licq

#endif // LICQ_CONTACTLIST_USERMANAGER_H
