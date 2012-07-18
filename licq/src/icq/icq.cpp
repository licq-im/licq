/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2012 Licq developers <licq-dev@googlegroups.com>
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

#include "icq.h"

#include <boost/foreach.hpp>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>

#include <licq/contactlist/group.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/inifile.h>
#include <licq/logging/log.h>
#include <licq/statistics.h>
#include <licq/oneventmanager.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/protocolsignal.h>
#include <licq/proxy.h>
#include <licq/translator.h>
#include <licq/userevents.h>
#include <licq/utility.h>

#include "../gettext.h"
#include "defines.h"
#include "oscarservice.h"
#include "owner.h"
#include "packet-srv.h"
#include "packet-tcp.h"
#include "socket.h"
#include "user.h"

using namespace std;
using namespace LicqIcq;
using Licq::Daemon;
using Licq::IcqPluginActive;
using Licq::IcqPluginBusy;
using Licq::IcqPluginInactive;
using Licq::Log;
using Licq::OnEventData;
using Licq::gDaemon;
using Licq::gLog;
using Licq::gOnEventManager;
using Licq::gPluginManager;
using Licq::gTranslator;


// list of plugins we currently support
const struct PluginList IcqProtocol::info_plugins[] =
{
  { "Picture"   , PLUGIN_PICTURE   , "Picture"                          },
  { "Phone Book", PLUGIN_PHONExBOOK, "Phone Book / Phone \"Follow Me\"" }
};

const struct PluginList IcqProtocol::status_plugins[] =
{
  {"Phone \"Follow Me\"", PLUGIN_FOLLOWxME, "Phone Book / Phone \"Follow Me\""},
  { "Shared Files Directory", PLUGIN_FILExSERVER, "Shared Files Directory" },
  { "ICQphone Status"       , PLUGIN_ICQxPHONE  , "ICQphone Status"        }
};


std::list <CReverseConnectToUserData *> IcqProtocol::m_lReverseConnect;
pthread_mutex_t IcqProtocol::mutex_reverseconnect = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t IcqProtocol::cond_reverseconnect_done = PTHREAD_COND_INITIALIZER;


CICQDaemon *gLicqDaemon = NULL;
LicqIcq::IcqProtocol LicqIcq::gIcqProtocol;
Licq::SocketManager gSocketManager;

IcqProtocol::IcqProtocol()
  : myDirectMode(true)
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

  myMaxUsersPerPacket = 100;

  // Proxy
  m_xProxy = NULL;

  // Services
  m_xBARTService = NULL;

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

  DcSocket* s = new DcSocket();
  m_nTCPSocketDesc = gDaemon.StartTCPServer(s);
  if (m_nTCPSocketDesc == -1)
  {
    gLog.error(tr("Unable to allocate TCP port for local server (No ports available)!"));
    return false;
  }
  gSocketManager.AddSocket(s);
  bool useBart = false;
  {
    OwnerWriteGuard o;
    if (o.isLocked())
    {
      o->SetIntIp(s->getLocalIpInt());
      o->SetPort(s->getLocalPort());
      useBart = o->useBart();
    }
  }
  CPacket::SetLocalPort(s->getLocalPort());
  gSocketManager.DropSocket(s);

  gLog.info(tr("Spawning daemon threads"));

  nResult = pthread_create(&thread_ping, NULL, &Ping_tep, this);
  if (nResult != 0)
  {
    gLog.error(tr("Unable to start ping thread: %s."), strerror(nResult));
    return false;
  }
  
  nResult = pthread_create(&thread_updateusers, NULL, &UpdateUsers_tep, this);
  if (nResult != 0)
  {
    gLog.error(tr("Unable to start users update thread: %s."), strerror(nResult));
    return false;
  }

  if (useBart)
  {
    m_xBARTService = new COscarService(ICQ_SNACxFAM_BART);
    nResult = pthread_create(&thread_ssbiservice, NULL,
                             &OscarServiceSendQueue_tep, m_xBARTService);
    if (nResult != 0)
    {
      gLog.error(tr("Unable to start BART service thread: %s."), strerror(nResult));
      return false;
    }
  }

  MonitorSockets_func();

  // Cancel the ping thread
  pthread_cancel(thread_ping);

  // Cancel the update users thread
  pthread_cancel(thread_updateusers);

  // Cancel the BART service thread
  if (m_xBARTService)
    pthread_cancel(thread_ssbiservice);

  if (m_nTCPSrvSocketDesc != -1 )
    icqLogoff();
  if (m_nTCPSocketDesc != -1)
    gSocketManager.CloseSocket(m_nTCPSocketDesc);

  return true;
}

void IcqProtocol::processSignal(Licq::ProtocolSignal* s)
{
  assert(s != NULL);
  switch (s->signal())
  {
    case Licq::ProtocolSignal::SignalLogon:
    {
      Licq::ProtoLogonSignal* sig = dynamic_cast<Licq::ProtoLogonSignal*>(s);
      logon(sig->status());
      break;
    }
    case Licq::ProtocolSignal::SignalLogoff:
      icqLogoff();
      break;
    case Licq::ProtocolSignal::SignalChangeStatus:
    {
      Licq::ProtoChangeStatusSignal* sig = dynamic_cast<Licq::ProtoChangeStatusSignal*>(s);
      setStatus(sig->status());
      break;
    }
    case Licq::ProtocolSignal::SignalAddUser:
      icqAddUser(s->userId(), false);
      break;
    case Licq::ProtocolSignal::SignalRemoveUser:
      icqRemoveUser(s->userId());
      Licq::gUserManager.removeLocalUser(s->userId());
      break;
    case Licq::ProtocolSignal::SignalRenameUser:
      icqRenameUser(s->userId());
      break;
    case Licq::ProtocolSignal::SignalChangeUserGroups:
      icqChangeGroup(s->userId());
      break;
    case Licq::ProtocolSignal::SignalSendMessage:
      icqSendMessage(dynamic_cast<Licq::ProtoSendMessageSignal*>(s));
      break;
    case Licq::ProtocolSignal::SignalNotifyTyping:
    {
      Licq::ProtoTypingNotificationSignal* sig = dynamic_cast<Licq::ProtoTypingNotificationSignal*>(s);
      icqTypingNotification(s->userId(), sig->active());
      break;
    }
    case Licq::ProtocolSignal::SignalGrantAuth:
      icqAuthorizeGrant(s);
      break;
    case Licq::ProtocolSignal::SignalRefuseAuth:
      icqAuthorizeRefuse(dynamic_cast<Licq::ProtoRefuseAuthSignal*>(s));
      break;
    case Licq::ProtocolSignal::SignalRequestInfo:
      icqRequestMetaInfo(s->userId(), s);
      break;
    case Licq::ProtocolSignal::SignalUpdateInfo:
      icqSetGeneralInfo(s);
      break;
    case Licq::ProtocolSignal::SignalRequestPicture:
      icqRequestPicture(s);
      break;
    case Licq::ProtocolSignal::SignalBlockUser:
      icqAddToInvisibleList(s->userId());
      break;
    case Licq::ProtocolSignal::SignalUnblockUser:
      icqRemoveFromInvisibleList(s->userId());
      break;
    case Licq::ProtocolSignal::SignalAcceptUser:
      icqAddToVisibleList(s->userId());
      break;
    case Licq::ProtocolSignal::SignalUnacceptUser:
      icqRemoveFromVisibleList(s->userId());
      break;
    case Licq::ProtocolSignal::SignalIgnoreUser:
      icqAddToIgnoreList(s->userId());
      break;
    case Licq::ProtocolSignal::SignalUnignoreUser:
      icqRemoveFromIgnoreList(s->userId());
      break;
    case Licq::ProtocolSignal::SignalSendFile:
      icqFileTransfer(dynamic_cast<Licq::ProtoSendFileSignal*>(s));
      break;
    case Licq::ProtocolSignal::SignalCancelEvent:
      CancelEvent(s->eventId());
      break;
    case Licq::ProtocolSignal::SignalSendReply:
    {
      Licq::ProtoSendEventReplySignal* sig = dynamic_cast<Licq::ProtoSendEventReplySignal*>(s);
      if (sig->accept())
        icqFileTransferAccept(sig);
      else
        icqFileTransferRefuse(sig);
      break;
    }
    case Licq::ProtocolSignal::SignalOpenSecure:
      icqOpenSecureChannel(s);
      break;
    case Licq::ProtocolSignal::SignalCloseSecure:
      icqCloseSecureChannel(s);
      break;
    case Licq::ProtocolSignal::SignalRequestAuth:
    {
      Licq::ProtoRequestAuthSignal* sig = dynamic_cast<Licq::ProtoRequestAuthSignal*>(s);
      icqRequestAuth(s->userId(), sig->message());
      break;
    }
    case Licq::ProtocolSignal::SignalRenameGroup:
      gIcqProtocol.icqRenameGroup(dynamic_cast<Licq::ProtoRenameGroupSignal*>(s));
      break;
    case Licq::ProtocolSignal::SignalRemoveGroup:
      gIcqProtocol.icqRemoveGroup(dynamic_cast<Licq::ProtoRemoveGroupSignal*>(s));
      break;
    case Licq::ProtocolSignal::SignalSendUrl:
      icqSendUrl(dynamic_cast<Licq::ProtoSendUrlSignal*>(s));
      break;
    default:
    {
      /* Unsupported action, if it has an eventId, cancel it */
      if (s->eventId() != 0)
        Licq::gPluginManager.pushPluginEvent(
            new Licq::Event(s, Licq::Event::ResultUnsupported));
      break;
    }
  }
}

bool IcqProtocol::directMode() const
{
  return (!gDaemon.behindFirewall() || (gDaemon.behindFirewall() && gDaemon.tcpEnabled()));
}

void IcqProtocol::InitProxy()
{
  if (m_xProxy != NULL)
  {
    delete m_xProxy;
    m_xProxy = NULL;
  }
  m_xProxy = gDaemon.createProxy();
}

unsigned short IcqProtocol::dcVersionToUse(unsigned short v_in)
{
  /*if (ICQ_VERSION_TCP & 4 && v & 4) return 4;
  if (ICQ_VERSION_TCP & 2 && v & 2) return 2;
  gLog.warning(tr("Unknown TCP version %d.  Attempting v2."), v);
  return 2;*/
  unsigned short v_out = v_in < ICQ_VERSION_TCP ? v_in : ICQ_VERSION_TCP;
  if (v_out < 2 || v_out == 5)
  {
    if (v_out == 5)
      v_out = 4;
    else
      v_out = 6;

    gLog.warning(tr("Invalid TCP version %d.  Attempting v%d."), v_in, v_out);
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
      gLog.error(tr("Unable to start BART service thread:%s."), strerror(nResult));
    }
  }

  OwnerWriteGuard o;
  o->setUseBart(b);
}

void IcqProtocol::ChangeUserStatus(User* u, unsigned long s, time_t onlineSince)
{
  //This is the v6 way of telling us phone follow me status
  if (s & ICQ_STATUS_FxPFM)
  {
    if (s & ICQ_STATUS_FxPFMxAVAILABLE)
      u->setPhoneFollowMeStatus(IcqPluginActive);
    else
      u->setPhoneFollowMeStatus(IcqPluginBusy);
  }
  else if (u->Version() < 7)
    u->setPhoneFollowMeStatus(IcqPluginInactive);

  u->setWebPresence(s & ICQ_STATUS_FxWEBxPRESENCE);
  u->setHideIp(s & ICQ_STATUS_FxHIDExIP);
  u->setBirthdayFlag(s & ICQ_STATUS_FxBIRTHDAY);
  u->setHomepageFlag(s & ICQ_STATUS_FxICQxHOMEPAGE);

  if (s & ICQ_STATUS_FxDIRECTxDISABLED)
    u->setDirectFlag(User::DirectDisabled);
  else if (s & ICQ_STATUS_FxDIRECTxLISTED)
    u->setDirectFlag(User::DirectListed);
  else if (s & ICQ_STATUS_FxDIRECTxAUTH)
    u->setDirectFlag(User::DirectAuth);
  else
    u->setDirectFlag(User::DirectAnyone);

  u->statusChanged(statusFromIcqStatus(s), onlineSince);
}

unsigned IcqProtocol::eventCommandFromPacket(Licq::Packet* p)
{
  if (p->SubCommand() == ICQ_CMDxSUB_MSG)
    return Licq::Event::CommandMessage;
  if (p->SubCommand() == ICQ_CMDxSUB_URL)
    return Licq::Event::CommandUrl;
  if (p->SubCommand() == ICQ_CMDxSUB_FILE)
    return Licq::Event::CommandFile;
  if (p->SubCommand() == ICQ_CMDxSUB_CHAT)
    return Licq::Event::CommandChatInvite;
  if (p->SubCommand() == ICQ_CMDxSUB_SECURExOPEN)
    return Licq::Event::CommandSecureOpen;
  return Licq::Event::CommandOther;
}

/*----------------------------------------------------------------------------
 * CICQDaemon::SendEvent
 *
 * Sends an event without expecting a reply.
 *--------------------------------------------------------------------------*/

void IcqProtocol::SendEvent_Server(CPacket *packet, const Licq::ProtocolSignal* ps)
{
#if 1
  Licq::Event* e;
  if (ps == NULL)
    e = new Licq::Event(m_nTCPSrvSocketDesc, packet, Licq::Event::ConnectServer);
  else
    e = new Licq::Event(ps->callerThread(), ps->eventId(), m_nTCPSrvSocketDesc, packet, Licq::Event::ConnectServer);
  e->myCommand = eventCommandFromPacket(packet);

  pthread_mutex_lock(&mutex_sendqueue_server);
  m_lxSendQueue_Server.push_back(e);
  pthread_mutex_unlock(&mutex_sendqueue_server);

  e->m_NoAck = true;
  int nResult = pthread_create(&e->thread_send, NULL, &ProcessRunningEvent_Server_tep, e);
  if (nResult != 0)
  {
    gLog.error(tr("Unable to start server event thread (#%hu): %s."),
        e->m_nSequence, strerror(nResult));
    e->m_eResult = Licq::Event::ResultError;
  }
#else
  SendEvent(m_nTCPSrvSocketDesc, *packet, true);
#endif
}

Licq::Event* IcqProtocol::SendExpectEvent_Server(const Licq::ProtocolSignal* ps,
    const Licq::UserId& userId, CSrvPacketTcp *packet, Licq::UserEvent *ue, bool bExtendedEvent)
{
  // If we are already shutting down, don't start any events
  if (gDaemon.shuttingDown())
  {
    if (packet != NULL) delete packet;
    if (ue != NULL) delete ue;
    return NULL;
  }

  Licq::Event* e;
  if (ps == NULL)
    e = new Licq::Event(m_nTCPSrvSocketDesc, packet, Licq::Event::ConnectServer, userId, ue);
  else
    e = new Licq::Event(ps->callerThread(), ps->eventId(), m_nTCPSrvSocketDesc, packet,
        Licq::Event::ConnectServer, userId, ue);
  e->myCommand = eventCommandFromPacket(packet);

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

Licq::Event* IcqProtocol::SendExpectEvent_Client(const Licq::ProtocolSignal* ps, const User* pUser,
    CPacketTcp* packet, Licq::UserEvent *ue)
{
  // If we are already shutting down, don't start any events
  if (gDaemon.shuttingDown())
  {
    if (packet != NULL) delete packet;
    if (ue != NULL) delete ue;
    return NULL;
  }

  Licq::Event* e;
  if (ps == NULL)
    e = new Licq::Event(pUser->socketDesc(packet->channel()), packet, Licq::Event::ConnectUser,
        pUser->id(), ue);
  else
    e = new Licq::Event(ps->callerThread(), ps->eventId(), pUser->socketDesc(packet->channel()),
        packet, Licq::Event::ConnectUser, pUser->id(), ue);
  e->myCommand = eventCommandFromPacket(packet);
  e->myFlags |= Licq::Event::FlagDirect;

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
    gLog.error(tr("Unable to start event thread (#%hu): %s."),
        e->m_nSequence, strerror(nResult));
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
  pSock->send(*buf);
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
      e->subResult != Licq::Event::SubResultReturn)
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
      Licq::gPluginManager.pushPluginEvent(e);
      break;

  case ICQ_CMDxSND_SETxSTATUS:
      if (e->m_eResult == Licq::Event::ResultAcked)
      {
        Licq::OwnerWriteGuard o(LICQ_PPID);
      ChangeUserStatus(o, ((CPU_SetStatus *)e->m_pPacket)->Status() );
      }
      Licq::gPluginManager.pushPluginEvent(e);
      break;

  case ICQ_CMDxSND_SETxRANDOMxCHAT:
      if (e->m_eResult == Licq::Event::ResultAcked)
      {
        Licq::OwnerWriteGuard o(LICQ_PPID);
      o->SetRandomChatGroup(((CPU_SetRandomChatGroup *)e->m_pPacket)->Group());
      }
      Licq::gPluginManager.pushPluginEvent(e);
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
          Licq::gPluginManager.pushPluginEvent(e);
          break;
        case Licq::Event::ResultAcked:  // push to extended event list
        PushExtendedEvent(e);
        break;
        default:
          gLog.error(tr("Internal error: ProcessDoneEvents(): Invalid result for extended event (%d)."),
              e->m_eResult);
          delete e;
          return;
    }
    break;
  }

    default:
      gLog.error(tr("Internal error: ProcessDoneEvents(): Unknown command (%04X)."),
          e->m_nCommand);
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
    gLog.warning(tr("Cancelled event not found."));
    return;
  }

  CancelEvent((eRun != NULL)? eRun : (eExt != NULL)? eExt : eSrv);
}

void IcqProtocol::CancelEvent(Licq::Event* e)
{
  e->m_eResult = Licq::Event::ResultCancelled;

  if (e->command() == Licq::Event::CommandChatInvite)
    icqChatRequestCancel(e->userId(), e->m_nSequence);
  else if (e->command() == Licq::Event::CommandFile)
    icqFileTransferCancel(e->userId(), e->m_nSequence);
  else if (e->command() == Licq::Event::CommandSecureOpen)
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
void IcqProtocol::ProcessMessage(Licq::User *u, CBuffer &packet, const string& message,
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

      // Check if message is marked as UTF8
      unsigned long guidlen;
      packet >> guidlen;
      bool isUtf8 = false;
      while (guidlen >= 38)
      {
        string guid = packet.unpackRawString(38);
        if (guid == ICQ_CAPABILITY_UTF8_STR)
          isUtf8 = true;
        guidlen -= 38;
      }

      Licq::EventMsg* e = new Licq::EventMsg(
          (isUtf8 ? message : Licq::gTranslator.fromUtf8(message, u->userEncoding())),
          Licq::EventMsg::TimeNow, nFlags);
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
      string chatClients = packet.unpackShortStringLE();
    unsigned short nPortReversed;
    nPortReversed = packet.UnpackUnsignedShortBE();
    packet.incDataPosRead(2);
    nPort = packet.UnpackUnsignedShort();

    if (nPort == 0)
      nPort = nPortReversed;

    if (!bIsAck)
    {
        Licq::EventChat* e = new Licq::EventChat(message, chatClients, nPort,
            nSequence, Licq::EventChat::TimeNow, nFlags, 0, nMsgID[0], nMsgID[1]);
        onEventType = OnEventData::OnEventChat;
      pEvent = e;
    }

    szType = strdup(tr("Chat request"));
    break;
  }

  case ICQ_CMDxSUB_FILE:
  {
    unsigned long nFileSize;

    // Port reversed: garbage when the request is refused
    packet.UnpackUnsignedShortBE();

    packet.UnpackUnsignedShort();

      string filename = packet.unpackLongStringLE();
      packet >> nFileSize;
    if (!bIsAck)
    {
        list<string> filelist;
        filelist.push_back(filename);

        Licq::EventFile* e = new Licq::EventFile(filename,
            Licq::gTranslator.fromUtf8(message, u->userEncoding()), nFileSize,
            filelist, nSequence, Licq::EventFile::TimeNow, nFlags, 0, nMsgID[0], nMsgID[1]);
        onEventType = OnEventData::OnEventFile;
      pEvent = e;
    }

    packet >> nPort;

    szType = strdup(tr("File transfer request through server"));
    break;
  }

  case ICQ_CMDxSUB_URL:
    {
      pEvent = parseUrlEvent(message, Licq::EventUrl::TimeNow, nFlags, u->userEncoding());
    CPU_AckGeneral *p = new CPU_AckGeneral(u, nMsgID[0], nMsgID[1],
                                           nSequence, ICQ_CMDxSUB_URL, true,
                                           nLevel);
    SendEvent_Server(p);

    szType = strdup(tr("URL"));
      onEventType = OnEventData::OnEventUrl;
      break;
    }
  case ICQ_CMDxSUB_CONTACTxLIST:
    {
      pEvent = parseContactEvent(message, Licq::EventContactList::TimeNow,
          nFlags, u->userEncoding());
    CPU_AckGeneral *p = new CPU_AckGeneral(u, nMsgID[0], nMsgID[1],
                                           nSequence, ICQ_CMDxSUB_CONTACTxLIST,
                                           true, nLevel);
    SendEvent_Server(p);

    szType = strdup(tr("Contact list"));
      onEventType = OnEventData::OnEventMessage;
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
        string msgUtf8 = gTranslator.toUtf8(message, u->userEncoding());
        if (u->autoResponse() != msgUtf8)
        {
          u->setAutoResponse(msgUtf8);
          u->SetShowAwayMsg(!msgUtf8.empty());
          gLog.info(tr("Auto response from %s (#%lu)."), u->getAlias().c_str(), nMsgID[1]);
        }
        Licq::Event* e = DoneServerEvent(nMsgID[1], Licq::Event::ResultAcked);
        if (e)
        {
          e->m_pExtendedAck = new Licq::ExtendedData(true, 0, msgUtf8);
          e->mySubResult = Licq::Event::SubResultReturn;
        ProcessDoneEvent(e);
      }
        else
          gLog.warning(tr("Ack for unknown event."));
      }
      else
      {
        gLog.info(tr("%s (%s) requested auto response."),
            u->getAlias().c_str(), u->accountId().c_str());

    CPU_AckGeneral *p = new CPU_AckGeneral(u, nMsgID[0], nMsgID[1],
                                           nSequence, nMsgType, true, nLevel);
    SendEvent_Server(p);

        Licq::gStatistics.increase(Licq::Statistics::AutoResponseCheckedCounter);
    u->SetLastCheckedAutoResponse();

        Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
            Licq::PluginSignal::SignalUser,
            Licq::PluginSignal::UserEvents, u->id()));
      }
    return;
    
    break; // bah!
  }

  case ICQ_CMDxSUB_ICBM:
  {
    unsigned short nLen;

    packet >> nLen;
    packet.incDataPosRead(18);
      string plugin = packet.unpackLongStringLE();

      packet.incDataPosRead(nLen - 22 - plugin.size()); // unknown
      packet.UnpackUnsignedLong();

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
        gLog.warning(tr("Unknown ICBM plugin type: %s"), plugin.c_str());
        return;
      }

      string msg2 = packet.unpackLongStringLE();

    /* if the auto response is non empty then this is a decline and we want
       to show the auto response rather than our original message */

    // recursion
      ProcessMessage(u, packet, (message.empty() ? msg2 : message), nCommand,
          nMask, nMsgID, nSequence, bIsAck, bNewUser);
      return;
    }

  default:
    szType = strdup(tr("unknown event"));
  } // switch nMsgType

  if (bIsAck)
  {
    Licq::Event* pAckEvent = DoneServerEvent(nMsgID[1], Licq::Event::ResultAcked);
    Licq::ExtendedData* pExtendedAck = new Licq::ExtendedData(true, nPort,
        gTranslator.toUtf8(message, u->userEncoding()));

    if (pAckEvent)
    {
      pAckEvent->m_pExtendedAck = pExtendedAck;
      pAckEvent->mySubResult = Licq::Event::SubResultAccept;
      gLog.info(tr("%s accepted from %s (%s)."), szType,
          u->getAlias().c_str(), u->accountId().c_str());
      u->unlockWrite();
      ProcessDoneEvent(pAckEvent);
      u->lockWrite();
    }
    else
    {
      gLog.warning(tr("Ack for unknown event."));
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
          gLog.info(tr("%s from new user (%s), ignoring."),
              szType, u->accountId().c_str());
          if (szType)  free(szType);
          gDaemon.rejectEvent(u->id(), pEvent);
          return;
        }
        gLog.info(tr("%s from new user (%s)."), szType, u->accountId().c_str());

        // Don't delete user when we're done
        bNewUser = false;
      }
      else
        gLog.info(tr("%s from %s (%s)."), szType, u->getAlias().c_str(),
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

void IcqProtocol::processServerMessage(int type, Licq::Buffer &packet,
    const Licq::UserId& userId, string& message, time_t timeSent,
    unsigned long flags)
{
  if (type & ICQ_CMDxSUB_FxMULTIREC)
  {
    flags |= Licq::UserEvent::FlagMultiRec;
    type &= ~ICQ_CMDxSUB_FxMULTIREC;
  }

  // Drop trailing nul characters from message
  while (message.size() > 0 && message[message.size()-1] == '\0')
    message.resize(message.size()-1);

  string userEncoding = getUserEncoding(userId);

  OnEventData::OnEventType onEventType = OnEventData::OnEventMessage;
  Licq::UserEvent* ue;

  switch (type)
  {
    case ICQ_CMDxSUB_MSG:
    {
      onEventType = OnEventData::OnEventMessage;
      ue = new Licq::EventMsg(
          gTranslator.toUtf8(gTranslator.returnToUnix(message), userEncoding),
          timeSent, flags);
      break;
    }
    case ICQ_CMDxSUB_URL:
    {
      onEventType = OnEventData::OnEventUrl;
      ue = parseUrlEvent(message, timeSent, flags, userEncoding);
      if (ue == NULL)
      {
        packet.log(Log::Warning, tr("Invalid URL message"));
        return;
      }
      break;
    }
    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      gLog.info(tr("Authorization request from %s"), userId.toString().c_str());

      vector<string> parts; // alias, first name, last name, email, auth, comment
      splitFE(parts, message, 6, userEncoding);
      if (parts.size() != 6)
      {
        packet.log(Log::Warning, tr("Invalid authorization request system message"));
        return;
      }

      ue = new Licq::EventAuthRequest(userId, parts.at(0), parts.at(1), parts.at(2),
          parts.at(3), gTranslator.returnToUnix(parts.at(5)), timeSent, flags);
      break;
    }
    case ICQ_CMDxSUB_AUTHxREFUSED:  // system message: authorization refused
    {
      gLog.info(tr("Authorization refused by %s"), userId.toString().c_str());
      ue = new Licq::EventAuthRefused(userId,
          gTranslator.returnToUnix(gTranslator.toUtf8(message, userEncoding)),
          timeSent, flags);
      break;
    }
    case ICQ_CMDxSUB_AUTHxGRANTED:  // system message: authorized
    {
      gLog.info(tr("Authorization granted by %s"), userId.toString().c_str());

      {
        Licq::UserWriteGuard u(userId);
        if (u.isLocked())
          u->SetAwaitingAuth(false);
      }

      ue = new Licq::EventAuthGranted(userId,
          gTranslator.returnToUnix(gTranslator.toUtf8(message, userEncoding)),
          timeSent, flags);
      break;
    }
    case ICQ_CMDxSUB_MSGxSERVER:
    {
      gLog.info(tr("Server message."));

      vector<string> parts;
      splitFE(parts, message, 6, userEncoding);
      if (parts.size() != 6)
      {
        packet.log(Log::Warning, tr("Invalid Server Message"));
        return;
      }

      ue = new Licq::EventServerMessage(parts.at(0), parts.at(3),
          gTranslator.returnToUnix(parts.at(5)), timeSent);
      break;
    }
    case ICQ_CMDxSUB_ADDEDxTOxLIST:  // system message: added to a contact list
    {
      gLog.info(tr("User %s added you to their contact list"), userId.toString().c_str());

      vector<string> parts; // alias, first name, last name, email, auth, comment
      splitFE(parts, message, 6, userEncoding);
      if (parts.size() != 6)
      {
        packet.log(Log::Warning, tr("Invalid added to list system message"));
        return;
      }

      ue = new Licq::EventAdded(userId, parts.at(0), parts.at(1), parts.at(2),
          parts.at(3), timeSent, flags);
      break;
    }
    case ICQ_CMDxSUB_WEBxPANEL:
    {
      gLog.info(tr("Message through web panel"));

      vector<string> parts; // name, ?, ?, email, ?, message
      splitFE(parts, message, 6, userEncoding);
      if (parts.size() != 6)
      {
        packet.log(Log::Warning, tr("Invalid web panel system message"));
        return;
      }

      gLog.info(tr("From %s (%s)"), parts.at(0).c_str(), parts.at(3).c_str());
      ue = new Licq::EventWebPanel(parts.at(0), parts.at(3),
          gTranslator.returnToUnix(parts.at(5)), timeSent, flags);
      break;
    }
    case ICQ_CMDxSUB_EMAILxPAGER:
    {
      gLog.info(tr("Email pager message"));

      vector<string> parts; // name, ?, ?, email, ?, message
      splitFE(parts, message, 6, userEncoding);
      if (parts.size() != 6)
      {
        packet.log(Log::Warning, tr("Invalid email pager system message"));
        return;
      }

      gLog.info(tr("From %s (%s)"), parts.at(0).c_str(), parts.at(3).c_str());
      ue = new Licq::EventEmailPager(parts.at(0), parts.at(3),
          gTranslator.returnToUnix(parts.at(5)), timeSent, flags);
      break;
    }
    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      onEventType = OnEventData::OnEventMessage;
      ue = parseContactEvent(message, timeSent, flags, userEncoding);
      if (ue == NULL)
      {
        packet.log(Log::Warning, tr("Invalid Contact List message"));
        return;
      }
      break;
    }
    case ICQ_CMDxSUB_SMS:
    {
      string xmlSms = getXmlTag(message, "sms_message");
      if (xmlSms.empty())
      {
        packet.log(Log::Warning, tr("Invalid SMS message"));
        return;
      }

      string number = getXmlTag(xmlSms, "sender");
      string msg = getXmlTag(xmlSms, "text");

      ue = new Licq::EventSms(number, msg, timeSent, flags);
      break;
    }
    default:
    {
      size_t pos = 0;
      while ((pos = message.find('\xFE')) != string::npos)
        message[pos] = '\n';

      packet.log(Log::Unknown, "Unknown system message (0x%04x)", type);
      ue = new Licq::EventUnknownSysMsg(type,
            ICQ_CMDxRCV_SYSxMSGxOFFLINE, userId, message, timeSent, 0);
      break;
    }
  }

  switch (type)
  {
    case ICQ_CMDxSUB_MSG:
    case ICQ_CMDxSUB_URL:
    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      // Get the user and allow adding unless we ignore new users
      Licq::UserWriteGuard u(userId, !gDaemon.ignoreType(Daemon::IgnoreNewUsers));
      if (!u.isLocked())
      {
        gLog.info(tr("%s from new user (%s), ignoring"),
            ue->description().c_str(), userId.toString().c_str());
        gDaemon.rejectEvent(userId, ue);
        break;
      }
      else
        gLog.info(tr("%s through server from %s (%s)"),
            ue->description().c_str(), u->getAlias().c_str(), userId.toString().c_str());

      u->setIsTyping(false);

      if (gDaemon.addUserEvent(*u, ue))
        gOnEventManager.performOnEvent(onEventType, *u);

      gPluginManager.pushPluginSignal(new Licq::PluginSignal(
          Licq::PluginSignal::SignalUser, Licq::PluginSignal::UserTyping, u->id()));

      break;
    }
    case ICQ_CMDxSUB_AUTHxREQUEST:
    case ICQ_CMDxSUB_AUTHxREFUSED:
    case ICQ_CMDxSUB_AUTHxGRANTED:
    case ICQ_CMDxSUB_MSGxSERVER:
    case ICQ_CMDxSUB_ADDEDxTOxLIST:
    case ICQ_CMDxSUB_WEBxPANEL:
    case ICQ_CMDxSUB_EMAILxPAGER:
    {
      bool bIgnore;
      {
        Licq::UserReadGuard u(userId);
        bIgnore = (u.isLocked() && u->IgnoreList());
      }

      if (bIgnore)
      {
        delete ue; // Processing stops here, needs to be deleted
        gLog.info("Ignored!");
        break;
      }

      Licq::OwnerWriteGuard o(LICQ_PPID);
      if (gDaemon.addUserEvent(*o, ue))
      {
        ue->AddToHistory(*o, true);
        gOnEventManager.performOnEvent(OnEventData::OnEventSysMsg, *o);
      }
      break;
    }

    case ICQ_CMDxSUB_SMS:
    {
      Licq::EventSms* eSms = dynamic_cast<Licq::EventSms*>(ue);
      string idSms = findUserByCellular(eSms->number());

      if (!idSms.empty())
      {
        Licq::UserWriteGuard u(Licq::UserId(idSms.c_str(), LICQ_PPID));
        gLog.info(tr("SMS from %s - %s (%s)"), eSms->number().c_str(),
            u->getAlias().c_str(), idSms.c_str());
        if (gDaemon.addUserEvent(*u, ue))
          gOnEventManager.performOnEvent(OnEventData::OnEventSms, *u);
      }
      else
      {
        Licq::OwnerWriteGuard o(LICQ_PPID);
        gLog.info(tr("SMS from %s."), eSms->number().c_str());
        if (gDaemon.addUserEvent(*o, ue))
        {
          ue->AddToHistory(*o, true);
          gOnEventManager.performOnEvent(OnEventData::OnEventSms, *o);
        }
      }
      break;
    }
    default:
    {
      Licq::OwnerWriteGuard o(LICQ_PPID);
      gDaemon.addUserEvent(*o, ue);
    }
  }
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
    gLog.warning(tr("Failed to find desired connection record."));
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
        gLog.warning(tr("Somebody else removed our connection record."));
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

string IcqProtocol::getXmlTag(const string& xmlSource, const string& tagName)
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

unsigned short IcqProtocol::icqStatusFromStatus(unsigned status)
{
  if (status == Licq::User::OfflineStatus)
    return ICQ_STATUS_OFFLINE;

  unsigned short icqStatus;
  if (status & Licq::User::DoNotDisturbStatus)
    icqStatus = ICQ_STATUS_DND | ICQ_STATUS_AWAY | ICQ_STATUS_OCCUPIED;
  else if (status & Licq::User::OccupiedStatus)
    icqStatus = ICQ_STATUS_OCCUPIED | ICQ_STATUS_AWAY;
  else if (status & Licq::User::NotAvailableStatus)
    icqStatus = ICQ_STATUS_NA | ICQ_STATUS_AWAY;
  else if (status & Licq::User::AwayStatus)
    icqStatus = ICQ_STATUS_AWAY;
  else if (status & Licq::User::FreeForChatStatus)
    icqStatus = ICQ_STATUS_FREEFORCHAT;
  else
    icqStatus = ICQ_STATUS_ONLINE;

  if (status & Licq::User::InvisibleStatus)
    icqStatus |= ICQ_STATUS_FxPRIVATE;

  return icqStatus;
}

unsigned IcqProtocol::statusFromIcqStatus(unsigned short icqStatus)
{
  // Build status from ICQ flags
  if (icqStatus == ICQ_STATUS_OFFLINE)
    return Licq::User::OfflineStatus;

  unsigned status = Licq::User::OnlineStatus;
  if (icqStatus & ICQ_STATUS_FxPRIVATE)
    status |= Licq::User::InvisibleStatus;
  if (icqStatus & ICQ_STATUS_DND)
    status |= Licq::User::DoNotDisturbStatus;
  else if (icqStatus & ICQ_STATUS_OCCUPIED)
    status |= Licq::User::OccupiedStatus;
  else if (icqStatus & ICQ_STATUS_NA)
    status |= Licq::User::NotAvailableStatus;
  else if (icqStatus & ICQ_STATUS_AWAY)
    status |= Licq::User::AwayStatus;
  if (icqStatus & ICQ_STATUS_FREEFORCHAT)
    status |= Licq::User::FreeForChatStatus;

  return status;
}

unsigned long IcqProtocol::addStatusFlags(unsigned long s, const User* u)
{
  s &= 0x0000FFFF;

  if (u->webPresence())
    s |= ICQ_STATUS_FxWEBxPRESENCE;
  if (u->hideIp())
    s |= ICQ_STATUS_FxHIDExIP;
  if (u->birthdayFlag())
    s |= ICQ_STATUS_FxBIRTHDAY;
  if (u->homepageFlag())
    s |= ICQ_STATUS_FxICQxHOMEPAGE;

  if (u->phoneFollowMeStatus() != IcqPluginInactive)
    s |= ICQ_STATUS_FxPFM;
  if (u->phoneFollowMeStatus() == IcqPluginActive)
    s |= ICQ_STATUS_FxPFMxAVAILABLE;

  switch (u->directFlag())
  {
    case User::DirectDisabled:
      s |= ICQ_STATUS_FxDIRECTxDISABLED;
      break;
    case User::DirectListed:
      s |= ICQ_STATUS_FxDIRECTxLISTED;
      break;
    case User::DirectAuth:
      s |= ICQ_STATUS_FxDIRECTxAUTH;
      break;
  }

  return s;
}

string IcqProtocol::getUserEncoding(const Licq::UserId& userId)
{
  Licq::UserReadGuard u(userId);
  if (u.isLocked())
    return u->userEncoding();
  else
    return Licq::gUserManager.defaultUserEncoding();
}

bool IcqProtocol::UseServerContactList() const
{
  OwnerReadGuard o;
  return o->useServerContactList();
}

int IcqProtocol::getGroupFromId(unsigned short gsid)
{
  return Licq::gUserManager.getGroupFromServerId(LICQ_PPID, gsid);
}

unsigned long IcqProtocol::icqOwnerUin()
{
  return strtoul(Licq::gUserManager.ownerUserId(LICQ_PPID).accountId().c_str(), (char**)NULL, 10);
}

unsigned short IcqProtocol::generateSid()
{
  unsigned short ownerPDINFO;
  {
    OwnerReadGuard o;
    ownerPDINFO = o->GetPDINFO();
  }

  // Generate a SID
  srand(time(NULL));
  int sid = 1+(int)(65535.0*rand()/(RAND_MAX+1.0));

  sid &= 0x7FFF; // server limit it looks like

  // Make sure we have a unique number - a map would be better
  bool done;
  do
  {
    done = true;
    bool checkGroup = true;

    if (sid == 0)
      ++sid;
    if (sid == ownerPDINFO)
      sid++;

    {
      Licq::UserListGuard userList(LICQ_PPID);
      BOOST_FOREACH(const Licq::User* user, **userList)
      {
        UserReadGuard u(dynamic_cast<const User*>(user));

        if (u->GetSID() == sid || u->GetInvisibleSID() == sid ||
          u->GetVisibleSID() == sid)
        {
          if (sid == 0x7FFF)
            sid = 1;
          else
            ++sid;
          done = false;	// Restart
          checkGroup = false;	// Don't waste time now
          break;
        }
      }
    }

    if (checkGroup)
    {
      // Check our groups too!
      Licq::GroupListGuard groupList;
      BOOST_FOREACH(const Licq::Group* group, **groupList)
      {
        Licq::GroupReadGuard g(group);

        unsigned short icqGroupId = g->serverId(LICQ_PPID);
        if (icqGroupId == sid)
        {
          if (sid == 0x7FFF)
            sid = 1;
          else
            ++sid;
          done = false;
          break;
        }
      }
    }

  } while (!done);

  return sid;
}

void IcqProtocol::splitFE(vector<string>& ret, const string& s, int maxcount,
    const string& userEncoding)
{
  size_t pos = 0;
  while (maxcount == 0 || maxcount > 1)
  {
    size_t pos2 = s.find('\xFE', pos);
    if (pos2 == string::npos)
      break;

    ret.push_back(gTranslator.toUtf8(s.substr(pos, pos2-pos), userEncoding));
    if (maxcount > 0)
      maxcount--;
    pos = pos2 + 1;
  }

  ret.push_back(gTranslator.toUtf8(s.substr(pos), userEncoding));
}

Licq::EventUrl* IcqProtocol::parseUrlEvent(const string& s, time_t timeSent,
    unsigned long flags, const string& userEncoding)
{
  vector<string> parts;
  splitFE(parts, s, 2, userEncoding);
  if (parts.size() < 2)
    return NULL;

  // Part 0 is URL, part 1 is description
  return new Licq::EventUrl(gTranslator.returnToUnix(parts.at(1)),
      parts.at(0), timeSent, flags);
}

Licq::EventContactList* IcqProtocol::parseContactEvent(const string& s,
    time_t timeSent, unsigned long flags, const string& userEncoding)
{
  vector<string> parts;
  splitFE(parts, s, 0, userEncoding);

  // First part is number of contacts in the list
  size_t count = atoi(parts.at(0).c_str());
  if (parts.size() < count*2+2)
    return NULL;

  Licq::EventContactList::ContactList vc;
  for (size_t i = 0; i < count; ++i)
  {
    Licq::UserId userId(parts.at(i*2+1), LICQ_PPID);
    vc.push_back(new Licq::EventContactList::Contact(userId, parts.at(i*2+2)));
  }

  return new Licq::EventContactList(vc, false, timeSent, flags);
}

string IcqProtocol::pipeInput(const string& message)
{
  string m(message);
  size_t posPipe = 0;

  while (true)
  {
    posPipe = m.find('|', posPipe);
    if (posPipe == string::npos)
      break;

    if (posPipe != 0 && m[posPipe-1] != '\n')
    {
      // Pipe char isn't at begining of a line, ignore it
      ++posPipe;
      continue;
    }

    // Find end of command
    size_t posEnd = m.find('\r', posPipe+1);
    if (posEnd == string::npos)
      posEnd = m.size();
    size_t cmdLen = posEnd - posPipe - 2;

    string cmd(m, posPipe+1, cmdLen);
    string cmdOutput;
    Licq::UtilityInternalWindow win;
    if (!win.POpen(cmd))
    {
      gLog.warning(tr("Could not execute \"%s\" for auto-response."), cmd.c_str());
    }
    else
    {
      int c;
      while ((c = fgetc(win.StdOut())) != EOF)
      {
        if (c == '\n')
          cmdOutput += '\r';
        cmdOutput += c;
      }

      int i;
      if ((i = win.PClose()) != 0)
      {
        gLog.warning(tr("%s returned abnormally: exit code %d."), cmd.c_str(), i);
        // do anything to cmdOutput ???
      }
    }

    m.replace(posPipe, cmdLen + 1, cmdOutput);
    posPipe += cmdOutput.size() + 1;
  }

  return m;
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

CUserProperties::CUserProperties()
  : normalSid(0),
    groupId(0),
    visibleSid(0),
    invisibleSid(0),
    inIgnoreList(false),
    awaitingAuth(false)
{
  tlvs.clear();
}
