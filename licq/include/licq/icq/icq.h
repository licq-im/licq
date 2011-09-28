/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2011 Licq developers
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

/*
ICQ.H
header file containing all the main procedures to interface with the ICQ server at mirabilis
*/

#ifndef LICQ_ICQ_H
#define LICQ_ICQ_H

#include <boost/noncopyable.hpp>
#include <list>
#include <map>
#include <string>

namespace Licq
{
typedef std::list<std::string> StringList;
typedef std::map<unsigned int, std::string> UserCategoryMap;
class Color;
class Proxy;
class UserId;
}

// To keep old code working
typedef Licq::UserCategoryMap UserCategoryMap;

/* Forward declarations for friend functions */
void *Ping_tep(void *p);
void *UpdateUsers_tep(void *p);



//=====CICQDaemon===============================================================

class CICQDaemon : private boost::noncopyable
{
public:
  static const int MaxMessageSize = 6800; // Maybe a little bigger?
  static const int MaxOfflineMessageSize = 450;

  enum IcqPluginStatus
  {
    IcqPluginInactive = 0,
    IcqPluginActive = 1,
    IcqPluginBusy = 2,
  };

  // ICQ functions still public as they don't have any general proto functions
  //   to call them yet and needs to be callable from plugins for now

  virtual unsigned long icqSendContactList(const Licq::UserId& userId, const Licq::StringList& users,
      unsigned flags = 0, const Licq::Color* pColor = NULL) = 0;

  // Auto Response
  virtual unsigned long icqFetchAutoResponse(const Licq::UserId& userId, bool bServer = false) = 0;
  // Chat Request
  virtual unsigned long icqChatRequest(const Licq::UserId& userId, const std::string& reason,
      unsigned flags = 0) = 0;
  virtual unsigned long icqMultiPartyChatRequest(const Licq::UserId& userId,
     const std::string& reason, const std::string& chatUsers, unsigned short nPort,
      unsigned flags = 0) = 0;
  virtual void icqChatRequestRefuse(const Licq::UserId& userId, const std::string& reason,
      unsigned short nSequence, const unsigned long nMsgID[], bool bDirect) = 0;
  virtual void icqChatRequestAccept(const Licq::UserId& userId, unsigned short nPort,
      const std::string& clients, unsigned short nSequence,
      const unsigned long nMsgID[], bool bDirect) = 0;
  virtual void icqChatRequestCancel(const Licq::UserId& userId, unsigned short nSequence) = 0;

  // Plugins
  virtual unsigned long icqRequestInfoPluginList(const Licq::UserId& userId,
     bool bServer = false) = 0;
  virtual unsigned long icqRequestPhoneBook(const Licq::UserId& userId, bool bServer = false) = 0;
  virtual unsigned long icqRequestPicture(const Licq::UserId& userId, bool bServer, size_t iconHashSize) = 0;
  virtual unsigned long icqRequestStatusPluginList(const Licq::UserId& userId,
     bool bServer = false) = 0;
  virtual unsigned long icqRequestSharedFiles(const Licq::UserId& userId, bool bServer = false) = 0;
  virtual unsigned long icqRequestPhoneFollowMe(const Licq::UserId& userId,
     bool bServer = false) = 0;
  virtual unsigned long icqRequestICQphone(const Licq::UserId& userId, bool bServer = false) = 0;

  // Server functions
  virtual void icqRegister(const std::string& passwd) = 0;
  virtual void icqVerifyRegistration() = 0;
  virtual void icqVerify(const std::string& verification) = 0;
  virtual unsigned long icqSetWorkInfo(const std::string& city, const std::string& state,
      const std::string& phone, const std::string& fax, const std::string& address,
      const std::string& zip, unsigned short companyCountry, const std::string& name,
      const std::string& department, const std::string& position, unsigned short companyOccupation,
      const std::string& homepage) = 0;
  virtual unsigned long icqSetGeneralInfo(const std::string& alias, const std::string& firstName,
      const std::string& lastName, const std::string& emailPrimary, const std::string& city,
      const std::string& state, const std::string& phoneNumber, const std::string& faxNumber,
      const std::string& address, const std::string& cellularNumber, const std::string& zipCode,
      unsigned short countryCode, bool hdeEmail) = 0;
  virtual unsigned long icqSetEmailInfo(const std::string& emailSecondary, const std::string& emailOld) = 0;
  virtual unsigned long icqSetMoreInfo(unsigned short age, char gender,
      const std::string& homepage, unsigned short birthYear, char birthMonth,
      char birthDay, char language1, char language2, char language3) = 0;
  virtual unsigned long icqSetSecurityInfo(bool bAuthorize, bool bHideIp, bool bWebAware) = 0;
  virtual unsigned long icqSetInterestsInfo(const Licq::UserCategoryMap& interests) = 0;
  virtual unsigned long icqSetOrgBackInfo(const Licq::UserCategoryMap& orgs,
      const Licq::UserCategoryMap& background) = 0;
  virtual unsigned long icqSetAbout(const std::string& about) = 0;
  virtual unsigned long icqSearchWhitePages(const std::string& firstName, const std::string& lastName,
      const std::string& alias, const std::string& email, unsigned short minAge, unsigned short maxAge,
      char gender, char language, const std::string& city, const std::string& state,
      unsigned short countryCode, const std::string& coName, const std::string& coDept,
      const std::string& coPos, const std::string& keyword, bool onlineOnly) = 0;
  virtual unsigned long icqSearchByUin(unsigned long) = 0;
  virtual unsigned long icqAuthorizeGrant(const Licq::UserId& userId, const std::string& message) = 0;
  virtual unsigned long icqAuthorizeRefuse(const Licq::UserId& userId, const std::string& message) = 0;
  virtual void icqRequestAuth(const Licq::UserId& userId, const std::string& message) = 0;
  virtual void icqAlertUser(const Licq::UserId& userId) = 0;
  virtual void icqUpdatePhoneBookTimestamp() = 0;
  virtual void icqUpdatePictureTimestamp() = 0;
  virtual void icqSetPhoneFollowMeStatus(unsigned newStatus) = 0;
  virtual void icqUpdateContactList() = 0;
  virtual void icqCheckInvisible(const Licq::UserId& userId) = 0;

  enum RandomChatGroups
  {
    RandomChatGroupNone         = 0,
    RandomChatGroupGeneral      = 1,
    RandomChatGroupRomance      = 2,
    RandomChatGroupGames        = 3,
    RandomChatGroupStudents     = 4,
    RandomChatGroup20Some       = 6,
    RandomChatGroup30Some       = 7,
    RandomChatGroup40Some       = 8,
    RandomChatGroup50Plus       = 9,
    RandomChatGroupSeekF        = 10,
    RandomChatGroupSeekM        = 11,
  };
  virtual unsigned long setRandomChatGroup(unsigned chatGroup) = 0;
  virtual unsigned long randomChatSearch(unsigned chatGroup) = 0;

  virtual void CheckExport() = 0;

  virtual void UpdateAllUsers() = 0;
  virtual void updateAllUsersInGroup(int groupId) = 0;

  // SMS
  virtual unsigned long icqSendSms(const Licq::UserId& userId,
      const std::string& number, const std::string& message) = 0;

  // Firewall options
  virtual void setDirectMode() = 0;

  // Proxy options
  void InitProxy();
  Licq::Proxy* GetProxy() {  return m_xProxy;  }

  bool AutoUpdateInfo() const                   { return m_bAutoUpdateInfo; }
  bool AutoUpdateInfoPlugins() const            { return m_bAutoUpdateInfoPlugins; }
  bool AutoUpdateStatusPlugins() const          { return m_bAutoUpdateStatusPlugins; }

  void SetAutoUpdateInfo(bool b)          { m_bAutoUpdateInfo = b; }
  void SetAutoUpdateInfoPlugins(bool b)   { m_bAutoUpdateInfoPlugins = b; }
  void SetAutoUpdateStatusPlugins(bool b) { m_bAutoUpdateStatusPlugins = b; }

  // ICQ options
  bool UseServerContactList() const             { return m_bUseSS; }
  bool UseServerSideBuddyIcons() const          { return m_bUseBART; }

  void SetUseServerContactList(bool b)    { m_bUseSS = b; }
  virtual void SetUseServerSideBuddyIcons(bool b) = 0;

  // Misc functions
  bool ReconnectAfterUinClash() const           { return m_bReconnectAfterUinClash; }
  void setReconnectAfterUinClash(bool b)     { m_bReconnectAfterUinClash = b; }
  static std::string getXmlTag(const std::string& xmlSource, const std::string& tagName);

protected:
  virtual ~CICQDaemon() { /* Empty */ }

  bool m_bAutoUpdateInfo, m_bAutoUpdateInfoPlugins, m_bAutoUpdateStatusPlugins;

  // Proxy
  Licq::Proxy* m_xProxy;

  // Misc
  bool m_bUseSS; // server side list
  bool m_bUseBART; // server side buddy icons
  bool m_bReconnectAfterUinClash; // reconnect after uin has been used from another location?

  // Declare all our thread functions as friends
  friend void *Ping_tep(void *p);
  friend void *UpdateUsers_tep(void *p);
};

// Global pointer
extern CICQDaemon *gLicqDaemon;

// Helper functions for the daemon
bool ParseFE(char *szBuffer, char ***szSubStr, int nMaxSubStr);
unsigned short VersionToUse(unsigned short);

#endif
