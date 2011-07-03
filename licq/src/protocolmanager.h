/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq developers
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

#ifndef LICQDAEMON_PROTOCOLMANAGER_H
#define LICQDAEMON_PROTOCOLMANAGER_H

#include <licq/protocolmanager.h>

namespace Licq
{
class ProtocolSignal;
}

namespace LicqDaemon
{

class ProtocolManager : public Licq::ProtocolManager
{
public:
  ProtocolManager();
  ~ProtocolManager();

  /**
   * Add a user to server side list
   * Used by UserManager after user has been added to local list.
   * Plugins should call gUserManager.addUser() instead
   *
   * @param userId User to add
   * @param groupId Initial group, only used for ICQ contacts
   */
  void addUser(const Licq::UserId& userId, int groupId);

  /**
   * Remove a user from the server side list
   * Used by UserManager before removing user from local list.
   * Plugins should call gUserManageer.removeUser() instead
   *
   * @param userId Id of user to remove
   */
  void removeUser(const Licq::UserId& userId);

  // From Licq::ProtocolManager
  void updateUserAlias(const Licq::UserId& userId);
  unsigned long setStatus(const Licq::UserId& ownerId,
      unsigned newStatus, const std::string& message = KeepAutoResponse);
  void sendTypingNotification(const Licq::UserId& userId, bool active, int nSocket = -1);
  unsigned long sendMessage(const Licq::UserId& userId, const std::string& message,
      unsigned flags, const Licq::Color* color = NULL, unsigned long convoId = 0);
  unsigned long sendUrl(const Licq::UserId& userId, const std::string& url,
      const std::string& message, unsigned flags, const Licq::Color* color = NULL);
  unsigned long requestUserAutoResponse(const Licq::UserId& userId);
  unsigned long fileTransferPropose(const Licq::UserId& userId, const std::string& filename,
      const std::string& message, const std::list<std::string>& files, unsigned flags);
  void fileTransferRefuse(const Licq::UserId& userId, const std::string& message,
      unsigned long eventId, unsigned long flag1, unsigned long flag2,
      bool viaServer = true);
  void fileTransferCancel(const Licq::UserId& userId, unsigned long eventId);
  void fileTransferAccept(const Licq::UserId& userId, unsigned short port,
      unsigned long eventId = 0, unsigned long flag1 = 0, unsigned long flag2 = 0,
      const std::string& message = "", const std::string filename = "",
      unsigned long filesize = 0, bool viaServer = true);
  unsigned long authorizeReply(const Licq::UserId& userId, bool grant, const std::string& message);
  void requestAuthorization(const Licq::UserId& userid,
      const std::string& message);
  unsigned long requestUserInfo(const Licq::UserId& userId);
  unsigned long updateOwnerInfo(const Licq::UserId& ownerId);
  unsigned long requestUserPicture(const Licq::UserId& userId);
  unsigned long secureChannelOpen(const Licq::UserId& userId);
  unsigned long secureChannelClose(const Licq::UserId& userId);
  void secureChannelCancelOpen(const Licq::UserId& userId, unsigned long eventId);
  void visibleListSet(const Licq::UserId& userId, bool visible);
  void invisibleListSet(const Licq::UserId& userId, bool invisible);
  void ignoreListSet(const Licq::UserId& userId, bool ignore);

private:
  /**
   * Get next available id to use for an event
   */
  unsigned long getNextEventId();

  /**
   * @return true if the protocol is connected
   */
  bool isProtocolConnected(const Licq::UserId& userId);

  void pushProtoSignal(Licq::ProtocolSignal* s, const Licq::UserId& userId);
};

extern ProtocolManager gProtocolManager;

} // namespace Licq

#endif
