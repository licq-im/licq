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

#include "pluginthread.h"

#include <licq/thread/condition.h>
#include <licq/thread/mutex.h>
#include <licq/thread/mutexlocker.h>

#include <boost/exception_ptr.hpp>
#include <csignal>

const pthread_t INVALID_THREAD_ID = 0;

using Licq::MutexLocker;
using namespace LicqDaemon;

struct PluginThread::Data
{
  enum ThreadState
  {
    STATE_STOP,
    STATE_WAITING,
    STATE_LOAD_PLUGIN,
    STATE_INIT_PLUGIN,
    STATE_START_PLUGIN,
    STATE_RUNNING,
    STATE_EXITED
  };

  // Syncronization between controlling thread and plugin thread
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

  Data() : myState(STATE_STOP) {}
};

static void* pluginThreadEntry(void* arg)
{
  PluginThread::Data& data = *static_cast<PluginThread::Data*>(arg);

  // The following signals should not be directed to the plugin but to the
  // daemon; thus we block them here.
  sigset_t signals;
  sigemptyset(&signals);
  sigaddset(&signals, SIGHUP);
  sigaddset(&signals, SIGINT);
  sigaddset(&signals, SIGTERM);
  ::pthread_sigmask(SIG_BLOCK, &signals, NULL);

  // Signal thread started
  MutexLocker locker(data.myMutex);
  data.myState = PluginThread::Data::STATE_WAITING;
  data.myCondition.signal();

  while (data.myState == PluginThread::Data::STATE_WAITING)
  {
    do
    {
      data.myCondition.wait(data.myMutex);
    } while (data.myState == PluginThread::Data::STATE_WAITING);

    switch (data.myState)
    {
      case PluginThread::Data::STATE_STOP:
        break;
      case PluginThread::Data::STATE_WAITING:
      case PluginThread::Data::STATE_RUNNING:
      case PluginThread::Data::STATE_EXITED:
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
          data.myState = PluginThread::Data::STATE_EXITED;
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

struct PluginThread::NewThreadData
{
  int (*myNewThreadEntry)(PluginThread::Ptr);
  PluginThread::Ptr myPluginThread;
};

int PluginThread::createWithCurrentThread(
    int (*newThreadEntry)(PluginThread::Ptr))
{
  // Create a PluginThread instance referencing the current thread.
  PluginThread::Ptr thread(new PluginThread(false));

  NewThreadData data;
  data.myNewThreadEntry = newThreadEntry;
  data.myPluginThread = thread;

  // Create a new thread that the caller will continue executing in.
  pthread_t newThread;
  ::pthread_create(&newThread, NULL, &PluginThread::newThreadEntry, &data);

  // Call the regular plugin thread entry function. The execution will block
  // there waiting for someone to use the PluginThread instance for running a
  // plugin (or until it is stopped).
  thread->myExitValue = pluginThreadEntry(thread->myData);

  {
    // Even if the thread will continue executing, from the PluginThread's
    // perspective the thread is now dead, so mark it as appropiate.
    MutexLocker locker(thread->myData->myMutex);
    thread->myData->myState = PluginThread::Data::STATE_EXITED;
    thread->myData->myCondition.signal();
  }

  thread.reset();

  // Before returning to the original caller we must wait for the caller's new
  // thread to exit.
  void* result;
  if (::pthread_join(newThread, &result) == 0
      && result != NULL && result != PTHREAD_CANCELED)
  {
    int* retval = static_cast<int*>(result);
    int value = *retval;
    delete retval;
    return value;
  }
  return -1;
}

PluginThread::PluginThread() :
  myIsThreadOwner(true),
  myData(new Data),
  myExitValue(NULL)
{
  ::pthread_create(&myThread, NULL, pluginThreadEntry, myData);
  waitForThreadToStart();
}

PluginThread::PluginThread(bool) :
  myIsThreadOwner(false),
  myData(new Data),
  myExitValue(NULL)
{
  myThread = ::pthread_self();
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
  if (myData->myState != PluginThread::Data::STATE_EXITED)
  {
    myData->myState = PluginThread::Data::STATE_STOP;
    myData->myCondition.signal();
  }
}

void* PluginThread::join()
{
  if (isThread(INVALID_THREAD_ID))
    return NULL;

  if (!myIsThreadOwner)
  {
    MutexLocker locker(myData->myMutex);
    while (myData->myState != PluginThread::Data::STATE_EXITED)
      myData->myCondition.wait(myData->myMutex);
    myThread = INVALID_THREAD_ID;
    return myExitValue;
  }

  void* result;
  if (::pthread_join(myThread, &result) == 0)
  {
    myThread = INVALID_THREAD_ID;
    return result;
  }
  return NULL;
}

void PluginThread::cancel()
{
  if (myIsThreadOwner && !isThread(INVALID_THREAD_ID))
    ::pthread_cancel(myThread);
}

bool PluginThread::isThread(const pthread_t& thread) const
{
  return ::pthread_equal(myThread, thread) != 0;
}

DynamicLibrary::Ptr PluginThread::loadPlugin(const std::string& path)
{
  assert(!isThread(INVALID_THREAD_ID));

  MutexLocker locker(myData->myMutex);
  assert(myData->myState == PluginThread::Data::STATE_WAITING);

  myData->myState = PluginThread::Data::STATE_LOAD_PLUGIN;
  myData->myPluginPath = path;
  myData->myException = boost::exception_ptr();
  myData->myCondition.signal();

  // Wait for thread to load plugin
  do
  {
    myData->myCondition.wait(myData->myMutex);
  } while (myData->myState == PluginThread::Data::STATE_LOAD_PLUGIN);

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
  assert(!isThread(INVALID_THREAD_ID));

  MutexLocker locker(myData->myMutex);
  assert(myData->myState == PluginThread::Data::STATE_WAITING);

  myData->myState = PluginThread::Data::STATE_INIT_PLUGIN;
  myData->myPluginInit = pluginInit;
  myData->myPluginInitArgument = argument;
  myData->myPluginInitResult = false;
  myData->myCondition.signal();

  // Wait for thread to initialize plugin
  do
  {
    myData->myCondition.wait(myData->myMutex);
  } while (myData->myState == PluginThread::Data::STATE_INIT_PLUGIN);

  // Get result and reset myData
  bool result = myData->myPluginInitResult;
  myData->myPluginInit = NULL;
  myData->myPluginInitArgument = NULL;

  return result;
}

void PluginThread::startPlugin(void* (*pluginStart)(void*), void* argument)
{
  assert(!isThread(INVALID_THREAD_ID));

  MutexLocker locker(myData->myMutex);
  assert(myData->myState == PluginThread::Data::STATE_WAITING);

  myData->myState = PluginThread::Data::STATE_START_PLUGIN;
  myData->myPluginStart = pluginStart;
  myData->myPluginStartArgument = argument;
  myData->myCondition.signal();

  // Wait for thread to start plugin
  do
  {
    myData->myCondition.wait(myData->myMutex);
  } while (myData->myState == PluginThread::Data::STATE_START_PLUGIN);

  // Reset myData
  myData->myPluginStart = NULL;
  myData->myPluginStartArgument = NULL;
}

void PluginThread::waitForThreadToStart()
{
  MutexLocker locker(myData->myMutex);
  while (myData->myState != PluginThread::Data::STATE_WAITING)
    myData->myCondition.wait(myData->myMutex);
}

void* PluginThread::newThreadEntry(void* voidData)
{
  // Wait for the old thread to reach the initial wait state in
  // pluginThreadEntry.
  NewThreadData* data = static_cast<NewThreadData*>(voidData);
  data->myPluginThread->waitForThreadToStart();

  // Continue the caller's execution in the given function.
  int* retval = new int;
  *retval = data->myNewThreadEntry(data->myPluginThread);

  // Stop plugin thread before returning as if the new thread entry function
  // never did anything with the plugin thread it will still be blocked waiting
  // in the initial wait state.
  data->myPluginThread->stop();
  data->myPluginThread.reset();
  return retval;
}
