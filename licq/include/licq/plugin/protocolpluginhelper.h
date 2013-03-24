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

#ifndef LICQ_PROTOCOLPLUGINHELPER_H
#define LICQ_PROTOCOLPLUGINHELPER_H

#include "protocolplugininterface.h"
#include "../macro.h"

namespace Licq
{

/**
 * Implements part of the ProtocolPluginInterface to help make the
 * implementation of a protocol plugin easier.
 */
class ProtocolPluginHelper : public ProtocolPluginInterface
{
public:
  // Notification that protocols can get via its pipe
  static const char PipeShutdown = 'X';
  static const char PipeSignal = 'S';

  /// Returns true without doing anything
  bool init(int argc, char** argv);

  /// Writes PipeShutdown to the pipe
  void shutdown();

  /// Queues the signal and writes PipeSignal to the pipe
  void pushSignal(boost::shared_ptr<const ProtocolSignal> signal);

protected:
  ProtocolPluginHelper();

  virtual ~ProtocolPluginHelper();

  /**
   * Get read end of pipe used to communicate with the plugin.
   *
   * @return A file descriptor that can be polled for new signals.
   */
  int getReadPipe() const;

  /**
   * Get a signal from the signal queue
   * Called from protocol plugin
   *
   * The plugin must call this function to fetch a signal after getting
   * notified via its pipe.
   *
   * @return The oldest signal on the queue, or NULL if queue is empty
   */
  boost::shared_ptr<const ProtocolSignal> popSignal();

private:
  LICQ_DECLARE_PRIVATE();
};

} // namespace Licq

#endif
