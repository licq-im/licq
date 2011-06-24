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

#include <boost/exception/info.hpp>
#include <boost/noncopyable.hpp>
#include <pthread.h>
#include <string>

#include "macro.h"

namespace LicqDaemon
{
class PluginManager;
}

class GeneralPluginTest;
class PluginTest;
class ProtocolPluginTest;

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
  class Params;

  typedef boost::error_info<struct tag_errinfo_symbol_name, std::string> errinfo_symbol_name;

  // Notification that plugins can get via its pipe
  static const char PipeSignal = 'S';
  static const char PipeShutdown = 'X';

  /// Get the plugin's unique id.
  int id() const;

  /// Get the plugin's name.
  std::string name() const;

  /// Get the plugin's version.
  std::string version() const;

  /// Configuration file for the plugin. Empty string if none. Path is relative to BASE_DIR
  std::string configFile() const;

  /// Get the name of the library from where the plugin was loaded.
  const std::string& libraryName() const;

  /// Ask the plugin to shutdown.
  void shutdown();

  /**
   * Check if a thread belongs to this plugin
   *
   * @param thread Thread to test
   * @return True if thread is the main thread for this plugin
   */
  bool isThread(const pthread_t& thread) const;

  /// Convenience function to check the current thread belongs to this plugin
  bool isThisThread() const
  { return isThread(::pthread_self()); }

protected:
  /**
   * Constructor
   *
   * @param p Paramaters from PluginManager
   * @param prefix Prefix for library symbols
   */
  Plugin(Params& p, const std::string& prefix);

  /// Destructor
  virtual ~Plugin();

  /**
   * Get read end of pipe used to communicate with the plugin
   *
   * @return A file descriptor that can be polled for new events and signals
   */
  int getReadPipe() const;

  /**
   * Send a notification to the plugin
   *
   * @param c A character, will be received by plugin through it's read pipe
   */
  void notify(char c);

  /// Load a symbol from the plugin library
  void loadSymbol(const std::string& name, void** symbol);

private:
  LICQ_DECLARE_PRIVATE();

  /// Convenience for friends to access myPrivate without casting from subclass
  Private* basePrivate() { return myPrivate; }

  /// Allow the plugin manager to access private members
  friend class LicqDaemon::PluginManager;

  /// Allow the unit tests to test private members
  friend class ::PluginTest;
  friend class ::GeneralPluginTest;
  friend class ::ProtocolPluginTest;
};

} // namespace Licq

#endif
