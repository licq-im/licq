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

#include "generalplugininstance.h"

#include <licq/plugin/generalpluginfactory.h>
#include <licq/plugin/generalplugininterface.h>

using namespace LicqDaemon;

static void nullDeleter(void*) { /* Empty */ }

GeneralPluginInstance::GeneralPluginInstance(
    int id, GeneralPlugin::Ptr plugin, PluginThread::Ptr thread)
  : PluginInstance(id, thread),
    myPlugin(plugin)
{
  // Empty
}

GeneralPluginInstance::~GeneralPluginInstance()
{
  if (myInterface)
    myPlugin->generalFactory()->destroyPlugin(myInterface.get());
}

boost::shared_ptr<Licq::GeneralPlugin> GeneralPluginInstance::plugin() const
{
  return myPlugin;
}

bool GeneralPluginInstance::isEnabled() const
{
  return myInterface->isEnabled();
}

void GeneralPluginInstance::enable()
{
  if (isRunning())
    myInterface->enable();
}

void GeneralPluginInstance::disable()
{
  if (isRunning())
    myInterface->disable();
}

bool GeneralPluginInstance::wantSignal(unsigned long signalType) const
{
  return myInterface->wantSignal(signalType);
}

void GeneralPluginInstance::pushSignal(
    boost::shared_ptr<const Licq::PluginSignal> signal)
{
  if (isRunning())
    myInterface->pushSignal(signal);
}

void GeneralPluginInstance::pushEvent(
    boost::shared_ptr<const Licq::Event> event)
{
  if (isRunning())
    myInterface->pushEvent(event);
}

void GeneralPluginInstance::createInterface()
{
  assert(!myInterface);
  myInterface.reset(myPlugin->generalFactory()->createPlugin(), &nullDeleter);
}

boost::shared_ptr<Licq::PluginInterface> GeneralPluginInstance::interface()
{
  return myInterface;
}

boost::shared_ptr<const Licq::PluginInterface>
GeneralPluginInstance::interface() const
{
  return myInterface;
}
