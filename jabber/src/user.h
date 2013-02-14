/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2013 Licq developers <licq-dev@googlegroups.com>
 *
 * Please refer to the COPYRIGHT file distributed with this source
 * distribution for the names of the individual contributors.
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

#ifndef LICQJABBER_USER_H
#define LICQJABBER_USER_H

#include <licq/contactlist/user.h>

#include <gloox/gloox.h>


namespace LicqJabber
{

class User : public virtual Licq::User
{
public:
 /// Constructor
  User(const Licq::UserId& id, bool temporary);

  /// Destructor
  virtual ~User();

  void setPictureSha1(const std::string sha1) { myPictureSha1 = sha1; }
  const std::string& pictureSha1() const { return myPictureSha1; }

private:
  /// Inherited from Licq::User to save local additions
  virtual void savePictureInfo();

  std::string myPictureSha1;
};

/**
 * Read mutex guard for LicqJabber::User
 */
class UserReadGuard : public Licq::UserReadGuard
{
public:
  // Derived costructors
  UserReadGuard(const Licq::UserId& userId)
    : Licq::UserReadGuard(userId)
  { }
  UserReadGuard(const User* user, bool locked = false)
    : Licq::UserReadGuard(user, locked)
  { }
  UserReadGuard(UserReadGuard* guard)
    : Licq::UserReadGuard(guard)
  { }

  // Access operators
  const User* operator*() const
  { return dynamic_cast<const User*>(Licq::UserReadGuard::operator*()); }
  const User* operator->() const
  { return dynamic_cast<const User*>(Licq::UserReadGuard::operator->()); }
};

/**
 * Write mutex guard for LicqJabber::User
 */
class UserWriteGuard : public Licq::UserWriteGuard
{
public:
  // Derived costructors
  UserWriteGuard(const Licq::UserId& userId)
    : Licq::UserWriteGuard(userId)
  { }
  UserWriteGuard(User* user, bool locked = false)
    : Licq::UserWriteGuard(user, locked)
  { }
  UserWriteGuard(UserWriteGuard* guard)
    : Licq::UserWriteGuard(guard)
  { }

  // Access operators
  User* operator*() const
  { return dynamic_cast<User*>(Licq::UserWriteGuard::operator*()); }
  User* operator->() const
  { return dynamic_cast<User*>(Licq::UserWriteGuard::operator->()); }
};

} // namespace LicqJabber

#endif
