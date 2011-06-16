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

namespace Licq
{

/**
 * A ProtocolPlugin implements support for a specific IM protocol.
 */
class ProtocolPlugin : public virtual Plugin
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

protected:
  /// Destructor
  virtual ~ProtocolPlugin() { /* Empty */ }
};

} // namespace Licq

#endif
