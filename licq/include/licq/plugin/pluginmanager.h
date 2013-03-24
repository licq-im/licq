/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "generalplugin.h"
#include "protocolplugin.h"
#include "protocolplugininstance.h"

namespace Licq
{
class Event;
class PluginSignal;
class ProtocolSignal;
class USerId;

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
 *       unsigned long protocolId = protocol->protocolId();
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
   * Get the protocol instance that is used for the given owner id.
   *
   * @param ownerId Owner to get protocol instance for.
   * @return Pointer to the ProtocolPlugin (if found) or an empty pointer if
   *         owner is invalid.
   */
  virtual ProtocolPluginInstance::Ptr
  getProtocolInstance(const UserId& ownerId) const = 0;

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
   * Shut down and unload a general plugin
   *
   * @param plugin The plugin to shut down
   */
  virtual void unloadGeneralPlugin(GeneralPlugin::Ptr plugin) = 0;

  /**
   * Shut down and unload a protocol plugin
   *
   * @param plugin The plugin to shut down
   */
  virtual void unloadProtocolPlugin(ProtocolPlugin::Ptr plugin) = 0;

  /**
   * Send an event to a general plugin
   *
   * Note: This function will return immediately. The event will be processed
   * by the plugin asynchrony
   *
   * @param event Event to forward to the plugin, will be deleted by receiver
   */
  virtual void pushPluginEvent(Event* event) = 0;

  /**
   * Send a signal to all general plugins
   *
   * Note: This function will return immediately. The event will be processed
   * by the plugin asynchrony
   *
   * @param signal Signal to forward to the plugins, will be deleted by receiver
   */
  virtual void pushPluginSignal(PluginSignal* signal) = 0;

  /**
   * Send a signal to a protocol plugin
   *
   * Note: This function will return immediately. The event will be processed
   * by the plugin asynchronously
   *
   * @param signal Signal to forward to the plugin, will be deleted by receiver
   */
  virtual void pushProtocolSignal(ProtocolSignal* signal) = 0;

protected:
  virtual ~PluginManager() { /* Empty */ }
};

extern PluginManager& gPluginManager;

} // namespace Licq

#endif
