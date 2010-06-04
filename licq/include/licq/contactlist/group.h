#ifndef LICQ_CONTACTLIST_GROUP_H
#define LICQ_CONTACTLIST_GROUP_H

#include <boost/noncopyable.hpp>
#include <string>

#include "../thread/lockable.h"

namespace Licq
{

/**
 * Class holding data for a user group in the contact list.
 * System groups only exists as a bitmask in LicqUser.
 *
 * Note: LicqGroup objects should only be created, deleted or modified from the
 * user manager. If set functions are called directly, plugins will not receive
 * any signal notifying them of the change.
 */
class Group : public Lockable, private boost::noncopyable
{
public:
  /**
   * Get id for group. This is an id used locally by Licq and is persistant for
   * each group.
   *
   * @return Group id
   */
  int id() const
  { return myId; }

  /**
   * Get name of group as should be displayed in the user interface
   *
   * @return Group name
   */
  const std::string& name() const
  { return myName; }

  /**
   * Get sorting index for the group. This is used by user interface plugins to
   * determine sorting order for the groups. Lower numbers should be displayed
   * higher in the list.
   *
   * @return Sorting index for this group
   */
  int sortIndex() const
  { return mySortIndex; }

  /**
   * Group id for this group at the server side
   *
   * @param protocolId Id of protocol to get group id for
   * @return Server side group id or 0 if not set or not known
   */
  virtual unsigned long serverId(unsigned long protocolId) const = 0;

  /**
   * Set group name
   *
   * @param name New group name
   */
  void setName(const std::string& name) { myName = name; }

  /**
   * Set sorting index for group
   *
   * @param sortIndex Group sorting index
   */
  void setSortIndex(int sortIndex) { mySortIndex = sortIndex; }

  /**
   * Set server side id for this group
   *
   * @param protocolId Id of protocol to set group id for
   * @param serverId Id for this group on server side list
   */
  virtual void setServerId(unsigned long protocolId, unsigned long serverId) = 0;

protected:
  virtual ~Group() { /* Empty */ }

  int myId;
  std::string myName;
  int mySortIndex;
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
  GroupReadGuard(int groupId);

  // Derived constructors
  GroupReadGuard(const Group* group, bool locked = false)
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
  GroupWriteGuard(int groupId);

  // Derived constructors
  GroupWriteGuard(Group* group, bool locked = false)
    : WriteMutexGuard<Group>(group, locked)
  { }
  GroupWriteGuard(WriteMutexGuard<Group>* guard)
    : WriteMutexGuard<Group>(guard)
  { }
};

} // namespace Licq

#endif // LICQ_CONTACTLIST_GROUP_H
