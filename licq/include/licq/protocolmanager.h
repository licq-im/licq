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

#ifndef LICQ_PROTOCOLMANAGER_H
#define LICQ_PROTOCOLMANAGER_H

#include <boost/noncopyable.hpp>
#include <list>
#include <string>


namespace Licq
{
class Color;
class UserId;

/**
 * Use this manager to communicate with protocol plugins
 *
 */
class ProtocolManager : private boost::noncopyable
{
public:
  static const char* const KeepAutoResponse;

  /**
   * Get next available id to use for an event
   * For use by protocols only
   */
  virtual unsigned long getNextEventId() = 0;

  /**
   * Update user alias on server contact list
   * Alias is taken from local contact list
   *
   * @param userId User to update
   */
  virtual void updateUserAlias(const UserId& userId) = 0;

  /**
   * Set status for a protocol
   *
   * @param ownerId Owner of protocol to change
   * @param newStatus The status to change to
   * @param message The status message to be set for the status
   * @return Event id
   */
  virtual unsigned long setStatus(const UserId& ownerId,
      unsigned newStatus, const std::string& message = KeepAutoResponse) = 0;

  /**
   * Notify a user that we've started/stopped typing
   *
   * @param userId User to notify
   * @param active True if we've started typing, false if we've stopped
   * @param nSocket ?
   */
  virtual void sendTypingNotification(const UserId& userId, bool active, int nSocket = -1) = 0;

  /**
   * Send a normal message to a user
   *
   * @param userId User to send message to
   * @param message The message to be sent
   * @param flags Flags from ProtocolSignal::SendFlags
   * @param color The color of the text and background (ICQ only)
   * @param convoId Conversation ID for group messages (Non-ICQ only)
   * @return Event id
   */
  virtual unsigned long sendMessage(const UserId& userId, const std::string& message,
      unsigned flags = 0, const Color* color = NULL, unsigned long convoId = 0) = 0;

  /**
   * Send URL message to a user
   *
   * @param userId User to sent URL to
   * @param url The URL to be sent
   * @param message Message or description of URL
   * @param flags Flags from ProtocolSignal::SendFlags
   * @param color The color of the text and background (ICQ only)
   * @return Event id
   */
  virtual unsigned long sendUrl(const UserId& userId, const std::string& url,
      const std::string& message, unsigned flags = 0, const Color* color = NULL) = 0;

  /**
   * Initiate a file transfer to a user
   *
   * @param userId User to send file to
   * @param filename Name of file to send
   * @param message Message or description of file(s)
   * @param files List of files to send
   * @param flags Flags from ProtocolSignal::SendFlags
   * @return Event id
   */
  virtual unsigned long fileTransferPropose(const UserId& userId,
      const std::string& filename, const std::string& message,
      const std::list<std::string>& files, unsigned flags = 0) = 0;

  /**
   * Refuse a proposed file transfer
   *
   * @param userId User to send refusal to
   * @param message Message with reason for refusal
   * @param eventId Event id of a pending transfer
   * @param flag1 ?
   * @param flag2 ?
   * @param viaServer True to send via server or false to use direct connection
   */
  virtual void fileTransferRefuse(const UserId& userId, const std::string& message,
      unsigned long eventId, unsigned long flag1, unsigned long flag2,
      bool viaServer = true) = 0;

  /**
   * Accept a proposed file transfer
   *
   * @param userId User to accept file transfer from
   * @param port Local tcp port to use
   * @param eventId Event id of transfer to accept
   * @param flag1 ?
   * @param flag2 ?
   * @param message Description text from file transfer event (ICQ only)
   * @param filename Filename from file transfer event (ICQ only)
   * @param filesize File size from file transfer event (ICQ only)
   * @param viaServer True to send via server or false to use direct connection
   */
  virtual void fileTransferAccept(const UserId& userId, unsigned short port,
      unsigned long eventId = 0, unsigned long flag1 = 0, unsigned long flag2 = 0,
      const std::string& message = "", const std::string filename = "",
      unsigned long filesize = 0, bool viaServer = true) = 0;

  /**
   * Grant or refuse authorization for a user to add us
   *
   * @param userId User to send grant to
   * @param grant True to grant or false to refuse
   * @param message Message to send with grant
   * @return Event id
   */
  virtual unsigned long authorizeReply(const UserId& userId, bool grant,
      const std::string& message) = 0;

  /**
   * Request user authorization
   *
   * @param userId User to request authorization from
   * @param message Message to send with request
   */
  virtual void requestAuthorization(const UserId& userid,
      const std::string& message) = 0;
   
  /**
   * Request user information from server
   *
   * @param userId User to get information for
   * @return Event id
   */
  virtual unsigned long requestUserInfo(const UserId& userId) = 0;

  /**
   * Update general user info for owner on server side
   * User info is taken from local data
   */
  virtual unsigned long updateOwnerInfo(const UserId& ownerId) = 0;

  /**
   * Request user picture from server
   *
   * @param userId User to get picture for
   * @return Event id
   */
  virtual unsigned long requestUserPicture(const UserId& userId) = 0;

  /**
   * Enable encrypted communication towards a user
   *
   * @param userId User to enable encryption for
   * @return Event id
   */
  virtual unsigned long secureChannelOpen(const UserId& userId) = 0;

  /**
   * Disable encrypted communication towards a user
   *
   * @param userId User to disable encryption for
   * @return Event id
   */
  virtual unsigned long secureChannelClose(const UserId& userId) = 0;

  /**
   * Cancel a request to file transfer / chat / secure channel
   *
   * @param userId User to cancel encryption for
   * @param eventId Event of request to cancel
   */
  virtual void cancelEvent(const UserId& userId, unsigned long eventId) = 0;

  /**
   * Add/remove a user to/from visible list
   *
   * @param userId User to change visible status for
   * @param visible True to add user to visible list or false to remove
   */
  virtual void visibleListSet(const UserId& userId, bool visible) = 0;

  /**
   * Add/remove a user to/from invisible list
   *
   * @param userId User to change invisible status for
   * @param invisible True to add user to invisible list or false to remove
   */
  virtual void invisibleListSet(const UserId& userId, bool invisible) = 0;

  /**
   * Add/remove a user to/from ignore list
   *
   * @param userId User to set ignore status for
   * @param ignore True to add user to ignore list or false to remove
   */
  virtual void ignoreListSet(const UserId& userId, bool ignore) = 0;

protected:
  virtual ~ProtocolManager() { /* Empty */ }
};

extern ProtocolManager& gProtocolManager;

} // namespace Licq

#endif
