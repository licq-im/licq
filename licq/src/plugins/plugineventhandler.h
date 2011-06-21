/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq developers
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

#ifndef LICQDAEMON_PLUGINEVENTHANDLER_H
#define LICQDAEMON_PLUGINEVENTHANDLER_H

#include <boost/noncopyable.hpp>

#include <licq/generalplugin.h>
#include <licq/protocolplugin.h>
#include <licq/thread/mutex.h>

namespace Licq
{
class PluginSignal;
class ProtocolSignal;
}

namespace LicqDaemon
{

class PluginEventHandler : private boost::noncopyable
{
public:
  PluginEventHandler(Licq::GeneralPluginsList& generalPlugins,
                     Licq::Mutex& generalPluginsMutex,
                     Licq::ProtocolPluginsList& protocolPlugins,
                     Licq::Mutex& protocolPluginsMutex);
  ~PluginEventHandler();

  void pushGeneralEvent(Licq::Event* event);

  void pushGeneralSignal(Licq::PluginSignal* signal);

  void pushProtocolSignal(Licq::ProtocolSignal* signal, unsigned long ppid);

private:
  Licq::GeneralPluginsList& myGeneralPlugins;
  Licq::Mutex& myGeneralPluginsMutex;

  Licq::ProtocolPluginsList& myProtocolPlugins;
  Licq::Mutex& myProtocolPluginsMutex;
};

} // namespace LicqDaemon

#endif
