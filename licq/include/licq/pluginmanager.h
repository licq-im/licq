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

#ifndef LICQ_PLUGINMANAGER_H
#define LICQ_PLUGINMANAGER_H

#include <boost/noncopyable.hpp>
#include <list>
#include <string>

#include "plugin.h"


namespace Licq
{
typedef std::list<std::string> StringList;

/// A list of pointers to GeneralPlugin instances.
typedef std::list<GeneralPlugin::Ptr> GeneralPluginsList;

/// A list of pointers to ProtocolPlugin instances.
typedef std::list<ProtocolPlugin::Ptr> ProtocolPluginsList;

/**
 * The class responsible for handling plugins.
 *
 * Plugins use this to register and unregister themselves, starting other
 * plugins and for getting information about active plugins.
 *
 * Usage example: Iterate over all protocol plugins:
 *     Licq::ProtocolPluginsList protocols;
 *     Licq::gPluginManager.getProtocolPluginsList(protocols);
 *     BOOST_FOREACH(Licq::ProtocolPlugin::Ptr protocol, protocols)
 *     {
 *       unsigned long protocolId = protocol->getProtocolId();
 *       ...
 *     }
 */
class PluginManager : private boost::noncopyable
{
public:
  /**
   * Get a list of all loaded general plugins.
   *
   * @param[out] plugins List to put pointers to plugins into.
   * @see GeneralPlugin
   */
  virtual void getGeneralPluginsList(GeneralPluginsList& plugins) const = 0;

  /**
   * Get a list of all loaded protocol plugins.
   *
   * @param[out] plugins List to put pointers to plugins into.
   * @see ProtocolPlugin
   */
  virtual void getProtocolPluginsList(ProtocolPluginsList& plugins) const = 0;

  /**
   * Get a list of all available general plugins
   *
   * @param[out] plugins List to put names of all found plugins into
   * @param includeLoaded False to exclude plugins already loaded
   */
  virtual void getAvailableGeneralPlugins(StringList& plugins,
                                          bool includeLoaded = true) const = 0;

  /**
   * Get a list of all available protocol plugins
   *
   * @param[out] plugins List to put names of all found plugins into
   * @param includeLoaded False to exclude plugins already loaded
   */
  virtual void getAvailableProtocolPlugins(StringList& plugins,
                                           bool includeLoaded = true) const = 0;

  /**
   * Get a protocol plugin based on the protocol id.
   *
   * @param protocolId Protocol id to get plugin for.
   * @return Pointer to the ProtocolPlugin (if found) or an empty pointer if
   *         protocol id is unknown.
   */
  virtual
  ProtocolPlugin::Ptr getProtocolPlugin(unsigned long protocolId) const = 0;

  /**
   * Load and start the general plugin @a name.
   *
   * @param name The plugin to start. May be an absolute path to an plugin or a
   *        simple name in which case the plugin will be searched for in the
   *        standard location.
   * @param argc Number of arguments in @a argv.
   * @param argv Arguments to pass to the plugin. The first argument (argv[0])
   *        will be passed to the plugin as argv[1]. May be NULL, in which case
   *        @a argc must be 0.
   * @return True if the plugin was found and started successfully.
   */
  virtual bool
  startGeneralPlugin(const std::string& name, int argc, char** argv) = 0;

  /**
   * Load and start the protocol plugin @a name.
   *
   * @param name The plugin to start. May be an absolute path to an plugin or a
   *        simple name in which case the plugin will be searched for in the
   *        standard location.
   * @return True if the plugin was found and started successfully.
   */
  virtual bool startProtocolPlugin(const std::string& name) = 0;

  /**
   * Registers current thread as new general plugin.
   *
   * General plugins must call this method when they have started to start
   * receiving signals.
   *
   * @param signalMask A mask indicating which signals the plugin wish to
   *        receive. Use the constant PluginSignal::SignalAll to receive all signals.
   * @return The pipe to listen on for notifications.
   */
  virtual int registerGeneralPlugin(unsigned long signalMask) = 0;

  /**
   * Unregisters current thread as a general plugin.
   *
   * General plugins must call this method before they shutdown to stop
   * receiving signal.
   */
  virtual void unregisterGeneralPlugin() = 0;

  /**
   * Registers current thread as a new protocol plugin.
   *
   * Protocol plugins must call this method when they have started to start
   * receiving signals.
   *
   * @return The pipe to listen on for notifications.
   */
  virtual int registerProtocolPlugin() = 0;

  /**
   * Unregisters current thread as a protocol plugin.
   *
   * Protocol plugins must call this method before they shutdown to stop
   * receiving signal.
   */
  virtual void unregisterProtocolPlugin() = 0;

protected:
  virtual ~PluginManager() { /* Empty */ }
};

extern PluginManager& gPluginManager;

} // namespace Licq

#endif
