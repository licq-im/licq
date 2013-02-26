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

#ifndef LICQDAEMON_PLUGININSTANCE_H
#define LICQDAEMON_PLUGININSTANCE_H

#include <licq/plugin/plugininstance.h>

#include "pluginthread.h"

#include <boost/enable_shared_from_this.hpp>

namespace LicqDaemon
{

class PluginInstance : public virtual Licq::PluginInstance,
                       public boost::enable_shared_from_this<PluginInstance>
{
public:
  typedef boost::shared_ptr<PluginInstance> Ptr;

  PluginInstance(int id, PluginThread::Ptr thread);
  ~PluginInstance();

  // From Licq::PluginInstance
  int id() const;
  boost::shared_ptr<Licq::PluginInterface> internalInterface();

  /**
   * Check if a thread belongs to this plugin
   *
   * @param thread Thread to test
   * @return True if thread is the main thread for this plugin
   */
  bool isThread(const pthread_t& thread) const;

  /**
   * Create the plugin instance
   *
   * @param callback Called in the thread just before the instance is created
   * @return True if the plugin was created successfully
   */
  bool create(void (*callback)(const PluginInstance&));

  /**
   * Initialize the plugin
   *
   * @param argc Number of command line parameters
   * @param argv Command line parameters
   * @return True if initialization was successful
   */
  bool init(int argc, char** argv);

  /**
   * Run the plugin
   *
   * @param startCallback Called in the thread just before run is called
   * @param exitCallback Called in the thread just after run returns
   */
  void run(void (*startCallback)(const PluginInstance&),
           void (*exitCallback)(const PluginInstance&));

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

  // For use by unit test
  void setIsRunning(bool running) { myIsRunning = running; }

  bool isRunning() const { return myIsRunning; }

protected:
  virtual void createInterface() = 0;
  virtual boost::shared_ptr<Licq::PluginInterface> interface() = 0;
  virtual boost::shared_ptr<const Licq::PluginInterface> interface() const = 0;

private:
  /// Entry point for creating plugin in plugin's thread
  static void createThreadEntry(void* plugin);

  /// Entry point for calling init() in plugin's thread
  static bool initThreadEntry(void* plugin);

  /// Entry point for calling run() in plugin's thread
  static void* startThreadEntry(void* plugin);

  const int myId;
  PluginThread::Ptr myThread;
  bool myIsRunning;

  int myArgc;
  char** myArgv;
  char** myArgvCopy;

  void (*myCreateCallback)(const PluginInstance&);
  void (*myStartCallback)(const PluginInstance&);
  void (*myExitCallback)(const PluginInstance&);
};

} // namespace LicqDaemon

#endif
