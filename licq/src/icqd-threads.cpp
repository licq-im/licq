#ifndef LICQTHREADS_H
#define LICQTHREADS_H

#include <errno.h>

#include "pthread_rdwr.h"
#include "time-fix.h"

#include "licq_icqd.h"
#include "licq_log.h"
#include "licq_packets.h"
#include "plugind.h"

#define DEBUG_THREADS(x)
//#define DEBUG_THREADS(x) printf(x)



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
  //static time_t s_nTimeLastSent = LONG_MAX;

  pthread_detach(pthread_self());

  DEBUG_THREADS("[ProcessRunningEvent_Server_tep] Caught event.\n");

  ICQEvent *e = (ICQEvent *)p;
  CICQDaemon *d = e->m_pDaemon;
  struct timeval tv;

  // Check if the socket is connected
  if (e->m_nSocketDesc == -1)
  {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    // Connect to the server if we are logging on
    if (e->m_pPacket->Command() == ICQ_CMDxSND_LOGON ||
        e->m_pPacket->Command() == ICQ_CMDxSND_REGISTERxUSER)
      e->m_nSocketDesc = d->ConnectToServer();

    // Check again, if still -1, fail the event
    if (e->m_nSocketDesc == -1)
    {
      if (d->DoneEvent(e, EVENT_ERROR) != NULL) d->ProcessDoneEvent(e);
      pthread_exit(NULL);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
  }

#if ICQ_VERSION == 5
  // Make sure we are the next ack in line
  pthread_cleanup_push( (void (*)(void *))&pthread_mutex_unlock, &d->mutex_serverack);
  pthread_mutex_lock(&d->mutex_serverack);
  while (e->m_nSequence > (unsigned short)(d->m_nServerAck + 1))
  {
    pthread_cond_wait(&d->cond_serverack, &d->mutex_serverack);
  }
  pthread_cleanup_pop(1);
#endif

  // Check that we aren't sending too fast
  /*
  while (time(NULL) <= s_nTimeLastSent + MIN_SEND_DELAY)
  {
    struct timeval tv = { MIN_SEND_DELAY, 0 };
    select(0, NULL, NULL, NULL, &tv);
  }
  */

  // Start sending the event
  for (int i = 0; i <= MAX_SERVER_RETRIES; i++)
  {
    if (i > 0)
      gLog.Info("%sTimed out after %d seconds (#%ld), retry %d of %d...\n",
                L_WARNxSTR, MAX_WAIT_ACK, e->m_nSequence, i, MAX_SERVER_RETRIES);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    INetSocket *s = gSocketManager.FetchSocket(e->m_nSocketDesc);
    if (s == NULL)
    {
      gLog.Warn("%sSocket not connected or invalid (#%ld).\n", L_WARNxSTR, e->m_nSequence);
      if (d->DoneEvent(e, EVENT_ERROR) != NULL) d->ProcessDoneEvent(e);
      pthread_exit(NULL);
    }
    CBuffer *buf = e->m_pPacket->Finalize(s);
    // We should delay here if the last packet was sent within the previous second
    // FIXME
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
    delete buf;
    gSocketManager.DropSocket(s);
    //s_nTimeLastSent = time(NULL);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();

    // Sleep a bit before retrying
    // We use select because sleep() awakes on every signal, including all
    // the thread ones
    tv.tv_sec = MAX_WAIT_ACK;
    tv.tv_usec = 0;
    select(0, NULL, NULL, NULL, &tv);
    pthread_testcancel();
  }

  // We timed out
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  gLog.Warn("%sTimed out (#%ld).\n", L_WARNxSTR, e->m_nSequence);
  if (d->DoneEvent(e, EVENT_TIMEDOUT) != NULL) d->ProcessDoneEvent(e);
  pthread_exit(NULL);
  // Avoid compiler warnings
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
#ifdef USE_OPENSSL
  // Check if this was a key request
  if (e->m_pPacket->SubCommand() == ICQ_CMDxSUB_SECURExOPEN)
  {
    if (s->DHKey() != NULL)
    {
      gLog.Warn("%sSent key request to %ld when channel already secure.\n",
       L_WARNxSTR, e->m_nDestinationUin);
      s->ClearDHKey();
    }
    s->SetDHKey( ((CPT_OpenSecureChannel *)e->m_pPacket)->GrabDHKey() );
  }
#endif
  gSocketManager.DropSocket(s);

  pthread_exit(NULL);
  // Avoid compiler warnings
  return NULL;
}


#if 0
/*------------------------------------------------------------------------------
 * ReverseConnectToUser
 *
 * Creates a new TCPSocket and connects it to a given user.  Adds the socket
 * to the global socket manager and to the user.
 *----------------------------------------------------------------------------*/
class CReverseConnectToUserData
{
public:
  CReverseConnectToUserData(CICQDaemon *d, unsigned long uin,
                            unsigned long ip, unsigned short port)
    : xDaemon(d), nUin(uin), nIp(ip), nPort(port) {}

  CICQDaemon *xDaemon;
  unsigned long nUin;
  unsigned long nIp;
  unsigned short nPort;
}

void *ReverseConnectToUser_tep(void *v)
{
  pthread_detach(pthread_self());

  DEBUG_THREADS("[ReverseConnectToUser_tep] Caught event.\n");

  struct SReverseConnectToUserData *p = (struct SReverseConnectToUserData *)v;
  CICQDaemon *d = p->xDaemon;
  unsigned long nUin = p->nUIn;
  unsigned long nIp = p->nIp;
  unsigned short nPort = p->nPort;
  delete p;

  TCPSocket *s = new TCPSocket(nUin);

  gLog.Info("%sReverse connecting to %ld on port %d.\n", L_TCPxSTR, nUin, nPort);

  // If we fail to set the remote address, the ip must be 0
  s->SetRemoteAddr(nIp, nPort);

  if (!s->OpenConnection())
  {
    char buf[128];
    gLog.Warn("%sReverse connect to %ld failed:\n%s%s.\n", L_WARNxSTR,
              nUin, L_BLANKxSTR, s->ErrorStr(buf, 128));
    return -1;
  }

  gLog.Info("%sReverse shaking hands with %ld.\n", L_TCPxSTR, nUin);
  CPacketTcp_Handshake p(s->LocalPort());
  if (!s->SendPacket(p.getBuffer()))
  {
    char buf[128];
    gLog.Warn("%sReverse handshake failed:\n%s%s.\n", L_WARNxSTR, L_BLANKxSTR, s->ErrorStr(buf, 128));
    delete s;
    return -1;
  }

  // Add the new socket to the socket manager
  gSocketManager.AddSocket(s);
  int nSD = s->Descriptor();
  gSocketManager.DropSocket(s);

  // Set the socket descriptor in the user if this user is on our list
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u != NULL)
  {
    u->SetSocketDesc(nSD, nPort);
    gUserManager.DropUser(u);
  }

  // Alert the select thread that there is a new socket
  write(d->pipe_newsocket[PIPE_WRITE], "S", 1);

  return nSD;
}
#endif




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
  TCPSocket *tcp;
  UDPSocket *udp;

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
        else if (nCurrentSocket == d->m_nUDPSocketDesc)
        {
          DEBUG_THREADS("[MonitorSockets_tep] Data on UDP socket.\n");
          udp = (UDPSocket *)gSocketManager.FetchSocket(nCurrentSocket);
          if (udp == NULL)
          {
            gLog.Warn("%sInvalid UDP socket in set.\n", L_WARNxSTR);
            close(nCurrentSocket);
          }
          else
          {
            udp->RecvRaw();
            bool r = d->ProcessUdpPacket(udp);
            udp->ClearRecvBuffer();
            gSocketManager.DropSocket(udp);
            if (!r) d->icqRelogon();
          }
        }

        // Connection on the server port ---------------------------------------
        else if (nCurrentSocket == d->m_nTCPSocketDesc)
        {
          DEBUG_THREADS("[MonitorSockets_tep] Data on TCP server socket.\n");
          tcp = (TCPSocket *)gSocketManager.FetchSocket(nCurrentSocket);
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
          tcp = (TCPSocket *)gSocketManager.FetchSocket(nCurrentSocket);
          // If tcp is NULL then the socket is no longer in the set, hence it
          // must have been closed by us and we can ignore it.
          if (tcp == NULL)
          {
            //gLog.Warn("%sInvalid user TCP socket in set.\n", L_WARNxSTR);
            //close(nCurrentSocket);
          }
          else
          {
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

              break;
            }
            if (tcp->RecvBufferFull())
            {
              bool r = false;
              if (tcp->Owner() == 0)
                r = d->ProcessTcpHandshake(tcp);
              else
                r = d->ProcessTcpPacket(tcp);
              tcp->ClearRecvBuffer();
              if (!r)
              {
                gLog.Info("%sClosing connection to %ld.\n", L_TCPxSTR, tcp->Owner());
                gSocketManager.DropSocket(tcp);
                gSocketManager.CloseSocket(nCurrentSocket);
              }
              else
                gSocketManager.DropSocket(tcp);
            }
            else
              gSocketManager.DropSocket(tcp);
          }
        }

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
  vector<CPlugin *>::iterator iter;
  pthread_mutex_lock(&d->mutex_plugins);
  for (iter = d->m_vPlugins.begin(); iter != d->m_vPlugins.end(); iter++)
  {
    (*iter)->Shutdown();
  }
  pthread_mutex_unlock(&d->mutex_plugins);

  // Cancel the monitor sockets thread (deferred until ready)
  //pthread_cancel(d->thread_monitorsockets);
  write(d->pipe_newsocket[PIPE_WRITE], "X", 1);

  // Cancel the ping thread
  pthread_cancel(d->thread_ping);

  // Join our threads
  pthread_join(d->thread_monitorsockets, NULL);

  gSocketManager.CloseSocket(d->m_nTCPSocketDesc);
  if (d->m_nUDPSocketDesc != -1) d->icqLogoff();

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
