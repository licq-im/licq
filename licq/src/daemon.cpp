/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#include <licq_icqd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <licq_constants.h>
#include <licq_icq.h>
#include <licq_log.h>
#include <licq_proxy.h>
#include <licq_translate.h>
#include <licq/inifile.h>

#include "contactlist/usermanager.h"
#include "gettext.h"
#include "gpghelper.h"
#include "licq.h"
#include "plugins/pluginmanager.h"

using namespace std;
using namespace LicqDaemon;

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


CICQDaemon *gLicqDaemon = NULL;
Licq::Daemon* Licq::gDaemon = NULL;

CICQDaemon::CICQDaemon(CLicq *_licq)
{
  string temp;

  licq = _licq;
  gLicqDaemon = this;
  Licq::gDaemon = this;

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
  Licq::IniFile licqConf("licq.conf");
  licqConf.loadFile();

  licqConf.setSection("network");

  // ICQ Server
  licqConf.get("ICQServer", myIcqServer, DEFAULT_SERVER_HOST);
  licqConf.get("ICQServerPort", myIcqServerPort, DEFAULT_SERVER_PORT);

  unsigned nTCPPortsLow, nTCPPortsHigh;
  licqConf.get("TCPPortsLow", nTCPPortsLow, 0);
  licqConf.get("TCPPortsHigh", nTCPPortsHigh, 0);
  SetTCPPorts(nTCPPortsLow, nTCPPortsHigh);
  licqConf.get("TCPEnabled", m_bTCPEnabled, true);
  licqConf.get("Firewall", m_bFirewall, false);
  SetTCPEnabled(!m_bFirewall || (m_bFirewall && m_bTCPEnabled));
  licqConf.get("MaxUsersPerPacket", myMaxUsersPerPacket, 100);
  licqConf.get("IgnoreTypes", m_nIgnoreTypes, 0);
  licqConf.get("AutoUpdateInfo", m_bAutoUpdateInfo, true);
  licqConf.get("AutoUpdateInfoPlugins", m_bAutoUpdateInfoPlugins, true);
  licqConf.get("AutoUpdateStatusPlugins", m_bAutoUpdateStatusPlugins, true);
  unsigned long nColor;
  licqConf.get("ForegroundColor", nColor, 0x00000000);
  CICQColor::SetDefaultForeground(nColor);
  licqConf.get("BackgroundColor", nColor, 0x00FFFFFF);
  CICQColor::SetDefaultBackground(nColor);


  // Rejects log file
  licqConf.get("Rejects", myRejectFile, "log.rejects");
  if (myRejectFile == "none")
    myRejectFile = "";

  // Error log file
  licqConf.get("Errors", myErrorFile, "log.errors");
  licqConf.get("ErrorTypes", myErrorTypes, L_ERROR | L_UNKNOWN);
  if (myErrorFile != "none")
  {
    string errorFile = BASE_DIR + myErrorFile;
    CLogService_File *l = new CLogService_File(myErrorTypes);
    if (!l->SetLogFile(errorFile.c_str(), "a"))
    {
      gLog.Error("%sUnable to open %s as error log:\n%s%s.\n",
          L_ERRORxSTR, errorFile.c_str(), L_BLANKxSTR, strerror(errno));
      delete l;
    }
    else
      gOldLog.AddService(l);
  }

  // Loading translation table from file
  licqConf.get("Translation", temp, "none");
  if (temp != "none")
  {
    string filename = SHARE_DIR;
    filename += TRANSLATION_DIR;
    filename += "/";
    filename += temp;
    gTranslator.setTranslationMap(temp.c_str());
  }

  // Terminal
  licqConf.get("Terminal", myTerminal, "xterm -T Licq -e ");

  // Proxy
  m_xProxy = NULL;
  licqConf.get("ProxyEnabled", myProxyEnabled, false);
  licqConf.get("ProxyServerType", myProxyType, PROXY_TYPE_HTTP);
  licqConf.get("ProxyServer", myProxyHost, "");
  licqConf.get("ProxyServerPort", myProxyPort, 0);
  licqConf.get("ProxyAuthEnabled", myProxyAuthEnabled, false);
  licqConf.get("ProxyLogin", myProxyLogin, "");
  licqConf.get("ProxyPassword", myProxyPasswd, "");

  // Services
  m_xBARTService = NULL;

  // Misc
  licqConf.get("UseSS", m_bUseSS, true); // server side list
  licqConf.get("UseBART", m_bUseBART, true); // server side buddy icons
  licqConf.get("SendTypingNotification", m_bSendTN, true);
  licqConf.get("ReconnectAfterUinClash", m_bReconnectAfterUinClash, false);

  // Statistics
  m_nResetTime = 0;
  m_sStats.push_back(CDaemonStats("Events Sent", "Sent"));
  m_sStats.push_back(CDaemonStats("Events Received", "Recv"));
  m_sStats.push_back(CDaemonStats("Events Rejected", "Reject"));
  m_sStats.push_back(CDaemonStats("Auto Response Checked", "ARC"));
#ifdef SAVE_STATS
  DaemonStatsList::iterator iter;
  if (licqConf.setSection("stats"))
  {
    unsigned long t;
    licqConf.get("Reset", t, 0);
    m_nResetTime = t;
    for (iter = m_sStats.begin(); iter != m_sStats.end(); ++iter)
    {
      licqConf.get(iter->m_szTag, iter->m_nTotal, 0);
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
  gGpgHelper.Start();

  // Init event id counter
  myNextEventId = 1;
  pthread_mutex_init(&myNextEventIdMutex, NULL);

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

#ifdef USE_FIFO
  // Open the fifo
  snprintf(sz, MAX_FILENAME_LEN, "%slicq_fifo", BASE_DIR);
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

  return startIcq();
}

Licq::LogService& CICQDaemon::getLogService()
{
  return licq->getLogService();
}

const char* CICQDaemon::Version() const
{
  return licq->Version();
}

CICQDaemon::~CICQDaemon()
{
  if(fifo_fs)         fclose(fifo_fs);
  gLicqDaemon = NULL;
  Licq::gDaemon = NULL;
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
  Licq::IniFile licqConf("licq.conf");
  if (!licqConf.loadFile())
    return;
  licqConf.setSection("stats");
  licqConf.set("Reset", (unsigned long)m_nResetTime);
  for (iter = m_sStats.begin(); iter != m_sStats.end(); ++iter)
  {
    licqConf.set(iter->m_szTag, iter->m_nTotal);
    iter->ClearDirty();
  }
  licqConf.writeFile();
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

void CICQDaemon::SaveConf()
{
  Licq::IniFile licqConf("licq.conf");
  if (!licqConf.loadFile())
    return;

  licqConf.setSection("network");

  // ICQ Server
  licqConf.set("ICQServer", myIcqServer);
  licqConf.set("ICQServerPort", myIcqServerPort);

  licqConf.set("TCPPortsLow", m_nTCPPortsLow);
  licqConf.set("TCPPortsHigh", m_nTCPPortsHigh);
  licqConf.set("TCPEnabled", m_bTCPEnabled);
  licqConf.set("Firewall", m_bFirewall);
  licqConf.set("MaxUsersPerPacket", myMaxUsersPerPacket);
  licqConf.set("IgnoreTypes", m_nIgnoreTypes);
  licqConf.set("AutoUpdateInfo", m_bAutoUpdateInfo);
  licqConf.set("AutoUpdateInfoPlugins", m_bAutoUpdateInfoPlugins);
  licqConf.set("AutoUpdateStatusPlugins", m_bAutoUpdateStatusPlugins);
  licqConf.set("ForegroundColor", CICQColor::DefaultForeground());
  licqConf.set("BackgroundColor", CICQColor::DefaultBackground());

  // Utility tab
  const char* pc = gTranslator.getMapName();
  if (pc == NULL)
    pc = "none";
  else
  {
    const char* pc2 = strrchr(pc, '/');
    if (pc2 != NULL)
      pc = pc2++;
  }
  licqConf.set("Translation", pc);
  licqConf.set("Terminal", myTerminal);
  licqConf.set("Errors", myErrorFile);
  licqConf.set("ErrorTypes", myErrorTypes);
  licqConf.set("Rejects", (myRejectFile.empty() ? "none" : myRejectFile));

  // Proxy
  licqConf.set("ProxyEnabled", myProxyEnabled);
  licqConf.set("ProxyServerType", myProxyType);
  licqConf.set("ProxyServer", myProxyHost);
  licqConf.set("ProxyServerPort", myProxyPort);
  licqConf.set("ProxyAuthEnabled", myProxyAuthEnabled);
  licqConf.set("ProxyLogin", myProxyLogin);
  licqConf.set("ProxyPassword", myProxyPasswd);

  // Misc
  licqConf.set("UseSS", m_bUseSS); // server side list
  licqConf.set("UseBART", m_bUseBART); // server side buddy icons
  licqConf.set("SendTypingNotification", m_bSendTN);
  licqConf.set("ReconnectAfterUinClash", m_bReconnectAfterUinClash);
  licqConf.set("DefaultUserEncoding", gUserManager.defaultUserEncoding());

  licqConf.setSection("owners");
  licqConf.set("NumOfOwners", (unsigned long)gUserManager.NumOwners());

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
      licqConf.set(szOwnerId, pOwner->IdString());
      licqConf.set(szOwnerPPID, szPPID);
    }
  }
  FOR_EACH_OWNER_END

  licqConf.writeFile();
}

bool CICQDaemon::haveGpgSupport() const
{
#ifdef HAVE_LIBGPGME
  return true;
#else
  return false;
#endif
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

  switch (myProxyType)
  {
    case PROXY_TYPE_HTTP :
      Proxy = new HTTPProxyServer();
      break;
    default:
      break;
  }

  if (Proxy != NULL)
  {
    gLog.Info(tr("%sResolving proxy: %s:%d...\n"), L_INITxSTR, myProxyHost.c_str(), myProxyPort);
    if (!Proxy->SetProxyAddr(myProxyHost.c_str(), myProxyPort))
    {
      char buf[128];

      gLog.Warn(tr("%sUnable to resolve proxy server %s:\n%s%s.\n"), L_ERRORxSTR,
          myProxyHost.c_str(), L_BLANKxSTR, Proxy->ErrorStr(buf, 128));
      delete Proxy;
      Proxy = NULL;
    }

    if (Proxy)
    {
      if (myProxyAuthEnabled)
        Proxy->SetProxyAuth(myProxyLogin.c_str(), myProxyPasswd.c_str());

      Proxy->InitProxy();
    }
  }

  return Proxy;
}

void CICQDaemon::ChangeUserStatus(Licq::User* u, unsigned long s)
{
  u->statusChanged(Licq::User::statusFromIcqStatus(s), s);
}

unsigned long CICQDaemon::getNextEventId()
{
  pthread_mutex_lock(&myNextEventIdMutex);
  unsigned long eventId = myNextEventId;
  if (++myNextEventId == 0)
    ++myNextEventId;
  pthread_mutex_unlock(&myNextEventIdMutex);
  return eventId;
}

void CICQDaemon::PushPluginEvent(ICQEvent *e)
{
  gPluginManager.getPluginEventHandler().pushGeneralEvent(e);
}

void CICQDaemon::pushPluginSignal(LicqSignal* s)
{
  gPluginManager.getPluginEventHandler().pushGeneralSignal(s);
}

LicqSignal* CICQDaemon::popPluginSignal()
{
  return gPluginManager.getPluginEventHandler().popGeneralSignal();
}

ICQEvent *CICQDaemon::PopPluginEvent()
{
  return gPluginManager.getPluginEventHandler().popGeneralEvent();
}

void CICQDaemon::PushProtoSignal(LicqProtoSignal* s, unsigned long _nPPID)
{
  gPluginManager.getPluginEventHandler().pushProtocolSignal(s, _nPPID);
}

LicqProtoSignal* CICQDaemon::PopProtoSignal()
{
  return gPluginManager.getPluginEventHandler().popProtocolSignal();
}

bool CICQDaemon::AddProtocolPlugins()
{
  Licq::IniFile licqConf("licq.conf");
  if (!licqConf.loadFile())
    return false;

  unsigned nNumProtoPlugins;
  char szKey[20];
  licqConf.setSection("plugins");
  licqConf.get("NumProtoPlugins", nNumProtoPlugins, 0);
  if (nNumProtoPlugins > 0)
  {
    for (unsigned i = 0; i < nNumProtoPlugins; i++)
    {
      string szData;
      sprintf(szKey, "ProtoPlugin%d", i + 1);
      if (!licqConf.get(szKey, szData))
        continue;
      if (!gPluginManager.startProtocolPlugin(szData))
        return false;
    }
  }

  return true;
}
