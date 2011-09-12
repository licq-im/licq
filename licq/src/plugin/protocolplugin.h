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

#ifndef LICQDAEMON_PROTOCOLPLUGIN_H
#define LICQDAEMON_PROTOCOLPLUGIN_H

#include <licq/plugin/protocolplugin.h>
#include "plugin.h"

#include <queue>

#include <licq/plugin/protocolbase.h>
#include <licq/thread/mutex.h>

namespace Licq
{

/**
 * Temporary class used to hold initalization data for ProtocolPlugin constructor
 */
class ProtocolPlugin::Params : public Plugin::Params
{
public:
  Params(int id, LicqDaemon::DynamicLibrary::Ptr lib,
      LicqDaemon::PluginThread::Ptr thread) :
    Plugin::Params(id, lib, thread)
  { /* Empty */ }
};

class ProtocolPlugin::Private
{
public:
  Private();

private:
  std::queue<Licq::ProtocolSignal*> mySignals;
  Licq::Mutex mySignalsMutex;

  friend class ProtocolPlugin;
};

} // namespace Licq

#endif
