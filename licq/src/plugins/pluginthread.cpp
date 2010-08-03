/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#include "pluginthread.h"

#include <licq/thread/condition.h>
#include <licq/thread/mutex.h>
#include <licq/thread/mutexlocker.h>

#include <boost/exception_ptr.hpp>

using Licq::MutexLocker;
using namespace LicqDaemon;

struct PluginThread::Data
{
  enum ThreadState
  {
    STATE_STOPPED,
    STATE_WAITING,
    STATE_LOAD_PLUGIN,
    STATE_INIT_PLUGIN,
    STATE_START_PLUGIN,
    STATE_RUNNING
  };

  // Syncronization between main and plugin thread
  Licq::Condition myCondition;
  Licq::Mutex myMutex;
  ThreadState myState;

  // For plugin load
  std::string myPluginPath;
  DynamicLibrary::Ptr myLib;
  boost::exception_ptr myException;

  // For plugin init
  bool (*myPluginInit)(void*);
  void* myPluginInitArgument;
  bool myPluginInitResult;

  // For plugin start
  void* (*myPluginStart)(void*);
  void* myPluginStartArgument;
};

static void* pluginThreadEntry(void* arg)
{
  PluginThread::Data& data = *reinterpret_cast<PluginThread::Data*>(arg);

  // Signal thread started
  MutexLocker locker(data.myMutex);
  data.myState = PluginThread::Data::STATE_WAITING;
  data.myCondition.signal();

  while (data.myState == PluginThread::Data::STATE_WAITING)
  {
    data.myCondition.wait(data.myMutex);

    switch (data.myState)
    {
      case PluginThread::Data::STATE_STOPPED:
        break;
      case PluginThread::Data::STATE_WAITING:
      case PluginThread::Data::STATE_RUNNING:
        assert(false);
        break;
      case PluginThread::Data::STATE_LOAD_PLUGIN:
        try
        {
          data.myLib.reset(new DynamicLibrary(data.myPluginPath));
        }
        catch (const DynamicLibrary::Exception&)
        {
          data.myException = boost::current_exception();
          data.myCondition.signal();
          return NULL;
        }
        data.myState = PluginThread::Data::STATE_WAITING;
        break;
      case PluginThread::Data::STATE_INIT_PLUGIN:
        data.myPluginInitResult = data.myPluginInit(data.myPluginInitArgument);
        data.myState = PluginThread::Data::STATE_WAITING;
        break;
      case PluginThread::Data::STATE_START_PLUGIN:
      {
        void* (*start)(void*) = data.myPluginStart;
        void* argument = data.myPluginStartArgument;
        data.myState = PluginThread::Data::STATE_RUNNING;
        data.myCondition.signal();
        locker.unlock();

        return start(argument);
      }
    }

    data.myCondition.signal();
  }

  return NULL;
}

PluginThread::PluginThread() :
  myData(new Data())
{
  myData->myState = PluginThread::Data::STATE_STOPPED;
  ::pthread_create(&myThread, NULL, pluginThreadEntry, myData);

  // Wait for thread to start
  MutexLocker locker(myData->myMutex);
  if (myData->myState != PluginThread::Data::STATE_WAITING)
    myData->myCondition.wait(myData->myMutex);
}

PluginThread::~PluginThread()
{
  stop();
  join();
  delete myData;
}

void PluginThread::stop()
{
  MutexLocker locker(myData->myMutex);
  myData->myState = PluginThread::Data::STATE_STOPPED;
  myData->myCondition.signal();
}

void* PluginThread::join()
{
  void* result;
  if (::pthread_join(myThread, &result) == 0)
    return result;
  return NULL;
}

void PluginThread::cancel()
{
  ::pthread_cancel(myThread);
}

bool PluginThread::isThread(const pthread_t& thread) const
{
  return ::pthread_equal(myThread, thread) != 0;
}

DynamicLibrary::Ptr PluginThread::loadPlugin(const std::string& path)
{
  MutexLocker locker(myData->myMutex);
  myData->myState = PluginThread::Data::STATE_LOAD_PLUGIN;
  myData->myPluginPath = path;
  myData->myException = boost::exception_ptr();
  myData->myCondition.signal();

  // Wait for thread to load plugin
  myData->myCondition.wait(myData->myMutex);
  if (myData->myException)
    boost::rethrow_exception(myData->myException);

  // Get result and reset myData
  DynamicLibrary::Ptr lib = myData->myLib;
  myData->myLib.reset();
  myData->myPluginPath.clear();

  return lib;
}

bool PluginThread::initPlugin(bool (*pluginInit)(void*), void* argument)
{
  MutexLocker locker(myData->myMutex);
  myData->myState = PluginThread::Data::STATE_INIT_PLUGIN;
  myData->myPluginInit = pluginInit;
  myData->myPluginInitArgument = argument;
  myData->myPluginInitResult = false;
  myData->myCondition.signal();

  // Wait for thread to initialize plugin
  myData->myCondition.wait(myData->myMutex);

  // Get result and reset myData
  bool result = myData->myPluginInitResult;
  myData->myPluginInit = NULL;
  myData->myPluginInitArgument = NULL;

  return result;
}

void PluginThread::startPlugin(void* (*pluginStart)(void*), void* argument)
{
  MutexLocker locker(myData->myMutex);
  myData->myState = PluginThread::Data::STATE_START_PLUGIN;
  myData->myPluginStart = pluginStart;
  myData->myPluginStartArgument = argument;
  myData->myCondition.signal();

  // Wait for thread to start plugin
  myData->myCondition.wait(myData->myMutex);

  // Reset myData
  myData->myPluginStart = NULL;
  myData->myPluginStartArgument = NULL;
}
