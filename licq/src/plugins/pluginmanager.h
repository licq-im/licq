/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Erik Johansson <erijo@licq.org>
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

#ifndef LICQDAEMON_PLUGINMANAGER_H
#define LICQDAEMON_PLUGINMANAGER_H

#include "generalplugin.h"
#include "protocolplugin.h"

#include "licq/thread/mutex.h"
#include "utils/dynamiclibrary.h"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace LicqDaemon
{

class PluginManager : private boost::noncopyable
{
public:
  PluginManager();
  ~PluginManager();

  GeneralPlugin::Ptr loadGeneralPlugin(
      const std::string& name, int argc, char** argv);
  ProtocolPlugin::Ptr loadProtocolPlugin(const std::string& name);

  void startPlugin(Plugin::Ptr plugin, CICQDaemon* daemon);

private:
  DynamicLibrary::Ptr loadPlugin(
      const std::string& name, const std::string& prefix);

  unsigned short myNextPluginId;

  typedef std::list<GeneralPlugin::Ptr> GeneralPluginsList;
  GeneralPluginsList myGeneralPlugins;
  Licq::Mutex myGeneralPluginsMutex;

  typedef std::list<ProtocolPlugin::Ptr> ProtocolPluginsList;
  ProtocolPluginsList myProtocolPlugins;
  Licq::Mutex myProtocolPluginsMutex;
};

} // namespace LicqDaemon

#endif
