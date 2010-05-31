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

#include "licq_events.h"

class CICQColor;

namespace Licq
{
typedef std::list<std::string> StringList;
typedef std::map<unsigned int, std::string> UserCategoryMap;
class Proxy;
class User;
class UserId;
}

// To keep old code working
typedef Licq::UserCategoryMap UserCategoryMap;

/* Forward declarations for friend functions */
void *Ping_tep(void *p);
void *UpdateUsers_tep(void *p);



//=====CICQDaemon===============================================================
enum EDaemonStatus {STATUS_ONLINE, STATUS_OFFLINE_MANUAL, STATUS_OFFLINE_FORCED };

class CICQDaemon : private boost::noncopyable
{
public:
  // ICQ functions still public as they don't have any general proto functions
  //   to call them yet and needs to be callable from plugins for now

  virtual unsigned long icqSendContactList(const char *szId, const Licq::StringList& users,
     bool bOnline, unsigned short nLevel, bool bMultipleRecipients = false,
     CICQColor *pColor = NULL) = 0;

  // Auto Response
  virtual unsigned long icqFetchAutoResponse(const char *_szId, unsigned long _nPPID, bool bServer = false) = 0;
  // Chat Request
  virtual unsigned long icqChatRequest(const char* id, const char *szReason,
     unsigned short nLevel, bool bServer) = 0;
  virtual unsigned long icqMultiPartyChatRequest(const char* id,
     const char *szReason, const char *szChatUsers, unsigned short nPort,
     unsigned short nLevel, bool bServer) = 0;
  virtual void icqChatRequestRefuse(const char* id, const char* szReason,
      unsigned short nSequence, const unsigned long nMsgID[], bool bDirect) = 0;
  virtual void icqChatRequestAccept(const char* id, unsigned short nPort,
      const char* szClients, unsigned short nSequence,
      const unsigned long nMsgID[], bool bDirect) = 0;
  virtual void icqChatRequestCancel(const char* id, unsigned short nSequence) = 0;

  // Plugins
  virtual unsigned long icqRequestInfoPluginList(const char *szId,
     bool bServer = false) = 0;
  virtual unsigned long icqRequestPhoneBook(const char *szId, bool bServer = false) = 0;
  virtual unsigned long icqRequestPicture(const Licq::UserId& userId, bool bServer, size_t iconHashSize) = 0;
  virtual unsigned long icqRequestStatusPluginList(const char *szId,
     bool bServer = false) = 0;
  virtual unsigned long icqRequestSharedFiles(const char *szId, bool bServer = false) = 0;
  virtual unsigned long icqRequestPhoneFollowMe(const char *szId,
     bool bServer = false) = 0;
  virtual unsigned long icqRequestICQphone(const char *szId, bool bServer = false) = 0;

  // Server functions
  virtual void icqRegister(const char *_szPasswd) = 0;
  virtual void icqVerifyRegistration() = 0;
  virtual void icqVerify(const char *) = 0;
  virtual unsigned long icqSetWorkInfo(const char *_szCity, const char *_szState,
                           const char *_szPhone,
                           const char *_szFax, const char *_szAddress,
                           const char *_szZip, unsigned short _nCompanyCountry,
                           const char *_szName, const char *_szDepartment,
                           const char *_szPosition, unsigned short _nCompanyOccupation,
                           const char *_szHomepage) = 0;
  virtual unsigned long icqSetGeneralInfo(const char *szAlias, const char *szFirstName,
                              const char *szLastName, const char *szEmailPrimary,
                              const char *szCity,
                              const char *szState, const char *szPhoneNumber,
                              const char *szFaxNumber, const char *szAddress,
                              const char *szCellularNumber, const char *szZipCode,
                              unsigned short nCountryCode, bool bHideEmail) = 0;
  virtual unsigned long icqSetEmailInfo(const char *szEmailSecondary, const char *szEmailOld) = 0;
  virtual unsigned long icqSetMoreInfo(unsigned short nAge,
                           char nGender, const char *szHomepage,
                           unsigned short nBirthYear, char nBirthMonth,
                           char nBirthDay, char nLanguage1,
                           char nLanguage2, char nLanguage3) = 0;
  virtual unsigned long icqSetSecurityInfo(bool bAuthorize, bool bHideIp, bool bWebAware) = 0;
  virtual unsigned long icqSetInterestsInfo(const Licq::UserCategoryMap& interests) = 0;
  virtual unsigned long icqSetOrgBackInfo(const Licq::UserCategoryMap& orgs,
      const Licq::UserCategoryMap& background) = 0;
  virtual unsigned long icqSetAbout(const char *szAbout) = 0;
  virtual unsigned long icqSetPassword(const char *szPassword) = 0;
  virtual unsigned long icqSetRandomChatGroup(unsigned long nGroup) = 0;
  virtual unsigned long icqRandomChatSearch(unsigned long nGroup) = 0;
  virtual unsigned long icqSearchWhitePages(const char *szFirstName,
                            const char *szLastName, const char *szAlias,
                            const char *szEmail, unsigned short nMinAge,
                            unsigned short nMaxAge, char nGender,
                            char nLanguage, const char *szCity,
                            const char *szState, unsigned short nCountryCode,
                            const char *szCoName, const char *szCoDept,
                            const char *szCoPos, const char *szKeyword,
                            bool bOnlineOnly) = 0;
  virtual unsigned long icqSearchByUin(unsigned long) = 0;
  virtual unsigned long icqAuthorizeGrant(const Licq::UserId& userId, const std::string& message) = 0;
  virtual unsigned long icqAuthorizeRefuse(const Licq::UserId& userId, const std::string& message) = 0;
  virtual void icqRequestAuth(const char* id, const char *_szMessage) = 0;
  virtual void icqAlertUser(const Licq::UserId& userId) = 0;
  virtual void icqUpdatePhoneBookTimestamp() = 0;
  virtual void icqUpdatePictureTimestamp() = 0;
  virtual void icqSetPhoneFollowMeStatus(unsigned long nNewStatus) = 0;
  virtual void icqUpdateContactList() = 0;
  virtual void icqCheckInvisible(const char *_szId) = 0;

  virtual void CheckExport() = 0;

  EDaemonStatus Status() const                  { return m_eStatus; }

  virtual void UpdateAllUsers() = 0;
  virtual void updateAllUsersInGroup(int groupId) = 0;

  // SMS
  virtual unsigned long icqSendSms(const char* id, unsigned long ppid,
      const char* number, const char* message) = 0;

  // ICQ Server options
  const std::string& icqServer() const          { return myIcqServer; }
  void setIcqServer(const std::string& s)       { myIcqServer = s;  }
  unsigned icqServerPort() const                { return myIcqServerPort; }
  void setIcqServerPort(unsigned p)             { myIcqServerPort = p; }

  // Firewall options
  void SetDirectMode();

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
  char *parseRTF(const char *);

protected:
  virtual ~CICQDaemon() { /* Empty */ }

  EDaemonStatus m_eStatus;

  bool m_bAutoUpdateInfo, m_bAutoUpdateInfoPlugins, m_bAutoUpdateStatusPlugins;

  // ICQ Server
  std::string myIcqServer;
  unsigned myIcqServerPort;

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
