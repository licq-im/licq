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

#ifndef LICQ_GENERALPLUGININTERFACE_H
#define LICQ_GENERALPLUGININTERFACE_H

#include "plugininterface.h"

#include <boost/shared_ptr.hpp>

namespace Licq
{

class Event;
class PluginSignal;

/**
 * Interface for general plugins.
 *
 * All general plugins must implement this interface. See documentation for
 * Licq::PluginInterface for additional information
 */
class GeneralPluginInterface : public PluginInterface
{
public:
  virtual ~GeneralPluginInterface() { /* Empty */ }

  /// Return true if the plugin is enabled
  virtual bool isEnabled() const = 0;

  /// Enable the plugin
  virtual void enable() = 0;

  /// Disable the plugin
  virtual void disable() = 0;

  /// Return true if a signal should be forwarded to this plugin; otherwise
  /// false
  virtual bool wantSignal(unsigned long signalType) const = 0;

  /**
   * Pushes a signal to the plugin.
   *
   * The plugin should take care not to block the caller (i.e. only queue the
   * signal for later processing).
   */
  virtual void pushSignal(boost::shared_ptr<const PluginSignal> signal) = 0;

  /**
   * Pushes an event to the plugin.
   *
   * The plugin should take care not to block the caller (i.e. only queue the
   * event for later processing).
   */
  virtual void pushEvent(boost::shared_ptr<const Event> event) = 0;
};

} // namespace Licq

#endif
