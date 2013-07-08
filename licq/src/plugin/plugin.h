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

#ifndef LICQDAEMON_PLUGIN_H
#define LICQDAEMON_PLUGIN_H

#include "utils/dynamiclibrary.h"

#include <licq/plugin/plugin.h>
#include <licq/thread/mutex.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/weak_ptr.hpp>
#include <vector>

namespace LicqDaemon
{

class PluginInstance;

class Plugin : public virtual Licq::Plugin,
               public boost::enable_shared_from_this<Plugin>
{
public:
  typedef boost::shared_ptr<Plugin> Ptr;

  Plugin(DynamicLibrary::Ptr lib);
  ~Plugin();

  // From Licq::Plugin
  std::string name() const;
  std::string version() const;
  std::string libraryName() const;
  boost::shared_ptr<Licq::PluginFactory> internalFactory();

protected:
  virtual boost::shared_ptr<Licq::PluginFactory> factory() = 0;
  virtual boost::shared_ptr<const Licq::PluginFactory> factory() const = 0;

  void registerInstance(boost::weak_ptr<PluginInstance> instance);

  mutable Licq::Mutex myMutex;
  std::vector< boost::weak_ptr<PluginInstance> > myInstances;

private:
  DynamicLibrary::Ptr myLibrary;
};

} // namespace LicqDaemon

#endif
