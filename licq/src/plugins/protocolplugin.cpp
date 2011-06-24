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


ProtocolPlugin::ProtocolPlugin(Params& p, bool icq)
  : Plugin(p, icq ? "LProto_icq" : "LProto"),
    myPrivate(new Private)
{
  LICQ_D();

  std::string prefix = (icq ? "LProto_icq" : "LProto");
  loadSymbol(prefix + "_PPID", (void**)(&d->myPpid));
  loadSymbol(prefix + "_SendFuncs", (void**)(&d->mySendFunctions));

  const char* (*getDefaultHost)() = NULL;
  loadSymbol(prefix + "_DefSrvHost", (void**)(&getDefaultHost));
  if (getDefaultHost != NULL)
    d->myDefaultHost = (*getDefaultHost)();

  int (*getDefaultPort)() = NULL;
  loadSymbol(prefix + "_DefSrvPort", (void**)(&getDefaultPort));
  d->myDefaultPort = (getDefaultPort == NULL ? 0 : (*getDefaultPort)() );

  const char* ppid = (*d->myPpid)();
  d->myProtocolId = ppid[0] << 24 | ppid[1] << 16 | ppid[2] << 8 | ppid[3];
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

unsigned long ProtocolPlugin::protocolId() const
{
  LICQ_D_CONST();
  return d->myProtocolId;
}

unsigned long ProtocolPlugin::capabilities() const
{
  LICQ_D_CONST();
  return (*d->mySendFunctions)();
}

string ProtocolPlugin::defaultServerHost() const
{
  LICQ_D_CONST();
  return d->myDefaultHost;
}

int ProtocolPlugin::defaultServerPort() const
{
  LICQ_D_CONST();
  return d->myDefaultPort;
}
