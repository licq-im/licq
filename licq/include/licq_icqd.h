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
#include "licq/contactlist/user.h" // For SetString
#include "licq/daemon.h"
#include "licq/userid.h"

class CPacketTcp;
class CLicq;
class CICQEventTag;
class CICQColor;
class TCPSocket;
class SrvSocket;
class INetSocket;
class ProxyServer;
class COscarService;
class CReverseConnectToUserData;
class CMSN;

namespace Licq
{
typedef std::list<std::string> StringList;
typedef std::map<unsigned int, std::string> UserCategoryMap;
class Packet;
}

// To keep old code working
typedef Licq::UserCategoryMap UserCategoryMap;
typedef std::map<int, std::string> GroupNameMap;
typedef Licq::Packet CPacket;

namespace LicqDaemon
{
class ProtocolManager;
class UserManager;
}

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

  Licq::TlvList tlvs;

friend class CICQDaemon;
};

typedef std::map<std::string, CUserProperties*> ContactUserList;
typedef ContactUserList::iterator ContactUserListIter;

//=====CICQDaemon===============================================================
enum EDaemonStatus {STATUS_ONLINE, STATUS_OFFLINE_MANUAL, STATUS_OFFLINE_FORCED };

class CICQDaemon : public Licq::Daemon
{
public:
  CICQDaemon(CLicq *);
  ~CICQDaemon();
  bool Start();
  const char* Version() const;
  pthread_t *Shutdown();
  void SaveConf();

  /**
   * Check if GPG support is enabled
   * This function allows plugins to check at runtime if GPG options are available
   *
   * @return True if GPG support is available in daemon
   */
  bool haveGpgSupport() const;

  // ICQ functions still public as they don't have any general proto functions
  //   to call them yet and needs to be callable from plugins for now

  unsigned long icqSendContactList(const char *szId, const Licq::StringList& users,
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

  // Plugins
  unsigned long icqRequestInfoPluginList(const char *szId,
     bool bServer = false);
  unsigned long icqRequestPhoneBook(const char *szId, bool bServer = false);
  unsigned long icqRequestPicture(const Licq::UserId& userId, bool bServer, size_t iconHashSize);
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
  unsigned long icqSetInterestsInfo(const Licq::UserCategoryMap& interests);
  unsigned long icqSetOrgBackInfo(const Licq::UserCategoryMap& orgs,
      const Licq::UserCategoryMap& background);
  unsigned long icqSetHomepageInfo(bool bCatetory, unsigned short nCategory,
                                const char *szHomepageDesc, bool bICQHomepage);
  unsigned long icqSetAbout(const char *szAbout);
  unsigned long icqSetPassword(const char *szPassword);
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
  unsigned long icqAuthorizeGrant(const Licq::UserId& userId, const std::string& message);
  unsigned long icqAuthorizeRefuse(const Licq::UserId& userId, const std::string& message);
  void icqRequestAuth(const char* id, const char *_szMessage);
  void icqAlertUser(const Licq::UserId& userId);
  void icqUpdatePhoneBookTimestamp();
  void icqUpdatePictureTimestamp();
  void icqSetPhoneFollowMeStatus(unsigned long nNewStatus);
  void icqUpdateContactList();
  void icqCheckInvisible(const char *_szId);

  void CheckExport();

  EDaemonStatus Status() const                  { return m_eStatus; }

  void pluginUIViewEvent(const Licq::UserId& userId)
  { pushPluginSignal(new LicqSignal(SIGNAL_UI_VIEWEVENT, 0, userId)); }

  void pluginUIMessage(const Licq::UserId& userId)
  { pushPluginSignal(new LicqSignal(SIGNAL_UI_MESSAGE, 0, userId)); }

  void UpdateAllUsers();
  void updateAllUsersInGroup(int groupId);
  void CancelEvent(unsigned long );
  void CancelEvent(ICQEvent *);
  bool OpenConnectionToUser(const char* id, TCPSocket *sock,
     unsigned short nPort);
  bool OpenConnectionToUser(const char *szAlias, unsigned long nIp,
     unsigned long nIntIp, TCPSocket *sock, unsigned short nPort,
     bool bSendIntIp);
  int StartTCPServer(TCPSocket *);

  // SMS
  unsigned long icqSendSms(const char* id, unsigned long ppid,
      const char* number, const char* message);

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

  /**
   * Get the next queued signal for a plugin
   * Checks calling thread to determine which plugin queue to pop
   *
   * @return The next queued signal or NULL if the queue is empty
   */
  LicqSignal* popPluginSignal();
  ICQEvent *PopPluginEvent();
  LicqProtoSignal* PopProtoSignal();

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

  // Common message handler
  void ProcessMessage(Licq::User* user, CBuffer& packet, char* message,
     unsigned short nMsgType, unsigned long nMask,
      const unsigned long nMsgID[], unsigned short nSequence,
     bool bIsAck, bool &bNewUser);

  bool ProcessPluginMessage(CBuffer& packet, Licq::User* user, unsigned char nChannel,
     bool bIsAck, unsigned long nMsgID1,
     unsigned long nMsgID2, unsigned short nSequence,
     TCPSocket *pSock);
  bool WaitForReverseConnection(unsigned short id, const char* userId);

  // From Licq::Daemon
  Licq::LogService& getLogService();

protected:

  // ICQ protocol functions, only called from general proto functions and will
  //   be removed when ICQ is moved to separate protocol plugin

  void icqSendMessage(unsigned long eventId, const Licq::UserId& userId, const std::string& message,
      bool viaServer, unsigned short nLevel, bool bMultipleRecipients = false,
     CICQColor *pColor = NULL);
  void icqSendUrl(unsigned long eventId, const Licq::UserId& userId, const std::string& url,
      const std::string& message, bool viaServer, unsigned short nLevel,
     bool bMultipleRecipients = false, CICQColor *pColor = NULL);
  void icqFileTransfer(unsigned long eventId, const Licq::UserId& userId, const std::string& filename,
      const std::string& message, ConstFileList &lFileList,
     unsigned short nLevel, bool bServer);
  void icqFileTransferRefuse(const Licq::UserId& userId, const std::string& message,
      unsigned short nSequence, const unsigned long nMsgID[], bool viaServer);
  void icqFileTransferCancel(const Licq::UserId& userId, unsigned short nSequence);
  void icqFileTransferAccept(const Licq::UserId& userId, unsigned short nPort,
      unsigned short nSequence, const unsigned long nMsgID[], bool viaServer,
      const std::string& message, const std::string& filename, unsigned long nFileSize);
  void icqOpenSecureChannel(unsigned long eventId, const Licq::UserId& userId);
  void icqCloseSecureChannel(unsigned long eventId, const Licq::UserId& userId);
  void icqOpenSecureChannelCancel(const Licq::UserId& userId, unsigned short nSequence);
  void icqFetchAutoResponseServer(unsigned long eventId, const char *);
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
  unsigned long icqSetStatus(unsigned short newStatus);
  void icqLogoff();
  void postLogoff(int nSD, ICQEvent *cancelledEvent);
  void icqRelogon();
  void icqAddUser(const char *_szId, bool _bAuthReq = false, unsigned short groupId = 0);
  void icqAddUserServer(const char *_szId, bool _bAuthReq, unsigned short groupId = 0);
  void icqAddGroup(const char *);
  void icqRemoveUser(const char *, bool ignored = false);
  void icqRemoveGroup(const char *);
  void icqChangeGroup(const char *_szId, unsigned long _nPPID,
                      unsigned short _nNewGroup, unsigned short _nOldGSID,
                      unsigned short _nNewType, unsigned short _nOldType);
  void icqRenameGroup(const char *_szNewName, unsigned short _nGSID);
  void icqRenameUser(const std::string& accountId, const std::string& newAlias);
  void icqExportUsers(const std::list<Licq::UserId>& users, unsigned short);
  void icqExportGroups(const GroupNameMap& groups);
  void icqUpdateServerGroups();
  void icqTypingNotification(const char *_szId, bool _bActive);
  void icqRequestService(unsigned short nFam);
  void icqAddToVisibleList(const Licq::UserId& userId);
  void icqRemoveFromVisibleList(const Licq::UserId& userId);
  void icqAddToInvisibleList(const Licq::UserId& userId);
  void icqRemoveFromInvisibleList(const Licq::UserId& userId);
  void icqAddToIgnoreList(const Licq::UserId& userId);
  void icqRemoveFromIgnoreList(const Licq::UserId& userId);
  void icqClearServerList();


  CLicq *licq;
  int pipe_newsocket[2], fifo_fd;
  FILE *fifo_fs;
  EDaemonStatus m_eStatus;
  char m_szConfigFile[MAX_FILENAME_LEN];

  char* m_szTerminal;
  char* m_szRejectFile;
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

  unsigned long myNextEventId;
  pthread_mutex_t myNextEventIdMutex;

  pthread_cond_t cond_serverack;
  pthread_mutex_t mutex_serverack;
  unsigned short m_nServerAck;

  /**
   * Get next available id to use for an event
   */
  unsigned long getNextEventId();

  void ChangeUserStatus(Licq::User* u, unsigned long s);

  bool AddUserEvent(Licq::User* user, CUserEvent* e);
  void RejectEvent(const Licq::UserId& userId, CUserEvent* e);
  Licq::User* FindUserForInfoUpdate(const Licq::UserId& userId, LicqEvent* e, const char*);
  std::string FindUserByCellular(const char* cellular);

  void icqRegisterFinish();
  void icqPing();
  void icqSendVisibleList();
  void icqSendInvisibleList();
  void icqCreatePDINFO();
  void icqRequestSystemMsg();
  LicqEvent* icqSendThroughServer(unsigned long eventId, const char *szId,
      unsigned char format, const std::string& message, CUserEvent *,
      unsigned short = 0, size_t = 0);

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

  /**
   * Add a signal to the signal queues of all plugins.
   *
   * @param signal Signal to send
   */
  void pushPluginSignal(LicqSignal* signal);

  void PushPluginEvent(ICQEvent *);
  void PushProtoSignal(LicqProtoSignal* s, unsigned long ppid);

  bool SendEvent(int nSD, CPacket &, bool);
  bool SendEvent(INetSocket *, CPacket &, bool);
  void SendEvent_Server(CPacket *packet);
  LicqEvent* SendExpectEvent_Server(unsigned long eventId, const Licq::UserId& userId, CPacket *, CUserEvent *, bool = false);

  LicqEvent* SendExpectEvent_Server(const Licq::UserId& userId, CPacket* packet, CUserEvent* ue, bool extendedEvent = false)
  { return SendExpectEvent_Server(getNextEventId(), userId, packet, ue, extendedEvent); }

  LicqEvent* SendExpectEvent_Server(unsigned long eventId, CPacket* packet, CUserEvent* ue, bool extendedEvent = false)
  { return SendExpectEvent_Server(eventId, Licq::UserId(), packet, ue, extendedEvent); }

  LicqEvent* SendExpectEvent_Server(CPacket* packet, CUserEvent* ue, bool extendedEvent = false)
  { return SendExpectEvent_Server(getNextEventId(), Licq::UserId(), packet, ue, extendedEvent); }

  LicqEvent* SendExpectEvent_Client(unsigned long eventId, const Licq::User* user, CPacket* packet, CUserEvent* ue);

  LicqEvent* SendExpectEvent_Client(const Licq::User* user, CPacket* packet, CUserEvent* ue)
  { return SendExpectEvent_Client(getNextEventId(), user, packet, ue); }

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
  unsigned long icqRequestInfoPlugin(Licq::User* user, bool, const char *);
  unsigned long icqRequestStatusPlugin(Licq::User* user, bool, const char *);
  void icqUpdateInfoTimestamp(const char *);

  void StupidChatLinkageFix();

  // Helpers
  void addToModifyUsers(unsigned long unique_id, const std::string data);

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
  friend class Licq::User;
  friend class CSocketManager;
  friend class COscarService;
  friend class CChatManager;
  friend class CFileTransferManager;
  friend class LicqDaemon::ProtocolManager;
  friend class LicqDaemon::UserManager;
  friend class CLicq;
  friend class CMSN;
  friend class Handler;
  friend class Jabber;
};

// Global pointer
extern CICQDaemon *gLicqDaemon;

// Helper functions for the daemon
bool ParseFE(char *szBuffer, char ***szSubStr, int nMaxSubStr);
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

