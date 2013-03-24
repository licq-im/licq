/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011, 2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQDAEMON_PROTOCOLPLUGIN_H
#define LICQDAEMON_PROTOCOLPLUGIN_H

#include "plugin.h"
#include "pluginthread.h"

#include <licq/plugin/protocolplugin.h>

namespace Licq
{
class Owner;
class ProtocolPluginFactory;
class ProtocolSignal;
class User;
class UserId;
}

namespace LicqDaemon
{

class ProtocolPluginInstance;

class ProtocolPlugin : public Plugin, public Licq::ProtocolPlugin
{
public:
  typedef boost::shared_ptr<ProtocolPlugin> Ptr;

  ProtocolPlugin(DynamicLibrary::Ptr lib,
                 boost::shared_ptr<Licq::ProtocolPluginFactory> factory,
                 PluginThread::Ptr thread);
  ~ProtocolPlugin();

  boost::shared_ptr<ProtocolPluginInstance> createInstance(
      int id, const Licq::UserId& ownerId,
      void (*callback)(const PluginInstance&));

  boost::shared_ptr<Licq::ProtocolPluginFactory> protocolFactory();

  // From Licq::ProtocolPlugin
  unsigned long protocolId() const;
  unsigned long capabilities() const;
  Instances instances() const;

  Licq::User* createUser(const Licq::UserId& id, bool temporary);
  Licq::Owner* createOwner(const Licq::UserId& id);

protected:
  // From Plugin
  boost::shared_ptr<Licq::PluginFactory> factory();
  boost::shared_ptr<const Licq::PluginFactory> factory() const;

private:
  boost::shared_ptr<Licq::ProtocolPluginFactory> myFactory;
  PluginThread::Ptr myMainThread;
};

} // namespace LicqDaemon

#endif
