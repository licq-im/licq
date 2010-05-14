/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#include "userhistory.h"

namespace LicqDaemon
{

class User : public virtual Licq::User
{
public:
  /**
   * Constructor to create a user object for an existing contact
   *
   * @param id User id
   * @param filename Filename to read user data from
   */
  User(const Licq::UserId& id, const std::string& filename);

  /**
   * Constructor to create a user object for a new contact
   *
   * @param id User id
   * @param temporary False if user is added permanently to list
   */
  User(const Licq::UserId& id, bool temporary = false);

  ~User();

  // From Licq::User
  void RemoveFiles();
  void WriteToHistory(const char*);
  void SetHistoryFile(const char*);
  int GetHistory(Licq::HistoryList& history) const;
  void SaveHistory(const char* buf);
  const char* HistoryName() const;
  const char* HistoryFile() const;
  void AddToContactList();

private:
  UserHistory myHistory;
};

} // namespace LicqDaemon

#endif
