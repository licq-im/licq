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

#ifndef LICQDAEMON_GENERALPLUGIN_H
#define LICQDAEMON_GENERALPLUGIN_H

#include "plugin.h"

#include <list>
#include <queue>

namespace Licq
{
class Event;
class PluginSignal;
}

namespace LicqDaemon
{

class GeneralPlugin : public Plugin,
                      public Licq::GeneralPlugin
{
public:
  typedef boost::shared_ptr<GeneralPlugin> Ptr;

  GeneralPlugin(DynamicLibrary::Ptr lib, PluginThread::Ptr pluginThread);
  virtual ~GeneralPlugin();

  bool init(int argc, char** argv);

  void pushSignal(Licq::PluginSignal* signal);
  Licq::PluginSignal* popSignal();

  void pushEvent(Licq::Event* event);
  Licq::Event* popEvent();

  // From Licq::GeneralPlugin
  const char* getStatus() const;
  const char* getDescription() const;
  const char* getUsage() const;
  const char* getConfigFile() const;
  const char* getBuildDate() const;
  const char* getBuildTime() const;
  void enable();
  void disable();

private:
  // From Plugin
  bool initThreadEntry();

  int myArgc;
  char** myArgv;
  char** myArgvCopy;

  std::queue<Licq::PluginSignal*> mySignals;
  Licq::Mutex mySignalsMutex;

  std::queue<Licq::Event*> myEvents;
  Licq::Mutex myEventsMutex;

  // Function pointers
  bool (*myInit)(int, char**);
  const char* (*myStatus)();
  const char* (*myDescription)();
  const char* (*myUsage)();
  const char* (*myConfigFile)();
  char* (*myBuildDate)();
  char* (*myBuildTime)();
};

typedef std::list<GeneralPlugin::Ptr> GeneralPluginsList;

} // namespace LicqDaemon

#endif
