/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2013 Licq Developers <licq-dev@googlegroups.com>
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

#include "factory.h"

#include <licq/plugin/protocolplugin.h>
#include <licq/version.h>

#include "icqprotocolplugin.h"
#include "owner.h"
#include "pluginversion.h"
#include "user.h"

using namespace LicqIcq;


std::string Factory::name() const
{
  return "ICQ";
}

std::string Factory::version() const
{
  return PLUGIN_VERSION_STRING;
}

void Factory::destroyPlugin(Licq::PluginInterface* plugin)
{
  delete plugin;
}

unsigned long Factory::protocolId() const
{
  return ICQ_PPID;
}

unsigned long Factory::capabilities() const
{
  using Licq::ProtocolPlugin;

  return ProtocolPlugin::CanSendMsg
      | ProtocolPlugin::CanSendUrl
      | ProtocolPlugin::CanSendFile
      | ProtocolPlugin::CanSendChat
      | ProtocolPlugin::CanSendContact
      | ProtocolPlugin::CanSendAuth
      | ProtocolPlugin::CanSendAuthReq
      | ProtocolPlugin::CanSendSms
      | ProtocolPlugin::CanSendSecure
      | ProtocolPlugin::CanSendDirect
      | ProtocolPlugin::CanHoldStatusMsg
      | ProtocolPlugin::CanVaryEncoding
      | ProtocolPlugin::CanSingleGroup;
}

Licq::ProtocolPluginInterface* Factory::createPlugin()
{
  return new IcqProtocolPlugin;
}

Licq::User* Factory::createUser(const Licq::UserId& id, bool temporary)
{
  return new User(id, temporary);
}

Licq::Owner* Factory::createOwner(const Licq::UserId& id)
{
  return new Owner(id);
}


static Licq::ProtocolPluginFactory* createFactory()
{
  static LicqIcq::Factory factory;
  return &factory;
}

static void destroyFactory(Licq::ProtocolPluginFactory*)
{
  // Empty
}

LICQ_PROTOCOL_PLUGIN_DATA(&createFactory, &destroyFactory);
