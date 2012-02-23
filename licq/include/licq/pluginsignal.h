/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_PLUGINSIGNAL_H
#define LICQ_PLUGINSIGNAL_H

#include "userid.h"

namespace Licq
{

/**
 * Asynchronous plugin notifications
 *
 * When a plugin registers with the Licq daemon it informs the daemon of what
 * signals it is interested in. From then on, at any time it may receive a
 * signal from the following list. Each signal contains the signal type, an
 * optional sub-type, uin and signal specific argument.
 */
class PluginSignal
{
public:
  enum SignalType
  {
    SignalList          = 1<<0,         // Contact list has changed
    SignalUser          = 1<<1,         // A user has changed
    SignalLogon         = 1<<2,         // Owner logged on, userId is owner
    SignalLogoff        = 1<<3,         // Owner logged off, userId is owner
    SignalOnEvent       = 1<<4,         // ?
    SignalAddedToServer = 1<<7,         // userId has been successfully added to server list
    SignalNewProtocol   = 1<<8,         // A new protocol plugin has been loaded, subtype is protocolId
    SignalConversation  = 1<<9,         // Conversation signal
    SignalVerifyImage   = 1<<17,        // A registration verification image is available as basedir/verify.jpg
    SignalNewOwner      = 1<<18,        // Registration complete, userId is new owner
    SignalPluginEvent   = 1<<19,        // Event to perform in a plugin, subtype specifies event
    SignalRemoveProtocol= 1<<20,        // A protocol plugin has been unloaded, subtype is protocolId

    SignalAll           = 0x001e039f,   // Mask for all known signals
  };

  enum ListSubSignal
  {
    ListUserAdded       = 1,            // userId was added to the list
    ListUserRemoved     = 2,            // userId was removed from the list
    ListInvalidate      = 3,            // Contact list must reloaded
    ListGroupAdded      = 4,            // A group was added to the list, argument is groupId
    ListGroupRemoved    = 5,            // A group was removed from the list, argument is groupId
    ListGroupChanged    = 6,            // Data for a group has changed, argument is groupId
    ListGroupsReordered = 7,            // Group sorting has changed
    ListOwnerAdded      = 8,            // An owner was added, userId is owner
    ListOwnerRemoved    = 9,            // An owner was removed, userId is owner
  };

  enum UserSubSignal
  {
    UserStatus          = 1,            // User status has changed, e.g. online, offline, away, etc.
    UserEvents          = 2,            // Number of unread events for user has changed
    UserBasic           = 3,            // Basic user info has changed, e.g. name, alias
    UserInfo            = 4,            // Additional user info has changed, e.g. email, birthday, etc.
    UserGroups          = 5,            // Group membership(s) for user has changed
    UserSettings        = 6,            // Settings for user has changed, e.g. auto accept, invisible, etc.
    UserSecurity        = 9,            // Encryption has been enabled/disabled or key has been added/removed
    UserPicture         = 13,           // User picture has been updated/removed
    UserTyping          = 14,           // Typing status for user has changed, argument is new status
    UserPluginStatus    = 15,           // Status for user plugins has changed
  };

  enum LogoffSubSignal
  {
    LogoffRequested     = 0,            // Logoff was requested by user
    LogoffRate          = 1,            // Logon failed, the rate limit of the server has been exceeded
    LogoffPassword      = 2,            // Logon failed, username and/or password are invalid
  };

  enum ConvoSubSignal
  {
    ConvoCreate         = 1,            // Conversation created for userId
    ConvoJoin           = 2,            // userId joinded conversation
    ConvoLeave          = 3,            // userId left conversation
  };

  enum PluginSubSignal
  {
    PluginViewEvent     = 1,            // UI should popup oldest unread event for userId (if set)
    PluginStartMessage  = 2,            // UI should open message dialog for userId
    PluginShowUserList  = 3,            // UI should show (if hidden) and raise contact list
    PluginHideUserList  = 4,            // UI should hide contact list
  };

  /**
   * Constructor
   *
   * @param signal Signal type
   * @param subSignal Signal sub type if applicable
   * @param userId Id of user affected by signal if applicable
   * @param argument Additional data for signal, usage is signal dependant
   * @param cid Conversation id, if applicable
   */
  PluginSignal(unsigned signal, unsigned subSignal,
      const UserId& userId = UserId(), int argument = 0, unsigned long cid = 0)
    : mySignal(signal),
      mySubSignal(subSignal),
      myUserId(userId),
      myArgument(argument),
      myCid(cid)
  { /* Empty */ }

  /**
   * Copy constructor
   *
   * @param s PluginSignal object to copy fields from
   */
  PluginSignal(const PluginSignal* s)
    : mySignal(s->mySignal),
      mySubSignal(s->mySubSignal),
      myUserId(s->myUserId),
      myArgument(s->myArgument),
      myCid(s->myCid)
  { /* Empty */ }

  /**
   * Get signal type
   */
  unsigned signal() const
  { return mySignal; }

  /**
   * Get signal subtype
   */
  unsigned subSignal() const
  { return mySubSignal; }

  /**
   * Get id for user related to this signal
   */
  const UserId& userId() const
  { return myUserId; }

  /**
   * Get signal specific argument
   */
  int argument() const
  { return myArgument; }

  /**
   * Get conversation id
   */
  unsigned long cid() const
  { return myCid; }

protected:
  const unsigned long mySignal;
  const unsigned long mySubSignal;
  const UserId myUserId;
  const int myArgument;
  const unsigned long myCid;
};

} // namespace Licq

#endif
