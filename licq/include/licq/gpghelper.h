/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2010 Licq developers
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

#ifndef LICQ_GPGHELPER_H
#define LICQ_GPGHELPER_H

#include <boost/noncopyable.hpp>
#include <list>
#include <string>

namespace Licq
{
class UserId;

// Structure for holding a user identity for a key
struct GpgUid
{
  std::string name;
  std::string email;
};

// Structure for holding information about a key
struct GpgKey
{
  // First uid in list is the primary uid
  std::list<GpgUid> uids;
  std::string keyid;
};

class GpgHelper : private boost::noncopyable
{
public:
  static const char pgpSig[];
  virtual char* Decrypt(const char* cipher) = 0;
  virtual char* Encrypt(const char* plain, const UserId& userId) = 0;

  /**
   * Get a list of keys
   * This function allows plugins to present a list of keys for user to select
   * from without making the plugin itself dependant on gpgme.
   *
   * This function only fetches the bare minimum information currently used by
   * the plugins, extended it and the structs above if additional fields are
   * needed.
   *
   * @return A list of keys, must be deleted by caller
   */
  virtual std::list<GpgKey>*  getKeyList() const = 0;

protected:
  virtual ~GpgHelper()
  { /* Empty */ }
};

extern GpgHelper& gGpgHelper;

} // namespace Licq

#endif
