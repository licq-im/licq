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

#include <licq/plugin/generalpluginfactory.h>
#include <licq/plugin/generalplugininterface.h>

using namespace LicqDaemon;

static void nullDeleter(void*) { /* Empty */ }

GeneralPlugin::GeneralPlugin(
    int id, DynamicLibrary::Ptr lib, PluginThread::Ptr thread,
    boost::shared_ptr<Licq::GeneralPluginFactory> factory)
  : Plugin(id, lib, thread),
    myFactory(factory)
{
  // Empty
}

GeneralPlugin::~GeneralPlugin()
{
  if (myInterface)
    myFactory->destroyPlugin(myInterface.get());
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

bool GeneralPlugin::isEnabled() const
{
  return myInterface->isEnabled();
}

void GeneralPlugin::enable()
{
  if (isRunning())
    myInterface->enable();
}

void GeneralPlugin::disable()
{
  if (isRunning())
    myInterface->disable();
}

bool GeneralPlugin::wantSignal(unsigned long signalType) const
{
  return myInterface->wantSignal(signalType);
}

void GeneralPlugin::pushSignal(
    boost::shared_ptr<const Licq::PluginSignal> signal)
{
  if (isRunning())
    myInterface->pushSignal(signal);
}

void GeneralPlugin::pushEvent(boost::shared_ptr<const Licq::Event> event)
{
  if (isRunning())
    myInterface->pushEvent(event);
}

void GeneralPlugin::createInterface()
{
  assert(!myInterface);
  myInterface.reset(myFactory->createPlugin(), &nullDeleter);
}

boost::shared_ptr<const Licq::PluginFactory> GeneralPlugin::factory() const
{
  return myFactory;
}

boost::shared_ptr<Licq::PluginInterface> GeneralPlugin::interface()
{
  return myInterface;
}

boost::shared_ptr<const Licq::PluginInterface> GeneralPlugin::interface() const
{
  return myInterface;
}
