// -*- c-basic-offset: 2 -*-

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

#include "licq_icq.h"
#include "licq_user.h"
#include "licq_constants.h"
#include "licq_file.h"
#include "licq_log.h"
#include "licq_translate.h"
#include "licq_packets.h"
#include "licq_plugind.h"
#include "licq.h"
#include "support.h"

#include "licq_icqd.h"

#define STRIP(x) while(isspace(*(x)) && *(x) != '\0') (x)++;

using namespace std;

CDaemonStats::CDaemonStats()
{
  m_nTotal = m_nOriginal = m_nLastSaved = 0;
  m_szName[0] = m_szTag[0] = '\0';
}

CDaemonStats::CDaemonStats(const char *name, const char *tag)
{
  m_nTotal = m_nOriginal = m_nLastSaved = 0;
  strcpy(m_szName, name);
  strcpy(m_szTag, tag);
}


void CDaemonStats::Reset()
{
  m_nTotal = m_nOriginal = 0;
}

void CDaemonStats::Init()
{
  m_nOriginal = m_nLastSaved = m_nTotal;
}


CICQDaemon *gLicqDaemon = NULL;



//-----CICQDaemon::constructor--------------------------------------------------
CICQDaemon::CICQDaemon(CLicq *_licq)
{
  char temp[MAX_FILENAME_LEN];

  licq = _licq;
  gLicqDaemon = this;

  // Initialise the data values
  m_nIgnoreTypes = 0;
  m_nTCPSocketDesc = -1;
  m_nTCPSrvSocketDesc = -1;
  m_eStatus = STATUS_OFFLINE_MANUAL;
  m_bShuttingDown = false;
  m_bRegistering = false;
  m_nServerAck = 0;
  m_bLoggingOn = false;
  m_bOnlineNotifies = true;

  // Begin parsing the config file
  snprintf(m_szConfigFile, MAX_FILENAME_LEN, "%s/%s", BASE_DIR, "licq.conf");
  CIniFile licqConf(INI_FxERROR | INI_FxFATAL);
  licqConf.LoadFile(m_szConfigFile);
  licqConf.SetFlags(0);

  licqConf.SetSection("network");

  // ICQ Server
  char szICQServer[MAX_HOSTNAME_LEN];

  licqConf.ReadStr("ICQServer", szICQServer, DEFAULT_SERVER_HOST);
  m_szICQServer = new char[strlen(szICQServer) + 1];
  strcpy(m_szICQServer, szICQServer);
  licqConf.ReadNum("ICQServerPort", m_nICQServerPort, DEFAULT_SERVER_PORT);

  bool bTcpEnabled;
  unsigned short nTCPPortsLow, nTCPPortsHigh;
  licqConf.ReadNum("TCPPortsLow", nTCPPortsLow, 0);
  licqConf.ReadNum("TCPPortsHigh", nTCPPortsHigh, 0);
  SetTCPPorts(nTCPPortsLow, nTCPPortsHigh);
  licqConf.ReadBool("TCPEnabled", bTcpEnabled, true);
  SetTCPEnabled(bTcpEnabled);
  licqConf.ReadNum("MaxUsersPerPacket", m_nMaxUsersPerPacket, 100);
  licqConf.ReadNum("IgnoreTypes", m_nIgnoreTypes, 0);
  unsigned long nColor;
  licqConf.ReadNum("ForegroundColor", nColor, 0x00000000);
  CICQColor::SetDefaultForeground(nColor);
  licqConf.ReadNum("BackgroundColor", nColor, 0x00FFFFFF);
  CICQColor::SetDefaultBackground(nColor);


  // Rejects log file
  licqConf.ReadStr("Rejects", temp, "log.rejects");
  if (strcmp(temp, "none") != 0)
  {
    m_szRejectFile = new char[MAX_FILENAME_LEN];
    snprintf(m_szRejectFile, MAX_FILENAME_LEN, "%s/%s", BASE_DIR, temp);
  }
  else
    m_szRejectFile = NULL;


  // Error log file
  licqConf.ReadStr("Errors", m_szErrorFile, "licq.log");
  licqConf.ReadNum("ErrorTypes", m_nErrorTypes, L_ERROR | L_UNKNOWN);
  if (strcmp(m_szErrorFile, "none") != 0)
  {
    snprintf(temp, MAX_FILENAME_LEN, "%s/%s", BASE_DIR, m_szErrorFile);
    CLogService_File *l = new CLogService_File(m_nErrorTypes);
    if (!l->SetLogFile(temp, "a"))
    {
      gLog.Error("%sUnable to open %s as error log:\n%s%s.\n",
                  L_ERRORxSTR, temp, L_BLANKxSTR, strerror(errno));
      delete l;
    }
    else
      gLog.AddService(l);
  }

  // Loading translation table from file
  licqConf.ReadStr("Translation", temp, "none");
  if (strncmp(temp, "none", 4) != 0)
  {
     char TranslationTableFileNameFull[MAX_FILENAME_LEN];
     snprintf(TranslationTableFileNameFull, MAX_FILENAME_LEN, "%s%s/%s", SHARE_DIR, TRANSLATION_DIR, temp);
     gTranslator.setTranslationMap (TranslationTableFileNameFull);
  }

  // Url viewer
  m_szUrlViewer = NULL;
  licqConf.ReadStr("UrlViewer", temp, "none");
  m_szUrlViewer = new char[strlen(temp) + 1];
  strcpy(m_szUrlViewer, temp);

  // Terminal
  m_szTerminal = NULL;
  licqConf.ReadStr("Terminal", temp, "xterm -T Licq -e ");
  m_szTerminal = new char[strlen(temp) + 1];
  strcpy(m_szTerminal, temp);

  // Proxy
  m_xProxy = NULL;
  char t_str[MAX_HOSTNAME_LEN];

  licqConf.ReadBool("ProxyEnabled", m_bProxyEnabled, false);
  licqConf.ReadNum("ProxyServerType", m_nProxyType, PROXY_TYPE_HTTP);
  licqConf.ReadStr("ProxyServer", t_str, "");
  m_szProxyHost = new char[strlen(t_str) + 1];
  strcpy(m_szProxyHost, t_str);
  licqConf.ReadNum("ProxyServerPort", m_nProxyPort, 0);
  licqConf.ReadBool("ProxyAuthEnabled", m_bProxyAuthEnabled, false);
  licqConf.ReadStr("ProxyLogin", t_str, "");
  m_szProxyLogin = new char[strlen(t_str) + 1];
  strcpy(m_szProxyLogin, t_str);
  licqConf.ReadStr("ProxyPassword", t_str, "");
  m_szProxyPasswd = new char[strlen(t_str) + 1];
  strcpy(m_szProxyPasswd, t_str);

  // -----OnEvent configuration-----
  char szOnEventCommand[MAX_FILENAME_LEN], *szOnParams[MAX_ON_EVENT];
  unsigned short nOnEventCmdType;

  licqConf.SetSection("onevent");
  licqConf.ReadNum("Enable", nOnEventCmdType, 0);
  licqConf.ReadBool("AlwaysOnlineNotify", m_bAlwaysOnlineNotify, false);
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
  licqConf.ReadStr("MsgSent", szOnParams[ON_EVENT_MSGSENT], "");
  m_xOnEventManager.SetParameters(szOnEventCommand, (const char **)szOnParams);
  for (int i = 0; i < MAX_ON_EVENT; i++)
    delete [] szOnParams[i];

  // Statistics
  m_nResetTime = 0;
  m_sStats.push_back(CDaemonStats("Events Sent", "Sent"));
  m_sStats.push_back(CDaemonStats("Events Received", "Recv"));
  m_sStats.push_back(CDaemonStats("Events Rejected", "Reject"));
  m_sStats.push_back(CDaemonStats("Auto Response Checked", "ARC"));
#ifdef SAVE_STATS
  DaemonStatsList::iterator iter;
  if (licqConf.SetSection("stats"))
  {
    unsigned long t;
    licqConf.ReadNum("Reset", t, 0);
    m_nResetTime = t;
    for (iter = m_sStats.begin(); iter != m_sStats.end(); iter++)
    {
      licqConf.ReadNum(iter->m_szTag, iter->m_nTotal, 0);
      iter->Init();
    }
  }
#endif
  m_nStartTime = time(NULL);
  if (m_nResetTime == 0) m_nResetTime = m_nStartTime;

  // Pipes
  pipe(pipe_newsocket);

  // Initialize the random number generator
  srand(time(NULL));

  // Start up our threads
  pthread_mutex_init(&mutex_runningevents, NULL);
  pthread_mutex_init(&mutex_extendedevents, NULL);
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
  CPacket::SetLocalPort(s->LocalPort());
  gSocketManager.DropSocket(s);


#ifdef USE_FIFO
  // Open the fifo
  snprintf(sz, MAX_FILENAME_LEN, "%s/licq_fifo", BASE_DIR);
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


/*------------------------------------------------------------------------------
 * RegisterPlugin
 *
 * Registers the current thread as a new plugin.  Returns the pipe to listen
 * on for notification.
 *----------------------------------------------------------------------------*/
int CICQDaemon::RegisterPlugin(unsigned long nSignalMask)
{
  PluginsListIter it;
  int p = -1;

  pthread_mutex_lock(&licq->mutex_plugins);
  for (it = licq->list_plugins.begin();
       it != licq->list_plugins.end();
       it++)
  {
    if ((*it)->CompareThread(pthread_self()))
    {
      p = (*it)->Pipe();
      (*it)->SetSignalMask(nSignalMask);
      break;
    }
  }
  pthread_mutex_unlock(&licq->mutex_plugins);

  if (p == -1)
    gLog.Error("%sInvalid thread in registration attempt.\n", L_ERRORxSTR);

  return p;
}


/*------------------------------------------------------------------------------
 * UnregisterPlugin
 *
 * Unregisters the current plugin thread.
 *----------------------------------------------------------------------------*/
void CICQDaemon::UnregisterPlugin()
{
  PluginsListIter iter;
  pthread_mutex_lock(&licq->mutex_plugins);
  for (iter = licq->list_plugins.begin();
       iter != licq->list_plugins.end();
       iter++)
  {
    if ((*iter)->CompareThread(pthread_self()))
    {
      //gLog.Info("%sUnregistering plugin %d.\n", L_ENDxSTR, (*iter)->Id());
      (*iter)->SetSignalMask(0);
      break;
    }
  }
  pthread_mutex_unlock(&licq->mutex_plugins);
}


/*------------------------------------------------------------------------------
 * PluginList
 *
 * Fetches the list of plugins.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PluginList(PluginsList &lPlugins)
{
  lPlugins.erase(lPlugins.begin(), lPlugins.end());
  pthread_mutex_lock(&licq->mutex_plugins);
  lPlugins = licq->list_plugins;
  pthread_mutex_unlock(&licq->mutex_plugins);
}


/*------------------------------------------------------------------------------
 * PluginShutdown
 *
 * Unloads the given plugin.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PluginShutdown(int id)
{
  PluginsListIter iter;
  pthread_mutex_lock(&licq->mutex_plugins);
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); iter++)
  {
    if (id == (*iter)->Id()) (*iter)->Shutdown();
  }
  pthread_mutex_unlock(&licq->mutex_plugins);
}

/*------------------------------------------------------------------------------
 * PluginDisable
 *
 * Disables the given plugin.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PluginDisable(int id)
{
  PluginsListIter iter;
  pthread_mutex_lock(&licq->mutex_plugins);
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); iter++)
  {
    if (id == (*iter)->Id()) (*iter)->Disable();
  }
  pthread_mutex_unlock(&licq->mutex_plugins);
}

/*------------------------------------------------------------------------------
 * PluginEnable
 *
 * Enableds the given plugin.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PluginEnable(int id)
{
  PluginsListIter iter;
  pthread_mutex_lock(&licq->mutex_plugins);
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); iter++)
  {
    if (id == (*iter)->Id()) (*iter)->Enable();
  }
  pthread_mutex_unlock(&licq->mutex_plugins);
}



/*------------------------------------------------------------------------------
 * PluginLoad
 *
 * Loads the given plugin.
 *----------------------------------------------------------------------------*/
bool CICQDaemon::PluginLoad(const char *szPlugin, int argc, char **argv)
{
  optind = 0;
  CPlugin *p = licq->LoadPlugin(szPlugin, argc, argv);

  if (p == NULL) return false;

  pthread_mutex_lock(&licq->mutex_plugins);
  licq->StartPlugin(p);
  pthread_mutex_unlock(&licq->mutex_plugins);
  return true;
}



const char *CICQDaemon::Version()
{
  return licq->Version();
}

//-----ICQ::destructor----------------------------------------------------------
CICQDaemon::~CICQDaemon()
{
  if(m_szUrlViewer)   delete []m_szUrlViewer;
  if(m_szRejectFile)  delete []m_szRejectFile;
  gLicqDaemon = NULL;
}


void CICQDaemon::FlushStats()
{
#ifdef SAVE_STATS
  // Verify we need to save anything
  DaemonStatsList::iterator iter;
  for (iter = m_sStats.begin(); iter != m_sStats.end(); iter++)
  {
    if (iter->Dirty()) break;
  }
  if (iter == m_sStats.end()) return;

  // Save the stats
  CIniFile licqConf(INI_FxALLOWxCREATE);
  if (!licqConf.LoadFile(m_szConfigFile)) return;
  licqConf.SetSection("stats");
  licqConf.WriteNum("Reset", (unsigned long)m_nResetTime);
  for (iter = m_sStats.begin(); iter != m_sStats.end(); iter++)
  {
    licqConf.WriteNum(iter->m_szTag, iter->m_nTotal);
    iter->ClearDirty();
  }
  licqConf.FlushFile();
#endif
}

void CICQDaemon::ResetStats()
{
  DaemonStatsList::iterator iter;
  for (iter = m_sStats.begin(); iter != m_sStats.end(); iter++)
  {
    iter->Reset();
  }
  m_nResetTime = time(NULL);
}


pthread_t *CICQDaemon::Shutdown()
{
  static pthread_t *thread_shutdown = NULL;
  if (m_bShuttingDown) return(thread_shutdown);
  thread_shutdown = (pthread_t *)malloc(sizeof(pthread_t));
  m_bShuttingDown = true;
  // Small race condition here if multiple plugins call shutdown at the same time
  SaveUserList();
  pthread_create (thread_shutdown, NULL, &Shutdown_tep, this);
  return (thread_shutdown);
}


//-----SaveConf-----------------------------------------------------------------
void CICQDaemon::SaveConf()
{
  CIniFile licqConf(INI_FxERROR | INI_FxALLOWxCREATE);
  if (!licqConf.LoadFile(m_szConfigFile)) return;

  licqConf.SetSection("network");

  // ICQ Server
  licqConf.WriteStr("ICQServer", m_szICQServer);
  licqConf.WriteNum("ICQServerPort", m_nICQServerPort);

  licqConf.WriteNum("TCPPortsLow", m_nTCPPortsLow);
  licqConf.WriteNum("TCPPortsHigh", m_nTCPPortsHigh);
  licqConf.WriteBool("TCPEnabled", CPacket::Mode() == MODE_DIRECT);
  licqConf.WriteNum("MaxUsersPerPacket", m_nMaxUsersPerPacket);
  licqConf.WriteNum("IgnoreTypes", m_nIgnoreTypes);
  licqConf.WriteNum("ForegroundColor", CICQColor::DefaultForeground());
  licqConf.WriteNum("BackgroundColor", CICQColor::DefaultBackground());


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
  licqConf.WriteNum("ErrorTypes", m_nErrorTypes);
  if (m_szRejectFile == NULL)
    licqConf.WriteStr("Rejects", "none");
  else
  {
    pc = strrchr(m_szRejectFile, '/');
    pc++;
    licqConf.WriteStr("Rejects", pc);
  }

  // Proxy
  licqConf.WriteBool("ProxyEnabled", m_bProxyEnabled);
  licqConf.WriteNum("ProxyServerType", m_nProxyType);
  licqConf.WriteStr("ProxyServer", m_szProxyHost);
  licqConf.WriteNum("ProxyServerPort", m_nProxyPort);
  licqConf.WriteBool("ProxyAuthEnabled", m_bProxyAuthEnabled);
  licqConf.WriteStr("ProxyLogin", m_szProxyLogin);
  licqConf.WriteStr("ProxyPassword", m_szProxyPasswd);

  // save the sound stuff
  licqConf.SetSection("onevent");
  COnEventManager *oem = OnEventManager();
  licqConf.WriteNum("Enable", oem->CommandType());
  licqConf.WriteBool("AlwaysOnlineNotify", m_bAlwaysOnlineNotify);
  oem->Lock();
  licqConf.WriteStr("Command", oem->Command());
  licqConf.WriteStr("Message", oem->Parameter(ON_EVENT_MSG));
  licqConf.WriteStr("Url", oem->Parameter(ON_EVENT_URL));
  licqConf.WriteStr("Chat",oem->Parameter(ON_EVENT_CHAT));
  licqConf.WriteStr("File",oem->Parameter(ON_EVENT_FILE));
  licqConf.WriteStr("OnlineNotify", oem->Parameter(ON_EVENT_NOTIFY));
  licqConf.WriteStr("SysMsg", oem->Parameter(ON_EVENT_SYSMSG));
  licqConf.WriteStr("MsgSent", oem->Parameter(ON_EVENT_MSGSENT));
  oem->Unlock();

  licqConf.FlushFile();

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  o->SaveLicqInfo();
  gUserManager.DropOwner();
}

//++++++NOT MT SAFE+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

const char *CICQDaemon::Terminal()       { return m_szTerminal; }
void CICQDaemon::SetTerminal(const char *s)  { SetString(&m_szTerminal, s); }
bool CICQDaemon::AlwaysOnlineNotify()  { return m_bAlwaysOnlineNotify; }
void CICQDaemon::SetAlwaysOnlineNotify(bool b)  { m_bAlwaysOnlineNotify = b; }

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


bool CICQDaemon::ViewUrl(const char *u)
{
  if (strcmp(m_szUrlViewer, "none") == 0) return false;

  char **arglist = (char**)malloc( 3*sizeof(char*));
  arglist[0] = m_szUrlViewer;
  arglist[1] = (char*)u;
  arglist[2] = NULL;

  if(!fork()) {
    execvp(arglist[0], arglist);
    _exit(-1);
  }
  free(arglist);

  return true;
}


int CICQDaemon::StartTCPServer(TCPSocket *s)
{
  if (m_nTCPPortsLow == 0)
  {
    s->StartServer(0);
  }
  else
  {
    for (unsigned short p = m_nTCPPortsLow; p <= m_nTCPPortsHigh; p++)
    {
      if (s->StartServer(p)) break;
    }
  }

  char sz[64];
  if (s->Descriptor() != -1)
  {
    gLog.Info("%sLocal TCP server started on port %d.\n", L_TCPxSTR, s->LocalPort());
  }
  else if (s->Error() == EADDRINUSE)
  {
    gLog.Warn("%sNo ports available for local TCP server.\n", L_WARNxSTR);
  }
  else
  {
    gLog.Warn("%sFailed to start local TCP server:\n%s%s\n", L_WARNxSTR,
       L_BLANKxSTR, s->ErrorStr(sz, 64));
  }

  return s->Descriptor();
}


void CICQDaemon::SetTCPPorts(unsigned short p, unsigned short r)
{
  m_nTCPPortsLow = p;
  m_nTCPPortsHigh = r;
  if (m_nTCPPortsHigh < m_nTCPPortsLow)
  {
    gLog.Warn("%sTCP high port (%d) is lower then TCP low port (%d).\n",
       L_WARNxSTR, m_nTCPPortsHigh, m_nTCPPortsLow);
    m_nTCPPortsHigh = m_nTCPPortsLow + 10;
  }
}

bool CICQDaemon::TCPEnabled()
{
  return CPacket::Mode() == MODE_DIRECT;
}


void CICQDaemon::SetTCPEnabled(bool b)
{
  CPacket::SetMode(b ? MODE_DIRECT : MODE_INDIRECT);
}


void CICQDaemon::InitProxy()
{
  if (m_xProxy != NULL)
  {
    delete m_xProxy;
    m_xProxy = NULL;
  }

  switch (m_nProxyType)
  {
    case PROXY_TYPE_HTTP :
      m_xProxy = new HTTPProxyServer();
      break;
    default:
      break;
  }

  if (m_xProxy != NULL)
  {
    gLog.Info("%sResolving proxy: %s:%d...\n", L_INITxSTR, m_szProxyHost, m_nProxyPort);
    if (!m_xProxy->SetProxyAddr(m_szProxyHost, m_nProxyPort)) {
      char buf[128];

      gLog.Warn("%sUnable to resolve proxy server %s:\n%s%s.\n", L_ERRORxSTR,
                 m_szProxyHost, L_BLANKxSTR, m_xProxy->ErrorStr(buf, 128));
      delete m_xProxy;
      m_xProxy = NULL;
    }

    if (m_xProxy)
    {
      if (m_bProxyAuthEnabled)
        m_xProxy->SetProxyAuth(m_szProxyLogin, m_szProxyPasswd);

      m_xProxy->InitProxy();
    }
  }
}


unsigned short VersionToUse(unsigned short v_in)
{
  /*if (ICQ_VERSION_TCP & 4 && v & 4) return 4;
  if (ICQ_VERSION_TCP & 2 && v & 2) return 2;
  gLog.Warn("%sUnknown TCP version %d.  Attempting v2.\n", L_WARNxSTR, v);
  return 2;*/
  unsigned short v_out = v_in < ICQ_VERSION_TCP ? v_in : ICQ_VERSION_TCP;
  if (v_out < 2)
  {
    gLog.Warn("%sInvalid TCP version %d.  Attempting v2.\n", L_WARNxSTR, v_out);
    v_out = 2;
  }
  return v_out;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//-----SaveUserList-------------------------------------------------------------
void CICQDaemon::SaveUserList()
{
  char filename[MAX_FILENAME_LEN];
  snprintf(filename, MAX_FILENAME_LEN, "%s/users.conf", BASE_DIR);
  FILE *usersConf = fopen(filename, "w");

  fprintf(usersConf, "[users]\nNumOfUsers = %d\n", gUserManager.NumUsers());

  unsigned short i = 1;
  FOR_EACH_UIN_START
  {
    fprintf(usersConf, "User%d = %ld\n", i, nUin);
    i++;
  }
  FOR_EACH_UIN_END

  fclose(usersConf);
}

void CICQDaemon::SetIgnore(unsigned short n, bool b)
{
  if (b)
    m_nIgnoreTypes |= n;
  else
    m_nIgnoreTypes &= ~n;
}


/*---------------------------------------------------------------------------
 * AddUserToList
 *
 * Adds the given uin to the contact list.  Note that when this call returns
 * the user is not locked.
 *-------------------------------------------------------------------------*/
bool CICQDaemon::AddUserToList(unsigned long nUin, bool bNotify)
{
  // Don't add invalid uins
  if (nUin == 0) return false;

  // Don't add a user we already have
  if (gUserManager.IsOnList(nUin))
  {
    gLog.Warn("%sUser %ld already on contact list.\n", L_WARNxSTR, nUin);
    return false;
  }

  ICQUser *u = new ICQUser(nUin);
  gUserManager.AddUser(u);
  gUserManager.DropUser(u);
  SaveUserList();

  if (m_nTCPSrvSocketDesc != -1 && bNotify) icqAddUser(nUin);

  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_ADD, nUin));

  return true;
}


/*---------------------------------------------------------------------------
 * AddUserToList
 *
 * Adds the given user to the contact list.  Note that when this call returns
 * the user is write locked and will need to be dropped.  When calling this
 * function it is important that the user not be locked in any way.
 *-------------------------------------------------------------------------*/
void CICQDaemon::AddUserToList(ICQUser *nu)
{
  // Don't add a user we already have
  if (gUserManager.IsOnList(nu->Uin()))
  {
    gLog.Warn("%sUser %ld already on contact list.\n", L_WARNxSTR, nu->Uin());
    return;
  }

  gUserManager.AddUser(nu);
  // At this point the user is write locked
  SaveUserList();

  if (m_nTCPSrvSocketDesc != -1) icqAddUser(nu->Uin());

  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_ADD, nu->Uin()));
}

//-----RemoveUserFromList-------------------------------------------------------
void CICQDaemon::RemoveUserFromList(unsigned long _nUin)
{
  if (m_nTCPSrvSocketDesc != -1) icqRemoveUser(_nUin);

  gUserManager.RemoveUser(_nUin);
  SaveUserList();

  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REMOVE, _nUin));
}


//-----ChangeUserStatus-------------------------------------------------------
void CICQDaemon::ChangeUserStatus(ICQUser *u, unsigned long s)
{
  unsigned short oldstatus = u->Status() | (u->StatusInvisible() << 8);
  int arg = 0;
  if (s == ICQ_STATUS_OFFLINE)
  {
    if (!u->StatusOffline()) arg = -1;
    u->SetStatusOffline();
  }
  else
  {
    if (u->StatusOffline()) arg = 1;
    u->SetStatus(s);
  }

  // Say that we know their status for sure
  u->SetOfflineOnDisconnect(false);

  if(oldstatus != (u->Status() | (u->StatusInvisible() << 8)))
  {
    u->Touch();
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                    USER_STATUS, u->Uin(), arg));
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
  u->EventPush(e);
  //u->Touch();
  m_sStats[STATS_EventsReceived].Inc();

  //PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_EVENTS,
  //   u->Uin(), e->Id()));
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
    gLog.Warn("%sUnable to open \"%s\" for writing.\n", L_WARNxSTR, m_szRejectFile);
  }
  else
  {
    fprintf(f, "Event from new user (%ld) rejected: \n%s\n--------------------\n\n",
            nUin, e->Text());
    fclose(f);
  }
  delete e;
  m_sStats[STATS_EventsRejected].Inc();
}


/*---------------------------------------------------------------------------
 * CheckBirthdays
 *
 * Send a signal for each user whose birthday occurs in the next few days.
 *-------------------------------------------------------------------------*/
void CICQDaemon::CheckBirthdays(UinList &uins)
{
  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->Birthday(m_nBirthdayRange) != -1)
      uins.push_back(pUser->Uin());
  }
  FOR_EACH_USER_END
}


/*----------------------------------------------------------------------------
 * CICQDaemon::SendEvent
 *
 * Sends an event without expecting a reply.
 *--------------------------------------------------------------------------*/

void CICQDaemon::SendEvent_Server(CPacket *packet)
{
#if 1
  ICQEvent *e = new ICQEvent(this, m_nTCPSrvSocketDesc, packet, CONNECT_SERVER, 0, NULL);
  e->m_NoAck = true;
  int nResult = pthread_create(&e->thread_send, NULL, &ProcessRunningEvent_Server_tep, e);
  if (nResult != 0)
  {
    gLog.Error("%sUnable to start server event thread (#%ld):\n%s%s.\n", L_ERRORxSTR,
       e->m_nSequence, L_BLANKxSTR, strerror(nResult));
    e->m_eResult = EVENT_ERROR;
  }
#else
  SendEvent(m_nTCPSrvSocketDesc, *packet, true);
#endif
}

ICQEvent *CICQDaemon::SendExpectEvent_Server(unsigned long nUin, CPacket *packet,
   CUserEvent *ue)
{
  // If we are already shutting down, don't start any events
  if (m_bShuttingDown) return NULL;

  if (ue != NULL) ue->m_eDir = D_SENDER;
  ICQEvent *e = new ICQEvent(this, m_nTCPSrvSocketDesc, packet, CONNECT_SERVER, nUin, ue);
  return SendExpectEvent(e, &ProcessRunningEvent_Server_tep);
}


ICQEvent *CICQDaemon::SendExpectEvent_Client(ICQUser *pUser, CPacket *packet,
   CUserEvent *ue)
{
  // If we are already shutting down, don't start any events
  if (m_bShuttingDown) return NULL;

  if (ue != NULL) ue->m_eDir = D_SENDER;
  ICQEvent *e = new ICQEvent(this, pUser->SocketDesc(), packet,
     CONNECT_USER, pUser->Uin(), ue);
  return SendExpectEvent(e, &ProcessRunningEvent_Client_tep);
}


ICQEvent *CICQDaemon::SendExpectEvent(ICQEvent *e, void *(*fcn)(void *))
{
  pthread_mutex_lock(&mutex_runningevents);
  m_lxRunningEvents.push_back(e);
  pthread_mutex_unlock(&mutex_runningevents);

  assert(e);

  gLog.Info("appending: %p\n", e);

  int nResult = pthread_create(&e->thread_send, NULL, fcn, e);
  if (nResult != 0)
  {
    gLog.Error("%sUnable to start event thread (#%ld):\n%s%s.\n", L_ERRORxSTR,
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
bool CICQDaemon::SendEvent(int nSD, CPacket &p, bool d)
{
  INetSocket *s = gSocketManager.FetchSocket(nSD);
  if (s == NULL) return false;
  bool r = SendEvent(s, p, d);
  gSocketManager.DropSocket(s);
  return r;
}

bool CICQDaemon::SendEvent(INetSocket *pSock, CPacket &p, bool d)
{
  CBuffer *buf = p.Finalize(pSock);
  pSock->Send(buf);
  if (d) delete buf;
  return true;
}


/*------------------------------------------------------------------------------
 * FailEvents
 *
 * Fails all events on the given socket.
 *----------------------------------------------------------------------------*/
void CICQDaemon::FailEvents(int sd, int err)
{
  // Go through all running events and fail all from this socket
  ICQEvent *e = NULL;
  do
  {
    e = NULL;
    pthread_mutex_lock(&mutex_runningevents);
    list<ICQEvent *>::iterator iter;
    for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); iter++)
    {
      if ((*iter)->m_nSocketDesc == sd)
      {
        e = *iter;
        break;
      }
    }
    pthread_mutex_unlock(&mutex_runningevents);
    if (e != NULL && DoneEvent(e, EVENT_ERROR) != NULL)
    {
      // If the connection was reset, we can try again
      if (err == ECONNRESET)
      {
        e->m_nSocketDesc = -1;
        // We have to decrypt the packet so we can re-encrypt it properly
        Decrypt_Client(e->m_pPacket->getBuffer(),
         ((CPacketTcp *)e->m_pPacket)->Version());
        SendExpectEvent(e, &ProcessRunningEvent_Client_tep);
      }
      else
      {
        ProcessDoneEvent(e);
      }
    }
  } while (e != NULL);
}


/*------------------------------------------------------------------------------
 * DoneEvent
 *
 * Marks the given event as done and removes it from the running events list.
 *----------------------------------------------------------------------------*/
ICQEvent *CICQDaemon::DoneEvent(ICQEvent *e, EventResult _eResult)
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

#if 0
  if (m_lxRunningEvents.size()) {
    gLog.Info("doneevents: for: %p pending: \n", e);
    for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); iter++)
    {
      gLog.Info("%p Command: %d SubCommand: %d Sequence: %ld SubSequence: %d: Uin: %ld\n", *iter,
                (*iter)->Command(), (*iter)->SubCommand(), (*iter)->Sequence(), (*iter)->SubSequence(),
                (*iter)->Uin());
    }
  }
#endif

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
ICQEvent *CICQDaemon::DoneEvent(int _nSD, unsigned long _nSequence, EventResult _eResult)
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


ICQEvent *CICQDaemon::DoneEvent(unsigned long tag, EventResult _eResult)
{
  pthread_mutex_lock(&mutex_runningevents);
  ICQEvent *e = NULL;
  list<ICQEvent *>::iterator iter;
  for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); iter++)
  {
    if ((*iter)->Equals(tag))
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
#if ICQ_VERSION < 8
void CICQDaemon::ProcessDoneEvent(ICQEvent *e)
{
#if ICQ_VERSION != 5
  static unsigned short s_nPingTimeOuts = 0;
#endif

  // Determine this now as later we might have deleted the event
  unsigned short nCommand = e->m_nCommand;
  EventResult eResult = e->m_eResult;

  // Write the event to the history file if appropriate
  if (e->m_pUserEvent != NULL &&
      e->m_eResult == EVENT_ACKED &&
      e->m_nSubResult != ICQ_TCPxACK_RETURN)
  {
    ICQUser *u = gUserManager.FetchUser(e->m_nDestinationUin, LOCK_R);
    if (u != NULL)
    {
      e->m_pUserEvent->AddToHistory(u, D_SENDER);
      u->SetLastSentEvent();
      m_xOnEventManager.Do(ON_EVENT_MSGSENT, u);
      gUserManager.DropUser(u);
    }
    m_sStats[STATS_EventsSent].Inc();
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
  case ICQ_CMDxSND_MODIFYxVIEWxLIST:
    PushPluginEvent(e);
    break;

  case ICQ_CMDxSND_SETxSTATUS:
    if (e->m_eResult == EVENT_ACKED)
    {
      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
      ChangeUserStatus(o, ((CPU_SetStatus *)e->m_pPacket)->Status() );
      gUserManager.DropOwner();
    }
    PushPluginEvent(e);
    break;

  case ICQ_CMDxSND_SETxRANDOMxCHAT:
    if (e->m_eResult == EVENT_ACKED)
    {
      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
      o->SetRandomChatGroup(((CPU_SetRandomChatGroup *)e->m_pPacket)->Group());
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
  {
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
  }

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
    {
      m_bLoggingOn = false;
      m_eStatus = STATUS_OFFLINE_FORCED;
    }
    else
      icqRelogon();
  }
#endif
}
#endif


/*------------------------------------------------------------------------------
 * DoneExtendedEvent
 *
 * Tracks down the relevant extended event, removes it from the list, and
 * returns it, marking the result as appropriate.
 *----------------------------------------------------------------------------*/
ICQEvent *CICQDaemon::DoneExtendedServerEvent(const unsigned short _nSubSequence, EventResult _eResult)
{
  pthread_mutex_lock(&mutex_extendedevents);
  ICQEvent *e = NULL;
  list<ICQEvent *>::iterator iter;
  for (iter = m_lxExtendedEvents.begin(); iter != m_lxExtendedEvents.end(); iter++)
  {
    if ((*iter)->m_nSubSequence == _nSubSequence)
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


ICQEvent *CICQDaemon::DoneExtendedEvent(ICQEvent *e, EventResult _eResult)
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


ICQEvent *CICQDaemon::DoneExtendedEvent(unsigned long tag, EventResult _eResult)
{
  pthread_mutex_lock(&mutex_extendedevents);
  ICQEvent *e = NULL;
  list<ICQEvent *>::iterator iter;
  for (iter = m_lxExtendedEvents.begin(); iter != m_lxExtendedEvents.end(); iter++)
  {
    if ((*iter)->Equals(tag))
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


/*------------------------------------------------------------------------------
 * PushExtendedEvent
 *
 * Takes the given event, moves it event into the extended event queue.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PushExtendedEvent(ICQEvent *e)
{
  pthread_mutex_lock(&mutex_extendedevents);
  m_lxExtendedEvents.push_back(e);
#if 0
  gLog.Info("%p pushing Command: %d SubCommand: %d Sequence: %ld SubSequence: %d: Uin: %ld\n", e,
            e->Command(), e->SubCommand(), e->Sequence(), e->SubSequence(), e->Uin());
#endif
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
  PluginsListIter iter;
  pthread_mutex_lock(&licq->mutex_plugins);
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); iter++)
  {
    if ((*iter)->CompareThread(e->thread_plugin))
    {
      (*iter)->PushEvent(e);
      break;
    }
  }
  // If no plugin got the event, then just delete it
  if (iter == licq->list_plugins.end()) delete e;
  pthread_mutex_unlock(&licq->mutex_plugins);
}


/*------------------------------------------------------------------------------
 * PushPluginSignal
 *
 * Sticks the given event into the gui signal queue.  Then signals that it is
 * there by sending data on the pipe.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PushPluginSignal(CICQSignal *s)
{
  PluginsListIter iter;
  pthread_mutex_lock(&licq->mutex_plugins);
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); iter++)
  {
    if ( (*iter)->CompareMask(s->Signal()) )
      (*iter)->PushSignal(new CICQSignal(s));
  }
  pthread_mutex_unlock(&licq->mutex_plugins);
  delete s;
}


/*------------------------------------------------------------------------------
 * PopPluginSignal
 *
 * Pops an event from the gui signal queue.
 *----------------------------------------------------------------------------*/
CICQSignal *CICQDaemon::PopPluginSignal()
{
  PluginsListIter iter;
  CICQSignal *s = NULL;
  pthread_mutex_lock(&licq->mutex_plugins);
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); iter++)
  {
    if ( (*iter)->CompareThread(pthread_self()) )
    {
      s = (*iter)->PopSignal();
      break;
    }
  }
  pthread_mutex_unlock(&licq->mutex_plugins);
  return s;
}

/*------------------------------------------------------------------------------
 * PopPluginEvent
 *
 * Pops an event from the gui event queue.
 *----------------------------------------------------------------------------*/
ICQEvent *CICQDaemon::PopPluginEvent()
{
  PluginsListIter iter;
  ICQEvent *e = NULL;
  pthread_mutex_lock(&licq->mutex_plugins);
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); iter++)
  {
    if ( (*iter)->CompareThread(pthread_self()) )
    {
      e = (*iter)->PopEvent();
      break;
    }
  }
  pthread_mutex_unlock(&licq->mutex_plugins);
  return e;
}


//-----CICQDaemon::CancelEvent---------------------------------------------------------
void CICQDaemon::CancelEvent(unsigned long t)
{
  ICQEvent *e = NULL;
  if ( (e = DoneEvent(t, EVENT_CANCELLED)) == NULL &&
       (e = DoneExtendedEvent(t, EVENT_CANCELLED)) == NULL)
  {
    gLog.Warn("%sCancelled event not found.\n", L_WARNxSTR);
    return;
  }

  CancelEvent(e);
}

void CICQDaemon::CancelEvent(ICQEvent *e)
{
  e->m_eResult = EVENT_CANCELLED;

  if (e->m_nSubCommand == ICQ_CMDxSUB_CHAT)
    icqChatRequestCancel(e->m_nDestinationUin, e->m_nSequence);
  else if (e->m_nSubCommand == ICQ_CMDxSUB_FILE)
    icqFileTransferCancel(e->m_nDestinationUin, e->m_nSequence);
  else if (e->m_nSubCommand == ICQ_CMDxSUB_SECURExOPEN)
    icqOpenSecureChannelCancel(e->m_nDestinationUin, e->m_nSequence);

  ProcessDoneEvent(e);
}


//-----updateAllUsers-------------------------------------------------------------------------
void CICQDaemon::UpdateAllUsers()
{
  FOR_EACH_UIN_START
  {
    icqRequestMetaInfo(nUin);
  }
  FOR_EACH_UIN_END
}


void CICQDaemon::UpdateAllUsersInGroup(GroupType g, unsigned short nGroup)
{
  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->GetInGroup(g, nGroup))
    {
      icqRequestMetaInfo(pUser->Uin());
    }
  }
  FOR_EACH_USER_END
}


//-----ParseFE------------------------------------------------------------------
bool ParseFE(char *szBuffer, char ***szSubStr, int nNumSubStr)
{
  char *pcEnd = szBuffer, *pcStart;
  unsigned short i = 0;

  // Clear the character pointers
  memset(*szSubStr, 0, nNumSubStr * sizeof(char *));

  while (*pcEnd && i < nNumSubStr)
  {
     pcStart = pcEnd;

     while (*pcEnd && (unsigned char)*pcEnd != (unsigned char)0xFE)  pcEnd++;
     if ((unsigned char)*pcEnd == (unsigned char)'\xFE')  *pcEnd++ = '\0';

     (*szSubStr)[i++] = pcStart;
  }

  while(i < nNumSubStr)  (*szSubStr)[i++] = pcEnd;

  return (!*pcEnd);
}


unsigned long StringToStatus(char *_szStatus)
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

int GetUinFromArg(char **p_szArg, unsigned long *nUin)
{
  char *szAlias, *szCmd;
  bool bCheckUin = true;
  char *szArg = *p_szArg;

  *nUin = 0;

  if (szArg == NULL) {
    return 0;
  }

  // Check if the alias is quoted
  if (szArg[0] == '"')
  {
    bCheckUin = false;
    szAlias = &szArg[1];
    szCmd = strchr(&szArg[1], '"');
    if (szCmd == NULL)
    {
      gLog.Warn("%sUnbalanced quotes.\n", L_WARNxSTR);
      return -1;
    }
    *szCmd++ = '\0';
    szCmd = strchr(szCmd, ' ');
  }
  else if (szArg[0] == '#')
  {
    *p_szArg = NULL;
    *nUin = gUserManager.OwnerUin();
    return 0;
  }
  else
  {
    szAlias = szArg;
    szCmd = strchr(szArg, ' ');
  }

  if (szCmd != NULL)
  {
    *szCmd++ = '\0';
    STRIP(szCmd);
  }
  *p_szArg = szCmd;

  // Find the user
  // See if all the chars are digits
  if (bCheckUin)
  {
    char *sz = szAlias;
    while (isdigit(*sz)) sz++;
    if (*sz == '\0') *nUin = atol(szAlias);
  }

  if (*nUin == 0)
  {
    FOR_EACH_USER_START(LOCK_R)
    {
      if (strcasecmp(szAlias, pUser->GetAlias()) == 0)
      {
        *nUin = pUser->Uin();
        FOR_EACH_USER_BREAK;
      }
    }
    FOR_EACH_USER_END
    if (*nUin == 0)
    {
      gLog.Warn("%sInvalid user: %s.\n", L_WARNxSTR, szAlias);
      return -1;
    }
  }
  else
  {
    if (!gUserManager.IsOnList(*nUin))
    {
      gLog.Warn("%sInvalid uin: %lu.\n", L_WARNxSTR, *nUin);
      return -1;
    }
  }

  return 0;
}

//-----ProcessFifo--------------------------------------------------------------
void CICQDaemon::ProcessFifo(char *_szBuf)
{
#ifdef USE_FIFO
  char *szCommand, *szRawArgs;

  // Make the command and data variables point to the relevant data in the buf
  szCommand = szRawArgs = _szBuf;
  while (*szRawArgs && !isspace(*szRawArgs)) szRawArgs++;
  if (*szRawArgs)
  {
    *szRawArgs = '\0';
    szRawArgs++;
    while (isspace(*szRawArgs)) szRawArgs++;
  }
  if (*szRawArgs && szRawArgs[strlen(szRawArgs) - 1] == '\n')
    szRawArgs[strlen(szRawArgs) - 1] = '\0';

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
      if (b)
        icqLogon(nStatus);
      else
        icqSetStatus(nStatus);
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
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    o->SetAutoResponse(szArgs);
    gUserManager.DropOwner();
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
  else if (strcasecmp(szCommand, "ui_viewevent") == 0)
  {
    unsigned long nUin;
    /* Empty argument string is acceptable */
    char *tmp = szRawArgs ? (*szRawArgs ? szRawArgs : NULL) : NULL;

    if (!GetUinFromArg(&tmp, &nUin))
    {
      PushPluginSignal(new CICQSignal(SIGNAL_UI_VIEWEVENT, 0, nUin, 0, 0));
    }
  }
  else if (strcasecmp(szCommand, "ui_message") == 0)
  {
    unsigned long nUin;

    if (!GetUinFromArg(szRawArgs ? &szRawArgs : NULL, &nUin))
    {
      if (nUin)
      {
        PushPluginSignal(new CICQSignal(SIGNAL_UI_MESSAGE, 0, nUin, 0, 0));
      }
      else
      {
        gLog.Warn("%sMissing argument user.\n", L_WARNxSTR);
      }
    }
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
              "%sexit\n"
              "%sui_viewevent [user name]\n"
              "%sui_message <user name>\n", L_FIFOxSTR, L_BLANKxSTR, L_BLANKxSTR,
              L_BLANKxSTR, L_BLANKxSTR, L_BLANKxSTR, L_BLANKxSTR, L_BLANKxSTR,
              L_BLANKxSTR, L_BLANKxSTR, L_BLANKxSTR);
  }
  else
  {
    gLog.Warn("%sUnknown fifo command \"%s\".\n", L_WARNxSTR, szCommand);
  }

fifo_done:
  delete [] szProcessedArgs;
#endif //USE_FIFO

}

