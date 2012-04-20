/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq Developers <licq-dev@googlegroups.com>
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

#include <licq/daemon.h>

namespace LicqTest {

/**
 * This is a dummy implementation of the daemon class
 * The purpose of this class is to make sure gDaemon is defined so unittest
 * can be run on classes that need to link against it
 */
class DaemonDummy : public Licq::Daemon
{
public:
  DaemonDummy() { }

  // From Licq::Daemon
  void Shutdown() { /* Empty */ }
  const char* Version() const { return NULL; }
  void SaveConf() { }
  bool addUserEvent(Licq::User*, Licq::UserEvent*) { return false; }
  void rejectEvent(const Licq::UserId&, Licq::UserEvent*) { }
};

} // namespace LicqTest

LicqTest::DaemonDummy gDaemonDummy;
Licq::Daemon& Licq::gDaemon(gDaemonDummy);
