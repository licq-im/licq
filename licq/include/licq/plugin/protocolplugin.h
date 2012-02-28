/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2011-2012 Licq developers <licq-dev@googlegroups.com>
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

#include "../macro.h"

namespace Licq
{
class Owner;
class ProtocolSignal;
class User;
class UserId;

/**
 * Base class for protocol plugins implementing support for an IM protocol
 *
 * All protocol plugins must have a subclass implementing this interface
 * See documentation for Licq::Plugin for additional information
 */
class ProtocolPlugin : public Plugin
{
public:
  class Params;

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
    CanVaryEncoding     = 1<<11,
    CanSingleGroup      = 1<<12,    // Contacts have only a single group in server list
  };

  /// A smart pointer to a ProtocolPlugin instance
  typedef boost::shared_ptr<ProtocolPlugin> Ptr;

  /// Get the protocol's unique identifier
  virtual unsigned long protocolId() const = 0;

  /// Get default server host to connect to
  virtual std::string defaultServerHost() const = 0;

  /// Get default server port to connect to
  virtual int defaultServerPort() const = 0;

  /**
   * Get protocol plugin supported features
   *
   * @return A mask of bits from Capabilities enum
   */
  virtual unsigned long capabilities() const = 0;

  /**
   * Push a signal to this protocol plugin
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
  void pushSignal(ProtocolSignal* signal);

protected:
  /**
   * Constructor
   *
   * @param p Paramaters from PluginManager
   */
  ProtocolPlugin(Params& p);

  /// Destructor
  virtual ~ProtocolPlugin();

  /**
   * Create a user object
   * Called by UserManager when users are loaded or added
   * Override this if protocol has subclassed Licq::User
   *
   * @param id User id
   * @param temporary True if user isn't permanently added to contact list
   * @return A newly created user object
   */
  virtual User* createUser(const UserId& id, bool temporary = false);

  /**
   * Create an owner object
   * Called by UserManager when owner is loaded or added
   * Override this if protocol has subclassed Licq::Owner (or Licq::User)
   *
   * @param id User id of owner
   * @return A newly created owner object
   */
  virtual Owner* createOwner(const UserId& id);

  /**
   * Get a signal from the signal queue
   * Called from protocol plugin
   *
   * The plugin must call this function to fetch a signal after getting
   * notified via its pipe. The signal must be deleted by the plugin after
   * processing.
   *
   * @return The oldest signal on the queue, or NULL if queue is empty
   */
  ProtocolSignal* popSignal();

private:
  LICQ_DECLARE_PRIVATE();

  /// Allow the plugin manager to access private members
  friend class LicqDaemon::PluginManager;
};

} // namespace Licq

#endif
