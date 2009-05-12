/*
 * This header holds common types used through out Licq
 * Headers that only need to know of the types can include this header instead
 * of having a dependancy on the larger class headers.
 */
#ifndef LICQ_TYPES_H
#define LICQ_TYPES_H

#include <map>
#include <set>
#include <string>

// Forward declarations of commonly used classes
class LicqUser;
class LicqOwner;
class LicqGroup;


// Define a type for user id so other code doesn't have to hardcode the real type everywhere
typedef std::string UserId;
// Test for a valid user id
#define USERID_ISVALID(x) ((x).size() > 4)
// Value to use in assignment to get an undefined (invalid) user id
#define USERID_NONE ("")
// Get a printable string, for use in log printouts etc
#define USERID_TOSTR(x) ((x).c_str())


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


// Types used for contact list
typedef std::map<UserId, class LicqUser*> UserMap;
typedef std::map<unsigned long, class LicqOwner*> OwnerMap;
typedef std::set<int> UserGroupList;
typedef std::map<int, LicqGroup*> GroupMap;
typedef std::map<unsigned int, std::string> UserCategoryMap;

#endif
