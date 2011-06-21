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

#include <licq/plugin.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <licq/pipe.h>

#include "utils/dynamiclibrary.h"
#include "pluginthread.h"

// From licq.cpp
extern char** global_argv;

using LicqDaemon::DynamicLibrary;
using LicqDaemon::PluginThread;
using namespace Licq;
using namespace std;


class Plugin::Private
{
public:
  Private(int id, DynamicLibrary::Ptr lib, PluginThread::Ptr thread);
  ~Private();

  /// Entry point for calling init() in plugin's thread
  static bool initThreadEntry(void* plugin);

  /// Entry point for calling run() in plugin's thread
  static void* startThreadEntry(void* plugin);

  const int myId;
  DynamicLibrary::Ptr myLib;
  Licq::Pipe myPipe;

  PluginThread::Ptr myThread;
  void (*myInitCallback)(const Plugin&);
  void (*myStartCallback)(const Plugin&);
  void (*myExitCallback)(const Plugin&);

  int myArgc;
  char** myArgv;
  char** myArgvCopy;
};

Plugin::Private::Private(int id, DynamicLibrary::Ptr lib, PluginThread::Ptr thread)
  : myId(id),
    myLib(lib),
    myThread(thread),
    myInitCallback(NULL),
    myStartCallback(NULL),
    myExitCallback(NULL),
    myArgc(0),
    myArgv(NULL),
    myArgvCopy(NULL)
{
  // Empty
}

Plugin::Private::~Private()
{
  for (int i = 0; i < myArgc; ++i)
    ::free(myArgv[i]);
  delete[] myArgv;
  delete[] myArgvCopy;
}

Plugin::Plugin(int id, LibraryPtr lib, ThreadPtr thread)
  : myPrivate(new Private(id, lib, thread))
{
  // Empty
}

Plugin::~Plugin()
{
  delete myPrivate;
}

bool Plugin::isThread(const pthread_t& thread) const
{
  LICQ_D();
  return d->myThread->isThread(thread);
}

bool Plugin::callInit(int argc, char** argv,
                         void (*callback)(const Plugin&))
{
  LICQ_D();
  assert(d->myInitCallback == NULL);

  const size_t size = argc + 2;

  d->myArgv = new char*[size];
  d->myArgvCopy = new char*[size];

  d->myArgv[size - 1] = NULL;

  // TODO: use licq or libname?
  //d->myArgv[0] = ::strdup(d->myLib->getName().c_str());
  d->myArgv[0] = ::strdup(global_argv[0]);

  for (int i = 0; i < argc; ++i)
    d->myArgv[i + 1] = ::strdup(argv[i]);

  d->myArgc = argc + 1;

  // We need to create a copy of myArgv and pass that to the plugin, since
  // e.g. KDE changes the pointers in argv (e.g. to strip the path in argv[0])
  // and that messes up free, causing SIGSEGV in the destructor.
  ::memcpy(d->myArgvCopy, d->myArgv, size * sizeof(char*));

  d->myInitCallback = callback;
  return d->myThread->initPlugin(&Private::initThreadEntry, this);
}

void Plugin::startThread(
    void (*startCallback)(const Plugin& plugin),
    void (*exitCallback)(const Plugin& plugin))
{
  LICQ_D();
  assert(d->myStartCallback == NULL && d->myExitCallback == NULL);
  d->myStartCallback = startCallback;
  d->myExitCallback = exitCallback;
  d->myThread->startPlugin(&Private::startThreadEntry, this);
}

int Plugin::joinThread()
{
  LICQ_D();
  void* result = d->myThread->join();
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
  LICQ_D();
  d->myThread->cancel();
}

int Plugin::id() const
{
  LICQ_D_CONST();
  return d->myId;
}

string Plugin::configFile() const
{
  return string();
}

const string& Plugin::libraryName() const
{
  LICQ_D_CONST();
  return d->myLib->getName();
}

void Plugin::shutdown()
{
  LICQ_D();
  d->myPipe.putChar(PipeShutdown);
}

int Plugin::getReadPipe() const
{
  LICQ_D_CONST();
  return d->myPipe.getReadFd();
}

void Plugin::notify(char c)
{
  LICQ_D();
  d->myPipe.putChar(c);
}

DynamicLibrary::Ptr Plugin::library()
{
  LICQ_D();
  return d->myLib;
}

bool Plugin::Private::initThreadEntry(void* plugin)
{
  Plugin* thisPlugin = static_cast<Plugin*>(plugin);
  Plugin::Private* const d = thisPlugin->myPrivate;

  if (d->myInitCallback)
    d->myInitCallback(*thisPlugin);

  // Set optind to 0 so plugins can use getopt
  optind = 0;

  return thisPlugin->init(d->myArgc, d->myArgvCopy);
}

void* Plugin::Private::startThreadEntry(void* plugin)
{
  Plugin* thisPlugin = static_cast<Plugin*>(plugin);
  Plugin::Private* const d = thisPlugin->myPrivate;

  if (d->myStartCallback != NULL)
    (*d->myStartCallback)(*thisPlugin);

  int* retval = new int;
  *retval = thisPlugin->run();

  if (d->myExitCallback != NULL)
    (*d->myExitCallback)(*thisPlugin);

  return retval;
}
