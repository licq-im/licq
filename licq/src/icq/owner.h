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

#ifndef LICQICQ_OWNER_H
#define LICQICQ_OWNER_H

#include <licq/icq/owner.h>
#include "user.h"

#include <ctime>


namespace LicqIcq
{

/**
 * An ICQ protocol account
 */
class Owner : public Licq::IcqOwner, public User
{
public:
  /// Constructor
  Owner(const Licq::UserId& id);

  /// Destructor
  virtual ~Owner();

  /// Set current random chat group, use IcqProtocol::setRandomChatGroup() from plugins
  void setRandomChatGroup(unsigned n)           { myRandomChatGroup = n; save(SaveOwnerInfo); }

  // Server Side List functions
  time_t GetSSTime() const                      { return m_nSSTime; }
  void SetSSTime(time_t t)                      { m_nSSTime = t; }
  unsigned short GetSSCount() const             { return mySsCount; }
  void SetSSCount(unsigned short n)             { mySsCount = n; }
  unsigned short GetPDINFO() const              { return myPDINFO; }
  void SetPDINFO(unsigned short n)              { myPDINFO = n; save(SaveOwnerInfo); }

  bool useBart() const                          { return myUseBart; }
  void setUseBart(bool b)                       { myUseBart = b; }

private:
  virtual void saveOwnerInfo();

  bool m_bWebAware;
  unsigned mySsCount;
  time_t m_nSSTime;
  unsigned myPDINFO;
  bool myUseBart;
};

/**
 * Read mutex guard for LicqIcq::Owner
 */
class OwnerReadGuard : public Licq::OwnerReadGuard
{
public:
  // Derived costructors
  OwnerReadGuard(const Licq::UserId& userId)
    : Licq::OwnerReadGuard(userId)
  { }
  OwnerReadGuard()
    : Licq::OwnerReadGuard(LICQ_PPID)
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
 * Write mutex guard for LicqIcq::Owner
 */
class OwnerWriteGuard : public Licq::OwnerWriteGuard
{
public:
  // Derived costructors
  OwnerWriteGuard(const Licq::UserId& userId)
    : Licq::OwnerWriteGuard(userId)
  { }
  OwnerWriteGuard()
    : Licq::OwnerWriteGuard(LICQ_PPID)
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

} // namespace LicqIcq

#endif
