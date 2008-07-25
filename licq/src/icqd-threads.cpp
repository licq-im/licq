// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifndef LICQTHREADS_H
#define LICQTHREADS_H

#include "licq_icqd.h"

#include <cerrno>
#include <unistd.h>

#include "pthread_rdwr.h"
#include "time-fix.h"

#include "licq_log.h"
#include "licq_packets.h"
#include "licq_oscarservice.h"
#include "licq_plugind.h"
#include "licq.h"

// Localization
#include "gettext.h"

#define MAX_CONNECTS  256
#define DEBUG_THREADS(x)
//#define DEBUG_THREADS(x) gLog.Info(x)

using namespace std;

void cleanup_mutex(void *m)
{
  pthread_mutex_unlock((pthread_mutex_t *)m);
}

void cleanup_socket(void *s)
{
  gSocketManager.DropSocket((INetSocket *)s);
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
  *((int *)s) = gLicqDaemon->ConnectToLoginServer();
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
  pthread_mutex_lock(&d->mutex_sendqueue_server);

  list<ICQEvent *>::iterator iter;
  ICQEvent *e = NULL;

  while (e == NULL)
  {

    for (iter = d->m_lxSendQueue_Server.begin();
         iter != d->m_lxSendQueue_Server.end(); ++iter)
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
      bool bEmpty = d->m_lxSendQueue_Server.empty();

      pthread_mutex_unlock(&d->mutex_sendqueue_server);
      pthread_mutex_unlock(&send_mutex);

      if (bEmpty)
        pthread_exit(NULL);

      struct timeval tv = { 1, 0 };
      select(0, NULL, NULL, NULL, &tv);
      pthread_mutex_lock(&send_mutex);
      pthread_mutex_lock(&d->mutex_sendqueue_server);
    }
    else
    {
      d->m_lxSendQueue_Server.erase(iter);

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
  pthread_mutex_unlock(&d->mutex_sendqueue_server);

  // declared here because pthread_cleanup_push starts a new block
  CBuffer *buf;
  bool sent = false;
  bool bExit = false;
  char szErrorBuf[128];

  pthread_cleanup_push(cleanup_mutex, &send_mutex);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();

    int socket = -1;
    unsigned short nSequence;
    INetSocket *s;
    
    // Check if the socket is connected
    if (e->m_nSocketDesc == -1)
    {
      // Connect to the server if we are logging on
      if (e->m_pPacket->Channel() == ICQ_CHNxNEW)
      {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        gLog.Info(tr("%sConnecting to login server.\n"), L_SRVxSTR);

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
          gLog.Info(tr("%sConnecting to login server failed, failing event\n"),
                    L_SRVxSTR);
          // we need to initialize the logon time for the next retry
          d->m_tLogonTime = time(NULL);
          d->m_eStatus = STATUS_OFFLINE_FORCED;
          d->m_bLoggingOn = false;
          if (d->DoneEvent(e, EVENT_ERROR) != NULL)
          {
            d->DoneExtendedEvent(e, EVENT_ERROR);
            d->ProcessDoneEvent(e);
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
        gLog.Info(tr("%sNot connected to server, failing event\n"), L_SRVxSTR);
        if (d->DoneEvent(e, EVENT_ERROR) != NULL)
        {
          d->DoneExtendedEvent(e, EVENT_ERROR);
          d->ProcessDoneEvent(e);
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
      gLog.Warn(tr("%sSocket not connected or invalid (#%hu).\n"), L_WARNxSTR,
                nSequence);
      if (d->DoneEvent(e, EVENT_ERROR) != NULL)
      {
        d->DoneExtendedEvent(e, EVENT_ERROR);
        d->ProcessDoneEvent(e);
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

      pthread_mutex_lock(&d->mutex_cancelthread);

      // check to make sure we were not cancelled already
      pthread_cleanup_push(cleanup_mutex, &d->mutex_cancelthread);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        //if we get here  then we haven't been cancelled and we won't be
        //as long as we hold mutex_cancelthread

        buf = e->m_pPacket->Finalize(NULL);

        pthread_mutex_unlock(&d->mutex_cancelthread);
      pthread_cleanup_pop(0); //mutex_cancelthread

      sent = s->Send(buf);
      delete buf;

      if (!sent)
      {
        s->ErrorStr(szErrorBuf, 128);
      }

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

    gLog.Warn(tr("%sError sending event (#%hu):\n%s%s.\n"), L_WARNxSTR,
              nSequence, L_BLANKxSTR, szErrorBuf);

    if (d->DoneEvent(e, EVENT_ERROR) != NULL)
    {
      d->DoneExtendedEvent(e, EVENT_ERROR);
      d->ProcessDoneEvent(e);
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
      if (d->DoneEvent(e, EVENT_ACKED) != NULL)
      {
        d->DoneExtendedEvent(e, EVENT_ACKED);
        d->ProcessDoneEvent(e);
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

  ICQEvent *e = (ICQEvent *)p;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_testcancel();

  CICQDaemon *d = e->m_pDaemon;

  // Check if the socket is connected
  if (e->m_nSocketDesc == -1)
  {
    string id = e->Id();
    unsigned long ppid = e->PPID();
    unsigned char nChannel = e->Channel();
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    const ICQUser* u = gUserManager.FetchUser(id.c_str(), ppid, LOCK_R);
    if (u == NULL)
    {
      if (d->DoneEvent(e, EVENT_ERROR) != NULL)
        d->ProcessDoneEvent(e);
      else
      {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        delete e;
      }
      pthread_exit(NULL);
    }

    unsigned long nVersion = u->Version();
    unsigned char nMode = u->Mode();
    unsigned short nRemotePort = u->Port();
    bool bSendIntIp = u->SendIntIp();
    gUserManager.DropUser(u);

    const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
    unsigned long nIP = bSendIntIp ? o->IntIp() : o->Ip();
    unsigned short nLocalPort = o->Port();
    gUserManager.DropOwner(o);

    int socket = -1;
    if (!bSendIntIp && nVersion > 6 && nMode != MODE_DIRECT)
    {
      int nId = d->RequestReverseConnection(id.c_str(), nChannel, nIP, nLocalPort, nRemotePort);
      if (nId != -1)
      {
        d->WaitForReverseConnection(nId, id.c_str());
        u = gUserManager.FetchUser(id.c_str(), LICQ_PPID, LOCK_R);
        if (u == NULL)
        {
          if (d->DoneEvent(e, EVENT_ERROR) != NULL)
            d->ProcessDoneEvent(e);
          else
          {
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
            pthread_testcancel();
            delete e;
          }
          pthread_exit(NULL);
        }
        socket = u->SocketDesc(nChannel);
        gUserManager.DropUser(u);
      }
      
      // if we failed, try direct anyway
      if (socket == -1)
      {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        socket = d->ConnectToUser(id.c_str(), nChannel);
      }
    }
    else
    {
      socket = d->ConnectToUser(id.c_str(), nChannel);

      // if we failed, try through server
      if (socket == -1)
      {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        int nId = d->RequestReverseConnection(id.c_str(), nChannel, nIP,
                                              nLocalPort, nRemotePort);
        if (nId != -1)
        {
          d->WaitForReverseConnection(nId, id.c_str());
          u = gUserManager.FetchUser(id.c_str(), LICQ_PPID, LOCK_R);
          if (u == NULL)
          {
            if (d->DoneEvent(e, EVENT_ERROR) != NULL)
              d->ProcessDoneEvent(e);
            else
            {
              pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
              pthread_testcancel();
              delete e;
            }
            pthread_exit(NULL);
          }
          socket = u->SocketDesc(nChannel);
          gUserManager.DropUser(u);
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
      if (d->DoneEvent(e, EVENT_ERROR) != NULL)
        d->ProcessDoneEvent(e);
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
  INetSocket *s = gSocketManager.FetchSocket(socket);
  if (s == NULL)
  {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    unsigned short nSequence = e->m_nSequence;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    gLog.Warn(tr("%sSocket %d does not exist (#%hu).\n"), L_WARNxSTR, socket,
       nSequence);
    if (d->DoneEvent(e, EVENT_ERROR) != NULL)
      d->ProcessDoneEvent(e);
    else
    {
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      delete e;
    }
    pthread_exit(NULL);
  }

  CBuffer *buf;
  bool sent;
  char szErrorBuf[128];
  pthread_cleanup_push(cleanup_socket, s);

    pthread_mutex_lock(&d->mutex_cancelthread);

    // check to make sure we were not cancelled already
    pthread_cleanup_push(cleanup_mutex, &d->mutex_cancelthread);
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

      //if we get here  then we haven't been cancelled and we won't be
      //as long as we hold mutex_cancelthread

      buf = e->m_pPacket->Finalize(s);

      pthread_mutex_unlock(&d->mutex_cancelthread);
    pthread_cleanup_pop(0);

    sent = s->Send(buf);

    if (!sent)
      s->ErrorStr(szErrorBuf, 128);

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

    gLog.Warn(tr("%sError sending event (#%hu):\n%s%s.\n"), L_WARNxSTR,
     -nSequence, L_BLANKxSTR, szErrorBuf);
    write(d->pipe_newsocket[PIPE_WRITE], "S", 1);
    // Kill the event, do after the above as ProcessDoneEvent erase the event
    if (d->DoneEvent(e, EVENT_ERROR) != NULL)
      d->ProcessDoneEvent(e);
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

  gLicqDaemon->ReverseConnectToUser(p->myIdString.c_str(), p->nIp, p->nPort,
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
    d->FlushStats();
    switch(d->m_eStatus)
    {
    case STATUS_ONLINE:
      d->icqPing();
      break;
    case STATUS_OFFLINE_MANUAL:
      break;
    case STATUS_OFFLINE_FORCED:
      if (time(NULL) > d->m_tLogonTime + LOGON_ATTEMPT_DELAY)
        d->icqRelogon();
      break;
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();

    tv.tv_sec = PING_FREQUENCY;
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
void *MonitorSockets_tep(void *p)
{
  //pthread_detach(pthread_self());

  CICQDaemon *d = (CICQDaemon *)p;

  fd_set f;
  int nSocketsAvailable, nCurrentSocket, nServiceSocket, l;
  char buf[1024];

  while (true)
  {
    /*pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();*/

    f = gSocketManager.SocketSet();
    l = gSocketManager.LargestSocket() + 1;

    // Add the new socket pipe descriptor
    FD_SET(d->pipe_newsocket[PIPE_READ], &f);
    if (d->pipe_newsocket[PIPE_READ] >= l) l = d->pipe_newsocket[PIPE_READ] + 1;

    // Add the fifo descriptor
    if (d->fifo_fd != -1)
    {
      FD_SET(d->fifo_fd, &f);
      if (d->fifo_fd >= l) l = d->fifo_fd + 1;
    }

    nSocketsAvailable = select(l, &f, NULL, NULL, NULL);

    /*pthread_testcancel();
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);*/
    if (d->m_xBARTService)
    {
      COscarService *svc = d->m_xBARTService;
      nServiceSocket = svc->GetSocketDesc();
    }
    else
      nServiceSocket = -1;
                                                  
    nCurrentSocket = 0;
    while (nSocketsAvailable > 0 && nCurrentSocket < l)
    {
      if (FD_ISSET(nCurrentSocket, &f))
      {
        // New socket event ----------------------------------------------------
        if (nCurrentSocket == d->pipe_newsocket[PIPE_READ])
        {
          read(d->pipe_newsocket[PIPE_READ], buf, 1);
          if (buf[0] == 'S')
          {
            DEBUG_THREADS("[MonitorSockets_tep] Reloading socket info.\n");
          }
          else if (buf[0] == 'X')
          {
            DEBUG_THREADS("[MonitorSockets_tep] Exiting.\n");
            pthread_exit(NULL);
          }
        }

        // Fifo event ----------------------------------------------------------
        if (nCurrentSocket == d->fifo_fd)
        {
          DEBUG_THREADS("[MonitorSockets_tep] Data on FIFO.\n");
          fgets(buf, 1024, d->fifo_fs);
          d->ProcessFifo(buf);
        }

        else
        {
          INetSocket *s = gSocketManager.FetchSocket(nCurrentSocket);
          if (s != NULL && s->OwnerId() != NULL &&
              s->OwnerId() == gUserManager.OwnerId(LICQ_PPID) &&
              d->m_nTCPSrvSocketDesc == -1)
          {
            /* This is the server socket and it is about to be destoryed
               so ignore this message (it's probably a disconnection anyway) */
            gSocketManager.DropSocket(s);
          }

          // Message from the server -------------------------------------------
          else if (nCurrentSocket == d->m_nTCPSrvSocketDesc)
          {
              DEBUG_THREADS("[MonitorSockets_tep] Data on TCP server socket.\n");
              SrvSocket *srvTCP = static_cast<SrvSocket*>(s);
              if (srvTCP == NULL)
              {
              gLog.Warn(tr("%sInvalid server socket in set.\n"), L_WARNxSTR);
              close(nCurrentSocket);
            }
            // DAW FIXME error handling when socket is closed..
            else if (srvTCP->Recv())
            {
              CBuffer packet(srvTCP->RecvBuffer());
              srvTCP->ClearRecvBuffer();
              gSocketManager.DropSocket(srvTCP);
              if (!d->ProcessSrvPacket(packet)) {} // d->icqRelogon();
            }
            else {
              // probably server closed socket, try to relogon after a while
              // if ping-thread is running already
              int nSD = d->m_nTCPSrvSocketDesc;
              d->m_nTCPSrvSocketDesc = -1;
              gLog.Info("%sDropping server connection.\n", L_SRVxSTR);
              gSocketManager.DropSocket(srvTCP);
              gSocketManager.CloseSocket(nSD);
              // we need to initialize the logon time for the next retry
              d->m_tLogonTime = time(NULL);
              d->m_eStatus = STATUS_OFFLINE_FORCED;
              d->m_bLoggingOn = false;
              d->postLogoff(nSD, NULL);
            }
          }

          // Message from the service sockets -----------------------------------
          else if (nCurrentSocket == nServiceSocket)
          {
            DEBUG_THREADS("[MonitorSockets_tep] Data on BART service socket.\n");
            COscarService *svc = d->m_xBARTService;
            SrvSocket *sock_svc = static_cast<SrvSocket*>(s);
            if (sock_svc->Recv())
            {
              CBuffer packet(sock_svc->RecvBuffer());
              sock_svc->ClearRecvBuffer();
              gSocketManager.DropSocket(sock_svc);
              if (!svc->ProcessPacket(packet))
              {
                gLog.Warn(tr("%sCan't process packet for service 0x%02X.\n"),
                          L_WARNxSTR, svc->GetFam());
                svc->ResetSocket();
                svc->ChangeStatus(STATUS_UNINITIALIZED);
                gSocketManager.CloseSocket(nCurrentSocket);
              }
            }
            else
            {
              gLog.Warn(tr("%sCan't receive packet for service 0x%02X.\n"),
                        L_WARNxSTR, svc->GetFam());
              svc->ResetSocket();
              svc->ChangeStatus(STATUS_UNINITIALIZED);
              gSocketManager.DropSocket(sock_svc);
              gSocketManager.CloseSocket(nCurrentSocket);
            }
          }

          // Connection on the server port -------------------------------------
          else if (nCurrentSocket == d->m_nTCPSocketDesc)
          {
            DEBUG_THREADS("[MonitorSockets_tep] Data on listening TCP socket."
                          "\n");
            TCPSocket* tcp = static_cast<TCPSocket *>(s);
            if (tcp == NULL)
            {
              gLog.Warn(tr("%sInvalid server TCP socket in set.\n"), L_WARNxSTR);
              close(nCurrentSocket);
            }
            else
            {
              TCPSocket *newSocket = new TCPSocket();
              bool ok = tcp->RecvConnection(*newSocket);
              gSocketManager.DropSocket(tcp);

              // Make sure we can handle another socket before accepting it
              if (!ok || gSocketManager.Num() > MAX_CONNECTS)
              {
                // Too many sockets, drop this one
                char remoteIp[32];
                gLog.Warn(tr("%sToo many connected sockets, rejecting connection from %s.\n"),
                    L_WARNxSTR, newSocket->RemoteIpStr(remoteIp));
                delete newSocket;
              }
              else
              {
                gSocketManager.AddSocket(newSocket);
                gSocketManager.DropSocket(newSocket);
              }
            }
          }

          // Message from connected socket--------------------------------------
          else
          {
            DEBUG_THREADS("[MonitorSockets_tep] Data on TCP user socket.\n");

            ssl_recv:

            TCPSocket *tcp = static_cast<TCPSocket *>(s);

            // If tcp is NULL then the socket is no longer in the set, hence it
            // must have been closed by us and we can ignore it.
            if (tcp == NULL)
              goto socket_done;

            if (!tcp->RecvPacket())
            {
              int err = tcp->Error();
              if (err == 0)
                gLog.Info(tr("%sConnection to %s was closed.\n"), L_TCPxSTR,
                    tcp->OwnerId());
              else
              {
                char buf[128];
                gLog.Info(tr("%sConnection to %s lost:\n%s%s.\n"), L_TCPxSTR,
                    tcp->OwnerId(), L_BLANKxSTR, tcp->ErrorStr(buf, 128));
              }
              ICQUser *u = gUserManager.FetchUser(tcp->OwnerId(),
                tcp->OwnerPPID(), LOCK_W);
              if (u && u->Secure())
              {
                u->ClearSocketDesc(ICQ_CHNxNONE);
                u->SetSecure(false);
                d->PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                  USER_SECURITY, u->IdString(), u->PPID(), 0));
              }
              gUserManager.DropUser(u);
              gSocketManager.DropSocket(tcp);
              gSocketManager.CloseSocket(nCurrentSocket);
              d->FailEvents(nCurrentSocket, err);

              break;
            }

            // Save the bytes pending status of the socket
            bool bPending = tcp->SSL_Pending();
            bool r = true;

            // Process the packet if the buffer is full
            if (tcp->RecvBufferFull())
            {
              if (tcp->OwnerPPID() != LICQ_PPID)
                r = d->ProcessTcpHandshake(tcp);
              else
                r = d->ProcessTcpPacket(tcp);
              tcp->ClearRecvBuffer();
            }

            // Kill the socket if there was a problem
            if (!r)
            {
              gLog.Info(tr("%sClosing connection to %s.\n"), L_TCPxSTR,
                  tcp->OwnerId());
              gSocketManager.DropSocket(tcp);
              gSocketManager.CloseSocket(nCurrentSocket);
              d->FailEvents(nCurrentSocket, 0);
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

        socket_done:

        nSocketsAvailable--;
      }
      nCurrentSocket++;
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

void *Shutdown_tep(void *p)
{
  CICQDaemon *d = (CICQDaemon *)p;

  // Shutdown
  gLog.Info(tr("%sShutting down daemon.\n"), L_ENDxSTR);

  // Send shutdown signal to all the plugins
  d->licq->ShutdownPlugins();

  // Cancel the monitor sockets thread (deferred until ready)
  write(d->pipe_newsocket[PIPE_WRITE], "X", 1);

  // Cancel the ping thread
  pthread_cancel(d->thread_ping);

  // Cancel the update users thread
  pthread_cancel(d->thread_updateusers);

  // Cancel the BART service thread
  if (d->m_xBARTService)
    pthread_cancel(d->thread_ssbiservice);

  // Join our threads
  pthread_join(d->thread_monitorsockets, NULL);

  if (d->m_nTCPSrvSocketDesc != -1 )
      d->icqLogoff();
  if (d->m_nTCPSocketDesc != -1)
    gSocketManager.CloseSocket(d->m_nTCPSocketDesc);

  // Flush the stats
  d->FlushStats();

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
      FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_R)
      {
        bool bSent = false;
        bool bBART = false;

        if (d->AutoUpdateInfo() && !pUser->UserUpdated() &&
            pUser->ClientTimestamp() != pUser->OurClientTimestamp()
            && pUser->ClientTimestamp() != 0)
        {
          d->icqRequestMetaInfo(pUser->IdString());
          bSent = true;
        }

        if (d->UseServerSideBuddyIcons() && d->AutoUpdateInfo() &&
            strlen(pUser->BuddyIconHash()) > 0 &&
            strcmp(pUser->BuddyIconHash(), pUser->OurBuddyIconHash()) != 0)
        {
          d->m_xBARTService->SendEvent(pUser->IdString(), ICQ_SNACxBART_DOWNLOADxREQUEST, true);
          bSent = true;
          bBART = true;
        }
        
        if (!pUser->StatusOffline() && !pUser->UserUpdated() &&
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
            gLog.Info("Updating %s's info plugins.\n", pUser->GetAlias());
            d->icqRequestInfoPlugin(pUser, true, PLUGIN_QUERYxINFO);
            d->icqRequestInfoPlugin(pUser, true, PLUGIN_PHONExBOOK);
            if (!bBART) // Send only if we didn't request BART already
              d->icqRequestInfoPlugin(pUser, true, PLUGIN_PICTURE);
            bSent = true;
          }

          if (d->AutoUpdateStatusPlugins() &&
             pUser->ClientStatusTimestamp() != pUser->OurClientStatusTimestamp()
              && pUser->ClientStatusTimestamp() != 0)
          {
            gLog.Info("Updating %s's status plugins.\n", pUser->GetAlias());
            d->icqRequestStatusPlugin(pUser, true, PLUGIN_QUERYxSTATUS);
            d->icqRequestStatusPlugin(pUser, true, PLUGIN_FILExSERVER);
            d->icqRequestStatusPlugin(pUser, true, PLUGIN_FOLLOWxME);
            d->icqRequestStatusPlugin(pUser, true, PLUGIN_ICQxPHONE);
            bSent = true;
          }

        }

        if (bSent)
        {
          pUser->SetUserUpdated(true);
          FOR_EACH_PROTO_USER_BREAK;
        }
      }
      FOR_EACH_PROTO_USER_END
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    }

    pthread_testcancel();

    tv.tv_sec = UPDATE_FREQUENCY;
    tv.tv_usec = 0;
    select(0, NULL, NULL, NULL, &tv);

    pthread_testcancel();
  }

  pthread_exit(NULL);
}

#endif
