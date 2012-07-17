/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQDAEMON_DAEMON_H
#define LICQDAEMON_DAEMON_H

#include <licq/daemon.h>

#include <licq/inifile.h>
#include <licq/thread/mutex.h>

class CLicq;

namespace LicqDaemon
{

class Daemon : public Licq::Daemon
{
public:
  static const char* const TranslationDir;
  static const char* const UtilityDir;

  Daemon();
  ~Daemon();

  void preInitialize(CLicq* _licq) { licq = _licq; }

  /**
   * Initialize the daemon
   */
  void initialize();

  /**
   * Set dir variable
   * Only called once during startup
   */
  void setBaseDir(const std::string& baseDir);

  /**
   * Set startup status for all protocols according to owner data
   * Only called once during startup
   */
  void autoLogon();

  /**
   * Get access to main config file
   * Caller must use unlock mutex by calling releaseLicqConf() when done
   *
   * @return Reference to licq.conf
   */
  Licq::IniFile& getLicqConf()
  { myLicqConfMutex.lock(); return myLicqConf; }

  /**
   * Unlock mutex for main config
   */
  void releaseLicqConf()
  { myLicqConfMutex.unlock(); }

  /// Notify main thread that a plugin has exited
  void notifyPluginExited();

  // From Licq::Daemon
  void Shutdown();
  const char* Version() const;
  void SaveConf();
  bool addUserEvent(Licq::User* u, Licq::UserEvent* e);
  void rejectEvent(const Licq::UserId& userId, Licq::UserEvent* e);

private:
  std::string myRejectFile;
  unsigned myErrorTypes;
  std::string myErrorFile;

  pthread_t thread_shutdown;

  Licq::IniFile myLicqConf;
  Licq::Mutex myLicqConfMutex;

  CLicq* licq;
};

extern Daemon gDaemon;

} // namespace LicqDaemon

#endif
