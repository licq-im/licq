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

#ifndef LICQ_GENERALPLUGINFACTORY_H
#define LICQ_GENERALPLUGINFACTORY_H

#include "pluginfactory.h"

#include <boost/shared_ptr.hpp>

namespace Licq
{

class GeneralPluginInterface;

/**
 * Factory for general plugins.
 *
 * All general plugins must implement this factory. See documentation for
 * Licq::PluginFactory for additional information
 */
class GeneralPluginFactory : public PluginFactory
{
public:
  /// Return the plugin's description
  virtual std::string description() const = 0;

  /// Return the plugin's usage instructions
  virtual std::string usage() const = 0;

  /// Return the path (relative to BASE_DIR) to the configuration file for the
  /// plugin; or an empty string if none.
  virtual std::string configFile() const = 0;

  /// Create a new plugin instance
  virtual GeneralPluginInterface* createPlugin() = 0;

protected:
  virtual ~GeneralPluginFactory() { /* Empty */ }
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

  /// Pointer to a function that creates and returns a factory object.
  GeneralPluginFactory* (*createFactory)();

  /// Called to destroy the factory when it is no longer needed
  void (*destroyFactory)(GeneralPluginFactory* factory);
};

} // namespace Licq

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Each plugin must contain the following symbol
 *
 * When a plugin is first loaded, this pointer is fetched and used to get
 * the GeneralPluginData struct for the plugin.
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
 * @param create Pointer to the create factory function
 * @param destroy Pointer to the destroy factory function
 */
#define LICQ_GENERAL_PLUGIN_DATA(create, destroy) \
  Licq::GeneralPluginData LicqGeneralPluginData = \
  { {'L', 'i', 'c', 'q' }, LICQ_VERSION, create, destroy }

#endif
