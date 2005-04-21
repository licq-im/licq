/*
ICQ.H
header file containing all the main procedures to interface with the ICQ server at mirabilis
*/

#ifndef ICQD_H
#define ICQD_H

#include <vector>
#include <list>
#include <deque>
#include <string>
#include <algorithm>
#include <stdarg.h>
#include <stdio.h>

#include "licq_events.h"
#include "licq_filetransfer.h"
#include "licq_onevent.h"
#include "licq_user.h"
#include "licq_plugind.h"
#include "licq_color.h"
#include "licq_protoplugind.h"

using std::string;

class CProtoPlugin;
class CPlugin;
class CPacket;
class CPacketTcp;
class CLicq;
class CUserManager;
class ICQUser;
class CICQEventTag;
class TCPSocket;
class SrvSocket;
class INetSocket;
class ProxyServer;
class CReverseConnectToUserData;
class CMSN;

const unsigned short IGNORE_MASSMSG    = 1;
const unsigned short IGNORE_NEWUSERS   = 2;
const unsigned short IGNORE_EMAILPAGER = 4;
const unsigned short IGNORE_WEBPANEL   = 8;

//-----Stats-----------------------------------------------------------------
//! Keeps information about statistics of the daemon
class CDaemonStats
{
public:
  // Accessors
  //! Total number of events.
  unsigned long Total() { return m_nTotal; }
  //! Total number of events for the current day only.
  unsigned long Today() { return m_nTotal - m_nOriginal; }
  //! Name of the kind of statistic.
  const char *Name()    { return m_szName; }

protected:
  CDaemonStats();
  CDaemonStats(const char *, const char *);

  bool Dirty() { return m_nLastSaved != m_nTotal; }
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
  bool IsEmpty()        { return m_vUsers.size() == 0; }
  int NumUsers()        { return m_vUsers.size(); }
    
  int Socket()          { return m_nSocket; }
  unsigned long CID()   { return m_nCID; }
  
  string GetUser(int n) { return m_vUsers[n]; }

private:  
  CConversation(int nSocket, unsigned long nPPID);
  ~CConversation() { }
    
  bool AddUser(const char *sz);
  bool RemoveUser(const char *sz);
  
  int m_nSocket;
  unsigned long m_nPPID;
  unsigned long m_nCID;
  vector<string> m_vUsers;
  
  static unsigned long s_nCID;
  static pthread_mutex_t s_xMutex;
  
  friend class CICQDaemon;
};

typedef std::list<CConversation *> ConversationList;

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
  const char *Version();
  pthread_t *Shutdown();
  void SaveConf();

  // GUI Plugins call these now
  //! Add a user to the local contact list.
  /*!
      \param szId The user ID to add.
      \param nPPID The user's protocol plugin ID.
      \param _bAuthRequired True if we need to get authorization first.
  */
  void ProtoAddUser(const char *szId, unsigned long nPPID, bool _bAuthRequired = false);

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

  unsigned long ProtoSendContactList(const char *szId, unsigned long nPPID,
     UinList &uins, bool bOnline, unsigned short nLevel,
     bool bMultipleRecipients = false, CICQColor *pColor = NULL);

  unsigned long ProtoFetchAutoResponseServer(const char *szId, unsigned long nPPID);

  unsigned long ProtoChatRequest(const char *szId, unsigned long nPPID,
     const char *szReason, unsigned short nLevel, bool bServer);
  unsigned long ProtoMultiPartyChatRequest(const char *szId, unsigned long nPPID,
     const char *szReason, const char *szChatUsers, unsigned short nPort,
     unsigned short nLevel, bool bServer);
  void ProtoChatRequestRefuse(const char *szId, unsigned long nPPID,
     const char *szReason, unsigned short nSequence, unsigned long nMsgID[],
     bool bDirect);
  void ProtoChatRequestAccept(const char *szId, unsigned long nPPID,
     unsigned short nPort, const char *szClients, unsigned long nSequeunce,
     unsigned long nMsgID[], bool bDirect);
  void ProtoChatRequestCancel(const char *szId, unsigned long nPPID,
     unsigned short nSequence);
  
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
    
  // TCP (user) functions
  // Message
  unsigned long icqSendMessage(const char *szId, const char *szMessage,
     bool bOnline, unsigned short nLevel, bool bMultipleRecipients = false,
     CICQColor *pColor = NULL);

  unsigned long icqSendMessage(unsigned long nUin, const char *szMessage,
     bool bOnline, unsigned short nLevel, bool bMultipleRecipients = false,
     CICQColor *pColor = NULL);
  // Url
  unsigned long icqSendUrl(unsigned long nUin, const char *szUrl,
     const char *szDescription, bool bOnline, unsigned short nLevel,
     bool bMultipleRecipients = false, CICQColor *pColor = NULL);
  // Contact List
  unsigned long icqSendContactList(const char *szId, UserStringList &users,
     bool bOnline, unsigned short nLevel, bool bMultipleRecipients = false,
     CICQColor *pColor = NULL);

  unsigned long icqSendContactList(unsigned long nUin, UinList &uins,
     bool bOnline, unsigned short nLevel, bool bMultipleRecipients = false,
     CICQColor *pColor = NULL);
  // Auto Response
  unsigned long icqFetchAutoResponse(unsigned long nUin, bool bServer = false) __attribute__ ((deprecated));
  unsigned long icqFetchAutoResponse(const char *_szId, unsigned long _nPPID, bool bServer = false);
  // Chat Request
  unsigned long icqChatRequest(unsigned long nUin, const char *szReason,
     unsigned short nLevel, bool bServer);
  unsigned long icqMultiPartyChatRequest(unsigned long nUin,
     const char *szReason, const char *szChatUsers, unsigned short nPort,
     unsigned short nLevel, bool bServer);
  void icqChatRequestRefuse(unsigned long nUin, const char *szReason,
     unsigned short nSequence, unsigned long nMsgID[], bool bDirect);
  void icqChatRequestAccept(unsigned long nUin, unsigned short nPort,
     const char *szClients, unsigned short nSequence, unsigned long nMsgID[], bool bDirect);
  void icqChatRequestCancel(unsigned long nUin, unsigned short nSequence);
  // File Transfer
  unsigned long icqFileTransfer(unsigned long nUin, const char *szFilename,
     const char *szDescription, ConstFileList &lFileList,
     unsigned short nLevel, bool bServer);
  void icqFileTransferRefuse(unsigned long nUin, const char *szReason,
     unsigned short nSequence, unsigned long nMsgID[], bool bDirect);
  void icqFileTransferCancel(unsigned long nUin, unsigned short nSequence);
  void icqFileTransferAccept(unsigned long nUin, unsigned short nPort,
     unsigned short nSequence, unsigned long nMsgID[], bool bDirect);
  unsigned long icqOpenSecureChannel(unsigned long nUin);
  unsigned long icqCloseSecureChannel(unsigned long nUin);
  void icqOpenSecureChannelCancel(unsigned long nUin, unsigned short nSequence);

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
  unsigned long icqFetchAutoResponseServer(const char *);
  unsigned long icqFetchAutoResponseServer(unsigned long);
  unsigned long icqLogon(unsigned short logonStatus);
  unsigned long icqUserBasicInfo(const char *);
  unsigned long icqUserBasicInfo(unsigned long);
  unsigned long icqUserExtendedInfo(const char *);
  unsigned long icqUserExtendedInfo(unsigned long);
  unsigned long icqRequestMetaInfo(const char *);
  unsigned long icqRequestMetaInfo(unsigned long);

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
  unsigned long icqAuthorizeGrant(unsigned long nUin, const char *szMessage);
  unsigned long icqAuthorizeGrant(const char *szId, const char *szMessage);
  unsigned long icqAuthorizeRefuse(unsigned long nUin, const char *szMessage);
  unsigned long icqAuthorizeRefuse(const char *szId, const char *szMessage);
  void icqRequestAuth(unsigned long _nUin, const char *_szMessage);
  void icqAlertUser(unsigned long _nUin);
  void icqAddUser(unsigned long _nUin, bool _bAuthReq = false);
  void icqAddUser(const char *_szId, bool _bAuthReq = false);
  void icqAddUserServer(const char *_szId, bool _bAuthReq);
  void icqAddUserServer(unsigned long _nUin, bool _bAuthReq);
  void icqAddGroup(const char *);
  void icqRemoveUser(unsigned long _nUin);
  void icqRemoveUser(const char *);
  void icqRemoveGroup(const char *);
  void icqChangeGroup(unsigned long _nUin, unsigned short _nNewGroup,
                      unsigned short _nOldGSID, unsigned short _nNewType,
                      unsigned short _nOldType);
  void icqChangeGroup(const char *_szId, unsigned long _nPPID,
                      unsigned short _nNewGroup, unsigned short _nOldGSID,
                      unsigned short _nNewType, unsigned short _nOldType);
  void icqRenameGroup(const char *_szNewName, unsigned short _nGSID);
  void icqRenameUser(unsigned long _nUin);
  void icqRenameUser(const char *_szId);
  void icqExportUsers(UserStringList &, unsigned short);
  void icqExportGroups(GroupList &);
  void icqUpdatePhoneBookTimestamp();
  void icqUpdatePictureTimestamp();
  void icqSetPhoneFollowMeStatus(unsigned long nNewStatus);
  void icqUpdateContactList();
  void icqTypingNotification(const char *_szId, bool _bActive);
  void icqCheckInvisible(const char *_szId);
  
  // Visible/Invisible/Ignore list functions
  void ProtoToggleInvisibleList(const char *_szId, unsigned long _nPPID);
  void ProtoToggleVisibleList(const char *_szId, unsigned long _nPPID);
  
  void icqAddToVisibleList(const char *_szId, unsigned long _nPPID);
  void icqRemoveFromVisibleList(const char *_szId, unsigned long _nPPID);
  void icqAddToInvisibleList(const char *_szId, unsigned long _nPPID);
  void icqRemoveFromInvisibleList(const char *_szId, unsigned long _nPPID);
  void icqAddToIgnoreList(const char *_szId, unsigned long _nPPID);
  void icqRemoveFromIgnoreList(const char *_szId, unsigned long _nPPID);
  void icqToggleIgnoreList(const char *_szId, unsigned long _nPPID);
  // Visible/Invisible/Ignore list functions (deprecated versions, for compatibility)
  void icqAddToVisibleList(unsigned long nUin) __attribute__ ((deprecated)); // deprecated!
  void icqRemoveFromVisibleList(unsigned long nUin) __attribute__ ((deprecated)); // deprecated!
  void icqToggleVisibleList(unsigned long nUin) __attribute__ ((deprecated)); // deprecated!
  void icqAddToInvisibleList(unsigned long nUin) __attribute__ ((deprecated)); // deprecated!
  void icqRemoveFromInvisibleList(unsigned long nUin) __attribute__ ((deprecated)); // deprecated!
  void icqToggleInvisibleList(unsigned long nUin) __attribute__ ((deprecated)); // deprecated!
  void icqAddToIgnoreList(unsigned long nUin) __attribute__ ((deprecated)); // deprecated!
  void icqRemoveFromIgnoreList(unsigned long nUin) __attribute__ ((deprecated)); // deprecated!
  void icqToggleIgnoreList(unsigned long nUin) __attribute__ ((deprecated)); // deprecated!

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
  char *ProtoPluginName(unsigned long);

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
  bool OpenConnectionToUser(unsigned long nUin, TCPSocket *sock,
     unsigned short nPort);
  bool OpenConnectionToUser(const char *szAlias, unsigned long nIp,
     unsigned long nIntIp, TCPSocket *sock, unsigned short nPort,
     bool bSendIntIp);
  int StartTCPServer(TCPSocket *);
  void CheckBirthdays(UinList &);
  unsigned short BirthdayRange() { return m_nBirthdayRange; }
  void BirthdayRange(unsigned short r) { m_nBirthdayRange = r; }

  bool AddUserToList(const char *szId, unsigned long PPID, bool bNotify = true,
                     bool bTempUser = false);
  bool AddUserToList(unsigned long _nUin, bool bNotify = true);
  void AddUserToList(ICQUser *);
  void RemoveUserFromList(unsigned long _nUin);
  void RemoveUserFromList(const char *szId, unsigned long nPPID);

  // SMS
  unsigned long icqSendSms(const char *szNumber, const char *szMessage,
			   unsigned long nUin);

  // NOT MT SAFE
  const char *getUrlViewer();
  void setUrlViewer(const char *s);

  bool ViewUrl(const char *url);

  // ICQ Server options
  const char *ICQServer() {  return m_szICQServer;  }
  void SetICQServer(const char *s) {  SetString(&m_szICQServer, s);  }
  unsigned short ICQServerPort() {  return m_nICQServerPort;  }
  void SetICQServerPort(unsigned short p) {  m_nICQServerPort = p; }

  // Firewall options
  bool TCPEnabled() { return m_bTCPEnabled; }
  void SetTCPEnabled(bool b) { m_bTCPEnabled = b; SetDirectMode(); }
  bool Firewall() { return m_bFirewall; }
  void SetFirewall(bool b) { m_bFirewall = b; SetDirectMode(); }
  void SetDirectMode();

  // Proxy options
  void InitProxy();
  bool ProxyEnabled() {  return m_bProxyEnabled;  }
  void SetProxyEnabled(bool b) {  m_bProxyEnabled = b;  }
  unsigned short ProxyType() {  return m_nProxyType;  }
  void SetProxyType(unsigned short t) {  m_nProxyType = t;  }
  const char *ProxyHost() {  return m_szProxyHost;  }
  void SetProxyHost(const char *s) {  SetString(&m_szProxyHost, s);  }
  unsigned short ProxyPort() {  return m_nProxyPort;  }
  void SetProxyPort(unsigned short p) {  m_nProxyPort = p;  }
  bool ProxyAuthEnabled() {  return m_bProxyAuthEnabled;  }
  void SetProxyAuthEnabled(bool b) {  m_bProxyAuthEnabled = b;  }
  const char *ProxyLogin() {  return m_szProxyLogin;  }
  void SetProxyLogin(const char *s) {  SetString(&m_szProxyLogin, s);  }
  const char *ProxyPasswd() {  return m_szProxyPasswd;  }
  void SetProxyPasswd(const char *s) {  SetString(&m_szProxyPasswd, s);  }

  unsigned short TCPPortsLow() { return m_nTCPPortsLow; }
  unsigned short TCPPortsHigh() { return m_nTCPPortsHigh; }
  void SetTCPPorts(unsigned short p, unsigned short r);
  static bool CryptoEnabled();

  bool AutoUpdateInfo()          { return m_bAutoUpdateInfo; }
  bool AutoUpdateInfoPlugins()   { return m_bAutoUpdateInfoPlugins; }
  bool AutoUpdateStatusPlugins() { return m_bAutoUpdateStatusPlugins; }

  void SetAutoUpdateInfo(bool b)          { m_bAutoUpdateInfo = b; }
  void SetAutoUpdateInfoPlugins(bool b)   { m_bAutoUpdateInfoPlugins = b; }
  void SetAutoUpdateStatusPlugins(bool b) { m_bAutoUpdateStatusPlugins = b; }

  const char *Terminal();
  void SetTerminal(const char *s);
  bool Ignore(unsigned short n)      { return m_nIgnoreTypes & n; }
  void SetIgnore(unsigned short, bool);

  COnEventManager *OnEventManager()  { return &m_xOnEventManager; }
  bool AlwaysOnlineNotify();
  void SetAlwaysOnlineNotify(bool);
  CICQSignal *PopPluginSignal();
  ICQEvent *PopPluginEvent();
  CSignal *PopProtoSignal();

  // Server Side List functions
  bool UseServerContactList()         { return m_bUseSS; }
  void SetUseServerContactList(bool b)  { m_bUseSS = b; }

  // Misc functions
  bool ReconnectAfterUinClash()              { return m_bReconnectAfterUinClash; }
  void setReconnectAfterUinClash(bool b)     { m_bReconnectAfterUinClash = b; }
  bool AddProtocolPlugins();
  char *parseRTF(const char *);

    
  // Statistics
  CDaemonStats *Stats(unsigned short n) { return n < 3 ? &m_sStats[n] : NULL; }
  DaemonStatsList &AllStats() { return m_sStats; }
  time_t ResetTime() { return m_nResetTime; }
  time_t StartTime() { return m_nStartTime; }
  time_t Uptime() { return time(NULL) - m_nStartTime; }
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
     unsigned long nMsgID[], unsigned short nSequence,
     bool bIsAck, bool &bNewUser);

  bool ProcessPluginMessage(CBuffer &packet, ICQUser *u, unsigned char nChannel,
     bool bIsAck, unsigned long nMsgID1,
     unsigned long nMsgID2, unsigned short nSequence,
     TCPSocket *pSock);
  bool WaitForReverseConnection(unsigned short id, unsigned long uin);
  
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
                 m_nErrorTypes,
                 m_nBirthdayRange;
  char m_szErrorFile[64];
  int m_nTCPSrvSocketDesc,
      m_nTCPSocketDesc;
  bool m_bShuttingDown,
       m_bLoggingOn,
       m_bRegistering,
       m_bOnlineNotifies,
       m_bAlwaysOnlineNotify,
       m_bTCPEnabled,
       m_bFirewall;
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

  // Misc
  bool m_bUseSS; // server side list
  bool m_bReconnectAfterUinClash; // reconnect after uin has been used from another location?
  
  // Statistics
  void FlushStats();
  DaemonStatsList m_sStats;
  time_t m_nStartTime, m_nResetTime;
  
  static std::list <CReverseConnectToUserData *> m_lReverseConnect;
  static pthread_mutex_t mutex_reverseconnect;
  static pthread_cond_t  cond_reverseconnect_done;

  ConversationList m_lConversations;
  pthread_mutex_t mutex_conversations;
  
  std::list <ICQEvent *> m_lxRunningEvents;
  pthread_mutex_t mutex_runningevents;
  std::list <ICQEvent *> m_lxExtendedEvents;
  pthread_mutex_t mutex_extendedevents;
  std::list <ICQEvent *> m_lxSendQueue_Server;
  pthread_mutex_t mutex_sendqueue_server;
  std::list <char *> m_lszModifyServerUsers;
  pthread_mutex_t mutex_modifyserverusers;
  pthread_mutex_t mutex_cancelthread;
  pthread_t thread_monitorsockets,
            thread_ping,
            thread_updateusers,
            thread_shutdown;

  pthread_cond_t cond_serverack;
  pthread_mutex_t mutex_serverack;
  unsigned short m_nServerAck;

  void ChangeUserStatus(ICQUser *u, unsigned long s);
  bool AddUserEvent(ICQUser *, CUserEvent *);
  void RejectEvent(unsigned long, CUserEvent *);
  ICQUser *FindUserForInfoUpdate(const char *szId, ICQEvent *e, const char *);
  unsigned long FindUinByCellular(const char *_szCellular);

  void icqRegisterFinish();
  void icqPing();
  void icqSendVisibleList();
  void icqSendInvisibleList();
  void icqRequestSystemMsg();
  ICQEvent *icqSendThroughServer(const char *szId, unsigned char format, char *_sMessage,
    CUserEvent *, unsigned short = 0);
  ICQEvent* icqSendThroughServer(unsigned long nUin, unsigned char format, char *_sMessage, CUserEvent *, unsigned short = 0);
  void SaveUserList();

  void FailEvents(int sd, int err);
  ICQEvent *DoneServerEvent(unsigned long, EventResult);
  ICQEvent *DoneEvent(ICQEvent *e, EventResult _eResult);
  ICQEvent *DoneEvent(int _nSD, unsigned short _nSequence, EventResult _eResult);
  ICQEvent *DoneEvent(unsigned long tag, EventResult _eResult);
  ICQEvent *DoneExtendedServerEvent(const unsigned short, EventResult);
  ICQEvent *DoneExtendedEvent(ICQEvent *, EventResult);
  ICQEvent *DoneExtendedEvent(unsigned long tag, EventResult _eResult);
  void ProcessDoneEvent(ICQEvent *);
  void PushExtendedEvent(ICQEvent *);
  void PushPluginSignal(CICQSignal *);
  void PushPluginEvent(ICQEvent *);
  void PushProtoSignal(CSignal *, unsigned long);

  bool SendEvent(int nSD, CPacket &, bool);
  bool SendEvent(INetSocket *, CPacket &, bool);
  void SendEvent_Server(CPacket *packet);
  ICQEvent *SendExpectEvent_Server(unsigned long nUin, CPacket *, CUserEvent *, bool = false);
  ICQEvent *SendExpectEvent_Server(const char *, unsigned long, CPacket *, CUserEvent *, bool = false);
  ICQEvent *SendExpectEvent_Client(ICQUser *, CPacket *, CUserEvent *);
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
  void ProcessNewUINFam(CBuffer &, unsigned short);

  void ProcessSystemMessage(CBuffer &packet, unsigned long checkUin, unsigned short newCommand, time_t timeSent);
  void ProcessMetaCommand(CBuffer &packet, unsigned short nMetaCommand, ICQEvent *e);
  bool ProcessTcpPacket(TCPSocket *);
  bool ProcessTcpHandshake(TCPSocket *);
  void ProcessFifo(char *);

  static bool Handshake_Send(TCPSocket *, unsigned long, unsigned short,
                             unsigned short, bool = true, unsigned long = 0);
  static bool Handshake_SendConfirm_v7(TCPSocket *);
  static bool Handshake_Recv(TCPSocket *, unsigned short, bool = true, bool = false);
  static bool Handshake_RecvConfirm_v7(TCPSocket *);
  int ConnectToServer(const char* server, unsigned short port);
  int ConnectToLoginServer();
  int ConnectToUser(unsigned long, unsigned char);
  int ReverseConnectToUser(unsigned long nUin, unsigned long nIp,
     unsigned short nPort, unsigned short nVersion, unsigned short nFailedPort,
     unsigned long nId, unsigned long nMsgID1, unsigned long nMsgID2);
  int RequestReverseConnection(unsigned long, unsigned long, unsigned long,
                               unsigned short, unsigned short);

  // Protected plugin related stuff
  unsigned long icqRequestInfoPlugin(ICQUser *, bool, const char *);
  unsigned long icqRequestStatusPlugin(ICQUser *, bool, const char *);
  void icqUpdateInfoTimestamp(const char *);

  void StupidChatLinkageFix();

  // Declare all our thread functions as friends
  friend void *Ping_tep(void *p);
  friend void *UpdateUsers_tep(void *p);
  friend void *MonitorSockets_tep(void *p);
  friend void *ReverseConnectToUser_tep(void *p);
  friend void *ProcessRunningEvent_Client_tep(void *p);
  friend void *ProcessRunningEvent_Server_tep(void *p);
  friend void *Shutdown_tep(void *p);
  friend void *ConnectToServer_tep(void *s);
  friend class ICQUser;
  friend class CSocketManager;
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
unsigned long StringToStatus(char *_szStatus);
unsigned short VersionToUse(unsigned short);

// Data structure for passing information to the reverse connection thread
class CReverseConnectToUserData
{
public:
  CReverseConnectToUserData(unsigned long uin, unsigned long id,
   unsigned long data, unsigned long ip, unsigned short port,
   unsigned short version, unsigned short failedport, unsigned long msgid1,
   unsigned long msgid2) :
   nUin(uin), nId(id), nData(data), nIp(ip), nPort(port),
   nFailedPort(failedport), nVersion(version), nMsgID1(msgid1),
   nMsgID2(msgid2), bSuccess(false), bFinished(false) {}

  unsigned long nUin;
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

