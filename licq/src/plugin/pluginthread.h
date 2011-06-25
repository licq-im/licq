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

#ifndef LICQDAEMON_PLUGINTHREAD_H
#define LICQDAEMON_PLUGINTHREAD_H

#include "utils/dynamiclibrary.h"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <pthread.h>

namespace LicqDaemon
{

/**
 * The PluginThread class starts a new thread and loads, initializes and starts
 * the plugin in that thread. By loading the plugin in the same thread as it is
 * later run in we hope to avoid problems with libraries' initialization
 * routines not being run in the "main" thread.
 */
class PluginThread : private boost::noncopyable
{
public:
  typedef boost::shared_ptr<PluginThread> Ptr;

  struct Data;

  /**
   * Create a new PluginThread instance, but without starting a new thread for
   * it. Instead make it use the current thread (i.e. the caller's thread). The
   * caller's execution will continue in a new thread with the entry point
   * given in @a newThreadEntry. The argument to @a newThreadEntry will be the
   * newly created PluginThread instance.
   */
  static int createWithCurrentThread(int (*newThreadEntry)(PluginThread::Ptr));

  PluginThread();
  ~PluginThread();

  /**
   * Stop thread if startPlugin() hasn't been called. Mostly for unit test.
   */
  void stop();

  /**
   * Wait for the thread to exit.
   * @return The thread's exit value. If the thread has been canceled, the
   * return value is PTHREAD_CANCELED.
   */
  void* join();

  void cancel();
  bool isThread(const pthread_t& thread) const;

  DynamicLibrary::Ptr loadPlugin(const std::string& path);
  bool initPlugin(bool (*pluginInit)(void*), void* argument);
  void startPlugin(void* (*pluginStart)(void*), void* argument);

private:
  explicit PluginThread(bool);
  void waitForThreadToStart();

  struct NewThreadData;
  static void* newThreadEntry(void* data);

  const bool myIsThreadOwner;
  pthread_t myThread;
  Data* myData;
  void* myExitValue;
};

} // namespace LicqDaemon

#endif
