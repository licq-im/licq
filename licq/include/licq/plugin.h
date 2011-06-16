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

#ifndef LICQ_PLUGIN_H
#define LICQ_PLUGIN_H

#include <boost/noncopyable.hpp>
#include <string>

namespace Licq
{

/**
 * The base class for plugin instances.
 *
 * Plugins are handled using the PluginManager.
 */
class Plugin : private boost::noncopyable
{
public:
  // Notification that plugins can get via its pipe
  static const char PipeSignal = 'S';
  static const char PipeShutdown = 'X';

  /// Get the plugin's unique id.
  virtual unsigned short getId() const = 0;

  /// Get the plugin's name.
  virtual const char* getName() const = 0;

  /// Get the plugin's version.
  virtual const char* getVersion() const = 0;

  /// Get the name of the plugin's config file. Can be NULL if the plugin
  /// doesn't have a config file.
  virtual const char* getConfigFile() const = 0;

  /// Get the name of the library from where the plugin was loaded.
  virtual const std::string& getLibraryName() const = 0;

  /// Ask the plugin to shutdown.
  virtual void shutdown() = 0;

protected:
  virtual ~Plugin() { /* Empty */ }
};


} // namespace Licq

#endif
