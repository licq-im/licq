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

#include <licq/plugin/protocolplugin.h>
#include <licq/plugin/protocolpluginfactory.h>
#include <licq/version.h>

#include "icqprotocolplugin.h"
#include "owner.h"
#include "pluginversion.h"
#include "user.h"

namespace LicqIcq
{

class Factory : public Licq::ProtocolPluginFactory
{
public:
  // From Licq::PluginFactory
  std::string name() const { return "ICQ"; }
  std::string version() const { return PLUGIN_VERSION_STRING; }
  void destroyPlugin(Licq::PluginInterface* plugin) { delete plugin; }

  // From Licq::ProtocolPluginFactory
  unsigned long protocolId() const { return ICQ_PPID; }
  unsigned long capabilities() const;
  Licq::ProtocolPluginInterface* createPlugin();
  Licq::User* createUser(const Licq::UserId& id, bool temporary);
  Licq::Owner* createOwner(const Licq::UserId& id);
};

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

} // namespace LicqIcq

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
