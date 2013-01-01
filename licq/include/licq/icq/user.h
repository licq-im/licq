/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012-2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_ICQ_USER_H
#define LICQ_ICQ_USER_H

#include "../contactlist/user.h"

#include <map>
#include <string>
#include <vector>

namespace LicqIcq
{
class User;
}

namespace Licq
{

enum IcqPluginStatus
{
  IcqPluginInactive = 0,
  IcqPluginActive = 1,
  IcqPluginBusy = 2,
};

enum IcqRandomChatGroups
{
  IcqRandomChatGroupNone        = 0,
  IcqRandomChatGroupGeneral     = 1,
  IcqRandomChatGroupRomance     = 2,
  IcqRandomChatGroupGames       = 3,
  IcqRandomChatGroupStudents    = 4,
  IcqRandomChatGroup20Some      = 6,
  IcqRandomChatGroup30Some      = 7,
  IcqRandomChatGroup40Some      = 8,
  IcqRandomChatGroup50Plus      = 9,
  IcqRandomChatGroupSeekF       = 10,
  IcqRandomChatGroupSeekM       = 11,
};

const unsigned short MAX_CATEGORY_SIZE  = 60;
const unsigned int MAX_CATEGORIES = 4;

typedef enum
{
  CAT_INTERESTS         = 0,
  CAT_ORGANIZATION      = 1,
  CAT_BACKGROUND        = 2,
  CAT_MAX               = 3,
} UserCat;

struct PhoneBookEntry
{
  std::string description;
  std::string areaCode;
  std::string phoneNumber;
  std::string extension;
  std::string country;
  unsigned long nActive;
  unsigned long nType;
  std::string gateway;
  unsigned long nGatewayType;
  unsigned long nSmsAvailable;
  unsigned long nRemoveLeading0s;
  unsigned long nPublish;
};

const unsigned short MAX_DESCRIPTION_SIZE  = 16;
const unsigned short MAX_AREAxCODE_SIZE    =  5;
const unsigned short MAX_PHONExNUMBER_SIZE = 16;
const unsigned short MAX_EXTENSION_SIZE    = 20;
const unsigned short MAX_GATEWAY_SIZE      = 64;
const unsigned short MAX_PICTURE_SIZE      = 8081;

enum EPhoneType
{
  TYPE_PHONE            = 0,
  TYPE_CELLULAR         = 1,
  TYPE_CELLULARxSMS     = 2,
  TYPE_FAX              = 3,
  TYPE_PAGER            = 4,
  TYPE_MAX              = 5
};

enum EGatewayType
{
  GATEWAY_BUILTIN = 1,
  GATEWAY_CUSTOM = 2,
};

enum EPublish
{
  PUBLISH_ENABLE = 1,
  PUBLISH_DISABLE = 2,
};


typedef std::vector<struct PhoneBookEntry> IcqPhoneBookVector;

typedef std::map<unsigned int, std::string> UserCategoryMap;

/**
 * An ICQ protocol contact
 */
class IcqUser : public virtual User
{
public:
  //!Retrieves the user's interests
  UserCategoryMap& getInterests()               { return myInterests; }
  const UserCategoryMap& getInterests() const   { return myInterests; }
  //!Retrieves the user's backgrounds
  UserCategoryMap& getBackgrounds()             { return myBackgrounds; }
  const UserCategoryMap& getBackgrounds() const { return myBackgrounds; }
  //!Retrieves the user's organizations
  UserCategoryMap& getOrganizations()           { return myOrganizations; }
  const UserCategoryMap& getOrganizations() const { return myOrganizations; }

  //!Retrives the user's phone book
  IcqPhoneBookVector& getPhoneBook()              { return myPhoneBook; }
  const IcqPhoneBookVector& getPhoneBook() const  { return myPhoneBook; }

  unsigned phoneFollowMeStatus() const          { return myPhoneFollowMeStatus; }
  unsigned icqPhoneStatus() const               { return myIcqPhoneStatus; }
  unsigned sharedFilesStatus() const            { return mySharedFilesStatus; }

protected:
  /// Constructor
  IcqUser(const UserId& id, bool temporary = false);

  /// Destructor
  virtual ~IcqUser();

  unsigned myPhoneFollowMeStatus;
  unsigned myIcqPhoneStatus;
  unsigned mySharedFilesStatus;

  UserCategoryMap myInterests;
  UserCategoryMap myBackgrounds;
  UserCategoryMap myOrganizations;
  IcqPhoneBookVector myPhoneBook;
};


/**
 * Read mutex guard for Licq::IcqUser
 */
class IcqUserReadGuard : public UserReadGuard
{
public:
  // Derived costructors
  IcqUserReadGuard(const UserId& userId, bool addUser = false, bool* retWasAdded = NULL)
    : UserReadGuard(userId, addUser, retWasAdded)
  { }
  IcqUserReadGuard(const IcqUser* user, bool locked = false)
    : UserReadGuard(user, locked)
  { }
  IcqUserReadGuard(IcqUserReadGuard* guard)
    : UserReadGuard(guard)
  { }

  // Access operators
  const IcqUser* operator*() const
  { return dynamic_cast<const IcqUser*>(UserReadGuard::operator*()); }
  const IcqUser* operator->() const
  { return dynamic_cast<const IcqUser*>(UserReadGuard::operator->()); }
};

/**
 * Write mutex guard for Licq::IcqUser
 */
class IcqUserWriteGuard : public UserWriteGuard
{
public:
  // Derived costructors
  IcqUserWriteGuard(const UserId& userId, bool addUser = false, bool* retWasAdded = NULL)
    : UserWriteGuard(userId, addUser, retWasAdded)
  { }
  IcqUserWriteGuard(IcqUser* user, bool locked = false)
    : UserWriteGuard(user, locked)
  { }
  IcqUserWriteGuard(IcqUserWriteGuard* guard)
    : UserWriteGuard(guard)
  { }

  // Access operators
  IcqUser* operator*() const
  { return dynamic_cast<IcqUser*>(UserWriteGuard::operator*()); }
  IcqUser* operator->() const
  { return dynamic_cast<IcqUser*>(UserWriteGuard::operator->()); }
};

} // namespace Licq

#endif
