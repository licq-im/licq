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

#ifndef LICQ_PLUGINMANAGER_H
#define LICQ_PLUGINMANAGER_H

#include "plugin.h"

#include <boost/noncopyable.hpp>
#include <list>

namespace Licq
{

typedef std::list<GeneralPlugin::Ptr> GeneralPluginsList;
typedef std::list<ProtocolPlugin::Ptr> ProtocolPluginsList;

class PluginManager : private boost::noncopyable
{
public:
  virtual ~PluginManager() { /* Empty */ }

  virtual void getGeneralPluginsList(GeneralPluginsList& plugins) = 0;
  virtual void getProtocolPluginsList(ProtocolPluginsList& plugins) = 0;

  virtual bool
  startGeneralPlugin(const std::string& name, int argc, char** argv) = 0;
  virtual bool startProtocolPlugin(const std::string& name) = 0;

  /**
   * Registers current thread as new general plugin.
   *
   * @param signalMask A mask indicating which signals the plugin wish to
   *        receive. Use the constant SIGNAL_ALL to receive all signals.
   * @return The pipe to listen on for notifications.
   */
  virtual int registerGeneralPlugin(unsigned long signalMask) = 0;

  /**
   * Unregisters current thread as a general plugin.
   */
  virtual void unregisterGeneralPlugin() = 0;

  /**
   * Registers current thread as a new protocol plugin.
   *
   * @return The pipe to listen on for notifications.
   */
  virtual int registerProtocolPlugin() = 0;

  /**
   * Unregisters current thread as a protocol plugin.
   */
  virtual void unregisterProtocolPlugin() = 0;
};

} // namespace Licq

#endif
