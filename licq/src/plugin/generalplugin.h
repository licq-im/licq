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

#ifndef LICQDAEMON_GENERALPLUGIN_H
#define LICQDAEMON_GENERALPLUGIN_H

#include "plugin.h"
#include "pluginthread.h"

#include <licq/plugin/generalplugin.h>

namespace Licq
{
class Event;
class GeneralPluginFactory;
class PluginSignal;
}

namespace LicqDaemon
{

class GeneralPluginInstance;

class GeneralPlugin : public Plugin, public Licq::GeneralPlugin
{
public:
  typedef boost::shared_ptr<GeneralPlugin> Ptr;

  GeneralPlugin(DynamicLibrary::Ptr lib,
                boost::shared_ptr<Licq::GeneralPluginFactory> factory,
                PluginThread::Ptr thread);
  ~GeneralPlugin();

  boost::shared_ptr<GeneralPluginInstance> createInstance(
      int id, void (*callback)(const PluginInstance&));

  boost::shared_ptr<Licq::GeneralPluginFactory> generalFactory();

  // From Licq::GeneralPlugin
  std::string description() const;
  std::string usage() const;
  std::string configFile() const;
  Licq::GeneralPluginInstance::Ptr instance() const;

protected:
  // From Plugin
  boost::shared_ptr<Licq::PluginFactory> factory();
  boost::shared_ptr<const Licq::PluginFactory> factory() const;

private:
  boost::shared_ptr<Licq::GeneralPluginFactory> myFactory;
  PluginThread::Ptr myThread;
};

} // namespace LicqDaemon

#endif
