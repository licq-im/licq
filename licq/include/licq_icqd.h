/*
ICQ.H
header file containing all the main procedures to interface with the ICQ server at mirabilis
*/

#ifndef ICQD_H
#define ICQD_H

#include <vector>
#include <list>
#include <map>
#include <string>

#include <boost/shared_array.hpp>

#include "licq_events.h"
#include "licq_filetransfer.h"
#include "licq_onevent.h"
#include "licq_plugind.h"
#include "licq_protoplugind.h"

class CProtoPlugin;
class CPlugin;
class CPacket;
class CPacketTcp;
class CLicq;
class CUserManager;
class ICQUser;
class CICQEventTag;
class CICQColor;
class TCPSocket;
class SrvSocket;
class INetSocket;
class ProxyServer;
class COscarService;
class CReverseConnectToUserData;
class CMSN;

// Define for marking functions as deprecated
#ifndef LICQ_DEPRECATED
# if defined(__GNUC__) && !defined(__INTEL_COMPILER) && (__GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2))
#  define LICQ_DEPRECATED __attribute__ ((__deprecated__))
# elif defined(_MSC_VER) && (_MSC_VER >= 1300)
#  define LICQ_DEPRECATED __declspec(deprecated)
# else
#  define LICQ_DEPRECATED
# endif
#endif


#define FOR_EACH_PROTO_PLUGIN_START(d)                             \
    {                                                              \
      ProtoPluginsList _pl_;                                       \
      ProtoPluginsListIter _ppit;                                  \
      d->ProtoPluginList(_pl_);                                    \
      for (_ppit = _pl_.begin(); _ppit != _pl_.end(); _ppit++)     \
      { 

#define FOR_EACH_PROTO_PLUGIN_END                                  \
      }                                                            \
    }

typedef std::list<std::string> StringList;

/* Forward declarations for friend functions */
void *Ping_tep(void *p);
void *UpdateUsers_tep(void *p);
void *MonitorSockets_tep(void *p);
void *ReverseConnectToUser_tep(void *p);
void *ProcessRunningEvent_Client_tep(void *p);
void *ProcessRunningEvent_Server_tep(void *p);
void *Shutdown_tep(void *p);
void *ConnectToServer_tep(void *s);

const unsigned short IGNORE_MASSMSG    = 1;
const unsigned short IGNORE_NEWUSERS   = 2;
const unsigned short IGNORE_EMAILPAGER = 4;
const unsigned short IGNORE_WEBPANEL   = 8;

//-----Stats-----------------------------------------------------------------
//! Keeps information about statistics of the daemon
class CDaemonStats
{
public:
  ~CDaemonStats();

  // Accessors
  //! Total number of events.
  unsigned long Total() const   { return m_nTotal; }
  //! Total number of events for the current day only.
  unsigned long Today() const   { return m_nTotal - m_nOriginal; }
  //! Name of the kind of statistic.
  const char* Name() const      { return m_szName; }

protected:
  CDaemonStats();
  CDaemonStats(const char *, const char *);

  bool Dirty() const            { return m_nLastSaved != m_nTotal; }
  void ClearDirty() { m_nLastSaved = m_nTotal; }

  void Init();
  void Reset();
  void Inc() { m_nTotal++; }

  unsigned long m_nTotal;

  unsigned long m_nOriginal;
  unsigned long m_nLastSaved;
  char m_szTag[16];
  char m_szName[32];

friend class CICQDaemon;
friend class CMSN;
};

typedef std::vector<CDaemonStats> DaemonStatsList;
#define STATS_EventsSent 0
#define STATS_EventsReceived 1
#define STATS_EventsRejected 2
#define STATS_AutoResponseChecked 3
// We will save the statistics to disk
#define SAVE_STATS


//====Conversations=============================================================
class CConversation
{
public:
  bool HasUser(const char *sz);
  bool IsEmpty() const                  { return m_vUsers.size() == 0; }
  int NumUsers() const                  { return m_vUsers.size(); }

  int Socket() const                    { return m_nSocket; }
  unsigned long CID() const             { return m_nCID; }

  std::string GetUser(int n) const      { return m_vUsers[n]; }

private:
  CConversation(int nSocket, unsigned long nPPID);
  ~CConversation();

  bool AddUser(const char *sz);
  bool RemoveUser(const char *sz);

  int m_nSocket;
  unsigned long m_nPPID;
  unsigned long m_nCID;
  std::vector<std::string> m_vUsers;

  static unsigned long s_nCID;
  static pthread_mutex_t s_xMutex;

friend class CICQDaemon;
};

typedef std::list<CConversation *> ConversationList;

/**
 * Internal template class for storing and processing received contact list.
 */
class CUserProperties
{
public:
  CUserProperties();

private:
  boost::shared_array<char> newAlias;
  boost::shared_array<char> newCellular;

  unsigned short normalSid;
  unsigned short groupId;

  unsigned short visibleSid;
  unsigned short invisibleSid;
  bool inIgnoreList;

  bool awaitingAuth;

  TLVList tlvs;

friend class CICQDaemon;
};

typedef std::map<std::string, CUserProperties*> ContactUserList;
typedef ContactUserList::iterator ContactUserListIter;

//=====CICQDaemon===============================================================
enum EDaemonStatus {STATUS_ONLINE, STATUS_OFFLINE_MANUAL, STATUS_OFFLINE_FORCED };

class CICQDaemon
{
public:
  CICQDaemon(CLicq *);
  ~CICQDaemon();
  int RegisterPlugin(unsigned long _nSignalMask);
  void UnregisterPlugin();
  bool Start();
  const char* Version() const;
  pthread_t *Shutdown();
  void SaveConf();

  // GUI Plugins call these now
  //! Add a user to the local contact list.
  /*!
      \param szId The user ID to add.
      \param nPPID The user's protocol plugin ID.
      \param groupId The group id to add the user into.
      \param _bAuthRequired True if we need to get authorization first.
  */
  void ProtoAddUser(const char *szId, unsigned long nPPID, bool _bAuthRequired = false, unsigned short groupId = 0);

  //! Remove a user from the local contact list.
  /*!
      \param szId The user ID to remove.
      \param nPPID The user's protocol plugin ID.
  */
  void ProtoRemoveUser(const char *szId, unsigned long nPPID);

  //! Rename a user on the server contact list.
  /*!
       \param szId The user ID to rename.
       \param nPPID The user's protocol plugin ID.
  */
  void ProtoRenameUser(const char *szId, unsigned long nPPID);

  //! Change status for a protocol.
  /*!
       \param nPPID The protocol ID.
       \param nNewStatus The status to change to.
  */
  unsigned long ProtoSetStatus(unsigned long nPPID, unsigned short nNewStatus);

  //! Logon for a protocol.
  /*!
        \param nPPID The protocol ID.
        \param nLogonStatus The initial status to set after logon is complete.
  */
  unsigned long ProtoLogon(unsigned long nPPID, unsigned short nLogonStatus);

  //! Logoff from a protocol.
  /*!
        \param nPPID The protocol ID.
  */
  void ProtoLogoff(unsigned long nPPID);

  //! Send typing notification.

  /*!
        \param szId The user ID.
        \param nPPID The protocol ID.
        \param bActive The state of the typing notification. TRUE if active.
  */
  void ProtoTypingNotification(const char *szId, unsigned long nPPID,
                               bool Active, int nSocket = -1);

  //! Send a message to a user on this protocol.
  /*!
        \param szId The user ID.
        \param nPPID The protocol ID.
        \param szMessage The message to be sent.
        \param bOnline True if the user is online.
        \param nLevel Any special flags (protocol specific).
        \param bMultipleRecipients True if sending the same message to
          more than one user.
        \param pColor The color of the text and background.
        \param nCID The conversation ID for group messages (MSN & IRC)
  */
  unsigned long ProtoSendMessage(const char *szId, unsigned long nPPID,
     const char *szMessage, bool bOnline, unsigned short nLevel,
     bool bMultipleRecipients = false, CICQColor *pColor = NULL,
     unsigned long nCID = 0);

  //! Send a URL to a user on this protocol
  /*!
      This may not be available for all protocols.
        \param szId The user ID.
        \param szUrl The URL to be sent.
        \param szDescription A description of the URL that can be sent.
        \param bOnline True if the user is online.
        \param nLevel Any special flags (protocol specific).
        \param bMultipleRecipients True if sending the same URL to
          more than one user.
        \param pColor The color of the text and background.
  */      
  unsigned long ProtoSendUrl(const char *szId, unsigned long nPPID,
     const char *szUrl, const char *szDescription, bool bOnline,
     unsigned short nLevel, bool bMultipleRecipients = false,
     CICQColor *pColor = NULL);

  unsigned long ProtoFetchAutoResponseServer(const char *szId, unsigned long nPPID);

  unsigned long ProtoChatRequest(const char *szId, unsigned long nPPID,
     const char *szReason, unsigned short nLevel, bool bServer);
  unsigned long ProtoMultiPartyChatRequest(const char *szId, unsigned long nPPID,
     const char *szReason, const char *szChatUsers, unsigned short nPort,
     unsigned short nLevel, bool bServer);
  void ProtoChatRequestRefuse(const char *szId, unsigned long nPPID,
      const char* szReason, unsigned short nSequence,
      const unsigned long nMsgID[], bool bDirect);
  void ProtoChatRequestAccept(const char *szId, unsigned long nPPID,
      unsigned short nPort, const char* szClients, unsigned long nSequeunce,
      const unsigned long nMsgID[], bool bDirect);
  void ProtoChatRequestCancel(const char *szId, unsigned long nPPID,
     unsigned short nSequence);

  unsigned long ProtoFileTransfer(const char *szId, unsigned long nPPID,
     const char *szFilename, const char *szDescription, ConstFileList &lFileList,
     unsigned short nLevel, bool bServer);
  void ProtoFileTransferRefuse(const char *szId, unsigned long nPPID,
     const char *szReason, unsigned long nSequence, unsigned long nFlag1,
     unsigned long nFlag2, bool bDirect = false);
  void ProtoFileTransferCancel(const char *szId, unsigned long nPPID,
     unsigned long nSequence);
  void ProtoFileTransferAccept(const char *szId, unsigned long nPPID,
     unsigned short nPort, unsigned long nSequence = 0, unsigned long nFlag1 = 0,
     unsigned long nFlag2 = 0, const char *szDesc = 0, const char *szFile = 0,
     unsigned long nFileSize = 0,bool bDirect = false);

  unsigned long ProtoAuthorizeGrant(const char *szId, unsigned long nPPID,
     const char *szMessage);

  unsigned long ProtoAuthorizeRefuse(const char *szId, unsigned long nPPID,
     const char *szMessage);

  unsigned long ProtoRequestInfo(const char *szId, unsigned long nPPID);

  unsigned long ProtoSetGeneralInfo(unsigned long nPPID, const char *szAlias,
    const char *szFirstName, const char *szLastName, const char *szEmailPrimary,
    const char *szCity, const char *szState, const char *szPhoneNumber,
    const char *szFaxNumber, const char *szAddress, const char *szCellularNumber,
    const char *szZipCode, unsigned short nCountryCode, bool bHideEmail);

  unsigned long ProtoRequestPicture(const char *szId, unsigned long nPPID);

  unsigned long ProtoOpenSecureChannel(const char *szId, unsigned long nPPID);
  unsigned long ProtoCloseSecureChannel(const char *szId, unsigned long nPPID);
  void ProtoOpenSecureChannelCancel(const char *szId, unsigned long nPPID,
    unsigned long nSequence);

  // TCP (user) functions
  // Message
  unsigned long icqSendMessage(const char *szId, const char *szMessage,
     bool bOnline, unsigned short nLevel, bool bMultipleRecipients = false,
     CICQColor *pColor = NULL);

  // Url
  unsigned long icqSendUrl(const char *szId, const char *szUrl,
     const char *szDescription, bool bOnline, unsigned short nLevel,
     bool bMultipleRecipients = false, CICQColor *pColor = NULL);
  // Contact List
  unsigned long icqSendContactList(const char *szId, const StringList& users,
     bool bOnline, unsigned short nLevel, bool bMultipleRecipients = false,
     CICQColor *pColor = NULL);

  // Auto Response
  unsigned long icqFetchAutoResponse(const char *_szId, unsigned long _nPPID, bool bServer = false);
  // Chat Request
  unsigned long icqChatRequest(const char* id, const char *szReason,
     unsigned short nLevel, bool bServer);
  unsigned long icqMultiPartyChatRequest(const char* id,
     const char *szReason, const char *szChatUsers, unsigned short nPort,
     unsigned short nLevel, bool bServer);
  void icqChatRequestRefuse(const char* id, const char* szReason,
      unsigned short nSequence, const unsigned long nMsgID[], bool bDirect);
  void icqChatRequestAccept(const char* id, unsigned short nPort,
      const char* szClients, unsigned short nSequence,
      const unsigned long nMsgID[], bool bDirect);
  void icqChatRequestCancel(const char* id, unsigned short nSequence);
  // File Transfer
  unsigned long icqFileTransfer(const char *szId, const char *szFilename,
     const char *szDescription, ConstFileList &lFileList,
     unsigned short nLevel, bool bServer);
  void icqFileTransferRefuse(const char *szId, const char *szReason,
      unsigned short nSequence, const unsigned long nMsgID[], bool bDirect);
  void icqFileTransferCancel(const char *szId, unsigned short nSequence);
  void icqFileTransferAccept(const char *szId, unsigned short nPort,
     unsigned short nSequence, const unsigned long nMsgID[], bool bDirect,
     const char *szDesc, const char *szFile, unsigned long nFileSize);  
  unsigned long icqOpenSecureChannel(const char *szId);
  unsigned long icqCloseSecureChannel(const char *szId);
  void icqOpenSecureChannelCancel(const char *szId, unsigned short nSequence);

  // Plugins
  unsigned long icqRequestInfoPluginList(const char *szId,
     bool bServer = false);
  unsigned long icqRequestPhoneBook(const char *szId, bool bServer = false);
  unsigned long icqRequestPicture(const char *szId, bool bServer = false);
  unsigned long icqRequestStatusPluginList(const char *szId,
     bool bServer = false);
  unsigned long icqRequestSharedFiles(const char *szId, bool bServer = false);
  unsigned long icqRequestPhoneFollowMe(const char *szId,
     bool bServer = false);
  unsigned long icqRequestICQphone(const char *szId, bool bServer = false);

  // Server functions
  void icqRegister(const char *_szPasswd);
  void icqVerifyRegistration();
  void icqVerify(const char *);
  unsigned long icqFetchAutoResponseServer(const char *);
  unsigned long icqLogon(unsigned short logonStatus);
  unsigned long icqRequestLogonSalt();
  unsigned long icqUserBasicInfo(const char *);
  unsigned long icqUserExtendedInfo(const char *);
  unsigned long icqRequestMetaInfo(const char *);

  unsigned long icqUpdateBasicInfo(const char *, const char *, const char *,
                                       const char *, bool);
  unsigned long icqUpdateExtendedInfo(const char *, unsigned short, const char *,
                                unsigned short, char, const char *,
                                const char *, const char *_sAbout, const char *);
  unsigned long icqSetWorkInfo(const char *_szCity, const char *_szState,
                           const char *_szPhone,
                           const char *_szFax, const char *_szAddress,
                           const char *_szZip, unsigned short _nCompanyCountry,
                           const char *_szName, const char *_szDepartment,
                           const char *_szPosition, unsigned short _nCompanyOccupation,
                           const char *_szHomepage);
  unsigned long icqSetGeneralInfo(const char *szAlias, const char *szFirstName,
                              const char *szLastName, const char *szEmailPrimary,
                              const char *szCity,
                              const char *szState, const char *szPhoneNumber,
                              const char *szFaxNumber, const char *szAddress,
                              const char *szCellularNumber, const char *szZipCode,
                              unsigned short nCountryCode, bool bHideEmail);
  unsigned long icqSetEmailInfo(const char *szEmailSecondary, const char *szEmailOld);
  unsigned long icqSetMoreInfo(unsigned short nAge,
                           char nGender, const char *szHomepage,
                           unsigned short nBirthYear, char nBirthMonth,
                           char nBirthDay, char nLanguage1,
                           char nLanguage2, char nLanguage3);
  unsigned long icqSetSecurityInfo(bool bAuthorize, bool bHideIp, bool bWebAware);
  unsigned long icqSetInterestsInfo(const ICQUserCategory *interests);
  unsigned long icqSetOrgBackInfo(const ICQUserCategory *orgs,
                                  const ICQUserCategory *background);
  unsigned long icqSetHomepageInfo(bool bCatetory, unsigned short nCategory,
                                const char *szHomepageDesc, bool bICQHomepage);
  unsigned long icqSetAbout(const char *szAbout);
  unsigned long icqSetPassword(const char *szPassword);
  unsigned long icqSetStatus(unsigned short newStatus);
  unsigned long icqSetRandomChatGroup(unsigned long nGroup);
  unsigned long icqRandomChatSearch(unsigned long nGroup);
  unsigned long icqSearchWhitePages(const char *szFirstName,
                            const char *szLastName, const char *szAlias,
                            const char *szEmail, unsigned short nMinAge,
                            unsigned short nMaxAge, char nGender,
                            char nLanguage, const char *szCity,
                            const char *szState, unsigned short nCountryCode,
                            const char *szCoName, const char *szCoDept,
                            const char *szCoPos, const char *szKeyword,
                            bool bOnlineOnly);
  unsigned long icqSearchByUin(unsigned long);

  void icqLogoff();
  void postLogoff(int nSD, ICQEvent *cancelledEvent);
  void icqRelogon();
  unsigned long icqAuthorizeGrant(const char *szId, const char *szMessage);
  unsigned long icqAuthorizeRefuse(const char *szId, const char *szMessage);
  void icqRequestAuth(const char* id, const char *_szMessage);
  void icqAlertUser(const char* id, unsigned long ppid);
  void icqAddUser(const char *_szId, bool _bAuthReq = false, unsigned short groupId = 0);
  void icqAddUserServer(const char *_szId, bool _bAuthReq, unsigned short groupId = 0);
  void icqAddGroup(const char *);
  void icqRemoveUser(const char *);
  void icqRemoveGroup(const char *);
  void icqChangeGroup(const char *_szId, unsigned long _nPPID,
                      unsigned short _nNewGroup, unsigned short _nOldGSID,
                      unsigned short _nNewType, unsigned short _nOldType);
  void icqRenameGroup(const char *_szNewName, unsigned short _nGSID);
  void icqRenameUser(const char *_szId);
  void icqExportUsers(const StringList& users, unsigned short);
  void icqExportGroups(const GroupNameMap& groups);
  void icqUpdateServerGroups();
  void icqUpdatePhoneBookTimestamp();
  void icqUpdatePictureTimestamp();
  void icqSetPhoneFollowMeStatus(unsigned long nNewStatus);
  void icqUpdateContactList();
  void icqTypingNotification(const char *_szId, bool _bActive);
  void icqCheckInvisible(const char *_szId);
  void icqRequestService(unsigned short nFam);

  // Visible/Invisible/Ignore list functions
  /**
   * Set visible list status for a contact
   *
   * @param id User id
   * @param ppid User protocol id
   * @param visible True to add user to visible list or false to remove
   */
  void ProtoSetInVisibleList(const char* id, unsigned long ppid, bool visible);

  /**
   * Set invisible list status for a contact
   *
   * @param id User id
   * @param ppid User protocol id
   * @param invisible True to add user to invisible list or false to remove
   */
  void ProtoSetInInvisibleList(const char* id, unsigned long ppid, bool invisible);

  /**
   * Set ignore list status for a contact
   *
   * @param id User id
   * @param ppid User protocol id
   * @param ignore True to add user to ignore list or false to remove
   */
  void ProtoSetInIgnoreList(const char* id, unsigned long ppid, bool ignore);

  void ProtoToggleInvisibleList(const char *_szId, unsigned long _nPPID);
  void ProtoToggleVisibleList(const char *_szId, unsigned long _nPPID);

  void icqAddToVisibleList(const char *_szId, unsigned long _nPPID);
  void icqRemoveFromVisibleList(const char *_szId, unsigned long _nPPID);
  void icqAddToInvisibleList(const char *_szId, unsigned long _nPPID);
  void icqRemoveFromInvisibleList(const char *_szId, unsigned long _nPPID);
  void icqAddToIgnoreList(const char *_szId, unsigned long _nPPID);
  void icqRemoveFromIgnoreList(const char *_szId, unsigned long _nPPID);
  void icqToggleIgnoreList(const char *_szId, unsigned long _nPPID);

  void icqClearServerList();
  void CheckExport();

  void PluginList(PluginsList &l);
  void PluginShutdown(int);
  void PluginEnable(int);
  void PluginDisable(int);
  bool PluginLoad(const char *, int, char **);


  void ProtoPluginList(ProtoPluginsList &);
  void ProtoPluginShutdown(unsigned short);
  bool ProtoPluginLoad(const char *);
  int RegisterProtoPlugin();
  void UnregisterProtoPlugin();
  const char* ProtoPluginName(unsigned long ppid) const;

  EDaemonStatus Status() const                  { return m_eStatus; }

  void PluginUIViewEvent(const char *szId, unsigned long nPPID ) {
    PushPluginSignal(new CICQSignal(SIGNAL_UI_VIEWEVENT, 0, szId, nPPID, 0));
  }
  void PluginUIMessage(const char *szId, unsigned long nPPID) {
    PushPluginSignal(new CICQSignal(SIGNAL_UI_MESSAGE, 0, szId, nPPID, 0));
  }

  void UpdateAllUsers();
  void UpdateAllUsersInGroup(GroupType, unsigned short);
  void CancelEvent(unsigned long );
  void CancelEvent(ICQEvent *);
  bool OpenConnectionToUser(const char* id, TCPSocket *sock,
     unsigned short nPort);
  bool OpenConnectionToUser(const char *szAlias, unsigned long nIp,
     unsigned long nIntIp, TCPSocket *sock, unsigned short nPort,
     bool bSendIntIp);
  int StartTCPServer(TCPSocket *);

  bool AddUserToList(const char *szId, unsigned long PPID, bool bNotify = true,
                     bool bTempUser = false, unsigned short groupId = 0);
  void AddUserToList(ICQUser *);
  void RemoveUserFromList(const char *szId, unsigned long nPPID);

  // SMS
  unsigned long icqSendSms(const char* id, unsigned long ppid,
      const char* number, const char* message);

  // NOT MT SAFE
  const char* getUrlViewer() const;
  void setUrlViewer(const char *s);

  bool ViewUrl(const char *url);

  // ICQ Server options
  const char* ICQServer() const                 { return m_szICQServer; }
  void SetICQServer(const char *s) {  SetString(&m_szICQServer, s);  }
  unsigned short ICQServerPort() const          { return m_nICQServerPort; }
  void SetICQServerPort(unsigned short p) {  m_nICQServerPort = p; }

  // Firewall options
  bool TCPEnabled() const                       { return m_bTCPEnabled; }
  void SetTCPEnabled(bool b) { m_bTCPEnabled = b; SetDirectMode(); }
  bool Firewall() const                         { return m_bFirewall; }
  void SetFirewall(bool b) { m_bFirewall = b; SetDirectMode(); }
  void SetDirectMode();

  // Proxy options
  void InitProxy();
  ProxyServer *CreateProxy();
  bool ProxyEnabled() const                     { return m_bProxyEnabled; }
  ProxyServer *GetProxy() {  return m_xProxy;  }
  void SetProxyEnabled(bool b) {  m_bProxyEnabled = b;  }
  unsigned short ProxyType() const              { return m_nProxyType; }
  void SetProxyType(unsigned short t) {  m_nProxyType = t;  }
  const char* ProxyHost() const                 { return m_szProxyHost; }
  void SetProxyHost(const char *s) {  SetString(&m_szProxyHost, s);  }
  unsigned short ProxyPort() const              { return m_nProxyPort; }
  void SetProxyPort(unsigned short p) {  m_nProxyPort = p;  }
  bool ProxyAuthEnabled() const                 { return m_bProxyAuthEnabled; }
  void SetProxyAuthEnabled(bool b) {  m_bProxyAuthEnabled = b;  }
  const char* ProxyLogin() const                { return m_szProxyLogin; }
  void SetProxyLogin(const char *s) {  SetString(&m_szProxyLogin, s);  }
  const char* ProxyPasswd() const               { return m_szProxyPasswd; }
  void SetProxyPasswd(const char *s) {  SetString(&m_szProxyPasswd, s);  }

  unsigned short TCPPortsLow() const            { return m_nTCPPortsLow; }
  unsigned short TCPPortsHigh() const           { return m_nTCPPortsHigh; }
  void SetTCPPorts(unsigned short p, unsigned short r);
  static bool CryptoEnabled();

  bool AutoUpdateInfo() const                   { return m_bAutoUpdateInfo; }
  bool AutoUpdateInfoPlugins() const            { return m_bAutoUpdateInfoPlugins; }
  bool AutoUpdateStatusPlugins() const          { return m_bAutoUpdateStatusPlugins; }

  void SetAutoUpdateInfo(bool b)          { m_bAutoUpdateInfo = b; }
  void SetAutoUpdateInfoPlugins(bool b)   { m_bAutoUpdateInfoPlugins = b; }
  void SetAutoUpdateStatusPlugins(bool b) { m_bAutoUpdateStatusPlugins = b; }

  // NOT MT SAFE
  const char* Terminal() const                  { return m_szTerminal; }
  void SetTerminal(const char *s);
  bool Ignore(unsigned short n) const           { return m_nIgnoreTypes & n; }
  void SetIgnore(unsigned short, bool);

  COnEventManager *OnEventManager()  { return &m_xOnEventManager; }
  // NOT MT SAFE
  bool AlwaysOnlineNotify() const               { return m_bAlwaysOnlineNotify; }
  void SetAlwaysOnlineNotify(bool);
  CICQSignal *PopPluginSignal();
  ICQEvent *PopPluginEvent();
  CSignal *PopProtoSignal();

  // ICQ options
  bool UseServerContactList() const             { return m_bUseSS; }
  bool UseServerSideBuddyIcons() const          { return m_bUseBART; }
  bool SendTypingNotification() const           { return m_bSendTN; }

  void SetUseServerContactList(bool b)    { m_bUseSS = b; }
  void SetUseServerSideBuddyIcons(bool b);
  void SetSendTypingNotification(bool b)  { m_bSendTN = b; }

  // Misc functions
  bool ReconnectAfterUinClash() const           { return m_bReconnectAfterUinClash; }
  void setReconnectAfterUinClash(bool b)     { m_bReconnectAfterUinClash = b; }
  bool AddProtocolPlugins();
  char *parseRTF(const char *);


  // Statistics
  CDaemonStats *Stats(unsigned short n) { return n < 3 ? &m_sStats[n] : NULL; }
  DaemonStatsList &AllStats() { return m_sStats; }
  time_t ResetTime() const                      { return m_nResetTime; }
  time_t StartTime() const                      { return m_nStartTime; }
  time_t Uptime() const                         { return time(NULL) - m_nStartTime; }
  void ResetStats();

  // Conversation functions
  CConversation *AddConversation(int nSocket, unsigned long nPPID);
  bool AddUserConversation(unsigned long nCID, const char *szId);
  bool AddUserConversation(int nSocket, const char *szId);
  bool RemoveUserConversation(unsigned long nCID, const char *szId);
  bool RemoveUserConversation(int nSocket, const char *szId);
  CConversation *FindConversation(int nSocket);
  CConversation *FindConversation(unsigned long nCID);
  bool RemoveConversation(unsigned long nCID);

  // Common message handler
  void ProcessMessage(ICQUser *user, CBuffer &packet, char *message,
     unsigned short nMsgType, unsigned long nMask,
      const unsigned long nMsgID[], unsigned short nSequence,
     bool bIsAck, bool &bNewUser);

  bool ProcessPluginMessage(CBuffer &packet, ICQUser *u, unsigned char nChannel,
     bool bIsAck, unsigned long nMsgID1,
     unsigned long nMsgID2, unsigned short nSequence,
     TCPSocket *pSock);
  bool WaitForReverseConnection(unsigned short id, const char* userId);

  // Deprecated functions, to be removed
  LICQ_DEPRECATED unsigned long icqSendMessage(unsigned long nUin, const char *szMessage,
     bool bOnline, unsigned short nLevel, bool bMultipleRecipients = false,
     CICQColor *pColor = NULL);
  LICQ_DEPRECATED unsigned long icqSendContactList(unsigned long nUin, UinList &uins,
     bool bOnline, unsigned short nLevel, bool bMultipleRecipients = false,
     CICQColor *pColor = NULL);
  LICQ_DEPRECATED unsigned long icqFetchAutoResponse(unsigned long nUin, bool bServer = false);
  LICQ_DEPRECATED unsigned long icqChatRequest(unsigned long nUin, const char *szReason,
     unsigned short nLevel, bool bServer);
  LICQ_DEPRECATED unsigned long icqMultiPartyChatRequest(unsigned long nUin,
     const char *szReason, const char *szChatUsers, unsigned short nPort,
     unsigned short nLevel, bool bServer);
  LICQ_DEPRECATED void icqChatRequestRefuse(unsigned long nUin, const char *szReason,
      unsigned short nSequence, const unsigned long nMsgID[], bool bDirect);
  LICQ_DEPRECATED void icqChatRequestAccept(unsigned long nUin, unsigned short nPort,
      const char* szClients, unsigned short nSequence,
      const unsigned long nMsgID[], bool bDirect);
  LICQ_DEPRECATED void icqChatRequestCancel(unsigned long nUin, unsigned short nSequence);
  LICQ_DEPRECATED unsigned long icqFileTransfer(unsigned long nUin, const char *szFilename,
     const char *szDescription, ConstFileList &lFileList,
     unsigned short nLevel, bool bServer);
  LICQ_DEPRECATED void icqFileTransferRefuse(unsigned long nUin, const char *szReason,
      unsigned short nSequence, const unsigned long nMsgID[], bool bDirect);
  LICQ_DEPRECATED void icqFileTransferCancel(unsigned long nUin, unsigned short nSequence);
  LICQ_DEPRECATED void icqFileTransferAccept(unsigned long nUin, unsigned short nPort,
     unsigned short nSequence, const unsigned long nMsgID[], bool bDirect,
     const char *szDesc, const char *szFile, unsigned long nFileSize);
  LICQ_DEPRECATED unsigned long icqOpenSecureChannel(unsigned long nUin);
  LICQ_DEPRECATED unsigned long icqCloseSecureChannel(unsigned long nUin);
  LICQ_DEPRECATED void icqOpenSecureChannelCancel(unsigned long nUin, unsigned short nSequence);
  LICQ_DEPRECATED unsigned long icqFetchAutoResponseServer(unsigned long);
  LICQ_DEPRECATED unsigned long icqUserBasicInfo(unsigned long);
  LICQ_DEPRECATED unsigned long icqUserExtendedInfo(unsigned long);
  LICQ_DEPRECATED unsigned long icqRequestMetaInfo(unsigned long);
  LICQ_DEPRECATED unsigned long icqAuthorizeGrant(unsigned long nUin, const char *szMessage);
  LICQ_DEPRECATED unsigned long icqAuthorizeRefuse(unsigned long nUin, const char *szMessage);
  LICQ_DEPRECATED void icqRequestAuth(unsigned long _nUin, const char *_szMessage);
  LICQ_DEPRECATED void icqAlertUser(unsigned long _nUin);
  LICQ_DEPRECATED void icqAddUser(unsigned long _nUin, bool _bAuthReq = false, unsigned short groupId = 0);
  LICQ_DEPRECATED void icqAddUserServer(unsigned long _nUin, bool _bAuthReq, unsigned short groupId = 0);
  LICQ_DEPRECATED void icqRemoveUser(unsigned long _nUin);
  LICQ_DEPRECATED void icqChangeGroup(unsigned long _nUin, unsigned short _nNewGroup,
                      unsigned short _nOldGSID, unsigned short _nNewType,
                      unsigned short _nOldType);
  LICQ_DEPRECATED void icqRenameUser(unsigned long _nUin);
  LICQ_DEPRECATED bool AddUserToList(unsigned long _nUin, bool bNotify = true,
                     bool bTempUser = false, unsigned short groupId = 0);
  LICQ_DEPRECATED void RemoveUserFromList(unsigned long _nUin);
  LICQ_DEPRECATED unsigned long icqSendSms(const char *szNumber, const char *szMessage,
                           unsigned long nUin);
  LICQ_DEPRECATED void icqAddToVisibleList(unsigned long nUin);
  LICQ_DEPRECATED void icqRemoveFromVisibleList(unsigned long nUin);
  LICQ_DEPRECATED void icqToggleVisibleList(unsigned long nUin);
  LICQ_DEPRECATED void icqAddToInvisibleList(unsigned long nUin);
  LICQ_DEPRECATED void icqRemoveFromInvisibleList(unsigned long nUin);
  LICQ_DEPRECATED void icqToggleInvisibleList(unsigned long nUin);
  LICQ_DEPRECATED void icqAddToIgnoreList(unsigned long nUin);
  LICQ_DEPRECATED void icqRemoveFromIgnoreList(unsigned long nUin);
  LICQ_DEPRECATED void icqToggleIgnoreList(unsigned long nUin);

protected:
  CLicq *licq;
  COnEventManager m_xOnEventManager;
  int pipe_newsocket[2], fifo_fd;
  FILE *fifo_fs;
  EDaemonStatus m_eStatus;
  char m_szConfigFile[MAX_FILENAME_LEN];

  char *m_szUrlViewer,
       *m_szTerminal,
       *m_szRejectFile;
  unsigned long m_nDesiredStatus,
                m_nIgnoreTypes;
  bool m_bAutoUpdateInfo, m_bAutoUpdateInfoPlugins, m_bAutoUpdateStatusPlugins;
  unsigned short m_nTCPPortsLow,
                 m_nTCPPortsHigh,
                 m_nMaxUsersPerPacket,
                 m_nServerSequence,
                 m_nErrorTypes;
  char m_szErrorFile[64];
  int m_nTCPSrvSocketDesc,
      m_nTCPSocketDesc;
  bool m_bShuttingDown,
       m_bLoggingOn,
       m_bRegistering,
       m_bOnlineNotifies,
       m_bAlwaysOnlineNotify,
       m_bTCPEnabled,
       m_bFirewall,
       m_bVerify,
       // NeedSalt is to let the daemon know when to make a salt request, which
       // should only happen when we first log on. After we get the credentials, we
       // do another logon, but it doesn't need to get a salt.
       m_bNeedSalt;
  time_t m_tLogonTime;
  char *m_szRegisterPasswd;
  pthread_t m_nRegisterThreadId;

  // ICQ Server
  char *m_szICQServer;
  unsigned short m_nICQServerPort;

  // Proxy
  bool m_bProxyEnabled;
  unsigned short m_nProxyType;
  char *m_szProxyHost;
  unsigned short m_nProxyPort;
  bool m_bProxyAuthEnabled;
  char *m_szProxyLogin;
  char *m_szProxyPasswd;
  ProxyServer *m_xProxy;

  // Services
  COscarService *m_xBARTService;

  // Misc
  bool m_bUseSS; // server side list
  bool m_bUseBART; // server side buddy icons
  bool m_bSendTN; // Send typing notifications
  bool m_bReconnectAfterUinClash; // reconnect after uin has been used from another location?

  // Statistics
  void FlushStats();
  DaemonStatsList m_sStats;
  time_t m_nStartTime, m_nResetTime;

  static std::list <CReverseConnectToUserData *> m_lReverseConnect;
  static pthread_mutex_t mutex_reverseconnect;
  static pthread_cond_t  cond_reverseconnect_done;

  ContactUserList receivedUserList;

  ConversationList m_lConversations;
  pthread_mutex_t mutex_conversations;

  std::list <ICQEvent *> m_lxRunningEvents;
  mutable pthread_mutex_t mutex_runningevents;
  std::list <ICQEvent *> m_lxExtendedEvents;
  pthread_mutex_t mutex_extendedevents;
  std::list <ICQEvent *> m_lxSendQueue_Server;
  pthread_mutex_t mutex_sendqueue_server;
  std::map <unsigned long, std::string> m_lszModifyServerUsers;
  pthread_mutex_t mutex_modifyserverusers;
  pthread_mutex_t mutex_cancelthread;
  pthread_t thread_monitorsockets,
            thread_ping,
            thread_updateusers,
            thread_ssbiservice,
            thread_shutdown;

  pthread_cond_t cond_serverack;
  pthread_mutex_t mutex_serverack;
  unsigned short m_nServerAck;

  void ChangeUserStatus(ICQUser *u, unsigned long s);
  bool AddUserEvent(ICQUser *, CUserEvent *);
  void RejectEvent(const char* id, CUserEvent* e);
  ICQUser *FindUserForInfoUpdate(const char *szId, ICQEvent *e, const char *);
  std::string FindUserByCellular(const char* cellular);

  void icqRegisterFinish();
  void icqPing();
  void icqSendVisibleList();
  void icqSendInvisibleList();
  void icqCreatePDINFO();
  void icqRequestSystemMsg();
  ICQEvent *icqSendThroughServer(const char *szId, unsigned char format, char *_sMessage,
    CUserEvent *, unsigned short = 0, size_t = 0);
  void SaveUserList();

  void FailEvents(int sd, int err);
  ICQEvent *DoneServerEvent(unsigned long, EventResult);
  ICQEvent *DoneEvent(ICQEvent *e, EventResult _eResult);
  ICQEvent *DoneEvent(int _nSD, unsigned short _nSequence, EventResult _eResult);
  ICQEvent *DoneEvent(unsigned long tag, EventResult _eResult);
  ICQEvent *DoneExtendedServerEvent(const unsigned short, EventResult);
  ICQEvent *DoneExtendedEvent(ICQEvent *, EventResult);
  ICQEvent *DoneExtendedEvent(unsigned long tag, EventResult _eResult);
  bool hasServerEvent(unsigned long subSequence) const;
  void ProcessDoneEvent(ICQEvent *);
  void PushEvent(ICQEvent *);
  void PushExtendedEvent(ICQEvent *);
  void PushPluginSignal(CICQSignal *);
  void PushPluginEvent(ICQEvent *);
  void PushProtoSignal(CSignal *, unsigned long);

  bool SendEvent(int nSD, CPacket &, bool);
  bool SendEvent(INetSocket *, CPacket &, bool);
  void SendEvent_Server(CPacket *packet);
  ICQEvent *SendExpectEvent_Server(const char *, unsigned long, CPacket *, CUserEvent *, bool = false);

  ICQEvent* SendExpectEvent_Server(CPacket* packet, CUserEvent* ue, bool extendedEvent = false)
  { return SendExpectEvent_Server("0", LICQ_PPID, packet, ue, extendedEvent); }

  ICQEvent* SendExpectEvent_Client(const ICQUser* u, CPacket* packet, CUserEvent* ue);
  ICQEvent *SendExpectEvent(ICQEvent *, void *(*fcn)(void *));
  void AckTCP(CPacketTcp &, int);
  void AckTCP(CPacketTcp &, TCPSocket *);

  bool ProcessSrvPacket(CBuffer&);

  //--- Channels ---------
  bool ProcessCloseChannel(CBuffer&);
  void ProcessDataChannel(CBuffer&);

  //--- Families ---------
  void ProcessServiceFam(CBuffer&, unsigned short);
  void ProcessLocationFam(CBuffer&, unsigned short);
  void ProcessBuddyFam(CBuffer&, unsigned short);
  void ProcessMessageFam(CBuffer&, unsigned short);
  void ProcessVariousFam(CBuffer&, unsigned short);
  void ProcessBOSFam(CBuffer&, unsigned short);
  void ProcessListFam(CBuffer &, unsigned short);
  void ProcessAuthFam(CBuffer &, unsigned short);

  void ProcessUserList();

  void ProcessSystemMessage(CBuffer &packet, unsigned long checkUin, unsigned short newCommand, time_t timeSent);
  void ProcessMetaCommand(CBuffer &packet, unsigned short nMetaCommand, ICQEvent *e);
  bool ProcessTcpPacket(TCPSocket *);
  bool ProcessTcpHandshake(TCPSocket *);
  void ProcessFifo(const char* buf);

  static bool Handshake_Send(TCPSocket *, const char* id, unsigned short,
                             unsigned short, bool = true, unsigned long = 0);
  static bool Handshake_SendConfirm_v7(TCPSocket *);
  static bool Handshake_Recv(TCPSocket *, unsigned short, bool = true, bool = false);
  static bool Handshake_RecvConfirm_v7(TCPSocket *);
  int ConnectToServer(const char* server, unsigned short port);
  int ConnectToLoginServer();
  int ConnectToUser(const char* id, unsigned char channel);
  int ReverseConnectToUser(const char* id, unsigned long nIp,
     unsigned short nPort, unsigned short nVersion, unsigned short nFailedPort,
     unsigned long nId, unsigned long nMsgID1, unsigned long nMsgID2);
  int RequestReverseConnection(const char* id, unsigned long, unsigned long,
                               unsigned short, unsigned short);

  // Protected plugin related stuff
  unsigned long icqRequestInfoPlugin(ICQUser *, bool, const char *);
  unsigned long icqRequestStatusPlugin(ICQUser *, bool, const char *);
  void icqUpdateInfoTimestamp(const char *);

  void StupidChatLinkageFix();

  // Helpers
  void addToModifyUsers(unsigned long unique_id, const std::string data);

  // Deprecated functions, to be removed
  LICQ_DEPRECATED void RejectEvent(unsigned long nUin, CUserEvent* e);
  LICQ_DEPRECATED unsigned long FindUinByCellular(const char *_szCellular);
  LICQ_DEPRECATED ICQEvent* icqSendThroughServer(unsigned long nUin,
      unsigned char format, char *_sMessage, CUserEvent *, unsigned short = 0);
  LICQ_DEPRECATED ICQEvent *SendExpectEvent_Server(unsigned long nUin,
      CPacket *, CUserEvent *, bool = false);

  // Declare all our thread functions as friends
  friend void *Ping_tep(void *p);
  friend void *UpdateUsers_tep(void *p);
  friend void *MonitorSockets_tep(void *p);
  friend void *ReverseConnectToUser_tep(void *p);
  friend void *ProcessRunningEvent_Client_tep(void *p);
  friend void *ProcessRunningEvent_Server_tep(void *p);
  friend void *OscarServiceSendQueue_tep(void *p);
  friend void *Shutdown_tep(void *p);
  friend void *ConnectToServer_tep(void *s);
  friend class ICQUser;
  friend class CSocketManager;
  friend class COscarService;
  friend class CChatManager;
  friend class CFileTransferManager;
  friend class COnEventManager;
  friend class CUserManager;
  friend class CLicq;
  friend class CMSN;
};

// Global pointer
extern CICQDaemon *gLicqDaemon;

// Helper functions for the daemon
bool ParseFE(char *szBuffer, char ***szSubStr, int nMaxSubStr);
unsigned long StringToStatus(const char* status);
unsigned short VersionToUse(unsigned short);

// Data structure for passing information to the reverse connection thread
class CReverseConnectToUserData
{
public:
  CReverseConnectToUserData(const char* idString, unsigned long id,
      unsigned long data, unsigned long ip, unsigned short port,
      unsigned short version, unsigned short failedport, unsigned long msgid1,
      unsigned long msgid2);
  ~CReverseConnectToUserData();

  std::string myIdString;
  unsigned long nId;
  unsigned long nData;
  unsigned long nIp;
  unsigned short nPort;
  unsigned short nFailedPort;
  unsigned short nVersion;
  unsigned long nMsgID1;
  unsigned long nMsgID2;
  bool bSuccess;
  bool bFinished;
};



#endif

