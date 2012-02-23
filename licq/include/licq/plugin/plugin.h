/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq developers <licq-dev@googlegroups.com>
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
#include <pthread.h>
#include <string>

#include "../macro.h"

namespace LicqDaemon
{
class PluginManager;
}

namespace Licq
{

/**
 * Base class for plugin instances
 *
 * All plugins must have a subclass implementing this interface
 *
 * Note: When a subclass is constructed, it should only perform minimal
 * initialization needed for simple functions like name() and version() to
 * be usable. Licq will call init() afterwards to properly initialze the
 * plugin before run() is called to start the plugin.
 *
 * Although a plugin will run in a separate thread, calls to the public
 * functions and the protected functions called from the protocol manager
 * can be made from any thread. It is the responsibility of the plugin to make
 * sure these functions are thread safe when needed.
 */
class Plugin : private boost::noncopyable
{
public:
  class Params;

  // Notification that plugins can get via its pipe
  static const char PipeSignal = 'S';
  static const char PipeShutdown = 'X';

  /// Get the plugin's unique id.
  int id() const;

  /// Get the plugin's name.
  virtual std::string name() const = 0;

  /// Get the plugin's version.
  virtual std::string version() const = 0;

  /// Configuration file for the plugin. Empty string if none. Path is relative to BASE_DIR
  virtual std::string configFile() const;

  /// Get the name of the library from where the plugin was loaded.
  const std::string& libraryName() const;

  /**
   * Check if a thread belongs to this plugin
   * Called by anyone
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
   */
  Plugin(Params& p);

  /// Destructor
  virtual ~Plugin();

  /**
   * Initialize the plugin
   * Called in plugin thread by PluginManager
   *
   * @param argc Number of command line parameters
   * @param argv Command line parameters
   * @return True if initialization was successful
   */
  virtual bool init(int argc, char** argv) = 0;

  /**
   * Run the plugin
   * Called in plugin thread by PluginManager
   *
   * This function will be called in a separate thread and may block
   *
   * @return Exit code for the plugin
   */
  virtual int run() = 0;

  /**
   * Delete the plugin object from the plugins context
   * This function will be called once after run() has returned and the plugin
   *   thread has terminated but before the library is closed.
   * Normally this function should only contain "delete this;"
   */
  virtual void destructor() = 0;

  /**
   * Get read end of pipe used to communicate with the plugin
   * Called from plugin
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

  /**
   * Tell a plugin to shut down
   * Called from PluginManager
   */
  void shutdown();

private:
  LICQ_DECLARE_PRIVATE();

  /// Convenience for friends to access myPrivate without casting from subclass
  Private* basePrivate() { return myPrivate; }

  /// Allow the plugin manager to access private members
  friend class LicqDaemon::PluginManager;
};

} // namespace Licq

#endif
