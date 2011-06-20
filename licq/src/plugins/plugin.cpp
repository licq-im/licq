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

#include "plugin.h"

#include <cassert>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

// From licq.cpp
extern char** global_argv;

using namespace LicqDaemon;
using namespace std;

Plugin::Plugin(DynamicLibrary::Ptr lib,
               PluginThread::Ptr pluginThread,
               const std::string& prefix) :
  myLib(lib),
  myThread(pluginThread),
  myInitCallback(NULL),
  myStartCallback(NULL),
  myExitCallback(NULL),
  myArgc(0),
  myArgv(NULL),
  myArgvCopy(NULL),
  myId(INVALID_ID)
{
  loadSymbol(prefix + "_Init", myInit);
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
  for (int i = 0; i < myArgc; ++i)
    ::free(myArgv[i]);
  delete[] myArgv;
  delete[] myArgvCopy;
}

bool Plugin::callInit(int argc, char** argv,
                         void (*callback)(const Plugin&))
{
  assert(myInitCallback == NULL);

  const size_t size = argc + 2;

  myArgv = new char*[size];
  myArgvCopy = new char*[size];

  myArgv[size - 1] = NULL;

  // TODO: use licq or libname?
  //myArgv[0] = ::strdup(myLib->getName().c_str());
  myArgv[0] = ::strdup(global_argv[0]);

  for (int i = 0; i < argc; ++i)
    myArgv[i + 1] = ::strdup(argv[i]);

  myArgc = argc + 1;

  // We need to create a copy of myArgv and pass that to the plugin, since
  // e.g. KDE changes the pointers in argv (e.g. to strip the path in argv[0])
  // and that messes up free, causing SIGSEGV in the destructor.
  ::memcpy(myArgvCopy, myArgv, size * sizeof(char*));

  myInitCallback = callback;
  return myThread->initPlugin(&Plugin::initThreadEntry, this);
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
    int* retval = static_cast<int*>(result);
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

int Plugin::id() const
{
  return myId;
}

string Plugin::name() const
{
  return (*myName)();
}

string Plugin::version() const
{
  return (*myVersion)();
}

string Plugin::configFile() const
{
  if (myConfigFile)
    return (*myConfigFile)();
  else
    return string();
}

string Plugin::libraryName() const
{
  return myLib->getName();
}

void Plugin::shutdown()
{
  myPipe.putChar(PipeShutdown);
}

bool Plugin::initThreadEntry(void* plugin)
{
  Plugin* thisPlugin = static_cast<Plugin*>(plugin);

  if (thisPlugin->myInitCallback)
    thisPlugin->myInitCallback(*thisPlugin);

  // Set optind to 0 so plugins can use getopt
  optind = 0;

  return (*thisPlugin->myInit)(thisPlugin->myArgc, thisPlugin->myArgvCopy);
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
