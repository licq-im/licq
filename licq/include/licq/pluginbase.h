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

#ifndef LICQ_PLUGINBASE_H
#define LICQ_PLUGINBASE_H

#include "generalplugin.h"

namespace Licq
{

typedef GeneralPlugin* (*GeneralPluginFactoryPtr)
    (int, Licq::Plugin::LibraryPtr, Licq::Plugin::ThreadPtr);

/**
 * This struct contains the initial data and functions needed by Licq
 * to load a plugin.
 */
struct GeneralPluginData
{
  /**
   * Magic value to identify an Licq plugin
   * Must contain the characters 'L', 'i', 'c', 'q'
   */
  char licqMagic[4];

  /**
   * Version of Licq this plugin is built for
   * Example of format: 1023 for version 1.2.3
   * Note: This is NOT the plugin version
   * This field can be set to LICQ_VERSION (from licq/version.h) at build time
   * Plugin load will be aborted unless major and minor versions match
   *   e.g. Licq 1.5.3 will load a plugin for 1.5.1 but not 1.4.x or 1.6.x
   */
  int licqVersion;

  /**
   * Pointer to a factory function that creates and returns a GeneralPlugin object
   * This function will be called once when the plugin is loaded
   * Note that the plugin should not be (fully) initialized by this.
   *   The init and run functions in Licq::Plugin will be called for this afterwards
   *
   * The factory function takes parameters that must be forwarded to the
   * constructor of the GeneralPlugin class.
   */
  GeneralPluginFactoryPtr pluginFactory;
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
extern struct Licq::GeneralPluginData LicqGeneralPluginData;

#ifdef __cplusplus
}
#endif


/**
 * Convenience macro to define plugin data in a plugin
 *
 * Note: <licq/version.h> must be included
 *
 * @param factory Pointer to the plugin factory function
 */
#define LICQ_GENERAL_PLUGIN_DATA(factory) \
struct Licq::GeneralPluginData LicqGeneralPluginData = { \
    {'L', 'i', 'c', 'q' }, \
    LICQ_VERSION, \
    factory, \
}


#endif
