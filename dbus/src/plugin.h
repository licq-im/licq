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

#ifndef LICQDBUS_PLUGIN_H
#define LICQDBUS_PLUGIN_H

#include <licq/plugin/generalpluginhelper.h>

#include <licq/mainloop.h>

#include "dbusinterface.h"

namespace Licq
{
class Event;
class PluginSignal;
class UserId;
}

namespace LicqDbus
{
class DbusInterface;

class Plugin : public Licq::GeneralPluginHelper, public Licq::MainLoopCallback, public DbusCallback
{
public:
  Plugin();
  ~Plugin();

  // From Licq::PluginInterface
  int run();

  // From Licq::GeneralPluginInterface
  bool isEnabled() const;

  // From Licq::MainLoopCallback
  void rawFileEvent(int id, int fd, int revents);
  void timeoutEvent(int id);

  // From DbusCallback
  void dbusConnected();
  int dbusMethod(const char* path, const char* iface, const char* member,
      DBusMessage* msgref, DBusMessageIter* argref, const char* fmt);

private:
  void processSignal(const Licq::PluginSignal* sig);
  std::string protocolIdToString(unsigned long protocolId);
  std::string userIdToObjectPath(const Licq::UserId& userId);
  Licq::UserId objectPathToUserId(const std::string& object);

  DbusInterface* myConn;
  Licq::MainLoop myMainLoop;
};


} // namespace LicqDbus

#endif
