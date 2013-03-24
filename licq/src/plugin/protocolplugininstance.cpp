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

#include "protocolplugininstance.h"

#include <licq/plugin/protocolpluginfactory.h>
#include <licq/plugin/protocolplugininterface.h>

using namespace LicqDaemon;

static void nullDeleter(void*) { /* Empty */ }

ProtocolPluginInstance::ProtocolPluginInstance(
    int id, const Licq::UserId& ownerId, ProtocolPlugin::Ptr plugin,
    PluginThread::Ptr thread)
  : PluginInstance(id, thread),
    myOwnerId(ownerId),
    myPlugin(plugin)
{
  // Empty
}

ProtocolPluginInstance::~ProtocolPluginInstance()
{
  if (myInterface)
    myPlugin->protocolFactory()->destroyPlugin(myInterface.get());
}

void ProtocolPluginInstance::run(void (*startCallback)(const PluginInstance&),
                                 void (*exitCallback)(const PluginInstance&))
{
  PluginInstance::run(startCallback, exitCallback);
}

boost::shared_ptr<Licq::ProtocolPlugin> ProtocolPluginInstance::plugin() const
{
  return myPlugin;
}

void ProtocolPluginInstance::pushSignal(
    boost::shared_ptr<const Licq::ProtocolSignal> signal)
{
  if (isRunning())
    myInterface->pushSignal(signal);
}

void ProtocolPluginInstance::createInterface()
{
  assert(!myInterface);
  myInterface.reset(myPlugin->protocolFactory()->createPlugin(), &nullDeleter);
}

boost::shared_ptr<Licq::PluginInterface> ProtocolPluginInstance::interface()
{
  return myInterface;
}

boost::shared_ptr<const Licq::PluginInterface>
ProtocolPluginInstance::interface() const
{
  return myInterface;
}
