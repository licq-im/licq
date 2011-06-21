/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2011 Licq developers
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

#include "daemon.h"

#include <boost/foreach.hpp>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <sys/stat.h> // chmod

#include <licq/logging/log.h>
#include <licq/logging/logservice.h>
#include <licq/logging/logutils.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/inifile.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/proxy.h>
#include <licq/socket.h>
#include <licq/statistics.h>
#include <licq/thread/mutexlocker.h>
#include <licq/translator.h>
#include <licq/userevents.h>

#include "contactlist/usermanager.h"
#include "gettext.h"
#include "gpghelper.h"
#include "filter.h"
#include "icq/icq.h"
#include "licq.h"
#include "logging/filelogsink.h"
#include "plugins/pluginmanager.h"

using namespace std;
using namespace LicqDaemon;
using Licq::gLog;
using Licq::PluginSignal;
using Licq::User;
using Licq::UserId;


// Declare global Daemon (internal for daemon)
LicqDaemon::Daemon LicqDaemon::gDaemon;

// Declare global Licq::Daemon to refer to the internal Daemon
Licq::Daemon& Licq::gDaemon(LicqDaemon::gDaemon);

const char* const Daemon::TranslationDir = "translations";
const char* const Daemon::UtilityDir = "utilities";

Daemon::Daemon() :
  licq(NULL)
{
  // Set static variables based on compile time constants
  myLibDir = INSTALL_LIBDIR;
  myShareDir = INSTALL_SHAREDIR;
}

Daemon::~Daemon()
{
  // Empty
}

void Daemon::setBaseDir(const string& baseDir)
{
  // This function is only called from Licq class and validation is done there
  myBaseDir = baseDir;

  // Make sure base dir always ends with a slash
  if (myBaseDir[myBaseDir.size()-1] != '/')
    myBaseDir += '/';
}

void Daemon::initialize()
{
  assert(licq != NULL);

  string temp;

  myShuttingDown = false;

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
  licqConf.get("ProxyServerType", myProxyType, ProxyTypeHttp);
  licqConf.get("ProxyServer", myProxyHost, "");
  licqConf.get("ProxyServerPort", myProxyPort, 0);
  licqConf.get("ProxyAuthEnabled", myProxyAuthEnabled, false);
  licqConf.get("ProxyLogin", myProxyLogin, "");
  licqConf.get("ProxyPassword", myProxyPasswd, "");

  // Rejects log file
  licqConf.get("Rejects", myRejectFile, "log.rejects");
  if (myRejectFile == "none")
    myRejectFile = "";

  // Error log file
  licqConf.get("Errors", myErrorFile, "log.errors");
  licqConf.get("ErrorTypes", myErrorTypes, 0x4 | 0x2); // error and unknown
  if (!myErrorFile.empty() && myErrorFile != "none")
  {
    string errorFile = baseDir() + myErrorFile;
    boost::shared_ptr<FileLogSink> logSink(new FileLogSink(errorFile));
    logSink->setLogLevelsFromBitmask(
        Licq::LogUtils::convertOldBitmaskToNew(myErrorTypes));
    logSink->setLogPackets(true);
    if (logSink->isOpen())
      getLogService().registerLogSink(logSink);
    else
      gLog.error("Unable to open %s as error log:\n%s",
                 errorFile.c_str(), strerror(errno));
  }

  // Loading translation table from file
  licqConf.get("Translation", temp, "none");
  if (!temp.empty() && temp != "none")
    Licq::gTranslator.setTranslationMap(shareDir() + TranslationDir + "/" + temp);

  // Misc
  licqConf.get("Terminal", myTerminal, "xterm -T Licq -e ");
  licqConf.get("SendTypingNotification", mySendTypingNotification, true);
  licqConf.get("IgnoreTypes", myIgnoreTypes, 0);

  // Initialize the random number generator
  srand(time(NULL));

  // start GPG helper
  LicqDaemon::gGpgHelper.Start();

  // Init event id counter
  myNextEventId = 1;
}

Licq::LogService& Daemon::getLogService()
{
  return licq->getLogService();
}

const char* Daemon::Version() const
{
  return licq->Version();
}

pthread_t* Daemon::Shutdown()
{
  if (myShuttingDown)
    return(&thread_shutdown);
  myShuttingDown = true;
  // Small race condition here if multiple plugins call shutdown at the same time
  pthread_create (&thread_shutdown, NULL, &Shutdown_tep, this);
  return (&thread_shutdown);
}

void Daemon::SaveConf()
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

  licqConf.set("Rejects", (myRejectFile.empty() ? "none" : myRejectFile));

  licqConf.set("Errors", myErrorFile);
  licqConf.set("ErrorTypes", myErrorTypes);

  string translation = Licq::gTranslator.getMapName();
  if (translation.empty())
    translation = "none";
  else
  {
    size_t pos = translation.rfind('/');
    if (pos != string::npos)
      translation.erase(0, pos+1);
  }
  licqConf.set("Translation", translation);
  licqConf.set("Terminal", myTerminal);
  licqConf.set("SendTypingNotification", mySendTypingNotification);
  licqConf.set("IgnoreTypes", myIgnoreTypes);

  licqConf.set("DefaultUserEncoding", gUserManager.defaultUserEncoding());

  licqConf.setSection("owners");
  licqConf.set("NumOfOwners", (unsigned long)gUserManager.NumOwners());

  int n = 1;
  {
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(Licq::Owner* owner, **ownerList)
    {
      Licq::OwnerWriteGuard o(owner);

      char szOwnerId[12], szOwnerPPID[14];
      char szPPID[5];
      sprintf(szOwnerId, "Owner%d.Id", n);
      sprintf(szOwnerPPID, "Owner%d.PPID", n++);

      o->SaveLicqInfo();
      if (o->accountId() != "0")
      {
        Licq::protocolId_toStr(szPPID, o->protocolId());
        licqConf.set(szOwnerId, o->accountId());
        licqConf.set(szOwnerPPID, szPPID);
      }
    }
  }

  gIcqProtocol.save(licqConf);

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

bool Licq::Daemon::haveCryptoSupport() const
{
#ifdef USE_OPENSSL
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

  if (s->Descriptor() != -1)
  {
    gLog.info(tr("Local TCP server started on port %d"), s->getLocalPort());
  }
  else if (s->Error() == EADDRINUSE)
  {
    gLog.warning(tr("No ports available for local TCP server."));
  }
  else
  {
    gLog.warning(tr("Failed to start local TCP server: %s"), s->errorStr().c_str());
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
    gLog.warning(tr("TCP high port (%d) is lower then TCP low port (%d)."),
        myTcpPortsHigh, myTcpPortsLow);
    myTcpPortsHigh = myTcpPortsLow + 10;
  }
}

void Licq::Daemon::setIgnoreType(unsigned type, bool ignore)
{
  if (ignore)
    myIgnoreTypes |= type;
  else
    myIgnoreTypes &= ~type;
}

Licq::Proxy* Licq::Daemon::createProxy()
{
  Licq::Proxy* Proxy = NULL;

  switch (myProxyType)
  {
    case ProxyTypeHttp:
      Proxy = new HttpProxy();
      break;
    default:
      break;
  }

  if (Proxy != NULL)
  {
    Proxy->setProxyAddr(myProxyHost, myProxyPort);
    if (myProxyAuthEnabled)
      Proxy->setProxyAuth(myProxyLogin, myProxyPasswd);
    Proxy->initProxy();
  }

  return Proxy;
}

unsigned long Daemon::getNextEventId()
{
  Licq::MutexLocker eventIdGuard(myNextEventIdMutex);
  unsigned long eventId = myNextEventId;
  if (++myNextEventId == 0)
    ++myNextEventId;
  return eventId;
}

bool Daemon::addUserEvent(Licq::User* u, Licq::UserEvent* e)
{
  int filteraction = gFilterManager.filterEvent(u, e);
  if (filteraction == Licq::FilterRule::ActionIgnore)
  {
    // Ignore => Just drop the event
    gLog.info("Event dropped by filter");
    delete e;
    return false;
  }

  if (u->isUser())
    e->AddToHistory(u, true);

  if (filteraction == Licq::FilterRule::ActionSilent)
  {
    // Accept silently => Logged to history but don't notify plugins
    delete e;
    return false;
  }

  // Don't log a user event if this user is on the ignore list
  if (u->IgnoreList() ||
      (e->IsMultiRec() && ignoreType(IgnoreMassMsg)) ||
      (e->eventType() == Licq::UserEvent::TypeEmailPager && ignoreType(IgnoreEmailPager)) ||
      (e->eventType() == Licq::UserEvent::TypeWebPanel && ignoreType(IgnoreWebPanel)) )
  {
    delete e;
    return false;
  }
  u->EventPush(e);
  //u->Touch();
  Licq::gStatistics.increase(Licq::Statistics::EventsReceivedCounter);

  //pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_EVENTS, u->id()));
  return true;
}

void Daemon::rejectEvent(const UserId& userId, Licq::UserEvent* e)
{
  if (myRejectFile.empty())
    return;

  string rejectFile = baseDir() + myRejectFile;
  FILE* f = fopen(rejectFile.c_str(), "a");
  if (f == NULL)
  {
    gLog.warning(tr("Unable to open \"%s\" for writing."), rejectFile.c_str());
  }
  else
  {
    fprintf(f, "Event from new user (%s) rejected: \n%s\n--------------------\n\n",
        userId.accountId().c_str(), e->text().c_str());
    chmod(rejectFile.c_str(), 00600);
    fclose(f);
  }
  delete e;
  Licq::gStatistics.increase(Licq::Statistics::EventsRejectedCounter);
}

void Licq::Daemon::cancelEvent(unsigned long eventId)
{
  gIcqProtocol.CancelEvent(eventId);
}

void Licq::Daemon::cancelEvent(Licq::Event* event)
{
  gIcqProtocol.CancelEvent(event);
}

void Licq::Daemon::PushPluginEvent(Licq::Event* e)
{
  LicqDaemon::gPluginManager.getPluginEventHandler().pushGeneralEvent(e);
}

void Licq::Daemon::pushPluginSignal(PluginSignal* s)
{
  LicqDaemon::gPluginManager.getPluginEventHandler().pushGeneralSignal(s);
}

void Licq::Daemon::PushProtoSignal(Licq::ProtocolSignal* s, unsigned long _nPPID)
{
  LicqDaemon::gPluginManager.getPluginEventHandler().pushProtocolSignal(s, _nPPID);
}

void Licq::Daemon::pluginUIViewEvent(const Licq::UserId& userId)
{
  pushPluginSignal(new PluginSignal(PluginSignal::SignalUiViewEvent, 0, userId));
}

void Licq::Daemon::pluginUIMessage(const Licq::UserId& userId)
{
  pushPluginSignal(new PluginSignal(PluginSignal::SignalUiMessage, 0, userId));
}

void Daemon::shutdownPlugins()
{
  licq->ShutdownPlugins();
}

void Daemon::autoLogon()
{
  map<UserId, unsigned> logonStatuses;
  {
    // Get statuses first as we cannot call setStatus while list is locked
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(const Licq::Owner* owner, **ownerList)
    {
      Licq::OwnerReadGuard o(owner);
      if (o->startupStatus() != Licq::User::OfflineStatus)
        logonStatuses[o->id()] = o->startupStatus();
    }
  }

  map<UserId, unsigned>::const_iterator iter;
  for (iter = logonStatuses.begin(); iter != logonStatuses.end(); ++iter)
    Licq::gProtocolManager.setStatus(iter->first, iter->second);
}
