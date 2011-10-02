/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq developers
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

namespace LicqDaemon
{
typedef std::map<std::string, boost::any> PropertyMap;

class User : public virtual Licq::User
{
public:
  static const char* const ConfigDir;
  static const char* const HistoryDir;
  static const char* const HistoryExt;
  static const char* const HistoryOldExt;

  /**
   * Constructor to create a user object for a new contact
   *
   * @param id User id
   * @param temporary False if user is added permanently to list
   * @param isOwner True if this is an owner
   */
  User(const Licq::UserId& id, bool temporary = false, bool isOwner = false);

  ~User();

  void writeToHistory(const std::string& text);

  // From Licq::User
  void save(unsigned group);
  void RemoveFiles();
  std::string getUserInfoString(const std::string& key) const;
  unsigned int getUserInfoUint(const std::string& key) const;
  bool getUserInfoBool(const std::string& key) const;
  void setUserInfoString(const std::string& key, const std::string& value);
  void setUserInfoUint(const std::string& key, unsigned int value);
  void setUserInfoBool(const std::string& key, bool value);
  void SetPermanent();
  int GetHistory(Licq::HistoryList& history) const;
  const std::string& historyName() const;
  const std::string& historyFile() const;
  void SetDefaults();
  void AddToContactList();

protected:
  virtual void saveLicqInfo();
  virtual void saveUserInfo();
  virtual void saveOwnerInfo();
  virtual void saveNewMessagesInfo();
  virtual void savePictureInfo();

  void setHistoryFile(const std::string& file);
  void LoadLicqInfo();
  void LoadPhoneBookInfo();
  void LoadPictureInfo();

  Licq::IniFile myConf;

private:
  void loadUserInfo();

  /**
   * Save a category list
   *
   * @param category The category map to save
   * @param key Base name of key in file for entries
   */
  void saveCategory(const Licq::UserCategoryMap& category, const std::string& key);

  /**
   * Load a category list
   *
   * @param category The category map to save
   * @param key Base name of key in file for entries
   */
  void loadCategory(Licq::UserCategoryMap& category, const std::string& key);

  /**
   * Initialize all user object. Contains common code for all constructors
   */
  void Init();

  UserHistory myHistory;

  // myUserInfo holds user information like email, address, homepage etc...
  PropertyMap myUserInfo;

};

} // namespace LicqDaemon

#endif
