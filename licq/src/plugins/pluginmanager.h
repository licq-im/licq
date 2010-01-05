/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Erik Johansson <erijo@licq.org>
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

#ifndef LICQDAEMON_PLUGINMANAGER_H
#define LICQDAEMON_PLUGINMANAGER_H

#include "generalplugin.h"
#include "plugineventhandler.h"
#include "protocolplugin.h"

#include "licq/pluginmanager.h"
#include "licq/thread/mutex.h"
#include "utils/dynamiclibrary.h"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace LicqDaemon
{

class PluginManager : public Licq::PluginManager
{
public:
  PluginManager();
  ~PluginManager();

  void setDaemon(CICQDaemon* daemon);

  GeneralPlugin::Ptr loadGeneralPlugin(
      const std::string& name, int argc, char** argv, bool keep = true);
  ProtocolPlugin::Ptr loadProtocolPlugin(
      const std::string& name, bool keep = true, bool icq = false);

  /// Start all plugins that have been loaded
  void startAllPlugins();

  /// Send shutdown signal to all the plugins
  void shutdownAllPlugins();

  unsigned short waitForPluginExit(unsigned int timeout = 0);

  void cancelAllPlugins();

  PluginEventHandler& getPluginEventHandler();

  size_t getGeneralPluginsCount() const;

  // From Licq::PluginManager
  void getGeneralPluginsList(Licq::GeneralPluginsList& plugins) const;
  void getProtocolPluginsList(Licq::ProtocolPluginsList& plugins) const;

  bool startGeneralPlugin(const std::string& name, int argc, char** argv);
  bool startProtocolPlugin(const std::string& name);

  int registerGeneralPlugin(unsigned long signalMask);
  void unregisterGeneralPlugin();

  int registerProtocolPlugin();
  void unregisterProtocolPlugin();

private:
  DynamicLibrary::Ptr loadPlugin(
      const std::string& name, const std::string& prefix);
  void startPlugin(Plugin::Ptr plugin);

  CICQDaemon* myDaemon;
  unsigned short myNextPluginId;

  GeneralPluginsList myGeneralPlugins;
  mutable Licq::Mutex myGeneralPluginsMutex;

  ProtocolPluginsList myProtocolPlugins;
  mutable Licq::Mutex myProtocolPluginsMutex;

  PluginEventHandler myPluginEventHandler;
};

inline void PluginManager::setDaemon(CICQDaemon* daemon)
{
  myDaemon = daemon;
}

inline PluginEventHandler& PluginManager::getPluginEventHandler()
{
  return myPluginEventHandler;
}

} // namespace LicqDaemon

#endif
