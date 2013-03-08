/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_ICQ_H
#define LICQ_ICQ_H

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include <map>
#include <string>

namespace Licq
{
typedef std::list<std::string> StringList;
typedef std::map<unsigned int, std::string> UserCategoryMap;
class Color;
class IcqChatManager;
class IcqFileTransferManager;
class UserId;


class IcqProtocol : private boost::noncopyable
{
public:
  typedef boost::shared_ptr<IcqProtocol> Ptr;

  static const int MaxMessageSize = 6800; // Maybe a little bigger?
  static const int MaxOfflineMessageSize = 450;

  // ICQ functions still public as they don't have any general proto functions
  //   to call them yet and needs to be callable from plugins for now

  virtual unsigned long icqSendContactList(const Licq::UserId& userId, const Licq::StringList& users,
      unsigned flags = 0, const Licq::Color* pColor = NULL) = 0;

  // Auto Response
  virtual unsigned long icqFetchAutoResponse(const Licq::UserId& userId) = 0;
  // Chat Request
  virtual unsigned long icqChatRequest(const Licq::UserId& userId, const std::string& reason,
      unsigned flags = 0, const std::string& chatUsers = "", unsigned short port = 0) = 0;
  virtual void icqChatRequestRefuse(const Licq::UserId& userId, const std::string& reason,
      unsigned short nSequence, unsigned long msgid1, unsigned long msgid2, bool bDirect) = 0;
  virtual void icqChatRequestAccept(const Licq::UserId& userId, unsigned short nPort,
      const std::string& clients, unsigned short nSequence,
      unsigned long msgid1, unsigned long msgid2, bool bDirect) = 0;

  // Plugins
  enum PluginType
  {
    PluginInfoList = 1,
    PluginPhoneBook = 2,
    PluginPicture = 3,
    PluginStatusList = 4,
    PluginSharedFiles = 5,
    PluginPhoneFollowMe = 6,
    PluginIcqPhone = 7,
  };
  virtual unsigned long icqRequestPluginInfo(const Licq::UserId& userId, PluginType type,
      bool bServer = false) = 0;
  virtual void icqUpdateInfoTimestamp(const Licq::UserId& ownerId, PluginType type) = 0;

  // Server functions
  virtual unsigned long icqSetWorkInfo(const Licq::UserId& ownerId,
      const std::string& city, const std::string& state,
      const std::string& phone, const std::string& fax, const std::string& address,
      const std::string& zip, unsigned short companyCountry, const std::string& name,
      const std::string& department, const std::string& position, unsigned short companyOccupation,
      const std::string& homepage) = 0;
  virtual unsigned long icqSetEmailInfo(const Licq::UserId& ownerId,
      const std::string& emailSecondary, const std::string& emailOld) = 0;
  virtual unsigned long icqSetMoreInfo(const Licq::UserId& ownerId, unsigned short age, char gender,
      const std::string& homepage, unsigned short birthYear, char birthMonth,
      char birthDay, char language1, char language2, char language3) = 0;
  virtual unsigned long icqSetSecurityInfo(const Licq::UserId& ownerId, bool bAuthorize, bool bWebAware) = 0;
  virtual unsigned long icqSetInterestsInfo(const Licq::UserId& ownerId, const Licq::UserCategoryMap& interests) = 0;
  virtual unsigned long icqSetOrgBackInfo(const Licq::UserId& ownerId, const Licq::UserCategoryMap& orgs,
      const Licq::UserCategoryMap& background) = 0;
  virtual unsigned long icqSetAbout(const Licq::UserId& ownerId, const std::string& about) = 0;
  virtual unsigned long icqSearchWhitePages(const Licq::UserId& ownerId,
      const std::string& firstName, const std::string& lastName,
      const std::string& alias, const std::string& email, unsigned short minAge, unsigned short maxAge,
      char gender, char language, const std::string& city, const std::string& state,
      unsigned short countryCode, const std::string& coName, const std::string& coDept,
      const std::string& coPos, const std::string& keyword, bool onlineOnly) = 0;
  virtual unsigned long icqSearchByUin(const Licq::UserId& userId) = 0;
  virtual void icqAlertUser(const Licq::UserId& userId) = 0;
  virtual void icqSetPhoneFollowMeStatus(const Licq::UserId& ownerId, unsigned newStatus) = 0;

  virtual unsigned long setRandomChatGroup(const Licq::UserId& ownerId, unsigned chatGroup) = 0;
  virtual unsigned long randomChatSearch(const Licq::UserId& ownerId, unsigned chatGroup) = 0;

  virtual void updateAllUsersInGroup(const Licq::UserId& ownerId, int groupId = 0) = 0;

  // SMS
  virtual unsigned long icqSendSms(const Licq::UserId& userId,
      const std::string& number, const std::string& message) = 0;

  virtual IcqFileTransferManager* createFileTransferManager(const UserId& userId) = 0;
  virtual IcqChatManager* createChatManager(const UserId& userId) = 0;

protected:
  virtual ~IcqProtocol() { /* Empty */ }
};

} // namespace Licq

#endif
