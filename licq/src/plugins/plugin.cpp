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
#include <cstdlib>
#include <cstring>
#include <unistd.h>

// From licq.cpp
extern char** global_argv;

using LicqDaemon::DynamicLibrary;
using namespace Licq;
using namespace std;


Plugin::Private::Private(Plugin* plugin, int id,
    LicqDaemon::DynamicLibrary::Ptr lib, LicqDaemon::PluginThread::Ptr thread)
  : myPlugin(plugin),
    myId(id),
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

Plugin::Plugin(Params& p)
  : myPrivate(new Private(this, p.myId, p.myLib, p.myThread))
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

bool Plugin::Private::callInit(int argc, char** argv,
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
  return myThread->initPlugin(&Private::initThreadEntry, myPlugin);
}

void Plugin::Private::startThread(
    void (*startCallback)(const Plugin& plugin),
    void (*exitCallback)(const Plugin& plugin))
{
  assert(myStartCallback == NULL && myExitCallback == NULL);
  myStartCallback = startCallback;
  myExitCallback = exitCallback;
  myThread->startPlugin(&Private::startThreadEntry, myPlugin);
}

int Plugin::Private::joinThread()
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

void Plugin::Private::cancelThread()
{
  myThread->cancel();
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
  notify(PipeShutdown);
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
