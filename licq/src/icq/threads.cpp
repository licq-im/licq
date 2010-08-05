// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998-2010 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#include "icq.h"

#include <boost/foreach.hpp>
#include <cerrno>
#include <ctime>
#include <unistd.h>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/event.h>
#include <licq/icqdefines.h>
#include <licq/packet.h>
#include <licq/pluginsignal.h>
#include <licq/socket.h>
#include <licq/logging/log.h>

#include "../daemon.h"
#include "../fifo.h"
#include "../gettext.h"
#include "../licq.h"
#include "../statistics.h"
#include "oscarservice.h"

#define MAX_CONNECTS  256
#define DEBUG_THREADS(x)
//#define DEBUG_THREADS(x) gLog.info(x)

using namespace std;
using Licq::gLog;

void cleanup_mutex(void *m)
{
  pthread_mutex_unlock((pthread_mutex_t *)m);
}

void cleanup_socket(void *s)
{
  gSocketManager.DropSocket((Licq::INetSocket *)s);
}

void *cleanup_thread_tep(void *t)
{
  pthread_detach(pthread_self());
  void *s;
  pthread_join(*((pthread_t *)t), &s);
  delete (pthread_t *)t;
  delete (int *)s;
  pthread_exit(NULL);
}

void cleanup_thread(void *t)
{
  pthread_t cleanup;
  pthread_create(&cleanup, NULL, &cleanup_thread_tep, t);
}

void *ConnectToServer_tep(void *s)
{
  *((int *)s) = gIcqProtocol.ConnectToLoginServer();
  pthread_exit(s);
}


/*------------------------------------------------------------------------------
 * ProcessRunningEvent_tep
 *
 * Thread entry point to run an event.  First checks to see if the socket for
 * the given event needs to be connected and calls the relevant connection
 * function.  Then sends the event, retrying after a timeout.  If an ack is
 * received, the thread will be cancelled by the receiving thread.
 * 
 * The parameter is only used to get the CICQDaemon, the actual event is
 * now popped off the send queue to prevent packets being sent out of order
 * which is a severe error with OSCAR.
 *----------------------------------------------------------------------------*/
void *ProcessRunningEvent_Server_tep(void* /* p */)
{
  pthread_detach(pthread_self());

  static unsigned short nNext = 0;
  static pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;

  /* want to be cancelled immediately so we don't try to derefrence the event
     after it has been deleted */
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  DEBUG_THREADS("[ProcessRunningEvent_Server_tep] Caught event.\n");

  CICQDaemon *d = gLicqDaemon;

  if (!d) pthread_exit(NULL);

  // Must send packets in sequential order
  pthread_mutex_lock(&send_mutex);
  pthread_mutex_lock(&gIcqProtocol.mutex_sendqueue_server);

  list<Licq::Event*>::iterator iter;
  Licq::Event* e = NULL;

  while (e == NULL)
  {

    for (iter = gIcqProtocol.m_lxSendQueue_Server.begin();
         iter != gIcqProtocol.m_lxSendQueue_Server.end(); ++iter)
    {
      if ((*iter)->Channel() == ICQ_CHNxNEW)
      {
        e = *iter;
        nNext = e->Sequence() + 1;
        break;
      }

      if ((*iter)->Sequence() == nNext)
      {
        e = *iter;
        nNext++;
        break;
      }
    }

    if (e == NULL)
    {
      bool bEmpty = gIcqProtocol.m_lxSendQueue_Server.empty();

      pthread_mutex_unlock(&gIcqProtocol.mutex_sendqueue_server);
      pthread_mutex_unlock(&send_mutex);

      if (bEmpty)
        pthread_exit(NULL);

      struct timeval tv = { 1, 0 };
      select(0, NULL, NULL, NULL, &tv);
      pthread_mutex_lock(&send_mutex);
      pthread_mutex_lock(&gIcqProtocol.mutex_sendqueue_server);
    }
    else
    {
      gIcqProtocol.m_lxSendQueue_Server.erase(iter);

      if (e->m_bCancelled)
      {
        delete e;
        e = NULL;
      }
    }
  }

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  e->thread_send = pthread_self();
  e->thread_running = true;

  // Done reading the queue now
  pthread_mutex_unlock(&gIcqProtocol.mutex_sendqueue_server);

  // declared here because pthread_cleanup_push starts a new block
  Licq::Buffer* buf;
  bool sent = false;
  bool bExit = false;
  string errorStr;

  pthread_cleanup_push(cleanup_mutex, &send_mutex);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();

    int socket = -1;
    unsigned short nSequence;
  Licq::INetSocket *s;

    // Check if the socket is connected
    if (e->m_nSocketDesc == -1)
    {
      // Connect to the server if we are logging on
      if (e->m_pPacket->Channel() == ICQ_CHNxNEW)
      {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        gLog.info(tr("%sConnecting to login server.\n"), L_SRVxSTR);

        pthread_t *t = new pthread_t;
        int *s = new int;
        pthread_create(t, NULL, ConnectToServer_tep, s);
        pthread_cleanup_push(cleanup_thread, t);
          pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
          pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
          pthread_testcancel();
          pthread_join(*t, NULL);
        pthread_cleanup_pop(0);
        int socket = *s;
        delete t;
        delete s;

        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
        pthread_testcancel();

        e->m_nSocketDesc = socket;

        // Check again, if still -1, fail the event
        if (e->m_nSocketDesc == -1)
        {
          pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
          gLog.info(tr("%sConnecting to login server failed, failing event\n"),
                    L_SRVxSTR);
          // we need to initialize the logon time for the next retry
          gIcqProtocol.m_tLogonTime = time(NULL);
          gIcqProtocol.m_eStatus = STATUS_OFFLINE_FORCED;
          gIcqProtocol.m_bLoggingOn = false;
        if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultError) != NULL)
        {
          gIcqProtocol.DoneExtendedEvent(e, Licq::Event::ResultError);
          gIcqProtocol.ProcessDoneEvent(e);
        }
        else
          {
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
            pthread_testcancel();
            delete e;
          }
          bExit = true;
          goto exit_server_thread;
        }
      }
      else
      {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        gLog.info(tr("%sNot connected to server, failing event\n"), L_SRVxSTR);
      if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultError) != NULL)
      {
        gIcqProtocol.DoneExtendedEvent(e, Licq::Event::ResultError);
        gIcqProtocol.ProcessDoneEvent(e);
      }
      else
        {
          pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
          pthread_testcancel();
          delete e;
        }
        bExit = true;
        goto exit_server_thread;
      }
    }

    socket = e->m_nSocketDesc;
    nSequence = e->m_nSequence;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    // Start sending the event
    s = gSocketManager.FetchSocket(socket);
    if (s == NULL)
    {
      gLog.warning(tr("%sSocket not connected or invalid (#%hu).\n"), L_WARNxSTR,
                nSequence);
    if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultError) != NULL)
    {
      gIcqProtocol.DoneExtendedEvent(e, Licq::Event::ResultError);
      gIcqProtocol.ProcessDoneEvent(e);
    }
    else
      {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        delete e;
      }
      bExit = true;
      goto exit_server_thread;
    }

    pthread_cleanup_push(cleanup_socket, s);

    pthread_mutex_lock(&gIcqProtocol.mutex_cancelthread);

      // check to make sure we were not cancelled already
    pthread_cleanup_push(cleanup_mutex, &gIcqProtocol.mutex_cancelthread);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        //if we get here  then we haven't been cancelled and we won't be
        //as long as we hold mutex_cancelthread

        buf = e->m_pPacket->Finalize(NULL);

    pthread_mutex_unlock(&gIcqProtocol.mutex_cancelthread);
      pthread_cleanup_pop(0); //mutex_cancelthread

      sent = s->Send(buf);
      delete buf;

      if (!sent)
        errorStr = s->errorStr();

      // We don't close the socket as it should be closed by the server thread
      gSocketManager.DropSocket(s);
    pthread_cleanup_pop(0); //socket

exit_server_thread:
    pthread_mutex_unlock(&send_mutex);
  pthread_cleanup_pop(0); //send_mutex

  if (bExit)
    pthread_exit(NULL);
    
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_testcancel();

  if (!sent)
  {
    unsigned short nSequence = e->m_nSequence;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    gLog.warning(tr("%sError sending event (#%hu):\n%s%s.\n"), L_WARNxSTR,
        nSequence, L_BLANKxSTR, errorStr.c_str());

    if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultError) != NULL)
    {
      gIcqProtocol.DoneExtendedEvent(e, Licq::Event::ResultError);
      gIcqProtocol.ProcessDoneEvent(e);
    }
    else
    {
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      delete e;
    }
  }
  else
  {
    if (e->m_NoAck)
    {
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
      // send successfully and we don't get an answer from the server
      if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultAcked) != NULL)
      {
        gIcqProtocol.DoneExtendedEvent(e, Licq::Event::ResultAcked);
        gIcqProtocol.ProcessDoneEvent(e);
      }
      else
      {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        delete e;
      }
    }
    else
    {
      e->thread_running = false;
      // pthread_exit is not async cancel safe???
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    }
  }

  pthread_exit(NULL);

  return NULL;
}


void *ProcessRunningEvent_Client_tep(void *p)
{
  pthread_detach(pthread_self());

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  /* want to be cancelled immediately so we don't try to derefence the event
     after it has been deleted */
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  DEBUG_THREADS("[ProcessRunningEvent_Client_tep] Caught event.\n");

  Licq::Event* e = (Licq::Event*)p;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_testcancel();

  // Check if the socket is connected
  if (e->m_nSocketDesc == -1)
  {
    Licq::UserId userId = e->userId();
    string id = userId.accountId();
    unsigned char nChannel = e->Channel();
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    unsigned long nVersion;
    unsigned char nMode;
    unsigned short nRemotePort;
    bool bSendIntIp;
    {
      Licq::UserReadGuard u(userId);
      if (!u.isLocked())
      {
        if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultError) != NULL)
          gIcqProtocol.ProcessDoneEvent(e);
        else
        {
          pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
          pthread_testcancel();
          delete e;
        }
        pthread_exit(NULL);
      }

      nVersion = u->Version();
      nMode = u->Mode();
      nRemotePort = u->Port();
      bSendIntIp = u->SendIntIp();
    }

    unsigned long nIP;
    unsigned short nLocalPort;
    {
      Licq::OwnerReadGuard o(LICQ_PPID);
      nIP = bSendIntIp ? o->IntIp() : o->Ip();
      nLocalPort = o->Port();
    }

    int socket = -1;
    if (!bSendIntIp && nVersion > 6 && nMode != MODE_DIRECT)
    {
      int nId = gIcqProtocol.requestReverseConnection(userId, nChannel, nIP, nLocalPort, nRemotePort);
      if (nId != -1)
      {
        gIcqProtocol.waitForReverseConnection(nId, userId);
        Licq::UserReadGuard u(userId);
        if (!u.isLocked())
        {
          if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultError) != NULL)
            gIcqProtocol.ProcessDoneEvent(e);
          else
          {
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
            pthread_testcancel();
            delete e;
          }
          pthread_exit(NULL);
        }
        socket = u->SocketDesc(nChannel);
      }
      
      // if we failed, try direct anyway
      if (socket == -1)
      {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        socket = gIcqProtocol.connectToUser(userId, nChannel);
      }
    }
    else
    {
      socket = gIcqProtocol.connectToUser(userId, nChannel);

      // if we failed, try through server
      if (socket == -1)
      {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        int nId = gIcqProtocol.requestReverseConnection(userId, nChannel, nIP,
                                              nLocalPort, nRemotePort);
        if (nId != -1)
        {
          gIcqProtocol.waitForReverseConnection(nId, userId);
          Licq::UserReadGuard u(userId);
          if (!u.isLocked())
          {
            if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultError) != NULL)
              gIcqProtocol.ProcessDoneEvent(e);
            else
            {
              pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
              pthread_testcancel();
              delete e;
            }
            pthread_exit(NULL);
          }
          socket = u->SocketDesc(nChannel);
        }
      }
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    e->m_nSocketDesc = socket;
    // Check again, if still -1, fail the event
    if (e->m_nSocketDesc == -1)
    {
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
      if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultError) != NULL)
        gIcqProtocol.ProcessDoneEvent(e);
      else
      {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        delete e;
      }
      pthread_exit(NULL);
    }
  }

  int socket = e->m_nSocketDesc;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  Licq::INetSocket* s = gSocketManager.FetchSocket(socket);
  if (s == NULL)
  {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    unsigned short nSequence = e->m_nSequence;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    gLog.warning(tr("%sSocket %d does not exist (#%hu).\n"), L_WARNxSTR, socket,
       nSequence);
    if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultError) != NULL)
      gIcqProtocol.ProcessDoneEvent(e);
    else
    {
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      delete e;
    }
    pthread_exit(NULL);
  }

  Licq::Buffer* buf;
  bool sent;
  string errorStr;
  pthread_cleanup_push(cleanup_socket, s);

  pthread_mutex_lock(&gIcqProtocol.mutex_cancelthread);

    // check to make sure we were not cancelled already
  pthread_cleanup_push(cleanup_mutex, &gIcqProtocol.mutex_cancelthread);
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

      //if we get here  then we haven't been cancelled and we won't be
      //as long as we hold mutex_cancelthread

      buf = e->m_pPacket->Finalize(s);

  pthread_mutex_unlock(&gIcqProtocol.mutex_cancelthread);
    pthread_cleanup_pop(0);

    sent = s->Send(buf);

    if (!sent)
      errorStr = s->errorStr();

    gSocketManager.DropSocket(s);
  pthread_cleanup_pop(0);

  if (!sent)
  {
    // Close the socket, alert the socket thread
    gSocketManager.CloseSocket(socket);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    unsigned short nSequence = e->m_nSequence;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    gLog.warning(tr("%sError sending event (#%hu):\n%s%s.\n"), L_WARNxSTR,
        -nSequence, L_BLANKxSTR, errorStr.c_str());
    gIcqProtocol.myNewSocketPipe.putChar('S');
    // Kill the event, do after the above as ProcessDoneEvent erase the event
    if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultError) != NULL)
      gIcqProtocol.ProcessDoneEvent(e);
    else
    {
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      delete e;
    }
    pthread_exit(NULL);
  }
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_testcancel();

  e->thread_running = false;

  // pthread_exit is not async cancel safe???
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  pthread_exit(NULL);
  // Avoid compiler warnings
  return NULL;
}


/*------------------------------------------------------------------------------
 * ReverseConnectToUser_tep
 *
 * Creates a new TCPSocket and connects it to a given user.  Adds the socket
 * to the global socket manager and to the user.
 *----------------------------------------------------------------------------*/
void *ReverseConnectToUser_tep(void *v)
{
  pthread_detach(pthread_self());

  DEBUG_THREADS("[ReverseConnectToUser_tep] Caught event.\n");

  CReverseConnectToUserData *p = (CReverseConnectToUserData *)v;

  Licq::UserId userId(p->myIdString, LICQ_PPID);
  gIcqProtocol.reverseConnectToUser(userId, p->nIp, p->nPort,
      p->nVersion, p->nFailedPort, p->nId, p->nMsgID1, p->nMsgID2);

  delete p;

  return NULL;
}




/*------------------------------------------------------------------------------
 * Ping_tep
 *
 * Thread entry point to ping the server every n minutes.
 *----------------------------------------------------------------------------*/
void *Ping_tep(void *p)
{
  pthread_detach(pthread_self());

  CICQDaemon *d = (CICQDaemon *)p;
  struct timeval tv;

  while (true)
  {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    LicqDaemon::gStatistics.flush();
    switch(d->m_eStatus)
    {
    case STATUS_ONLINE:
        gIcqProtocol.icqPing();
      break;
    case STATUS_OFFLINE_MANUAL:
      break;
    case STATUS_OFFLINE_FORCED:
      if (time(NULL) > gIcqProtocol.m_tLogonTime + IcqProtocol::LogonAttemptDelay)
        gIcqProtocol.icqRelogon();
      break;
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();

    tv.tv_sec = IcqProtocol::PingFrequency;
    tv.tv_usec = 0;
    select(0, NULL, NULL, NULL, &tv);

    pthread_testcancel();
  }
  return NULL;
}



/*------------------------------------------------------------------------------
 * MonitorSockets_tep
 *
 * The server thread lives here.  The main guy who waits on socket activity
 * and processes incoming packets.
 *----------------------------------------------------------------------------*/
void *MonitorSockets_tep(void* /* p */)
{
  //pthread_detach(pthread_self());

  fd_set f;
  int nSocketsAvailable, nServiceSocket, l;

  while (true)
  {
    /*pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();*/

    f = gSocketManager.socketSet();
    l = gSocketManager.LargestSocket() + 1;

    // Add the new socket pipe descriptor
    FD_SET(gIcqProtocol.myNewSocketPipe.getReadFd(), &f);
    if (gIcqProtocol.myNewSocketPipe.getReadFd() >= l)
      l = gIcqProtocol.myNewSocketPipe.getReadFd() + 1;

    // Add the fifo descriptor
    if (LicqDaemon::gFifo.fifo_fd != -1)
    {
      FD_SET(LicqDaemon::gFifo.fifo_fd, &f);
      if (LicqDaemon::gFifo.fifo_fd >= l)
        l = LicqDaemon::gFifo.fifo_fd + 1;
    }

    nSocketsAvailable = select(l, &f, NULL, NULL, NULL);

    /*pthread_testcancel();
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);*/
    if (gIcqProtocol.m_xBARTService)
    {
      COscarService *svc = gIcqProtocol.m_xBARTService;
      nServiceSocket = svc->GetSocketDesc();
    }
    else
      nServiceSocket = -1;

    for (int nCurrentSocket = 0; nSocketsAvailable >= 0 && nCurrentSocket < l; ++nCurrentSocket)
    {
      if (!FD_ISSET(nCurrentSocket, &f))
        continue;

      --nSocketsAvailable;


        // New socket event ----------------------------------------------------
      if (nCurrentSocket == gIcqProtocol.myNewSocketPipe.getReadFd())
      {
        char buf = gIcqProtocol.myNewSocketPipe.getChar();
        if (buf == 'S')
        {
            DEBUG_THREADS("[MonitorSockets_tep] Reloading socket info.\n");
          }
        else if (buf == 'X')
        {
            DEBUG_THREADS("[MonitorSockets_tep] Exiting.\n");
            pthread_exit(NULL);
          }
        }

        // Fifo event ----------------------------------------------------------
      if (nCurrentSocket == LicqDaemon::gFifo.fifo_fd)
      {
          DEBUG_THREADS("[MonitorSockets_tep] Data on FIFO.\n");
        char buf[1024];
        fgets(buf, 1024, LicqDaemon::gFifo.fifo_fs);
        LicqDaemon::gFifo.process(buf);
        continue;
      }

      Licq::INetSocket *s = gSocketManager.FetchSocket(nCurrentSocket);
      if (s != NULL && s->userId().isValid() &&
          s->userId() == Licq::gUserManager.ownerUserId(LICQ_PPID) &&
              gIcqProtocol.m_nTCPSrvSocketDesc == -1)
          {
            /* This is the server socket and it is about to be destoryed
               so ignore this message (it's probably a disconnection anyway) */
            gSocketManager.DropSocket(s);
            continue;
          }

          // Message from the server -------------------------------------------
      if (nCurrentSocket == gIcqProtocol.m_nTCPSrvSocketDesc)
      {
              DEBUG_THREADS("[MonitorSockets_tep] Data on TCP server socket.\n");
        Licq::SrvSocket* srvTCP = dynamic_cast<Licq::SrvSocket*>(s);
              if (srvTCP == NULL)
              {
              gLog.warning(tr("%sInvalid server socket in set.\n"), L_WARNxSTR);
              close(nCurrentSocket);
          continue;
        }

            // DAW FIXME error handling when socket is closed..
        if (srvTCP->Recv())
        {
          Licq::Buffer packet(srvTCP->RecvBuffer());
              srvTCP->ClearRecvBuffer();
              gSocketManager.DropSocket(srvTCP);
          if (!gIcqProtocol.ProcessSrvPacket(packet))
          {} // gIcqProtocol.icqRelogon();
        }
            else {
              // probably server closed socket, try to relogon after a while
              // if ping-thread is running already
          int nSD = gIcqProtocol.m_nTCPSrvSocketDesc;
          gIcqProtocol.m_nTCPSrvSocketDesc = -1;
              gLog.info("%sDropping server connection.\n", L_SRVxSTR);
              gSocketManager.DropSocket(srvTCP);
              gSocketManager.CloseSocket(nSD);
              // we need to initialize the logon time for the next retry
          gIcqProtocol.m_tLogonTime = time(NULL);
          gIcqProtocol.m_eStatus = STATUS_OFFLINE_FORCED;
          gIcqProtocol.m_bLoggingOn = false;
          gIcqProtocol.postLogoff(nSD, NULL);
        }
      }

          // Message from the service sockets -----------------------------------
          else if (nCurrentSocket == nServiceSocket)
          {
            DEBUG_THREADS("[MonitorSockets_tep] Data on BART service socket.\n");
        COscarService *svc = gIcqProtocol.m_xBARTService;
        Licq::SrvSocket* sock_svc = dynamic_cast<Licq::SrvSocket*>(s);
        if (sock_svc == NULL)
        {
          gLog.warning(tr("%sInvalid BART service socket in set.\n"), L_WARNxSTR);
          close(nCurrentSocket);
          continue;
        }
            if (sock_svc->Recv())
            {
          Licq::Buffer packet(sock_svc->RecvBuffer());
              sock_svc->ClearRecvBuffer();
              gSocketManager.DropSocket(sock_svc);
              if (!svc->ProcessPacket(packet))
              {
                gLog.warning(tr("%sCan't process packet for service 0x%02X.\n"),
                          L_WARNxSTR, svc->GetFam());
                svc->ResetSocket();
                svc->ChangeStatus(STATUS_UNINITIALIZED);
                gSocketManager.CloseSocket(nCurrentSocket);
              }
            }
            else
            {
              gLog.warning(tr("%sCan't receive packet for service 0x%02X.\n"),
                        L_WARNxSTR, svc->GetFam());
              svc->ResetSocket();
              svc->ChangeStatus(STATUS_UNINITIALIZED);
              gSocketManager.DropSocket(sock_svc);
              gSocketManager.CloseSocket(nCurrentSocket);
            }
          }

          // Connection on the server port -------------------------------------
      else if (nCurrentSocket == gIcqProtocol.m_nTCPSocketDesc)
      {
            DEBUG_THREADS("[MonitorSockets_tep] Data on listening TCP socket."
                          "\n");
        Licq::TCPSocket* tcp = dynamic_cast<Licq::TCPSocket*>(s);
            if (tcp == NULL)
            {
              gLog.warning(tr("%sInvalid server TCP socket in set.\n"), L_WARNxSTR);
              close(nCurrentSocket);
          continue;
        }

        Licq::TCPSocket* newSocket = new Licq::TCPSocket();
              bool ok = tcp->RecvConnection(*newSocket);
              gSocketManager.DropSocket(tcp);

              // Make sure we can handle another socket before accepting it
              if (!ok || gSocketManager.Num() > MAX_CONNECTS)
              {
                // Too many sockets, drop this one
                gLog.warning(tr("%sToo many connected sockets, rejecting connection from %s.\n"),
                    L_WARNxSTR, newSocket->getRemoteIpString().c_str());
                delete newSocket;
              }
              else
              {
                gSocketManager.AddSocket(newSocket);
                gSocketManager.DropSocket(newSocket);
            }
          }

          // Message from connected socket--------------------------------------
          else
          {
            DEBUG_THREADS("[MonitorSockets_tep] Data on TCP user socket.\n");

            ssl_recv:

        Licq::TCPSocket* tcp = dynamic_cast<Licq::TCPSocket*>(s);

            // If tcp is NULL then the socket is no longer in the set, hence it
            // must have been closed by us and we can ignore it.
            if (tcp == NULL)
          continue;

            if (!tcp->RecvPacket())
            {
              int err = tcp->Error();
              if (err == 0)
                gLog.info(tr("%sConnection to %s was closed.\n"), L_TCPxSTR,
                tcp->userId().toString().c_str());
              else
                gLog.info(tr("%sConnection to %s lost:\n%s%s.\n"), L_TCPxSTR,
                    tcp->userId().toString().c_str(), L_BLANKxSTR, tcp->errorStr().c_str());
          if (tcp->userId().isValid())
          {
            Licq::UserWriteGuard u(tcp->userId());
            if (u.isLocked() && u->Secure())
            {
              u->ClearSocketDesc(ICQ_CHNxNONE);
              u->SetSecure(false);
              Licq::gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                  Licq::PluginSignal::UserSecurity, u->id(), 0));
            }
          }
              gSocketManager.DropSocket(tcp);
              gSocketManager.CloseSocket(nCurrentSocket);
          gIcqProtocol.FailEvents(nCurrentSocket, err);

              break;
            }

            // Save the bytes pending status of the socket
            bool bPending = tcp->SSL_Pending();
            bool r = true;

            // Process the packet if the buffer is full
            if (tcp->RecvBufferFull())
            {
          if (tcp->userId().protocolId() != LICQ_PPID)
            r = gIcqProtocol.ProcessTcpHandshake(tcp);
          else
            r = gIcqProtocol.ProcessTcpPacket(tcp);
              tcp->ClearRecvBuffer();
            }

            // Kill the socket if there was a problem
            if (!r)
            {
              gLog.info(tr("%sClosing connection to %s.\n"), L_TCPxSTR,
              tcp->userId().toString().c_str());
              gSocketManager.DropSocket(tcp);
              gSocketManager.CloseSocket(nCurrentSocket);
          gIcqProtocol.FailEvents(nCurrentSocket, 0);
              bPending = false;
            }
            else
            {
              gSocketManager.DropSocket(tcp);
            }

            // If there is more data pending then go again
            if (bPending) goto ssl_recv;
      }
    }
  }
  return NULL;
}



/*------------------------------------------------------------------------------
 * Shutdown_tep
 *
 * Shutdown the daemon and all the plugins.
 *----------------------------------------------------------------------------*/
extern pthread_cond_t LP_IdSignal;
extern pthread_mutex_t LP_IdMutex;
extern list<unsigned short> LP_Ids;

void *Shutdown_tep(void* /* p */)
{
  // Shutdown
  gLog.info(tr("Shutting down daemon"));

  // Send shutdown signal to all the plugins
  LicqDaemon::gDaemon.shutdownPlugins();

  // Cancel the monitor sockets thread (deferred until ready)
  gIcqProtocol.myNewSocketPipe.putChar('X');

  // Cancel the ping thread
  pthread_cancel(gIcqProtocol.thread_ping);

  // Cancel the update users thread
  pthread_cancel(gIcqProtocol.thread_updateusers);

  // Cancel the BART service thread
  if (gIcqProtocol.m_xBARTService)
    pthread_cancel(gIcqProtocol.thread_ssbiservice);

  // Join our threads
  pthread_join(gIcqProtocol.thread_monitorsockets, NULL);

  if (gIcqProtocol.m_nTCPSrvSocketDesc != -1 )
    gIcqProtocol.icqLogoff();
  if (gIcqProtocol.m_nTCPSocketDesc != -1)
    gSocketManager.CloseSocket(gIcqProtocol.m_nTCPSocketDesc);

  // Flush the stats
  LicqDaemon::gStatistics.flush();

  // Signal that we are shutdown
  pthread_mutex_lock(&LP_IdMutex);
  LP_Ids.push_back(0);
  pthread_mutex_unlock(&LP_IdMutex);
  pthread_cond_signal(&LP_IdSignal);

  return NULL;
}

void *UpdateUsers_tep(void *p)
{
  pthread_detach(pthread_self());

  CICQDaemon *d = (CICQDaemon *)p;
  struct timeval tv;

  while (true)
  {
    if (d->m_eStatus == STATUS_ONLINE)
    {
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
      Licq::UserListGuard userList(LICQ_PPID);
      BOOST_FOREACH(Licq::User* user, **userList)
      {
        Licq::UserWriteGuard pUser(user);
        bool bSent = false;
        bool bBART = false;

        if (d->AutoUpdateInfo() && !pUser->UserUpdated() &&
            pUser->ClientTimestamp() != pUser->OurClientTimestamp()
            && pUser->ClientTimestamp() != 0)
        {
          gIcqProtocol.icqRequestMetaInfo(pUser->id());
          bSent = true;
        }

        if (d->UseServerSideBuddyIcons() && d->AutoUpdateInfo() &&
            pUser->buddyIconHash().size() > 0 &&
            pUser->buddyIconHash() != pUser->ourBuddyIconHash())
        {
          gIcqProtocol.m_xBARTService->SendEvent(pUser->id(), ICQ_SNACxBART_DOWNLOADxREQUEST, true);
          bSent = true;
          bBART = true;
        }

        if (pUser->isOnline() && !pUser->UserUpdated() &&
            //Don't bother clients that we know don't support plugins
            pUser->Version() >= 7 &&
            //Old versions of Licq
            (((pUser->ClientTimestamp() & 0xFFFF0000) != LICQ_WITHSSL &&
              (pUser->ClientTimestamp() & 0xFFFF0000) != LICQ_WITHOUTSSL) ||
             (pUser->ClientTimestamp() & 0xFFFF) > 1026) &&
            pUser->ClientTimestamp() != 0xFFFFFF42 && //mICQ
            pUser->ClientTimestamp() != 0xFFFFFFFF && //Miranda
            pUser->ClientTimestamp() != 0xFFFFFF7F && //&RQ
            pUser->ClientTimestamp() != 0xFFFFFFBE && //Alicq
            pUser->ClientTimestamp() != 0x3B75AC09 && //Trillian
            pUser->ClientTimestamp() != 0x3AA773EE && //libICQ2000 based clients
            pUser->ClientTimestamp() != 0x3BC1252C && //ICQ Interest Search
            pUser->ClientTimestamp() != 0x3B176B57 && //jcq2k
            pUser->ClientTimestamp() != 0x3BA76E2E && //SmartICQ
            pUser->ClientTimestamp() != 0x3C7D8CBC && //Vista
            pUser->ClientTimestamp() != 0x3CFE0688 && //Meca
            pUser->ClientTimestamp() != 0x3BFF8C98 //IGA
           )
        {
          if (d->AutoUpdateInfoPlugins() &&
              pUser->ClientInfoTimestamp() != pUser->OurClientInfoTimestamp() &&
              pUser->ClientInfoTimestamp() != 0)
          {
            gLog.info("Updating %s's info plugins.\n", pUser->GetAlias());
            gIcqProtocol.icqRequestInfoPlugin(*pUser, true, PLUGIN_QUERYxINFO);
            gIcqProtocol.icqRequestInfoPlugin(*pUser, true, PLUGIN_PHONExBOOK);
            if (!bBART) // Send only if we didn't request BART already
              gIcqProtocol.icqRequestInfoPlugin(*pUser, true, PLUGIN_PICTURE);
            bSent = true;
          }

          if (d->AutoUpdateStatusPlugins() &&
             pUser->ClientStatusTimestamp() != pUser->OurClientStatusTimestamp()
              && pUser->ClientStatusTimestamp() != 0)
          {
            gLog.info("Updating %s's status plugins.\n", pUser->GetAlias());
            gIcqProtocol.icqRequestStatusPlugin(*pUser, true, PLUGIN_QUERYxSTATUS);
            gIcqProtocol.icqRequestStatusPlugin(*pUser, true, PLUGIN_FILExSERVER);
            gIcqProtocol.icqRequestStatusPlugin(*pUser, true, PLUGIN_FOLLOWxME);
            gIcqProtocol.icqRequestStatusPlugin(*pUser, true, PLUGIN_ICQxPHONE);
            bSent = true;
          }

        }

        if (bSent)
        {
          pUser->SetUserUpdated(true);
          break;
        }
      }
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    }

    pthread_testcancel();

    tv.tv_sec = IcqProtocol::UpdateFrequency;
    tv.tv_usec = 0;
    select(0, NULL, NULL, NULL, &tv);

    pthread_testcancel();
  }

  pthread_exit(NULL);
}
