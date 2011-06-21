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

#ifndef LICQ_PROTOCOLPLUGIN_H
#define LICQ_PROTOCOLPLUGIN_H

#include "plugin.h"

#include <boost/shared_ptr.hpp>
#include <list>

namespace LicqDaemon
{
class PluginEventHandler;
class PluginManager;
}

namespace Licq
{
class ProtocolSignal;

/**
 * A ProtocolPlugin implements support for a specific IM protocol.
 */
class ProtocolPlugin : public Plugin
{
public:
  enum Capabilities
  {
    CanSendMsg          = 1<<0,
    CanSendUrl          = 1<<1,
    CanSendFile         = 1<<2,
    CanSendChat         = 1<<3,
    CanSendContact      = 1<<4,
    CanSendAuth         = 1<<5,
    CanSendAuthReq      = 1<<6,
    CanSendSms          = 1<<7,
    CanSendSecure       = 1<<8,
    CanSendDirect       = 1<<9,
    CanHoldStatusMsg     = 1<<10,
  };

  /// A smart pointer to a ProtocolPlugin instance
  typedef boost::shared_ptr<ProtocolPlugin> Ptr;

  /// Get the protocol's unique identifier
  unsigned long protocolId() const;

  /// Get default server host to connect to
  std::string defaultServerHost() const;

  /// Get default server port to connect to
  int defaultServerPort() const;

  /**
   * Get protocol plugin supported features
   *
   * @return A mask of bits from Capabilities enum
   */
  unsigned long capabilities() const;

  /**
   * Push a signal to this protocol plugin
   *
   * The signal will be added to the signal queue and the plugin will be
   * notified via its pipe.
   *
   * Note: This function will return immediately. The signal will be processed
   * by the plugin asynchrony
   *
   * @param signal Signal to forward to the plugin
   */
  void pushSignal(ProtocolSignal* signal);

protected:
  /**
   * Constructor
   *
   * @param id Unique id for this plugin
   * @param lib Library plugin was loaded from
   * @param thread Thread for plugin to run in
   * @param icq True if this is the internal ICQ protocol plugin
   */
  ProtocolPlugin(int id, LibraryPtr lib, ThreadPtr thread, bool icq = false);

  /// Destructor
  virtual ~ProtocolPlugin();

  /**
   * Get a signal from the signal queue
   *
   * @return The oldest signal on the queue, or NULL if queue is empty
   */
  ProtocolSignal* popSignal();

private:
  LICQ_DECLARE_PRIVATE();

  /// Allow the plugin manager to access protected members
  friend class LicqDaemon::PluginManager;

  /// Allow PluginEventHandler to call popSignal()
  friend class LicqDaemon::PluginEventHandler;
};

typedef std::list<ProtocolPlugin::Ptr> ProtocolPluginsList;

} // namespace Licq

#endif
