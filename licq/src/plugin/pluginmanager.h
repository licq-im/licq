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

#ifndef LICQDAEMON_PLUGINMANAGER_H
#define LICQDAEMON_PLUGINMANAGER_H

#include <licq/plugin/pluginmanager.h>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <queue>

#include <licq/plugin/generalplugin.h>
#include <licq/plugin/protocolplugin.h>
#include <licq/thread/condition.h>
#include <licq/thread/mutex.h>

#include "../utils/dynamiclibrary.h"
#include "pluginthread.h"

namespace LicqDaemon
{

class PluginManager : public Licq::PluginManager
{
public:
  static const unsigned int MAX_WAIT_PLUGIN = 10;
  static const unsigned short DAEMON_ID = 0;

  PluginManager();
  ~PluginManager();

  void setGuiThread(PluginThread::Ptr guiThread) { myGuiThread = guiThread; }

  Licq::GeneralPlugin::Ptr loadGeneralPlugin(
      const std::string& name, int argc, char** argv, bool keep = true);
  Licq::ProtocolPlugin::Ptr loadProtocolPlugin(
      const std::string& name, bool keep = true, bool icq = false);

  /// Start all plugins that have been loaded
  void startAllPlugins();

  /// Send shutdown signal to all the plugins
  void shutdownAllPlugins();

  /// Notify the manager that a plugin has exited
  void pluginHasExited(unsigned short id);

  /**
   * Wait for a plugin to exit.
   *
   * @param timeout If a plugin has not exited in @a timeout seconds, a
   *        Licq::Exception is thrown.
   * @throw Licq::Exception if there are no plugins to wait for or if the
   *        timeout expires before a plugin exits.
   * @return The id of the plugin that exited, or DaemonId for the daemon.
   */
  unsigned short waitForPluginExit(unsigned int timeout = 0);

  /// Cancel all plugins' threads.
  void cancelAllPlugins();

  size_t getGeneralPluginsCount() const;

  // From Licq::PluginManager
  void getGeneralPluginsList(Licq::GeneralPluginsList& plugins) const;
  void getProtocolPluginsList(Licq::ProtocolPluginsList& plugins) const;
  void getAvailableGeneralPlugins(Licq::StringList& plugins,
                                  bool includeLoaded = true) const;
  void getAvailableProtocolPlugins(Licq::StringList& plugins,
                                   bool includeLoaded = true) const;
  Licq::ProtocolPlugin::Ptr getProtocolPlugin(unsigned long protocolId) const;

  bool startGeneralPlugin(const std::string& name, int argc, char** argv);
  bool startProtocolPlugin(const std::string& name);
  void pushPluginEvent(Licq::Event* event);
  void pushPluginSignal(Licq::PluginSignal* signal);
  void pushProtocolSignal(Licq::ProtocolSignal* signal, unsigned long protocolId);

private:
  /// Helper function to delete a general plugin and close library in the correct order
  static void deleteGeneralPlugin(Licq::GeneralPlugin* plugin);

  /// Helper function to delete a protocol plugin and close library in the correct order
  static void deleteProtocolPlugin(Licq::ProtocolPlugin* plugin);

  DynamicLibrary::Ptr loadPlugin(PluginThread::Ptr pluginThread,
                                 const std::string& name,
                                 const std::string& prefix);

  void startPlugin(Licq::GeneralPlugin::Ptr plugin);
  void startPlugin(Licq::ProtocolPlugin::Ptr plugin);

  void getAvailablePlugins(Licq::StringList& plugins,
                           const std::string& prefix) const;

  unsigned short myNextPluginId;
  PluginThread::Ptr myGuiThread;

  Licq::GeneralPluginsList myGeneralPlugins;
  mutable Licq::Mutex myGeneralPluginsMutex;

  Licq::ProtocolPluginsList myProtocolPlugins;
  mutable Licq::Mutex myProtocolPluginsMutex;

  Licq::Mutex myExitListMutex;
  Licq::Condition myExitListSignal;
  std::queue<unsigned short> myExitList;
};

extern PluginManager gPluginManager;

} // namespace LicqDaemon

#endif
