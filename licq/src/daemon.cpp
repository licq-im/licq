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
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <licq_constants.h>
#include <licq_icq.h>
#include <licq_log.h>
#include <licq_proxy.h>
#include <licq_translate.h>
#include <licq/inifile.h>
#include <licq/thread/mutexlocker.h>

#include "contactlist/usermanager.h"
#include "gettext.h"
#include "gpghelper.h"
#include "licq.h"
#include "plugins/pluginmanager.h"

using namespace std;
using namespace LicqDaemon;
using Licq::DaemonStats;

DaemonStats::DaemonStats()
{
  m_nTotal = m_nOriginal = m_nLastSaved = 0;
  m_szName[0] = m_szTag[0] = '\0';
}

DaemonStats::DaemonStats(const char *name, const char *tag)
{
  m_nTotal = m_nOriginal = m_nLastSaved = 0;
  strcpy(m_szName, name);
  strcpy(m_szTag, tag);
}

DaemonStats::~DaemonStats()
{
  // Empty
}

void DaemonStats::Reset()
{
  m_nTotal = m_nOriginal = 0;
}

void DaemonStats::Init()
{
  m_nOriginal = m_nLastSaved = m_nTotal;
}


Licq::Daemon* Licq::gDaemon = NULL;

Licq::Daemon::Daemon(CLicq* _licq)
{
  string temp;

  licq = _licq;
  Licq::gDaemon = this;

  myShuttingDown = false;
  fifo_fs = NULL;

  // Begin parsing the config file
  Licq::IniFile licqConf("licq.conf");
  licqConf.loadFile();

  licqConf.setSection("network");

  unsigned nTCPPortsLow, nTCPPortsHigh;
  licqConf.get("TCPPortsLow", nTCPPortsLow, 0);
  licqConf.get("TCPPortsHigh", nTCPPortsHigh, 0);
  setTcpPorts(nTCPPortsLow, nTCPPortsHigh);
  licqConf.get("TCPEnabled", myTcpEnabled, true);
  licqConf.get("Firewall", myBehindFirewall, false);
  setTcpEnabled(!myBehindFirewall || (myBehindFirewall && myTcpEnabled));

  licqConf.get("ProxyEnabled", myProxyEnabled, false);
  licqConf.get("ProxyServerType", myProxyType, PROXY_TYPE_HTTP);
  licqConf.get("ProxyServer", myProxyHost, "");
  licqConf.get("ProxyServerPort", myProxyPort, 0);
  licqConf.get("ProxyAuthEnabled", myProxyAuthEnabled, false);
  licqConf.get("ProxyLogin", myProxyLogin, "");
  licqConf.get("ProxyPassword", myProxyPasswd, "");

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

  // Statistics
  m_nResetTime = 0;
  m_sStats.push_back(DaemonStats("Events Sent", "Sent"));
  m_sStats.push_back(DaemonStats("Events Received", "Recv"));
  m_sStats.push_back(DaemonStats("Events Rejected", "Reject"));
  m_sStats.push_back(DaemonStats("Auto Response Checked", "ARC"));
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

  // Initialize the random number generator
  srand(time(NULL));

  // start GPG helper
  LicqDaemon::gGpgHelper.Start();

  // Init event id counter
  myNextEventId = 1;
}

bool Licq::Daemon::Start()
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

  return gLicqDaemon->startIcq();
}

Licq::LogService& Licq::Daemon::getLogService()
{
  return licq->getLogService();
}

const char* Licq::Daemon::Version() const
{
  return licq->Version();
}

Licq::Daemon::~Daemon()
{
  if (fifo_fs != NULL)
    fclose(fifo_fs);
  Licq::gDaemon = NULL;
}

void Licq::Daemon::FlushStats()
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

void Licq::Daemon::ResetStats()
{
  DaemonStatsList::iterator iter;
  for (iter = m_sStats.begin(); iter != m_sStats.end(); ++iter)
  {
    iter->Reset();
  }
  m_nResetTime = time(NULL);
}

pthread_t* Licq::Daemon::Shutdown()
{
  if (myShuttingDown)
    return(&thread_shutdown);
  myShuttingDown = true;
  // Small race condition here if multiple plugins call shutdown at the same time
  pthread_create (&thread_shutdown, NULL, &Shutdown_tep, this);
  return (&thread_shutdown);
}

void Licq::Daemon::SaveConf()
{
  Licq::IniFile licqConf("licq.conf");
  if (!licqConf.loadFile())
    return;

  licqConf.setSection("network");

  licqConf.set("TCPPortsLow", myTcpPortsLow);
  licqConf.set("TCPPortsHigh", myTcpPortsHigh);
  licqConf.set("TCPEnabled", myTcpEnabled);
  licqConf.set("Firewall", myBehindFirewall);

  // Proxy
  licqConf.set("ProxyEnabled", myProxyEnabled);
  licqConf.set("ProxyServerType", myProxyType);
  licqConf.set("ProxyServer", myProxyHost);
  licqConf.set("ProxyServerPort", myProxyPort);
  licqConf.set("ProxyAuthEnabled", myProxyAuthEnabled);
  licqConf.set("ProxyLogin", myProxyLogin);
  licqConf.set("ProxyPassword", myProxyPasswd);

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

  gLicqDaemon->saveIcqConf(licqConf);

  licqConf.writeFile();
}

bool Licq::Daemon::haveGpgSupport() const
{
#ifdef HAVE_LIBGPGME
  return true;
#else
  return false;
#endif
}

int Licq::Daemon::StartTCPServer(TCPSocket *s)
{
  if (myTcpPortsLow == 0)
  {
    s->StartServer(0);
  }
  else
  {
    for (unsigned p = myTcpPortsLow; p <= myTcpPortsHigh; p++)
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

void Licq::Daemon::setTcpEnabled(bool b)
{
  myTcpEnabled = b;
  gLicqDaemon->SetDirectMode();
}

void Licq::Daemon::setBehindFirewall(bool b)
{
  myBehindFirewall = b;
  gLicqDaemon->SetDirectMode();
}

void Licq::Daemon::setTcpPorts(unsigned lowPort, unsigned highPort)
{
  myTcpPortsLow = lowPort;
  myTcpPortsHigh = highPort;
  if (myTcpPortsHigh < myTcpPortsLow)
  {
    gLog.Warn(tr("%sTCP high port (%d) is lower then TCP low port (%d).\n"),
       L_WARNxSTR, myTcpPortsHigh, myTcpPortsLow);
    myTcpPortsHigh = myTcpPortsLow + 10;
  }
}

ProxyServer* Licq::Daemon::createProxy()
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

unsigned long Licq::Daemon::getNextEventId()
{
  Licq::MutexLocker eventIdGuard(myNextEventIdMutex);
  unsigned long eventId = myNextEventId;
  if (++myNextEventId == 0)
    ++myNextEventId;
  return eventId;
}

void Licq::Daemon::PushPluginEvent(LicqEvent* e)
{
  LicqDaemon::gPluginManager.getPluginEventHandler().pushGeneralEvent(e);
}

void Licq::Daemon::pushPluginSignal(LicqSignal* s)
{
  LicqDaemon::gPluginManager.getPluginEventHandler().pushGeneralSignal(s);
}

LicqSignal* Licq::Daemon::popPluginSignal()
{
  return LicqDaemon::gPluginManager.getPluginEventHandler().popGeneralSignal();
}

LicqEvent* Licq::Daemon::PopPluginEvent()
{
  return LicqDaemon::gPluginManager.getPluginEventHandler().popGeneralEvent();
}

void Licq::Daemon::PushProtoSignal(LicqProtoSignal* s, unsigned long _nPPID)
{
  LicqDaemon::gPluginManager.getPluginEventHandler().pushProtocolSignal(s, _nPPID);
}

LicqProtoSignal* Licq::Daemon::PopProtoSignal()
{
  return LicqDaemon::gPluginManager.getPluginEventHandler().popProtocolSignal();
}
