/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012-2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQICQ_ICQPROTOCOLPLUGIN_H
#define LICQICQ_ICQPROTOCOLPLUGIN_H

#include <licq/plugin/protocolpluginhelper.h>
#include <licq/icq/icq.h>

namespace LicqIcq
{

class Factory;

class IcqProtocolPlugin : public Licq::ProtocolPluginHelper, public Licq::IcqProtocol
{
public:
  IcqProtocolPlugin();

  /// Read and process next event from plugin pipe
  void processPipe();

  // Make read pipe available to monitor thread
  using Licq::ProtocolPluginHelper::getReadPipe;

  // From Licq::PluginInterface
  bool init(int argc, char** argv);
  int run();

  std::string defaultServerHost() const;
  int defaultServerPort() const;

  // From Licq::IcqProtocol
  unsigned long icqSendContactList(const Licq::UserId& userId, const Licq::StringList& users,
      unsigned flags = 0, const Licq::Color* color = NULL);
  unsigned long icqFetchAutoResponse(const Licq::UserId& userId);
  unsigned long icqChatRequest(const Licq::UserId& userId, const std::string& reason,
      unsigned flags = 0, const std::string& chatUsers = "", unsigned short port = 0);
  void icqChatRequestRefuse(const Licq::UserId& userId, const std::string& reason,
      unsigned short sequence, unsigned long msgid1, unsigned long msgid2, bool direct);
  void icqChatRequestAccept(const Licq::UserId& userId, unsigned short port,
      const std::string& clients, unsigned short sequence,
      unsigned long msgid1, unsigned long msgid2, bool direct);
  unsigned long icqRequestPluginInfo(const Licq::UserId& userId, Licq::IcqProtocol::PluginType type,
      bool server = false);
  void icqUpdateInfoTimestamp(const Licq::UserId& ownerId, Licq::IcqProtocol::PluginType type);
  unsigned long icqSetWorkInfo(const Licq::UserId& ownerId, const std::string& city, const std::string& state,
      const std::string& phone, const std::string& fax, const std::string& address,
      const std::string& zip, unsigned short country, const std::string& name,
      const std::string& department, const std::string& position, unsigned short occupation,
      const std::string& homepage);
  unsigned long icqSetEmailInfo(const Licq::UserId& ownerId, const std::string& emailSecondary, const std::string& emailOld);
  unsigned long icqSetMoreInfo(const Licq::UserId& ownerId, unsigned short age, char gender,
      const std::string& homepage, unsigned short birthYear, char birthMonth,
      char birthDay, char language1, char language2, char language3);
  unsigned long icqSetSecurityInfo(const Licq::UserId& ownerId, bool authorize, bool webAware);
  unsigned long icqSetInterestsInfo(const Licq::UserId& ownerId, const Licq::UserCategoryMap& interests);
  unsigned long icqSetOrgBackInfo(const Licq::UserId& ownerId, const Licq::UserCategoryMap& organisations,
      const Licq::UserCategoryMap& background);
  unsigned long icqSetAbout(const Licq::UserId& ownerId, const std::string& about);
  unsigned long icqSearchWhitePages(const Licq::UserId& ownerId, const std::string& firstName, const std::string& lastName,
      const std::string& alias, const std::string& email, unsigned short minAge, unsigned short maxAge,
      char gender, char language, const std::string& city, const std::string& state,
      unsigned short country, const std::string& coName, const std::string& coDept,
      const std::string& coPos, const std::string& keyword, bool onlineOnly);
  unsigned long icqSearchByUin(const Licq::UserId& userId);
  void icqAlertUser(const Licq::UserId& userId);
  void icqSetPhoneFollowMeStatus(const Licq::UserId& ownerId, unsigned status);
  unsigned long setRandomChatGroup(const Licq::UserId& ownerId, unsigned chatGroup);
  unsigned long randomChatSearch(const Licq::UserId& ownerId, unsigned chatGroup);
  void updateAllUsersInGroup(const Licq::UserId& ownerId, int groupId = 0);
  unsigned long icqSendSms(const Licq::UserId& userId,
      const std::string& number, const std::string& message);
  Licq::IcqFileTransferManager* createFileTransferManager(const Licq::UserId& userId);
  Licq::IcqChatManager* createChatManager(const Licq::UserId& userId);

private:
  bool isOwnerOnline(const Licq::UserId& userId);

  using ProtocolPluginHelper::pushSignal;
  void pushSignal(Licq::ProtocolSignal* signal);

  friend class Factory;
  ~IcqProtocolPlugin();
};

extern IcqProtocolPlugin* gIcqProtocolPlugin;

} // namespace LicqIcq

#endif
