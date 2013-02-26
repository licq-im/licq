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

#include "plugininstance.h"

#include <licq/plugin/plugininterface.h>

#include <cassert>
#include <cstring>

// From licq.cpp
extern char** global_argv;

using namespace LicqDaemon;

PluginInstance::PluginInstance(
    int id, PluginThread::Ptr thread)
  : myId(id),
    myThread(thread),
    myIsRunning(false),
    myArgc(0),
    myArgv(NULL),
    myArgvCopy(NULL),
    myCreateCallback(NULL),
    myStartCallback(NULL),
    myExitCallback(NULL)
{
  // Empty
}

PluginInstance::~PluginInstance()
{
  for (int i = 0; i < myArgc; ++i)
    ::free(myArgv[i]);
  delete[] myArgv;
  delete[] myArgvCopy;
}

int PluginInstance::id() const
{
  return myId;
}

boost::shared_ptr<Licq::PluginInterface> PluginInstance::internalInterface()
{
  // Create a shared_ptr that keeps this object alive at least until the
  // returned pointer goes out of scope.
  return boost::shared_ptr<Licq::PluginInterface>(
      shared_from_this(), interface().get());
}

bool PluginInstance::isThread(const pthread_t& thread) const
{
  return myThread->isThread(thread);
}

bool PluginInstance::create(void (*callback)(const PluginInstance&))
{
  assert(myCreateCallback == NULL);
  myCreateCallback = callback;

  myThread->createPlugin(&createThreadEntry, this);
  return !! interface();
}

bool PluginInstance::init(int argc, char** argv)
{
  const size_t size = argc + 2;

  myArgv = new char*[size];
  myArgvCopy = new char*[size];

  myArgv[size - 1] = NULL;

  myArgv[0] = ::strdup(global_argv[0]);

  for (int i = 0; i < argc; ++i)
    myArgv[i + 1] = ::strdup(argv[i]);

  myArgc = argc + 1;

  // We need to create a copy of myArgv and pass that to the plugin, since
  // e.g. KDE changes the pointers in argv (e.g. to strip the path in argv[0])
  // and that messes up free, causing SIGSEGV in the destructor.
  ::memcpy(myArgvCopy, myArgv, size * sizeof(char*));

  return myThread->initPlugin(&initThreadEntry, this);
}

void PluginInstance::run(void (*startCallback)(const PluginInstance&),
                         void (*exitCallback)(const PluginInstance&))
{
  assert(myStartCallback == NULL && myExitCallback == NULL);
  myStartCallback = startCallback;
  myExitCallback = exitCallback;

  myThread->startPlugin(&startThreadEntry, this);
}

void PluginInstance::shutdown()
{
  interface()->shutdown();
  myIsRunning = false;
}

int PluginInstance::joinThread()
{
  void* result = myThread->join();
  if (result != NULL && result != PTHREAD_CANCELED)
  {
    int* retval = static_cast<int*>(result);
    int value = *retval;
    delete retval;
    return value;
  }

  return -1;
}

void PluginInstance::cancelThread()
{
  myThread->cancel();
}

void PluginInstance::createThreadEntry(void* plugin)
{
  PluginInstance* thisPlugin = static_cast<PluginInstance*>(plugin);

  if (thisPlugin->myCreateCallback)
    thisPlugin->myCreateCallback(*thisPlugin);

  thisPlugin->createInterface();
}

bool PluginInstance::initThreadEntry(void* plugin)
{
  PluginInstance* thisPlugin = static_cast<PluginInstance*>(plugin);

  // Set optind to 0 so plugins can use getopt
  optind = 0;

  return thisPlugin->interface()->init(
      thisPlugin->myArgc, thisPlugin->myArgvCopy);
}

void* PluginInstance::startThreadEntry(void* plugin)
{
  PluginInstance* thisPlugin = static_cast<PluginInstance*>(plugin);

  if (thisPlugin->myStartCallback != NULL)
    (*thisPlugin->myStartCallback)(*thisPlugin);

  thisPlugin->myIsRunning = true;

  int* retval = new int;
  *retval = thisPlugin->interface()->run();

  if (thisPlugin->myExitCallback != NULL)
    (*thisPlugin->myExitCallback)(*thisPlugin);

  return retval;
}
