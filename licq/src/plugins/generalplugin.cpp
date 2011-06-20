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

#include "generalplugin.h"

#include <licq/thread/mutexlocker.h>

#include <cstring>

using Licq::MutexLocker;
using namespace LicqDaemon;
using namespace std;

GeneralPlugin::GeneralPlugin(DynamicLibrary::Ptr lib,
                             PluginThread::Ptr pluginThread) :
  Plugin(lib, pluginThread, "LP"),
  mySignalMask(0)
{
  loadSymbol("LP_Status", myStatus);
  loadSymbol("LP_Description", myDescription);
  loadSymbol("LP_Usage", myUsage);
}

GeneralPlugin::~GeneralPlugin()
{
  // Empty
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

bool GeneralPlugin::wantSignal(unsigned long signalType) const
{
  return (signalType & mySignalMask);
}

void GeneralPlugin::setSignalMask(unsigned long signalMask)
{
  mySignalMask = signalMask;
}

bool GeneralPlugin::isEnabled() const
{
  const char* strStatus = (*myStatus)();
  return (strstr(strStatus, "enabled") != NULL || strstr(strStatus, "running"));
}

string GeneralPlugin::description() const
{
  return (*myDescription)();
}

string GeneralPlugin::usage() const
{
  return (*myUsage)();
}

void GeneralPlugin::enable()
{
  myPipe.putChar(PipeEnable);
}

void GeneralPlugin::disable()
{
  myPipe.putChar(PipeDisable);
}
