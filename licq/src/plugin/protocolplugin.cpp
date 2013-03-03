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

#include "protocolplugin.h"
#include "protocolplugininstance.h"

#include <licq/plugin/protocolpluginfactory.h>
#include <licq/thread/mutexlocker.h>

#include <boost/make_shared.hpp>

using namespace LicqDaemon;

ProtocolPlugin::ProtocolPlugin(
    DynamicLibrary::Ptr lib,
    boost::shared_ptr<Licq::ProtocolPluginFactory> factory)
  : Plugin(lib),
    myFactory(factory)
{
  // Empty
}

ProtocolPlugin::~ProtocolPlugin()
{
  // Empty
}

boost::shared_ptr<ProtocolPluginInstance> ProtocolPlugin::createInstance(
    int id, PluginThread::Ptr thread)
{
  ProtocolPluginInstance::Ptr instance =
      boost::make_shared<ProtocolPluginInstance>(
          id, boost::dynamic_pointer_cast<ProtocolPlugin>(shared_from_this()),
          thread);

  if (instance->create())
    registerInstance(instance);
  else
    instance.reset();

  return instance;
}

boost::shared_ptr<Licq::ProtocolPluginFactory>
ProtocolPlugin::protocolFactory()
{
  return myFactory;
}

unsigned long ProtocolPlugin::protocolId() const
{
  return myFactory->protocolId();
}

unsigned long ProtocolPlugin::capabilities() const
{
  return myFactory->capabilities();
}

Licq::ProtocolPlugin::Instances ProtocolPlugin::instances() const
{
  Instances list;

  Licq::MutexLocker locker(myMutex);

  for (std::vector< boost::weak_ptr<PluginInstance> >::const_iterator it =
           myInstances.begin(); it != myInstances.end(); ++it)
  {
    ProtocolPluginInstance::Ptr instance =
        boost::dynamic_pointer_cast<ProtocolPluginInstance>(it->lock());
    if (instance)
      list.push_back(instance);
  }

  return list;
}

Licq::User* ProtocolPlugin::createUser(const Licq::UserId& id, bool temporary)
{
  return myFactory->createUser(id, temporary);
}

Licq::Owner* ProtocolPlugin::createOwner(const Licq::UserId& id)
{
  return myFactory->createOwner(id);
}

boost::shared_ptr<const Licq::PluginFactory> ProtocolPlugin::factory() const
{
  return myFactory;
}
