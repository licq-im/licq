/*
 * This header holds common types used through out Licq
 * Headers that only need to know of the types can include this header instead
 * of having a dependancy on the larger class headers.
 */
#ifndef LICQ_TYPES_H
#define LICQ_TYPES_H

#include <list>
#include <map>
#include <string>

namespace Licq
{

// Declare a convenient name for the list of strings
typedef std::list<std::string> StringList;

// Types used for contact list
typedef std::map<unsigned int, std::string> UserCategoryMap;

} // namespace Licq

#endif
