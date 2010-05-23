// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#include "config.h"

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <ctime>
#include <sys/stat.h>

#include <licq/oneventmanager.h>
#include "licq_icq.h"
#include "licq_user.h"
#include "licq_constants.h"
#include "licq_log.h"
#include "licq_translate.h"

#include "contactlist/user.h"
#include "gettext.h"
#include "icqpacket.h"
#include "oscarservice.h"
#include "support.h"

#include "licq_icqd.h"

using namespace std;
using namespace LicqDaemon;
using Licq::OnEventManager;
using Licq::gOnEventManager;

std::list <CReverseConnectToUserData *> CICQDaemon::m_lReverseConnect;
pthread_mutex_t CICQDaemon::mutex_reverseconnect = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  CICQDaemon::cond_reverseconnect_done = PTHREAD_COND_INITIALIZER;


bool CICQDaemon::startIcq()
{
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
    m_xBARTService = new COscarService(ICQ_SNACxFAM_BART);
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

void CICQDaemon::SetDirectMode()
{
  bool bDirect = (!m_bFirewall || (m_bFirewall && m_bTCPEnabled));
  CPacket::SetMode(bDirect ? MODE_DIRECT : MODE_INDIRECT);
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
    m_xBARTService = new COscarService(ICQ_SNACxFAM_BART);
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

bool CICQDaemon::AddUserEvent(ICQUser *u, CUserEvent *e)
{
  if (u->isUser())
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
  if (myRejectFile.empty())
    return;

  string rejectFile = BASE_DIR + myRejectFile;
  FILE *f = fopen(rejectFile.c_str(), "a");
  if (f == NULL)
  {
    gLog.Warn(tr("%sUnable to open \"%s\" for writing.\n"), L_WARNxSTR, rejectFile.c_str());
  }
  else
  {
    fprintf(f, "Event from new user (%s) rejected: \n%s\n--------------------\n\n",
        LicqUser::getUserAccountId(userId).c_str(), e->Text());
    chmod(rejectFile.c_str(), 00600);
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
  unsigned long eventId = getNextEventId();
  LicqEvent* e = new LicqEvent(eventId, m_nTCPSrvSocketDesc, packet, CONNECT_SERVER);

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

LicqEvent* CICQDaemon::SendExpectEvent_Server(unsigned long eventId, const UserId& userId,
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
  LicqEvent* e = new LicqEvent(eventId, m_nTCPSrvSocketDesc, packet, CONNECT_SERVER, userId, ue);

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

ICQEvent* CICQDaemon::SendExpectEvent_Client(unsigned long eventId, const LicqUser* pUser, CPacket* packet,
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
  LicqEvent* e = new LicqEvent(eventId, pUser->SocketDesc(packet->Channel()), packet,
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
    icqFileTransferCancel(e->userId(), e->m_nSequence);
  else if (e->m_nSubCommand == ICQ_CMDxSUB_SECURExOPEN)
    icqOpenSecureChannelCancel(e->userId(), e->m_nSequence);

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

void CICQDaemon::updateAllUsersInGroup(int groupId)
{
  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->isInGroup(groupId))
    {
      icqRequestMetaInfo(pUser->IdString());
    }
  }
  FOR_EACH_USER_END
}

//-----ProcessMessage-----------------------------------------------------------
void CICQDaemon::ProcessMessage(ICQUser *u, CBuffer &packet, char *message,
    unsigned short nMsgType, unsigned long nMask, const unsigned long nMsgID[2],
                                unsigned short nSequence, bool bIsAck,
                                bool &bNewUser)
{
  char *szType = NULL;
  CUserEvent *pEvent = NULL;
  OnEventManager::OnEventType onEventType = OnEventManager::OnEventMessage;

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
      onEventType = OnEventManager::OnEventMessage;
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
        onEventType = OnEventManager::OnEventChat;
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
        onEventType = OnEventManager::OnEventFile;
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
      onEventType = OnEventManager::OnEventUrl;
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
      onEventType = OnEventManager::OnEventMessage;
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

        // Fetch the just added user and use it from here on
        u = gUserManager.fetchUser(u->id(), LOCK_W);
      }
      else
        gLog.Info(tr("%s%s from %s (%s).\n"), L_SRVxSTR, szType, u->GetAlias(),
            u->IdString());

      if (AddUserEvent(u, pEvent))
        gOnEventManager.performOnEvent(onEventType, u);
    }
    else // invalid parse or unknown event
    {
      gLog.warning(tr("Invalid %s, see packet log for details"), szType);
      packet.log(tr("Invalid %s:"), szType);
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
