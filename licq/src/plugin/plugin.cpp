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

#include "plugin.h"

#include <licq/logging/log.h>
#include <licq/plugin/pluginfactory.h>
#include <licq/thread/mutexlocker.h>

using namespace LicqDaemon;

Plugin::Plugin(DynamicLibrary::Ptr lib)
  : myLibrary(lib)
{
  Licq::gLog.debug("Loading plugin %s", libraryName().c_str());
}

Plugin::~Plugin()
{
  Licq::gLog.debug("Unloading plugin %s", libraryName().c_str());
}

std::string Plugin::name() const
{
  return factory()->name();
}

std::string Plugin::version() const
{
  return factory()->version();
}

std::string Plugin::libraryName() const
{
  return myLibrary->getName();
}

boost::shared_ptr<Licq::PluginFactory> Plugin::internalFactory()
{
  // Create a shared_ptr that keeps this object alive at least until the
  // returned pointer goes out of scope.
  return boost::shared_ptr<Licq::PluginFactory>(
      shared_from_this(), factory().get());
}

void Plugin::registerInstance(boost::weak_ptr<PluginInstance> instance)
{
  Licq::MutexLocker locker(myMutex);

  // Clean up stale entries
  for (std::vector< boost::weak_ptr<PluginInstance> >::iterator it =
           myInstances.begin(); it != myInstances.end();)
  {
    if (!it->lock())
      it = myInstances.erase(it);
    else
      ++it;
  }

  myInstances.push_back(instance);
}
