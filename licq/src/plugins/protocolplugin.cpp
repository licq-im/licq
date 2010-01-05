/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Erik Johansson <erijo@licq.org>
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

#include "licq_constants.h"
#include "licq/thread/mutexlocker.h"

using Licq::MutexLocker;
using namespace LicqDaemon;

ProtocolPlugin::ProtocolPlugin(DynamicLibrary::Ptr lib, bool icq)
  : Plugin(lib, icq ? "LProto_icq" : "LProto", icq)
{
  std::string prefix = (icq ? "LProto_icq" : "LProto");
  loadSymbol(prefix + "_Init", myInit);
  loadSymbol(prefix + "_PPID", myPpid);
  loadSymbol(prefix + "_SendFuncs", mySendFunctions);

  const char* ppid = (*myPpid)();
  myProtocolId = ppid[0] << 24 | ppid[1] << 16 | ppid[2] << 8 | ppid[3];
}

ProtocolPlugin::~ProtocolPlugin()
{
  // Empty
}

void ProtocolPlugin::pushSignal(LicqProtoSignal* signal)
{
  MutexLocker locker(mySignalsMutex);
  mySignals.push_back(signal);
  locker.unlock();
  myPipe.putChar(PLUGIN_SIGNAL);
}

LicqProtoSignal* ProtocolPlugin::popSignal()
{
  MutexLocker locker(mySignalsMutex);
  if (!mySignals.empty())
  {
    LicqProtoSignal* signal = mySignals.front();
    mySignals.pop_front();
    return signal;
  }
  return NULL;
}

unsigned long ProtocolPlugin::getProtocolId() const
{
  return myProtocolId;
}

unsigned long ProtocolPlugin::getSendFunctions() const
{
  return (*mySendFunctions)();
}
