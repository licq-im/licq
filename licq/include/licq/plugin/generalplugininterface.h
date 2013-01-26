/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2013 Licq Developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_GENERALPLUGININTERFACE_H
#define LICQ_GENERALPLUGININTERFACE_H

#include "plugininterface.h"

namespace Licq
{

class Event;
class PluginSignal;

/**
 * Interface for general plugins.
 *
 * All general plugins must implement this interface. See documentation for
 * Licq::PluginInterface for additional information
 */
class GeneralPluginInterface : public PluginInterface
{
public:
  /// Return the plugin's description
  virtual std::string description() const = 0;

  /// Return the plugin's usage instructions
  virtual std::string usage() const = 0;

  /// Return the path (relative to BASE_DIR) to the configuration file for the
  /// plugin; or an empty string if none.
  virtual std::string configFile() const = 0;

  /// Return true if the plugin is enabled
  virtual bool isEnabled() const = 0;

  /// Enable the plugin
  virtual void enable() = 0;

  /// Disable the plugin
  virtual void disable() = 0;

  /// Return true if a signal should be forwarded to this plugin; otherwise
  /// false
  virtual bool wantSignal(unsigned long signalType) const = 0;

  /**
   * Pushes a signal to the plugin.
   *
   * The plugin takes ownership of the @a signal and is responsible for
   * deleting it when done with it. The plugin should also take care not to
   * block the caller (i.e. only queue the signal for later processing).
   */
  virtual void pushSignal(PluginSignal* signal) = 0;

  /**
   * Pushes an event to the plugin.
   *
   * The plugin takes ownership of the @a event and is responsible for
   * deleting it when done with it. The plugin should also take care not to
   * block the caller (i.e. only queue the event for later processing).
   */
  virtual void pushEvent(Event* event) = 0;

protected:
  virtual ~GeneralPluginInterface() { /* Empty */ }
};

/**
 * This struct contains the initial data and functions needed by Licq to load a
 * plugin. Use the LICQ_GENERAL_PLUGIN_DATA macro to create it.
 */
struct GeneralPluginData
{
  /// Magic value to identify an Licq plugin
  char magic[4];

  /// Version of Licq this plugin is built for
  int licqVersion;

  /**
   * Pointer to a factory function that creates and returns an object that
   * implements the GeneralPluginInterface interface.
   *
   * Note that the plugin should not be (fully) initialized by this. The init
   * and run functions in Licq::Plugin will be called for this afterwards
   *
   */
  GeneralPluginInterface* (*pluginFactory)();
};

} // namespace Licq

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Each plugin must contain the following symbol
 *
 * When a plugin is first loaded, this pointer is fetched and used to get
 * the PluginData struct for the plugin.
 */
extern Licq::GeneralPluginData LicqGeneralPluginData;

#ifdef __cplusplus
}
#endif

/*
 * Convenience macro to define plugin data in a plugin
 *
 * Note: <licq/version.h> must be included
 *
 * @param factory Pointer to the plugin factory function
 */
#define LICQ_GENERAL_PLUGIN_DATA(factory) \
  Licq::GeneralPluginData LicqGeneralPluginData = \
  { {'L', 'i', 'c', 'q' }, LICQ_VERSION, factory }

#endif
