/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2010 Licq developers
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

#include <licq/pluginmanager.h>
#include <licq/protocolbase.h>

#include "pluginversion.h"
#include "msn.h"

using Licq::gPluginManager;

char *LProto_Name()
{
  static char szName[] = "MSN";
  return szName;
}

char *LProto_Version()
{
  static char szVersion[] = PLUGIN_VERSION_STRING;
  return szVersion;
}

const char *LProto_Description()
{
  static char szDesc[] = "MSN Protocol Plugin";
  return szDesc;
}

unsigned long LProto_SendFuncs()
{
  return Licq::ProtocolPlugin::CanSendMsg |
      Licq::ProtocolPlugin::CanSendAuth |
      Licq::ProtocolPlugin::CanSendAuthReq;
}

unsigned long LProto_Capabilities()
{
  return 0;
}

int LProto_Main()
{
  int nPipe = gPluginManager.registerProtocolPlugin();

  CMSN* pMSN = new CMSN(nPipe);
  pMSN->Run();

  gPluginManager.unregisterProtocolPlugin();

  delete pMSN;
  
  return 0;
}

char *LProto_PPID()
{
  static char szId[] = "MSN_";
  return szId;
}

bool LProto_Init()
{
  return true;
}
