// -*- c-basic-offset: 2 -*-
#ifndef LICQTHREADS_H
#define LICQTHREADS_H

#include <errno.h>

#include "pthread_rdwr.h"
#include "time-fix.h"

#include "licq_icqd.h"
#include "licq_log.h"
#include "licq_packets.h"
#include "licq_plugind.h"
#include "licq.h"

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
  int *s;
  pthread_join(*((pthread_t *)t), (void **)&s);
  delete (pthread_t *)t;
  delete s;
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
void *ProcessRunningEvent_Server_tep(void *p)
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
         iter != d->m_lxSendQueue_Server.end(); iter++)
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
  bool sent;
  char szErrorBuf[128];

  pthread_cleanup_push(cleanup_mutex, &send_mutex);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();

    // Check if the socket is connected
    if (e->m_nSocketDesc == -1)
    {
      // Connect to the server if we are logging on
      if (e->m_pPacket->Channel() == ICQ_CHNxNEW)
      {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        gLog.Info("%sConnecting to login server.\n", L_SRVxSTR);

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
          gLog.Info("%sConnecting to login server failed, failing event\n",
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
          pthread_exit(NULL);
        }
      }
      else
      {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        gLog.Info("%sNot connected to server, failing event\n", L_SRVxSTR);
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
        pthread_exit(NULL);
      }
    }

    int socket = e->m_nSocketDesc;
    unsigned long nSequence = e->m_nSequence;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    // Start sending the event
    INetSocket *s = gSocketManager.FetchSocket(socket);
    if (s == NULL)
    {
      gLog.Warn("%sSocket not connected or invalid (#%ld).\n", L_WARNxSTR,
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
      pthread_exit(NULL);
    }

    pthread_cleanup_push(cleanup_socket, s);

      /* FIXME ugly hack, but prevents event from being deleted while we still
         need it and cannot be canceled */
      pthread_mutex_lock(&d->mutex_runningevents);

      // check to make sure we were not cancelled already
      pthread_cleanup_push(cleanup_mutex, &d->mutex_runningevents);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        //if we get here  then we haven't been cancelled and we won't be
        //as long as we hold mutex_runningevents

        buf = e->m_pPacket->Finalize(NULL);

        pthread_mutex_unlock(&d->mutex_runningevents);
      pthread_cleanup_pop(0); //mutex_runningevents

      sent = s->Send(buf);
      delete buf;

      if (!sent)
      {
        s->ErrorStr(szErrorBuf, 128);
      }

      // We don't close the socket as it should be closed by the server thread
      gSocketManager.DropSocket(s);
    pthread_cleanup_pop(0); //socket

    pthread_mutex_unlock(&send_mutex);
  pthread_cleanup_pop(0); //send_mutex

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_testcancel();

  if (!sent)
  {
    unsigned long nSequence = e->m_nSequence;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    gLog.Warn("%sError sending event (#%ld):\n%s%s.\n", L_WARNxSTR,
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
    unsigned long nDestinationUin = e->m_nDestinationUin;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    int socket = d->ConnectToUser(nDestinationUin);
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
    unsigned long nSequence = e->m_nSequence;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    gLog.Warn("%sSocket %d does not exist (#%ld).\n", L_WARNxSTR, socket,
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
    /* FIXME ugly hack, but prevents event from being deleted while we still
       need it and cannot be canceled */
    pthread_mutex_lock(&d->mutex_runningevents);

    // check to make sure we were not cancelled already
    pthread_cleanup_push(cleanup_mutex, &d->mutex_runningevents);
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

      //if we get here  then we haven't been cancelled and we won't be
      //as long as we hold mutex_runningevents

      buf = e->m_pPacket->Finalize(s);

      pthread_mutex_unlock(&d->mutex_runningevents);
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
    unsigned long nSequence = e->m_nSequence;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    gLog.Warn("%sError sending event (#%ld):\n%s%s.\n", L_WARNxSTR,
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

  gLicqDaemon->ReverseConnectToUser(p->nUin, p->nIp, p->nPort, p->nVersion,
    p->nFailedPort);

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
  int nSocketsAvailable, nCurrentSocket, l;
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

        // Message from the server ---------------------------------------------
        else if (nCurrentSocket == d->m_nTCPSrvSocketDesc)
        {
          DEBUG_THREADS("[MonitorSockets_tep] Data on TCP server socket.\n");
          SrvSocket *srvTCP = static_cast<SrvSocket*>(gSocketManager.FetchSocket(nCurrentSocket));
          if (srvTCP == NULL)
          {
            gLog.Warn("%sInvalid server socket in set.\n", L_WARNxSTR);
            close(nCurrentSocket);
          }
          // DAW FIXME error handling when socket is closed..
          else if (srvTCP->Recv())
          {
            CBuffer packet(srvTCP->RecvBuffer());
            srvTCP->ClearRecvBuffer();
            gSocketManager.DropSocket(srvTCP);
            if (!d->ProcessSrvPacket(packet));// d->icqRelogon();
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

        // Connection on the server port ---------------------------------------
        else if (nCurrentSocket == d->m_nTCPSocketDesc)
        {
          DEBUG_THREADS("[MonitorSockets_tep] Data on listening TCP socket.\n");
          TCPSocket *tcp = (TCPSocket *)gSocketManager.FetchSocket(nCurrentSocket);
          if (tcp == NULL)
          {
            gLog.Warn("%sInvalid server TCP socket in set.\n", L_WARNxSTR);
            close(nCurrentSocket);
          }
          else
          {
            TCPSocket *newSocket = new TCPSocket(0);
            tcp->RecvConnection(*newSocket);
            gSocketManager.DropSocket(tcp);
            gSocketManager.AddSocket(newSocket);
            gSocketManager.DropSocket(newSocket);
          }
        }

        // Message from connected socket----------------------------------------
        else
        {
          DEBUG_THREADS("[MonitorSockets_tep] Data on TCP user socket.\n");

          ssl_recv:

          TCPSocket *tcp = (TCPSocket *)gSocketManager.FetchSocket(nCurrentSocket);

          // If tcp is NULL then the socket is no longer in the set, hence it
          // must have been closed by us and we can ignore it.
          if (tcp == NULL)
            goto socket_done;

          if (!tcp->RecvPacket())
          {
            int err = tcp->Error();
            if (err == 0)
              gLog.Info("%sConnection to %ld was closed.\n", L_TCPxSTR, tcp->Owner());
            else
            {
              char buf[128];
              gLog.Info("%sConnection to %ld lost:\n%s%s.\n", L_TCPxSTR, tcp->Owner(),
                        L_BLANKxSTR, tcp->ErrorStr(buf, 128));
            }
            gSocketManager.DropSocket(tcp);
            gSocketManager.CloseSocket(nCurrentSocket);
            d->FailEvents(nCurrentSocket, err);
            /*
            // Go through all running events and fail all from this socket
            ICQEvent *e = NULL;
            do
            {
              e = NULL;
              pthread_mutex_lock(&d->mutex_runningevents);
              list<ICQEvent *>::iterator iter;
              for (iter = d->m_lxRunningEvents.begin(); iter != d->m_lxRunningEvents.end(); iter++)
              {
                if ((*iter)->m_nSocketDesc == nCurrentSocket)
                {
                  e = *iter;
                  break;
                }
              }
              pthread_mutex_unlock(&d->mutex_runningevents);
              if (e != NULL && d->DoneEvent(e, EVENT_ERROR) != NULL)
              {
                // If the connection was reset, we can try again
                if (err == ECONNRESET)
                {
                  e->m_nSocketDesc = -1;
                  d->SendExpectEvent(e, &ProcessRunningEvent_Client_tep);
                }
                else
                {
                  d->ProcessDoneEvent(e);
                }
              }
            } while (e != NULL);
            */

            //goto socket_done;
            break;
          }

          // Save the bytes pending status of the socket
          bool bPending = tcp->SSL_Pending();
          bool r = true;

          // Process the packet if the buffer is full
          if (tcp->RecvBufferFull())
          {
            if (tcp->Owner() == 0)
              r = d->ProcessTcpHandshake(tcp);
            else
              r = d->ProcessTcpPacket(tcp);
            tcp->ClearRecvBuffer();
          }

          // Kill the socket if there was a problem
          if (!r)
          {
            gLog.Info("%sClosing connection to %ld.\n", L_TCPxSTR, tcp->Owner());
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
  gLog.Info("%sShutting down daemon.\n", L_ENDxSTR);

  // Send shutdown signal to all the plugins
  d->licq->ShutdownPlugins();

  // Cancel the monitor sockets thread (deferred until ready)
  write(d->pipe_newsocket[PIPE_WRITE], "X", 1);

  // Cancel the ping thread
  pthread_cancel(d->thread_ping);

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


#endif
