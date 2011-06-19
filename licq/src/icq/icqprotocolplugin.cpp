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

#include "icq.h"

#include <unistd.h>

#include <licq/pluginmanager.h>
#include <licq/version.h>

#define LProto_Name LProto_icq_Name
#define LProto_Version LProto_icq_Version
#define LProto_PPID LProto_icq_PPID
#define LProto_Init LProto_icq_Init
#define LProto_SendFuncs LProto_icq_SendFuncs
#define LProto_Main LProto_icq_Main
#define LProto_DefSrvHost LProto_icq_DefSrvHost
#define LProto_DefSrvPort LProto_icq_DefSrvPort

#include <licq/protocolbase.h>

using Licq::gPluginManager;
using Licq::ProtocolPlugin;

const char* LProto_icq_Name()
{
  static char name[] = "ICQ";
  return name;
}

const char* LProto_icq_Version()
{
  static char version[] = LICQ_VERSION_STRING;
  return version;
}

const char* LProto_icq_PPID()
{
  static char ppid[] = "Licq";
  return ppid;
}

bool LProto_icq_Init()
{
  gIcqProtocol.initialize();
  return true;
}

unsigned long LProto_icq_SendFuncs()
{
  return ProtocolPlugin::CanSendMsg | ProtocolPlugin::CanSendUrl |
      ProtocolPlugin::CanSendFile | ProtocolPlugin::CanSendChat |
      ProtocolPlugin::CanSendContact | ProtocolPlugin::CanSendAuth |
      ProtocolPlugin::CanSendAuthReq | ProtocolPlugin::CanSendSms |
      ProtocolPlugin::CanSendSecure | ProtocolPlugin::CanSendDirect |
      ProtocolPlugin::CanHoldStatusMsg;
}

const char* LProto_icq_DefSrvHost()
{
  static char defaultHost[] = "login.icq.com";
  return defaultHost;
}

int LProto_icq_DefSrvPort()
{
  return 5190;
}

int LProto_icq_Main()
{
  if (!gIcqProtocol.start())
    return 1;
  return 0;
}
