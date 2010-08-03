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

#ifndef LICQDAEMON_GPGHELPER_H
#define LICQDAEMON_GPGHELPER_H

#include <licq/gpghelper.h>

#include "config.h"

#ifdef HAVE_LIBGPGME
#include <gpgme.h>
#endif

#include <licq/inifile.h>
#include <licq/thread/mutex.h>

namespace LicqDaemon
{

class GpgHelper : public Licq::GpgHelper
{
public:
  GpgHelper();
  ~GpgHelper();
  void Start();

  // From Licq::GpgHelper
  char* Decrypt(const char* cipher);
  char* Encrypt(const char* plain, const Licq::UserId& userId);
  std::list<Licq::GpgKey>* getKeyList() const;

private:
#ifdef HAVE_LIBGPGME
  static gpgme_error_t PassphraseCallback(void* helperPtr, const char *, const char*, int, int);

  gpgme_ctx_t mCtx;
#endif
  std::string myGpgPassphrase;
  Licq::IniFile myKeysIni;
  mutable Licq::Mutex myMutex;
};

extern GpgHelper gGpgHelper;

} // namespace LicqDaemon

#endif
