#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include "time-fix.h"

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#include "icq-defines.h"
#include "user.h"
#include "constants.h"
#include "file.h"
#include "log.h"
#include "translate.h"
#include "plugind.h"
#include "icqpacket.h"
#include "licq.h"
#include "support.h"

#include "icqd.h"
#include "icq-udp.h"
#include "icq-tcp.h"
#include "icq-threads.h"

//-----CICQDaemon::constructor--------------------------------------------------
CICQDaemon::CICQDaemon(CLicq *_licq)
{
  char szFilename[MAX_FILENAME_LEN];

  licq = _licq;

  // Initialise the data values
  m_nIgnoreTypes = 0;
  m_nUDPSocketDesc = -1;
  m_nTCPSocketDesc = -1;
  m_eStatus = STATUS_OFFLINE_MANUAL;
  m_bShuttingDown = false;
  m_nServerAck = 0;
  m_szFirewallHost = NULL;

  // Begin parsing the config file
  sprintf(szFilename, "%s/%s", BASE_DIR, "licq.conf");
  CIniFile licqConf(INI_FxERROR | INI_FxFATAL);
  licqConf.LoadFile(szFilename);
  licqConf.ClearFlag(INI_FxFATAL | INI_FxERROR);
  //licqConf.SetFlag(INI_FxWARN);

  // -----Network configuration-----
  gLog.Info("%sNetwork configuration.\n", L_INITxSTR);

  unsigned short numRemoteServers, remoteServerPort;
  char remoteServerID[32], remotePortID[32], remoteServerName[64];

  licqConf.SetSection("network");
  licqConf.ReadNum("NumOfServers", numRemoteServers, 1);

  // Check for a default server port - if it's not there, we set it to the internal default
  licqConf.ReadNum("DefaultServerPort", m_nDefaultRemotePort, DEFAULT_SERVER_PORT);

  // read in all the servers
  for(int i = 0; i < numRemoteServers; i++)
  {
     sprintf(remoteServerID, "Server%d", i + 1);
     sprintf(remotePortID, "ServerPort%d", i + 1);
     if (!licqConf.ReadStr(remoteServerID, remoteServerName)) continue;
     licqConf.ClearFlag(INI_FxWARN);
     licqConf.ReadNum(remotePortID, remoteServerPort, getDefaultRemotePort());
     licqConf.SetFlag(INI_FxWARN);
     icqServers.addServer(remoteServerName, remoteServerPort);
  }

  bool bTcpEnabled;
  unsigned short nTCPBasePort, nTCPBaseRange;
  licqConf.ReadNum("TCPServerPort", nTCPBasePort, 0);
  licqConf.ReadNum("TCPServerPortRange", nTCPBaseRange, 10);
  SetTCPBasePort(nTCPBasePort, nTCPBaseRange);
  licqConf.ReadBool("TCPEnabled", bTcpEnabled, true);
  SetTCPEnabled(bTcpEnabled);
  licqConf.ReadNum("MaxUsersPerPacket", m_nMaxUsersPerPacket, 100);
  licqConf.ReadNum("IgnoreTypes", m_nIgnoreTypes, 0);
  licqConf.ReadStr("FirewallHost", szFilename, "");
  SetFirewallHost(szFilename);

  // Rejects log file
  licqConf.ReadStr("Rejects", szFilename, "log.rejects");
  if (strcmp(szFilename, "none") != 0)
  {
    m_szRejectFile = new char[256];
    sprintf(m_szRejectFile, "%s/%s", BASE_DIR, szFilename);
  }
  else
    m_szRejectFile = NULL;

  // Error log file
  licqConf.ReadStr("Errors", m_szErrorFile, "log.errors");
  if (strcmp(m_szErrorFile, "none") != 0)
  {
    sprintf(szFilename, "%s/%s", BASE_DIR, m_szErrorFile);
    CLogService_File *l = new CLogService_File(L_ERROR | L_UNKNOWN);
    if (!l->SetLogFile(szFilename, "a"))
    {
      gLog.Error("%sUnable to open %s as error log:\n%s%s.\n",
                  L_ERRORxSTR, szFilename, L_BLANKxSTR, strerror(errno));
      delete l;
    }
    else
      gLog.AddService(l);
  }

  // Loading translation table from file
  licqConf.ReadStr("Translation", szFilename, "none");
  if (strncmp(szFilename, "none", 4) != 0)
  {
     char TranslationTableFileNameFull[MAX_FILENAME_LEN];
     sprintf(TranslationTableFileNameFull, "%s%s/%s", SHARE_DIR, TRANSLATION_DIR, szFilename);
     gTranslator.setTranslationMap (TranslationTableFileNameFull);
  }

  // Url viewer
  m_szUrlViewer = NULL;
  licqConf.ReadStr("UrlViewer", szFilename, "none");
  m_szUrlViewer = new char[strlen(szFilename) + 1];
  strcpy(m_szUrlViewer, szFilename);

  // Terminal
  m_szTerminal = NULL;
  licqConf.ReadStr("Terminal", szFilename, "xterm -T Licq -e ");
  m_szTerminal = new char[strlen(szFilename) + 1];
  strcpy(m_szTerminal, szFilename);

  // -----OnEvent configuration-----
  char szOnEventCommand[MAX_FILENAME_LEN], *szOnParams[MAX_ON_EVENT];
  unsigned short nOnEventCmdType;

  gLog.Info("%sOnEvent configuration.\n", L_INITxSTR);
  licqConf.SetSection("onevent");
  licqConf.ReadNum("Enable", nOnEventCmdType, 0);
  m_xOnEventManager.SetCommandType(nOnEventCmdType);
  for (int i = 0; i < MAX_ON_EVENT; i++)
    szOnParams[i] = new char[MAX_FILENAME_LEN];
  licqConf.ReadStr("Command", szOnEventCommand, "play");
  licqConf.ReadStr("Message", szOnParams[ON_EVENT_MSG], "");
  licqConf.ReadStr("Url", szOnParams[ON_EVENT_URL], "");
  licqConf.ReadStr("Chat", szOnParams[ON_EVENT_CHAT], "");
  licqConf.ReadStr("File", szOnParams[ON_EVENT_FILE], "");
  licqConf.ReadStr("OnlineNotify", szOnParams[ON_EVENT_NOTIFY], "");
  licqConf.ReadStr("SysMsg", szOnParams[ON_EVENT_SYSMSG], "");
  m_xOnEventManager.SetParameters(szOnEventCommand, (const char **)szOnParams);
  for (int i = 0; i < MAX_ON_EVENT; i++)
    delete [] szOnParams[i];

  icqServers.setServer(1);    // set the initial UDP remote server (opened in ConnectToServer)

  // Pipes
  gLog.Info("%sCreating pipes.\n", L_INITxSTR);
  pipe(pipe_newsocket);

  // Initialize the random number generator
  srand(time(NULL));

  // Start up our threads
  gLog.Info("%sInitializing thread data.\n", L_INITxSTR);
  pthread_mutex_init(&mutex_runningevents, NULL);
  pthread_mutex_init(&mutex_extendedevents, NULL);
  pthread_mutex_init(&mutex_plugins, NULL);
  pthread_cond_init(&cond_serverack, NULL);
  pthread_mutex_init(&mutex_serverack, NULL);
}


bool CICQDaemon::Start()
{
  char sz[MAX_FILENAME_LEN];
  int nResult = 0;

  TCPSocket *s = new TCPSocket(0);
  m_nTCPSocketDesc = StartTCPServer(s);
  if (m_nTCPSocketDesc == -1)
  {
     gLog.Error("%sUnable to allocate TCP port for local server (%s)!\n",
                L_ERRORxSTR, "No ports available");
     return false;
  }
  gSocketManager.AddSocket(s);
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->SetIpPort(s->LocalIp(), s->LocalPort());
  gUserManager.DropOwner();
  gSocketManager.DropSocket(s);


#ifdef USE_FIFO
  // Open the fifo
  sprintf(sz, "%s/licq_fifo", BASE_DIR);
  gLog.Info("%sOpening fifo.\n", L_INITxSTR);
  fifo_fd = open(sz, O_RDWR);
  if (fifo_fd == -1)
  {
    if (mkfifo(sz, 00600) == -1)
      gLog.Warn("%sUnable to create fifo:\n%s%s.\n", L_WARNxSTR, L_BLANKxSTR, strerror(errno));
    else
    {
      fifo_fd = open(sz, O_RDWR);
      if (fifo_fd == -1)
        gLog.Warn("%sUnable to open fifo:\n%s%s.\n", L_WARNxSTR, L_BLANKxSTR, strerror(errno));
    }
  }
  fifo_fs = NULL;
  if (fifo_fd != -1)
  {
    struct stat buf;
    fstat(fifo_fd, &buf);
    if (!S_ISFIFO(buf.st_mode))
    {
      gLog.Warn("%s%s is not a FIFO, disabling fifo support.\n", L_WARNxSTR, sz);
      close(fifo_fd);
      fifo_fd = -1;
    }
    else
      fifo_fs = fdopen(fifo_fd, "r");
  }
#else
  fifo_fs = NULL;
  fifo_fd = -1;
#endif

  gLog.Info("%sSpawning daemon threads.\n", L_INITxSTR);
  nResult = pthread_create(&thread_monitorsockets, NULL, &MonitorSockets_tep, this);
  if (nResult != 0)
  {
    gLog.Error("%sUnable to start socket monitor thread:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
    return false;
  }
  nResult = pthread_create(&thread_ping, NULL, &Ping_tep, this);
  if (nResult != 0)
  {
    gLog.Error("%sUnable to start ping thread:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
    return false;
  }
  return true;
}

bool CICQDaemon::SocksEnabled()
{
#ifdef USE_SOCKS5
  return true;
#else
  return false;
#endif
}

/*------------------------------------------------------------------------------
 * RegisterPlugin
 *
 * Registers the current thread as a new plugin.  Returns the pipe to listen
 * on for notification.
 *----------------------------------------------------------------------------*/
int CICQDaemon::RegisterPlugin(unsigned long _nSignalMask)
{
  PluginsListIter it;
  bool found = false;

  pthread_mutex_lock(&licq->mutex_pluginfunctions);
  CPlugin *p = new CPlugin(_nSignalMask);
  for (it = licq->m_vPluginFunctions.begin();
       it != licq->m_vPluginFunctions.end();
       it++)
  {
    if (p->CompareThread((*it)->thread_plugin))
    {
      p->SetId((*it)->Id());
      found = true;
      break;
    }
  }
  pthread_mutex_unlock(&licq->mutex_pluginfunctions);

  if (!found)
  {
    gLog.Error("%sInvalid thread in registration attempt.\n", L_ERRORxSTR);
    delete p;
    return -1;
  }

  pthread_mutex_lock(&mutex_plugins);
  m_vPlugins.push_back(p);
  pthread_mutex_unlock(&mutex_plugins);
  int n = p->Pipe();
  return n;
}


/*------------------------------------------------------------------------------
 * UnregisterPlugin
 *
 * Unregisters the current plugin thread.
 *----------------------------------------------------------------------------*/
void CICQDaemon::UnregisterPlugin()
{
  vector<CPlugin *>::iterator iter;
  pthread_mutex_lock(&mutex_plugins);
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    if ((*iter)->CompareThread(pthread_self()))
    {
      //gLog.Info("%sUnregistering plugin %d.\n", L_ENDxSTR, (*iter)->Id());
      delete *iter;
      m_vPlugins.erase(iter);
      break;
    }
  }
  pthread_mutex_unlock(&mutex_plugins);
}


/*------------------------------------------------------------------------------
 * PluginList
 *
 * Fetches the list of plugins.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PluginList(PluginsList &lPlugins)
{
  unsigned short nId;
  vector<CPlugin *>::iterator iter;
  PluginsListIter it;
  lPlugins.clear();
  pthread_mutex_lock(&mutex_plugins);
  // Go through our list of registered plugins
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    nId = (*iter)->Id();
    // Cross reference with the master list
    pthread_mutex_lock(&licq->mutex_pluginfunctions);
    for (it = licq->m_vPluginFunctions.begin();
         it != licq->m_vPluginFunctions.end();
         it++)
    {
      if (nId == (*it)->Id())
      {
        lPlugins.push_back(*it);
        break;
      }
    }
    pthread_mutex_unlock(&licq->mutex_pluginfunctions);
  }
  pthread_mutex_unlock(&mutex_plugins);
}

/*------------------------------------------------------------------------------
 * PluginShutdown
 *
 * Unloads the given plugin.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PluginShutdown(int id)
{
  pthread_mutex_lock(&mutex_plugins);
  vector<CPlugin *>::iterator iter;
  // Go through our list of registered plugins
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    if (id == (*iter)->Id()) (*iter)->Shutdown();
  }
  pthread_mutex_unlock(&mutex_plugins);
}

/*------------------------------------------------------------------------------
 * PluginDisable
 *
 * Disables the given plugin.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PluginDisable(int id)
{
  pthread_mutex_lock(&mutex_plugins);
  vector<CPlugin *>::iterator iter;
  // Go through our list of registered plugins
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    if (id == (*iter)->Id()) (*iter)->Disable();
  }
  pthread_mutex_unlock(&mutex_plugins);
}

/*------------------------------------------------------------------------------
 * PluginEnable
 *
 * Enableds the given plugin.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PluginEnable(int id)
{
  pthread_mutex_lock(&mutex_plugins);
  vector<CPlugin *>::iterator iter;
  // Go through our list of registered plugins
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    if (id == (*iter)->Id()) (*iter)->Enable();
  }
  pthread_mutex_unlock(&mutex_plugins);
}



/*------------------------------------------------------------------------------
 * PluginLoad
 *
 * Loads the given plugin.
 *----------------------------------------------------------------------------*/
bool CICQDaemon::PluginLoad(const char *szPlugin, int argc, char **argv)
{
  optind = 0;
  CPluginFunctions *p = licq->LoadPlugin(szPlugin, argc, argv);

  if (p == NULL) return false;

  pthread_mutex_lock(&licq->mutex_pluginfunctions);
  licq->StartPlugin(p);
  pthread_mutex_unlock(&licq->mutex_pluginfunctions);
  return true;
}



const char *CICQDaemon::Version()
{
  return licq->Version();
}

//-----ICQ::destructor----------------------------------------------------------
CICQDaemon::~CICQDaemon()
{
  if (m_szUrlViewer != NULL) delete [] m_szUrlViewer;
  if (m_szRejectFile != NULL) delete [] m_szRejectFile;
}

pthread_t *CICQDaemon::Shutdown()
{
  static pthread_t *thread_shutdown = (pthread_t *)malloc(sizeof(pthread_t));
  if (m_bShuttingDown) return(thread_shutdown);
  // Small race condition here if multiple plugins call shutdown at the same time
  m_bShuttingDown = true;
  SaveUserList();
  pthread_create (thread_shutdown, NULL, &Shutdown_tep, this);
  return (thread_shutdown);
}


//-----SaveConf-----------------------------------------------------------------
void CICQDaemon::SaveConf()
{
  char filename[MAX_FILENAME_LEN];
  sprintf(filename, "%s/licq.conf", BASE_DIR);
  CIniFile licqConf(INI_FxERROR | INI_FxALLOWxCREATE);
  if (!licqConf.LoadFile(filename)) return;

  licqConf.SetSection("network");
  licqConf.WriteNum("DefaultServerPort", getDefaultRemotePort());
  licqConf.WriteNum("TCPServerPort", m_nTCPBasePort);
  licqConf.WriteNum("TCPServerPortRange", m_nTCPBaseRange);
  licqConf.WriteBool("TCPEnabled", CPacket::Mode() == MODE_DIRECT);
  licqConf.WriteNum("MaxUsersPerPacket", m_nMaxUsersPerPacket);
  licqConf.WriteNum("IgnoreTypes", m_nIgnoreTypes);
  licqConf.WriteStr("FirewallHost", m_szFirewallHost);

  // Utility tab
  licqConf.WriteStr("UrlViewer", m_szUrlViewer);
  const char *pc = strrchr(gTranslator.getMapName(), '/');
  if (pc != NULL)
    pc++;
  else
    pc = gTranslator.getMapName();
  licqConf.WriteStr("Translation", pc);
  licqConf.WriteStr("Terminal", m_szTerminal);
  licqConf.WriteStr("Errors", m_szErrorFile);
  if (m_szRejectFile == NULL)
    licqConf.WriteStr("Rejects", "none");
  else
  {
    pc = strrchr(m_szRejectFile, '/');
    pc++;
    licqConf.WriteStr("Rejects", pc);
  }

  // save the sound stuff
  licqConf.SetSection("onevent");
  COnEventManager *oem = OnEventManager();
  licqConf.WriteNum("Enable", oem->CommandType());
  oem->Lock();
  licqConf.WriteStr("Command", oem->Command());
  licqConf.WriteStr("Message", oem->Parameter(ON_EVENT_MSG));
  licqConf.WriteStr("Url", oem->Parameter(ON_EVENT_URL));
  licqConf.WriteStr("Chat",oem->Parameter(ON_EVENT_CHAT));
  licqConf.WriteStr("File",oem->Parameter(ON_EVENT_FILE));
  licqConf.WriteStr("OnlineNotify", oem->Parameter(ON_EVENT_NOTIFY));
  licqConf.WriteStr("SysMsg", oem->Parameter(ON_EVENT_SYSMSG));
  oem->Unlock();

  licqConf.FlushFile();

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  o->SaveLicqInfo();
  gUserManager.DropOwner();
}

//++++++NOT MT SAFE+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

const char *CICQDaemon::Terminal()       { return m_szTerminal; }
void CICQDaemon::SetTerminal(const char *s)  { SetString(&m_szTerminal, s); }

const char *CICQDaemon::getUrlViewer()
{
  if (strcmp(m_szUrlViewer, "none") == 0)
    return (NULL);
  else
    return (m_szUrlViewer);
}

void CICQDaemon::setUrlViewer(const char *s)
{
  SetString(&m_szUrlViewer, s);
}


void CICQDaemon::SetFirewallHost(const char *s)
{
  if (s == NULL || s[0] == '\0')
  {
    SetString(&m_szFirewallHost, "");
    CPacket::SetLocalIp(0);
  }
  else
  {
    SetString(&m_szFirewallHost, s);
    unsigned long n = INetSocket::GetIpByName(s);
    if (n == 0)
      gLog.Error("%sInvalid firewall hostname: %s\n", L_ERRORxSTR);
    else
      CPacket::SetLocalIp(n);
  }
}


int CICQDaemon::StartTCPServer(TCPSocket *s)
{
  if (m_nTCPBasePort == 0)
  {
    s->StartServer(0);
  }
  else
  {
    for (unsigned short p = m_nTCPBasePort; p < m_nTCPBasePort + m_nTCPBaseRange; p++)
    {
      if (s->StartServer(p)) break;
    }
  }

  if (s->Descriptor() != -1)
  {
    char sz[64];
    gLog.Info("%sLocal TCP server started on %s:%d.\n", L_TCPxSTR, s->LocalIpStr(sz), s->LocalPort());
  }

  return s->Descriptor();
}


void CICQDaemon::SetTCPBasePort(unsigned short p, unsigned short r)
{
  m_nTCPBasePort = p;
  m_nTCPBaseRange = r;
}

unsigned short CICQDaemon::TCPEnabled()
{
  return CPacket::Mode() == MODE_DIRECT;
}


void CICQDaemon::SetTCPEnabled(bool b)
{
  CPacket::SetMode(b ? MODE_DIRECT : MODE_INDIRECT);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//-----SaveUserList-------------------------------------------------------------
void CICQDaemon::SaveUserList()
{
  char filename[MAX_FILENAME_LEN];
  sprintf(filename, "%s/users.conf", BASE_DIR);
  FILE *usersConf = fopen(filename, "w");

  fprintf(usersConf, "[users]\nNumOfUsers = %d\n", gUserManager.NumUsers());

  unsigned short i = 1;
  FOR_EACH_USER_START(LOCK_R)
  {
    fprintf(usersConf, "User%d = %ld\n", i, pUser->Uin());
    i++;
  }
  FOR_EACH_USER_END

  fclose(usersConf);
}

void CICQDaemon::SetIgnore(unsigned short n, bool b)
{
  if (b)
    m_nIgnoreTypes |= n;
  else
    m_nIgnoreTypes &= ~n;
}


//-----AddUserToList------------------------------------------------------------
void CICQDaemon::AddUserToList(unsigned long _nUin)
{
  // Don't add a user we already have
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
  if (u != NULL)
  {
    gUserManager.DropUser(u);
    gLog.Warn("%sUser %ld already on contact list.\n", L_WARNxSTR, _nUin);
    return;
  }

  gUserManager.AddUser(new ICQUser(_nUin));
  SaveUserList();

  if (m_nUDPSocketDesc != -1) icqAddUser(_nUin);

  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_ADD, _nUin));
}


//-----AddUserToList------------------------------------------------------------
void CICQDaemon::AddUserToList(ICQUser *nu)
{
  // Don't add a user we already have
  ICQUser *u = gUserManager.FetchUser(nu->Uin(), LOCK_R);
  if (u != NULL)
  {
    gUserManager.DropUser(u);
    gLog.Warn("%sUser %ld already on contact list.\n", L_WARNxSTR, nu->Uin());
    return;
  }

  gUserManager.AddUser(nu);
  SaveUserList();

  if (m_nUDPSocketDesc != -1) icqAddUser(nu->Uin());

  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_ADD, nu->Uin()));
}

//-----RemoveUserFromList-------------------------------------------------------
void CICQDaemon::RemoveUserFromList(unsigned long _nUin)
{
  gUserManager.RemoveUser(_nUin);
  SaveUserList();
  //icqRemoveUser(id);

  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REMOVE, _nUin));
}


//-----ChangeUserStatus-------------------------------------------------------
void CICQDaemon::ChangeUserStatus(ICQUser *u, unsigned long s)
{
  unsigned short oldstatus = u->Status() | (u->StatusInvisible() << 8);
  if (s == ICQ_STATUS_OFFLINE)
    u->SetStatusOffline();
  else
    u->SetStatus(s);

  if(oldstatus != (u->Status() | (u->StatusInvisible() << 8))) {
    u->Touch();
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                    USER_STATUS, u->Uin()));
  }
}


//-----AddUserEvent-----------------------------------------------------------
bool CICQDaemon::AddUserEvent(ICQUser *u, CUserEvent *e)
{
  if (u->User()) e->AddToHistory(u, D_RECEIVER);
  // Don't log a user event if this user is on the ignore list
  if (u->IgnoreList() ||
      (e->IsMultiRec() && Ignore(IGNORE_MASSMSG)) ||
      (e->SubCommand() == ICQ_CMDxSUB_EMAILxPAGER && Ignore(IGNORE_EMAILPAGER)) ||
      (e->SubCommand() == ICQ_CMDxSUB_WEBxPANEL && Ignore(IGNORE_WEBPANEL)) )
  {
    delete e;
    return false;
  }
  u->AddEvent(e);
  u->Touch();
  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_EVENTS,
                                  u->Uin()));
  return true;
}


/*----------------------------------------------------------------------------
 * CICQDaemon::RejectEvent
 *
 *--------------------------------------------------------------------------*/
void CICQDaemon::RejectEvent(unsigned long nUin, CUserEvent *e)
{
  if (m_szRejectFile == NULL) return;

  FILE *f = fopen(m_szRejectFile, "a");
  if (f == NULL)
  {
    gLog.Warn("%sUnable to open \"%s\" for writing.\n", m_szRejectFile);
  }
  else
  {
    fprintf(f, "Event from new user (%ld) rejected: \n%s\n--------------------\n\n",
            nUin, e->Text());
    fclose(f);
  }
  delete e;
}


/*----------------------------------------------------------------------------
 * CICQDaemon::SendExpectEvent
 *
 * Sends an event and expects a reply.  Packages the given information into
 * an event structure and sticks it on the pending events queue.  Then signals
 * that it's there.
 *--------------------------------------------------------------------------*/
ICQEvent *CICQDaemon::SendExpectEvent(int _nSD, CPacket *packet, EConnect _eConnect)
{
  return SendExpectEvent(_nSD, packet, _eConnect, 0, NULL);
}

ICQEvent *CICQDaemon::SendExpectEvent(int _nSD, CPacket *packet, EConnect _eConnect,
                                      unsigned long _nDestinationUin, CUserEvent *ue)
{
  // If we are already shutting down, don't start any events
  if (m_bShuttingDown) return NULL;

  ICQEvent *e = new ICQEvent(this, _nSD, packet, _eConnect, _nDestinationUin, ue);

  pthread_mutex_lock(&mutex_runningevents);
  m_lxRunningEvents.push_back(e);
  pthread_mutex_unlock(&mutex_runningevents);

  DEBUG_THREADS("[SendExpectEvent] Throwing running event.\n");
  int nResult = pthread_create(&e->thread_send, NULL, &ProcessRunningEvent_tep, e);
  if (nResult != 0)
  {
    gLog.Warn("%sUnable to start event thread (#%d):\n%s%s.\n", L_ERRORxSTR,
              e->m_nSequence, L_BLANKxSTR, strerror(nResult));
    e->m_eResult = EVENT_ERROR;
    ProcessDoneEvent(e);
    return NULL;
  }

  return (e);
}


/*------------------------------------------------------------------------------
 * CICQDaemon::SendEvent
 *
 * Sends an event without expecting a reply, does not create an event
 * structure, and does not attempt a connection if the socket is invalid.
 * Can possibly block on send, but this is ok as it is never called from the
 * gui thread.
 * Note that the user who owns the given socket is probably read-locked at
 * this point.
 *----------------------------------------------------------------------------*/
bool CICQDaemon::SendEvent(int _nSD, CPacket &p)
{
  INetSocket *s = gSocketManager.FetchSocket(_nSD);
  if (s == NULL) return false;
  CBuffer *buf = p.Finalize();
  s->Send(buf);
  delete buf;
  gSocketManager.DropSocket(s);
  return true;
}


/*------------------------------------------------------------------------------
 * DoneEvent
 *
 * Marks the given event as done and removes it from the running events list.
 * Then calls PushDoneEvent on the event.
 *----------------------------------------------------------------------------*/
ICQEvent *CICQDaemon::DoneEvent(ICQEvent *e, EEventResult _eResult)
{
  pthread_mutex_lock(&mutex_runningevents);
  list<ICQEvent *>::iterator iter;
  bool bFound = false;
  for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); iter++)
  {
    if (e == *iter)
    {
      bFound = true;
      m_lxRunningEvents.erase(iter);
      break;
    }
  }
  //bool bFound = (iter == m_lxRunningEvents.end());
  pthread_mutex_unlock(&mutex_runningevents);

  // If we didn't find the event, it must have already been removed, we are too late
  if (!bFound) return (NULL);

  e->m_eResult = _eResult;

#if 0
#if ICQ_VERSION == 5
  if (_eResult == EVENT_CANCELLED && e->m_nSocket == m_nUDPSocketDesc)
  {
    pthread_mutex_lock(&mutex_runningevents);
    ICQEvent *e2 = new ICQEvent(e);
    e2->m_bCancelled = true;
    e2->m_xPacket = e->m_xPacket;
    m_lxRunningEvents.push_back(e2);
    pthread_mutex_unlock(&mutex_runningevents);
  }
  else
#endif
#endif
  {
    // Check if we should cancel a processing thread
    if (!pthread_equal(e->thread_send, pthread_self()))
      pthread_cancel(e->thread_send);
  }

  return (e);
}

/*------------------------------------------------------------------------------
 * DoneEvent (2)
 *
 * Differs from above only in that it takes a socket descriptor and sequence
 * instead of the event itself.  Thus it will be called from an acking thread
 * as opposed to a timed out or cancelling thread.
 * Note: There is a potential race condition here.  There is a time between
 *       when this function is called and when it finally cancels any sending
 *       thread during which the sending thread may continue to do stuff.
 *       The result is extra sends or time out warnings.
 *----------------------------------------------------------------------------*/
ICQEvent *CICQDaemon::DoneEvent(int _nSD, unsigned long _nSequence, EEventResult _eResult)
{
  pthread_mutex_lock(&mutex_runningevents);
  ICQEvent *e = NULL;
  list<ICQEvent *>::iterator iter;
  for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); iter++)
  {
    if ((*iter)->CompareEvent(_nSD, _nSequence) )
    {
      e = *iter;
      m_lxRunningEvents.erase(iter);
      break;
    }
  }
  pthread_mutex_unlock(&mutex_runningevents);

  // If we didn't find the event, it must have already been removed, we are too late
  if (e == NULL) return (NULL);

  e->m_eResult = _eResult;

  // Check if we should cancel a processing thread
  if (!pthread_equal(e->thread_send, pthread_self()))
    pthread_cancel(e->thread_send);

  return(e);
}


/*------------------------------------------------------------------------------
 * ProcessDoneEvent
 *
 * Processes the given event possibly passes the result to the gui.
 *----------------------------------------------------------------------------*/
void CICQDaemon::ProcessDoneEvent(ICQEvent *e)
{
#if ICQ_VERSION != 5
  static unsigned short s_nPingTimeOuts = 0;
#endif

  // Determine this now as later we might have deleted the event
  unsigned short nCommand = e->m_nCommand;
  EEventResult eResult = e->m_eResult;

  // Write the event to the history file if appropriate
  if (e->m_xUserEvent != NULL &&
      e->m_eResult == EVENT_ACKED &&
      e->m_nSubResult != ICQ_TCPxACK_RETURN)
  {
    ICQUser *u = gUserManager.FetchUser(e->m_nDestinationUin, LOCK_R);
    if (u != NULL)
    {
      e->m_xUserEvent->AddToHistory(u, D_SENDER);
      gUserManager.DropUser(u);
    }
  }

  // Process the event
  switch (e->m_nCommand)
  {
  // Ping is always sent by the daemon
  case ICQ_CMDxSND_PING:
#if ICQ_VERSION != 5
    if (e->m_eResult == EVENT_ACKED)
      s_nPingTimeOuts = 0;
    else
    {
      s_nPingTimeOuts++;
      if (s_nPingTimeOuts > MAX_PING_TIMEOUTS)
      {
        s_nPingTimeOuts = 0;
        icqRelogon();
      }
    }
#endif
    break;
  // Regular events
  case ICQ_CMDxTCP_START:
  case ICQ_CMDxSND_THRUxSERVER:
  case ICQ_CMDxSND_USERxADD:
  case ICQ_CMDxSND_USERxLIST:
  case ICQ_CMDxSND_SYSxMSGxREQ:
  case ICQ_CMDxSND_SYSxMSGxDONExACK:
  case ICQ_CMDxSND_AUTHORIZE:
  case ICQ_CMDxSND_VISIBLExLIST:
  case ICQ_CMDxSND_INVISIBLExLIST:
    PushPluginEvent(e);
    break;

  case ICQ_CMDxSND_SETxSTATUS:
    if (e->m_eResult == EVENT_ACKED)
    {
      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
      ChangeUserStatus(o, ((CPU_SetStatus *)e->m_xPacket)->Status() );
      gUserManager.DropOwner();
    }
    PushPluginEvent(e);
    break;

  case ICQ_CMDxSND_SETxRANDOMxCHAT:
    if (e->m_eResult == EVENT_ACKED)
    {
      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
      o->SetRandomChatGroup(((CPU_SetRandomChatGroup *)e->m_xPacket)->Group());
      gUserManager.DropOwner();
    }
    PushPluginEvent(e);
    break;

  // Extended events
  case ICQ_CMDxSND_LOGON:
  case ICQ_CMDxSND_USERxGETINFO:
  case ICQ_CMDxSND_USERxGETDETAILS:
  case ICQ_CMDxSND_UPDATExDETAIL:
  case ICQ_CMDxSND_UPDATExBASIC:
  case ICQ_CMDxSND_SEARCHxINFO:
  case ICQ_CMDxSND_SEARCHxUIN:
  case ICQ_CMDxSND_REGISTERxUSER:
  case ICQ_CMDxSND_META:
  case ICQ_CMDxSND_RANDOMxSEARCH:
    switch (e->m_eResult)
    {
      case EVENT_ERROR:
      case EVENT_TIMEDOUT:
      case EVENT_FAILED:
      case EVENT_SUCCESS:
      case EVENT_CANCELLED:
        PushPluginEvent(e);
        break;
      case EVENT_ACKED:  // push to extended event list
        PushExtendedEvent(e);
        break;
      default:
        gLog.Error("%sInternal error: ProcessDoneEvents(): Invalid result for extended event (%d).\n",
                   L_ERRORxSTR, e->m_eResult);
        delete e;
        return;
    }
    break;

  default:
    gLog.Error("%sInternal error: ProcessDoneEvents(): Unknown command (%04X).\n",
               L_ERRORxSTR, e->m_nCommand);
    delete e;
    return;
  }

  // Some special commands to deal with
#if ICQ_VERSION == 5
  if (nCommand != ICQ_CMDxTCP_START &&
           (eResult == EVENT_TIMEDOUT || eResult == EVENT_ERROR) )
  {
    if (nCommand == ICQ_CMDxSND_LOGON)
      m_eStatus = STATUS_OFFLINE_FORCED;
    else
      icqRelogon();
  }
#endif
}



/*------------------------------------------------------------------------------
 * DoneExtendedEvent
 *
 * Tracks down the relevant extended event, removes it from the list, and
 * returns it, marking the result as appropriate.
 *----------------------------------------------------------------------------*/
ICQEvent *CICQDaemon::DoneExtendedEvent(const unsigned short _nCommand, const unsigned short _nSubSequence, EEventResult _eResult)
{
  pthread_mutex_lock(&mutex_extendedevents);
  ICQEvent *e = NULL;
  list<ICQEvent *>::iterator iter;
  for (iter = m_lxExtendedEvents.begin(); iter != m_lxExtendedEvents.end(); iter++)
  {
    if ((*iter)->m_nSubSequence == _nSubSequence && (*iter)->m_nCommand == _nCommand)
    {
      e = *iter;
      m_lxExtendedEvents.erase(iter);
      break;
    }
  }
  pthread_mutex_unlock(&mutex_extendedevents);
  if (e != NULL) e->m_eResult = _eResult;
  return(e);
}


ICQEvent *CICQDaemon::DoneExtendedEvent(ICQEvent *e, EEventResult _eResult)
{
  pthread_mutex_lock(&mutex_extendedevents);
  list<ICQEvent *>::iterator iter;
  for (iter = m_lxExtendedEvents.begin(); iter != m_lxExtendedEvents.end(); iter++)
  {
    if (e == (*iter))
    {
      m_lxExtendedEvents.erase(iter);
      break;
    }
  }
  pthread_mutex_unlock(&mutex_extendedevents);
  if (iter == m_lxExtendedEvents.end()) return NULL;
  e->m_eResult = _eResult;
#if 0
  // If the event was cancelled we still want to wait internally for the reply
  if (_eResult == EVENT_CANCELLED)
  {
    pthread_mutex_lock(&mutex_extendedevents);
    ICQEvent *e2 = new ICQEvent(e);
    e2->m_bCancelled = true;
    e2->m_xPacket = e->m_xPacket;
    e->m_xPacket = NULL;
    m_lxExtendedEvents.push_back(e2);
    pthread_mutex_unlock(&mutex_extendedevents);
  }
#endif
  return(e);
}


/*------------------------------------------------------------------------------
 * PushExtendedEvent
 *
 * Takes the given event, moves it event into the extended event queue.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PushExtendedEvent(ICQEvent *e)
{
  pthread_mutex_lock(&mutex_extendedevents);
  m_lxExtendedEvents.push_back(e);
  pthread_mutex_unlock(&mutex_extendedevents);
}


/*------------------------------------------------------------------------------
 * PushPluginEvent
 *
 * Sticks the given event into the gui event queue.  Then signals that it is
 * there by sending data on the pipe.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PushPluginEvent(ICQEvent *e)
{
  vector<CPlugin *>::iterator iter;
  pthread_mutex_lock(&mutex_plugins);
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    if ((*iter)->CompareThread(e->thread_plugin))
    {
      (*iter)->PushEvent(e);
      break;
    }
  }
  // If no plugin got the event, then just delete it
  if (iter == m_vPlugins.end()) delete e;
  pthread_mutex_unlock(&mutex_plugins);
}


/*------------------------------------------------------------------------------
 * PushPluginSignal
 *
 * Sticks the given event into the gui signal queue.  Then signals that it is
 * there by sending data on the pipe.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PushPluginSignal(CICQSignal *s)
{
  vector<CPlugin *>::iterator iter;
  pthread_mutex_lock(&mutex_plugins);
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    if ( (*iter)->CompareMask(s->Signal()) )
      (*iter)->PushSignal(new CICQSignal(s));
  }
  pthread_mutex_unlock(&mutex_plugins);
  delete s;
}


/*------------------------------------------------------------------------------
 * PopPluginSignal
 *
 * Pops an event from the gui signal queue.
 *----------------------------------------------------------------------------*/
CICQSignal *CICQDaemon::PopPluginSignal()
{
  vector<CPlugin *>::iterator iter;
  CICQSignal *s = NULL;
  pthread_mutex_lock(&mutex_plugins);
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    if ( (*iter)->CompareThread(pthread_self()) )
    {
      s = (*iter)->PopSignal();
      break;
    }
  }
  pthread_mutex_unlock(&mutex_plugins);
  return s;
}

/*------------------------------------------------------------------------------
 * PopPluginEvent
 *
 * Pops an event from the gui event queue.
 *----------------------------------------------------------------------------*/
ICQEvent *CICQDaemon::PopPluginEvent()
{
  vector<CPlugin *>::iterator iter;
  ICQEvent *e = NULL;
  pthread_mutex_lock(&mutex_plugins);
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    if ( (*iter)->CompareThread(pthread_self()) )
    {
      e = (*iter)->PopEvent();
      break;
    }
  }
  pthread_mutex_unlock(&mutex_plugins);
  return e;
}


//-----CICQDaemon::CancelEvent---------------------------------------------------------
void CICQDaemon::CancelEvent(CICQEventTag *t)
{
  ICQEvent *e = NULL;
  if ( (e = DoneEvent(t->m_nSocketDesc, t->m_nSequence, EVENT_CANCELLED)) == NULL &&
       (e = DoneExtendedEvent(t->m_nSocketDesc, t->m_nSequence, EVENT_CANCELLED)) == NULL) return;
  ProcessDoneEvent(e);

  if (e->m_nSubCommand == ICQ_CMDxSUB_CHAT)
    icqChatRequestCancel(e->m_nDestinationUin, e->m_nSequence);
  else if (e->m_nSubCommand == ICQ_CMDxSUB_FILE)
    icqFileTransferCancel(e->m_nDestinationUin, e->m_nSequence);
}


//-----updateAllUsers-------------------------------------------------------------------------
void CICQDaemon::UpdateAllUsers()
{
/*
  for (unsigned short i = 0; i < getNumUsers(); i++)
  {
    icqUserBasicInfo(getUser(i));
    icqUserExtInfo(getUser(i));
  }
*/
  gLog.Warn("%sThis feature does not work right now.\n", L_WARNxSTR);
}


//-----ParseFE------------------------------------------------------------------
bool CICQDaemon::ParseFE(char *szBuffer, char ***szSubStr, int nNumSubStr)
{
   char *pcEnd = szBuffer, *pcStart;
   unsigned short i = 0;
   bool bDone = false;
   // Clear the character pointers
   memset(*szSubStr, 0, nNumSubStr * sizeof(char *));

   while (!bDone && i < nNumSubStr)
   {
      pcStart = pcEnd;
      while (*pcEnd != '\0' && (unsigned char)*pcEnd != (unsigned char)0xFE)
        pcEnd++;
      if (*pcEnd == '\0')
         bDone = true;
      else // we are at an FE boundary
         *pcEnd++ = '\0';
      (*szSubStr)[i++] = pcStart;
   }

   return (bDone && i == nNumSubStr);
}


unsigned long CICQDaemon::StringToStatus(char *_szStatus)
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned long nStatus = o->AddStatusFlags(0);
  gUserManager.DropOwner();

  if (_szStatus[0] == '*')
  {
    _szStatus++;
    nStatus |= ICQ_STATUS_FxPRIVATE;
  }
  if (strcasecmp(_szStatus, "online") == 0)
    nStatus |= ICQ_STATUS_ONLINE;
  else if (strcasecmp(_szStatus, "away") == 0)
    nStatus |= ICQ_STATUS_AWAY;
  else if (strcasecmp(_szStatus, "na") == 0)
    nStatus |= ICQ_STATUS_NA;
  else if (strcasecmp(_szStatus, "occupied") == 0)
    nStatus |= ICQ_STATUS_OCCUPIED;
  else if (strcasecmp(_szStatus, "dnd") == 0)
    nStatus |= ICQ_STATUS_DND;
  else if (strcasecmp(_szStatus, "ffc") == 0)
    nStatus |= ICQ_STATUS_FREEFORCHAT;
  else if (strcasecmp(_szStatus, "offline") == 0)
    nStatus = ICQ_STATUS_OFFLINE;
  else
  {
    nStatus = INT_MAX;
  }
  return nStatus;
}


//-----ProcessFifo--------------------------------------------------------------
void CICQDaemon::ProcessFifo(char *_szBuf)
{
#ifdef USE_FIFO
  char *szCommand, *szRawArgs;

  // Make the command and data variables point to the relevant data in the buf
  szCommand = szRawArgs = _szBuf;
  while (*szRawArgs != '\0' && !isspace(*szRawArgs)) szRawArgs++;
  if (szRawArgs != '\0')
  {
    *szRawArgs = '\0';
    szRawArgs++;
    while (isspace(*szRawArgs)) szRawArgs++;
  }
  if (szRawArgs[strlen(szRawArgs) - 1] == '\n') szRawArgs[strlen(szRawArgs) - 1] = '\0';

  gLog.Info("%sReceived command \"%s\" with arguments \"%s\".\n", L_FIFOxSTR,
            szCommand, szRawArgs);

  char *szProcessedArgs = new char[strlen(szRawArgs) + 1];
  AddNewLines(szProcessedArgs, szRawArgs);
  char *szArgs = szProcessedArgs;

  // Process the command
  if (strcasecmp(szCommand, "status") == 0)
  {
    char *szStatus = szArgs;
    if (*szStatus == '\0')
    {
      gLog.Warn("%sFifo \"status\" command with no argument.\n", L_WARNxSTR);
      goto fifo_done;
    }
    while (!isspace(*szArgs) && *szArgs != '\0') szArgs++;
    if (*szArgs != '\0')
    {
      *szArgs = '\0';
      szArgs++;
    }
    // Determine the status to go to
    ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
    bool b = o->StatusOffline();
    gUserManager.DropOwner();

    unsigned long nStatus = StringToStatus(szStatus);
    if (nStatus == INT_MAX)
    {
      gLog.Warn("%sFifo \"status\" command with invalid status \"%s\".\n",
                L_WARNxSTR, szStatus);
      return;
    }

    if (nStatus == ICQ_STATUS_OFFLINE)
    {
      if (!b) icqLogoff();
    }
    else
    {
      CICQEventTag *t = NULL;
      if (b)
        t = icqLogon(nStatus);
      else
        t = icqSetStatus(nStatus);
      if (t == NULL) delete t;
    }

    // Now set the auto response
    while (isspace(*szArgs) && *szArgs != '\0') szArgs++;
    if (*szArgs != '\0')
    {
      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
      o->SetAutoResponse(szArgs);
      gUserManager.DropOwner();
    }
  }
  else if (strcasecmp(szCommand, "auto_response") == 0)
  {
    if (*szArgs != '\0')
    {
      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
      o->SetAutoResponse(szArgs);
      gUserManager.DropOwner();
    }
    else
    {
      gLog.Warn("%sFifo \"auto_response\" command with no argument.\n", L_WARNxSTR);
    }
  }
  else if (strcasecmp(szCommand, "message") == 0)
  {
    if (*szArgs == '\0')
    {
      gLog.Warn("%sFifo \"message\" with no UIN.\n", L_WARNxSTR);
      goto fifo_done;
    }
    char *szUin = szArgs;
    while(!isspace(*szArgs) && *szArgs != '\0') szArgs++;
    if (*szArgs == '\0')
    {
      gLog.Warn("%sFifo \"message\" with no message.\n", L_WARNxSTR);
      goto fifo_done;
    }
    *szArgs = '\0';
    szArgs++;
    unsigned long nUin = atoi(szUin);
    icqSendMessage(nUin, szArgs, false, false);
  }
  else if (strcasecmp(szCommand, "url") == 0)
  {
    if (*szArgs == '\0')
    {
      gLog.Warn("%sFifo \"url\" with no UIN.\n", L_WARNxSTR);
      goto fifo_done;
    }
    char *szUin = szArgs;
    while(!isspace(*szArgs) && *szArgs != '\0') szArgs++;
    if (*szArgs == '\0')
    {
      gLog.Warn("%sFifo \"url\" with no URL.\n", L_WARNxSTR);
      goto fifo_done;
    }
    *szArgs = '\0';
    szArgs++;
    unsigned long nUin = atoi(szUin);
    char *szUrl = szArgs;
    while(!isspace(*szArgs) && *szArgs != '\0') szArgs++;
    if (*szArgs == '\0')
    {
      gLog.Warn("%sFifo \"url\" with no description.\n", L_WARNxSTR);
      goto fifo_done;
    }
    *szArgs = '\0';
    szArgs++;
    icqSendUrl(nUin, szUrl, szArgs, false, false);
  }
  else if (strcasecmp(szCommand, "redirect") == 0)
  {
    if (*szArgs != '\0')
    {
      if (!Redirect(szArgs))
      {
        gLog.Warn("%sRedirection to \"%s\" failed:\n%s%s.\n", L_WARNxSTR,
                  szArgs, L_BLANKxSTR, strerror(errno));
      }
      else
        gLog.Info("%sOutput redirected to \"%s\".\n", L_INITxSTR, szArgs);
    }
    else
    {
      gLog.Warn("%sFifo \"redirect\" command with no argument.\n", L_WARNxSTR);
    }
  }
  else if (strcasecmp(szCommand, "adduser") == 0)
  {
    if (*szArgs == '\0')
    {
      gLog.Warn("%sFifo \"adduser\" with no UIN.\n", L_WARNxSTR);
      goto fifo_done;
    }
    char *szUin = szArgs;
    unsigned long nUin = atoi(szUin);
    AddUserToList(nUin);
  }
  else if (strcasecmp(szCommand, "userinfo") == 0)
  {
    if (*szArgs == '\0')
    {
      gLog.Warn("%sFifo \"userinfo\" with no UIN.\n", L_WARNxSTR);
      goto fifo_done;
    }
    char *szUin = szArgs;
    unsigned long nUin = atoi(szUin);
    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
    if (u == NULL)
    {
      gLog.Warn("%sUser %ld not on contact list, not retrieving info.\n", L_WARNxSTR, nUin);
    }
    else
    {
      gUserManager.DropUser(u);
      icqRequestMetaInfo(nUin);
    }
  }
  else if (strcasecmp(szCommand, "exit") == 0)
  {
    Shutdown();
  }
  else if (strcasecmp(szCommand, "help") == 0)
  {
    gLog.Info("%sFifo Help:\n"
              "%sstatus [*]<status> [auto response]\n"
              "%sauto_response <auto response>\n"
              "%smessage <uin> <message>\n"
              "%surl <uin> <url> <description>\n"
              "%sadduser <uin>\n"
              "%suserinfo <uin>\n"
              "%sredirect <device>\n"
              "%sexit\n", L_FIFOxSTR, L_BLANKxSTR, L_BLANKxSTR, L_BLANKxSTR,
              L_BLANKxSTR, L_BLANKxSTR, L_BLANKxSTR, L_BLANKxSTR, L_BLANKxSTR);
  }
  else
  {
    gLog.Warn("%sUnknown fifo command \"%s\".\n", L_WARNxSTR, szCommand);
  }

fifo_done:
  delete [] szProcessedArgs;
#endif //USE_FIFO

}

