/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq developers <licq-dev@googlegroups.com>
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
class UserId;

/**
 * Class for holding event configuration for a user/a group/global
 */
class OnEventData : private boost::noncopyable
{
public:
  enum OnEventType
  {
    OnEventMessage = 0,
    OnEventUrl = 1,
    OnEventChat = 2,
    OnEventFile = 3,
    OnEventSms = 4,
    OnEventOnline = 5,
    OnEventSysMsg = 6,
    OnEventMsgSent = 7,
  };
  static const int NumOnEventTypes = 8;

  // Constant for string parameters to mean value hasn't been overridden
  static const char* const Default;

  // Values for enable property
  enum EnabledLevel
  {
    EnabledDefault = -1,
    EnabledNever = 0,           // Never perform on events
    EnabledOnline = 1,          // Only perform on events if online
    EnabledAway = 2,            // Only perform on events if online or away
    EnabledNotAvailable = 3,    // Don't perform on events if occupied or dnd
    EnabledOccupied = 4,        // Don't perform on events if dnd
    EnabledAlways = 5,          // Always perform on events
  };

  /**
   * Check if on events actions are enabled
   *
   * @return value from EnableLevel
   */
  int enabled() const
  { return myEnabled; }

  /**
   * Enabled or disable on events actions
   *
   * @param enabled value from EnableLevel
   */
  virtual void setEnabled(int enabled) = 0;

  /**
   * Should we notify for online users during sign on
   *
   * @return zero/one if set disabled/enabled, -1 to not override
   */
  int alwaysOnlineNotify() const
  { return myAlwaysOnlineNotify; }

  /**
   * Enabled or disable online notify during sign on
   *
   * @param enabled zero/one to set disabled/enabled, -1 to not override
   */
  virtual void setAlwaysOnlineNotify(int alwaysOnlineNotify) = 0;

  /**
   * Get command to run for invoking on events
   *
   * @return Command including any static parameters
   */
  const std::string& command() const
  { return myCommand; }

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
  const std::string& parameter(int event) const
  { return myParameters[event]; }

  /**
   * Set parameters for a specific event
   *
   * @param event Event to set parameters for
   * @param parameter Parameter(s) to set
   */
  virtual void setParameter(int event, const std::string& parameter) = 0;

protected:
  virtual ~OnEventData() { /* Empty */ }

  int myEnabled;
  int myAlwaysOnlineNotify;
  std::string myCommand;
  std::string myParameters[NumOnEventTypes];
};


/**
 * Manager class for commands to run on events
 * Normally this is used for playing sounds for various events but can be used
 *   to trigger any external command.
 */
class OnEventManager : private boost::noncopyable
{
public:
  /**
   * Get global configuration
   *
   * @return The global configuration locked for access
   */
  virtual OnEventData* lockGlobal() = 0;

  /**
   * Get group configuration
   *
   * @param groupId Id of group to get
   * @param create True to create group data if not existing
   * @return The group configuration locked for access or NULL
   */
  virtual OnEventData* lockGroup(int groupId, bool create = false) = 0;

  /**
   * Get user configuration
   *
   * @param userId Id of user to get
   * @param create True to create user data if not existing
   * @return The user configuration locked for access or NULL
   */
  virtual OnEventData* lockUser(const UserId& userId, bool create = false) = 0;

  /**
   * Unlock event data object
   * This function must be called after lock function that didn't return NULL
   *
   * @param save True if configuration has changed and needs to be saved
   */
  virtual void unlock(const OnEventData* data, bool save = false) = 0;

  /**
   * Get effective configuration for a user
   * Use the returned data to see what parameters would actually be used
   *
   * @param user User to get configuration for
   * @return User configuration with defaults filled in
   */
  virtual OnEventData* getEffectiveUser(const User* user) = 0;

  /**
   * Get effective configuration for a group
   * Use the returned data to see what parameters would actually be used
   *
   * @param groupId Id of group to get configuration for
   * @return Group configuration with defaults filled in
   */
  virtual OnEventData* getEffectiveGroup(int groupId) = 0;

  /**
   * Delete a data object returned by getEffectiveUser
   *
   * @param data Data object to delete
   */
  virtual void dropEffective(OnEventData* data) = 0;

  /**
   * Execute on event command
   * Note: Do not lock before calling this function
   *
   * @param event Type of event
   * @param user User associated with event
   */
  virtual void performOnEvent(OnEventData::OnEventType event, const User* user) = 0;

protected:
  virtual ~OnEventManager() { /* Empty */ }
};

extern OnEventManager& gOnEventManager;

} // namespace Licq

#endif
