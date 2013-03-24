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

#ifndef LICQ_PROTOCOLPLUGININTERFACE_H
#define LICQ_PROTOCOLPLUGININTERFACE_H

#include "plugininterface.h"

#include <boost/shared_ptr.hpp>

namespace Licq
{

class ProtocolSignal;

/**
 * Interface for protocol plugins implementing support for an IM protocol
 *
 * All protocol plugins must implement this interface. See documentation for
 * Licq::PluginInterface for additional information
 */
class ProtocolPluginInterface : public PluginInterface
{
public:
  virtual ~ProtocolPluginInterface() { /* Empty */ }

  /**
   * Pushes a signal to the plugin.
   *
   * The plugin should take care not to block the caller (i.e. only queue the
   * signal for later processing).
   */
  virtual void pushSignal(boost::shared_ptr<const ProtocolSignal> signal) = 0;
};

} // namespace Licq

#endif
