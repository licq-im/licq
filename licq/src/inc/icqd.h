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
#include <pthread.h>
#include <stdio.h>

#include "socket.h"
#include "icqevent.h"
#include "remoteserver.h"
#include "onevent.h"

class CPlugin;
class CPacket;
class CPacketTcp;
class CLicq;
class ICQUser;

class CPluginFunctions
{
public:
  const char *Name(void)    { return (*fName)(); }
  const char *Version(void) { return (*fVersion)(); }
  unsigned short Id(void)   { return *nId; }

protected:
  const char *(*fName)(void);
  const char *(*fVersion)(void);
  void (*fUsage)(void);
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
  ~CICQDaemon(void);
  int RegisterPlugin(unsigned long _nSignalMask);
  void UnregisterPlugin(void);
  bool Start(void);
  const char *Version(void);
  pthread_t *Shutdown(void);
  void SaveConf(void);

  // TCP (user) functions
  ICQEvent *icqSendMessage(unsigned long _nUin, const char *m, bool online, bool _bUrgent, unsigned long id = 0);
  ICQEvent *icqSendUrl(unsigned long _nUin, const char *url, const char *description, bool online, bool _bUrgent, unsigned long id = 0);
  ICQEvent *icqFetchAutoResponse(unsigned long _nUin, unsigned long id = 0);
  ICQEvent *icqChatRequest(unsigned long _nUin, const char *reason, bool online, bool _bUrgent, unsigned long id = 0);
  ICQEvent *icqFileTransfer(unsigned long _nUin, const char *_szFilename, const char *_szDescription, bool online, bool _bUrgent, unsigned long id = 0);
  void icqFileTransferRefuse(unsigned long _nUin, const char *reason, unsigned long theSequence);
  void icqFileTransferCancel(unsigned long _nUin, unsigned long seq);
  void icqFileTransferAccept(unsigned long _nUin, unsigned short thePort, unsigned long theSequence);
  void icqChatRequestRefuse(unsigned long _nUin, const char *reason, unsigned long theSequence);
  void icqChatRequestAccept(unsigned long _nUin, unsigned short thePort, unsigned long theSequence);
  void icqChatRequestCancel(unsigned long _nUin, unsigned long seq);

  // UDP (server) functions
  void icqRegister(const char *_szPasswd);
  ICQEvent *icqLogon(unsigned long logonStatus);
  ICQEvent *icqUserBasicInfo(unsigned long);
  ICQEvent *icqUserExtendedInfo(unsigned long);
  ICQEvent *icqUpdateBasicInfo(const char *, const char *, const char *,
                                       const char *, bool);
  ICQEvent *icqUpdateExtendedInfo(const char *, unsigned short, const char *,
                                unsigned short, char, const char *,
                                const char *, const char *_sAbout, unsigned long);
  unsigned short icqStartSearch(const char *, const char *, const char *, const char *);
  void icqLogoff(void);
  ICQEvent *icqSetStatus(unsigned long newStatus);
  void icqAuthorize(unsigned long uinToAuthorize);
  void icqAlertUser(unsigned long _nUin);
  void icqAddUser(unsigned long);
  void icqUpdateContactList(void);

  void icqPing(void);
  void icqRelogon(void);
  void icqSendVisibleList(bool _bSendIfEmpty = false);
  void icqSendInvisibleList(bool _bSendIfEmpty = false);
  void icqRequestSystemMsg(void);

  void PluginList(PluginsList &l);

  void UpdateAllUsers();
  void SwitchServer(void);
  void CancelEvent(ICQEvent *);

  void AddUserToList(unsigned long _nUin);
  void AddUserToList(ICQUser *);
  void RemoveUserFromList(unsigned long _nUin);
  void SaveUserList(void);

  // NOT MT SAFE
  unsigned short getMaxUsersPerPacket(void)  { return m_nMaxUsersPerPacket; }
  bool getTcpPort(unsigned short);
  void setTcpPort(unsigned short, bool);
  const char *getUrlViewer(void);
  unsigned short getTcpServerPort(void) { return(m_nTcpServerPort); }
  unsigned short getDefaultRemotePort(void)  { return(m_nDefaultRemotePort); }
  void setTcpServerPort(unsigned short n)  { m_nTcpServerPort = n; }
  void setDefaultRemotePort(unsigned short n)  { m_nDefaultRemotePort = n; }
  void setMaxUsersPerPacket(unsigned short n)  { m_nMaxUsersPerPacket = n; }
  void setUrlViewer(const char *s);
  const char *Terminal(void);
  void SetTerminal(const char *s);
  bool Ignore(unsigned short n)      { return m_nIgnoreTypes & n; }
  void SetIgnore(unsigned short, bool);

  COnEventManager *OnEventManager(void)  { return &m_xOnEventManager; }
  CICQSignal *PopPluginSignal(void);
  ICQEvent *PopPluginEvent(void);

  ICQRemoteServers icqServers;

protected:
  CLicq *licq;
  vector<bool> m_vbTcpPorts;
  COnEventManager m_xOnEventManager;
  vector<CPlugin *> m_vPlugins;
  pthread_mutex_t mutex_plugins;
  int pipe_newsocket[2], fifo_fd;
  FILE *fifo_fs;
  EDaemonStatus m_eStatus;

  char *m_szUrlViewer,
       *m_szTerminal,
       *m_szRejectFile;
  unsigned long m_nDesiredStatus,
                m_nIgnoreTypes;
  unsigned short m_nAllowUpdateUsers,
                 m_nTcpServerPort,
                 m_nDefaultRemotePort,
                 m_nMaxUsersPerPacket;
  char m_szErrorFile[64];
  int m_nUDPSocketDesc,
      m_nTCPSocketDesc;
  bool m_bShuttingDown;
  time_t m_tLogonTime;

  list <ICQEvent *> m_lxRunningEvents;
  pthread_mutex_t mutex_runningevents;
  list <ICQEvent *> m_lxExtendedEvents;
  pthread_mutex_t mutex_extendedevents;
  pthread_t thread_monitorsockets,
            thread_ping;

  void ParseFE(char *szBuffer, char ***szSubStr, int nMaxSubStr);
  void ChangeUserStatus(ICQUser *u, unsigned long s);
  bool AddUserEvent(ICQUser *u, CUserEvent *e);
  void RejectEvent(unsigned long, CUserEvent *);

  ICQEvent *DoneEvent(ICQEvent *e, EEventResult _eResult);
  ICQEvent *DoneEvent(int _nSD, unsigned long _nSequence, EEventResult _eResult);
  ICQEvent *DoneExtendedEvent(const unsigned short, const unsigned short, EEventResult);
  ICQEvent *DoneExtendedEvent(ICQEvent *, EEventResult);
  void ProcessDoneEvent(ICQEvent *e);
  void PushExtendedEvent(ICQEvent *e);
  void PushPluginSignal(CICQSignal *s);
  void PushPluginEvent(ICQEvent *e);
  void SendEvent(int _nSD, CPacket &p);
  ICQEvent *SendExpectEvent(int _nSD, CPacket *packet, EConnect _eConnect);
  ICQEvent *SendExpectEvent(int _nSD, CPacket *packet, EConnect _eConnect,
                            unsigned long _nDestinationUin, CUserEvent *e);
  void AckUDP(unsigned short, unsigned short);
  void AckTCP(CPacketTcp &, int _nSD);

  unsigned short ProcessUdpPacket(CBuffer &packet, bool = false);
  void ProcessSystemMessage(CBuffer &packet, unsigned long checkUin, unsigned short newCommand, time_t timeSent);
  bool ProcessTcpPacket(CBuffer &packet, int sockfd);
  bool ProcessTcpHandshake(TCPSocket *);
  void ProcessFifo(char *);

  int ConnectToServer(void);
  int ConnectToUser(unsigned long);

  // Declare all our thread functions as friends
  friend void *Ping_tep(void *p);
  friend void *MonitorSockets_tep(void *p);
  friend void *ProcessRunningEvent_tep(void *p);
  friend void *Shutdown_tep(void *p);
};


#endif

