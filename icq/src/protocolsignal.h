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

#ifndef LICQICQ_PROTOCOLSIGNAL_H
#define LICQICQ_PROTOCOLSIGNAL_H

#include <licq/protocolsignal.h>
#include <licq/icq/icq.h>

namespace LicqIcq
{

class ProtocolSignal : public Licq::ProtocolSignal
{
public:
  enum IcqSignalType
  {
    SignalIcqSendContacts       = 1,    // Send contacts to user
    SignalIcqSendSms            = 2,    // Send SMS message
    SignalIcqFetchAutoResponse  = 3,    // Fetch away message from user
    SignalIcqChatRequest        = 4,    // Send chat request ot user
    SignalIcqChatRefuse         = 5,    // Refuse a chat request
    SignalIcqChatAccept         = 6,    // Accept a chat request
    SignalIcqRequestPlugin      = 7,    // Request plugin info
    SignalIcqUpdateWork         = 8,    // Update more info
    SignalIcqUpdateEmail        = 9,    // Update email info
    SignalIcqUpdateMore         = 10,   // Update more info
    SignalIcqUpdateSecurity     = 11,   // Update security info
    SignalIcqUpdateInterests    = 12,   // Update interests info
    SignalIcqUpdateOrgBack      = 13,   // Update organisation and background info
    SignalIcqUpdateAbout        = 14,   // Update about info
    SignalIcqSearchWhitePages   = 15,   // Search white pages
    SignalIcqSearchUin          = 16,   // Search using UIN
    SignalIcqNotifyAdded        = 17,   // Alert user
    SignalIcqUpdateTimestamp    = 18,   // Update plugin timestamp
    SignalIcqSetPhoneFollowMe   = 19,   // Set phone follow me status
    SignalIcqUpdateRandomChat   = 20,   // Set random chat group
    SignalIcqSearchRandom       = 21,   // Random chat search
    SignalIcqUpdateUsers        = 22,   // Update users
  };

  ProtocolSignal(IcqSignalType icqSignal, const Licq::UserId& userId, unsigned long eventId = 0);

  virtual ~ProtocolSignal();

  IcqSignalType icqSignal() const
  { return myIcqSignal; }

private:
  IcqSignalType myIcqSignal;
};


class ProtoSendContactsSignal : public ProtocolSignal
{
public:
  ProtoSendContactsSignal(unsigned long eventId, const Licq::UserId& userId,
      const Licq::StringList& users, unsigned flags = 0, const Licq::Color* color = NULL);
  ~ProtoSendContactsSignal();

  const Licq::StringList& users() const { return myUsers; }
  unsigned flags() const { return myFlags; }
  const Licq::Color* color() const { return myColor; }

private:
  Licq::StringList myUsers;
  unsigned myFlags;
  Licq::Color* myColor;
};


class ProtoSendSmsSignal : public ProtocolSignal
{
public:
  ProtoSendSmsSignal(unsigned long eventId, const Licq::UserId& userId,
      const std::string& number, const std::string& message);
  ~ProtoSendSmsSignal();

  const std::string& number() const { return myNumber; }
  const std::string& message() const { return myMessage; }

private:
  std::string myNumber;
  std::string myMessage;
};


class ProtoFetchAutoResponseSignal : public ProtocolSignal
{
public:
  ProtoFetchAutoResponseSignal(unsigned long eventId, const Licq::UserId& userId);
  ~ProtoFetchAutoResponseSignal();
};


class ProtoChatRequestSignal : public ProtocolSignal
{
public:
  ProtoChatRequestSignal(unsigned long eventId, const Licq::UserId& userId,
      const std::string& reason, unsigned flags = 0,
      const std::string& chatUsers = "", unsigned short port = 0);
  ~ProtoChatRequestSignal();

  const std::string& reason() const { return myReason; }
  unsigned flags() const { return myFlags; }
  const std::string& chatUsers() const { return myChatUsers; }
  unsigned short port() const { return myPort; }

private:
  std::string myReason;
  unsigned myFlags;
  std::string myChatUsers;
  unsigned short myPort;
};


class ProtoChatRefuseSignal : public ProtocolSignal
{
public:
  ProtoChatRefuseSignal(const Licq::UserId& userId, const std::string& reason,
      unsigned short sequence, unsigned long msgid1, unsigned long msgid2, bool direct);
  ~ProtoChatRefuseSignal();

  const std::string& reason() const { return myReason; }
  unsigned short sequence() const { return mySequence; }
  unsigned long msgid1() const { return myMsgid1; }
  unsigned long msgid2() const { return myMsgid2; }
  bool direct() const { return myDirect; }

private:
  std::string myReason;
  unsigned short mySequence;
  unsigned long myMsgid1, myMsgid2;
  bool myDirect;
};


class ProtoChatAcceptSignal : public ProtocolSignal
{
public:
  ProtoChatAcceptSignal(const Licq::UserId& userId, unsigned short port,
      const std::string& clients, unsigned short sequence,
      unsigned long msgid1, unsigned long msgid2, bool direct);
  ~ProtoChatAcceptSignal();

  unsigned short port() const { return myPort; }
  const std::string& clients() const { return myClients; }
  unsigned short sequence() const { return mySequence; }
  unsigned long msgid1() const { return myMsgid1; }
  unsigned long msgid2() const { return myMsgid2; }
  bool direct() const { return myDirect; }

private:
  unsigned short myPort;
  std::string myClients;
  unsigned short mySequence;
  unsigned long myMsgid1, myMsgid2;
  bool myDirect;
};


class ProtoRequestPluginSignal : public ProtocolSignal
{
public:
  ProtoRequestPluginSignal(unsigned long eventId, const Licq::UserId& userId,
      Licq::IcqProtocol::PluginType type, bool direct = false);
  ~ProtoRequestPluginSignal();

  Licq::IcqProtocol::PluginType type() const { return myType; }
  bool direct() const { return myDirect; }

private:
  Licq::IcqProtocol::PluginType myType;
  bool myDirect;
};


class ProtoUpdateWorkSignal : public ProtocolSignal
{
public:
  ProtoUpdateWorkSignal(unsigned long eventId, const Licq::UserId& ownerId,
      const std::string& city, const std::string& state,
      const std::string& phone, const std::string& fax, const std::string& address,
      const std::string& zip, unsigned short country, const std::string& name,
      const std::string& department, const std::string& position, unsigned short occupation,
      const std::string& homepage);
  ~ProtoUpdateWorkSignal();

  const std::string& city() const { return myCity; }
  const std::string& state() const { return myState; }
  const std::string& phone() const { return myPhone; }
  const std::string& fax() const { return myFax; }
  const std::string& address() const { return myAddress; }
  const std::string& zip() const { return myZip; }
  unsigned short country() const { return myCountry; }
  const std::string& name() const { return myName; }
  const std::string& department() const { return myDepartment; }
  const std::string& position() const { return myPosition; }
  unsigned short occupation() const { return myOccupation; }
  const std::string& homepage() const { return myHomepage; }

private:
  std::string myCity;
  std::string myState;
  std::string myPhone;
  std::string myFax;
  std::string myAddress;
  std::string myZip;
  unsigned short myCountry;
  std::string myName;
  std::string myDepartment;
  std::string myPosition;
  unsigned short myOccupation;
  std::string myHomepage;
};


class ProtoUpdateEmailSignal : public ProtocolSignal
{
public:
  ProtoUpdateEmailSignal(unsigned long eventId, const Licq::UserId& ownerId,
      const std::string& emailSecondary, const std::string& emailOld);
  ~ProtoUpdateEmailSignal();

  const std::string& emailSecondary() const { return myEmailSecondary; }
  const std::string& emailOld() const { return myEmailOld; }

private:
  std::string myEmailSecondary;
  std::string myEmailOld;
};


class ProtoUpdateMoreSignal : public ProtocolSignal
{
public:
  ProtoUpdateMoreSignal(unsigned long eventId, const Licq::UserId& ownerId,
      unsigned short age, char gender, const std::string& homepage,
      unsigned short birthYear, char birthMonth, char birthDay,
      char language1, char language2, char language3);
  ~ProtoUpdateMoreSignal();

  unsigned short age() const { return myAge; }
  char gender() const { return myGender; }
  const std::string& homepage() const { return myHomepage; }
  unsigned short birthYear() const { return myBirthYear; }
  char birthMonth() const { return myBirthMonth; }
  char birthDay() const { return myBirthDay; }
  char language1() const { return myLanguage1; }
  char language2() const { return myLanguage2; }
  char language3() const { return myLanguage3; }

private:
  unsigned short myAge;
  char myGender;
  std::string myHomepage;
  unsigned short myBirthYear;
  char myBirthMonth;
  char myBirthDay;
  char myLanguage1;
  char myLanguage2;
  char myLanguage3;
};


class ProtoUpdateSecuritySignal : public ProtocolSignal
{
public:
  ProtoUpdateSecuritySignal(unsigned long eventId, const Licq::UserId& ownerId,
      bool authorize, bool webAware);
  ~ProtoUpdateSecuritySignal();

  bool authorize() const { return myAuthorize; }
  bool webAware() const { return myWebAware; }

private:
  bool myAuthorize;
  bool myWebAware;
};


class ProtoUpdateInterestsSignal : public ProtocolSignal
{
public:
  ProtoUpdateInterestsSignal(unsigned long eventId, const Licq::UserId& ownerId,
      const Licq::UserCategoryMap& interests);
  ~ProtoUpdateInterestsSignal();

  const Licq::UserCategoryMap& interests() const { return myInterests; }

private:
  Licq::UserCategoryMap myInterests;
};


class ProtoUpdateOrgBackSignal : public ProtocolSignal
{
public:
  ProtoUpdateOrgBackSignal(unsigned long eventId, const Licq::UserId& ownerId,
      const Licq::UserCategoryMap& organisations, const Licq::UserCategoryMap& background);
  ~ProtoUpdateOrgBackSignal();

  const Licq::UserCategoryMap& organisations() const { return myOrganisations; }
  const Licq::UserCategoryMap& background() const { return myBackground; }

private:
  Licq::UserCategoryMap myOrganisations;
  Licq::UserCategoryMap myBackground;
};


class ProtoUpdateAboutSignal : public ProtocolSignal
{
public:
  ProtoUpdateAboutSignal(unsigned long eventId, const Licq::UserId& ownerId,
      const std::string& about);
  ~ProtoUpdateAboutSignal();

  const std::string& about() const { return myAbout; }

private:
  std::string myAbout;
};


class ProtoSearchWhitePagesSignal : public ProtocolSignal
{
public:
  ProtoSearchWhitePagesSignal(unsigned long eventId, const Licq::UserId& ownerId,
      const std::string& firstName, const std::string& lastName,
      const std::string& alias, const std::string& email, unsigned short minAge, unsigned short maxAge,
      char gender, char language, const std::string& city, const std::string& state,
      unsigned short country, const std::string& coName, const std::string& coDept,
      const std::string& coPos, const std::string& keyword, bool onlineOnly);
  ~ProtoSearchWhitePagesSignal();

  const std::string& firstName() const { return myFirstName; }
  const std::string& lastName() const { return myLastName; }
  const std::string& alias() const { return myAlias; }
  const std::string& email() const { return myEmail; }
  unsigned short minAge() const { return myMinAge; }
  unsigned short maxAge() const { return myMaxAge; }
  char gender() const { return myGender; }
  char language() const { return myLanguage; }
  const std::string& city() const { return myCity; }
  const std::string& state() const { return myState; }
  unsigned short country() const { return myCountry; }
  const std::string& coName() const { return myCoName; }
  const std::string& coDept() const { return myCoDept; }
  const std::string& coPos() const { return myCoPos; }
  const std::string& keyword() const { return myKeyword; }
  bool onlineOnly() const { return myOnlineOnly; }

private:
  std::string myFirstName;
  std::string myLastName;
  std::string myAlias;
  std::string myEmail;
  unsigned short myMinAge;
  unsigned short myMaxAge;
  char myGender;
  char myLanguage;
  std::string myCity;
  std::string myState;
  unsigned short myCountry;
  std::string myCoName;
  std::string myCoDept;
  std::string myCoPos;
  std::string myKeyword;
  bool myOnlineOnly;
};


class ProtoSearchUinSignal : public ProtocolSignal
{
public:
  ProtoSearchUinSignal(unsigned long eventId, const Licq::UserId& userId);
  ~ProtoSearchUinSignal();
};


class ProtoAddedSignal : public ProtocolSignal
{
public:
  ProtoAddedSignal(const Licq::UserId& userId);
  ~ProtoAddedSignal();
};


class ProtoUpdateTimestampSignal : public ProtocolSignal
{
public:
  ProtoUpdateTimestampSignal(const Licq::UserId& ownerId, Licq::IcqProtocol::PluginType type);
  ~ProtoUpdateTimestampSignal();

  Licq::IcqProtocol::PluginType type() const { return myType; }

private:
  Licq::IcqProtocol::PluginType myType;
};


class ProtoSetPhoneFollowMeSignal : public ProtocolSignal
{
public:
  ProtoSetPhoneFollowMeSignal(const Licq::UserId& ownerId, unsigned status);
  ~ProtoSetPhoneFollowMeSignal();

  unsigned status() const { return myStatus; }

private:
  unsigned myStatus;
};


class ProtoUpdateRandomChatSignal : public ProtocolSignal
{
public:
  ProtoUpdateRandomChatSignal(unsigned long eventId, const Licq::UserId& ownerId, unsigned chatGroup);
  ~ProtoUpdateRandomChatSignal();

  unsigned chatGroup() const { return myChatGroup; }

private:
  unsigned myChatGroup;
};


class ProtoSearchRandomSignal : public ProtocolSignal
{
public:
  ProtoSearchRandomSignal(unsigned long eventId, const Licq::UserId& ownerId, unsigned chatGroup);
  ~ProtoSearchRandomSignal();

  unsigned chatGroup() const { return myChatGroup; }

private:
  unsigned myChatGroup;
};


class ProtoUpdateUsersSignal : public ProtocolSignal
{
public:
  ProtoUpdateUsersSignal(const Licq::UserId& ownerId, int groupId = 0);
  ~ProtoUpdateUsersSignal();

  int groupId() const { return myGroupId; }

private:
  int myGroupId;
};

} // namespace LicqIcq

#endif
