/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2011 Licq developers
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

#ifndef LICQ_GENERALPLUGIN_H
#define LICQ_GENERALPLUGIN_H

#include "plugin.h"

#include <boost/shared_ptr.hpp>

#include "../macro.h"

namespace Licq
{
class Event;
class PluginSignal;

/**
 * Base class for general plugins
 *
 * All general plugins must have a subclass implementing this interface
 * See documentation for Licq::Plugin for additional information
 */
class GeneralPlugin : public Plugin
{
public:
  class Params;

  // Notification that general plugins can get via its pipe
  static const char PipeEvent = 'E';
  static const char PipeDisable = '0';
  static const char PipeEnable = '1';

  /// A smart pointer to a GeneralPlugin instance
  typedef boost::shared_ptr<GeneralPlugin> Ptr;

  /// Get the plugin's status.
  virtual bool isEnabled() const;

  /// Get the plugin's description
  virtual std::string description() const = 0;

  /// Get the plugin's usage instructions
  virtual std::string usage() const = 0;

  /// Ask the plugin to enable itself
  void enable();

  /// Ask the plugin to disable itself
  void disable();

  /// Check if a signal should be forwarded to this plugin
  bool wantSignal(unsigned long signalType) const;

  /**
   * Push a signal to this plugin
   * Called by anyone
   *
   * The signal will be added to the signal queue and the plugin will be
   * notified via its pipe.
   *
   * Note: This function will return immediately. The signal will be processed
   * by the plugin asynchrony
   *
   * @param signal Signal to forward to the plugin
   */
  void pushSignal(PluginSignal* signal);

  /**
   * Push an event to this plugin
   * Called by anyone
   *
   * The event will be added to the event queue and teh plugin will be
   * notified via its pipe.
   *
   * Note: This function will return immediately. The event will be processed
   * by the plugin asynchrony
   *
   * @param event Event to forward to the plugin
   */
  void pushEvent(Event* event);

protected:
  /**
   * Constructor
   *
   * @param p Paramaters from PluginManager
   */
  GeneralPlugin(Params& p);

  /// Destructor
  virtual ~GeneralPlugin();

  /**
   * Specify which signals to forward to plugin
   * Called from plugin
   *
   * @param signalMask Mask of signals from PluginSignal::SignalType to accept
   */
  void setSignalMask(unsigned long signalMask);

  /**
   * Get a signal from the signal queue
   * Called from plugin
   *
   * The plugin must call this function to fetch a signal after getting
   * notified via its pipe. The signal must be deleted by the plugin after
   * processing.
   *
   * @return The oldest signal on the queue, or NULL if queue is empty
   */
  PluginSignal* popSignal();

  /**
   * Get an event from the event queue
   * Called from plugin
   *
   * The plugin must call this function to fetch an event after getting
   * notified via its pipe. The event must be deleted by the plugin after
   * processing.
   *
   * @return The oldest event on the queue, or NULL if queue is empty
   */
  Event* popEvent();

private:
  LICQ_DECLARE_PRIVATE();

  /// Allow the plugin manager to access private members
  friend class LicqDaemon::PluginManager;
};

} // namespace Licq

#endif
