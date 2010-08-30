/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq Developers <licq-dev@googlegroups.com>
 *
 * Please refer to the COPYRIGHT file distributed with this source
 * distribution for the names of the individual contributors.
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

#include "config.h"
#include "plugin.h"
#include "pluginversion.h"

#include <licq/pluginmanager.h>
#include <licq/protocolbase.h>

char* LProto_Name()
{
  static char name[] = "Jabber";
  return name;
}

char* LProto_Version()
{
  static char version[] = PLUGIN_VERSION_STRING;
  return version;
}

char* LProto_PPID()
{
  static char ppid[] = "XMPP";
  return ppid;
}

bool LProto_Init()
{
  return true;
}

unsigned long LProto_SendFuncs()
{
  return Licq::ProtocolPlugin::CanSendMsg
      | Licq::ProtocolPlugin::CanHoldStatusMsg
      | Licq::ProtocolPlugin::CanSendAuth
      | Licq::ProtocolPlugin::CanSendAuthReq;
}

int LProto_Main()
{
  Jabber::Config config("licq_jabber.conf");

  int pipe = Licq::gPluginManager.registerProtocolPlugin();
  int res = Jabber::Plugin(config).run(pipe);
  Licq::gPluginManager.unregisterProtocolPlugin();
  return res;
}
