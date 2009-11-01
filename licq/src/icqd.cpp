// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "time-fix.h"

// Localization
#include "gettext.h"

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#include "licq_icq.h"
#include "licq_user.h"
#include "licq_oscarservice.h"
#include "licq_constants.h"
#include "licq_file.h"
#include "licq_log.h"
#include "licq_translate.h"
#include "licq_packets.h"
#include "licq_plugind.h"
#include "licq_proxy.h"
#include "licq_gpg.h"    // ##
#include "licq.h"
#include "support.h"

#include "licq_icqd.h"

using namespace std;

std::list <CReverseConnectToUserData *> CICQDaemon::m_lReverseConnect;
pthread_mutex_t CICQDaemon::mutex_reverseconnect = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  CICQDaemon::cond_reverseconnect_done = PTHREAD_COND_INITIALIZER;

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

CDaemonStats::~CDaemonStats()
{
  // Empty
}

void CDaemonStats::Reset()
{
  m_nTotal = m_nOriginal = 0;
}

void CDaemonStats::Init()
{
  m_nOriginal = m_nLastSaved = m_nTotal;
}


unsigned long CConversation::s_nCID = 0;
pthread_mutex_t CConversation::s_xMutex = PTHREAD_MUTEX_INITIALIZER;

CConversation::CConversation(int nSocket, unsigned long nPPID)
{
  m_nSocket = nSocket;
  m_nPPID = nPPID;
  pthread_mutex_lock(&s_xMutex);
  m_nCID = ++s_nCID;
  pthread_mutex_unlock(&s_xMutex);
}

CConversation::~CConversation()
{
  // Empty
}

bool CConversation::HasUser(const char *szUser)
{
  return (std::find(m_vUsers.begin(), m_vUsers.end(), szUser) != m_vUsers.end()); 
}

bool CConversation::AddUser(const char *szUser)
{
  if (!HasUser(szUser))
  {
    m_vUsers.push_back(szUser);
    return true;
  }
  else
    return false;
}

bool CConversation::RemoveUser(const char *szUser)
{
  if (HasUser(szUser))
  {
    string strUser(szUser);
    vector<string>::iterator i;
    for (i = m_vUsers.begin(); i != m_vUsers.end(); ++i)
    {
      if (*i == strUser)
      {
        m_vUsers.erase(i);
        return true;
      }
    }
  }
  
  return false;
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
  m_bAutoUpdateInfo = m_bAutoUpdateInfoPlugins = m_bAutoUpdateStatusPlugins
                    = true;
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
  m_bVerify = false;
  m_bNeedSalt = true;
  m_szRegisterPasswd = 0;
  m_nRegisterThreadId = 0;

  fifo_fs = NULL;

  receivedUserList.clear();

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
  licqConf.ReadBool("AutoUpdateInfo", m_bAutoUpdateInfo, true);
  licqConf.ReadBool("AutoUpdateInfoPlugins", m_bAutoUpdateInfoPlugins, true);
  licqConf.ReadBool("AutoUpdateStatusPlugins", m_bAutoUpdateStatusPlugins,
    true);
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

  // Services
  m_xBARTService = NULL;

  // Misc
  licqConf.ReadBool("UseSS", m_bUseSS, true); // server side list
  licqConf.ReadBool("UseBART", m_bUseBART, true); // server side buddy icons
  licqConf.ReadBool("SendTypingNotification", m_bSendTN, true);
  licqConf.ReadBool("ReconnectAfterUinClash", m_bReconnectAfterUinClash, false);

  // -----OnEvent configuration-----
  string onEventCommand, onEventParams[MAX_ON_EVENT];
  unsigned short nOnEventCmdType;

  licqConf.SetSection("onevent");
  licqConf.ReadNum("Enable", nOnEventCmdType, 0);
  licqConf.ReadBool("AlwaysOnlineNotify", m_bAlwaysOnlineNotify, false);
  m_xOnEventManager.SetCommandType(nOnEventCmdType);

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

  licqConf.readString("Command", onEventCommand, "play");
  licqConf.readString("Message", onEventParams[ON_EVENT_MSG], DEF_MESSAGE);
  licqConf.readString("Url", onEventParams[ON_EVENT_URL], DEF_URL);
  licqConf.readString("Chat", onEventParams[ON_EVENT_CHAT], DEF_CHAT);
  licqConf.readString("File", onEventParams[ON_EVENT_FILE], DEF_FILE);
  licqConf.readString("OnlineNotify", onEventParams[ON_EVENT_NOTIFY], DEF_NOTIFY);
  licqConf.readString("SysMsg", onEventParams[ON_EVENT_SYSMSG], DEF_SYSMSG);
  licqConf.readString("MsgSent", onEventParams[ON_EVENT_MSGSENT], DEF_MSGSENT);
  m_xOnEventManager.setParameters(onEventCommand, onEventParams);

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
    for (iter = m_sStats.begin(); iter != m_sStats.end(); ++iter)
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

  // start GPG helper
  gGPGHelper.Start();

  // Start up our threads
  pthread_mutex_init(&mutex_runningevents, NULL);
  pthread_mutex_init(&mutex_extendedevents, NULL);
  pthread_mutex_init(&mutex_sendqueue_server, NULL);
  pthread_mutex_init(&mutex_modifyserverusers, NULL);
  pthread_mutex_init(&mutex_cancelthread, NULL);
  pthread_cond_init(&cond_serverack, NULL);
  pthread_mutex_init(&mutex_serverack, NULL);
  pthread_mutex_init(&mutex_conversations, NULL);
}


bool CICQDaemon::Start()
{
  char sz[MAX_FILENAME_LEN];
  int nResult = 0;

  TCPSocket* s = new TCPSocket();
  m_nTCPSocketDesc = StartTCPServer(s);
  if (m_nTCPSocketDesc == -1)
  {
     gLog.Error(tr("%sUnable to allocate TCP port for local server (%s)!\n"),
                L_ERRORxSTR, tr("No ports available"));
     return false;
  }
  gSocketManager.AddSocket(s);
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  if (o != NULL)
  {
    o->SetIntIp(s->getLocalIpInt());
    o->SetPort(s->getLocalPort());
    gUserManager.DropOwner(o);
  }
  CPacket::SetLocalPort(s->getLocalPort());
  gSocketManager.DropSocket(s);


#ifdef USE_FIFO
  // Open the fifo
  snprintf(sz, MAX_FILENAME_LEN, "%s/licq_fifo", BASE_DIR);
  sz[MAX_FILENAME_LEN - 1] = '\0';
  gLog.Info(tr("%sOpening fifo.\n"), L_INITxSTR);
  fifo_fd = open(sz, O_RDWR);
  if (fifo_fd == -1)
  {
    if (mkfifo(sz, 00600) == -1)
      gLog.Warn(tr("%sUnable to create fifo:\n%s%s.\n"), L_WARNxSTR, L_BLANKxSTR, strerror(errno));
    else
    {
      fifo_fd = open(sz, O_RDWR);
      if (fifo_fd == -1)
        gLog.Warn(tr("%sUnable to open fifo:\n%s%s.\n"), L_WARNxSTR, L_BLANKxSTR, strerror(errno));
    }
  }
  fifo_fs = NULL;
  if (fifo_fd != -1)
  {
    struct stat buf;
    fstat(fifo_fd, &buf);
    if (!S_ISFIFO(buf.st_mode))
    {
      gLog.Warn(tr("%s%s is not a FIFO, disabling fifo support.\n"), L_WARNxSTR, sz);
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

  gLog.Info(tr("%sSpawning daemon threads.\n"), L_INITxSTR);
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
  
  nResult = pthread_create(&thread_updateusers, NULL, &UpdateUsers_tep, this);
  if (nResult != 0)
  {
    gLog.Error("%sUnable to start users update thread:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
    return false;
  }

  if (UseServerSideBuddyIcons())
  {
    m_xBARTService = new COscarService(this, ICQ_SNACxFAM_BART);
    nResult = pthread_create(&thread_ssbiservice, NULL,
                             &OscarServiceSendQueue_tep, m_xBARTService);
    if (nResult != 0)
    {
      gLog.Error(tr("%sUnable to start BART service thread:\n%s%s.\n"),
                 L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
      return false;
    }
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
       ++iter)
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
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); ++iter)
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
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); ++iter)
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
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); ++iter)
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
  // Check to make sure it's not already loaded
  CProtoPlugin *p = licq->LoadProtoPlugin(szPlugin);
  if (p == NULL) return false;

  pthread_mutex_lock(&licq->mutex_protoplugins);
  licq->StartProtoPlugin(p);
  pthread_mutex_unlock(&licq->mutex_protoplugins);

  return true;
}

int CICQDaemon::RegisterProtoPlugin()
{
  int p = -1;

  pthread_mutex_lock(&licq->mutex_protoplugins);
  ProtoPluginsListIter it;
  for (it = licq->list_protoplugins.begin();
       it != licq->list_protoplugins.end();
       it++)
  {
    if ((*it)->CompareThread(pthread_self()))
    {
      p = (*it)->Pipe();
      (*it)->SetSignals(true);
      break;
    }
  }
  pthread_mutex_unlock(&licq->mutex_protoplugins);

  if (p == -1)
    gLog.Error("%sInvalid thread in registration attempt.\n", L_ERRORxSTR);

  return p;
}

void CICQDaemon::UnregisterProtoPlugin()
{
  ProtoPluginsListIter iter;
  pthread_mutex_lock(&licq->mutex_protoplugins);
  for (iter = licq->list_protoplugins.begin();
       iter != licq->list_protoplugins.end();
       ++iter)
  {
    if ((*iter)->CompareThread(pthread_self()))
    {
      //gLog.Info("%sUnregistering plugin %d.\n", L_ENDxSTR, (*iter)->Id());
      (*iter)->SetSignals(false);
      break;
    }
  }
  pthread_mutex_unlock(&licq->mutex_protoplugins);
}

const char* CICQDaemon::ProtoPluginName(unsigned long _nPPID) const
{
  ProtoPluginsListIter it;
  const char* p = 0;

  pthread_mutex_lock(&licq->mutex_protoplugins);
  for (it = licq->list_protoplugins.begin();
       it != licq->list_protoplugins.end();
       it++)
  {
    if ((*it)->PPID() == _nPPID)
    {
      p = (*it)->Name();
      break;
    }
  }
  pthread_mutex_unlock(&licq->mutex_protoplugins);
  return p;
}

//---ProtoPluginShutdown-------------------------------------------------------
/*! \brief Unloads the given proto plugin */
void CICQDaemon::ProtoPluginShutdown(unsigned short _nId)
{
  ProtoPluginsListIter iter;
  pthread_mutex_lock(&licq->mutex_protoplugins);
  for (iter = licq->list_protoplugins.begin(); iter != licq->list_protoplugins.end(); ++iter)
  {
    if ((*iter)->Id() == _nId) (*iter)->Shutdown();
  }
  pthread_mutex_unlock(&licq->mutex_protoplugins);
}

//---Version-------------------------------------------------------------------
/*! \brief Returns the version of Licq */
const char* CICQDaemon::Version() const
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
  for (iter = m_sStats.begin(); iter != m_sStats.end(); ++iter)
  {
    if (iter->Dirty()) break;
  }
  if (iter == m_sStats.end()) return;

  // Save the stats
  CIniFile licqConf(INI_FxALLOWxCREATE);
  if (!licqConf.LoadFile(m_szConfigFile)) return;
  licqConf.SetSection("stats");
  licqConf.WriteNum("Reset", (unsigned long)m_nResetTime);
  for (iter = m_sStats.begin(); iter != m_sStats.end(); ++iter)
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
  for (iter = m_sStats.begin(); iter != m_sStats.end(); ++iter)
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
  licqConf.WriteBool("AutoUpdateInfo", m_bAutoUpdateInfo);
  licqConf.WriteBool("AutoUpdateInfoPlugins", m_bAutoUpdateInfoPlugins);
  licqConf.WriteBool("AutoUpdateStatusPlugins", m_bAutoUpdateStatusPlugins);
  licqConf.WriteNum("ForegroundColor", CICQColor::DefaultForeground());
  licqConf.WriteNum("BackgroundColor", CICQColor::DefaultBackground());


  // Utility tab
  licqConf.WriteStr("UrlViewer", m_szUrlViewer);
  const char* pc = gTranslator.getMapName();
  if (pc == NULL)
    pc = "none";
  else
  {
    const char* pc2 = strrchr(pc, '/');
    if (pc2 != NULL)
      pc = pc2++;
  }
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
  licqConf.WriteBool("UseBART", m_bUseBART); // server side buddy icons
  licqConf.WriteBool("SendTypingNotification", m_bSendTN); 
  licqConf.WriteBool("ReconnectAfterUinClash", m_bReconnectAfterUinClash);
  licqConf.WriteStr("DefaultUserEncoding", gUserManager.DefaultUserEncoding());

  // save the sound stuff
  licqConf.SetSection("onevent");
  COnEventManager *oem = OnEventManager();
  licqConf.WriteNum("Enable", oem->CommandType());
  licqConf.WriteBool("AlwaysOnlineNotify", m_bAlwaysOnlineNotify);
  oem->Lock();
  licqConf.writeString("Command", oem->command());
  licqConf.writeString("Message", oem->parameter(ON_EVENT_MSG));
  licqConf.writeString("Url", oem->parameter(ON_EVENT_URL));
  licqConf.writeString("Chat",oem->parameter(ON_EVENT_CHAT));
  licqConf.writeString("File",oem->parameter(ON_EVENT_FILE));
  licqConf.writeString("OnlineNotify", oem->parameter(ON_EVENT_NOTIFY));
  licqConf.writeString("SysMsg", oem->parameter(ON_EVENT_SYSMSG));
  licqConf.writeString("MsgSent", oem->parameter(ON_EVENT_MSGSENT));
  oem->Unlock();

  licqConf.FlushFile();

  licqConf.SetSection("owners");
  licqConf.WriteNum("NumOfOwners", (unsigned long)gUserManager.NumOwners());

  int n = 1;
  FOR_EACH_OWNER_START(LOCK_R)
  {
    char szOwnerId[12], szOwnerPPID[14];
    char szPPID[5];
    sprintf(szOwnerId, "Owner%d.Id", n);
    sprintf(szOwnerPPID, "Owner%d.PPID", n++);

    szPPID[0] = (pOwner->PPID() & 0xFF000000) >> 24;
    szPPID[1] = (pOwner->PPID() & 0x00FF0000) >> 16;
    szPPID[2] = (pOwner->PPID() & 0x0000FF00) >> 8;
    szPPID[3] = (pOwner->PPID() & 0x000000FF);
    szPPID[4] = '\0';

    pOwner->SaveLicqInfo();
    if (strcmp(pOwner->IdString(), "0") != 0)
    {
      licqConf.WriteStr(szOwnerId, pOwner->IdString());
      licqConf.WriteStr(szOwnerPPID, szPPID);
    }
  }
  FOR_EACH_OWNER_END

  licqConf.FlushFile();
}

bool CICQDaemon::haveGpgSupport() const
{
#ifdef HAVE_LIBGPGME
  return true;
#else
  return false;
#endif
}

//++++++NOT MT SAFE+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CICQDaemon::SetTerminal(const char *s)  { SetString(&m_szTerminal, s); }
void CICQDaemon::SetAlwaysOnlineNotify(bool b)  { m_bAlwaysOnlineNotify = b; }

const char* CICQDaemon::getUrlViewer() const
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
    gLog.Info(tr("%sLocal TCP server started on port %d.\n"), L_TCPxSTR, s->getLocalPort());
  }
  else if (s->Error() == EADDRINUSE)
  {
    gLog.Warn(tr("%sNo ports available for local TCP server.\n"), L_WARNxSTR);
  }
  else
  {
    gLog.Warn(tr("%sFailed to start local TCP server:\n%s%s\n"), L_WARNxSTR,
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
    gLog.Warn(tr("%sTCP high port (%d) is lower then TCP low port (%d).\n"),
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
  m_xProxy = CreateProxy();
}

ProxyServer *CICQDaemon::CreateProxy()
{
  ProxyServer *Proxy = NULL;

  switch (m_nProxyType)
  {
    case PROXY_TYPE_HTTP :
      Proxy = new HTTPProxyServer();
      break;
    default:
      break;
  }

  if (Proxy != NULL)
  {
    gLog.Info(tr("%sResolving proxy: %s:%d...\n"), L_INITxSTR, m_szProxyHost, m_nProxyPort);
    if (!Proxy->SetProxyAddr(m_szProxyHost, m_nProxyPort))
    {
      char buf[128];

      gLog.Warn(tr("%sUnable to resolve proxy server %s:\n%s%s.\n"), L_ERRORxSTR,
                m_szProxyHost, L_BLANKxSTR, Proxy->ErrorStr(buf, 128));
      delete Proxy;
      Proxy = NULL;
    }

    if (Proxy)
    {
      if (m_bProxyAuthEnabled)
        Proxy->SetProxyAuth(m_szProxyLogin, m_szProxyPasswd);

      Proxy->InitProxy();
    }
  }

  return Proxy;
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

    gLog.Warn(tr("%sInvalid TCP version %d.  Attempting v%d.\n"), L_WARNxSTR, v_in,
                                                              v_out);
  }
  return v_out;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CICQDaemon::SetIgnore(unsigned short n, bool b)
{
  if (b)
    m_nIgnoreTypes |= n;
  else
    m_nIgnoreTypes &= ~n;
}

void CICQDaemon::SetUseServerSideBuddyIcons(bool b)
{
  if (b && m_xBARTService == NULL)
  {
    m_xBARTService = new COscarService(this, ICQ_SNACxFAM_BART);
    int nResult = pthread_create(&thread_ssbiservice, NULL,
                                 &OscarServiceSendQueue_tep, m_xBARTService);
    if (nResult != 0)
    {
      gLog.Error(tr("%sUnable to start BART service thread:\n%s%s.\n"),
                 L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
    }
    else
      m_bUseBART = true;
  }
  else
    m_bUseBART = b;
}

//-----ChangeUserStatus-------------------------------------------------------
void CICQDaemon::ChangeUserStatus(ICQUser *u, unsigned long s)
{
  unsigned long oldstatus = u->StatusFull();
  int arg = 0;
  
  if (oldstatus == ICQ_STATUS_OFFLINE)
    u->SetUserUpdated(false);
    
  if (s == ICQ_STATUS_OFFLINE)
  {
    if (!u->StatusOffline()) arg = -1;
    u->SetStatusOffline();
  }
  else
  {
    if (u->StatusOffline()) arg = 1;
    u->SetStatus(s);
    
    //This is the v6 way of telling us phone follow me status
    if (s & ICQ_STATUS_FxPFM)
    {
      if (s & ICQ_STATUS_FxPFMxAVAILABLE)
        u->SetPhoneFollowMeStatus(ICQ_PLUGIN_STATUSxACTIVE);
      else
        u->SetPhoneFollowMeStatus(ICQ_PLUGIN_STATUSxBUSY);
    }
    else if (u->Version() < 7)
      u->SetPhoneFollowMeStatus(ICQ_PLUGIN_STATUSxINACTIVE);
  }

  // Say that we know their status for sure
  u->SetOfflineOnDisconnect(false);

  if(oldstatus != s)
  {
    u->Touch();
    pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_STATUS, u->id(), arg));
  }
}


//-----AddUserEvent-----------------------------------------------------------
bool CICQDaemon::AddUserEvent(ICQUser *u, CUserEvent *e)
{
  if (u->User())
    e->AddToHistory(u, D_RECEIVER);
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

  //pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_EVENTS, u->id()));
  return true;
}

void CICQDaemon::RejectEvent(const UserId& userId, CUserEvent* e)
{
  if (m_szRejectFile == NULL) return;

  FILE *f = fopen(m_szRejectFile, "a");
  if (f == NULL)
  {
    gLog.Warn(tr("%sUnable to open \"%s\" for writing.\n"), L_WARNxSTR, m_szRejectFile);
  }
  else
  {
    fprintf(f, "Event from new user (%s) rejected: \n%s\n--------------------\n\n",
        LicqUser::getUserAccountId(userId).c_str(), e->Text());
    chmod(m_szRejectFile, 00600);
    fclose(f);
  }
  delete e;
  m_sStats[STATS_EventsRejected].Inc();
}

/*----------------------------------------------------------------------------
 * CICQDaemon::SendEvent
 *
 * Sends an event without expecting a reply.
 *--------------------------------------------------------------------------*/

void CICQDaemon::SendEvent_Server(CPacket *packet)
{
#if 1
  LicqEvent* e = new LicqEvent(this, m_nTCPSrvSocketDesc, packet, CONNECT_SERVER);

  if (e == NULL)  return;
 
  pthread_mutex_lock(&mutex_sendqueue_server);
  m_lxSendQueue_Server.push_back(e);
  pthread_mutex_unlock(&mutex_sendqueue_server);

  e->m_NoAck = true;
  int nResult = pthread_create(&e->thread_send, NULL, &ProcessRunningEvent_Server_tep, e);
  if (nResult != 0)
  {
    gLog.Error("%sUnable to start server event thread (#%hu):\n%s%s.\n", L_ERRORxSTR,
       e->m_nSequence, L_BLANKxSTR, strerror(nResult));
    e->m_eResult = EVENT_ERROR;
  }
#else
  SendEvent(m_nTCPSrvSocketDesc, *packet, true);
#endif
}

ICQEvent *CICQDaemon::SendExpectEvent_Server(const UserId& userId,
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
  LicqEvent* e = new LicqEvent(this, m_nTCPSrvSocketDesc, packet, CONNECT_SERVER, userId, ue);

	if (e == NULL)  return NULL;

  if (bExtendedEvent) PushExtendedEvent(e);

  ICQEvent *result = SendExpectEvent(e, &ProcessRunningEvent_Server_tep);

  // if an error occured, remove the event from the extended queue as well
  if (result == NULL && bExtendedEvent)
  {
    pthread_mutex_lock(&mutex_extendedevents);
    std::list<ICQEvent *>::iterator i;
    for (i = m_lxExtendedEvents.begin(); i != m_lxExtendedEvents.end(); ++i)
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

ICQEvent* CICQDaemon::SendExpectEvent_Client(const ICQUser* pUser, CPacket* packet,
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
  LicqEvent* e = new LicqEvent(this, pUser->SocketDesc(packet->Channel()), packet,
     CONNECT_USER, pUser->id(), ue);

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
    gLog.Error("%sUnable to start event thread (#%hu):\n%s%s.\n", L_ERRORxSTR,
       e->m_nSequence, L_BLANKxSTR, strerror(nResult));
    DoneEvent(e, EVENT_ERROR);
    if (e->m_nSocketDesc == m_nTCPSrvSocketDesc)
    {
      pthread_mutex_lock(&mutex_sendqueue_server);
      list<ICQEvent *>::iterator iter;
      for (iter = m_lxSendQueue_Server.begin();
           iter != m_lxSendQueue_Server.end(); ++iter)
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
    for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); ++iter)
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

/**
 * Search the running event queue for a specific event by subsequence.
 */
bool CICQDaemon::hasServerEvent(unsigned long _nSubSequence) const
{
  bool hasEvent = false;
  pthread_mutex_lock(&mutex_runningevents);
  list<ICQEvent*>::const_iterator iter;
  for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); ++iter)
  {
    if ((*iter)->CompareSubSequence(_nSubSequence))
    {
      hasEvent = true;
      break;
    }
  }

  pthread_mutex_unlock(&mutex_runningevents);
  return hasEvent;
}
 

//---DoneSrvEvent--------------------------------------------------------------
/*! \brief Marks the given event as done.
 *
 * Marks the given event as done and removes it from the running events list. 
 * This is for new OSCAR server events. 
 * Basically this is DoneEvent (2)
 */
ICQEvent *CICQDaemon::DoneServerEvent(unsigned long _nSubSeq, EventResult _eResult)
{
  pthread_mutex_lock(&mutex_runningevents);
  ICQEvent *e = NULL;
  list<ICQEvent *>::iterator iter;
  for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); ++iter)
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
  for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); ++iter)
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
    for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); ++iter)
    {
      gLog.Info("%p Command: %d SubCommand: %d Sequence: %hu SubSequence: %d: Uin: %lu\n", *iter,
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
ICQEvent *CICQDaemon::DoneEvent(int _nSD, unsigned short _nSequence, EventResult _eResult)
{
  pthread_mutex_lock(&mutex_runningevents);
  ICQEvent *e = NULL;
  list<ICQEvent *>::iterator iter;
  for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); ++iter)
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
  for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); ++iter)
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
    const ICQUser* u = gUserManager.FetchUser(e->m_nDestinationUin, LOCK_R);
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
        ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      ChangeUserStatus(o, ((CPU_SetStatus *)e->m_pPacket)->Status() );
        gUserManager.DropOwner(o);
    }
    PushPluginEvent(e);
    break;

  case ICQ_CMDxSND_SETxRANDOMxCHAT:
    if (e->m_eResult == EVENT_ACKED)
    {
        ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      o->SetRandomChatGroup(((CPU_SetRandomChatGroup *)e->m_pPacket)->Group());
        gUserManager.DropOwner(o);
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
  for (iter = m_lxExtendedEvents.begin(); iter != m_lxExtendedEvents.end(); ++iter)
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
  for (iter = m_lxExtendedEvents.begin(); iter != m_lxExtendedEvents.end(); ++iter)
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
  for (iter = m_lxExtendedEvents.begin(); iter != m_lxExtendedEvents.end(); ++iter)
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

void CICQDaemon::PushEvent(ICQEvent *e)
{
  assert(e != NULL);
  pthread_mutex_lock(&mutex_runningevents);
  m_lxRunningEvents.push_back(e);
  pthread_mutex_unlock(&mutex_runningevents);
}

/*------------------------------------------------------------------------------
 * PushExtendedEvent
 *
 * Takes the given event, moves it event into the extended event queue.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PushExtendedEvent(ICQEvent *e)
{
  assert(e != NULL);
  pthread_mutex_lock(&mutex_extendedevents);
  m_lxExtendedEvents.push_back(e);
#if 0
  gLog.Info("%p pushing Command: %d SubCommand: %d Sequence: %hu SubSequence: %d: Uin: %lu\n", e,
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
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); ++iter)
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

void CICQDaemon::pushPluginSignal(LicqSignal* s)
{
  PluginsListIter iter;
  pthread_mutex_lock(&licq->mutex_plugins);
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); ++iter)
  {
    if ( (*iter)->CompareMask(s->Signal()) )
      (*iter)->pushSignal(new LicqSignal(s));
  }
  pthread_mutex_unlock(&licq->mutex_plugins);
  delete s;
}

LicqSignal* CICQDaemon::popPluginSignal()
{
  PluginsListIter iter;
  LicqSignal* s = NULL;
  pthread_mutex_lock(&licq->mutex_plugins);
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); ++iter)
  {
    if ( (*iter)->CompareThread(pthread_self()) )
    {
      s = (*iter)->popSignal();
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
  for (iter = licq->list_plugins.begin(); iter != licq->list_plugins.end(); ++iter)
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
  bool bExists = false;
  pthread_mutex_lock(&licq->mutex_protoplugins);
  for (iter = licq->list_protoplugins.begin(); iter != licq->list_protoplugins.end();
       ++iter)
  {
    if ((*iter)->PPID() == _nPPID)
    {
      bExists = true;
      if ((*iter)->SendSignals())
        (*iter)->PushSignal(s);
      break;
    }
  }
  pthread_mutex_unlock(&licq->mutex_protoplugins);

  if (!bExists)
  {
    gLog.Info("%sInvalid protocol plugin requested (%ld).\n", L_ERRORxSTR, _nPPID);
    delete s;
  }
}

CSignal *CICQDaemon::PopProtoSignal()
{
  ProtoPluginsListIter iter;
  CSignal *s = NULL;
  pthread_mutex_lock(&licq->mutex_protoplugins);
  for (iter = licq->list_protoplugins.begin(); iter != licq->list_protoplugins.end();
       ++iter)
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
       iter != m_lxSendQueue_Server.end(); ++iter)
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
    gLog.Warn(tr("%sCancelled event not found.\n"), L_WARNxSTR);
    return;
  }

  CancelEvent((eRun != NULL)? eRun : (eExt != NULL)? eExt : eSrv);
}

void CICQDaemon::CancelEvent(ICQEvent *e)
{
  e->m_eResult = EVENT_CANCELLED;

  if (e->m_nSubCommand == ICQ_CMDxSUB_CHAT)
    icqChatRequestCancel(LicqUser::getUserAccountId(e->userId()).c_str(), e->m_nSequence);
  else if (e->m_nSubCommand == ICQ_CMDxSUB_FILE)
    icqFileTransferCancel(LicqUser::getUserAccountId(e->userId()).c_str(), e->m_nSequence);
  else if (e->m_nSubCommand == ICQ_CMDxSUB_SECURExOPEN)
    icqOpenSecureChannelCancel(LicqUser::getUserAccountId(e->userId()).c_str(), e->m_nSequence);

  ProcessDoneEvent(e);
}


//-----updateAllUsers-------------------------------------------------------------------------
void CICQDaemon::UpdateAllUsers()
{
  FOR_EACH_USER_START(LOCK_R)
  {
    icqRequestMetaInfo(pUser->IdString());
  }
  FOR_EACH_USER_END
}


void CICQDaemon::UpdateAllUsersInGroup(GroupType g, unsigned short nGroup)
{
  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->GetInGroup(g, nGroup))
    {
      icqRequestMetaInfo(pUser->IdString());
    }
  }
  FOR_EACH_USER_END
}

//-----AddProtocolPlugins-------------------------------------------------------
bool CICQDaemon::AddProtocolPlugins()
{
  char szConf[MAX_FILENAME_LEN];
  CIniFile licqConf(INI_FxWARN);
  snprintf(szConf, MAX_FILENAME_LEN, "%s/licq.conf", BASE_DIR);
  szConf[MAX_FILENAME_LEN - 1] = '\0';
  if (licqConf.LoadFile(szConf) == false)
    return false;
    
  unsigned short nNumProtoPlugins = 0;
  char szData[MAX_FILENAME_LEN];
  char szKey[20];
  licqConf.SetSection("plugins");
  licqConf.ReadNum("NumProtoPlugins", nNumProtoPlugins, 0);
  if (nNumProtoPlugins > 0)
  {
    for (int i = 0; i < nNumProtoPlugins; i++)
    {
      sprintf(szKey, "ProtoPlugin%d", i + 1);
      if (!licqConf.ReadStr(szKey, szData)) continue;
      if (ProtoPluginLoad(szData) == false) return false;
    }
  }
  
  return true;
}

//-----Conversation functions---------------------------------------------------
CConversation *CICQDaemon::AddConversation(int _nSocket, unsigned long _nPPID)
{
  CConversation *pNew = new CConversation(_nSocket, _nPPID);
  pthread_mutex_lock(&mutex_conversations);
  m_lConversations.push_back(pNew);
  pthread_mutex_unlock(&mutex_conversations);
  
  return pNew;
}

bool CICQDaemon::AddUserConversation(unsigned long _nCID, const char *_szId)
{
  bool bAdded = false;
  ConversationList::iterator iter;
  pthread_mutex_lock(&mutex_conversations);
  for (iter = m_lConversations.begin(); iter != m_lConversations.end(); ++iter)
  {
    if ((*iter)->CID() == _nCID)
    {
      bAdded = true;
      (*iter)->AddUser(_szId);
      break;
    }
  }
  pthread_mutex_unlock(&mutex_conversations);
  
  return bAdded;
}

bool CICQDaemon::AddUserConversation(int _nSocket, const char *_szId)
{
  bool bAdded = false;
  ConversationList::iterator iter;
  pthread_mutex_lock(&mutex_conversations);
  for (iter = m_lConversations.begin(); iter != m_lConversations.end(); ++iter)
  {
    if ((*iter)->Socket() == _nSocket)
    {
      bAdded = true;
      (*iter)->AddUser(_szId);
      break;
    }
  }
  pthread_mutex_unlock(&mutex_conversations);
  
  return bAdded;
}

bool CICQDaemon::RemoveUserConversation(unsigned long _nCID, const char *_szId)
{
  bool bRemoved = false;
  ConversationList::iterator iter;
  pthread_mutex_lock(&mutex_conversations);
  for (iter = m_lConversations.begin(); iter != m_lConversations.end(); ++iter)
  {
    if ((*iter)->CID() == _nCID)
    {
      bRemoved = (*iter)->RemoveUser(_szId);
      break;
    }
  }
  pthread_mutex_unlock(&mutex_conversations);
  
  return bRemoved;
}

bool CICQDaemon::RemoveUserConversation(int _nSocket, const char *_szId)
{
  bool bRemoved = false;
  ConversationList::iterator iter;
  pthread_mutex_lock(&mutex_conversations);
  for (iter = m_lConversations.begin(); iter != m_lConversations.end(); ++iter)
  {
    if ((*iter)->Socket() == _nSocket)
    {
      bRemoved = (*iter)->RemoveUser(_szId);
      break;
    }
  }
  pthread_mutex_unlock(&mutex_conversations);
  
  return bRemoved;
}

CConversation *CICQDaemon::FindConversation(int _nSocket)
{
  pthread_mutex_lock(&mutex_conversations);
  ConversationList::iterator iter;
  for (iter = m_lConversations.begin(); iter != m_lConversations.end(); ++iter)
    if ((*iter)->Socket() == _nSocket)
      break;
  pthread_mutex_unlock(&mutex_conversations);
  
  return (iter == m_lConversations.end()) ? 0 : *iter;
}

CConversation *CICQDaemon::FindConversation(unsigned long _nCID)
{
  pthread_mutex_lock(&mutex_conversations);
  ConversationList::iterator iter;
  for (iter = m_lConversations.begin(); iter != m_lConversations.end(); ++iter)
    if ((*iter)->CID() == _nCID)
      break;
  pthread_mutex_unlock(&mutex_conversations);
  
  return (iter == m_lConversations.end()) ? 0 : *iter;
}

bool CICQDaemon::RemoveConversation(unsigned long _nCID)
{
  bool bDeleted = false;
  pthread_mutex_lock(&mutex_conversations);
  ConversationList::iterator iter;
  for (iter = m_lConversations.begin(); iter != m_lConversations.end(); ++iter)
  {
    if ((*iter)->CID() == _nCID)
    {
      bDeleted = true;
      m_lConversations.erase(iter);
      delete *iter;  
      break;
    }
  }
  pthread_mutex_unlock(&mutex_conversations);  

  return bDeleted;
}

//-----ProcessMessage-----------------------------------------------------------
void CICQDaemon::ProcessMessage(ICQUser *u, CBuffer &packet, char *message,
    unsigned short nMsgType, unsigned long nMask, const unsigned long nMsgID[2],
                                unsigned short nSequence, bool bIsAck,
                                bool &bNewUser)
{
  char *szType = NULL;
  CUserEvent *pEvent = NULL;
  unsigned short nEventType = 0;

  // for acks
  unsigned short nPort;

  // Do we accept it if we are in Occ or DND?
  // const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  // unsigned short nOwnerStatus = o->Status();
  // gUserManager.DropOwner(o);

  unsigned short nLevel = nMask;
  unsigned long nFlags = ((nMask & ICQ_CMDxSUB_FxMULTIREC) ? E_MULTIxREC : 0)
                         | ((nMask & ICQ_TCPxMSG_URGENT) ? E_URGENT : 0);

  u->Lock(LOCK_W);

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

    szType = strdup(tr("Message"));
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
                                     TIME_NOW, nFlags, 0, nMsgID[0], nMsgID[1]);
      nEventType = ON_EVENT_CHAT;
      pEvent = e;
    }

    szType = strdup(tr("Chat request"));
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
        string filename = packet.unpackRawString(nFilenameLen);
      packet >> nFileSize;
      ConstFileList filelist;
        filelist.push_back(strdup(filename.c_str()));

        CEventFile* e = new CEventFile(filename.c_str(), message, nFileSize,
                                     filelist, nSequence, TIME_NOW, nFlags,
                                     0, nMsgID[0], nMsgID[1]);
      nEventType = ON_EVENT_FILE;
      pEvent = e;
    }
    else
      packet.incDataPosRead(nFilenameLen + 4);

    packet >> nPort;

    szType = strdup(tr("File transfer request through server"));
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

    szType = strdup(tr("URL"));
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

    szType = strdup(tr("Contact list"));
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
        gLog.Info(tr("%sAuto response from %s (#%lu).\n"), L_SRVxSTR, u->GetAlias(),
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
        gLog.Warn(tr("%sAck for unknown event.\n"), L_SRVxSTR);
    }
    else
    {
      gLog.Info(tr("%s%s (%s) requested auto response.\n"), L_SRVxSTR,
          u->GetAlias(), u->IdString());

    CPU_AckGeneral *p = new CPU_AckGeneral(u, nMsgID[0], nMsgID[1],
                                           nSequence, nMsgType, true, nLevel);
    SendEvent_Server(p);

    m_sStats[STATS_AutoResponseChecked].Inc();
    u->SetLastCheckedAutoResponse();

        pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_EVENTS, u->id()));
      }
    u->Unlock();
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
      string plugin = packet.unpackRawString(nLongLen);

    packet.incDataPosRead(nLen - 22 - nLongLen); // unknown
    packet >> nLongLen; // bytes remaining

    int nCommand = 0;
      if (plugin.find("File") != string::npos)
      nCommand = ICQ_CMDxSUB_FILE;
      else if (plugin.find("URL") != string::npos)
      nCommand = ICQ_CMDxSUB_URL;
      else if (plugin.find("Chat") != string::npos)
      nCommand = ICQ_CMDxSUB_CHAT;
      else if (plugin.find("Contacts") != string::npos)
      nCommand = ICQ_CMDxSUB_CONTACTxLIST;

    if (nCommand == 0)
    {
        gLog.Warn(tr("%sUnknown ICBM plugin type: %s\n"), L_SRVxSTR, plugin.c_str());
      u->Unlock();
      return;
    }

    packet >> nLongLen;
      char* szMessage = new char[nLongLen+1];
    for (unsigned long i = 0; i < nLongLen; i++)
      packet >> szMessage[i];
    szMessage[nLongLen] = '\0';

    /* if the auto response is non empty then this is a decline and we want
       to show the auto response rather than our original message */
    char *msg = (message[0] != '\0') ? message : szMessage;

    // recursion
    u->Unlock();
    ProcessMessage(u, packet, msg, nCommand, nMask, nMsgID,
                   nSequence, bIsAck, bNewUser);
      delete [] szMessage;
    return;

    break; // bah!
  }

  default:
    gTranslator.ServerToClient(message);
    szType = strdup(tr("unknown event"));
  } // switch nMsgType

  if (bIsAck)
  {
    ICQEvent *pAckEvent = DoneServerEvent(nMsgID[1], EVENT_ACKED);
    CExtendedAck *pExtendedAck = new CExtendedAck(true, nPort, message);

    if (pAckEvent)
    {
      pAckEvent->m_pExtendedAck = pExtendedAck;
      pAckEvent->m_nSubResult = ICQ_TCPxACK_ACCEPT;
      gLog.Info(tr("%s%s accepted from %s (%s).\n"), L_SRVxSTR, szType,
          u->GetAlias(), u->IdString());
      u->Unlock();
      ProcessDoneEvent(pAckEvent);
      u->Lock(LOCK_W);
    }
    else
    {
      gLog.Warn(tr("%sAck for unknown event.\n"), L_SRVxSTR);
      delete pExtendedAck;
     }
  }
  else
  {
    // If it parsed, did it parse properly?
    if (pEvent)
    {
      if (bNewUser)
      {
        if (Ignore(IGNORE_NEWUSERS))
        {
          gLog.Info(tr("%s%s from new user (%s), ignoring.\n"), L_SRVxSTR,
                    szType, u->IdString());
          if (szType)  free(szType);
          RejectEvent(u->id(), pEvent);
          u->Unlock();
          return;
        }
        gLog.Info(tr("%s%s from new user (%s).\n"), L_SRVxSTR, szType, u->IdString());
        u->Unlock();
        gUserManager.addUser(u->id(), false);
        bNewUser = false;
      }
      else
        gLog.Info(tr("%s%s from %s (%s).\n"), L_SRVxSTR, szType, u->GetAlias(),
            u->IdString());

      if (AddUserEvent(u, pEvent))
        m_xOnEventManager.Do(nEventType, u);
    }
    else // invalid parse or unknown event
    {
      char *buf;
      gLog.Warn(tr("%sInvalid %s:\n%s\n"), L_WARNxSTR, szType, packet.print(buf));
      delete [] buf;
    }
  }

  u->Unlock();

  if (szType)  free(szType);
}

bool CICQDaemon::WaitForReverseConnection(unsigned short id, const char* userId)
{
  bool bSuccess = false;
  pthread_mutex_lock(&mutex_reverseconnect);

  std::list<CReverseConnectToUserData *>::iterator iter;
  for (iter = m_lReverseConnect.begin(); iter != m_lReverseConnect.end();
    ++iter)
  {
    if ((*iter)->nId == id && (*iter)->myIdString == userId)
      break;
  }

  if (iter == m_lReverseConnect.end())
  {
    gLog.Warn("%sFailed to find desired connection record.\n", L_WARNxSTR);
    goto done;
  }

  struct timespec ts;
  ts.tv_nsec = 0;
  //wait for 30 seconds
  ts.tv_sec = time(NULL) + 30;

  while (pthread_cond_timedwait(&cond_reverseconnect_done,
    &mutex_reverseconnect, &ts) == 0)
  {
    for (iter = m_lReverseConnect.begin(); ; ++iter)
    {
      if (iter == m_lReverseConnect.end())
      {
        gLog.Warn("%sSomebody else removed our connection record.\n",
          L_WARNxSTR);
        goto done;
      }
      if ((*iter)->nId == id && (*iter)->myIdString == userId)
      {
        if ((*iter)->bFinished)
        {
          bSuccess = (*iter)->bSuccess;
          delete *iter;
          m_lReverseConnect.erase(iter);
          goto done;
        }
        break;
      }
    }
  }

  // timed out, just remove the record
  for (iter = m_lReverseConnect.begin(); iter != m_lReverseConnect.end();
    ++iter)
  {
    if ((*iter)->nId == id && (*iter)->myIdString == userId)
    {
      delete *iter;
      m_lReverseConnect.erase(iter);
      break;
    }
  }

done:
  pthread_mutex_unlock(&mutex_reverseconnect);
  return bSuccess;
}

CReverseConnectToUserData::CReverseConnectToUserData(const char* idString, unsigned long id,
      unsigned long data, unsigned long ip, unsigned short port,
      unsigned short version, unsigned short failedport, unsigned long msgid1,
      unsigned long msgid2) :
  myIdString(idString), nId(id), nData(data), nIp(ip), nPort(port),
  nFailedPort(failedport), nVersion(version), nMsgID1(msgid1),
  nMsgID2(msgid2), bSuccess(false), bFinished(false)
{
  // Empty
}

CReverseConnectToUserData::~CReverseConnectToUserData()
{
  // Empty
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

CUserProperties::CUserProperties()
  : newAlias(NULL),
    newCellular(NULL),
    normalSid(0),
    groupId(0),
    visibleSid(0),
    invisibleSid(0),
    inIgnoreList(false),
    awaitingAuth(false)
{
  tlvs.clear();
}
