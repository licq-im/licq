/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq developers <licq-dev@googlegroups.com>
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
#include <licq/thread/mutexlocker.h>
#include <licq/userid.h>

#include "contactlist/user.h"
#include "daemon.h"
#include "gettext.h"

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
using std::list;
using std::string;

// Declare global PluginManager (internal for daemon)
LicqDaemon::ProtocolManager LicqDaemon::gProtocolManager;

// Initialize global Licq::PluginManager to refer to the internal PluginManager
Licq::ProtocolManager& Licq::gProtocolManager(LicqDaemon::gProtocolManager);

const char* const Licq::ProtocolManager::KeepAutoResponse = "__unset__";


ProtocolManager::ProtocolManager()
  : myNextEventId(1)
{
  // Empty
}

ProtocolManager::~ProtocolManager()
{
  // Empty
}

unsigned long ProtocolManager::getNextEventId()
{
  Licq::MutexLocker eventIdGuard(myNextEventIdMutex);
  unsigned long eventId = myNextEventId;
  if (++myNextEventId == 0)
    ++myNextEventId;
  return eventId;
}

bool ProtocolManager::isProtocolConnected(const UserId& userId)
{
  OwnerReadGuard owner(userId.ownerId());
  return owner.isLocked() && owner->isOnline();
}

void ProtocolManager::addUser(const UserId& userId)
{
  if (!isProtocolConnected(userId))
    return;

  gPluginManager.pushProtocolSignal(new Licq::ProtoAddUserSignal(userId));
}

void ProtocolManager::updateUserAlias(const UserId& userId)
{
  if (!isProtocolConnected(userId))
    return;

  gPluginManager.pushProtocolSignal(new Licq::ProtoRenameUserSignal(userId));
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
    gPluginManager.pushProtocolSignal(new Licq::ProtoLogoffSignal(ownerId));
  else if(isOffline)
    gPluginManager.pushProtocolSignal(new Licq::ProtoLogonSignal(ownerId, newStatus));
  else
    gPluginManager.pushProtocolSignal(new Licq::ProtoChangeStatusSignal(ownerId, newStatus));
  return eventId;
}

void ProtocolManager::sendTypingNotification(const UserId& userId, bool active, int nSocket)
{
  if (!isProtocolConnected(userId))
    return;

  if (!gDaemon.sendTypingNotification())
    return;

  gPluginManager.pushProtocolSignal(new Licq::ProtoTypingNotificationSignal(userId, active, nSocket));
}

unsigned long ProtocolManager::sendMessage(const UserId& userId, const string& message,
    unsigned flags, const Licq::Color* color, unsigned long convoId)
{
  if (!isProtocolConnected(userId))
    return 0;

  unsigned long eventId = getNextEventId();
  gPluginManager.pushProtocolSignal(new Licq::ProtoSendMessageSignal(
      eventId, userId, message, flags, color, convoId));
  return eventId;
}

unsigned long ProtocolManager::sendUrl(const UserId& userId, const string& url,
    const string& message, unsigned flags, const Licq::Color* color)
{
  if (!isProtocolConnected(userId))
    return 0;

  unsigned long eventId = getNextEventId();
  gPluginManager.pushProtocolSignal(new Licq::ProtoSendUrlSignal(
      eventId, userId, url, message, flags, color));
  return eventId;
}

unsigned long ProtocolManager::fileTransferPropose(const UserId& userId,
    const string& filename, const string& message, const list<string>& files,
    unsigned flags)
{
  if (!isProtocolConnected(userId))
    return 0;

  unsigned long eventId = getNextEventId();
  gPluginManager.pushProtocolSignal(new Licq::ProtoSendFileSignal(
      eventId, userId, filename, message, files, flags));
  return eventId;
}

void ProtocolManager::fileTransferAccept(const UserId& userId, unsigned short port,
    unsigned long eventId, unsigned long flag1, unsigned long flag2,
    const string& message, const string filename, unsigned long filesize,
    bool viaServer)
{
  if (!isProtocolConnected(userId))
    return;

  gPluginManager.pushProtocolSignal(new Licq::ProtoSendEventReplySignal(
      userId, message, true, port, eventId, flag1, flag2, !viaServer, filename, filesize));
}

void ProtocolManager::fileTransferRefuse(const UserId& userId, const string& message,
    unsigned long eventId, unsigned long flag1, unsigned long flag2, bool viaServer)
{
  if (!isProtocolConnected(userId))
    return;

  gPluginManager.pushProtocolSignal(new Licq::ProtoSendEventReplySignal(
      userId, message, false, 0, eventId, flag1, flag2, !viaServer));
}

unsigned long ProtocolManager::authorizeReply(const UserId& userId, bool grant, const string& message)
{
  if (!isProtocolConnected(userId))
    return 0;

  unsigned long eventId = getNextEventId();

  if (grant)
    gPluginManager.pushProtocolSignal(new Licq::ProtoGrantAuthSignal(eventId, userId, message));
  else
    gPluginManager.pushProtocolSignal(new Licq::ProtoRefuseAuthSignal(eventId, userId, message));

  return eventId;
}

void ProtocolManager::requestAuthorization(
    const UserId& userId, const string& message)
{
  if (!isProtocolConnected(userId))
    return;

  gPluginManager.pushProtocolSignal(new Licq::ProtoRequestAuthSignal(userId, message));
}

unsigned long ProtocolManager::requestUserInfo(const UserId& userId)
{
  if (!isProtocolConnected(userId))
    return 0;

  unsigned long eventId = getNextEventId();
  gPluginManager.pushProtocolSignal(new Licq::ProtoRequestInfo(eventId, userId));
  return eventId;
}

unsigned long ProtocolManager::updateOwnerInfo(const UserId& ownerId)
{
  if (!isProtocolConnected(ownerId))
    return 0;

  unsigned long eventId = getNextEventId();
  gPluginManager.pushProtocolSignal(new Licq::ProtoUpdateInfoSignal(eventId, ownerId));
  return eventId;
}

unsigned long ProtocolManager::requestUserPicture(const UserId& userId)
{
  if (!isProtocolConnected(userId))
    return 0;

  unsigned long eventId = getNextEventId();
  gPluginManager.pushProtocolSignal(new Licq::ProtoRequestPicture(eventId, userId));
  return eventId;
}

unsigned long ProtocolManager::secureChannelOpen(const UserId& userId)
{
  if (!isProtocolConnected(userId))
    return 0;

  if (userId.isOwner())
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
  gPluginManager.pushProtocolSignal(new Licq::ProtoOpenSecureSignal(eventId, userId));
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
  gPluginManager.pushProtocolSignal(new Licq::ProtoCloseSecureSignal(eventId, userId));
  return eventId;
}

void ProtocolManager::cancelEvent(const UserId& userId, unsigned long eventId)
{
  if (!isProtocolConnected(userId))
    return;

  gPluginManager.pushProtocolSignal(new Licq::ProtoCancelEventSignal(userId, eventId));
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
    gPluginManager.pushProtocolSignal(new Licq::ProtoAcceptUserSignal(userId));
  else
    gPluginManager.pushProtocolSignal(new Licq::ProtoUnacceptUserSignal(userId));

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
    gPluginManager.pushProtocolSignal(new Licq::ProtoBlockUserSignal(userId));
  else
    gPluginManager.pushProtocolSignal(new Licq::ProtoUnblockUserSignal(userId));

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
    gPluginManager.pushProtocolSignal(new Licq::ProtoIgnoreUserSignal(userId));
  else
    gPluginManager.pushProtocolSignal(new Licq::ProtoUnignoreUserSignal(userId));

  gUserManager.notifyUserUpdated(userId, PluginSignal::UserSettings);
}
