/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQDAEMON_CONTACTLIST_USER_H
#define LICQDAEMON_CONTACTLIST_USER_H

#include <licq/contactlist/user.h>
#include <licq/inifile.h>

#include <boost/any.hpp>
#include <map>
#include <string>

#include "userhistory.h"

namespace Licq
{
typedef std::map<std::string, boost::any> PropertyMap;

class User::Private
{
public:
  Private(User* user, const UserId& id);
  ~Private();

  void writeToHistory(const std::string& text);

  void removeFiles();

  void setPermanent();

  void setDefaults();

  void addToContactList();

  void loadLicqInfo();
  void loadPictureInfo();
  void loadUserInfo();

private:
  /**
   * Initialize all user object. Contains common code for all constructors
   */
  void Init();

  User* const myUser;
  const UserId myId;
  IniFile myConf;
  LicqDaemon::UserHistory myHistory;

  // myUserInfo holds user information like email, address, homepage etc...
  PropertyMap myUserInfo;

  friend class User;
};

} // namespace Licq

#endif
