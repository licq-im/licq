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

#include "protocolplugin.h"

#include <licq/thread/mutexlocker.h>

using namespace Licq;
using namespace std;


ProtocolPlugin::Private::Private()
{
  // Empty
}


ProtocolPlugin::ProtocolPlugin(Params& p)
  : Plugin(p),
    myPrivate(new Private())
{
  // Empty
}

ProtocolPlugin::~ProtocolPlugin()
{
  delete myPrivate;
}

void ProtocolPlugin::pushSignal(ProtocolSignal* signal)
{
  LICQ_D();
  MutexLocker locker(d->mySignalsMutex);
  d->mySignals.push(signal);
  notify(PipeSignal);
}

ProtocolSignal* ProtocolPlugin::popSignal()
{
  LICQ_D();
  MutexLocker locker(d->mySignalsMutex);
  if (!d->mySignals.empty())
  {
    ProtocolSignal* signal = d->mySignals.front();
    d->mySignals.pop();
    return signal;
  }
  return NULL;
}
