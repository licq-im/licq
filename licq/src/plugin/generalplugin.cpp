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

#include <licq/plugin/generalplugininterface.h>
#include "generalplugin.h"

using namespace LicqDaemon;

static void destroyGeneralPluginInterface(Licq::GeneralPluginInterface* plugin)
{
  if (plugin != NULL)
    plugin->destructor();
}

GeneralPlugin::GeneralPlugin(
    int id, DynamicLibrary::Ptr lib, PluginThread::Ptr thread,
    Licq::GeneralPluginInterface* (*factory)())
  : Plugin(id, lib, thread),
    myFactory(factory)
{
  // Empty
}

GeneralPlugin::GeneralPlugin(
    int id, DynamicLibrary::Ptr lib, PluginThread::Ptr thread,
    boost::shared_ptr<Licq::GeneralPluginInterface> interface)
  : Plugin(id, lib, thread),
    myFactory(NULL),
    myInterface(interface)
{
  // Empty
}

GeneralPlugin::~GeneralPlugin()
{
  // Empty
}

std::string GeneralPlugin::description() const
{
  return myInterface->description();
}

std::string GeneralPlugin::usage() const
{
  return myInterface->usage();
}

std::string GeneralPlugin::configFile() const
{
  return myInterface->configFile();
}

bool GeneralPlugin::isEnabled() const
{
  return myInterface->isEnabled();
}

void GeneralPlugin::enable()
{
  myInterface->enable();
}

void GeneralPlugin::disable()
{
  myInterface->disable();
}

bool GeneralPlugin::wantSignal(unsigned long signalType) const
{
  return myInterface->wantSignal(signalType);
}

void GeneralPlugin::pushSignal(Licq::PluginSignal* signal)
{
  myInterface->pushSignal(signal);
}

void GeneralPlugin::pushEvent(Licq::Event* event)
{
  myInterface->pushEvent(event);
}

void GeneralPlugin::createInterface()
{
  assert(!myInterface);
  myInterface.reset((*myFactory)(), &destroyGeneralPluginInterface);
}

boost::shared_ptr<Licq::PluginInterface> GeneralPlugin::interface()
{
  return myInterface;
}

boost::shared_ptr<const Licq::PluginInterface> GeneralPlugin::interface() const
{
  return myInterface;
}
