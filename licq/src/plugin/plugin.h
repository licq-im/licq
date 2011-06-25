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

#ifndef LICQDAEMON_PLUGIN_H
#define LICQDAEMON_PLUGIN_H

#include <licq/plugin/plugin.h>

#include <licq/pipe.h>

#include "utils/dynamiclibrary.h"
#include "pluginthread.h"

namespace Licq
{

/**
 * Temporary class used to hold initalization data for Plugin constructor
 */
class Plugin::Params
{
public:
  Params(int id, LicqDaemon::DynamicLibrary::Ptr lib,
      LicqDaemon::PluginThread::Ptr thread) :
    myId(id), myLib(lib), myThread(thread)
  { /* Empty */ }

  int myId;
  LicqDaemon::DynamicLibrary::Ptr myLib;
  LicqDaemon::PluginThread::Ptr myThread;
};

class Plugin::Private
{
public:
  Private(Plugin* plugin, int id, LicqDaemon::DynamicLibrary::Ptr lib,
      LicqDaemon::PluginThread::Ptr thread);
  ~Private();

  /**
   * Initialize the plugin
   *
   * This is a wrapper that initializes the plugin in it's thread and waits
   * for the init function to return.
   *
   * @param argc Number of arguments for plugin
   * @param argv Arguments for plugin
   * @param callback Function to call in plugin's thread before calling plugin
   * @return Return value from plugin initialization
   */
  bool callInit(int argc = 0, char** argv = NULL, void (*callback)(const Plugin&) = NULL);

  /**
   * Start the plugin
   *
   * This is a wrapper that runs the plugin in its thread
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
   *
   * @return Exit code from plugin thread
   */
  int joinThread();

  /**
   * Cancel the plugin's thread
   */
  void cancelThread();

  /**
   * Get the library object for this plugin
   */
  LicqDaemon::DynamicLibrary::Ptr library()
  { return myLib; }

private:
  /// Entry point for calling init() in plugin's thread
  static bool initThreadEntry(void* plugin);

  /// Entry point for calling run() in plugin's thread
  static void* startThreadEntry(void* plugin);

  Plugin* const myPlugin;
  const int myId;
  LicqDaemon::DynamicLibrary::Ptr myLib;
  Licq::Pipe myPipe;

  LicqDaemon::PluginThread::Ptr myThread;
  void (*myInitCallback)(const Plugin&);
  void (*myStartCallback)(const Plugin&);
  void (*myExitCallback)(const Plugin&);

  int myArgc;
  char** myArgv;
  char** myArgvCopy;

  friend class Plugin;
};

} // namespace Licq

#endif
