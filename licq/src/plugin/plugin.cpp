/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq developers <licq-dev@googlegroups.com>
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

#include <licq/plugin/plugininterface.h>
#include "plugin.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

// From licq.cpp
extern char** global_argv;

using namespace LicqDaemon;
using namespace std;

Plugin::Plugin(int id, DynamicLibrary::Ptr lib, PluginThread::Ptr thread)
  : myId(id),
    myLibrary(lib),
    myThread(thread),
    myArgc(0),
    myArgv(NULL),
    myArgvCopy(NULL),
    myInitCallback(NULL),
    myStartCallback(NULL),
    myExitCallback(NULL)
{
  // Empty
}

Plugin::~Plugin()
{
  for (int i = 0; i < myArgc; ++i)
    ::free(myArgv[i]);
  delete[] myArgv;
  delete[] myArgvCopy;
}

int Plugin::id() const
{
  return myId;
}

std::string Plugin::name() const
{
  return interface()->name();
}

std::string Plugin::version() const
{
  return interface()->version();
}

std::string Plugin::libraryName() const
{
  return myLibrary->getName();
}

boost::shared_ptr<Licq::PluginInterface> Plugin::internalInterface()
{
  // Create a shared_ptr that keeps this object alive at least until the
  // returned pointer goes out of scope.
  return boost::shared_ptr<Licq::PluginInterface>(
      shared_from_this(), interface().get());
}

bool Plugin::isThread(const pthread_t& thread) const
{
  return myThread->isThread(thread);
}

bool Plugin::create()
{
  myThread->createPlugin(&createThreadEntry, this);
  return !! interface();
}

bool Plugin::init(int argc, char** argv, void (*callback)(const Plugin&))
{
  assert(myInitCallback == NULL);

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

  myInitCallback = callback;
  return myThread->initPlugin(&initThreadEntry, this);
}

void Plugin::run(void (*startCallback)(const Plugin&),
                 void (*exitCallback)(const Plugin&))
{
  assert(myStartCallback == NULL && myExitCallback == NULL);
  myStartCallback = startCallback;
  myExitCallback = exitCallback;

  myThread->startPlugin(&startThreadEntry, this);
}

void Plugin::shutdown()
{
  interface()->shutdown();
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

void Plugin::createThreadEntry(void* plugin)
{
  Plugin* thisPlugin = static_cast<Plugin*>(plugin);
  thisPlugin->createInterface();
}

bool Plugin::initThreadEntry(void* plugin)
{
  Plugin* thisPlugin = static_cast<Plugin*>(plugin);

  if (thisPlugin->myInitCallback)
    thisPlugin->myInitCallback(*thisPlugin);

  // Set optind to 0 so plugins can use getopt
  optind = 0;

  return thisPlugin->interface()->init(
      thisPlugin->myArgc, thisPlugin->myArgvCopy);
}

void* Plugin::startThreadEntry(void* plugin)
{
  Plugin* thisPlugin = static_cast<Plugin*>(plugin);

  if (thisPlugin->myStartCallback != NULL)
    (*thisPlugin->myStartCallback)(*thisPlugin);

  int* retval = new int;
  *retval = thisPlugin->interface()->run();

  if (thisPlugin->myExitCallback != NULL)
    (*thisPlugin->myExitCallback)(*thisPlugin);

  return retval;
}
