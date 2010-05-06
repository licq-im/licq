/*
 * This header exists only to support old code and will be removed in the
 * future. New code should include licq/types.h directly.
 */
#ifndef LICQ_TYPES_COMPAT_H
#define LICQ_TYPES_COMPAT_H

#include "licq/types.h"
#include "licq/userid.h"

typedef Licq::Group LicqGroup;
typedef Licq::Owner LicqOwner;
typedef Licq::User LicqUser;

using Licq::UserId;
#define USERID_ISVALID(x) ((x).isValid())
#define USERID_NONE Licq::UserId()
#define USERID_TOSTR(x) ((x).toString().c_str())

typedef Licq::UserMap UserMap;
typedef Licq::OwnerMap OwnerMap;
typedef Licq::UserGroupList UserGroupList;
typedef Licq::GroupMap GroupMap;
typedef Licq::UserCategoryMap UserCategoryMap;

#endif
