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

/*------------------------------------------------------------------------------
 * ProcessRunningEvent_tep
 *
 * Thread entry point to run an event.  First checks to see if the socket for
 * the given event needs to be connected and calls the relevant connection
 * function.  Then sends the event, retrying after a timeout.  If an ack is
 * received, the thread will be cancelled by the receiving thread.
 *----------------------------------------------------------------------------*/
void *ProcessRunningEvent_Server_tep(void *p)
{
  pthread_detach(pthread_self());

  DEBUG_THREADS("[ProcessRunningEvent_Server_tep] Caught event.\n");

  ICQEvent *e = (ICQEvent *)p;
  CICQDaemon *d = e->m_pDaemon;
  //struct timeval tv;

  // Check if the socket is connected
  if (e->m_nSocketDesc == -1)
  {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    // Connect to the server if we are logging on
    if (e->m_pPacket->Command() == ICQ_CHNxNEW)
    {
      gLog.Info("%sConnecting to login server.\n", L_SRVxSTR);
      e->m_nSocketDesc = d->ConnectToLoginServer();
    }

    // Check again, if still -1, fail the event
    if (e->m_nSocketDesc == -1)
    {
        gLog.Info("%sConnecting to login server failed, failing event", L_SRVxSTR);
      if (d->DoneEvent(e, EVENT_ERROR) != NULL) d->ProcessDoneEvent(e);
      pthread_exit(NULL);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
  }

// Start sending the event
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  INetSocket *s = gSocketManager.FetchSocket(e->m_nSocketDesc);
  if (s == NULL)
  {
    gLog.Warn("%sSocket not connected or invalid (#%ld).\n", L_WARNxSTR, e->m_nSequence);
    if (d->DoneEvent(e, EVENT_ERROR) != NULL) d->ProcessDoneEvent(e);
    pthread_exit(NULL);
  }


//    tv.tv_sec = 1;
//    tv.tv_usec = 100000;
//    select(0, NULL, NULL, NULL, &tv);

  CBuffer *buf = e->m_pPacket->Finalize(NULL);
  if (!s->Send(buf))
  {
    delete buf;
    char szErrorBuf[128];
    gLog.Warn("%sError sending event (#%ld):\n%s%s.\n", L_WARNxSTR,
              e->m_nSequence, L_BLANKxSTR, s->ErrorStr(szErrorBuf, 128));
    // We don't close the socket as it should be closed by the server thread
    gSocketManager.DropSocket(s);
    if (d->DoneEvent(e, EVENT_ERROR) != NULL) d->ProcessDoneEvent(e);
    pthread_exit(NULL);
  }
  else if (e->m_NoAck) {
    // send successfully and we don't get an answer from the server
    if (d->DoneEvent(e, EVENT_ACKED) != NULL) d->ProcessDoneEvent(e);
  }
  delete buf;
  gSocketManager.DropSocket(s);


  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_testcancel();

  return NULL;
}


void *ProcessRunningEvent_Client_tep(void *p)
{
  pthread_detach(pthread_self());

  DEBUG_THREADS("[ProcessRunningEvent_Client_tep] Caught event.\n");

  ICQEvent *e = (ICQEvent *)p;
  CICQDaemon *d = e->m_pDaemon;

  // Check if the socket is connected
  if (e->m_nSocketDesc == -1)
  {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    e->m_nSocketDesc = d->ConnectToUser(e->m_nDestinationUin);
    // Check again, if still -1, fail the event
    if (e->m_nSocketDesc == -1)
    {
      if (d->DoneEvent(e, EVENT_ERROR) != NULL) d->ProcessDoneEvent(e);
      pthread_exit(NULL);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
  }

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  INetSocket *s = gSocketManager.FetchSocket(e->m_nSocketDesc);
  if (s == NULL)
  {
    gLog.Warn("%sSocket %d does not exist (#%ld).\n", L_WARNxSTR, e->m_nSocketDesc,
       e->m_nSequence);
    if (d->DoneEvent(e, EVENT_ERROR) != NULL) d->ProcessDoneEvent(e);
    pthread_exit(NULL);
  }
  CBuffer *buf = e->m_pPacket->Finalize(s);
  if (!s->Send(buf))
  {
    char szErrorBuf[128];
    gLog.Warn("%sError sending event (#%ld):\n%s%s.\n", L_WARNxSTR,
     -e->m_nSequence, L_BLANKxSTR, s->ErrorStr(szErrorBuf, 128));
    // Close the socket, alert the socket thread
    gSocketManager.DropSocket(s);
    gSocketManager.CloseSocket(e->m_nSocketDesc);
    write(d->pipe_newsocket[PIPE_WRITE], "S", 1);
    // Kill the event, do after the above as ProcessDoneEvent erase the event
    if (d->DoneEvent(e, EVENT_ERROR) != NULL) d->ProcessDoneEvent(e);
    pthread_exit(NULL);
  }
  gSocketManager.DropSocket(s);

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
            d->m_eStatus = STATUS_OFFLINE_FORCED;
            d->m_bLoggingOn = false;
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
