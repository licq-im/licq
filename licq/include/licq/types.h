/*
 * This header holds common types used through out Licq
 * Headers that only need to know of the types can include this header instead
 * of having a dependancy on the larger class headers.
 */
#ifndef LICQ_TYPES_H
#define LICQ_TYPES_H

#include <list>
#include <map>
#include <set>
#include <string>

namespace Licq
{
// Forward declarations of commonly used classes
class Group;
class Owner;
class User;
class UserId;

// Declare a convenient name for the list of strings
typedef std::list<std::string> StringList;

// Convenience function to convert protocolId to a string
// ret must be able to hold at least 5 characters
inline char* protocolId_toStr(char* ret, unsigned long protocolId)
{
  ret[0] = ((protocolId & 0xFF000000) >> 24);
  ret[1] = ((protocolId & 0x00FF0000) >> 16);
  ret[2] = ((protocolId & 0x0000FF00) >> 8);
  ret[3] = ((protocolId & 0x000000FF));
  ret[4] = '\0';
  return ret;
}

// Group types used in contact list
enum GroupType { GROUPS_SYSTEM, GROUPS_USER };

// System groups
const int GROUP_ALL_USERS       = 0;
const int GROUP_ONLINE_NOTIFY   = 1;
const int GROUP_VISIBLE_LIST    = 2;
const int GROUP_INVISIBLE_LIST  = 3;
const int GROUP_IGNORE_LIST     = 4;
const int GROUP_NEW_USERS       = 5;

// The amount of registered system groups
const int NUM_GROUPS_SYSTEM_ALL = 6;

#define LICQ_PPID 0x4C696371  // "Licq"

// Types used for contact list
typedef std::map<UserId, class Licq::User*> UserMap;
typedef std::map<unsigned long, class Licq::Owner*> OwnerMap;
typedef std::set<int> UserGroupList;
typedef std::map<int, Licq::Group*> GroupMap;
typedef std::map<unsigned int, std::string> UserCategoryMap;

} // namespace Licq

#endif
