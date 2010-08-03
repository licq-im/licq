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

#include "generalplugin.h"

#include <licq/thread/mutexlocker.h>

#include <cstring>

// From licq.cpp
extern char** global_argv;

using Licq::MutexLocker;
using namespace LicqDaemon;

GeneralPlugin::GeneralPlugin(DynamicLibrary::Ptr lib,
                             PluginThread::Ptr pluginThread) :
  Plugin(lib, pluginThread, "LP"),
  myArgc(0),
  myArgv(NULL),
  myArgvCopy(NULL)
{
  loadSymbol("LP_Init", myInit);
  loadSymbol("LP_Status", myStatus);
  loadSymbol("LP_Description", myDescription);
  loadSymbol("LP_Usage", myUsage);
  loadSymbol("LP_BuildDate", myBuildDate);
  loadSymbol("LP_BuildTime", myBuildTime);

  try
  {
    // LP_ConfigFile is not required
    loadSymbol("LP_ConfigFile", myConfigFile);
  }
  catch (DynamicLibrary::Exception&)
  {
    myConfigFile = NULL;
  }
}

GeneralPlugin::~GeneralPlugin()
{
  for (int i = 0; i < myArgc; ++i)
    ::free(myArgv[i]);
  delete[] myArgv;
  delete[] myArgvCopy;
}

bool GeneralPlugin::init(int argc, char** argv)
{
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

  return callInitInThread();
}

void GeneralPlugin::pushSignal(Licq::PluginSignal* signal)
{
  MutexLocker locker(mySignalsMutex);
  mySignals.push(signal);
  locker.unlock();
  myPipe.putChar(PipeSignal);
}

Licq::PluginSignal* GeneralPlugin::popSignal()
{
  MutexLocker locker(mySignalsMutex);
  if (!mySignals.empty())
  {
    Licq::PluginSignal* signal = mySignals.front();
    mySignals.pop();
    return signal;
  }
  return NULL;
}

void GeneralPlugin::pushEvent(Licq::Event* event)
{
  MutexLocker locker(myEventsMutex);
  myEvents.push(event);
  locker.unlock();
  myPipe.putChar(PipeEvent);
}

Licq::Event* GeneralPlugin::popEvent()
{
  MutexLocker locker(myEventsMutex);
  if (!myEvents.empty())
  {
    Licq::Event* event = myEvents.front();
    myEvents.pop();
    return event;
  }
  return NULL;
}

const char* GeneralPlugin::getStatus() const
{
  return (*myStatus)();
}

const char* GeneralPlugin::getDescription() const
{
  return (*myDescription)();
}

const char* GeneralPlugin::getUsage() const
{
  return (*myUsage)();
}

const char* GeneralPlugin::getConfigFile() const
{
  if (myConfigFile)
    return (*myConfigFile)();
  else
    return NULL;
}

const char* GeneralPlugin::getBuildDate() const
{
  return (*myBuildDate)();
}

const char* GeneralPlugin::getBuildTime() const
{
  return (*myBuildTime)();
}

void GeneralPlugin::enable()
{
  myPipe.putChar(PipeEnable);
}

void GeneralPlugin::disable()
{
  myPipe.putChar(PipeDisable);
}

bool GeneralPlugin::initThreadEntry()
{
  // Set optind to 0 so plugins can use getopt
  optind = 0;

  return (*myInit)(myArgc, myArgvCopy);
}
