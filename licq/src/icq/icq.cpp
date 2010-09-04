// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998-2010 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#include "config.h"

#include "icq.h"

#include <boost/foreach.hpp>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <ctime>
#include <sys/stat.h>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/event.h>
#include <licq/icqdefines.h>
#include <licq/inifile.h>
#include <licq/logging/log.h>
#include <licq/logging/logservice.h>
#include <licq/logging/logutils.h>
#include <licq/statistics.h>
#include <licq/oneventmanager.h>
#include <licq/pluginsignal.h>
#include <licq/proxy.h>
#include <licq/translator.h>
#include <licq/userevents.h>

#include "../daemon.h"
#include "../gettext.h"
#include "../logging/filelogsink.h"
#include "../support.h"
#include "oscarservice.h"
#include "packet.h"

using namespace std;
using namespace LicqDaemon;
using Licq::OnEventData;
using Licq::gLog;
using Licq::gOnEventManager;

// Constants
const char* const CICQDaemon::DefaultServerHost = "login.icq.com";

std::list <CReverseConnectToUserData *> IcqProtocol::m_lReverseConnect;
pthread_mutex_t IcqProtocol::mutex_reverseconnect = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t IcqProtocol::cond_reverseconnect_done = PTHREAD_COND_INITIALIZER;


CICQDaemon *gLicqDaemon = NULL;
IcqProtocol gIcqProtocol;
Licq::SocketManager gSocketManager;

IcqProtocol::IcqProtocol()
{
  // Empty
}

IcqProtocol::~IcqProtocol()
{
  gLicqDaemon = NULL;
}

void IcqProtocol::initialize()
{
  gLicqDaemon = this;

  // Initialise the data values
  m_bAutoUpdateInfo = m_bAutoUpdateInfoPlugins = m_bAutoUpdateStatusPlugins
                    = true;
  m_nTCPSocketDesc = -1;
  m_nTCPSrvSocketDesc = -1;
  m_eStatus = STATUS_OFFLINE_MANUAL;
  //just in case we need to sign on automatically
  m_nDesiredStatus = ICQ_STATUS_ONLINE;
  m_bRegistering = false;
  m_nServerAck = 0;
  m_bLoggingOn = false;
  m_bOnlineNotifies = true;
  m_bVerify = false;
  m_bNeedSalt = true;
  m_nRegisterThreadId = 0;

  receivedUserList.clear();

  // Begin parsing the config file
  Licq::IniFile licqConf("licq.conf");
  licqConf.loadFile();

  licqConf.setSection("network");

  // ICQ Server
  licqConf.get("ICQServer", myIcqServer, DefaultServerHost);
  licqConf.get("ICQServerPort", myIcqServerPort, DefaultServerPort);

  licqConf.get("MaxUsersPerPacket", myMaxUsersPerPacket, 100);
  licqConf.get("AutoUpdateInfo", m_bAutoUpdateInfo, true);
  licqConf.get("AutoUpdateInfoPlugins", m_bAutoUpdateInfoPlugins, true);
  licqConf.get("AutoUpdateStatusPlugins", m_bAutoUpdateStatusPlugins, true);
  unsigned long nColor;
  licqConf.get("ForegroundColor", nColor, 0x00000000);
  Licq::Color::setDefaultForeground(nColor);
  licqConf.get("BackgroundColor", nColor, 0x00FFFFFF);
  Licq::Color::setDefaultBackground(nColor);

  // Error log file
  // TODO: Move this to the daemon
  licqConf.get("Errors", myErrorFile, "log.errors");
  licqConf.get("ErrorTypes", myErrorTypes, 0x4 | 0x2); // error and unknown
  if (myErrorFile != "none")
  {
    string errorFile = Licq::gDaemon.baseDir() + myErrorFile;
    boost::shared_ptr<LicqDaemon::FileLogSink> logSink(
        new LicqDaemon::FileLogSink(errorFile));
    logSink->setLogLevelsFromBitmask(
        Licq::LogUtils::convertOldBitmaskToNew(myErrorTypes));
    logSink->setLogPackets(true);
    if (logSink->isOpen())
      gDaemon.getLogService().registerLogSink(logSink);
    else
      gLog.error("Unable to open %s as error log:\n%s",
                 errorFile.c_str(), strerror(errno));
  }

  // Proxy
  m_xProxy = NULL;

  // Services
  m_xBARTService = NULL;

  // Misc
  licqConf.get("UseSS", m_bUseSS, true); // server side list
  licqConf.get("UseBART", m_bUseBART, true); // server side buddy icons
  licqConf.get("ReconnectAfterUinClash", m_bReconnectAfterUinClash, false);

  // Start up our threads
  pthread_mutex_init(&mutex_runningevents, NULL);
  pthread_mutex_init(&mutex_extendedevents, NULL);
  pthread_mutex_init(&mutex_sendqueue_server, NULL);
  pthread_mutex_init(&mutex_modifyserverusers, NULL);
  pthread_mutex_init(&mutex_cancelthread, NULL);
  pthread_cond_init(&cond_serverack, NULL);
  pthread_mutex_init(&mutex_serverack, NULL);
}

bool IcqProtocol::start()
{
  int nResult = 0;

  Licq::TCPSocket* s = new Licq::TCPSocket();
  m_nTCPSocketDesc = gDaemon.StartTCPServer(s);
  if (m_nTCPSocketDesc == -1)
  {
     gLog.error(tr("%sUnable to allocate TCP port for local server (%s)!\n"),
                L_ERRORxSTR, tr("No ports available"));
     return false;
  }
  gSocketManager.AddSocket(s);
  {
    Licq::OwnerWriteGuard o(LICQ_PPID);
    if (o.isLocked())
    {
      o->SetIntIp(s->getLocalIpInt());
      o->SetPort(s->getLocalPort());
    }
  }
  CPacket::SetLocalPort(s->getLocalPort());
  gSocketManager.DropSocket(s);

  gLog.info(tr("Spawning daemon threads"));
  nResult = pthread_create(&thread_monitorsockets, NULL, &MonitorSockets_tep, this);
  if (nResult != 0)
  {
    gLog.error("%sUnable to start socket monitor thread:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
    return false;
  }
  nResult = pthread_create(&thread_ping, NULL, &Ping_tep, this);
  if (nResult != 0)
  {
    gLog.error("%sUnable to start ping thread:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
    return false;
  }
  
  nResult = pthread_create(&thread_updateusers, NULL, &UpdateUsers_tep, this);
  if (nResult != 0)
  {
    gLog.error("%sUnable to start users update thread:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
    return false;
  }

  if (UseServerSideBuddyIcons())
  {
    m_xBARTService = new COscarService(ICQ_SNACxFAM_BART);
    nResult = pthread_create(&thread_ssbiservice, NULL,
                             &OscarServiceSendQueue_tep, m_xBARTService);
    if (nResult != 0)
    {
      gLog.error(tr("%sUnable to start BART service thread:\n%s%s.\n"),
                 L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
      return false;
    }
  }

  return true;
}

void IcqProtocol::save(Licq::IniFile& licqConf)
{
  licqConf.setSection("network");

  // ICQ Server
  licqConf.set("ICQServer", myIcqServer);
  licqConf.set("ICQServerPort", myIcqServerPort);

  licqConf.set("MaxUsersPerPacket", myMaxUsersPerPacket);
  licqConf.set("AutoUpdateInfo", m_bAutoUpdateInfo);
  licqConf.set("AutoUpdateInfoPlugins", m_bAutoUpdateInfoPlugins);
  licqConf.set("AutoUpdateStatusPlugins", m_bAutoUpdateStatusPlugins);
  licqConf.set("ForegroundColor", Licq::Color::defaultForeground());
  licqConf.set("BackgroundColor", Licq::Color::defaultBackground());

  licqConf.set("Errors", myErrorFile);
  licqConf.set("ErrorTypes", myErrorTypes);

  // Misc
  licqConf.set("UseSS", m_bUseSS); // server side list
  licqConf.set("UseBART", m_bUseBART); // server side buddy icons
  licqConf.set("ReconnectAfterUinClash", m_bReconnectAfterUinClash);
}

void CICQDaemon::SetDirectMode()
{
  bool bDirect = (!gDaemon.behindFirewall() || (gDaemon.behindFirewall() && gDaemon.tcpEnabled()));
  CPacket::SetMode(bDirect ? MODE_DIRECT : MODE_INDIRECT);
}

void CICQDaemon::InitProxy()
{
  if (m_xProxy != NULL)
  {
    delete m_xProxy;
    m_xProxy = NULL;
  }
  m_xProxy = gDaemon.createProxy();
}

unsigned short VersionToUse(unsigned short v_in)
{
  /*if (ICQ_VERSION_TCP & 4 && v & 4) return 4;
  if (ICQ_VERSION_TCP & 2 && v & 2) return 2;
  gLog.warning("%sUnknown TCP version %d.  Attempting v2.\n", L_WARNxSTR, v);
  return 2;*/
  unsigned short v_out = v_in < ICQ_VERSION_TCP ? v_in : ICQ_VERSION_TCP;
  if (v_out < 2 || v_out == 5)
  {
    if (v_out == 5)
      v_out = 4;
    else
      v_out = 6;

    gLog.warning(tr("%sInvalid TCP version %d.  Attempting v%d.\n"), L_WARNxSTR, v_in,
                                                              v_out);
  }
  return v_out;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void IcqProtocol::SetUseServerSideBuddyIcons(bool b)
{
  if (b && m_xBARTService == NULL)
  {
    m_xBARTService = new COscarService(ICQ_SNACxFAM_BART);
    int nResult = pthread_create(&thread_ssbiservice, NULL,
                                 &OscarServiceSendQueue_tep, m_xBARTService);
    if (nResult != 0)
    {
      gLog.error(tr("%sUnable to start BART service thread:\n%s%s.\n"),
                 L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
    }
    else
      m_bUseBART = true;
  }
  else
    m_bUseBART = b;
}

void IcqProtocol::ChangeUserStatus(Licq::User* u, unsigned long s)
{
  u->statusChanged(Licq::User::statusFromIcqStatus(s), s);
}

/*----------------------------------------------------------------------------
 * CICQDaemon::SendEvent
 *
 * Sends an event without expecting a reply.
 *--------------------------------------------------------------------------*/

void IcqProtocol::SendEvent_Server(CPacket *packet)
{
#if 1
  unsigned long eventId = gDaemon.getNextEventId();
  Licq::Event* e = new Licq::Event(eventId, m_nTCPSrvSocketDesc, packet, Licq::Event::ConnectServer);

  if (e == NULL)  return;
 
  pthread_mutex_lock(&mutex_sendqueue_server);
  m_lxSendQueue_Server.push_back(e);
  pthread_mutex_unlock(&mutex_sendqueue_server);

  e->m_NoAck = true;
  int nResult = pthread_create(&e->thread_send, NULL, &ProcessRunningEvent_Server_tep, e);
  if (nResult != 0)
  {
    gLog.error("%sUnable to start server event thread (#%hu):\n%s%s.\n", L_ERRORxSTR,
       e->m_nSequence, L_BLANKxSTR, strerror(nResult));
    e->m_eResult = Licq::Event::ResultError;
  }
#else
  SendEvent(m_nTCPSrvSocketDesc, *packet, true);
#endif
}

Licq::Event* IcqProtocol::SendExpectEvent_Server(unsigned long eventId, const Licq::UserId& userId,
    Licq::Packet *packet, Licq::UserEvent *ue, bool bExtendedEvent)
{
  // If we are already shutting down, don't start any events
  if (gDaemon.shuttingDown())
  {
    if (packet != NULL) delete packet;
    if (ue != NULL) delete ue;
    return NULL;
  }

  if (ue != NULL)
    ue->setIsReceiver(false);
  Licq::Event* e = new Licq::Event(eventId, m_nTCPSrvSocketDesc, packet, Licq::Event::ConnectServer, userId, ue);

	if (e == NULL)  return NULL;

  if (bExtendedEvent) PushExtendedEvent(e);

  Licq::Event *result = SendExpectEvent(e, &ProcessRunningEvent_Server_tep);

  // if an error occured, remove the event from the extended queue as well
  if (result == NULL && bExtendedEvent)
  {
    pthread_mutex_lock(&mutex_extendedevents);
    std::list<Licq::Event*>::iterator i;
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

Licq::Event* IcqProtocol::SendExpectEvent_Client(unsigned long eventId, const Licq::User* pUser,
    Licq::Packet* packet, Licq::UserEvent *ue)
{
  // If we are already shutting down, don't start any events
  if (gDaemon.shuttingDown())
  {
    if (packet != NULL) delete packet;
    if (ue != NULL) delete ue;
    return NULL;
  }

  if (ue != NULL)
    ue->setIsReceiver(false);
  Licq::Event* e = new Licq::Event(eventId, pUser->SocketDesc(packet->Channel()), packet,
      Licq::Event::ConnectUser, pUser->id(), ue);

  if (e == NULL) return NULL;

  return SendExpectEvent(e, &ProcessRunningEvent_Client_tep);
}


Licq::Event* IcqProtocol::SendExpectEvent(Licq::Event* e, void *(*fcn)(void *))
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
    gLog.error("%sUnable to start event thread (#%hu):\n%s%s.\n", L_ERRORxSTR,
       e->m_nSequence, L_BLANKxSTR, strerror(nResult));
    DoneEvent(e, Licq::Event::ResultError);
    if (e->m_nSocketDesc == m_nTCPSrvSocketDesc)
    {
      pthread_mutex_lock(&mutex_sendqueue_server);
      list<Licq::Event*>::iterator iter;
      for (iter = m_lxSendQueue_Server.begin();
           iter != m_lxSendQueue_Server.end(); ++iter)
      {
        if (e == *iter)
        {
          m_lxSendQueue_Server.erase(iter);

          Licq::Event* cancelled = new Licq::Event(e);
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
bool IcqProtocol::SendEvent(int nSD, CPacket &p, bool d)
{
  Licq::INetSocket* s = gSocketManager.FetchSocket(nSD);
  if (s == NULL) return false;
  bool r = SendEvent(s, p, d);
  gSocketManager.DropSocket(s);
  return r;
}

bool IcqProtocol::SendEvent(Licq::INetSocket* pSock, CPacket &p, bool d)
{
  CBuffer *buf = p.Finalize(pSock);
  pSock->Send(buf);
  if (d) delete buf;
  return true;
}


//---FailEvents----------------------------------------------------------------
/*! \brief Fails all events on the given socket. */
void IcqProtocol::FailEvents(int sd, int err)
{
  // Go through all running events and fail all from this socket
  Licq::Event* e = NULL;
  do
  {
    e = NULL;
    pthread_mutex_lock(&mutex_runningevents);
    list<Licq::Event*>::iterator iter;
    for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); ++iter)
    {
      if ((*iter)->m_nSocketDesc == sd)
      {
        e = *iter;
        break;
      }
    }
    pthread_mutex_unlock(&mutex_runningevents);
    if (e != NULL && DoneEvent(e, Licq::Event::ResultError) != NULL)
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
bool IcqProtocol::hasServerEvent(unsigned long _nSubSequence) const
{
  bool hasEvent = false;
  pthread_mutex_lock(&mutex_runningevents);
  list<Licq::Event*>::const_iterator iter;
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
Licq::Event* IcqProtocol::DoneServerEvent(unsigned long _nSubSeq, Licq::Event::ResultType _eResult)
{
  pthread_mutex_lock(&mutex_runningevents);
  Licq::Event* e = NULL;
  list<Licq::Event*>::iterator iter;
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
Licq::Event* IcqProtocol::DoneEvent(Licq::Event* e, Licq::Event::ResultType _eResult)
{
  pthread_mutex_lock(&mutex_runningevents);
  list<Licq::Event*>::iterator iter;
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
    gLog.info("doneevents: for: %p pending: \n", e);
    for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); ++iter)
    {
      gLog.info("%p Command: %d SubCommand: %d Sequence: %hu SubSequence: %d: Uin: %lu\n", *iter,
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
  if (_eResult == Licq::Event::ResultCancelled && e->m_nSocket == m_nUDPSocketDesc)
  {
    pthread_mutex_lock(&mutex_runningevents);
    Licq::Event* e2 = new Licq::Event(e);
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
Licq::Event* IcqProtocol::DoneEvent(int _nSD, unsigned short _nSequence, Licq::Event::ResultType _eResult)
{
  pthread_mutex_lock(&mutex_runningevents);
  Licq::Event* e = NULL;
  list<Licq::Event*>::iterator iter;
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

Licq::Event* IcqProtocol::DoneEvent(unsigned long tag, Licq::Event::ResultType _eResult)
{
  pthread_mutex_lock(&mutex_runningevents);
  Licq::Event* e = NULL;
  list<Licq::Event*>::iterator iter;
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
void CICQDaemon::ProcessDoneEvent(Licq::Event* e)
{
#if ICQ_VERSION != 5
  static unsigned short s_nPingTimeOuts = 0;
#endif

  // Determine this now as later we might have deleted the event
  unsigned short nCommand = e->m_nCommand;
  Licq::Event::ResultType eResult = e->m_eResult;

  // Write the event to the history file if appropriate
  if (e->m_pUserEvent != NULL &&
      e->m_eResult == Licq::Event::ResultAcked &&
      e->m_nSubResult != ICQ_TCPxACK_RETURN)
  {
    Licq::UserReadGuard(e->userId());
    if (u.isLocked())
    {
      e->m_pUserEvent->AddToHistory(*u, false);
      u->SetLastSentEvent();
      m_xOnEventManager.Do(ON_EVENT_MSGSENT, *u);
    }
    Licq::gStatistics.increase(Licq::Statistics::EventsSentCounter);
  }

  // Process the event
  switch (e->m_nCommand)
  {
  // Ping is always sent by the daemon
  case ICQ_CMDxSND_PING:
#if ICQ_VERSION != 5
      if (e->m_eResult == Licq::Event::ResultAcked)
      s_nPingTimeOuts = 0;
    else
    {
      s_nPingTimeOuts++;
      if (s_nPingTimeOuts > MaxPingTimeouts)
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
      if (e->m_eResult == Licq::Event::ResultAcked)
      {
        Licq::OwnerWriteGuard o(LICQ_PPID);
      ChangeUserStatus(o, ((CPU_SetStatus *)e->m_pPacket)->Status() );
      }
    PushPluginEvent(e);
    break;

  case ICQ_CMDxSND_SETxRANDOMxCHAT:
      if (e->m_eResult == Licq::Event::ResultAcked)
      {
        Licq::OwnerWriteGuard o(LICQ_PPID);
      o->SetRandomChatGroup(((CPU_SetRandomChatGroup *)e->m_pPacket)->Group());
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
        case Licq::Event::ResultError:
        case Licq::Event::ResultTimedout:
        case Licq::Event::ResultFailed:
        case Licq::Event::ResultSuccess:
        case Licq::Event::ResultCancelled:
        PushPluginEvent(e);
          break;
        case Licq::Event::ResultAcked:  // push to extended event list
        PushExtendedEvent(e);
        break;
      default:
        gLog.error("%sInternal error: ProcessDoneEvents(): Invalid result for extended event (%d).\n",
                   L_ERRORxSTR, e->m_eResult);
        delete e;
        return;
    }
    break;
  }

  default:
    gLog.error("%sInternal error: ProcessDoneEvents(): Unknown command (%04X).\n",
               L_ERRORxSTR, e->m_nCommand);
    delete e;
    return;
  }

  // Some special commands to deal with
#if ICQ_VERSION == 5
  if (nCommand != ICQ_CMDxTCP_START &&
      (eResult == Licq::Event::ResultTimedout || eResult == Licq::Event::ResultError) )
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
Licq::Event* IcqProtocol::DoneExtendedServerEvent(const unsigned short _nSubSequence, Licq::Event::ResultType _eResult)
{
  pthread_mutex_lock(&mutex_extendedevents);
  Licq::Event* e = NULL;
  list<Licq::Event*>::iterator iter;
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


Licq::Event* IcqProtocol::DoneExtendedEvent(Licq::Event* e, Licq::Event::ResultType _eResult)
{
  pthread_mutex_lock(&mutex_extendedevents);
  list<Licq::Event*>::iterator iter;
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
  if (_eResult == Licq::Event::ResultCancelled)
  {
    pthread_mutex_lock(&mutex_extendedevents);
    Licq::Event* e2 = new Licq::Event(e);
    e2->m_bCancelled = true;
    e2->m_xPacket = e->m_xPacket;
    e->m_xPacket = NULL;
    m_lxExtendedEvents.push_back(e2);
    pthread_mutex_unlock(&mutex_extendedevents);
  }
#endif
  return(e);
}

Licq::Event* IcqProtocol::DoneExtendedEvent(unsigned long tag, Licq::Event::ResultType _eResult)
{
  pthread_mutex_lock(&mutex_extendedevents);
  Licq::Event* e = NULL;
  list<Licq::Event*>::iterator iter;
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

void IcqProtocol::PushEvent(Licq::Event* e)
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
void IcqProtocol::PushExtendedEvent(Licq::Event* e)
{
  assert(e != NULL);
  pthread_mutex_lock(&mutex_extendedevents);
  m_lxExtendedEvents.push_back(e);
#if 0
  gLog.info("%p pushing Command: %d SubCommand: %d Sequence: %hu SubSequence: %d: Uin: %lu\n", e,
            e->Command(), e->SubCommand(), e->Sequence(), e->SubSequence(), e->Uin());
#endif
  pthread_mutex_unlock(&mutex_extendedevents);
}

//-----CICQDaemon::CancelEvent---------------------------------------------------------
void IcqProtocol::CancelEvent(unsigned long t)
{
  Licq::Event* eSrv = NULL;
  pthread_mutex_lock(&mutex_sendqueue_server);
  list<Licq::Event*>::iterator iter;
  for (iter = m_lxSendQueue_Server.begin();
       iter != m_lxSendQueue_Server.end(); ++iter)
  {
    if ((*iter)->Equals(t))
    {
      eSrv = *iter;
      m_lxSendQueue_Server.erase(iter);

      Licq::Event* cancelled = new Licq::Event(eSrv);
      cancelled->m_bCancelled = true;
      m_lxSendQueue_Server.push_back(cancelled);
      break;
    }
  }
  pthread_mutex_unlock(&mutex_sendqueue_server);

  Licq::Event* eRun = DoneEvent(t, Licq::Event::ResultCancelled);
  Licq::Event* eExt = DoneExtendedEvent(t, Licq::Event::ResultCancelled);

  if (eRun == NULL && eExt == NULL && eSrv == NULL)
  {
    gLog.warning(tr("%sCancelled event not found.\n"), L_WARNxSTR);
    return;
  }

  CancelEvent((eRun != NULL)? eRun : (eExt != NULL)? eExt : eSrv);
}

void IcqProtocol::CancelEvent(Licq::Event* e)
{
  e->m_eResult = Licq::Event::ResultCancelled;

  if (e->m_nSubCommand == ICQ_CMDxSUB_CHAT)
    icqChatRequestCancel(e->userId(), e->m_nSequence);
  else if (e->m_nSubCommand == ICQ_CMDxSUB_FILE)
    icqFileTransferCancel(e->userId(), e->m_nSequence);
  else if (e->m_nSubCommand == ICQ_CMDxSUB_SECURExOPEN)
    icqOpenSecureChannelCancel(e->userId(), e->m_nSequence);

  ProcessDoneEvent(e);
}


//-----updateAllUsers-------------------------------------------------------------------------
void IcqProtocol::UpdateAllUsers()
{
  Licq::UserListGuard userList;
  BOOST_FOREACH(Licq::User* u, **userList)
    icqRequestMetaInfo(u->id());
}

void IcqProtocol::updateAllUsersInGroup(int groupId)
{
  Licq::UserListGuard userList;
  BOOST_FOREACH(Licq::User* user, **userList)
  {
    Licq::UserReadGuard pUser(user);
    if (pUser->isInGroup(groupId))
      icqRequestMetaInfo(pUser->id());
  }
}

//-----ProcessMessage-----------------------------------------------------------
void IcqProtocol::ProcessMessage(Licq::User *u, CBuffer &packet, char *message,
    unsigned short nMsgType, unsigned long nMask, const unsigned long nMsgID[2],
                                unsigned short nSequence, bool bIsAck,
                                bool &bNewUser)
{
  char *szType = NULL;
  Licq::UserEvent* pEvent = NULL;
  OnEventData::OnEventType onEventType = OnEventData::OnEventMessage;

  // for acks
  unsigned short nPort;

  // Do we accept it if we are in Occ or DND?
  // unsigned short nOwnerStatus;
  // {
  //   Licq::OwnerReadGuard o(LICQ_PPID);
  //   nOwnerStatus = o->Status();
  // }

  unsigned short nLevel = nMask;
  unsigned long nFlags = ((nMask & ICQ_CMDxSUB_FxMULTIREC) ? (unsigned)Licq::UserEvent::FlagMultiRec : 0)
      | ((nMask & ICQ_TCPxMSG_URGENT) ? (unsigned)Licq::UserEvent::FlagUrgent : 0);

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

      Licq::EventMsg* e = new Licq::EventMsg(Licq::gTranslator.serverToClient(message),
          ICQ_CMDxRCV_SYSxMSGxONLINE, Licq::EventMsg::TimeNow, nFlags);
    e->SetColor(fore, back);

    CPU_AckGeneral *p = new CPU_AckGeneral(u, nMsgID[0], nMsgID[1],
                                           nSequence, ICQ_CMDxSUB_MSG, true,
                                           nLevel);
    SendEvent_Server(p);

    szType = strdup(tr("Message"));
      onEventType = OnEventData::OnEventMessage;
    pEvent = e;
    break;
  }

  case ICQ_CMDxSUB_CHAT:
  {
    char szChatClients[1024];
    unsigned short nPortReversed;

      Licq::gTranslator.ServerToClient(message);

    packet.UnpackString(szChatClients, sizeof(szChatClients));
    nPortReversed = packet.UnpackUnsignedShortBE();
    packet.incDataPosRead(2);
    nPort = packet.UnpackUnsignedShort();

    if (nPort == 0)
      nPort = nPortReversed;

    if (!bIsAck)
    {
        Licq::EventChat* e = new Licq::EventChat(message, szChatClients, nPort,
            nSequence, Licq::EventChat::TimeNow, nFlags, 0, nMsgID[0], nMsgID[1]);
        onEventType = OnEventData::OnEventChat;
      pEvent = e;
    }

    szType = strdup(tr("Chat request"));
    break;
  }

  case ICQ_CMDxSUB_FILE:
  {
    unsigned short nFilenameLen;
    unsigned long nFileSize;

    Licq::gTranslator.ServerToClient(message);

    // Port reversed: garbage when the request is refused
    packet.UnpackUnsignedShortBE();

    packet.UnpackUnsignedShort();

    packet >> nFilenameLen;
    if (!bIsAck)
    {
        string filename = packet.unpackRawString(nFilenameLen);
      packet >> nFileSize;
        list<string> filelist;
        filelist.push_back(filename);

        Licq::EventFile* e = new Licq::EventFile(filename.c_str(), message, nFileSize,
            filelist, nSequence, Licq::EventFile::TimeNow, nFlags, 0, nMsgID[0], nMsgID[1]);
        onEventType = OnEventData::OnEventFile;
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
      Licq::EventUrl* e = Licq::EventUrl::Parse(message, ICQ_CMDxRCV_SYSxMSGxONLINE,
          Licq::EventUrl::TimeNow, nFlags);
    CPU_AckGeneral *p = new CPU_AckGeneral(u, nMsgID[0], nMsgID[1],
                                           nSequence, ICQ_CMDxSUB_URL, true,
                                           nLevel);
    SendEvent_Server(p);

    szType = strdup(tr("URL"));
      onEventType = OnEventData::OnEventUrl;
    pEvent = e;
    break;
  }

  case ICQ_CMDxSUB_CONTACTxLIST:
  {
      Licq::EventContactList* e = Licq::EventContactList::Parse(message,
          ICQ_CMDxRCV_SYSxMSGxONLINE, Licq::EventContactList::TimeNow, nFlags);
    CPU_AckGeneral *p = new CPU_AckGeneral(u, nMsgID[0], nMsgID[1],
                                           nSequence, ICQ_CMDxSUB_CONTACTxLIST,
                                           true, nLevel);
    SendEvent_Server(p);

    szType = strdup(tr("Contact list"));
      onEventType = OnEventData::OnEventMessage;
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
      if (u->autoResponse() != message)
      {
        u->setAutoResponse(message);
        u->SetShowAwayMsg(*message);
        gLog.info(tr("%sAuto response from %s (#%lu).\n"), L_SRVxSTR, u->GetAlias(),
                  nMsgID[1]);
        }
        Licq::Event* e = DoneServerEvent(nMsgID[1], Licq::Event::ResultAcked);
        if (e)
      {
        e->m_pExtendedAck = new Licq::ExtendedData(true, 0, message);
        e->m_nSubResult = ICQ_TCPxACK_RETURN;
        ProcessDoneEvent(e);
      }
      else
        gLog.warning(tr("%sAck for unknown event.\n"), L_SRVxSTR);
    }
    else
    {
      gLog.info(tr("%s%s (%s) requested auto response.\n"), L_SRVxSTR,
            u->getAlias().c_str(), u->accountId().c_str());

    CPU_AckGeneral *p = new CPU_AckGeneral(u, nMsgID[0], nMsgID[1],
                                           nSequence, nMsgType, true, nLevel);
    SendEvent_Server(p);

        Licq::gStatistics.increase(Licq::Statistics::AutoResponseCheckedCounter);
    u->SetLastCheckedAutoResponse();

        gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
            Licq::PluginSignal::UserEvents, u->id()));
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
        gLog.warning(tr("%sUnknown ICBM plugin type: %s\n"), L_SRVxSTR, plugin.c_str());
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
    ProcessMessage(u, packet, msg, nCommand, nMask, nMsgID,
                   nSequence, bIsAck, bNewUser);
      delete [] szMessage;
    return;

    break; // bah!
  }

  default:
      Licq::gTranslator.ServerToClient(message);
    szType = strdup(tr("unknown event"));
  } // switch nMsgType

  if (bIsAck)
  {
    Licq::Event* pAckEvent = DoneServerEvent(nMsgID[1], Licq::Event::ResultAcked);
    Licq::ExtendedData* pExtendedAck = new Licq::ExtendedData(true, nPort, message);

    if (pAckEvent)
    {
      pAckEvent->m_pExtendedAck = pExtendedAck;
      pAckEvent->m_nSubResult = ICQ_TCPxACK_ACCEPT;
      gLog.info(tr("%s%s accepted from %s (%s).\n"), L_SRVxSTR, szType,
          u->getAlias().c_str(), u->accountId().c_str());
      u->unlockWrite();
      ProcessDoneEvent(pAckEvent);
      u->lockWrite();
    }
    else
    {
      gLog.warning(tr("%sAck for unknown event.\n"), L_SRVxSTR);
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
        if (gDaemon.ignoreType(Licq::Daemon::IgnoreNewUsers))
        {
          gLog.info(tr("%s%s from new user (%s), ignoring.\n"), L_SRVxSTR,
              szType, u->accountId().c_str());
          if (szType)  free(szType);
          gDaemon.rejectEvent(u->id(), pEvent);
          return;
        }
        gLog.info(tr("%s%s from new user (%s).\n"), L_SRVxSTR, szType, u->accountId().c_str());

        // Don't delete user when we're done
        bNewUser = false;
      }
      else
        gLog.info(tr("%s%s from %s (%s).\n"), L_SRVxSTR, szType, u->GetAlias(),
            u->accountId().c_str());

      if (gDaemon.addUserEvent(u, pEvent))
        gOnEventManager.performOnEvent(onEventType, u);
    }
    else // invalid parse or unknown event
    {
      packet.log(Licq::Log::Warning, tr("Invalid %s"), szType);
    }
  }

  if (szType)  free(szType);
}

bool IcqProtocol::waitForReverseConnection(unsigned short id, const Licq::UserId& userId)
{
  bool bSuccess = false;
  pthread_mutex_lock(&mutex_reverseconnect);

  std::list<CReverseConnectToUserData *>::iterator iter;
  for (iter = m_lReverseConnect.begin(); iter != m_lReverseConnect.end();
    ++iter)
  {
    if ((*iter)->nId == id && (*iter)->myIdString == userId.accountId())
      break;
  }

  if (iter == m_lReverseConnect.end())
  {
    gLog.warning("%sFailed to find desired connection record.\n", L_WARNxSTR);
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
        gLog.warning("%sSomebody else removed our connection record.\n",
          L_WARNxSTR);
        goto done;
      }
      if ((*iter)->nId == id && (*iter)->myIdString == userId.accountId())
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
    if ((*iter)->nId == id && (*iter)->myIdString == userId.accountId())
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

Licq::Event* IcqProtocol::SendExpectEvent_Server(const Licq::UserId& userId, Licq::Packet* packet, Licq::UserEvent* ue, bool extendedEvent)
{
  return SendExpectEvent_Server(gDaemon.getNextEventId(), userId, packet, ue, extendedEvent);
}

Licq::Event* IcqProtocol::SendExpectEvent_Server(Licq::Packet* packet, Licq::UserEvent* ue, bool extendedEvent)
{
  return SendExpectEvent_Server(gDaemon.getNextEventId(), Licq::UserId(), packet, ue, extendedEvent);
}

Licq::Event* IcqProtocol::SendExpectEvent_Client(const Licq::User* user, Licq::Packet* packet, Licq::UserEvent* ue)
{
  return SendExpectEvent_Client(gDaemon.getNextEventId(), user, packet, ue);
}

string CICQDaemon::getXmlTag(const string& xmlSource, const string& tagName)
{
  size_t startPos = xmlSource.find("<" + tagName + ">");
  size_t endPos = xmlSource.find("</" + tagName + ">");
  if (startPos == string::npos || endPos == string::npos)
    return "";
  startPos += tagName.size() + 2;
  if (startPos > endPos)
    return "";
  return xmlSource.substr(startPos, endPos - startPos);
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
