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
               const std::string& prefix, bool prefixId) :
  myLib(lib),
  myThread(pluginThread),
  mySignalMask(0),
  myStartCallback(NULL)
{
  loadSymbol(prefix + "_Main_tep", myMainThreadEntryPoint);
  loadSymbol(prefix + "_Name", myName);
  loadSymbol(prefix + "_Version", myVersion);

  if (prefixId)
    loadSymbol(prefix + "_Id", myId);
  else
    loadSymbol("LP_Id", myId);
  *myId = INVALID_ID;
}

Plugin::~Plugin()
{
  // Empty
}

void Plugin::startThread(void (*startCallback)(Plugin& plugin))
{
  assert(myStartCallback == NULL);
  myStartCallback = startCallback;
  myThread->startPlugin(startThreadEntry, this);
}

int Plugin::joinThread()
{
  void* result = myThread->join();
  if (result != NULL && result != PTHREAD_CANCELED)
  {
    int* retval = reinterpret_cast<int*>(result);
    int value = *retval;
    ::free(retval);
    return value;
  }

  return -1;
}

void Plugin::cancelThread()
{
  myThread->cancel();
}

const char* Plugin::getName() const
{
  return (*myName)();
}

const char* Plugin::getVersion() const
{
  return (*myVersion)();
}

unsigned short Plugin::getId() const
{
  return *myId;
}

const std::string& Plugin::getLibraryName() const
{
  return myLib->getName();
}

void Plugin::shutdown()
{
  myPipe.putChar(PipeShutdown);
}

bool Plugin::callInitInThread()
{
  return myThread->initPlugin(&Plugin::initThreadEntry, this);
}

bool Plugin::initThreadEntry(void* plugin)
{
  return static_cast<Plugin*>(plugin)->initThreadEntry();
}

void* Plugin::startThreadEntry(void* plugin)
{
  Plugin* thisPlugin = static_cast<Plugin*>(plugin);

  if (thisPlugin->myStartCallback)
    (*thisPlugin->myStartCallback)(*thisPlugin);

  return thisPlugin->myMainThreadEntryPoint(NULL);
}
