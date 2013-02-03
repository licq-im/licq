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

class Owner;
class ProtocolSignal;
class User;
class UserId;

/**
 * Interface for protocol plugins implementing support for an IM protocol
 *
 * All protocol plugins must implement this interface. See documentation for
 * Licq::PluginInterface for additional information
 */
class ProtocolPluginInterface : public PluginInterface
{
public:
  /// Return the protocol's unique identifier
  virtual unsigned long protocolId() const = 0;

  /**
   * Return the protocol plugin's supported features
   *
   * @return A mask of bits from Capabilities enum
   */
  virtual unsigned long capabilities() const = 0;

  /**
   * Pushes a signal to the plugin.
   *
   * The plugin should take care not to block the caller (i.e. only queue the
   * signal for later processing).
   */
  virtual void pushSignal(boost::shared_ptr<ProtocolSignal> signal) = 0;

  /**
   * Create a user object
   *
   * Called by UserManager when users are loaded or added.
   *
   * @param id User id
   * @param temporary True if user isn't permanently added to contact list
   * @return A newly created Licq::User object or a subclass of the same.
   */
  virtual User* createUser(const UserId& id, bool temporary) = 0;

  /**
   * Create an owner object
   *
   * Called by UserManager when owner is loaded or added.
   *
   * @param id User id of owner
   * @return A newly created Licq::Owner object or a subclass of the same.
   */
  virtual Owner* createOwner(const UserId& id) = 0;

protected:
  virtual ~ProtocolPluginInterface() { /* Empty */ }
};

/**
 * This struct contains the initial data and functions needed by Licq
 * to load a protocol plugin.
 */
struct ProtocolPluginData
{
  /// Magic value to identify an Licq protocol
  char magic[4];

  /// Version of Licq this plugin is built for
  int licqVersion;

  /**
   * Pointer to a factory function that creates and returns an object that
   * implements the ProtocolPluginInterface interface.
   *
   * Note that the protocol should not be (fully) initialized by this. The init
   * and run functions in Licq::Plugin will be called for this afterwards
   *
   */
  ProtocolPluginInterface* (*pluginFactory)();
};

} // namespace Licq

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Each protocol must contain the following symbol
 *
 * When a protocol is first loaded, this pointer is fetched and used to get
 * the ProtocolPluginData struct for the plugin.
 */
extern Licq::ProtocolPluginData LicqProtocolPluginData;

#ifdef __cplusplus
}
#endif

/*
 * Convenience macro to define protocol data in a plugin
 *
 * Note: <licq/version.h> must be included
 *
 * @param factory Pointer to the protocol factory function
 */
#define LICQ_PROTOCOL_PLUGIN_DATA(factory) \
  Licq::ProtocolPluginData LicqProtocolPluginData = \
  { {'L', 'i', 'c', 'q' }, LICQ_VERSION, factory }

#endif
