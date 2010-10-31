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

#include "plugin.h"

#include <licq/thread/mutexlocker.h>

#include <cassert>
#include <pthread.h>

using Licq::MutexLocker;
using namespace LicqDaemon;

Plugin::Plugin(DynamicLibrary::Ptr lib,
               PluginThread::Ptr pluginThread,
               const std::string& prefix) :
  myLib(lib),
  myThread(pluginThread),
  mySignalMask(0),
  myInitCallback(NULL),
  myStartCallback(NULL),
  myExitCallback(NULL),
  myId(INVALID_ID)
{
  loadSymbol(prefix + "_Main", myMain);
  loadSymbol(prefix + "_Name", myName);
  loadSymbol(prefix + "_Version", myVersion);

  try
  {
    // ConfigFile is not required
    loadSymbol(prefix + "_ConfigFile", myConfigFile);
  }
  catch (DynamicLibrary::Exception&)
  {
    myConfigFile = NULL;
  }
}

Plugin::~Plugin()
{
  // Empty
}

void Plugin::startThread(
    void (*startCallback)(const Plugin& plugin),
    void (*exitCallback)(const Plugin& plugin))
{
  assert(myStartCallback == NULL && myExitCallback == NULL);
  myStartCallback = startCallback;
  myExitCallback = exitCallback;
  myThread->startPlugin(startThreadEntry, this);
}

int Plugin::joinThread()
{
  void* result = myThread->join();
  if (result != NULL && result != PTHREAD_CANCELED)
  {
    int* retval = reinterpret_cast<int*>(result);
    int value = *retval;
    delete retval;
    return value;
  }

  return -1;
}

void Plugin::cancelThread()
{
  myThread->cancel();
}

unsigned short Plugin::getId() const
{
  return myId;
}

const char* Plugin::getName() const
{
  return (*myName)();
}

const char* Plugin::getVersion() const
{
  return (*myVersion)();
}

const char* Plugin::getConfigFile() const
{
  if (myConfigFile)
    return (*myConfigFile)();
  else
    return NULL;
}

const std::string& Plugin::getLibraryName() const
{
  return myLib->getName();
}

void Plugin::shutdown()
{
  myPipe.putChar(PipeShutdown);
}

bool Plugin::callInitInThread(void (*initCallback)(const Plugin&))
{
  assert(myInitCallback == NULL);
  myInitCallback = initCallback;
  return myThread->initPlugin(&Plugin::initThreadEntry, this);
}

bool Plugin::initThreadEntry(void* plugin)
{
  Plugin* thisPlugin = static_cast<Plugin*>(plugin);

  if (thisPlugin->myInitCallback)
    thisPlugin->myInitCallback(*thisPlugin);

  return thisPlugin->initThreadEntry();
}

void* Plugin::startThreadEntry(void* plugin)
{
  Plugin* thisPlugin = static_cast<Plugin*>(plugin);

  if (thisPlugin->myStartCallback)
    (*thisPlugin->myStartCallback)(*thisPlugin);

  int* retval = new int;
  *retval = thisPlugin->myMain();

  if (thisPlugin->myExitCallback)
    (*thisPlugin->myExitCallback)(*thisPlugin);

  return retval;
}
