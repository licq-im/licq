/*
ICQ.H
header file containing all the main procedures to interface with the ICQ server at mirabilis
*/

#ifndef ICQD_H
#define ICQD_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector.h>
#include <list.h>
#include <deque.h>
#include <stdarg.h>
#include <stdio.h>

#include "licq_socket.h"
#include "licq_events.h"
#include "licq_remoteserver.h"
#include "licq_onevent.h"

class CPlugin;
class CPacket;
class CPacketTcp;
class CLicq;
class ICQUser;
class CICQEventTag;

class CPluginFunctions
{
public:
  const char *Name()    { return (*fName)(); }
  const char *Version() { return (*fVersion)(); }
  const char *Description() { return (*fDescription)(); }
  const char *Status() { return (*fStatus)(); }
  const char *Usage() { return (*fUsage)(); }
  const char *BuildDate() { return (*fBuildDate)(); }
  const char *BuildTime() { return (*fBuildTime)(); }
  unsigned short Id()   { return *nId; }

protected:
  const char *(*fName)();
  const char *(*fVersion)();
  const char *(*fStatus)();
  const char *(*fDescription)();
  const char *(*fBuildDate)();
  const char *(*fBuildTime)();
  const char *(*fUsage)();
  bool (*fInit)(int, char **);
  int (*fMain)(CICQDaemon *);
  void *(*fMain_tep)(void *);
  unsigned short *nId;

  void *dl_handle;
  pthread_t thread_plugin;

friend class CLicq;
friend class CICQDaemon;
};

typedef list<CPluginFunctions *> PluginsList;
typedef list<CPluginFunctions *>::iterator PluginsListIter;

const unsigned short IGNORE_MASSMSG    = 1;
const unsigned short IGNORE_NEWUSERS   = 2;
const unsigned short IGNORE_EMAILPAGER = 4;
const unsigned short IGNORE_WEBPANEL   = 8;


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
  CICQEventTag *icqSendMessage(unsigned long nUin, const char *szMessage,
     bool bOnline, unsigned short nLevel, unsigned long nSpoofUin = 0);
  CICQEventTag *icqSendUrl(unsigned long nUin, const char *szUrl,
     const char *szDescription, bool bOnline, unsigned short nLevel,
     unsigned long nSpoofUin = 0);
  CICQEventTag *icqFetchAutoResponse(unsigned long nUin);
  CICQEventTag *icqChatRequest(unsigned long nUin, const char *szReason,
     unsigned short nLevel);
  CICQEventTag *icqMultiPartyChatRequest(unsigned long nUin,
     const char *szReason, const char *szChatUsers, unsigned short nPort,
     unsigned short nLevel);
  CICQEventTag *icqFileTransfer(unsigned long nUin, const char *szFilename,
     const char *szDescription, unsigned short nLevel);
  void icqFileTransferRefuse(unsigned long nUin, const char *szReason,
     unsigned long nSequence);
  void icqFileTransferCancel(unsigned long nUin, unsigned long nSequence);
  void icqFileTransferAccept(unsigned long nUin, unsigned short nPort,
     unsigned long nSequence);
  void icqChatRequestRefuse(unsigned long nUin, const char *szReason,
     unsigned long nSequence);
  void icqChatRequestAccept(unsigned long nUin, unsigned short nPort,
     unsigned long nSequence);
  void icqChatRequestCancel(unsigned long nUin, unsigned long nSequence);

  // UDP (server) functions
  void icqRegister(const char *_szPasswd);
  CICQEventTag *icqLogon(unsigned short logonStatus);
  CICQEventTag *icqUserBasicInfo(unsigned long);
  CICQEventTag *icqUserExtendedInfo(unsigned long);
  CICQEventTag *icqRequestMetaInfo(unsigned long);

  CICQEventTag *icqUpdateBasicInfo(const char *, const char *, const char *,
                                       const char *, bool);
  CICQEventTag *icqUpdateExtendedInfo(const char *, unsigned short, const char *,
                                unsigned short, char, const char *,
                                const char *, const char *_sAbout, unsigned long);
  CICQEventTag *icqSetWorkInfo(const char *_szCity, const char *_szState,
                           const char *_szPhone,
                           const char *_szFax, const char *_szAddress,
                           const char *_szName, const char *_szDepartment,
                           const char *_szPosition, const char *_szHomepage);
  CICQEventTag *icqSetGeneralInfo(const char *szAlias, const char *szFirstName,
                              const char *szLastName, const char *szEmail1,
                              const char *szEmail2, const char *szCity,
                              const char *szState, const char *szPhoneNumber,
                              const char *szFaxNumber, const char *szAddress,
                              const char *szCellularNumber, unsigned long nZipCode,
                              unsigned short nCountryCode, bool bHideEmail);
  CICQEventTag *icqSetMoreInfo(unsigned short nAge,
                           char nGender, const char *szHomepage,
                           char nBirthYear, char nBirthMonth,
                           char nBirthDay, char nLanguage1,
                           char nLanguage2, char nLanguage3);
  CICQEventTag *icqSetSecurityInfo(bool bAuthorize, bool bHideIp, bool bWebAware);
  CICQEventTag *icqSetAbout(const char *szAbout);
  CICQEventTag *icqSetPassword(const char *szPassword);
  CICQEventTag *icqSetStatus(unsigned short newStatus);
  CICQEventTag *icqSetRandomChatGroup(unsigned long nGroup);
  CICQEventTag *icqRandomChatSearch(unsigned long nGroup);

  unsigned short icqSearchByInfo(const char *, const char *, const char *, const char *);
  unsigned short icqSearchByUin(unsigned long);
  void icqLogoff();
  void icqAuthorize(unsigned long uinToAuthorize);
  void icqAlertUser(unsigned long _nUin);
  void icqAddUser(unsigned long);
  void icqUpdateContactList();

  void icqPing();
  void icqRelogon(bool bChangeServer = false);
  void icqSendVisibleList(bool _bSendIfEmpty = false);
  void icqSendInvisibleList(bool _bSendIfEmpty = false);
  void icqRequestSystemMsg();

  void PluginList(PluginsList &l);
  void PluginShutdown(int);
  void PluginEnable(int);
  void PluginDisable(int);
  bool PluginLoad(const char *, int, char **);

  void UpdateAllUsers();
  void SwitchServer();
  void CancelEvent(CICQEventTag *);
  bool OpenConnectionToUser(unsigned long nUin, TCPSocket *sock,
     unsigned short nPort);
  bool OpenConnectionToUser(const char *szAlias, unsigned long nIp,
     unsigned long nRealIp, TCPSocket *sock, unsigned short nPort);
  int StartTCPServer(TCPSocket *);

  void AddUserToList(unsigned long _nUin);
  void AddUserToList(ICQUser *);
  void RemoveUserFromList(unsigned long _nUin);
  void SaveUserList();

  // NOT MT SAFE
  const char *getUrlViewer();
  unsigned short getDefaultRemotePort()  { return(m_nDefaultRemotePort); }
  void setDefaultRemotePort(unsigned short n)  { n == 0 ? m_nDefaultRemotePort = 4000 : m_nDefaultRemotePort = n; }
  void setUrlViewer(const char *s);

  // Firewall options
  unsigned short TCPEnabled();
  void SetTCPEnabled(bool b);
  const char *FirewallHost()  { return m_szFirewallHost; }
  void SetFirewallHost(const char *);
  unsigned short TCPBasePort() { return m_nTCPBasePort; }
  unsigned short TCPBaseRange() { return m_nTCPBaseRange; }
  void SetTCPBasePort(unsigned short p, unsigned short r);
  bool SocksEnabled();
  const char *SocksServer()  {  return getenv("SOCKS5_SERVER"); }

  const char *Terminal();
  void SetTerminal(const char *s);
  bool Ignore(unsigned short n)      { return m_nIgnoreTypes & n; }
  void SetIgnore(unsigned short, bool);
  unsigned long StringToStatus(char *_szStatus);

  COnEventManager *OnEventManager()  { return &m_xOnEventManager; }
  CICQSignal *PopPluginSignal();
  ICQEvent *PopPluginEvent();

  ICQRemoteServers icqServers;

protected:
  CLicq *licq;
  COnEventManager m_xOnEventManager;
  vector<CPlugin *> m_vPlugins;
  pthread_mutex_t mutex_plugins;
  int pipe_newsocket[2], fifo_fd;
  FILE *fifo_fs;
  EDaemonStatus m_eStatus;

  char *m_szUrlViewer,
       *m_szTerminal,
       *m_szRejectFile,
       *m_szFirewallHost;
  unsigned long m_nDesiredStatus,
                m_nIgnoreTypes;
  unsigned short m_nTCPBasePort,
                 m_nTCPBaseRange,
                 m_nDefaultRemotePort,
                 m_nMaxUsersPerPacket,
                 m_nServerSequence,
                 m_nErrorTypes;
  char m_szErrorFile[64];
  int m_nUDPSocketDesc,
      m_nTCPSocketDesc;
  bool m_bShuttingDown, m_bLoggingOn;
  time_t m_tLogonTime;

  list <ICQEvent *> m_lxRunningEvents;
  pthread_mutex_t mutex_runningevents;
  list <ICQEvent *> m_lxExtendedEvents;
  pthread_mutex_t mutex_extendedevents;
  pthread_t thread_monitorsockets,
            thread_ping;

  pthread_cond_t cond_serverack;
  pthread_mutex_t mutex_serverack;
  unsigned short m_nServerAck;

  bool ParseFE(char *szBuffer, char ***szSubStr, int nMaxSubStr);
  void ChangeUserStatus(ICQUser *u, unsigned long s);
  bool AddUserEvent(ICQUser *, CUserEvent *);
  void RemoveUserEvent(ICQUser *, unsigned long);
  void RejectEvent(unsigned long, CUserEvent *);

  ICQEvent *DoneEvent(ICQEvent *e, EventResult _eResult);
  ICQEvent *DoneEvent(int _nSD, unsigned long _nSequence, EventResult _eResult);
  ICQEvent *DoneEvent(CICQEventTag *tag, EventResult _eResult);
  ICQEvent *DoneExtendedEvent(const unsigned short, const unsigned short, EventResult);
  ICQEvent *DoneExtendedEvent(ICQEvent *, EventResult);
  ICQEvent *DoneExtendedEvent(CICQEventTag *tag, EventResult _eResult);
  void ProcessDoneEvent(ICQEvent *e);
  void PushExtendedEvent(ICQEvent *e);
  void PushPluginSignal(CICQSignal *s);
  void PushPluginEvent(ICQEvent *e);
  bool SendEvent(int _nSD, CPacket &p);
  ICQEvent *SendExpectEvent(int _nSD, CPacket *packet, ConnectType _eConnect);
  ICQEvent *SendExpectEvent(int _nSD, CPacket *packet, ConnectType _eConnect,
                            unsigned long _nDestinationUin, CUserEvent *e);
  void AckUDP(unsigned short, unsigned short);
  void AckTCP(CPacketTcp &, int _nSD);

  unsigned short ProcessUdpPacket(CBuffer &packet, unsigned short = 0);
  void ProcessSystemMessage(CBuffer &packet, unsigned long checkUin, unsigned short newCommand, time_t timeSent);
  void ProcessMetaCommand(CBuffer &packet, unsigned short nMetaCommand, ICQEvent *e);
  bool ProcessTcpPacket(CBuffer &packet, int sockfd);
  bool ProcessTcpHandshake(TCPSocket *);
  void ProcessFifo(char *);

  int ConnectToServer();
  int ConnectToUser(unsigned long);
  int ReverseConnectToUser(unsigned long nUin, unsigned long nUin,
                           unsigned short nPort);

  // Declare all our thread functions as friends
  friend void *Ping_tep(void *p);
  friend void *MonitorSockets_tep(void *p);
  friend void *ProcessRunningEvent_tep(void *p);
  friend void *Shutdown_tep(void *p);
  friend class ICQUser;
};


#endif

