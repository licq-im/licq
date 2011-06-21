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

using Licq::MutexLocker;
using namespace std;
using namespace LicqDaemon;

ProtocolPlugin::ProtocolPlugin(DynamicLibrary::Ptr lib,
                               PluginThread::Ptr pluginThread,
                               bool icq) :
  Plugin(lib, pluginThread, icq ? "LProto_icq" : "LProto")
{
  std::string prefix = (icq ? "LProto_icq" : "LProto");
  loadSymbol(prefix + "_PPID", myPpid);
  loadSymbol(prefix + "_SendFuncs", mySendFunctions);

  const char* (*getDefaultHost)();
  loadSymbol(prefix + "_DefSrvHost", getDefaultHost);
  if (getDefaultHost != NULL)
    myDefaultHost = (*getDefaultHost)();

  int (*getDefaultPort)();
  loadSymbol(prefix + "_DefSrvPort", getDefaultPort);
  myDefaultPort = (getDefaultPort == NULL ? 0 : (*getDefaultPort)() );

  const char* ppid = (*myPpid)();
  myProtocolId = ppid[0] << 24 | ppid[1] << 16 | ppid[2] << 8 | ppid[3];
}

ProtocolPlugin::~ProtocolPlugin()
{
  // Empty
}

void ProtocolPlugin::pushSignal(Licq::ProtocolSignal* signal)
{
  MutexLocker locker(mySignalsMutex);
  mySignals.push(signal);
  locker.unlock();
  myPipe.putChar(PipeSignal);
}

Licq::ProtocolSignal* ProtocolPlugin::popSignal()
{
  MutexLocker locker(mySignalsMutex);
  if (!mySignals.empty())
  {
    Licq::ProtocolSignal* signal = mySignals.front();
    mySignals.pop();
    return signal;
  }
  return NULL;
}

unsigned long ProtocolPlugin::protocolId() const
{
  return myProtocolId;
}

unsigned long ProtocolPlugin::capabilities() const
{
  return (*mySendFunctions)();
}

string ProtocolPlugin::defaultServerHost() const
{
  return myDefaultHost;
}

int ProtocolPlugin::defaultServerPort() const
{
  return myDefaultPort;
}
