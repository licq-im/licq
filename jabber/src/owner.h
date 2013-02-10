/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012-2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQJABBER_OWNER_H
#define LICQJABBER_OWNER_H

#include "user.h"

#include <licq/contactlist/owner.h>

#include <gloox/gloox.h>


namespace LicqJabber
{

class Owner : public Licq::Owner, public User
{
public:
 /// Constructor
  Owner(const Licq::UserId& id);

  /// Destructor
  virtual ~Owner();

  // Network settings
  gloox::TLSPolicy tlsPolicy() const { return myTlsPolicy; }
  const std::string& resource() const { return myResource; }

private:
  /// Inherited from Licq::Owner to save local additions
  virtual void saveOwnerInfo();

  gloox::TLSPolicy myTlsPolicy;
  std::string myResource;
};

/**
 * Read mutex guard for LicqJabber::Owner
 */
class OwnerReadGuard : public Licq::OwnerReadGuard
{
public:
  // Derived costructors
  OwnerReadGuard(const Licq::UserId& userId)
    : Licq::OwnerReadGuard(userId)
  { }
  OwnerReadGuard(const Owner* owner, bool locked = false)
    : Licq::OwnerReadGuard(owner, locked)
  { }
  OwnerReadGuard(OwnerReadGuard* guard)
    : Licq::OwnerReadGuard(guard)
  { }

  // Access operators
  const Owner* operator*() const
  { return dynamic_cast<const Owner*>(Licq::OwnerReadGuard::operator*()); }
  const Owner* operator->() const
  { return dynamic_cast<const Owner*>(Licq::OwnerReadGuard::operator->()); }
};

/**
 * Write mutex guard for LicqJabber::Owner
 */
class OwnerWriteGuard : public Licq::OwnerWriteGuard
{
public:
  // Derived costructors
  OwnerWriteGuard(const Licq::UserId& userId)
    : Licq::OwnerWriteGuard(userId)
  { }
  OwnerWriteGuard(Owner* owner, bool locked = false)
    : Licq::OwnerWriteGuard(owner, locked)
  { }
  OwnerWriteGuard(OwnerWriteGuard* guard)
    : Licq::OwnerWriteGuard(guard)
  { }

  // Access operators
  Owner* operator*() const
  { return dynamic_cast<Owner*>(Licq::OwnerWriteGuard::operator*()); }
  Owner* operator->() const
  { return dynamic_cast<Owner*>(Licq::OwnerWriteGuard::operator->()); }
};

} // namespace LicqJabber

#endif
