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

#ifndef LICQ_PLUGIN_H
#define LICQ_PLUGIN_H

#include "plugininterface.h"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

namespace Licq
{

/**
 * Represents a loaded plugin.
 */
class Plugin : private boost::noncopyable
{
public:
  /// A smart pointer to a Plugin instance
  typedef boost::shared_ptr<Plugin> Ptr;

  /// Get the plugin's unique id.
  virtual int id() const = 0;

  /// Get the plugin's name.
  virtual std::string name() const = 0;

  /// Get the plugin's version.
  virtual std::string version() const = 0;

  /// Get the name of the library from where the plugin was loaded.
  virtual std::string libraryName() const = 0;

protected:
  /// Destructor
  virtual ~Plugin() { }

  virtual boost::shared_ptr<PluginInterface> internalInterface() = 0;
  template <typename T> friend boost::shared_ptr<T> plugin_internal_cast(Ptr);
};

/**
 * Function to cast a plugin to a plugin specific interface to get access to
 * methods that only apply for a specific plugin. To e.g. get access to ICQ
 * specific methods, do:
 * @code
 * Licq::IcqProtocol::Ptr icq = plugin_internal_cast<Licq::IcqProtocol>(
 *     Licq::gPluginManager.getProtocolPlugin(LICQ_PPID));
 * if (icq)
 *   icq->icqSendSms(...);
 * @endcode
 */
template <typename T>
inline boost::shared_ptr<T> plugin_internal_cast(Plugin::Ptr plugin)
{
  return plugin
      ? boost::dynamic_pointer_cast<T>(plugin->internalInterface())
      : boost::shared_ptr<T>();
}

// plugin_internal_cast<>() is not supposed to be used to cast to
// PluginInterface, GeneralPluginInterface or ProtocolPluginInterface; only to
// plugin specific interfaces.

template <> inline boost::shared_ptr<PluginInterface>
plugin_internal_cast<PluginInterface>(Plugin::Ptr);

class GeneralPluginInterface;
template <> boost::shared_ptr<GeneralPluginInterface>
plugin_internal_cast<GeneralPluginInterface>(Plugin::Ptr);

class ProtocolPluginInterface;
template <> boost::shared_ptr<ProtocolPluginInterface>
plugin_internal_cast<ProtocolPluginInterface>(Plugin::Ptr);

} // namespace Licq

// Make available in global namespace
using Licq::plugin_internal_cast;

#endif
