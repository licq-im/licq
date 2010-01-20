#ifndef LICQ_CONTACTLIST_GROUP_H
#define LICQ_CONTACTLIST_GROUP_H

#include <boost/noncopyable.hpp>
#include <map>
#include <string>

#include "../../licq_mutex.h"

class CIniFile;

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
   * Constructor, creates a new user group
   *
   * @param id Group id, must be unique
   * @param name Group name
   */
  Group(int id, const std::string& name);

  /**
   * Destructor
   */
  virtual ~Group();

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
  unsigned long serverId(unsigned long protocolId) const;

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
  void setServerId(unsigned long protocolId, unsigned long serverId);

  /**
   * Save group to file
   * Note: This function should only be called by UserManager
   *
   * @param file Open file to write group data to
   * @param num Number of group entry to write to file
   */
  void save(CIniFile& file, int num) const;

private:
  int myId;
  std::string myName;
  int mySortIndex;
  std::map<unsigned long, unsigned long> myServerIds;
};

/**
 * Helper function for sorting group list
 *
 * @param first Left hand group to compare
 * @param second Right hand group to compare
 * @return True if first has a lower sorting index than second
 */
bool compare_groups(const Group* first, const Group* second);

} // namespace Licq

#endif // LICQ_CONTACTLIST_GROUP_H
