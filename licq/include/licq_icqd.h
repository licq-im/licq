/*
ICQ.H
header file containing all the main procedures to interface with the ICQ server at mirabilis
*/

#ifndef ICQD_H
#define ICQD_H

#include <vector.h>
#include <list.h>
#include <deque.h>
#include <stdarg.h>
#include <stdio.h>

#include "licq_events.h"
#include "licq_remoteserver.h"
#include "licq_onevent.h"
#include "licq_user.h"
#include "licq_plugind.h"

class CPlugin;
class CPacket;
class CPacketTcp;
class CLicq;
class ICQUser;
class CICQEventTag;
class TCPSocket;
class INetSocket;
class UDPSocket;


const unsigned short IGNORE_MASSMSG    = 1;
const unsigned short IGNORE_NEWUSERS   = 2;
const unsigned short IGNORE_EMAILPAGER = 4;
const unsigned short IGNORE_WEBPANEL   = 8;

//-----Stats-----------------------------------------------------------------
class CDaemonStats
{
public:
  // Accessors
  unsigned long Total() { return m_nTotal; }
  unsigned long Today() { return m_nTotal - m_nOriginal; }
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
};

typedef vector<CDaemonStats> DaemonStatsList;
#define STATS_EventsSent 0
#define STATS_EventsReceived 1
#define STATS_EventsRejected 2
#define STATS_AutoResponseChecked 3
// We will save the statistics to disk
#define SAVE_STATS


//---Color-------------------------------------------------------------------
class CICQColor
{
public:

  int fore_red, fore_green, fore_blue;
  int back_red, back_green, back_blue;

  // Plugins should not need these functions
  CICQColor() {}
  CICQColor(int fore, int back) {}

  int foreground()
   {
     return ((fore_red << 0) & 0xFF) + ((fore_green << 8) & 0xFF00) +
      ((fore_blue << 16) & 0xFF0000);
   }

  int background()
   {
     return ((back_red << 0) & 0xFF) + ((back_green << 8) & 0xFF00) +
      ((back_blue << 16) & 0xFF0000);
   }

};



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

  // TCP (user) functions
  // Message
  unsigned long icqSendMessage(unsigned long nUin, const char *szMessage,
     bool bOnline, unsigned short nLevel, bool bMultipleRecipients = false);
  // Url
  unsigned long icqSendUrl(unsigned long nUin, const char *szUrl,
     const char *szDescription, bool bOnline, unsigned short nLevel,
     bool bMultipleRecipients = false);
  // Contact List
  unsigned long icqSendContactList(unsigned long nUin, UinList &uins,
     bool bOnline, unsigned short nLevel, bool bMultipleRecipients = false);
  // Auto Response
  unsigned long icqFetchAutoResponse(unsigned long nUin);
  // Chat Request
  unsigned long icqChatRequest(unsigned long nUin, const char *szReason,
     unsigned short nLevel);
  unsigned long icqMultiPartyChatRequest(unsigned long nUin,
     const char *szReason, const char *szChatUsers, unsigned short nPort,
     unsigned short nLevel);
  void icqChatRequestRefuse(unsigned long nUin, const char *szReason,
     unsigned long nSequence);
  void icqChatRequestAccept(unsigned long nUin, unsigned short nPort,
     unsigned long nSequence);
  void icqChatRequestCancel(unsigned long nUin, unsigned long nSequence);
  // File Transfer
  unsigned long icqFileTransfer(unsigned long nUin, const char *szFilename,
     const char *szDescription, unsigned short nLevel);
  void icqFileTransferRefuse(unsigned long nUin, const char *szReason,
     unsigned long nSequence);
  void icqFileTransferCancel(unsigned long nUin, unsigned long nSequence);
  void icqFileTransferAccept(unsigned long nUin, unsigned short nPort,
     unsigned long nSequence);
  unsigned long icqOpenSecureChannel(unsigned long nUin);
  unsigned long icqCloseSecureChannel(unsigned long nUin);
  void icqOpenSecureChannelCancel(unsigned long nUin, unsigned long nSequence);

  // UDP (server) functions
  void icqRegister(const char *_szPasswd);
  unsigned long icqLogon(unsigned short logonStatus);
  unsigned long icqUserBasicInfo(unsigned long);
  unsigned long icqUserExtendedInfo(unsigned long);
  unsigned long icqRequestMetaInfo(unsigned long);

  unsigned long icqUpdateBasicInfo(const char *, const char *, const char *,
                                       const char *, bool);
  unsigned long icqUpdateExtendedInfo(const char *, unsigned short, const char *,
                                unsigned short, char, const char *,
                                const char *, const char *_sAbout, const char *);
  unsigned long icqSetWorkInfo(const char *_szCity, const char *_szState,
                           const char *_szPhone,
                           const char *_szFax, const char *_szAddress,
                           const char *_szName, const char *_szDepartment,
                           const char *_szPosition, const char *_szHomepage);
  unsigned long icqSetGeneralInfo(const char *szAlias, const char *szFirstName,
                              const char *szLastName, const char *szEmailPrimary,
                              const char *szEmailSecondary, const char *szEmailOld, const char *szCity,
                              const char *szState, const char *szPhoneNumber,
                              const char *szFaxNumber, const char *szAddress,
                              const char *szCellularNumber, const char *szZipCode,
                              unsigned short nCountryCode, bool bHideEmail);
  unsigned long icqSetMoreInfo(unsigned short nAge,
                           char nGender, const char *szHomepage,
                           unsigned short nBirthYear, char nBirthMonth,
                           char nBirthDay, char nLanguage1,
                           char nLanguage2, char nLanguage3);
  unsigned long icqSetSecurityInfo(bool bAuthorize, bool bHideIp, bool bWebAware);
  unsigned long icqSetAbout(const char *szAbout);
  unsigned long icqSetPassword(const char *szPassword);
  unsigned long icqSetStatus(unsigned short newStatus);
  unsigned long icqSetRandomChatGroup(unsigned long nGroup);
  unsigned long icqRandomChatSearch(unsigned long nGroup);
  unsigned long icqSearchByInfo(const char *, const char *, const char *, const char *);
  unsigned long icqSearchByUin(unsigned long);
  unsigned long icqSearchWhitePages(const char *szFirstName,
                            const char *szLastName, const char *szAlias,
                            const char *szEmail, unsigned short nMinAge,
                            unsigned short nMaxAge, char nGender,
                            char nLanguage, const char *szCity,
                            const char *szState, unsigned short nCountryCode,
                            const char *szCoName, const char *szCoDept,
                            const char *szCoPos, bool bOnlineOnly);

  void icqLogoff();
  void icqRelogon(bool bChangeServer = false);
  unsigned long icqAuthorizeGrant(unsigned long nUin, const char *szMessage);
  unsigned long icqAuthorizeRefuse(unsigned long nUin, const char *szMessage);
  void icqAlertUser(unsigned long _nUin);
  void icqAddUser(unsigned long);
  void icqUpdateContactList();

  // Visible/Invisible list functions
  void icqAddToVisibleList(unsigned long nUin);
  void icqRemoveFromVisibleList(unsigned long nUin);
  void icqToggleVisibleList(unsigned long nUin);
  void icqAddToInvisibleList(unsigned long nUin);
  void icqRemoveFromInvisibleList(unsigned long nUin);
  void icqToggleInvisibleList(unsigned long nUin);

  void PluginList(PluginsList &l);
  void PluginShutdown(int);
  void PluginEnable(int);
  void PluginDisable(int);
  bool PluginLoad(const char *, int, char **);

  void UpdateAllUsers();
  void UpdateAllUsersInGroup(GroupType, unsigned short);
  void SwitchServer();
  void CancelEvent(unsigned long );
  void CancelEvent(ICQEvent *);
  bool OpenConnectionToUser(unsigned long nUin, TCPSocket *sock,
     unsigned short nPort);
  bool OpenConnectionToUser(const char *szAlias, unsigned long nIp,
     unsigned long nRealIp, TCPSocket *sock, unsigned short nPort,
     bool bSendRealIp);
  int StartTCPServer(TCPSocket *);
  void CheckBirthdays(UinList &);
  unsigned short BirthdayRange() { return m_nBirthdayRange; }
  void BirthdayRange(unsigned short r) { m_nBirthdayRange = r; }

  bool AddUserToList(unsigned long _nUin);
  void AddUserToList(ICQUser *);
  void RemoveUserFromList(unsigned long _nUin);

  // NOT MT SAFE
  const char *getUrlViewer();
  unsigned short getDefaultRemotePort()  { return(m_nDefaultRemotePort); }
  void setDefaultRemotePort(unsigned short n)  { n == 0 ? m_nDefaultRemotePort = 4000 : m_nDefaultRemotePort = n; }
  void setUrlViewer(const char *s);

  bool ViewUrl(const char *url);

  // Firewall options
  bool TCPEnabled();
  void SetTCPEnabled(bool b);
  const char *FirewallHost()  { return m_szFirewallHost; }
  void SetFirewallHost(const char *);
  unsigned short TCPPortsLow() { return m_nTCPPortsLow; }
  unsigned short TCPPortsHigh() { return m_nTCPPortsHigh; }
  void SetTCPPorts(unsigned short p, unsigned short r);
  static bool SocksEnabled();
  const char *SocksServer()  {  return getenv("SOCKS5_SERVER"); }
  static bool CryptoEnabled();

  const char *Terminal();
  void SetTerminal(const char *s);
  bool Ignore(unsigned short n)      { return m_nIgnoreTypes & n; }
  void SetIgnore(unsigned short, bool);

  COnEventManager *OnEventManager()  { return &m_xOnEventManager; }
  bool AlwaysOnlineNotify();
  void SetAlwaysOnlineNotify(bool);
  CICQSignal *PopPluginSignal();
  ICQEvent *PopPluginEvent();

  ICQRemoteServers icqServers;

  // Statistics
  CDaemonStats *Stats(unsigned short n) { return n < 3 ? &m_sStats[n] : NULL; }
  DaemonStatsList &AllStats() { return m_sStats; }
  time_t ResetTime() { return m_nResetTime; }
  time_t StartTime() { return m_nStartTime; }
  time_t Uptime() { return time(NULL) - m_nStartTime; }
  void ResetStats();

protected:
  CLicq *licq;
  COnEventManager m_xOnEventManager;
  int pipe_newsocket[2], fifo_fd;
  FILE *fifo_fs;
  EDaemonStatus m_eStatus;
  char m_szConfigFile[MAX_FILENAME_LEN];

  char *m_szUrlViewer,
       *m_szTerminal,
       *m_szRejectFile,
       *m_szFirewallHost;
  unsigned long m_nDesiredStatus,
                m_nIgnoreTypes;
  unsigned short m_nTCPPortsLow,
                 m_nTCPPortsHigh,
                 m_nDefaultRemotePort,
                 m_nMaxUsersPerPacket,
                 m_nServerSequence,
                 m_nErrorTypes,
                 m_nBirthdayRange;
  char m_szErrorFile[64];
  int m_nUDPSocketDesc,
      m_nTCPSocketDesc;
  bool m_bShuttingDown,
       m_bLoggingOn,
       m_bOnlineNotifies,
       m_bAlwaysOnlineNotify;
  time_t m_tLogonTime;

  // Statistics
  void FlushStats();
  DaemonStatsList m_sStats;
  time_t m_nStartTime, m_nResetTime;

  list <ICQEvent *> m_lxRunningEvents;
  pthread_mutex_t mutex_runningevents;
  list <ICQEvent *> m_lxExtendedEvents;
  pthread_mutex_t mutex_extendedevents;
  pthread_t thread_monitorsockets,
            thread_ping;

  pthread_cond_t cond_serverack;
  pthread_mutex_t mutex_serverack;
  unsigned short m_nServerAck;

  void ChangeUserStatus(ICQUser *u, unsigned long s);
  bool AddUserEvent(ICQUser *, CUserEvent *);
  void RejectEvent(unsigned long, CUserEvent *);
  ICQUser *FindUserForInfoUpdate(unsigned long nUin, ICQEvent *e, const char *);

  void icqPing();
  void icqSendVisibleList();
  void icqSendInvisibleList();
  void icqRequestSystemMsg();
  void SaveUserList();

  void FailEvents(int sd, int err);
  ICQEvent *DoneEvent(ICQEvent *e, EventResult _eResult);
  ICQEvent *DoneEvent(int _nSD, unsigned long _nSequence, EventResult _eResult);
  ICQEvent *DoneEvent(unsigned long tag, EventResult _eResult);
  ICQEvent *DoneExtendedEvent(const unsigned short, const unsigned short, EventResult);
  ICQEvent *DoneExtendedEvent(ICQEvent *, EventResult);
  ICQEvent *DoneExtendedEvent(unsigned long tag, EventResult _eResult);
  void ProcessDoneEvent(ICQEvent *);
  void PushExtendedEvent(ICQEvent *);
  void PushPluginSignal(CICQSignal *);
  void PushPluginEvent(ICQEvent *);
  bool SendEvent(int nSD, CPacket &, bool);
  bool SendEvent(INetSocket *, CPacket &, bool);
  /*ICQEvent *SendExpectEvent(int _nSD, CPacket *packet, ConnectType _eConnect);
  ICQEvent *SendExpectEvent(int _nSD, CPacket *packet, ConnectType _eConnect,
                            unsigned long _nDestinationUin, CUserEvent *e);*/
  ICQEvent *SendExpectEvent_Server(CPacket *packet);
  ICQEvent *SendExpectEvent_Server(unsigned long nUin, CPacket *, CUserEvent *);
  ICQEvent *SendExpectEvent_Client(ICQUser *, CPacket *, CUserEvent *);
  ICQEvent *SendExpectEvent(ICQEvent *, void *(*fcn)(void *));
  void AckUDP(unsigned short, unsigned short, UDPSocket *);
  void AckTCP(CPacketTcp &, int);
  void AckTCP(CPacketTcp &, TCPSocket *);

  unsigned short ProcessUdpPacket(UDPSocket *, unsigned short = 0);
  void ProcessSystemMessage(CBuffer &packet, unsigned long checkUin, unsigned short newCommand, time_t timeSent);
  void ProcessMetaCommand(CBuffer &packet, unsigned short nMetaCommand, ICQEvent *e);
  bool ProcessTcpPacket(TCPSocket *);
  bool ProcessTcpHandshake(TCPSocket *);
  void ProcessFifo(char *);

  static bool Handshake_Send(TCPSocket *, unsigned long, unsigned short, unsigned short);
  static bool Handshake_Recv(TCPSocket *, unsigned short);
  int ConnectToServer();
  int ConnectToUser(unsigned long);
  int ReverseConnectToUser(unsigned long nUin, unsigned long nUin,
     unsigned short nPort, unsigned short nVersion, unsigned short nFailedPort);

  void StupidChatLinkageFix();

  // Declare all our thread functions as friends
  friend void *Ping_tep(void *p);
  friend void *MonitorSockets_tep(void *p);
  friend void *ReverseConnectToUser_tep(void *p);
  friend void *ProcessRunningEvent_Client_tep(void *p);
  friend void *ProcessRunningEvent_Server_tep(void *p);
  friend void *Shutdown_tep(void *p);
  friend class ICQUser;
  friend class CSocketManager;
  friend class CChatManager;
  friend class CFileTransferManager;
  friend class COnEventManager;
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
  CReverseConnectToUserData(unsigned long uin, unsigned long ip,
   unsigned short port, unsigned short version, unsigned short failedport) :
   nUin(uin), nIp(ip), nPort(port), nFailedPort(failedport),
   nVersion(version) {}

  unsigned long nUin;
  unsigned long nIp;
  unsigned short nPort;
  unsigned short nFailedPort;
  unsigned short nVersion;
};



#endif

