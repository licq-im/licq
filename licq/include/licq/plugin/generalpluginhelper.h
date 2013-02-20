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

#ifndef LICQ_GENERALPLUGINHELPER_H
#define LICQ_GENERALPLUGINHELPER_H

#include "generalplugininterface.h"
#include "../macro.h"

namespace Licq
{

/**
 * Implements part of the GeneralPluginInterface to help make the
 * implementation of a general plugin easier.
 */
class GeneralPluginHelper : public GeneralPluginInterface
{
public:
  // Notification that general plugins can get via its pipe
  static const char PipeDisable = '0';
  static const char PipeEnable = '1';
  static const char PipeEvent = 'E';
  static const char PipeShutdown = 'X';
  static const char PipeSignal = 'S';

  /// Returns true without doing anything
  bool init(int argc, char** argv);

  /// Writes PipeShutdown to the pipe
  void shutdown();

  /// Writes PipeEnable to the pipe
  void enable();

  /// Writes PipeDisable to the pipe
  void disable();

  /// Returns true if signalType is in the signal mask set by setSignalMask
  bool wantSignal(unsigned long signalType) const;

  /// Queues the signal and writes PipeSignal to the pipe
  void pushSignal(boost::shared_ptr<const PluginSignal> signal);

  // Queues the event and writes PipeEvent to the pipe
  void pushEvent(boost::shared_ptr<const Event> event);

protected:
  GeneralPluginHelper();

  virtual ~GeneralPluginHelper();

  /**
   * Get read end of pipe used to communicate with the plugin.
   *
   * @return A file descriptor that can be polled for new signals and events.
   */
  int getReadPipe() const;

  /**
   * Specify which signals to forward to plugin
   *
   * @param signalMask Mask of signals from PluginSignal::SignalType to accept
   */
  void setSignalMask(unsigned long signalMask);

  /**
   * Get a signal from the signal queue
   *
   * The plugin must call this function to fetch a signal after getting
   * notified via its pipe.
   *
   * @return The oldest signal on the queue, or NULL if queue is empty
   */
  boost::shared_ptr<const PluginSignal> popSignal();

  /**
   * Get an event from the event queue
   *
   * The plugin must call this function to fetch an event after getting
   * notified via its pipe.
   *
   * @return The oldest event on the queue, or NULL if queue is empty
   */
  boost::shared_ptr<const Event> popEvent();

private:
  LICQ_DECLARE_PRIVATE();
};

} // namespace Licq

#endif
