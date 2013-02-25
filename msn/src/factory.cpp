/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "msn.h"
#include "owner.h"
#include "pluginversion.h"
#include "user.h"

namespace LicqMsn
{

class Factory : public Licq::ProtocolPluginFactory
{
public:
  // From Licq::PluginFactory
  std::string name() const { return "MSN"; }
  std::string version() const { return PLUGIN_VERSION_STRING; }
  void destroyPlugin(Licq::PluginInterface* plugin) { delete plugin; }

  // From Licq::ProtocolPluginFactory
  unsigned long protocolId() const { return MSN_PPID; }
  unsigned long capabilities() const;
  Licq::ProtocolPluginInterface* createPlugin();
  Licq::User* createUser(const Licq::UserId& id, bool temporary);
  Licq::Owner* createOwner(const Licq::UserId& id);
};

unsigned long Factory::capabilities() const
{
  using Licq::ProtocolPlugin;

  return ProtocolPlugin::CanSendMsg
      | ProtocolPlugin::CanSendAuth
      | ProtocolPlugin::CanSendAuthReq
      | ProtocolPlugin::CanMultipleOwners;
}

Licq::ProtocolPluginInterface* Factory::createPlugin()
{
  return new CMSN;
}

Licq::User* Factory::createUser(const Licq::UserId& id, bool temporary)
{
  return new User(id, temporary);
}

Licq::Owner* Factory::createOwner(const Licq::UserId& id)
{
  return new Owner(id);
}

} // namespace LicqMsn

static Licq::ProtocolPluginFactory* createFactory()
{
  static LicqMsn::Factory factory;
  return &factory;
}

static void destroyFactory(Licq::ProtocolPluginFactory*)
{
  // Empty
}

LICQ_PROTOCOL_PLUGIN_DATA(&createFactory, &destroyFactory);
