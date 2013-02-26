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

#include "generalplugin.h"
#include "generalplugininstance.h"

#include <licq/plugin/generalpluginfactory.h>
#include <licq/thread/mutexlocker.h>

#include <boost/make_shared.hpp>

using namespace LicqDaemon;

GeneralPlugin::GeneralPlugin(
    DynamicLibrary::Ptr lib,
    boost::shared_ptr<Licq::GeneralPluginFactory> factory,
    PluginThread::Ptr thread)
  : Plugin(lib),
    myFactory(factory),
    myThread(thread)
{
  // Empty
}

GeneralPlugin::~GeneralPlugin()
{
  // Empty
}

boost::shared_ptr<GeneralPluginInstance> GeneralPlugin::createInstance(
    int id, void (*callback)(const PluginInstance&))
{
  assert(myThread);

  GeneralPluginInstance::Ptr instance =
      boost::make_shared<GeneralPluginInstance>(
          id, boost::dynamic_pointer_cast<GeneralPlugin>(shared_from_this()),
          myThread);
  myThread.reset();

  if (instance->create(callback))
    registerInstance(instance);
  else
    instance.reset();

  return instance;
}

boost::shared_ptr<Licq::GeneralPluginFactory>
GeneralPlugin::generalFactory()
{
  return myFactory;
}

std::string GeneralPlugin::description() const
{
  return myFactory->description();
}

std::string GeneralPlugin::usage() const
{
  return myFactory->usage();
}

std::string GeneralPlugin::configFile() const
{
  return myFactory->configFile();
}

Licq::GeneralPluginInstance::Ptr GeneralPlugin::instance() const
{
  Licq::MutexLocker locker(myMutex);
  assert(myInstances.size() == 1);
  return boost::dynamic_pointer_cast<Licq::GeneralPluginInstance>(
      myInstances.front().lock());
}

boost::shared_ptr<Licq::PluginFactory> GeneralPlugin::factory()
{
  return myFactory;
}

boost::shared_ptr<const Licq::PluginFactory> GeneralPlugin::factory() const
{
  return myFactory;
}
