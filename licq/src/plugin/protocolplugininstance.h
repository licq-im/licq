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

#ifndef LICQDAEMON_PROTOCOLPLUGININSTANCE_H
#define LICQDAEMON_PROTOCOLPLUGININSTANCE_H

#include "plugininstance.h"
#include "protocolplugin.h"

#include <licq/plugin/protocolplugininstance.h>

#include <licq/userid.h>

namespace Licq
{
class ProtocolPluginFactory;
class ProtocolSignal;
}

namespace LicqDaemon
{

class ProtocolPluginInstance : public PluginInstance,
                               public Licq::ProtocolPluginInstance
{
public:
  typedef boost::shared_ptr<ProtocolPluginInstance> Ptr;

  ProtocolPluginInstance(
      int id, const Licq::UserId& ownerId, ProtocolPlugin::Ptr plugin,
      PluginThread::Ptr thread);
  ~ProtocolPluginInstance();

  ProtocolPlugin::Ptr plugin() { return myPlugin; }

  // From PluginInstance
  void run(void (*startCallback)(const PluginInstance&),
           void (*exitCallback)(const PluginInstance&));

  // From Licq::ProtocolPluginInstance
  boost::shared_ptr<Licq::ProtocolPlugin> plugin() const;
  const Licq::UserId& ownerId() const { return myOwnerId; }

  void pushSignal(boost::shared_ptr<const Licq::ProtocolSignal> signal);

protected:
  // From PluginInstance
  void createInterface();
  boost::shared_ptr<Licq::PluginInterface> interface();
  boost::shared_ptr<const Licq::PluginInterface> interface() const;

private:
  Licq::UserId myOwnerId;
  ProtocolPlugin::Ptr myPlugin;
  boost::shared_ptr<Licq::ProtocolPluginInterface> myInterface;
};

} // namespace LicqDaemon

#endif
