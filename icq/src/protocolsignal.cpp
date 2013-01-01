/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#include "protocolsignal.h"


using namespace LicqIcq;

ProtocolSignal::ProtocolSignal(IcqSignalType icqSignal, const Licq::UserId& userId, unsigned long eventId)
  : Licq::ProtocolSignal(SignalProtocolSpecific, userId, eventId),
    myIcqSignal(icqSignal)
{
  // Empty
}

ProtocolSignal::~ProtocolSignal()
{
  // Empty
}


ProtoSendContactsSignal::ProtoSendContactsSignal(unsigned long eventId, const Licq::UserId& userId,
    const Licq::StringList& users, unsigned flags, const Licq::Color* color)
  : ProtocolSignal(SignalIcqSendContacts, userId, eventId),
    myUsers(users),
    myFlags(flags),
    myColor(color != NULL ? new Licq::Color(color) : NULL)
{
  // Empty
}

ProtoSendContactsSignal::~ProtoSendContactsSignal()
{
  delete myColor;
}


ProtoSendSmsSignal::ProtoSendSmsSignal(unsigned long eventId, const Licq::UserId& userId,
    const std::string& number, const std::string& message)
  : ProtocolSignal(SignalIcqSendSms, userId, eventId),
    myNumber(number),
    myMessage(message)
{
  // Empty
}

ProtoSendSmsSignal::~ProtoSendSmsSignal()
{
  // Empty
}


ProtoFetchAutoResponseSignal::ProtoFetchAutoResponseSignal(unsigned long eventId,
    const Licq::UserId& userId)
  : ProtocolSignal(SignalIcqFetchAutoResponse, userId, eventId)
{
  // Empty
}

ProtoFetchAutoResponseSignal::~ProtoFetchAutoResponseSignal()
{
  // Empty
}


ProtoChatRequestSignal::ProtoChatRequestSignal(unsigned long eventId,
    const Licq::UserId& userId, const std::string& reason, unsigned flags,
    const std::string& chatUsers, unsigned short port)
  : ProtocolSignal(SignalIcqChatRequest, userId, eventId),
    myReason(reason),
    myFlags(flags),
    myChatUsers(chatUsers),
    myPort(port)
{
  // Empty
}

ProtoChatRequestSignal::~ProtoChatRequestSignal()
{
  // Empty
}


ProtoChatRefuseSignal::ProtoChatRefuseSignal(const Licq::UserId& userId,
    const std::string& reason, unsigned short sequence, unsigned long msgid1,
    unsigned long msgid2, bool direct)
  : ProtocolSignal(SignalIcqChatRefuse, userId),
    myReason(reason),
    mySequence(sequence),
    myMsgid1(msgid1),
    myMsgid2(msgid2),
    myDirect(direct)
{
  // Empty
}

ProtoChatRefuseSignal::~ProtoChatRefuseSignal()
{
  // Empty
}


ProtoChatAcceptSignal::ProtoChatAcceptSignal(const Licq::UserId& userId,
    unsigned short port, const std::string& clients, unsigned short sequence,
    unsigned long msgid1, unsigned long msgid2, bool direct)
  : ProtocolSignal(SignalIcqChatAccept, userId),
    myPort(port),
    myClients(clients),
    mySequence(sequence),
    myMsgid1(msgid1),
    myMsgid2(msgid2),
    myDirect(direct)
{
  // Empty
}

ProtoChatAcceptSignal::~ProtoChatAcceptSignal()
{
  // Empty
}


ProtoRequestPluginSignal::ProtoRequestPluginSignal(unsigned long eventId,
    const Licq::UserId& userId, Licq::IcqProtocol::PluginType type, bool direct)
  : ProtocolSignal(SignalIcqRequestPlugin, userId, eventId),
    myType(type),
    myDirect(direct)
{
  // Empty
}

ProtoRequestPluginSignal::~ProtoRequestPluginSignal()
{
  // Empty
}


ProtoUpdateWorkSignal::ProtoUpdateWorkSignal(unsigned long eventId,
    const Licq::UserId& ownerId, const std::string& city, const std::string& state,
    const std::string& phone, const std::string& fax, const std::string& address,
    const std::string& zip, unsigned short country, const std::string& name,
    const std::string& department, const std::string& position, unsigned short occupation,
    const std::string& homepage)
  : ProtocolSignal(SignalIcqUpdateWork, ownerId, eventId),
    myCity(city),
    myState(state),
    myPhone(phone),
    myFax(fax),
    myAddress(address),
    myZip(zip),
    myCountry(country),
    myName(name),
    myDepartment(department),
    myPosition(position),
    myOccupation(occupation),
    myHomepage(homepage)
{
  // Empty
}

ProtoUpdateWorkSignal::~ProtoUpdateWorkSignal()
{
  // Empty
}


ProtoUpdateEmailSignal::ProtoUpdateEmailSignal(unsigned long eventId, const Licq::UserId& ownerId,
    const std::string& emailSecondary, const std::string& emailOld)
  : ProtocolSignal(SignalIcqUpdateEmail, ownerId, eventId),
    myEmailSecondary(emailSecondary),
    myEmailOld(emailOld)
{
  // Empty
}

ProtoUpdateEmailSignal::~ProtoUpdateEmailSignal()
{
  // Empty
}


ProtoUpdateMoreSignal::ProtoUpdateMoreSignal(unsigned long eventId,
    const Licq::UserId& ownerId, unsigned short age, char gender,
    const std::string& homepage, unsigned short birthYear, char birthMonth,
    char birthDay, char language1, char language2, char language3)
  : ProtocolSignal(SignalIcqUpdateMore, ownerId, eventId),
    myAge(age),
    myGender(gender),
    myHomepage(homepage),
    myBirthYear(birthYear),
    myBirthMonth(birthMonth),
    myBirthDay(birthDay),
    myLanguage1(language1),
    myLanguage2(language2),
    myLanguage3(language3)
{
  // Empty
}

ProtoUpdateMoreSignal::~ProtoUpdateMoreSignal()
{
  // Empty
}


ProtoUpdateSecuritySignal::ProtoUpdateSecuritySignal(unsigned long eventId,
    const Licq::UserId& ownerId, bool authorize, bool webAware)
  : ProtocolSignal(SignalIcqUpdateSecurity, ownerId, eventId),
    myAuthorize(authorize),
    myWebAware(webAware)
{
  // Empty
}

ProtoUpdateSecuritySignal::~ProtoUpdateSecuritySignal()
{
  // Empty
}


ProtoUpdateInterestsSignal::ProtoUpdateInterestsSignal(unsigned long eventId,
    const Licq::UserId& ownerId, const Licq::UserCategoryMap& interests)
  : ProtocolSignal(SignalIcqUpdateInterests, ownerId, eventId),
    myInterests(interests)
{
  // Empty
}

ProtoUpdateInterestsSignal::~ProtoUpdateInterestsSignal()
{
  // Empty
}


ProtoUpdateOrgBackSignal::ProtoUpdateOrgBackSignal(unsigned long eventId, const Licq::UserId& ownerId,
    const Licq::UserCategoryMap& organisations, const Licq::UserCategoryMap& background)
  : ProtocolSignal(SignalIcqUpdateOrgBack, ownerId, eventId),
    myOrganisations(organisations),
    myBackground(background)
{
  // Empty
}

ProtoUpdateOrgBackSignal::~ProtoUpdateOrgBackSignal()
{
  // Empty
}


ProtoUpdateAboutSignal::ProtoUpdateAboutSignal(unsigned long eventId, const Licq::UserId& ownerId,
    const std::string& about)
  : ProtocolSignal(SignalIcqUpdateAbout, ownerId, eventId),
    myAbout(about)
{
  // Empty
}

ProtoUpdateAboutSignal::~ProtoUpdateAboutSignal()
{
  // Empty
}


ProtoSearchWhitePagesSignal::ProtoSearchWhitePagesSignal(unsigned long eventId,
    const Licq::UserId& ownerId, const std::string& firstName, const std::string& lastName,
    const std::string& alias, const std::string& email, unsigned short minAge, unsigned short maxAge,
    char gender, char language, const std::string& city, const std::string& state,
    unsigned short country, const std::string& coName, const std::string& coDept,
    const std::string& coPos, const std::string& keyword, bool onlineOnly)
  : ProtocolSignal(SignalIcqSearchWhitePages, ownerId, eventId),
    myFirstName(firstName),
    myLastName(lastName),
    myAlias(alias),
    myEmail(email),
    myMinAge(minAge),
    myMaxAge(maxAge),
    myGender(gender),
    myLanguage(language),
    myCity(city),
    myState(state),
    myCountry(country),
    myCoName(coName),
    myCoDept(coDept),
    myCoPos(coPos),
    myKeyword(keyword),
    myOnlineOnly(onlineOnly)
{
  // Empty
}

ProtoSearchWhitePagesSignal::~ProtoSearchWhitePagesSignal()
{
  // Empty
}


ProtoSearchUinSignal::ProtoSearchUinSignal(unsigned long eventId, const Licq::UserId& userId)
  : ProtocolSignal(SignalIcqSearchUin, userId, eventId)
{
  // Empty
}

ProtoSearchUinSignal::~ProtoSearchUinSignal()
{
  // Empty
}


ProtoAddedSignal::ProtoAddedSignal(const Licq::UserId& userId)
  : ProtocolSignal(SignalIcqNotifyAdded, userId)
{
  // Empty
}

ProtoAddedSignal::~ProtoAddedSignal()
{
  // Empty
}


ProtoUpdateTimestampSignal::ProtoUpdateTimestampSignal(const Licq::UserId& ownerId,
    Licq::IcqProtocol::PluginType type)
  : ProtocolSignal(SignalIcqUpdateTimestamp, ownerId),
    myType(type)
{
  // Empty
}

ProtoUpdateTimestampSignal::~ProtoUpdateTimestampSignal()
{
  // Empty
}


ProtoSetPhoneFollowMeSignal::ProtoSetPhoneFollowMeSignal(const Licq::UserId& ownerId, unsigned status)
  : ProtocolSignal(SignalIcqSetPhoneFollowMe, ownerId),
    myStatus(status)
{
  // Empty
}

ProtoSetPhoneFollowMeSignal::~ProtoSetPhoneFollowMeSignal()
{
  // Empty
}


ProtoUpdateRandomChatSignal::ProtoUpdateRandomChatSignal(unsigned long eventId,
    const Licq::UserId& ownerId, unsigned chatGroup)
  : ProtocolSignal(SignalIcqUpdateRandomChat, ownerId, eventId),
    myChatGroup(chatGroup)
{
  // Empty
}

ProtoUpdateRandomChatSignal::~ProtoUpdateRandomChatSignal()
{
  // Empty
}


ProtoSearchRandomSignal::ProtoSearchRandomSignal(unsigned long eventId,
    const Licq::UserId& ownerId, unsigned chatGroup)
  : ProtocolSignal(SignalIcqSearchRandom, ownerId, eventId),
    myChatGroup(chatGroup)
{
  // Empty
}

ProtoSearchRandomSignal::~ProtoSearchRandomSignal()
{
  // Empty
}


ProtoUpdateUsersSignal::ProtoUpdateUsersSignal(const Licq::UserId& ownerId, int groupId)
  : ProtocolSignal(SignalIcqUpdateUsers, ownerId),
    myGroupId(groupId)
{
  // Empty
}

ProtoUpdateUsersSignal::~ProtoUpdateUsersSignal()
{
  // Empty
}
