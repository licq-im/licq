/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#include "plugin.h"

#include <list>
#include <queue>

namespace Licq
{
class ProtocolSignal;
}

namespace LicqDaemon
{

class ProtocolPlugin : public Plugin,
                       public Licq::ProtocolPlugin
{
public:
  typedef boost::shared_ptr<ProtocolPlugin> Ptr;

  ProtocolPlugin(DynamicLibrary::Ptr lib, PluginThread::Ptr pluginThread,
                 bool icq = false);
  virtual ~ProtocolPlugin();

  bool init();

  void pushSignal(Licq::ProtocolSignal* signal);
  Licq::ProtocolSignal* popSignal();

  // From Licq::ProtocolPlugin
  unsigned long getProtocolId() const;
  unsigned long getSendFunctions() const;

private:
  // From Plugin
  bool initThreadEntry();

  unsigned long myProtocolId;

  std::queue<Licq::ProtocolSignal*> mySignals;
  Licq::Mutex mySignalsMutex;

  bool (*myInit)();
  char* (*myPpid)();
  unsigned long (*mySendFunctions)();
};

typedef std::list<ProtocolPlugin::Ptr> ProtocolPluginsList;

} // namespace LicqDaemon

#endif
