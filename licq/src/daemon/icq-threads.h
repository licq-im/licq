#ifndef LICQTHREADS_H
#define LICQTHREADS_H


#define DEBUG_THREADS(x)
//#define DEBUG_THREADS(x) printf(x)

struct SRunEventThreadData
{
  CICQDaemon *d;
  ICQEvent *e;
};


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

  // Finalize the creation of the packet
  e->m_xPacket->Create();

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


#if 0
/*------------------------------------------------------------------------------
 * ProcessPendingEvents_tep
 *
 * One thread runs here waiting for events to become pending, then it spawns
 * another thread and passes it the new event for sending.
 *----------------------------------------------------------------------------*/
void *ProcessPendingEvents_tep(void *p)
{
  //pthread_detach(pthread_self());

  CICQDaemon *d = (CICQDaemon *)p;

  ICQEvent *e = NULL;
  while (true)
  {
    pthread_mutex_lock(&d->mutex_pendingevents);
    while (d->m_lxPendingEvents.size() == 0)
      pthread_cond_wait(&d->cond_pendingevents, &d->mutex_pendingevents);
    DEBUG_THREADS("[ProcessPendingEvents_tep] Caught event.\n");
    e = d->m_lxPendingEvents.front();
    d->m_lxPendingEvents.pop_front();
    pthread_mutex_unlock(&d->mutex_pendingevents);
    if (e == NULL)
    {
      DEBUG_THREADS("[ProcessPendingEvents_tep] NULL event, exiting.\n");
      pthread_exit(NULL);
    }

    pthread_mutex_lock(&d->mutex_runningevents);
    d->m_lxRunningEvents.push_back(e);
    pthread_mutex_unlock(&d->mutex_runningevents);

    struct SRunEventThreadData *s = new SRunEventThreadData;
    s->d = d;
    s->e = e;
    DEBUG_THREADS("[ProcessPendingEvents_tep] Throwing running event.\n");
    int nResult = pthread_create(&e->thread_send, NULL, &ProcessRunningEvent_tep, s);
    if (nResult != 0)
    {
      gLog.Warn("%sUnable to start event thread (#%d):\n%s%s.\n", L_ERRORxSTR,
                e->m_nSequence, L_BLANKxSTR, strerror(nResult));
      e->m_eResult = EVENT_ERROR;
      d->PushDoneEvent(e);
      delete s;
    }
  }
}
#endif

#if 0
/*------------------------------------------------------------------------------
 * ProcessDoneEvent_tep
 *
 * One thread runs here waiting for events to be done, then it processes
 * them and possibly passes the result to the gui.
 *----------------------------------------------------------------------------*/
void *ProcessDoneEvents_tep(void *p)
{
  //pthread_detach(pthread_self());

  CICQDaemon *d = (CICQDaemon *)p;
  //bool bLogoff;

  ICQEvent *e = NULL;
  while (true)
  {
    pthread_mutex_lock(&d->mutex_doneevents);
    while (d->m_qxDoneEvents.size() == 0)
      pthread_cond_wait(&d->cond_doneevents, &d->mutex_doneevents);
    DEBUG_THREADS("[ProcessDoneEvents_tep] Caught event.\n");
    e = d->m_qxDoneEvents.front();
    d->m_qxDoneEvents.pop_front();
    pthread_mutex_unlock(&d->mutex_doneevents);
    if (e == NULL)
    {
      DEBUG_THREADS("[ProcessDoneEvents_tep] NULL event, exiting.\n");
      pthread_exit(NULL);
    }

    // Determine this now as later we might have deleted the event
    unsigned short nCommand = e->m_nCommand;
    EEventResult eResult = e->m_eResult;
/*    bLogoff = (   eResult != EVENT_ACKED
               && eResult != EVENT_SUCCESS
               && eResult != EVENT_CANCELLED
               && nCommand != ICQ_CMDxTCP_START
               && nCommand != ICQ_CMDxSND_LOGON );*/

    // Write the event to the history file if appropriate
    if (e->m_xUserEvent != NULL)
    {
      ICQUser *u = gUserManager.FetchUser(e->m_nDestinationUin, LOCK_W);
      if (u != NULL)
      {
        e->m_xUserEvent->AddToHistory(u, D_SENDER);
        gUserManager.DropUser(u);
      }
    }

    // Process the event
    switch (e->m_nCommand)
    {
    // Regular events
    case ICQ_CMDxSND_SETxSTATUS:
    case ICQ_CMDxTCP_START:
    case ICQ_CMDxSND_THRUxSERVER:
    case ICQ_CMDxSND_PING:
    case ICQ_CMDxSND_USERxADD:
    case ICQ_CMDxSND_USERxLIST:
    case ICQ_CMDxSND_SYSxMSGxREQ:
    case ICQ_CMDxSND_SYSxMSGxDONExACK:
    case ICQ_CMDxSND_AUTHORIZE:
    case ICQ_CMDxSND_VISIBLExLIST:
    case ICQ_CMDxSND_INVISIBLExLIST:
      d->PushPluginEvent(e);
      break;

    // Extended events
    case ICQ_CMDxSND_LOGON:
    case ICQ_CMDxSND_USERxGETINFO:
    case ICQ_CMDxSND_USERxGETDETAILS:
    case ICQ_CMDxSND_UPDATExDETAIL:
    case ICQ_CMDxSND_UPDATExBASIC:
    case ICQ_CMDxSND_SEARCHxSTART:
    case ICQ_CMDxSND_REGISTERxUSER:
      switch (e->m_eResult)
      {
      case EVENT_ERROR:
      case EVENT_TIMEDOUT:
      case EVENT_FAILED:
      case EVENT_SUCCESS:
        d->PushPluginEvent(e);
        break;
      case EVENT_ACKED:  // push to extended event list
        d->PushExtendedEvent(e);
        break;
      default:
        gLog.Error("%sInternal error: ProcessDoneEvents_tep(): Invalid result for extended event (%d).\n",
                   L_ERRORxSTR, e->m_eResult);
        delete e;
        continue;
      }
      break;

    default:
      gLog.Error("%sInternal error: ProcessDoneEvents_tep(): Unknown command (%04X).\n",
                 L_ERRORxSTR, e->m_nCommand);
      delete e;
      continue;
    }

    // Some special commands to deal with
    if (nCommand == ICQ_CMDxSND_SETxSTATUS && eResult == EVENT_ACKED)
    {
      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
      d->ChangeUserStatus(o, d->m_nDesiredStatus);
      gUserManager.DropOwner();
    }

    // Logoff if server timed out in anything except first logon packet
    /*if (bLogoff)
    {
      if (d->m_eStatus != STATUS_OFFLINE_FORCED) d->icqLogoff(true);
      d->m_eStatus = STATUS_OFFLINE_FORCED;
    }*/
  }
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
  gLog.Warn("%sShutting down daemon.\n", L_ENDxSTR);

  // Send shutdown signal to all the plugins
  vector<CPlugin *>::iterator iter;
  for (iter = d->m_vPlugins.begin(); iter != d->m_vPlugins.end(); iter++)
  {
    (*iter)->Shutdown();
  }

  // Cancel the monitor sockets thread (deferred until ready)
  //pthread_cancel(d->thread_monitorsockets);
  write(d->pipe_newsocket[PIPE_WRITE], "X", 1);

  // Send a NULL event to the pending events thread to cancel it
  /*pthread_mutex_lock(&d->mutex_pendingevents);
  d->m_lxPendingEvents.push_back(NULL);
  pthread_mutex_unlock(&d->mutex_pendingevents);
  DEBUG_THREADS("[Shutdown_tep] Throwing NULL pending event.\n");
  pthread_cond_signal(&d->cond_pendingevents);*/

  // Cancel the ping thread
  pthread_cancel(d->thread_ping);

  // Push a NULL event onto the done queue to cancel that thread
  /*DEBUG_THREADS("[Shutdown_tep] Throwing NULL done event.\n");
  d->PushDoneEvent(NULL);*/

  // Join our threads
  pthread_join(d->thread_monitorsockets, NULL);
  //pthread_join(d->thread_ping, NULL);
  /*pthread_join(d->thread_pendingevents, NULL);
  pthread_join(d->thread_doneevents, NULL);*/

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
