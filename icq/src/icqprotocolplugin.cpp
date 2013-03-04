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

#include "icqprotocolplugin.h"

#include <licq/logging/log.h>
#include <licq/protocolmanager.h>
#include <licq/protocolsignal.h>
#include <licq/version.h>

#include "chat.h"
#include "filetransfer.h"
#include "gettext.h"
#include "icq.h"
#include "owner.h"
#include "protocolsignal.h"
#include "user.h"

using namespace LicqIcq;
using Licq::gLog;

IcqProtocolPlugin* LicqIcq::gIcqProtocolPlugin(NULL);

IcqProtocolPlugin::IcqProtocolPlugin()
{
  gIcqProtocolPlugin = this;
}

IcqProtocolPlugin::~IcqProtocolPlugin()
{
  gIcqProtocolPlugin = NULL;
}

bool IcqProtocolPlugin::init(int /*argc*/, char** /*argv*/)
{
  gIcqProtocol.initialize();
  return true;
}

int IcqProtocolPlugin::run()
{
  if (!gIcqProtocol.start())
    return 1;
  return 0;
}

std::string IcqProtocolPlugin::defaultServerHost() const
{
  return "login.icq.com";
}

int IcqProtocolPlugin::defaultServerPort() const
{
  return 5190;
}

void IcqProtocolPlugin::processPipe()
{
  char c;
  read(getReadPipe(), &c, 1);
  switch (c)
  {
    case PipeSignal:
      gIcqProtocol.processSignal(popSignal().get());
      break;
    case PipeShutdown:
      gIcqProtocol.shutdown();
      break;
    default:
      gLog.error(tr("Unknown command via plugin pipe: %c"), c);
  }
}

bool IcqProtocolPlugin::isOwnerOnline(const Licq::UserId& userId)
{
  Licq::OwnerReadGuard owner(userId.ownerId());
  return owner.isLocked() && owner->isOnline();
}

void IcqProtocolPlugin::pushSignal(Licq::ProtocolSignal* signal)
{
  pushSignal(boost::shared_ptr<const Licq::ProtocolSignal>(signal));
}

unsigned long IcqProtocolPlugin::icqSendContactList(const Licq::UserId& userId,
    const Licq::StringList& users, unsigned flags, const Licq::Color* color)
{
  if (!isOwnerOnline(userId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoSendContactsSignal(eventId, userId, users, flags, color));
  return eventId;
}

unsigned long IcqProtocolPlugin::icqFetchAutoResponse(const Licq::UserId& userId)
{
  if (!isOwnerOnline(userId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoFetchAutoResponseSignal(eventId, userId));
  return eventId;
}

unsigned long IcqProtocolPlugin::icqChatRequest(const Licq::UserId& userId, const std::string& reason,
    unsigned flags, const std::string& chatUsers, unsigned short port)
{
  if (!isOwnerOnline(userId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoChatRequestSignal(eventId, userId, reason, flags, chatUsers, port));
  return eventId;
}

void IcqProtocolPlugin::icqChatRequestRefuse(const Licq::UserId& userId, const std::string& reason,
    unsigned short sequence, unsigned long msgid1, unsigned long msgid2, bool direct)
{
  if (!isOwnerOnline(userId))
    return;

  pushSignal(new ProtoChatRefuseSignal(userId, reason, sequence, msgid1, msgid2, direct));
}

void IcqProtocolPlugin::icqChatRequestAccept(const Licq::UserId& userId, unsigned short port,
    const std::string& clients, unsigned short sequence,
    unsigned long msgid1, unsigned long msgid2, bool direct)
{
  if (!isOwnerOnline(userId))
    return;

  pushSignal(new ProtoChatAcceptSignal(userId, port, clients, sequence, msgid1, msgid2, direct));
}

unsigned long IcqProtocolPlugin::icqRequestPluginInfo(const Licq::UserId& userId,
    Licq::IcqProtocol::PluginType type, bool server)
{
  if (!isOwnerOnline(userId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoRequestPluginSignal(eventId, userId, type, server));
  return eventId;
}

void IcqProtocolPlugin::icqUpdateInfoTimestamp(const Licq::UserId& ownerId, Licq::IcqProtocol::PluginType type)
{
  if (!isOwnerOnline(ownerId))
    return;

  pushSignal(new ProtoUpdateTimestampSignal(ownerId, type));
}

unsigned long IcqProtocolPlugin::icqSetWorkInfo(const Licq::UserId& ownerId,
    const std::string& city, const std::string& state, const std::string& phone,
    const std::string& fax, const std::string& address, const std::string& zip,
    unsigned short country, const std::string& name, const std::string& department,
    const std::string& position, unsigned short occupation, const std::string& homepage)
{
  if (!isOwnerOnline(ownerId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoUpdateWorkSignal(eventId, ownerId, city, state, phone, fax, address,
      zip, country, name, department, position, occupation, homepage));
  return eventId;
}

unsigned long IcqProtocolPlugin::icqSetEmailInfo(const Licq::UserId& ownerId,
    const std::string& emailSecondary, const std::string& emailOld)
{
  if (!isOwnerOnline(ownerId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoUpdateEmailSignal(eventId, ownerId, emailSecondary, emailOld));
  return eventId;
}

unsigned long IcqProtocolPlugin::icqSetMoreInfo(const Licq::UserId& ownerId,
    unsigned short age, char gender, const std::string& homepage,
    unsigned short birthYear, char birthMonth, char birthDay,
    char language1, char language2, char language3)
{
  if (!isOwnerOnline(ownerId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoUpdateMoreSignal(eventId, ownerId, age, gender, homepage,
      birthYear, birthMonth, birthDay, language1, language2, language3));
  return eventId;
}

unsigned long IcqProtocolPlugin::icqSetSecurityInfo(const Licq::UserId& ownerId,
    bool authorize, bool webAware)
{
  if (!isOwnerOnline(ownerId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoUpdateSecuritySignal(eventId, ownerId, authorize, webAware));
  return eventId;
}

unsigned long IcqProtocolPlugin::icqSetInterestsInfo(const Licq::UserId& ownerId,
    const Licq::UserCategoryMap& interests)
{
  if (!isOwnerOnline(ownerId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoUpdateInterestsSignal(eventId, ownerId, interests));
  return eventId;
}

unsigned long IcqProtocolPlugin::icqSetOrgBackInfo(const Licq::UserId& ownerId,
    const Licq::UserCategoryMap& organisations, const Licq::UserCategoryMap& background)
{
  if (!isOwnerOnline(ownerId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoUpdateOrgBackSignal(eventId, ownerId, organisations, background));
  return eventId;
}

unsigned long IcqProtocolPlugin::icqSetAbout(const Licq::UserId& ownerId, const std::string& about)
{
  if (!isOwnerOnline(ownerId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoUpdateAboutSignal(eventId, ownerId, about));
  return eventId;
}

unsigned long IcqProtocolPlugin::icqSearchWhitePages(const Licq::UserId& ownerId,
    const std::string& firstName, const std::string& lastName, const std::string& alias,
    const std::string& email, unsigned short minAge, unsigned short maxAge, char gender,
    char language, const std::string& city, const std::string& state, unsigned short country,
    const std::string& coName, const std::string& coDept, const std::string& coPos,
    const std::string& keyword, bool onlineOnly)
{
  if (!isOwnerOnline(ownerId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoSearchWhitePagesSignal(eventId, ownerId, firstName, lastName,
      alias, email, minAge, maxAge, gender, language, city, state, country, coName,
      coDept, coPos, keyword, onlineOnly));
  return eventId;
}

unsigned long IcqProtocolPlugin::icqSearchByUin(const Licq::UserId& userId)
{
  if (!isOwnerOnline(userId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoSearchUinSignal(eventId, userId));
  return eventId;
}

void IcqProtocolPlugin::icqAlertUser(const Licq::UserId& userId)
{
  if (!isOwnerOnline(userId))
    return;

  pushSignal(new ProtoAddedSignal(userId));
}

void IcqProtocolPlugin::icqSetPhoneFollowMeStatus(const Licq::UserId& ownerId, unsigned status)
{
  if (!isOwnerOnline(ownerId))
    return;

  pushSignal(new ProtoSetPhoneFollowMeSignal(ownerId, status));
}

unsigned long IcqProtocolPlugin::setRandomChatGroup(const Licq::UserId& ownerId, unsigned chatGroup)
{
  if (!isOwnerOnline(ownerId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoUpdateRandomChatSignal(eventId, ownerId, chatGroup));
  return eventId;
}

unsigned long IcqProtocolPlugin::randomChatSearch(const Licq::UserId& ownerId, unsigned chatGroup)
{
  if (!isOwnerOnline(ownerId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoSearchRandomSignal(eventId, ownerId, chatGroup));
  return eventId;
}

void IcqProtocolPlugin::updateAllUsersInGroup(const Licq::UserId& ownerId, int groupId)
{
  if (!isOwnerOnline(ownerId))
    return;

  pushSignal(new ProtoUpdateUsersSignal(ownerId, groupId));
}

unsigned long IcqProtocolPlugin::icqSendSms(const Licq::UserId& userId,
    const std::string& number, const std::string& message)
{
  if (!isOwnerOnline(userId))
    return 0;

  unsigned long eventId = Licq::gProtocolManager.getNextEventId();
  pushSignal(new ProtoSendSmsSignal(eventId, userId, number, message));
  return eventId;
}

Licq::IcqFileTransferManager* IcqProtocolPlugin::createFileTransferManager(const Licq::UserId& userId)
{
  return new FileTransferManager(userId);
}

Licq::IcqChatManager* IcqProtocolPlugin::createChatManager(const Licq::UserId& userId)
{
  return new ChatManager(userId);
}
