// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

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
#include <assert.h>
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
  //just in case we need to sign on automatically
  m_nDesiredStatus = ICQ_STATUS_ONLINE;
  m_bShuttingDown = false;
  m_bRegistering = false;
  m_nServerAck = 0;
  m_bLoggingOn = false;
  m_bOnlineNotifies = true;

  fifo_fs = NULL;

  // Begin parsing the config file
  snprintf(m_szConfigFile, MAX_FILENAME_LEN, "%s/%s", BASE_DIR, "licq.conf");
  m_szConfigFile[MAX_FILENAME_LEN - 1] = '\0';
  CIniFile licqConf(INI_FxERROR | INI_FxFATAL);
  licqConf.LoadFile(m_szConfigFile);
  licqConf.SetFlags(0);

  licqConf.SetSection("network");

  // ICQ Server
  char szICQServer[MAX_LINE_LEN];

  licqConf.ReadStr("ICQServer", szICQServer, DEFAULT_SERVER_HOST);
  m_szICQServer = (char *)malloc(strlen(szICQServer) + 1);
  strcpy(m_szICQServer, szICQServer);
  licqConf.ReadNum("ICQServerPort", m_nICQServerPort, DEFAULT_SERVER_PORT);

  unsigned short nTCPPortsLow, nTCPPortsHigh;
  licqConf.ReadNum("TCPPortsLow", nTCPPortsLow, 0);
  licqConf.ReadNum("TCPPortsHigh", nTCPPortsHigh, 0);
  SetTCPPorts(nTCPPortsLow, nTCPPortsHigh);
  licqConf.ReadBool("TCPEnabled", m_bTCPEnabled, true);
  licqConf.ReadBool("Firewall", m_bFirewall, false);
  SetTCPEnabled(!m_bFirewall || (m_bFirewall && m_bTCPEnabled));
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
    m_szRejectFile[MAX_FILENAME_LEN - 1] = '\0';
  }
  else
    m_szRejectFile = NULL;


  // Error log file
  licqConf.ReadStr("Errors", m_szErrorFile, "licq.log");
  licqConf.ReadNum("ErrorTypes", m_nErrorTypes, L_ERROR | L_UNKNOWN);
  if (strcmp(m_szErrorFile, "none") != 0)
  {
    snprintf(temp, MAX_FILENAME_LEN, "%s/%s", BASE_DIR, m_szErrorFile);
    temp[MAX_FILENAME_LEN - 1] = '\0';
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
     TranslationTableFileNameFull[MAX_FILENAME_LEN - 1] = '\0';
     gTranslator.setTranslationMap (TranslationTableFileNameFull);
  }

  // Url viewer
  m_szUrlViewer = NULL;
  licqConf.ReadStr("UrlViewer", temp, "none");
  m_szUrlViewer = (char *)malloc(strlen(temp) + 1);
  strcpy(m_szUrlViewer, temp);

  // Terminal
  m_szTerminal = NULL;
  licqConf.ReadStr("Terminal", temp, "xterm -T Licq -e ");
  m_szTerminal = (char *)malloc(strlen(temp) + 1);
  strcpy(m_szTerminal, temp);

  // Proxy
  m_xProxy = NULL;
  char t_str[MAX_LINE_LEN];

  licqConf.ReadBool("ProxyEnabled", m_bProxyEnabled, false);
  licqConf.ReadNum("ProxyServerType", m_nProxyType, PROXY_TYPE_HTTP);
  licqConf.ReadStr("ProxyServer", t_str, "");
  m_szProxyHost = (char *)malloc(strlen(t_str) + 1);
  strcpy(m_szProxyHost, t_str);
  licqConf.ReadNum("ProxyServerPort", m_nProxyPort, 0);
  licqConf.ReadBool("ProxyAuthEnabled", m_bProxyAuthEnabled, false);
  licqConf.ReadStr("ProxyLogin", t_str, "");
  m_szProxyLogin = (char *)malloc(strlen(t_str) + 1);
  strcpy(m_szProxyLogin, t_str);
  licqConf.ReadStr("ProxyPassword", t_str, "");
  m_szProxyPasswd = (char *)malloc(strlen(t_str) + 1);
  strcpy(m_szProxyPasswd, t_str);

  // Misc
  licqConf.ReadBool("UseSS", m_bUseSS, true); // server side list
  licqConf.ReadBool("ReconnectAfterUinClash", m_bReconnectAfterUinClash, false);

  // -----OnEvent configuration-----
  char szOnEventCommand[MAX_FILENAME_LEN], *szOnParams[MAX_ON_EVENT];
  unsigned short nOnEventCmdType;

  licqConf.SetSection("onevent");
  licqConf.ReadNum("Enable", nOnEventCmdType, 0);
  licqConf.ReadBool("AlwaysOnlineNotify", m_bAlwaysOnlineNotify, false);
  m_xOnEventManager.SetCommandType(nOnEventCmdType);
  for (int i = 0; i < MAX_ON_EVENT; i++) {
    szOnParams[i] = new char[MAX_FILENAME_LEN];
    szOnParams[i][0] = '\0';
  }

	// Prepare default values for onEvent
	char DEF_MESSAGE[MAX_FILENAME_LEN];
	char DEF_URL[MAX_FILENAME_LEN];
	char DEF_CHAT[MAX_FILENAME_LEN];
	char DEF_FILE[MAX_FILENAME_LEN];
	char DEF_NOTIFY[MAX_FILENAME_LEN];
	char DEF_SYSMSG[MAX_FILENAME_LEN];
	char DEF_MSGSENT[MAX_FILENAME_LEN];
	strcpy(DEF_MESSAGE, SHARE_DIR);
	strcpy(DEF_URL, SHARE_DIR);
	strcpy(DEF_CHAT, SHARE_DIR);
	strcpy(DEF_FILE, SHARE_DIR);
	strcpy(DEF_NOTIFY, SHARE_DIR);
	strcpy(DEF_SYSMSG, SHARE_DIR);
	strcpy(DEF_MSGSENT, SHARE_DIR);
	// be paranoid, don't let it overflow
	unsigned short MAX_APPEND = (MAX_FILENAME_LEN - strlen(SHARE_DIR) - 1);  // max chars to append via strncat()
	strncat(DEF_MESSAGE, "sounds/icq/Message.wav", MAX_APPEND);
	strncat(DEF_URL,     "sounds/icq/URL.wav", MAX_APPEND);
	strncat(DEF_CHAT,    "sounds/icq/Chat.wav", MAX_APPEND);
	strncat(DEF_FILE,    "sounds/icq/File.wav", MAX_APPEND);
	strncat(DEF_NOTIFY,  "sounds/icq/Online.wav", MAX_APPEND);
	strncat(DEF_SYSMSG,  "sounds/icq/System.wav", MAX_APPEND);
	strncat(DEF_MSGSENT, "sounds/icq/Message.wav", MAX_APPEND);

  licqConf.ReadStr("Command", szOnEventCommand, "play");
  licqConf.ReadStr("Message", szOnParams[ON_EVENT_MSG], DEF_MESSAGE);
  licqConf.ReadStr("Url", szOnParams[ON_EVENT_URL], DEF_URL);
  licqConf.ReadStr("Chat", szOnParams[ON_EVENT_CHAT], DEF_CHAT);
  licqConf.ReadStr("File", szOnParams[ON_EVENT_FILE], DEF_FILE);
  licqConf.ReadStr("OnlineNotify", szOnParams[ON_EVENT_NOTIFY], DEF_NOTIFY);
  licqConf.ReadStr("SysMsg", szOnParams[ON_EVENT_SYSMSG], DEF_SYSMSG);
  licqConf.ReadStr("MsgSent", szOnParams[ON_EVENT_MSGSENT], DEF_MSGSENT);
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
  pthread_mutex_init(&mutex_sendqueue_server, NULL);
  pthread_mutex_init(&mutex_modifyserverusers, NULL);
  pthread_mutex_init(&mutex_cancelthread, NULL);
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
  if (o != NULL)
  {
    o->SetIntIp(s->LocalIp());
    o->SetPort(s->LocalPort());
    gUserManager.DropOwner();
  }
  CPacket::SetLocalPort(s->LocalPort());
  gSocketManager.DropSocket(s);


#ifdef USE_FIFO
  // Open the fifo
  snprintf(sz, MAX_FILENAME_LEN, "%s/licq_fifo", BASE_DIR);
  sz[MAX_FILENAME_LEN - 1] = '\0';
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


//---RegisterPlugin------------------------------------------------------------
/*! \brief Registers current thread as new plugin
 *
 * Registers the current thread as a new plugin. 
 *
 * \return Returns the pipe to listen on for notification.
 */
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


//---UnregisterPlugin----------------------------------------------------------
/*! \brief Unregisters current plugin thread
 *
 * Unregisters the current plugin thread.
 */
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


//---PluginList----------------------------------------------------------------
/*! \brief Fetches the list of plugins */
void CICQDaemon::PluginList(PluginsList &lPlugins)
{
  lPlugins.erase(lPlugins.begin(), lPlugins.end());
  pthread_mutex_lock(&licq->mutex_plugins);
  lPlugins = licq->list_plugins;
  pthread_mutex_unlock(&licq->mutex_plugins);
}


//---PluginShutdown------------------------------------------------------------
/*! \brief Unloads the given plugin */
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

//---PluginDisablen------------------------------------------------------------
/*! \brief Disables the given plugin. */
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

//---PluginEnable--------------------------------------------------------------
/*! \brief Enables the given plugin. */
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



//---PluginLoad----------------------------------------------------------------
/*! \brief Loads the given plugin. 
 *
 *  \return Returns true on success, else returns false.
 */
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

void CICQDaemon::ProtoPluginList(ProtoPluginsList &lPlugins)
{
  lPlugins.erase(lPlugins.begin(), lPlugins.end());
  pthread_mutex_lock(&licq->mutex_protoplugins);
  lPlugins = licq->list_protoplugins;
  pthread_mutex_unlock(&licq->mutex_protoplugins);
}

bool CICQDaemon::ProtoPluginLoad(const char *szPlugin)
{
  CProtoPlugin *p = licq->LoadProtoPlugin(szPlugin);
  if (p == NULL) return false;

  licq->StartProtoPlugin(p);

  return true;
}

int CICQDaemon::RegisterProtoPlugin()
{
  ProtoPluginsListIter it;
  int p = -1;

  pthread_mutex_lock(&licq->mutex_protoplugins);
  for (it = licq->list_protoplugins.begin();
       it != licq->list_protoplugins.end();
       it++)
  {
    if ((*it)->CompareThread(pthread_self()))
    {
      p = (*it)->Pipe();
      break;
    }
  }
  pthread_mutex_unlock(&licq->mutex_protoplugins);

  if (p == -1)
    gLog.Error("%sInvalid thread in registration attempt.\n", L_ERRORxSTR);

  return p;
}

char *CICQDaemon::ProtoPluginName(unsigned long _nPPID)
{
  ProtoPluginsListIter it;
  char *p = 0;

  pthread_mutex_lock(&licq->mutex_protoplugins);
  for (it = licq->list_protoplugins.begin();
       it != licq->list_protoplugins.end();
       it++)
  {
    if ((*it)->Id() == _nPPID)
    {
      p = (char *)(*it)->Name();
      break;
    }
  }
  pthread_mutex_unlock(&licq->mutex_protoplugins);
  return p;
}

//---Version-------------------------------------------------------------------
/*! \brief Returns the version of Licq */
const char *CICQDaemon::Version()
{
  return licq->Version();
}

//-----ICQ::destructor----------------------------------------------------------
CICQDaemon::~CICQDaemon()
{
  if(m_szUrlViewer)   free(m_szUrlViewer);
  if(m_szRejectFile)  delete []m_szRejectFile;
  if(m_szICQServer)   free(m_szICQServer);
  if(m_szProxyHost)   free(m_szProxyHost);
  if(m_szProxyLogin)  free(m_szProxyLogin);
  if(m_szProxyPasswd) free(m_szProxyPasswd);
  if(m_szTerminal)    free(m_szTerminal);
  if(fifo_fs)         fclose(fifo_fs);
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
  if (m_bShuttingDown) return(&thread_shutdown);
  m_bShuttingDown = true;
  // Small race condition here if multiple plugins call shutdown at the same time
  SaveUserList();
  pthread_create (&thread_shutdown, NULL, &Shutdown_tep, this);
  return (&thread_shutdown);
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
  licqConf.WriteBool("TCPEnabled", m_bTCPEnabled);
  licqConf.WriteBool("Firewall", m_bFirewall);
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

  // Misc
  licqConf.WriteBool("UseSS", m_bUseSS); // server side list
  licqConf.WriteBool("ReconnectAfterUinClash", m_bReconnectAfterUinClash);

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
  if (o != NULL)
  {
    o->SaveLicqInfo();
    // TODO: Make this work with multiple owners
    if (strcmp(o->IdString(), "0") != 0)
    {
      licqConf.SetSection("owners");
      OwnerList *ol = gUserManager.LockOwnerList(LOCK_R);
      licqConf.WriteNum("NumOfOwners", (unsigned long)ol->size());
      gUserManager.UnlockOwnerList();
      licqConf.WriteStr("Owner1.Id", o->IdString());
      licqConf.FlushFile();
    }
    
    gUserManager.DropOwner();
  }
}

//++++++NOT MT SAFE+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

const char *CICQDaemon::Terminal()       { return m_szTerminal; }
void CICQDaemon::SetTerminal(const char *s)  { SetString(&m_szTerminal, s); }
bool CICQDaemon::AlwaysOnlineNotify()  { return m_bAlwaysOnlineNotify; }
void CICQDaemon::SetAlwaysOnlineNotify(bool b)  { m_bAlwaysOnlineNotify = b; }

const char *CICQDaemon::getUrlViewer()
{
  if ((strcmp(m_szUrlViewer, "none") == 0) || (strlen(m_szUrlViewer) == 0))
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
  if (getUrlViewer() == NULL) return false;

  char **arglist = (char**)malloc( 3*sizeof(char*));
  arglist[0] = m_szUrlViewer;
  arglist[1] = (char*)u;
  arglist[2] = NULL;

  int pp[2];  
  if (pipe(pp) < 0) return false;

  switch (fork()) {
  case -1:    
    close(pp[0]);
    close(pp[1]);
    return false;
  case 0:
    close(pp[0]);

    // set close-on-exec flag
    fcntl(pp[1], F_SETFD, 1);

    execvp(arglist[0], arglist);

    // send a message indicating the failure
    write(pp[1], (char *) &errno, sizeof errno);
    _exit(-1);
  }
  free(arglist);

  close(pp[1]);
  int err;
  int n = read(pp[0], (char *) &err, sizeof err);
  close(pp[0]);

  return (n == 0);
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

void CICQDaemon::SetDirectMode()
{
  bool bDirect = (!m_bFirewall || (m_bFirewall && m_bTCPEnabled));
  CPacket::SetMode(bDirect ? MODE_DIRECT : MODE_INDIRECT);
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
  if (v_out < 2 || v_out == 5)
  {
    if (v_out == 5)
      v_out = 4;
    else
      v_out = 6;

    gLog.Warn("%sInvalid TCP version %d.  Attempting v%d.\n", L_WARNxSTR, v_in,
                                                              v_out);
  }
  return v_out;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//-----SaveUserList-------------------------------------------------------------
void CICQDaemon::SaveUserList()
{
  static const char suffix[] = ".new";
  static const char file[] = "users.conf";

  size_t nLen = strlen(BASE_DIR) + sizeof(file) + sizeof(suffix) + 2;
  char szTmpName[nLen], szFilename[nLen], buff[128];
  int nRet, n, fd;
 
  snprintf(szFilename, nLen, "%s/%s", BASE_DIR, file);
  szFilename[nLen - 1] = '\0';
  strcpy(szTmpName, szFilename);
  strcat(szTmpName, suffix);

  fd = open(szTmpName, O_WRONLY | O_CREAT | O_TRUNC, 00600);
  if (fd == -1)
  {
    // Avoid sending the message to the plugins, a race exists if we are
    // shutting down
    if (!m_bShuttingDown)
      gLog.Error("%sFailed updating %s: `%s'\n", L_ERRORxSTR,
                 szFilename, strerror(errno));
    return;
  }
     
  n = sprintf(buff, "[users]\nNumOfUsers = %d\n", gUserManager.NumUsers());
  nRet = write(fd, buff, n);

  unsigned short i = 1;
  //TODO: Work with other protocols
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_R)
  {
    n = sprintf(buff, "User%d = %s.Licq\n", i, pUser->IdString());
    nRet = write(fd, buff, n);
    if (nRet == -1)
      FOR_EACH_PROTO_USER_BREAK

    i++;
  }
  FOR_EACH_PROTO_USER_END

  close(fd);

  if (nRet != -1)
  {
    if (rename(szTmpName, szFilename))
      unlink(szTmpName);
  }
  else if (!m_bShuttingDown)
    gLog.Error("%sFailed updating %s: `%s'\n", L_ERRORxSTR,
               szFilename, strerror(errno));
}

void CICQDaemon::SetIgnore(unsigned short n, bool b)
{
  if (b)
    m_nIgnoreTypes |= n;
  else
    m_nIgnoreTypes &= ~n;
}

bool CICQDaemon::AddUserToList(const char *szId, unsigned long nPPID,
                               bool bNotify)
{
  // Don't add invalid uins
  if (szId == 0 || nPPID == 0) return false;

  // Don't add a user we already have
  if (gUserManager.IsOnList(szId, nPPID))
  {
    gLog.Warn("%sUser %s already on contact list.\n", L_WARNxSTR, szId);
    return false;
  }

  ICQUser *u = new ICQUser(szId, nPPID);
  gUserManager.AddUser(u);
  gUserManager.DropUser(u);
  SaveUserList();

  // this notify is for local only adds
  if (nPPID == LICQ_PPID && m_nTCPSrvSocketDesc != -1 && bNotify)
    icqAddUser(szId);

  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_ADD, szId, nPPID));

  return true;
}

//---AddUserToList-------------------------------------------------------------
/*! \brief Adds Uin to contact list
 *
 * Adds the given uin to the contact list.
 *
 * \return Returns true on success, else returns false. Please note that when 
 * this call returns the user is not locked.
 */
bool CICQDaemon::AddUserToList(unsigned long nUin, bool bNotify)
{
  // Don't add invalid uins
  if (nUin == 0) return false;

  // Don't add a user we already have
  if (gUserManager.IsOnList(nUin))
  {
    gLog.Warn("%sUser %lu already on contact list.\n", L_WARNxSTR, nUin);
    return false;
  }

  ICQUser *u = new ICQUser(nUin);
  gUserManager.AddUser(u);
  gUserManager.DropUser(u);
  SaveUserList();

  // this notify is for local only adds
  if (m_nTCPSrvSocketDesc != -1 && bNotify)  icqAddUser(nUin);

  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_ADD, nUin));

  return true;
}


//---AddUserToList-------------------------------------------------------------
/*! \brief Adds User to contact list
 *
 * Adds the given user to the contact list. NOTE: When this call returns the 
 * user is write locked and will need to be dropped! When calling this 
 * function it is important that the user not be locked in any way.
 *
 * \return Returns true on success, else returns false.
 */
void CICQDaemon::AddUserToList(ICQUser *nu)
{
  // Don't add a user we already have
  if (gUserManager.IsOnList(nu->Uin()))
  {
    gLog.Warn("%sUser %lu already on contact list.\n", L_WARNxSTR, nu->Uin());
    return;
  }

  gUserManager.AddUser(nu);
  // At this point the user is write locked
  SaveUserList();

  if (m_nTCPSrvSocketDesc != -1)
  {
    // XXX if adding to server list, it will get write lock FIX THAT SHIT!
    unsigned long nUin = nu->Uin();
    gUserManager.DropUser(nu);
    icqAddUser(nUin);
    nu = gUserManager.FetchUser(nUin, LOCK_W);
  }

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

void CICQDaemon::RemoveUserFromList(const char *szId, unsigned long nPPID)
{
  if (nPPID == LICQ_PPID && m_nTCPSrvSocketDesc != -1) icqRemoveUser(szId);
  
  gUserManager.RemoveUser(szId, nPPID);
  SaveUserList();
  
  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REMOVE, szId,
    nPPID));
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
                                    USER_STATUS, u->IdString(),
                                    u->PPID(), arg));
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
    fprintf(f, "Event from new user (%lu) rejected: \n%s\n--------------------\n\n",
            nUin, e->Text());
    chmod(m_szRejectFile, 00600);
    fclose(f);
  }
  delete e;
  m_sStats[STATS_EventsRejected].Inc();
}


//---CheckBirthdays------------------------------------------------------------
/*! \brief Sends a signal for each user whose birthday occurs in the next few days.
 *
 * This function is NOT IMPLEMENTED yet. Don't expect it to do anything usefull ;-P
 */
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

  if (e == NULL)  return;
 
  pthread_mutex_lock(&mutex_sendqueue_server);
  m_lxSendQueue_Server.push_back(e);
  pthread_mutex_unlock(&mutex_sendqueue_server);

  e->m_NoAck = true;
  int nResult = pthread_create(&e->thread_send, NULL, &ProcessRunningEvent_Server_tep, e);
  if (nResult != 0)
  {
    gLog.Error("%sUnable to start server event thread (#%lu):\n%s%s.\n", L_ERRORxSTR,
       e->m_nSequence, L_BLANKxSTR, strerror(nResult));
    e->m_eResult = EVENT_ERROR;
  }
#else
  SendEvent(m_nTCPSrvSocketDesc, *packet, true);
#endif
}

ICQEvent *CICQDaemon::SendExpectEvent_Server(const char *szId, unsigned long nPPID,
   CPacket *packet, CUserEvent *ue, bool bExtendedEvent)
{
  // If we are already shutting down, don't start any events
  if (m_bShuttingDown)
  {
    if (packet != NULL) delete packet;
    if (ue != NULL) delete ue;
    return NULL;
  }

  if (ue != NULL) ue->m_eDir = D_SENDER;
  ICQEvent *e = new ICQEvent(this, m_nTCPSrvSocketDesc, packet, CONNECT_SERVER, szId,
    LICQ_PPID, ue);

	if (e == NULL)  return NULL;

  if (bExtendedEvent) PushExtendedEvent(e);

  ICQEvent *result = SendExpectEvent(e, &ProcessRunningEvent_Server_tep);

  // if an error occured, remove the event from the extended queue as well
  if (result == NULL && bExtendedEvent)
  {
    pthread_mutex_lock(&mutex_extendedevents);
    std::list<ICQEvent *>::iterator i;
    for (i = m_lxExtendedEvents.begin(); i != m_lxExtendedEvents.end(); i++)
    {
      if (*i == e)
      {
        m_lxExtendedEvents.erase(i);
        break;
      }
    }
    pthread_mutex_unlock(&mutex_extendedevents);
  }

  return result;
}

ICQEvent *CICQDaemon::SendExpectEvent_Server(unsigned long nUin, CPacket *packet,
   CUserEvent *ue, bool bExtendedEvent)
{
  // If we are already shutting down, don't start any events
  if (m_bShuttingDown)
  {
    if (packet != NULL) delete packet;
    if (ue != NULL) delete ue;
    return NULL;
  }

  if (ue != NULL) ue->m_eDir = D_SENDER;
  ICQEvent *e = new ICQEvent(this, m_nTCPSrvSocketDesc, packet, CONNECT_SERVER, nUin, ue);

  if (e == NULL)  return NULL;

  if (bExtendedEvent) PushExtendedEvent(e);

  ICQEvent *result = SendExpectEvent(e, &ProcessRunningEvent_Server_tep);

  // if an error occured, remove the event from the extended queue as well
  if (result == NULL && bExtendedEvent)
  {
    pthread_mutex_lock(&mutex_extendedevents);
    std::list<ICQEvent *>::iterator i;
    for (i = m_lxExtendedEvents.begin(); i != m_lxExtendedEvents.end(); i++)
    {
      if (*i == e)
      {
        m_lxExtendedEvents.erase(i);
        break;
      }
    }
    pthread_mutex_unlock(&mutex_extendedevents);
  }

  return result;
}


ICQEvent *CICQDaemon::SendExpectEvent_Client(ICQUser *pUser, CPacket *packet,
   CUserEvent *ue)
{
  // If we are already shutting down, don't start any events
  if (m_bShuttingDown)
  {
    if (packet != NULL) delete packet;
    if (ue != NULL) delete ue;
    return NULL;
  }

  if (ue != NULL) ue->m_eDir = D_SENDER;
  ICQEvent *e = new ICQEvent(this, pUser->SocketDesc(), packet,
     CONNECT_USER, pUser->Uin(), ue);

  if (e == NULL) return NULL;

  return SendExpectEvent(e, &ProcessRunningEvent_Client_tep);
}


ICQEvent *CICQDaemon::SendExpectEvent(ICQEvent *e, void *(*fcn)(void *))
{
  // don't release the mutex until thread is running so that cancelling the
  // event cancels the thread as well
  pthread_mutex_lock(&mutex_runningevents);
  m_lxRunningEvents.push_back(e);

  assert(e);

  if (fcn == ProcessRunningEvent_Server_tep)
  {
    pthread_mutex_lock(&mutex_sendqueue_server);
    m_lxSendQueue_Server.push_back(e);
    pthread_mutex_unlock(&mutex_sendqueue_server); 
  }

  int nResult = pthread_create(&e->thread_send, NULL, fcn, e);
  if (fcn != ProcessRunningEvent_Server_tep)
    e->thread_running = true;
  pthread_mutex_unlock(&mutex_runningevents);

  if (nResult != 0)
  {
    gLog.Error("%sUnable to start event thread (#%lu):\n%s%s.\n", L_ERRORxSTR,
       e->m_nSequence, L_BLANKxSTR, strerror(nResult));
    DoneEvent(e, EVENT_ERROR);
    if (e->m_nSocketDesc == m_nTCPSrvSocketDesc)
    {
      pthread_mutex_lock(&mutex_sendqueue_server);
      list<ICQEvent *>::iterator iter;
      for (iter = m_lxSendQueue_Server.begin();
           iter != m_lxSendQueue_Server.end(); iter++)
      {
        if (e == *iter)
        {
          m_lxSendQueue_Server.erase(iter);

          ICQEvent *cancelled = new ICQEvent(e);
          cancelled->m_bCancelled = true;
          m_lxSendQueue_Server.push_back(cancelled);
          break;
        }
      }
      pthread_mutex_unlock(&mutex_sendqueue_server);
    }
    ProcessDoneEvent(e);
    return NULL;
  }

  return (e);
}


//---SendEvent-----------------------------------------------------------------
/*! \brief Sends an event without expecting a reply
 *
 * Sends an event without expecting a reply, does not create an event
 * structure, and does not attempt a connection if the socket is invalid. 
 * Can possibly block on send, but this is ok as it is never called from the 
 * gui thread. 
 * Note that the user who owns the given socket is probably read-locked at 
 * this point.
 *
 * \return Returns true on success, else returns false
 */
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


//---FailEvents----------------------------------------------------------------
/*! \brief Fails all events on the given socket. */
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


//---DoneSrvEvent--------------------------------------------------------------
/*! \brief Marks the given event as done.
 *
 * Marks the given event as done and removese it from the running events list. 
 * This is for new OSCAR server events. 
 * Basically this is DoneEvent (2)
 */
ICQEvent *CICQDaemon::DoneServerEvent(unsigned long _nSubSeq, EventResult _eResult)
{
  pthread_mutex_lock(&mutex_runningevents);
  ICQEvent *e = NULL;
  list<ICQEvent *>::iterator iter;
  for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); iter++)
  {
    if ((*iter)->CompareSubSequence(_nSubSeq) )
    {
      e = *iter;
      m_lxRunningEvents.erase(iter);
      // Check if we should cancel a processing thread
      if (e->thread_running && !pthread_equal(e->thread_send, pthread_self()))
      {
        pthread_mutex_lock(&mutex_cancelthread);
        pthread_cancel(e->thread_send);
        pthread_mutex_unlock(&mutex_cancelthread);
        e->thread_running = false;
      }
      break;
    }
  }
  pthread_mutex_unlock(&mutex_runningevents);

  // If we didn't find the event, it must have already been removed, we are too late

  if (e == NULL) return (NULL);

  e->m_eResult = _eResult;

  return(e);
}

//---DoneEvent-----------------------------------------------------------------
/*! \brief Marks the given event as done.
 *
 * Marks the given event as done and removes it from the running events list.
 */
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
      // Check if we should cancel a processing thread
      if (e->thread_running && !pthread_equal(e->thread_send, pthread_self()))
      {
        pthread_mutex_lock(&mutex_cancelthread);
        pthread_cancel(e->thread_send);
        pthread_mutex_unlock(&mutex_cancelthread);
        e->thread_running = false;
      }
      break;
    }
  }

#if 0
  if (m_lxRunningEvents.size()) {
    gLog.Info("doneevents: for: %p pending: \n", e);
    for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); iter++)
    {
      gLog.Info("%p Command: %d SubCommand: %d Sequence: %lu SubSequence: %d: Uin: %lu\n", *iter,
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
      // Check if we should cancel a processing thread
      if (e->thread_running && !pthread_equal(e->thread_send, pthread_self()))
      {
        pthread_mutex_lock(&mutex_cancelthread);
        pthread_cancel(e->thread_send);
        pthread_mutex_unlock(&mutex_cancelthread);
        e->thread_running = false;
      }
      break;
    }
  }
  pthread_mutex_unlock(&mutex_runningevents);

  // If we didn't find the event, it must have already been removed, we are too late
  if (e == NULL) return (NULL);

  e->m_eResult = _eResult;

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
      // Check if we should cancel a processing thread
      if (e->thread_running && !pthread_equal(e->thread_send, pthread_self()))
      {
        pthread_mutex_lock(&mutex_cancelthread);
        pthread_cancel(e->thread_send);
        pthread_mutex_unlock(&mutex_cancelthread);
        e->thread_running = false;
      }
      break;
    }
  }
  pthread_mutex_unlock(&mutex_runningevents);

  // If we didn't find the event, it must have already been removed, we are too late
  if (e == NULL) return (NULL);

  e->m_eResult = _eResult;

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
  gLog.Info("%p pushing Command: %d SubCommand: %d Sequence: %lu SubSequence: %d: Uin: %lu\n", e,
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

void CICQDaemon::PushProtoSignal(CSignal *s, unsigned long _nPPID)
{
  ProtoPluginsListIter iter;
  pthread_mutex_lock(&licq->mutex_protoplugins);
  for (iter = licq->list_protoplugins.begin(); iter != licq->list_protoplugins.end();
       iter++)
  {
    if ((*iter)->Id() == _nPPID)
    {
      (*iter)->PushSignal(s);
      break;
    }
  }
  pthread_mutex_unlock(&licq->mutex_protoplugins);
}

CSignal *CICQDaemon::PopProtoSignal()
{
  ProtoPluginsListIter iter;
  CSignal *s = NULL;
  pthread_mutex_lock(&licq->mutex_protoplugins);
  for (iter = licq->list_protoplugins.begin(); iter != licq->list_protoplugins.end();
       iter++)
  {
    if ((*iter)->CompareThread(pthread_self()))
    {
      s = (*iter)->PopSignal();
      break;
    }
  }
  pthread_mutex_unlock(&licq->mutex_protoplugins);
  return s;
}

//-----CICQDaemon::CancelEvent---------------------------------------------------------
void CICQDaemon::CancelEvent(unsigned long t)
{
  ICQEvent *eSrv = NULL;
  pthread_mutex_lock(&mutex_sendqueue_server);
  list<ICQEvent *>::iterator iter;
  for (iter = m_lxSendQueue_Server.begin();
       iter != m_lxSendQueue_Server.end(); iter++)
  {
    if ((*iter)->Equals(t))
    {
      eSrv = *iter;
      m_lxSendQueue_Server.erase(iter);

      ICQEvent *cancelled = new ICQEvent(eSrv);
      cancelled->m_bCancelled = true;
      m_lxSendQueue_Server.push_back(cancelled);
      break;
    }
  }
  pthread_mutex_unlock(&mutex_sendqueue_server);

  ICQEvent *eRun = DoneEvent(t, EVENT_CANCELLED);
  ICQEvent *eExt = DoneExtendedEvent(t, EVENT_CANCELLED);

  if (eRun == NULL && eExt == NULL && eSrv == NULL)
  {
    gLog.Warn("%sCancelled event not found.\n", L_WARNxSTR);
    return;
  }

  CancelEvent((eRun != NULL)? eRun : (eExt != NULL)? eExt : eSrv);
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


//-----ProcessMessage-----------------------------------------------------------
void CICQDaemon::ProcessMessage(ICQUser *u, CBuffer &packet, char *message,
                                unsigned short nMsgType, unsigned long nMask,
                                unsigned long nMsgID[2],
                                unsigned long nSequence, bool bIsAck,
                                bool &bNewUser)
{
  char *szType = NULL;
  CUserEvent *pEvent = NULL;
  unsigned short nEventType = 0;

  // for acks
  unsigned short nPort;

  // Do we accept it if we are in Occ or DND?
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short nOwnerStatus = o->Status();
  gUserManager.DropOwner();

  unsigned short nLevel;
  if (nMask & ICQ_TCPxMSG_URGENT)
    nLevel = ICQ_TCPxMSG_URGENT2;
  else if (nMask & ICQ_TCPxMSG_LIST)
    nLevel = ICQ_TCPxMSG_LIST2;
  else
    nLevel = nMask;

  bool bAccept = nMask & ICQ_TCPxMSG_URGENT || nMask & ICQ_TCPxMSG_LIST;
  // Flag as sent urgent as well if we are in occ or dnd and auto-accept is on
  if ( ((nOwnerStatus == ICQ_STATUS_OCCUPIED || u->StatusToUser() == ICQ_STATUS_OCCUPIED)
         && u->AcceptInOccupied() ) ||
       ((nOwnerStatus == ICQ_STATUS_DND || u->StatusToUser() == ICQ_STATUS_DND)
         && u->AcceptInDND() ) ||
       (u->StatusToUser() != ICQ_STATUS_OFFLINE && u->StatusToUser() != ICQ_STATUS_OCCUPIED
         && u->StatusToUser() != ICQ_STATUS_DND) )
  {
    bAccept = true;
    nLevel = ICQ_TCPxMSG_URGENT2;
  }

  unsigned long nFlags = ((nMask & ICQ_CMDxSUB_FxMULTIREC) ? E_MULTIxREC : 0)
                         | ((nMask & ICQ_TCPxMSG_URGENT) ? E_URGENT : 0);

  switch (nMsgType)
  {
  case ICQ_CMDxSUB_MSG:
  {
    unsigned long fore, back;
    packet >> fore >> back;
    if (back == fore)
    {
      back = 0xFFFFFF;
      fore = 0x000000;
    }

    CEventMsg *e = CEventMsg::Parse(message, ICQ_CMDxRCV_SYSxMSGxONLINE,
                                    TIME_NOW, nFlags);
    e->SetColor(fore, back);

    CPU_AckGeneral *p = new CPU_AckGeneral(u, nMsgID[0], nMsgID[1],
                                           nSequence, ICQ_CMDxSUB_MSG, true,
                                           nLevel);
    SendEvent_Server(p);

    szType = strdup("Message");
    nEventType = ON_EVENT_MSG;
    pEvent = e;
    break;
  }

  case ICQ_CMDxSUB_CHAT:
  {
    char szChatClients[1024];
    unsigned short nPortReversed;

    gTranslator.ServerToClient(message);

    packet.UnpackString(szChatClients, sizeof(szChatClients));
    nPortReversed = packet.UnpackUnsignedShortBE();
    packet.incDataPosRead(2);
    nPort = packet.UnpackUnsignedShort();

    if (nPort == 0)
      nPort = nPortReversed;

    if (!bIsAck)
    {
      CEventChat *e = new CEventChat(message, szChatClients, nPort, nSequence,
                                     TIME_NOW, nFlags, nMsgID[0], nMsgID[1]);
      nEventType = ON_EVENT_CHAT;
      pEvent = e;
    }

    szType = strdup("Chat request");
    break;
  }

  case ICQ_CMDxSUB_FILE:
  {
    unsigned short nFilenameLen, nPortReversed;
    unsigned long nFileSize;

    gTranslator.ServerToClient(message);

    nPortReversed = packet.UnpackUnsignedShortBE(); /* this is garbage when
                                                      the request is refused */
    packet.UnpackUnsignedShort();

    packet >> nFilenameLen;
    if (!bIsAck)
    {
      char szFilename[nFilenameLen+1];
      for (unsigned short i = 0; i < nFilenameLen; i++)
        packet >> szFilename[i];
      szFilename[nFilenameLen] = '\0'; // be safe
      packet >> nFileSize;
      ConstFileList filelist;
      filelist.push_back(strdup(szFilename));

      CEventFile *e = new CEventFile(szFilename, message, nFileSize,
                                     filelist, nSequence, TIME_NOW, nFlags,
                                     nMsgID[0], nMsgID[1]);
      nEventType = ON_EVENT_FILE;
      pEvent = e;
    }
    else
      packet.incDataPosRead(nFilenameLen + 4);

    packet >> nPort;

    szType = strdup("File transfer request through server");
    break;
  }

  case ICQ_CMDxSUB_URL:
  {
    CEventUrl *e = CEventUrl::Parse(message, ICQ_CMDxRCV_SYSxMSGxONLINE,
                                    TIME_NOW, nFlags);
    CPU_AckGeneral *p = new CPU_AckGeneral(u, nMsgID[0], nMsgID[1],
                                           nSequence, ICQ_CMDxSUB_URL, true,
                                           nLevel);
    SendEvent_Server(p);

    szType = strdup("URL");
    nEventType = ON_EVENT_URL;
    pEvent = e;
    break;
  }

  case ICQ_CMDxSUB_CONTACTxLIST:
  {
    CEventContactList *e = CEventContactList::Parse(message,
                                                    ICQ_CMDxRCV_SYSxMSGxONLINE,
                                                    TIME_NOW, nFlags);
    CPU_AckGeneral *p = new CPU_AckGeneral(u, nMsgID[0], nMsgID[1],
                                           nSequence, ICQ_CMDxSUB_CONTACTxLIST,
                                           true, nLevel);
    SendEvent_Server(p);

    szType = strdup("Contact list");
    nEventType = ON_EVENT_MSG;
    pEvent = e;
    break;
  }

  case ICQ_CMDxTCP_READxNAxMSG:
  case ICQ_CMDxTCP_READxDNDxMSG:
  case ICQ_CMDxTCP_READxOCCUPIEDxMSG:
  case ICQ_CMDxTCP_READxFFCxMSG:
  case ICQ_CMDxTCP_READxAWAYxMSG:
  {
    if (bIsAck)
    {
      if (strcmp(u->AutoResponse(), message))
      {
        u->SetAutoResponse(message);
        u->SetShowAwayMsg(*message);
        gLog.Info("%sAuto response from %s (#%lu).\n", L_SRVxSTR, u->GetAlias(),
                  nMsgID[1]);
      }
      ICQEvent *e = DoneServerEvent(nMsgID[1], EVENT_ACKED);
      if (e)
      {
        e->m_pExtendedAck = new CExtendedAck(true, 0, message);
        e->m_nSubResult = ICQ_TCPxACK_RETURN;
        ProcessDoneEvent(e);
      }
      else
        gLog.Warn("%sAck for unknown event.\n", L_SRVxSTR);
    }
    else
    {
      gLog.Info("%s%s (%lu) requested auto response.\n", L_SRVxSTR,
                u->GetAlias(), u->Uin());

    CPU_AckGeneral *p = new CPU_AckGeneral(u, nMsgID[0], nMsgID[1],
                                           nSequence, nMsgType, true, nLevel);
    SendEvent_Server(p);

    m_sStats[STATS_AutoResponseChecked].Inc();
    u->SetLastCheckedAutoResponse();

      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_EVENTS,
                       u->Uin()));
    }
    return;
    
    break; // bah!
  }

  case ICQ_CMDxSUB_ICBM:
  {
    unsigned short nLen;
    unsigned long nLongLen;

    packet >> nLen;
    packet.incDataPosRead(18);
    packet >> nLongLen; // plugin len
    char szPlugin[nLongLen+1];
    for (unsigned long i = 0; i < nLongLen; i++)
      packet >> szPlugin[i];
    szPlugin[nLongLen] = '\0';

    packet.incDataPosRead(nLen - 22 - nLongLen); // unknown
    packet >> nLongLen; // bytes remaining

    int nCommand = 0;
    if (strstr(szPlugin, "File"))
      nCommand = ICQ_CMDxSUB_FILE;
    else if (strstr(szPlugin, "URL"))
      nCommand = ICQ_CMDxSUB_URL;
    else if (strstr(szPlugin, "Chat"))
      nCommand = ICQ_CMDxSUB_CHAT;
    else if (strstr(szPlugin, "Contacts"))
      nCommand = ICQ_CMDxSUB_CONTACTxLIST;

    if (nCommand == 0)
    {
      gLog.Warn("%sUnknown ICBM plugin type: %s\n", L_SRVxSTR, szPlugin);
      return;
    }

    packet >> nLongLen;
    char szMessage[nLongLen+1];
    for (unsigned long i = 0; i < nLongLen; i++)
      packet >> szMessage[i];
    szMessage[nLongLen] = '\0';

    /* if the auto response is non empty then this is a decline and we want
       to show the auto response rather than our original message */
    char *msg = (message[0] != '\0') ? message : szMessage;

    // recursion
    ProcessMessage(u, packet, msg, nCommand, nMask, nMsgID,
                   nSequence, bIsAck, bNewUser);
    return;

    break; // bah!
  }

  default:
    gTranslator.ServerToClient(message);
    szType = strdup("unknown event");
  } // switch nMsgType

  if (bIsAck)
  {
    ICQEvent *pAckEvent = DoneServerEvent(nMsgID[1], EVENT_ACKED);
    CExtendedAck *pExtendedAck = new CExtendedAck(true, nPort, message);

    if (pAckEvent)
    {
      pAckEvent->m_pExtendedAck = pExtendedAck;
      pAckEvent->m_nSubResult = ICQ_TCPxACK_ACCEPT;
      gLog.Info("%s%s accepted from %s (%lu).\n", L_SRVxSTR, szType,
                u->GetAlias(), u->Uin());
      gUserManager.DropUser(u);
      ProcessDoneEvent(pAckEvent);
    }
    else
    {
      gLog.Warn("%sAck for unknown event.\n", L_SRVxSTR);
      delete pExtendedAck;
     }
  }
  else
  {
    // If it parsed, did it parse properly?
    if (pEvent)
    {
      // If we are in DND or Occupied and message isn't urgent then we ignore it
      if (!bAccept)
      {
        if (nOwnerStatus == ICQ_STATUS_OCCUPIED || nOwnerStatus == ICQ_STATUS_DND)
        {
          delete pEvent;
          if (szType) free(szType);
          return;
        }
      }

      if (bNewUser)
      {
        if (Ignore(IGNORE_NEWUSERS))
        {
          gLog.Info("%s%s from new user (%lu), ignoring.\n", L_SRVxSTR,
                    szType, u->Uin());
          if (szType)  free(szType);
          RejectEvent(u->Uin(), pEvent);
          return;
        }
        gLog.Info("%s%s from new user (%lu).\n", L_SRVxSTR, szType, u->Uin());
        AddUserToList(u);
        bNewUser = false;
      }
      else
        gLog.Info("%s%s from %s (%lu).\n", L_SRVxSTR, szType, u->GetAlias(),
                  u->Uin());

      if (AddUserEvent(u, pEvent))
        m_xOnEventManager.Do(nEventType, u);
    }
    else // invalid parse or unknown event
    {
      char *buf;
      gLog.Warn("%sInvalid %s:\n%s\n", L_WARNxSTR, szType, packet.print(buf));
      delete [] buf;
    }
  }

  if (szType)  free(szType);
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

