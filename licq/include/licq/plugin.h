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
#include <boost/shared_ptr.hpp>
#include <pthread.h>
#include <string>

#include "macro.h"

namespace LicqDaemon
{
class DynamicLibrary;
class PluginManager;
class PluginThread;
}

class GeneralPluginTest;
class PluginTest;
class ProtocolPluginTest;

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
  typedef boost::error_info<struct tag_errinfo_symbol_name, std::string> errinfo_symbol_name;

  // Short names for convenience in sub plugins
  typedef boost::shared_ptr<LicqDaemon::DynamicLibrary> LibraryPtr;
  typedef boost::shared_ptr<LicqDaemon::PluginThread> ThreadPtr;

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

  /// Ask the plugin to shutdown.
  void shutdown();

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
   * NOT called in plugin thread
   *
   * @param id Unique id for this plugin
   * @param lib Library plugin was loaded from
   * @param thread Thread for plugin to run in
   */
  Plugin(int id, LibraryPtr lib, ThreadPtr thread);

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

private:
  LICQ_DECLARE_PRIVATE();

  /**
   * Initialize the plugin
   * Called from PluginManager
   *
   * This is a wrapper that calls init() from the plugin's thread and waits
   * for init() to return.
   *
   * @param argc Number of arguments for plugin
   * @param argv Arguments for plugin
   * @param callback Function to call in plugin's thread before calling plugin
   * @return Return value from init()
   */
  bool callInit(int argc = 0, char** argv = NULL, void (*callback)(const Plugin&) = NULL);

  /**
   * Start the plugin
   * Called from PluginManager
   *
   * This is a wrapper that calls run() from the plugin's thread.
   * This function returns immediately
   *
   * @param startCallback Function to call in the plugin's thread just before
   *                      the plugin's main function is called
   * @param exitCallback Function to call when thread exists
   */
  void startThread(void (*startCallback)(const Plugin&) = NULL,
      void (*exitCallback)(const Plugin&) = NULL);

  /**
   * Wait for the plugin to stop
   * Called from PluginManager
   *
   * @return Exit code from run()
   */
  int joinThread();

  /**
   * Cancel the plugin's thread
   * Called from PluginManager
   */
  void cancelThread();

  /**
   * Get the library object for this plugin
   * Called from PluginManager
   */
  LibraryPtr library();

  /// Allow the plugin manager to access private members
  friend class LicqDaemon::PluginManager;

  /// Allow the unit tests to test private members
  friend class ::PluginTest;
  friend class ::GeneralPluginTest;
  friend class ::ProtocolPluginTest;
};

} // namespace Licq

#endif
