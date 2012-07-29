/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQMSN_USER_H
#define LICQMSN_USER_H

#include <licq/contactlist/user.h>

#include <licq/socket.h>

namespace LicqMsn
{

/**
 * An MSN protocol contact
 */
class User : public virtual Licq::User
{
public:
  /// Constructor
  User(const Licq::UserId& id, bool temporary = false, bool isOwner = false);

  /// Destructor
  virtual ~User();

  /// Get object id of current picture
  const std::string& pictureObject() const
  { return myPictureObject; }

  /// Set object id of current picture
  void setPictureObject(const std::string& s)
  { myPictureObject = s; save(SavePictureInfo); }

  void setNormalSocketDesc(Licq::INetSocket* s) { myNormalSocketDesc = s->Descriptor(); }
  void setInfoSocketDesc(Licq::INetSocket* s) { myInfoSocketDesc = s->Descriptor(); }
  void clearSocketDesc(Licq::INetSocket* s);
  void clearAllSocketDesc() { clearSocketDesc(NULL); }
  int normalSocketDesc() const { return myNormalSocketDesc; }
  void clearNormalSocketDesc() { myNormalSocketDesc = -1; }

private:
  /// Inherited from Licq::User to save local additions
  virtual void savePictureInfo();

  std::string myPictureObject;

  int myNormalSocketDesc;
  int myInfoSocketDesc;
};


/**
 * Read mutex guard for LicqMsn::User
 */
class UserReadGuard : public Licq::UserReadGuard
{
public:
  // Derived costructors
  UserReadGuard(const Licq::UserId& userId, bool addUser = false, bool* retWasAdded = NULL)
    : Licq::UserReadGuard(userId, addUser, retWasAdded)
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
 * Write mutex guard for LicqMsn::User
 */
class UserWriteGuard : public Licq::UserWriteGuard
{
public:
  // Derived costructors
  UserWriteGuard(const Licq::UserId& userId, bool addUser = false, bool* retWasAdded = NULL)
    : Licq::UserWriteGuard(userId, addUser, retWasAdded)
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

} // namespace LicqMsn

#endif
