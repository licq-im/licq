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

#include "protocolmanager.h"

#include <licq_events.h>
#include <licq_icqd.h>
#include <licq_log.h>
#include <licq/contactlist/usermanager.h>
#include <licq/userid.h>

#include "gettext.h"

using namespace std;
using namespace LicqDaemon;
using Licq::OwnerReadGuard;
using Licq::OwnerWriteGuard;
using Licq::UserId;
using Licq::UserReadGuard;
using Licq::gUserManager;


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

unsigned long ProtocolManager::getNextEventId()
{
  // Event id generation is still owned by daemon as it's used directly by some ICQ functions
  return gLicqDaemon->getNextEventId();
}

void ProtocolManager::pushProtoSignal(LicqProtoSignal* s, const UserId& userId)
{
  gLicqDaemon->PushProtoSignal(s, userId.protocolId());
}

void ProtocolManager::addUser(const UserId& userId, int groupId)
{
  if (userId.protocolId() == LICQ_PPID)
    gLicqDaemon->icqAddUser(userId.accountId().c_str(), false, groupId);
  else
    pushProtoSignal(new LicqProtoAddUserSignal(userId, false), userId);
}

void ProtocolManager::removeUser(const UserId& userId)
{
  bool tempUser;

  {
    UserReadGuard user(userId);
    if (!user.isLocked())
      return;

    tempUser = user->NotInList();
  }

  if (userId.protocolId() == LICQ_PPID)
  {
    if (!tempUser)
      gLicqDaemon->icqRemoveUser(userId.accountId().c_str());
  }
  else
    pushProtoSignal(new LicqProtoRemoveUserSignal(userId), userId);
}

void ProtocolManager::updateUserAlias(const UserId& userId)
{
  string newAlias;

  {
    UserReadGuard user(userId);
    if (!user.isLocked())
      return;

    newAlias = user->getAlias();
  }

  if (userId.protocolId() == LICQ_PPID)
    gLicqDaemon->icqRenameUser(userId.accountId(), newAlias);
  else
    pushProtoSignal(new LicqProtoRenameUserSignal(userId), userId);
}

unsigned long ProtocolManager::setStatus(const UserId& ownerId,
    unsigned short newStatus, const string& message)
{
  bool isOffline;

  {
    OwnerWriteGuard owner(ownerId);
    if (!owner.isLocked())
      return 0;

    isOffline = owner->StatusOffline();
    if (message != KeepAutoResponse)
      owner->SetAutoResponse(message.c_str());
  }

  unsigned long eventId = 0;

  if (newStatus == ICQ_STATUS_OFFLINE)
  {
    if (isOffline)
      return 0;

    if (ownerId.protocolId() == LICQ_PPID)
      gLicqDaemon->icqLogoff();
    else
      pushProtoSignal(new LicqProtoLogoffSignal(), ownerId);
  }
  else if(isOffline)
  {
    if (ownerId.protocolId() == LICQ_PPID)
      eventId = gLicqDaemon->icqLogon(newStatus);
    else
      pushProtoSignal(new LicqProtoLogonSignal(newStatus), ownerId);
  }
  else
  {
    if (ownerId.protocolId() == LICQ_PPID)
      eventId = gLicqDaemon->icqSetStatus(newStatus);
    else
      pushProtoSignal(new LicqProtoChangeStatusSignal(newStatus), ownerId);
  }

  return eventId;
}

void ProtocolManager::sendTypingNotification(const UserId& userId, bool active, int nSocket)
{
  if (!gLicqDaemon->SendTypingNotification())
    return;

  if (userId.protocolId() == LICQ_PPID)
    gLicqDaemon->icqTypingNotification(userId.accountId().c_str(), active);
  else
    pushProtoSignal(new LicqProtoTypingNotificationSignal(userId, active, nSocket), userId);
}

unsigned long ProtocolManager::sendMessage(const UserId& userId, const string& message,
    bool viaServer, unsigned short flags, bool multipleRecipients, CICQColor* color,
    unsigned long convoId)
{
  unsigned long eventId = getNextEventId();

  if (userId.protocolId() == LICQ_PPID)
    gLicqDaemon->icqSendMessage(eventId, userId, message, viaServer, flags, multipleRecipients, color);
  else
    pushProtoSignal(new LicqProtoSendMessageSignal(eventId, userId, message, convoId), userId);

  return eventId;
}

unsigned long ProtocolManager::sendUrl(const UserId& userId, const string& url,
    const string& message, bool viaServer, unsigned short flags,
    bool multipleRecipients, CICQColor* color)
{
  unsigned long eventId = getNextEventId();

  if (userId.protocolId() == LICQ_PPID)
    gLicqDaemon->icqSendUrl(eventId, userId, url, message, viaServer, flags, multipleRecipients, color);
  else
    eventId = 0;

  return eventId;
}

unsigned long ProtocolManager::requestUserAutoResponse(const UserId& userId)
{
  unsigned long eventId = getNextEventId();

  if (userId.protocolId() == LICQ_PPID)
    gLicqDaemon->icqFetchAutoResponseServer(eventId, userId.accountId().c_str());
  else
    eventId = 0;

  return eventId;
}


unsigned long ProtocolManager::fileTransferPropose(const UserId& userId,
    const string& filename, const string& message, ConstFileList& files,
    unsigned short flags, bool viaServer)
{
  unsigned long eventId = getNextEventId();

  if (userId.protocolId() == LICQ_PPID)
    gLicqDaemon->icqFileTransfer(eventId, userId, filename, message, files, flags, viaServer);
  else
    pushProtoSignal(new LicqProtoSendFileSignal(eventId, userId, filename, message, files), userId);

  return eventId;
}

void ProtocolManager::fileTransferCancel(const UserId& userId, unsigned long eventId)
{
  if (userId.protocolId() == LICQ_PPID)
    gLicqDaemon->icqFileTransferCancel(userId, (unsigned short)eventId);
  else
    pushProtoSignal(new LicqProtoCancelEventSignal(userId, eventId), userId);
}

void ProtocolManager::fileTransferAccept(const UserId& userId, unsigned short port,
    unsigned long eventId, unsigned long flag1, unsigned long flag2,
    const string& message, const string filename, unsigned long filesize,
    bool viaServer)
{
  if (userId.protocolId() == LICQ_PPID)
  {
    unsigned long nMsgId[] = { flag1, flag2 };
    gLicqDaemon->icqFileTransferAccept(userId, port, (unsigned short)eventId,
        nMsgId, viaServer, message, filename, filesize);
  }
  else
    pushProtoSignal(new LicqProtoSendEventReplySignal(userId, string(), true, port,
        eventId, flag1, flag2, !viaServer), userId);
}

void ProtocolManager::fileTransferRefuse(const UserId& userId, const string& message,
    unsigned long eventId, unsigned long flag1, unsigned long flag2, bool viaServer)
{
  if (userId.protocolId() == LICQ_PPID)
  {
    unsigned long msgId[] = { flag1, flag2 };
    gLicqDaemon->icqFileTransferRefuse(userId, message, (unsigned short)eventId, msgId, viaServer);
  }
  else
    pushProtoSignal(new LicqProtoSendEventReplySignal(userId, message, false,
        eventId, flag1, flag2, !viaServer), userId);
}

unsigned long ProtocolManager::authorizeReply(const UserId& userId, bool grant, const string& message)
{
  unsigned long eventId = 0;

  if (grant)
  {
    if (userId.protocolId() == LICQ_PPID)
      eventId = gLicqDaemon->icqAuthorizeGrant(userId, message);
    else
      pushProtoSignal(new LicqProtoGrantAuthSignal(userId, message), userId);
  }
  else
  {
    if (userId.protocolId() == LICQ_PPID)
      eventId = gLicqDaemon->icqAuthorizeRefuse(userId, message);
    else
      pushProtoSignal(new LicqProtoRefuseAuthSignal(userId, message), userId);
  }

  return eventId;
}

unsigned long ProtocolManager::requestUserInfo(const UserId& userId)
{
  unsigned long eventId = 0;

  if (userId.protocolId() == LICQ_PPID)
    eventId = gLicqDaemon->icqRequestMetaInfo(userId.accountId().c_str());
  else
    pushProtoSignal(new LicqProtoRequestInfo(userId), userId);

  return eventId;
}

unsigned long ProtocolManager::updateOwnerInfo(const UserId& ownerId)
{
  string alias, firstName, lastName, email, address, city, state, zipCode;
  string phoneNumber, faxNumber, cellNumber;
  unsigned short countryCode;
  bool hideEmail;

  {
    OwnerReadGuard owner(ownerId);
    if (!owner.isLocked())
      return 0;

    alias = owner->getAlias();
    firstName = owner->getFirstName();
    firstName = owner->getLastName();
    email = owner->getUserInfoString("Email1");
    address = owner->getUserInfoString("Address");
    city = owner->getUserInfoString("City");
    state = owner->getUserInfoString("State");
    zipCode = owner->getUserInfoString("Zipcode");
    phoneNumber = owner->getUserInfoString("PhoneNumber");
    faxNumber = owner->getUserInfoString("FaxNumber");
    cellNumber = owner->getUserInfoString("CellularNumber");
    countryCode = owner->getUserInfoUint("Country");
    hideEmail = owner->getUserInfoBool("HideEmail");
  }

  unsigned long eventId = 0;

  if (ownerId.protocolId() == LICQ_PPID)
    eventId = gLicqDaemon->icqSetGeneralInfo(alias.c_str(), firstName.c_str(),
        lastName.c_str(), email.c_str(), city.c_str(), state.c_str(),
        phoneNumber.c_str(), faxNumber.c_str(), address.c_str(),
        cellNumber.c_str(), zipCode.c_str(), countryCode, hideEmail);
  else
    pushProtoSignal(new LicqProtoUpdateInfoSignal(alias, firstName,
        lastName, email, city, state, phoneNumber, faxNumber, address,
        cellNumber, zipCode), ownerId);

  return eventId;
}

unsigned long ProtocolManager::requestUserPicture(const UserId& userId)
{
  size_t iconHashSize;
  bool sendServer;
  {
    UserReadGuard user(userId);
    if (!user.isLocked())
      return 0;

    iconHashSize = strlen(user->BuddyIconHash());
    sendServer = (user->SocketDesc(ICQ_CHNxINFO) < 0);
  }

  unsigned long eventId = 0;

  if (userId.protocolId() == LICQ_PPID)
    eventId = gLicqDaemon->icqRequestPicture(userId, sendServer, iconHashSize);
  else
    pushProtoSignal(new LicqProtoRequestPicture(userId), userId);

  return eventId;
}

unsigned long ProtocolManager::secureChannelOpen(const UserId& userId)
{
  if (gUserManager.isOwner(userId))
    return 0;

  {
    UserReadGuard user(userId);
    if (!user.isLocked())
    {
      gLog.Warn(tr("%sCannot send secure channel request to user not on list (%s).\n"),
          L_WARNxSTR, userId.toString().c_str());
      return 0;
    }

    // Check that the user doesn't already have a secure channel
    if (user->Secure())
    {
      gLog.Warn(tr("%s%s (%s) already has a secure channel.\n"), L_WARNxSTR,
          user->getAlias().c_str(), userId.toString().c_str());
      return 0;
    }
  }

  unsigned long eventId = getNextEventId();

  if (userId.protocolId() == LICQ_PPID)
    gLicqDaemon->icqOpenSecureChannel(eventId, userId);
  else
    pushProtoSignal(new LicqProtoOpenSecureSignal(eventId, userId), userId);

  return eventId;
}

unsigned long ProtocolManager::secureChannelClose(const UserId& userId)
{
  {
    UserReadGuard user(userId);
    if (!user.isLocked())
    {
      gLog.Warn(tr("%sCannot send secure channel request to user not on list (%s).\n"),
          L_WARNxSTR, userId.toString().c_str());
      return 0;
    }

    // Check that the user have a secure channel to close
    if (!user->Secure())
    {
      gLog.Warn(tr("%s%s (%s) does not have a secure channel.\n"), L_WARNxSTR,
          user->getAlias().c_str(), userId.toString().c_str());
      return 0;
    }
  }

  unsigned long eventId = getNextEventId();

  if (userId.protocolId() == LICQ_PPID)
    gLicqDaemon->icqCloseSecureChannel(eventId, userId);
  else
    pushProtoSignal(new LicqProtoCloseSecureSignal(eventId, userId), userId);

  return eventId;
}

void ProtocolManager::secureChannelCancelOpen(const UserId& userId, unsigned long eventId)
{
  if (userId.protocolId() == LICQ_PPID)
    gLicqDaemon->icqOpenSecureChannelCancel(userId, (unsigned short)eventId);
  else
    pushProtoSignal(new LicqProtoCancelEventSignal(userId, eventId), userId);
}

void ProtocolManager::visibleListSet(const UserId& userId, bool visible)
{
  if (visible)
    if (userId.protocolId() == LICQ_PPID)
      gLicqDaemon->icqAddToVisibleList(userId);
    else
      pushProtoSignal(new LicqProtoAcceptUserSignal(userId), userId);
  else
    if (userId.protocolId() == LICQ_PPID)
      gLicqDaemon->icqRemoveFromVisibleList(userId);
    else
      pushProtoSignal(new LicqProtoUnacceptUserSignal(userId), userId);
}

void ProtocolManager::invisibleListSet(const UserId& userId, bool invisible)
{
  if (invisible)
    if (userId.protocolId() == LICQ_PPID)
      gLicqDaemon->icqAddToInvisibleList(userId);
    else
      pushProtoSignal(new LicqProtoBlockUserSignal(userId), userId);
  else
    if (userId.protocolId() == LICQ_PPID)
      gLicqDaemon->icqRemoveFromInvisibleList(userId);
    else
      pushProtoSignal(new LicqProtoUnblockUserSignal(userId), userId);
}

void ProtocolManager::ignoreListSet(const UserId& userId, bool ignore)
{
  if (ignore)
    if (userId.protocolId() == LICQ_PPID)
      gLicqDaemon->icqAddToIgnoreList(userId);
    else
      pushProtoSignal(new LicqProtoIgnoreUserSignal(userId), userId);
  else
    if (userId.protocolId() == LICQ_PPID)
      gLicqDaemon->icqRemoveFromIgnoreList(userId);
    else
      pushProtoSignal(new LicqProtoUnignoreUserSignal(userId), userId);
}
