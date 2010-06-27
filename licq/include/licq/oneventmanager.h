/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#ifndef LICQ_ONEVENTMANAGER_H
#define LICQ_ONEVENTMANAGER_H

#include <boost/noncopyable.hpp>
#include <string>

namespace Licq
{
class User;

/**
 * Manager class for commands to run on events
 * Normally this is used for playing sounds for various events but can be used
 *   to trigger any external command.
 */
class OnEventManager : private boost::noncopyable
{
public:
  enum OnEventType
  {
    OnEventMessage = 0,
    OnEventUrl = 1,
    OnEventChat = 2,
    OnEventFile = 3,
    OnEventOnline = 4,
    OnEventSysMsg = 5,
    OnEventMsgSent = 6,
    OnEventSms = 7,
  };
  static const int NumOnEventTypes = 8;

  /**
   * Lock event manager configuration
   * Call this function before accessing the configuration
   */
  virtual void lock() = 0;

  /**
   * Unlock event manager configuration
   *
   * @param save True if configuration has changed and needs to be saved
   */
  virtual void unlock(bool save = false) = 0;

  /**
   * Check if on events actions are enabled
   *
   * @param True if on events will be performed
   */
  virtual bool enabled() const = 0;

  /**
   * Enabled or disable on events actions
   *
   * @param enabled True if on events should be performed
   */
  virtual void setEnabled(bool enabled) = 0;

  /**
   * Should we notify for online users during sign on
   *
   * @return True if online notify is enabled during sign on
   */
  virtual bool alwaysOnlineNotify() const = 0;

  /**
   * Enabled or disable online notify during sign on
   *
   * @param alwaysOnlineNotify True to enabled
   */
  virtual void setAlwaysOnlineNotify(bool alwaysOnlineNotify) = 0;

  /**
   * Get command to run for invoking on events
   *
   * @return Command including any static parameters
   */
  virtual std::string command() const = 0;

  /**
   * Set command to run for invoking events
   *
   * @param command Command including any static parameters
   */
  virtual void setCommand(const std::string& command) = 0;

  /**
   * Get parameters for a specific event
   * Normally this will be just a path for a sound file
   *
   * @param event Event to get parameters for
   * @return Parameters to add to command when event happens
   */
  virtual std::string parameter(OnEventType event) const = 0;

  /**
   * Set parameters for a specific event
   *
   * @param event Event to set parameters for
   * @param parameter Parameter(s) to set
   */
  virtual void setParameter(OnEventType event, const std::string& parameter) = 0;

  /**
   * Execute on event command
   * Note: Do not lock before calling this function
   *
   * @param event Type of event
   * @param user User associated with event
   */
  virtual void performOnEvent(OnEventType event, const User* user) = 0;

protected:
  virtual ~OnEventManager() { /* Empty */ }
};

extern OnEventManager& gOnEventManager;

} // namespace Licq

#endif
