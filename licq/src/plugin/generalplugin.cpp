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

using namespace Licq;
using namespace std;


GeneralPlugin::Private::Private() :
    mySignalMask(0)
{
  // Empty
}


GeneralPlugin::GeneralPlugin(Params& p)
  : Plugin(p),
    myPrivate(new Private())
{
  // Empty
}

GeneralPlugin::~GeneralPlugin()
{
  delete myPrivate;
}

void GeneralPlugin::pushSignal(PluginSignal* signal)
{
  LICQ_D();
  MutexLocker locker(d->mySignalsMutex);
  d->mySignals.push(signal);
  notify(PipeSignal);
}

PluginSignal* GeneralPlugin::popSignal()
{
  LICQ_D();
  MutexLocker locker(d->mySignalsMutex);
  if (!d->mySignals.empty())
  {
    PluginSignal* signal = d->mySignals.front();
    d->mySignals.pop();
    return signal;
  }
  return NULL;
}

void GeneralPlugin::pushEvent(Event* event)
{
  LICQ_D();
  MutexLocker locker(d->myEventsMutex);
  d->myEvents.push(event);
  notify(PipeEvent);
}

Event* GeneralPlugin::popEvent()
{
  LICQ_D();
  MutexLocker locker(d->myEventsMutex);
  if (!d->myEvents.empty())
  {
    Event* event = d->myEvents.front();
    d->myEvents.pop();
    return event;
  }
  return NULL;
}

bool GeneralPlugin::isEnabled() const
{
  return true;
}

bool GeneralPlugin::wantSignal(unsigned long signalType) const
{
  LICQ_D_CONST();
  return (signalType & d->mySignalMask);
}

void GeneralPlugin::setSignalMask(unsigned long signalMask)
{
  LICQ_D();
  d->mySignalMask = signalMask;
}

void GeneralPlugin::enable()
{
  notify(PipeEnable);
}

void GeneralPlugin::disable()
{
  notify(PipeDisable);
}
