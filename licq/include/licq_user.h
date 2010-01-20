#ifndef USER_H
#define USER_H

#include "licq_types.h"
#include "licq/contactlist/group.h"
#include "licq/contactlist/owner.h"
#include "licq/contactlist/user.h"
#include "licq/contactlist/usermanager.h"

// Added for plugin convenience
#include "licq_constants.h"


// Make Licq::User constants and structs visible to old code that needs them
using Licq::GENDER_UNSPECIFIED;
using Licq::GENDER_FEMALE;
using Licq::GENDER_MALE;
using Licq::AGE_UNSPECIFIED;
using Licq::TIMEZONE_UNKNOWN;
using Licq::SECURE_CHANNEL_NOTSUPPORTED;
using Licq::SECURE_CHANNEL_SUPPORTED;
using Licq::USPRINTF_NTORN;
using Licq::USPRINTF_NOFW;
using Licq::USPRINTF_LINEISCMD;
using Licq::USPRINTF_PIPEISCMD;
using Licq::MAX_CATEGORY_SIZE;
using Licq::MAX_CATEGORIES;
using Licq::CAT_INTERESTS;
using Licq::CAT_ORGANIZATION;
using Licq::CAT_BACKGROUND;
using Licq::UserCat;
using Licq::PhoneBookEntry;
using Licq::MAX_DESCRIPTION_SIZE;
using Licq::MAX_AREAxCODE_SIZE;
using Licq::MAX_PHONExNUMBER_SIZE;
using Licq::MAX_EXTENSION_SIZE;
using Licq::MAX_GATEWAY_SIZE;
using Licq::MAX_PICTURE_SIZE;
using Licq::TYPE_PHONE;
using Licq::TYPE_CELLULAR;
using Licq::TYPE_CELLULARxSMS;
using Licq::TYPE_FAX;
using Licq::TYPE_PAGER;
using Licq::GATEWAY_BUILTIN;
using Licq::GATEWAY_CUSTOM;
using Licq::PUBLISH_DISABLE;
using Licq::ICQUserPhoneBook;

// Make Licq::UserManager visible to old code
using Licq::gUserManager;
typedef Licq::UserReadGuard LicqUserReadGuard;
typedef Licq::UserWriteGuard LicqUserWriteGuard;
typedef Licq::GroupReadGuard LicqGroupReadGuard;
typedef Licq::GroupWriteGuard LicqGroupWriteGuard;
typedef Licq::UserManager CUserManager;
using Licq::NUM_GROUPS_SYSTEM;
using Licq::GroupsSystemNames;


// Returned value must be cleared with delete[]
extern char* PPIDSTRING(unsigned long ppid);


// Temporary until all occurenses of deprecated names ICQUser ICQOwner have been removed
typedef LicqUser ICQUser;
typedef LicqOwner ICQOwner;

#endif
