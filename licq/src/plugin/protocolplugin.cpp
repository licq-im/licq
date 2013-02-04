/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq developers <licq-dev@googlegroups.com>
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

#include <licq/plugin/protocolplugininterface.h>
#include "protocolplugin.h"

using namespace LicqDaemon;

static void destroyProtocolPluginInterface(
    Licq::ProtocolPluginInterface* plugin)
{
  if (plugin != NULL)
    plugin->destructor();
}

ProtocolPlugin::ProtocolPlugin(
    int id, DynamicLibrary::Ptr lib, PluginThread::Ptr thread,
    Licq::ProtocolPluginInterface* (*factory)())
  : Plugin(id, lib, thread),
    myInterface((*factory)(), &destroyProtocolPluginInterface)
{
  if (!myInterface)
    throw std::exception();
}

ProtocolPlugin::ProtocolPlugin(
    int id, DynamicLibrary::Ptr lib, PluginThread::Ptr thread,
    boost::shared_ptr<Licq::ProtocolPluginInterface> interface)
  : Plugin(id, lib, thread),
    myInterface(interface)
{
  if (!myInterface)
    throw std::exception();
}

ProtocolPlugin::~ProtocolPlugin()
{
  // Empty
}

unsigned long ProtocolPlugin::protocolId() const
{
  return myInterface->protocolId();
}

unsigned long ProtocolPlugin::capabilities() const
{
  return myInterface->capabilities();
}

void ProtocolPlugin::pushSignal(
    boost::shared_ptr<const Licq::ProtocolSignal> signal)
{
  myInterface->pushSignal(signal);
}

Licq::User* ProtocolPlugin::createUser(const Licq::UserId& id, bool temporary)
{
  return myInterface->createUser(id, temporary);
}

Licq::Owner* ProtocolPlugin::createOwner(const Licq::UserId& id)
{
  return myInterface->createOwner(id);
}

boost::shared_ptr<Licq::PluginInterface> ProtocolPlugin::interface()
{
  return myInterface;
}

boost::shared_ptr<const Licq::PluginInterface>
ProtocolPlugin::interface() const
{
  return myInterface;
}
