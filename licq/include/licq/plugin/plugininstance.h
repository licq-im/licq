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

#ifndef LICQ_PLUGININSTANCE_H
#define LICQ_PLUGININSTANCE_H

#include "plugininterface.h"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace Licq
{

/**
 * Represents an instance of a loaded plugin.
 */
class PluginInstance : private boost::noncopyable
{
public:
  /// A smart pointer to a Plugin instance
  typedef boost::shared_ptr<PluginInstance> Ptr;

  /// Get the instance's unique id.
  virtual int id() const = 0;

protected:
  virtual ~PluginInstance() { }

  virtual boost::shared_ptr<PluginInterface> internalInterface() = 0;
  template <typename T> friend boost::shared_ptr<T> plugin_internal_cast(Ptr);
};

/**
 * Function to cast a plugin instance to a plugin specific interface to get
 * access to methods that only apply for a specific plugin instance. To
 * e.g. get access to ICQ specific methods, do:
 * @code
 * Licq::IcqProtocol::Ptr icq = plugin_internal_cast<Licq::IcqProtocol>(
 *     Licq::gPluginManager.getProtocolInstance(myIcqOwner));
 * if (icq)
 *   icq->icqSendSms(...);
 * @endcode
 */
template <typename T>
inline boost::shared_ptr<T> plugin_internal_cast(PluginInstance::Ptr instance)
{
  return instance
      ? boost::dynamic_pointer_cast<T>(instance->internalInterface())
      : boost::shared_ptr<T>();
}

// plugin_internal_cast<>() is not supposed to be used to cast to
// PluginInterface, GeneralPluginInterface or ProtocolPluginInterface; only to
// plugin specific interfaces.

template <> inline boost::shared_ptr<PluginInterface>
plugin_internal_cast<PluginInterface>(PluginInstance::Ptr);

class GeneralPluginInterface;
template <> boost::shared_ptr<GeneralPluginInterface>
plugin_internal_cast<GeneralPluginInterface>(PluginInstance::Ptr);

class ProtocolPluginInterface;
template <> boost::shared_ptr<ProtocolPluginInterface>
plugin_internal_cast<ProtocolPluginInterface>(PluginInstance::Ptr);

} // namespace Licq

// Make available in global namespace
using Licq::plugin_internal_cast;

#endif
