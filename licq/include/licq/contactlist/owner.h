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

#ifndef LICQ_CONTACTLIST_OWNER_H
#define LICQ_CONTACTLIST_OWNER_H

#include "user.h"

namespace Licq
{

/**
 * A protocol account including all user information for that account
 *
 * Inherits LicqUser to hold all user information associated with the account.
 */
class Owner : public virtual User
{
public:
  // Owner specific functions
  const std::string& password() const           { return myPassword; }
  void setPassword(const std::string& s)        { myPassword = s; save(SaveOwnerInfo); }
  void SetSavePassword(bool b) {  m_bSavePassword = b; save(SaveOwnerInfo); }
  bool SavePassword() const                     { return m_bSavePassword; }

  /**
   * Get status to change to at startup
   */
  unsigned startupStatus() const
  { return myStartupStatus; }

  /**
   * Set status to change to at startup
   */
  void setStartupStatus(unsigned status)
  { myStartupStatus = status; }

  /// Get server to connect to
  const std::string& serverHost() const         { return myServerHost; }

  /// Get server port to connect to
  int serverPort() const                        { return myServerPort; }

  /**
   * Set server to use when connecting
   *
   * @param host Host to connect to
   * @param port Port to connect to
   */
  void setServer(const std::string& host, int port)
  { myServerHost = host; myServerPort = port; save(SaveOwnerInfo); }

  void SetPicture(const char *f);

protected:
  /// Constructor
  Owner(const UserId& id);

  /// Destructor
  virtual ~Owner();

  virtual void saveOwnerInfo();

  std::string myPassword;
  unsigned myStartupStatus;
  std::string myServerHost;
  int myServerPort;
  bool m_bSavePassword;

private:

  // Allow the user manager to access private members
  friend class LicqDaemon::UserManager;
};

/**
 * Read mutex guard for Licq::Owner
 */
class OwnerReadGuard : public ReadMutexGuard<Owner>
{
public:
  /**
   * Constructor, will fetch and lock an owner based on user id
   * Note: Always check that the owner was actually fetched before using
   *
   * @param userId Id of owner to fetch
   */
  OwnerReadGuard(const UserId& userId);

  // Derived constructors
  OwnerReadGuard(const Owner* owner, bool locked = false)
    : ReadMutexGuard<Owner>(owner, locked)
  { }
  OwnerReadGuard(ReadMutexGuard<Owner>* guard)
    : ReadMutexGuard<Owner>(guard)
  { }
};

/**
 * Write mutex guard for Licq::Owner
 */
class OwnerWriteGuard : public WriteMutexGuard<Owner>
{
public:
  /**
   * Constructor, will fetch and lock an owner based on user id
   * Note: Always check that the owner was actually fetched before using
   *
   * @param userId Id of owner to fetch
   */
  OwnerWriteGuard(const UserId& userId);

  // Derived constructors
  OwnerWriteGuard(Owner* owner, bool locked = false)
    : WriteMutexGuard<Owner>(owner, locked)
  { }
  OwnerWriteGuard(WriteMutexGuard<Owner>* guard)
    : WriteMutexGuard<Owner>(guard)
  { }
};

} // namespace Licq

#endif // LICQ_CONTACTLIST_OWNER_H
