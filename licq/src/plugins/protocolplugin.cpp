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

using namespace LicqDaemon;

ProtocolPlugin::ProtocolPlugin(DynamicLibrary::Ptr lib)
  : Plugin(lib, "LProto")
{
  loadSymbol("LProto_Init", myInit);
  loadSymbol("LProto_PPID", myPpid);
  loadSymbol("LProto_SendFuncs", mySendFunctions);

  const char* ppid = (*myPpid)();
  myProtocolId = ppid[0] << 24 | ppid[1] << 16 | ppid[2] << 8 | ppid[3];
}

ProtocolPlugin::~ProtocolPlugin()
{
  // Empty
}

unsigned long ProtocolPlugin::getProtocolId() const
{
  return myProtocolId;
}

unsigned long ProtocolPlugin::getSendFunctions() const
{
  return (*mySendFunctions)();
}
