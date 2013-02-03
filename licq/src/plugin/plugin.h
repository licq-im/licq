/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011, 2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQDAEMON_PLUGIN_H
#define LICQDAEMON_PLUGIN_H

#include <licq/plugin/plugin.h>

#include "utils/dynamiclibrary.h"
#include "pluginthread.h"

#include <boost/enable_shared_from_this.hpp>

namespace Licq
{
class PluginInterface;
}

namespace LicqDaemon
{

class Plugin : public virtual Licq::Plugin,
               public boost::enable_shared_from_this<Plugin>
{
public:
  Plugin(int id, DynamicLibrary::Ptr lib, PluginThread::Ptr thread);
  ~Plugin();

  // From Licq::Plugin
  int id() const;
  std::string name() const;
  std::string version() const;
  std::string libraryName() const;
  boost::shared_ptr<Licq::PluginInterface> internalInterface();

  /**
   * Check if a thread belongs to this plugin
   *
   * @param thread Thread to test
   * @return True if thread is the main thread for this plugin
   */
  bool isThread(const pthread_t& thread) const;

  /**
   * Initialize the plugin
   *
   * @param argc Number of command line parameters
   * @param argv Command line parameters
   * @param callback Called in the thread just before init is called
   * @return True if initialization was successful
   */
  bool init(int argc, char** argv, void (*callback)(const Plugin&));

  /**
   * Run the plugin
   *
   * @param startCallback Called in the thread just before run is called
   * @param exitCallback Called in the thread just after run returns
   */
  void run(void (*startCallback)(const Plugin&),
           void (*exitCallback)(const Plugin&));

  /**
   * Tell a plugin to shut down
   */
  void shutdown();

  /**
   * Wait for the plugin to stop
   *
   * @return Exit code from plugin thread
   */
  int joinThread();

  /**
   * Cancel the plugin's thread
   */
  void cancelThread();

protected:
  virtual boost::shared_ptr<Licq::PluginInterface> interface() = 0;
  virtual boost::shared_ptr<const Licq::PluginInterface> interface() const = 0;

private:
  /// Entry point for calling init() in plugin's thread
  static bool initThreadEntry(void* plugin);

  /// Entry point for calling run() in plugin's thread
  static void* startThreadEntry(void* plugin);

  const int myId;
  DynamicLibrary::Ptr myLibrary;
  PluginThread::Ptr myThread;

  int myArgc;
  char** myArgv;
  char** myArgvCopy;

  void (*myInitCallback)(const Plugin&);
  void (*myStartCallback)(const Plugin&);
  void (*myExitCallback)(const Plugin&);
};

} // namespace LicqDaemon

#endif
