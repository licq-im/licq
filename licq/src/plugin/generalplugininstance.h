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

#ifndef LICQDAEMON_GENERALPLUGININSTANCE_H
#define LICQDAEMON_GENERALPLUGININSTANCE_H

#include "generalplugin.h"
#include "plugininstance.h"

#include <licq/plugin/generalplugininstance.h>

namespace Licq
{
class Event;
class GeneralPluginFactory;
class PluginSignal;
}

namespace LicqDaemon
{

class GeneralPluginInstance : public PluginInstance,
                              public Licq::GeneralPluginInstance
{
public:
  typedef boost::shared_ptr<GeneralPluginInstance> Ptr;

  GeneralPluginInstance(
      int id, GeneralPlugin::Ptr plugin, PluginThread::Ptr thread);
  ~GeneralPluginInstance();

  GeneralPlugin::Ptr plugin() { return myPlugin; }

  // From Licq::GeneralPluginInstance
  boost::shared_ptr<Licq::GeneralPlugin> plugin() const;
  bool isEnabled() const;
  void enable();
  void disable();

  bool wantSignal(unsigned long signalType) const;
  void pushSignal(boost::shared_ptr<const Licq::PluginSignal> signal);
  void pushEvent(boost::shared_ptr<const Licq::Event> event);

protected:
  // From PluginInstance
  void createInterface();
  boost::shared_ptr<Licq::PluginInterface> interface();
  boost::shared_ptr<const Licq::PluginInterface> interface() const;

private:
  GeneralPlugin::Ptr myPlugin;
  boost::shared_ptr<Licq::GeneralPluginInterface> myInterface;
};

} // namespace LicqDaemon

#endif
