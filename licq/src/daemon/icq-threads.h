#ifndef LICQTHREADS_H
#define LICQTHREADS_H


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
void *ProcessRunningEvent_tep(void *p)
{
  pthread_detach(pthread_self());

  DEBUG_THREADS("[ProcessRunningEvent_tep] Caught event.\n");

  ICQEvent *e = (ICQEvent *)p;
  CICQDaemon *d = e->m_xDaemon;
  struct timeval tv;

  // Check if the socket is connected
  if (e->m_nSocketDesc == -1)
  {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    switch (e->m_eConnect)
    {
    case CONNECT_SERVER:
      e->m_nSocketDesc = d->ConnectToServer();
      break;
    case CONNECT_USER:
      e->m_nSocketDesc = d->ConnectToUser(e->m_nDestinationUin);
      if (e->m_nSocketDesc != -1)
      {
        // Set the local port in the tcp packet now
        INetSocket *s = gSocketManager.FetchSocket(e->m_nSocketDesc);
        if (s == NULL) break;
        ((CPacketTcp *)e->m_xPacket)->LocalPortOffset()[0] = s->LocalPort() & 0xFF;
        ((CPacketTcp *)e->m_xPacket)->LocalPortOffset()[1] = (s->LocalPort() >> 8) & 0xFF;
        gSocketManager.DropSocket(s);
      }
      break;
    case CONNECT_NONE:
      break;
    default:
      gLog.Error("%sInternal error: ProcessRunningEvent_tep(): invalid connect type %d.\n",
                 L_ERRORxSTR, e->m_eConnect);
      break;
    }
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
  if (e->m_nSocketDesc == d->m_nUDPSocketDesc)
  {
    pthread_cleanup_push( (void (*)(void *))&pthread_mutex_unlock, &d->mutex_serverack);
    pthread_mutex_lock(&d->mutex_serverack);
    while (e->m_nSequence > (unsigned short)(d->m_nServerAck + 1))
    {
      pthread_cond_wait(&d->cond_serverack, &d->mutex_serverack);
    }
    pthread_cleanup_pop(1);
  }
#endif


  // Start sending the event
  for (int i = 0; i <= MAX_SERVER_RETRIES; i++)
  {
    if (i > 0)
      gLog.Info("%sTimed out after %d seconds (#%d), retry %d of %d...\n",
                L_WARNxSTR, MAX_WAIT_ACK, e->m_nSequence, i, MAX_SERVER_RETRIES);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    INetSocket *s = gSocketManager.FetchSocket(e->m_nSocketDesc);
    if (s == NULL)
    {
      gLog.Warn("%sSocked not connected or invalid (#%d).\n", L_WARNxSTR, e->m_nSequence);
      if (d->DoneEvent(e, EVENT_ERROR) != NULL) d->ProcessDoneEvent(e);
      pthread_exit(NULL);
    }
    if (!s->Send(e->m_xPacket->getBuffer()))
    {
      char szErrorBuf[128];
      gLog.Warn("%sError sending event (#%d):\n%s%s.\n", L_WARNxSTR,
                e->m_nSequence, L_BLANKxSTR, s->ErrorStr(szErrorBuf, 128));
      // We don't close the socket as it should be closed by the server thread
      gSocketManager.DropSocket(s);
      if (d->DoneEvent(e, EVENT_ERROR) != NULL) d->ProcessDoneEvent(e);
      pthread_exit(NULL);
    }
    gSocketManager.DropSocket(s);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();

    // If this is a tcp connection, then we don't do retries
    if (e->m_nCommand == ICQ_CMDxTCP_START) pthread_exit(NULL);

    // Otherwise sleep a bit before retrying
    // We use select because sleep() awakes on every signal, including all
    // the thread ones
    tv.tv_sec = MAX_WAIT_ACK;
    tv.tv_usec = 0;
    select(0, NULL, NULL, NULL, &tv);
    //sleep (MAX_WAIT_ACK);
    pthread_testcancel();
  }

  // We timed out
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  gLog.Warn("%sTimed out (#%d).\n", L_WARNxSTR, e->m_nSequence);
  if (d->DoneEvent(e, EVENT_TIMEDOUT) != NULL) d->ProcessDoneEvent(e);
  pthread_exit(NULL);
  // Avoid compiler warnings
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
    tv.tv_sec = PING_FREQUENCY;
    tv.tv_usec = 0;
    select(0, NULL, NULL, NULL, &tv);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
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
  char buf[128];
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
          fgets(buf, 128, d->fifo_fs);
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
            CBuffer b(udp->RecvBuffer());
            udp->ClearRecvBuffer();
            gSocketManager.DropSocket(udp);
            d->ProcessUdpPacket(b);
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
              if (tcp->Error() == 0)
                gLog.Info("%sConnection to %d closed.\n", L_TCPxSTR, tcp->Owner());
              else
              {
                char buf[128];
                gLog.Info("%sConnection to %d lost:\n%s%s.\n", L_TCPxSTR, tcp->Owner(),
                          L_BLANKxSTR, tcp->ErrorStr(buf, 128));
              }
              ICQUser *u = gUserManager.FetchUser(tcp->Owner(), LOCK_W);
              if (u != NULL)
              {
                u->SetSocketDesc(-1);
                gUserManager.DropUser(u);
              }
              gSocketManager.DropSocket(tcp);
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
                  d->ProcessDoneEvent(e);
              } while (e != NULL);

              gSocketManager.CloseSocket(nCurrentSocket);
              break;
            }
            if (tcp->RecvBufferFull())
            {
              if (tcp->Owner() == 0)
              {
                if (!d->ProcessTcpHandshake(tcp))
                {
                  gSocketManager.DropSocket(tcp);
                  gSocketManager.CloseSocket(nCurrentSocket, false);
                }
                else
                {
                  tcp->ClearRecvBuffer();
                  gSocketManager.DropSocket(tcp);
                }
              }
              else
              {
                CBuffer b(tcp->RecvBuffer());
                tcp->ClearRecvBuffer();
                gSocketManager.DropSocket(tcp);
                if (!d->ProcessTcpPacket(b, nCurrentSocket))
                  gSocketManager.CloseSocket(nCurrentSocket, false);
              }
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

  // Signal that we are shutdown
  pthread_mutex_lock(&LP_IdMutex);
  LP_Ids.push_back(0);
  pthread_mutex_unlock(&LP_IdMutex);
  pthread_cond_signal(&LP_IdSignal);

  return NULL;
}


#endif
