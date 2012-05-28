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

#include "protocolmanager.h"

#include <licq/logging/log.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/protocolsignal.h>
#include <licq/userid.h>

#include "contactlist/user.h"
#include "daemon.h"
#include "gettext.h"
#include "icq/icq.h"

using namespace std;
using namespace LicqDaemon;
using Licq::OwnerReadGuard;
using Licq::OwnerWriteGuard;
using Licq::PluginSignal;
using Licq::User;
using Licq::UserId;
using Licq::UserReadGuard;
using Licq::UserWriteGuard;
using Licq::gLog;
using Licq::gPluginManager;
using Licq::gUserManager;
using LicqIcq::gIcqProtocol;


// Declare global PluginManager (internal for daemon)
LicqDaemon::ProtocolManager LicqDaemon::gProtocolManager;

// Initialize global Licq::PluginManager to refer to the internal PluginManager
Licq::ProtocolManager& Licq::gProtocolManager(LicqDaemon::gProtocolManager);

const char* const Licq::ProtocolManager::KeepAutoResponse = "__unset__";


ProtocolManager::ProtocolManager()
{
  // Empty
}

ProtocolManager::~ProtocolManager()
{
  // Empty
}

inline unsigned long ProtocolManager::getNextEventId()
{
  // Event id generation is still owned by daemon as it's used directly by some ICQ functions
  return gDaemon.getNextEventId();
}

bool ProtocolManager::isProtocolConnected(const UserId& userId)
{
  OwnerReadGuard owner(userId.protocolId());
  return owner.isLocked() && owner->isOnline();
}

void ProtocolManager::pushProtoSignal(Licq::ProtocolSignal* s, const UserId& userId)
{
  gPluginManager.pushProtocolSignal(s, userId.protocolId());
}

void ProtocolManager::addUser(const UserId& userId)
{
  if (!isProtocolConnected(userId))
    return;

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.icqAddUser(userId, false);
  else
    pushProtoSignal(new Licq::ProtoAddUserSignal(userId, false), userId);
}

void ProtocolManager::removeUser(const UserId& userId)
{
  if (!isProtocolConnected(userId))
    return;

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.icqRemoveUser(userId);
  else
    pushProtoSignal(new Licq::ProtoRemoveUserSignal(userId), userId);
}

void ProtocolManager::updateUserAlias(const UserId& userId)
{
  if (!isProtocolConnected(userId))
    return;

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.icqRenameUser(userId);
  else
    pushProtoSignal(new Licq::ProtoRenameUserSignal(userId), userId);
}

unsigned long ProtocolManager::setStatus(const UserId& ownerId,
    unsigned newStatus, const string& message)
{
  bool isOffline;

  {
    OwnerWriteGuard owner(ownerId);
    if (!owner.isLocked())
      return 0;

    isOffline = !owner->isOnline();
    if (message != KeepAutoResponse)
    {
      owner->setAutoResponse(message);
      owner->save(Licq::Owner::SaveOwnerInfo);
    }
  }

  unsigned long eventId = 0;
  if (newStatus == User::OfflineStatus)
  {
    if (ownerId.protocolId() == LICQ_PPID)
      gIcqProtocol.icqLogoff();
    else
      pushProtoSignal(new Licq::ProtoLogoffSignal(), ownerId);
  }
  else if(isOffline)
  {
    if (ownerId.protocolId() == LICQ_PPID)
      eventId = gIcqProtocol.logon(newStatus);
    else
      pushProtoSignal(new Licq::ProtoLogonSignal(newStatus), ownerId);
  }
  else
  {
    if (ownerId.protocolId() == LICQ_PPID)
      eventId = gIcqProtocol.setStatus(newStatus);
    else
      pushProtoSignal(new Licq::ProtoChangeStatusSignal(newStatus), ownerId);
  }

  return eventId;
}

void ProtocolManager::sendTypingNotification(const UserId& userId, bool active, int nSocket)
{
  if (!isProtocolConnected(userId))
    return;

  if (!gDaemon.sendTypingNotification())
    return;

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.icqTypingNotification(userId, active);
  else
    pushProtoSignal(new Licq::ProtoTypingNotificationSignal(userId, active, nSocket), userId);
}

unsigned long ProtocolManager::sendMessage(const UserId& userId, const string& message,
    unsigned flags, const Licq::Color* color, unsigned long convoId)
{
  if (!isProtocolConnected(userId))
    return 0;

  unsigned long eventId = getNextEventId();

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.icqSendMessage(eventId, userId, message, flags, color);
  else
    pushProtoSignal(new Licq::ProtoSendMessageSignal(eventId, userId, message, flags, convoId), userId);

  return eventId;
}

unsigned long ProtocolManager::sendUrl(const UserId& userId, const string& url,
    const string& message, unsigned flags, const Licq::Color* color)
{
  if (!isProtocolConnected(userId))
    return 0;

  unsigned long eventId = getNextEventId();

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.icqSendUrl(eventId, userId, url, message, flags, color);
  else
    eventId = 0;

  return eventId;
}

unsigned long ProtocolManager::requestUserAutoResponse(const UserId& userId)
{
  if (!isProtocolConnected(userId))
    return 0;

  unsigned long eventId = getNextEventId();

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.icqFetchAutoResponseServer(eventId, userId);
  else
    eventId = 0;

  return eventId;
}

unsigned long ProtocolManager::fileTransferPropose(const UserId& userId,
    const string& filename, const string& message, const list<string>& files,
    unsigned flags)
{
  if (!isProtocolConnected(userId))
    return 0;

  unsigned long eventId = getNextEventId();

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.icqFileTransfer(eventId, userId, filename, message, files, flags);
  else
    pushProtoSignal(new Licq::ProtoSendFileSignal(eventId, userId, filename, message, files), userId);

  return eventId;
}

void ProtocolManager::fileTransferAccept(const UserId& userId, unsigned short port,
    unsigned long eventId, unsigned long flag1, unsigned long flag2,
    const string& message, const string filename, unsigned long filesize,
    bool viaServer)
{
  if (!isProtocolConnected(userId))
    return;

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.icqFileTransferAccept(userId, port, (unsigned short)eventId,
        flag1, flag2, viaServer, message, filename, filesize);
  else
    pushProtoSignal(new Licq::ProtoSendEventReplySignal(userId, string(), true, port,
        eventId, flag1, flag2, !viaServer), userId);
}

void ProtocolManager::fileTransferRefuse(const UserId& userId, const string& message,
    unsigned long eventId, unsigned long flag1, unsigned long flag2, bool viaServer)
{
  if (!isProtocolConnected(userId))
    return;

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.icqFileTransferRefuse(userId, message, (unsigned short)eventId, flag1, flag2, viaServer);
  else
    pushProtoSignal(new Licq::ProtoSendEventReplySignal(userId, message, false,
        eventId, flag1, flag2, !viaServer), userId);
}

unsigned long ProtocolManager::authorizeReply(const UserId& userId, bool grant, const string& message)
{
  if (!isProtocolConnected(userId))
    return 0;

  unsigned long eventId = 0;

  if (grant)
  {
    if (userId.protocolId() == LICQ_PPID)
      eventId = gIcqProtocol.icqAuthorizeGrant(userId, message);
    else
      pushProtoSignal(new Licq::ProtoGrantAuthSignal(userId, message), userId);
  }
  else
  {
    if (userId.protocolId() == LICQ_PPID)
      eventId = gIcqProtocol.icqAuthorizeRefuse(userId, message);
    else
      pushProtoSignal(new Licq::ProtoRefuseAuthSignal(userId, message), userId);
  }

  return eventId;
}

void ProtocolManager::requestAuthorization(
    const UserId& userId, const string& message)
{
  if (!isProtocolConnected(userId))
    return;

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.icqRequestAuth(userId, message);
  else
    pushProtoSignal(new Licq::ProtoRequestAuthSignal(userId, message), userId);
}

unsigned long ProtocolManager::requestUserInfo(const UserId& userId)
{
  if (!isProtocolConnected(userId))
    return 0;

  unsigned long eventId = 0;

  if (userId.protocolId() == LICQ_PPID)
    eventId = gIcqProtocol.icqRequestMetaInfo(userId);
  else
    pushProtoSignal(new Licq::ProtoRequestInfo(userId), userId);

  return eventId;
}

unsigned long ProtocolManager::updateOwnerInfo(const UserId& ownerId)
{
  if (!isProtocolConnected(ownerId))
    return 0;

  unsigned long eventId = 0;

  if (ownerId.protocolId() == LICQ_PPID)
    eventId = gIcqProtocol.icqSetGeneralInfo(ownerId);
  else
    pushProtoSignal(new Licq::ProtoUpdateInfoSignal(), ownerId);

  return eventId;
}

unsigned long ProtocolManager::requestUserPicture(const UserId& userId)
{
  if (!isProtocolConnected(userId))
    return 0;

  unsigned long eventId = 0;

  if (userId.protocolId() == LICQ_PPID)
    eventId = gIcqProtocol.icqRequestPicture(userId);
  else
    pushProtoSignal(new Licq::ProtoRequestPicture(userId), userId);

  return eventId;
}

unsigned long ProtocolManager::secureChannelOpen(const UserId& userId)
{
  if (!isProtocolConnected(userId))
    return 0;

  if (gUserManager.isOwner(userId))
    return 0;

  {
    UserReadGuard user(userId);
    if (!user.isLocked())
    {
      gLog.warning(tr("Cannot send secure channel request to user not on list (%s)."),
          userId.toString().c_str());
      return 0;
    }

    // Check that the user doesn't already have a secure channel
    if (user->Secure())
    {
      gLog.warning(tr("%s (%s) already has a secure channel."),
          user->getAlias().c_str(), userId.toString().c_str());
      return 0;
    }
  }

  unsigned long eventId = getNextEventId();

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.icqOpenSecureChannel(eventId, userId);
  else
    pushProtoSignal(new Licq::ProtoOpenSecureSignal(eventId, userId), userId);

  return eventId;
}

unsigned long ProtocolManager::secureChannelClose(const UserId& userId)
{
  if (!isProtocolConnected(userId))
    return 0;

  {
    UserReadGuard user(userId);
    if (!user.isLocked())
    {
      gLog.warning(tr("Cannot send secure channel request to user not on list (%s)."),
          userId.toString().c_str());
      return 0;
    }

    // Check that the user have a secure channel to close
    if (!user->Secure())
    {
      gLog.warning(tr("%s (%s) does not have a secure channel."),
          user->getAlias().c_str(), userId.toString().c_str());
      return 0;
    }
  }

  unsigned long eventId = getNextEventId();

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.icqCloseSecureChannel(eventId, userId);
  else
    pushProtoSignal(new Licq::ProtoCloseSecureSignal(eventId, userId), userId);

  return eventId;
}

void ProtocolManager::cancelEvent(const UserId& userId, unsigned long eventId)
{
  if (!isProtocolConnected(userId))
    return;

  if (userId.protocolId() == LICQ_PPID)
    gIcqProtocol.CancelEvent(eventId);
  else
    pushProtoSignal(new Licq::ProtoCancelEventSignal(userId, eventId), userId);
}

void ProtocolManager::visibleListSet(const UserId& userId, bool visible)
{
  if (!isProtocolConnected(userId))
    return;

  {
    UserWriteGuard u(userId);
    u->SetVisibleList(visible);
  }

  if (visible)
    if (userId.protocolId() == LICQ_PPID)
      gIcqProtocol.icqAddToVisibleList(userId);
    else
      pushProtoSignal(new Licq::ProtoAcceptUserSignal(userId), userId);
  else
    if (userId.protocolId() == LICQ_PPID)
      gIcqProtocol.icqRemoveFromVisibleList(userId);
    else
      pushProtoSignal(new Licq::ProtoUnacceptUserSignal(userId), userId);

  gUserManager.notifyUserUpdated(userId, PluginSignal::UserSettings);
}

void ProtocolManager::invisibleListSet(const UserId& userId, bool invisible)
{
  if (!isProtocolConnected(userId))
    return;

  {
    UserWriteGuard u(userId);
    u->SetInvisibleList(invisible);
  }

  if (invisible)
    if (userId.protocolId() == LICQ_PPID)
      gIcqProtocol.icqAddToInvisibleList(userId);
    else
      pushProtoSignal(new Licq::ProtoBlockUserSignal(userId), userId);
  else
    if (userId.protocolId() == LICQ_PPID)
      gIcqProtocol.icqRemoveFromInvisibleList(userId);
    else
      pushProtoSignal(new Licq::ProtoUnblockUserSignal(userId), userId);

  gUserManager.notifyUserUpdated(userId, PluginSignal::UserSettings);
}

void ProtocolManager::ignoreListSet(const UserId& userId, bool ignore)
{
  if (!isProtocolConnected(userId))
    return;

  {
    UserWriteGuard u(userId);
    u->SetIgnoreList(ignore);
  }

  if (ignore)
    if (userId.protocolId() == LICQ_PPID)
      gIcqProtocol.icqAddToIgnoreList(userId);
    else
      pushProtoSignal(new Licq::ProtoIgnoreUserSignal(userId), userId);
  else
    if (userId.protocolId() == LICQ_PPID)
      gIcqProtocol.icqRemoveFromIgnoreList(userId);
    else
      pushProtoSignal(new Licq::ProtoUnignoreUserSignal(userId), userId);

  gUserManager.notifyUserUpdated(userId, PluginSignal::UserSettings);
}
