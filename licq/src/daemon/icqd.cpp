#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include "time-fix.h"

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#include "log.h"
#include "translate.h"
#include "plugind.h"
#include "icqpacket.h"
#include "licq.h"

#include "icqd.h"
#include "icq-udp.h"
#include "icq-tcp.h"
#include "icq-threads.h"

//-----CICQDaemon::constructor--------------------------------------------------
CICQDaemon::CICQDaemon(CLicq *_licq) : m_vbTcpPorts(10)
{
  char szFilename[MAX_FILENAME_LEN];

  licq = _licq;

  // Initialise the data values
  m_nAllowUpdateUsers = 0;
  m_nUDPSocketDesc = -1;
  m_nTCPSocketDesc = -1;
  m_eStatus = STATUS_OFFLINE_MANUAL;

  // Begin parsing the config file
  sprintf(szFilename, "%s/%s", BASE_DIR, "licq.conf");
  CIniFile licqConf(INI_FxERROR | INI_FxFATAL);
  licqConf.LoadFile(szFilename);
  licqConf.ClearFlag(INI_FxFATAL | INI_FxERROR);
  licqConf.SetFlag(INI_FxWARN);

  // -----Network configuration-----
  gLog.Info("%sNetwork configuration.\n", L_INITxSTR);

  unsigned short numRemoteServers, remoteServerPort;
  char remoteServerID[32], remotePortID[32], remoteServerName[64];

  licqConf.SetSection("network");
  licqConf.ReadNum("NumOfServers", numRemoteServers, 1);

  // Check for a default server port - if it's not there, we set it to the internal default
  licqConf.ReadNum("DefaultServerPort", m_nDefaultRemotePort, DEFAULT_SERVER_PORT);

  // read in all the servers
  for(int i = 0; i < numRemoteServers; i++)
  {
     sprintf(remoteServerID, "Server%d", i + 1);
     sprintf(remotePortID, "ServerPort%d", i + 1);
     if (!licqConf.ReadStr(remoteServerID, remoteServerName)) continue;
     licqConf.ClearFlag(INI_FxWARN);
     licqConf.ReadNum(remotePortID, remoteServerPort, getDefaultRemotePort());
     licqConf.SetFlag(INI_FxWARN);
     icqServers.addServer(remoteServerName, remoteServerPort);
  }

  licqConf.ReadNum("TCPServerPort", m_nTcpServerPort, 0);
  licqConf.ReadNum("MaxUsersPerPacket", m_nMaxUsersPerPacket, 100);
  licqConf.ReadBool("AllowNewUsers", m_bAllowNewUsers, true);

  // Error log file
  licqConf.ReadStr("Errors", szFilename, "none");
  if (strcmp(szFilename, "none") != 0)
  {
    char errorFileNameFull[256];
    sprintf(errorFileNameFull, "%s/%s", BASE_DIR, szFilename);
    CLogService_File *l = new CLogService_File(L_ERROR | L_UNKNOWN);
    if (!l->SetLogFile(errorFileNameFull, "a"))
    {
      gLog.Error("%sUnable to open %s as error log:\n%s%s.\n",
                  L_ERRORxSTR, errorFileNameFull, L_BLANKxSTR, strerror(errno));
      delete l;
    }
    else
      gLog.AddService(l);
  }

  // Loading translation table from file
  licqConf.ReadStr("Translation", szFilename, "none");
  if (strncmp(szFilename, "none", 4) != 0)
  {
     char TranslationTableFileNameFull[MAX_FILENAME_LEN];
     sprintf(TranslationTableFileNameFull, "%s%s/%s", SHARE_DIR, TRANSLATION_DIR, szFilename);
     gTranslator.setTranslationMap (TranslationTableFileNameFull);
  }

  // Url viewer
  m_szUrlViewer = NULL;
  licqConf.ReadStr("UrlViewer", szFilename, "none");
  m_szUrlViewer = new char[strlen(szFilename) + 1];
  strcpy(m_szUrlViewer, szFilename);

  // Terminal
  m_szTerminal = NULL;
  licqConf.ReadStr("Terminal", szFilename, "xterm -T Licq -e ");
  m_szTerminal = new char[strlen(szFilename) + 1];
  strcpy(m_szTerminal, szFilename);

  // -----OnEvent configuration-----
  char szOnEventCommand[MAX_FILENAME_LEN], *szOnParams[MAX_ON_EVENT];
  unsigned short nOnEventCmdType;

  gLog.Info("%sOnEvent configuration.\n", L_INITxSTR);
  licqConf.SetSection("onevent");
  licqConf.ReadNum("Enable", nOnEventCmdType, 0);
  m_xOnEventManager.SetCommandType(nOnEventCmdType);
  for (int i = 0; i < MAX_ON_EVENT; i++)
    szOnParams[i] = new char[MAX_FILENAME_LEN];
  licqConf.ReadStr("Command", szOnEventCommand, "play");
  licqConf.ReadStr("Message", szOnParams[ON_EVENT_MSG], "");
  licqConf.ReadStr("Url", szOnParams[ON_EVENT_URL], "");
  licqConf.ReadStr("Chat", szOnParams[ON_EVENT_CHAT], "");
  licqConf.ReadStr("File", szOnParams[ON_EVENT_FILE], "");
  licqConf.ReadStr("OnlineNotify", szOnParams[ON_EVENT_NOTIFY], "");
  licqConf.ReadStr("SysMsg", szOnParams[ON_EVENT_SYSMSG], "");
  m_xOnEventManager.SetParameters(szOnEventCommand, (const char **)szOnParams);
  for (int i = 0; i < MAX_ON_EVENT; i++)
    delete [] szOnParams[i];

  icqServers.setServer(1);    // set the initial UDP remote server (opened in ConnectToServer)
  for (unsigned short i = 0; i < 10; i++) m_vbTcpPorts[i] = false;

  // Pipes
  gLog.Info("%sCreating pipes.\n", L_INITxSTR);
  pipe(pipe_newsocket);

  // Start up our threads
  gLog.Info("%sInitializing thread data.\n", L_INITxSTR);
  pthread_mutex_init(&mutex_pendingevents, NULL);
  pthread_cond_init(&cond_pendingevents, NULL);
  pthread_mutex_init(&mutex_runningevents, NULL);
  pthread_mutex_init(&mutex_doneevents, NULL);
  pthread_cond_init(&cond_doneevents, NULL);
  pthread_mutex_init(&mutex_extendedevents, NULL);
  pthread_mutex_init(&mutex_plugins, NULL);
}


int CICQDaemon::Start(void)
{
  char buf[MAX_FILENAME_LEN];
  int nResult = 0;

  gLog.Info("%sStarting TCP server.\n", L_INITxSTR);
  TCPSocket *s = new TCPSocket(0);
  if (!s->StartServer(m_nTcpServerPort))    // start up the TCP server
  {
     gLog.Error("%sUnable to allocate TCP port for local server (%s)!\n",
                L_ERRORxSTR, s->ErrorStr(buf, 128));
     return EXIT_STARTxSERVERxFAIL;
  }
  m_nTCPSocketDesc = s->Descriptor();
  gSocketManager.AddSocket(s);
  gLog.Info("%sTCP server started on %s:%d.\n", L_TCPxSTR, s->LocalIpStr(buf), s->LocalPort());
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->SetIpPort(s->LocalIp(), s->LocalPort());
  gUserManager.DropOwner();
  gSocketManager.DropSocket(s);

#ifdef USE_FIFO
  // Open the fifo
  sprintf(buf, "%s/licq_fifo", BASE_DIR);
  gLog.Info("%sOpening fifo.\n", L_INITxSTR);
  fifo_fd = open(buf, O_RDWR);
  if (fifo_fd == -1)
  {
    if (mkfifo(buf, 00600) == -1)
      gLog.Warn("%sUnable to create fifo:\n%s%s.\n", L_WARNxSTR, L_BLANKxSTR, strerror(errno));
    else
    {
      fifo_fd = open(buf, O_RDWR);
      if (fifo_fd == -1)
        gLog.Warn("%sUnable to open fifo:\n%s%s.\n", L_WARNxSTR, L_BLANKxSTR, strerror(errno));
    }
  }
  fifo_fs = NULL;
  if (fifo_fd != -1) fifo_fs = fdopen(fifo_fd, "r");
#else
  fifo_fs = NULL;
  fifo_fd = -1;
#endif

  gLog.Info("%sSpawning daemon threads.\n", L_INITxSTR);
  nResult = pthread_create(&thread_monitorsockets, NULL, &MonitorSockets_tep, this);
  if (nResult != 0)
  {
    gLog.Error("%sUnable to start socket monitor thread:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
    return EXIT_THREADxFAIL;
  }
  nResult = pthread_create(&thread_ping, NULL, &Ping_tep, this);
  if (nResult != 0)
  {
    gLog.Error("%sUnable to start ping thread:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
    return EXIT_THREADxFAIL;
  }
  nResult = pthread_create(&thread_pendingevents , NULL, &ProcessPendingEvents_tep, this);
  if (nResult != 0)
  {
    gLog.Error("%sUnable to start pending events thread:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
    return EXIT_THREADxFAIL;
  }
  nResult = pthread_create(&thread_doneevents, NULL, &ProcessDoneEvents_tep, this);
  if (nResult != 0)
  {
    gLog.Error("%sUnable to start done events thread:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
    return EXIT_THREADxFAIL;
  }

  //gLog.Info("%sDaemon succesfully started.\n", L_INITxSTR);
  return EXIT_SUCCESS;
}


/*------------------------------------------------------------------------------
 * RegisterPlugin
 *
 * Registers the current thread as a new plugin.  Returns the pipe to listen
 * on for notification.
 *----------------------------------------------------------------------------*/
int CICQDaemon::RegisterPlugin(unsigned long _nSignalMask)
{
  pthread_mutex_lock(&mutex_plugins);
  m_vPlugins.push_back(new CPlugin(_nSignalMask));
  int p = m_vPlugins[m_vPlugins.size() - 1]->Pipe();
  pthread_mutex_unlock(&mutex_plugins);
  return p;
}


/*------------------------------------------------------------------------------
 * UnregisterPlugin
 *
 * Unregisters the current plugin thread.
 *----------------------------------------------------------------------------*/
void CICQDaemon::UnregisterPlugin(void)
{
  vector<CPlugin *>::iterator iter;
  pthread_mutex_lock(&mutex_plugins);
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    if ((*iter)->CompareThread(pthread_self()))
    {
      delete *iter;
      m_vPlugins.erase(iter);
      break;
    }
  }
  pthread_mutex_unlock(&mutex_plugins);
}


const char *CICQDaemon::Version(void)
{
  return licq->Version();
}

//-----ICQ::destructor----------------------------------------------------------
CICQDaemon::~CICQDaemon(void)
{
  if (m_szUrlViewer != NULL) delete [] m_szUrlViewer;
}

pthread_t *CICQDaemon::Shutdown(void)
{
  static pthread_t *thread_shutdown = (pthread_t *)malloc(sizeof(pthread_t));
  static bool bShuttingDown = false;
  if (bShuttingDown) return(thread_shutdown);
  bShuttingDown = true;
  SaveUserList();
  pthread_create (thread_shutdown, NULL, &Shutdown_tep, this);
  return (thread_shutdown);
}


//-----SaveConf-----------------------------------------------------------------
void CICQDaemon::SaveConf(void)
{
  char filename[MAX_FILENAME_LEN];
  sprintf(filename, "%s/licq.conf", BASE_DIR);
  CIniFile licqConf(INI_FxERROR | INI_FxALLOWxCREATE);
  if (!licqConf.LoadFile(filename)) return;

  licqConf.SetSection("network");
  licqConf.WriteNum("DefaultServerPort", getDefaultRemotePort());
  licqConf.WriteNum("TCPServerPort", getTcpServerPort());
  licqConf.WriteNum("MaxUsersPerPacket", getMaxUsersPerPacket());
  licqConf.WriteBool("AllowNewUsers", AllowNewUsers());

  // Utility tab
  //licqConf.WriteStr("Errors", server->getErrorLogName());
  licqConf.WriteStr("UrlViewer", m_szUrlViewer);
  const char *pc = strrchr(gTranslator.getMapName(), '/');
  if (pc != NULL)
    pc++;
  else
    pc = gTranslator.getMapName();
  licqConf.WriteStr("Translation", pc);
  licqConf.WriteStr("Terminal", m_szTerminal);

  //optionsDlg->cmbServers->clear();
  //unsigned short i;
  //for (i = 0; i < server->icqServers.numServers(); i++)
  //   optionsDlg->cmbServers->insertItem(server->icqServers.servers[i]->name());

  // save the sound stuff
  licqConf.SetSection("onevent");
  COnEventManager *oem = OnEventManager();
  licqConf.WriteNum("Enable", oem->CommandType());
  oem->Lock();
  licqConf.WriteStr("Command", oem->Command());
  licqConf.WriteStr("Message", oem->Parameter(ON_EVENT_MSG));
  licqConf.WriteStr("Url", oem->Parameter(ON_EVENT_URL));
  licqConf.WriteStr("Chat",oem->Parameter(ON_EVENT_CHAT));
  licqConf.WriteStr("File",oem->Parameter(ON_EVENT_FILE));
  licqConf.WriteStr("OnlineNotify", oem->Parameter(ON_EVENT_NOTIFY));
  licqConf.WriteStr("SysMsg", oem->Parameter(ON_EVENT_SYSMSG));
  oem->Unlock();

  licqConf.FlushFile();

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  o->saveInfo();
  gUserManager.DropOwner();
}

//++++++NOT MT SAFE+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// This needs to be changed
bool CICQDaemon::getTcpPort(unsigned short i)  { return (m_vbTcpPorts[i]); }
void CICQDaemon::setTcpPort(unsigned short i, bool b) { m_vbTcpPorts[i] = b; }
const char *CICQDaemon::Terminal(void)       { return m_szTerminal; }
void CICQDaemon::SetTerminal(const char *s)  { SetString(&m_szTerminal, s); }

const char *CICQDaemon::getUrlViewer(void)
{
  if (strcmp(m_szUrlViewer, "none") == 0)
    return (NULL);
  else
    return (m_szUrlViewer);
}

void CICQDaemon::setUrlViewer(const char *s)
{
  SetString(&m_szUrlViewer, s);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//-----SaveUserList-------------------------------------------------------------
void CICQDaemon::SaveUserList(void)
{
  char filename[MAX_FILENAME_LEN];
  sprintf(filename, "%s/users.conf", BASE_DIR);
  FILE *usersConf = fopen(filename, "w");

  fprintf(usersConf, "[users]\nNumOfUsers = %d\n", gUserManager.NumUsers());

  unsigned short i = 1;
  FOR_EACH_USER_START(LOCK_R)
  {
    fprintf(usersConf, "User%d = %ld\n", i, pUser->getUin());
    i++;
  }
  FOR_EACH_USER_END

  fclose(usersConf);
}


//-----AddUserToList------------------------------------------------------------
void CICQDaemon::AddUserToList(unsigned long _nUin)
{
  // Don't add a user we already have
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
  if (u != NULL)
  {
    gUserManager.DropUser(u);
    gLog.Warn("%sUser %ld already on contact list.\n", L_WARNxSTR, _nUin);
    return;
  }

  gUserManager.AddUser(new ICQUser(_nUin));
  SaveUserList();

  if (m_nUDPSocketDesc != -1) icqAddUser(_nUin);

  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_ADD, _nUin));
}


//-----AddUserToList------------------------------------------------------------
void CICQDaemon::AddUserToList(ICQUser *nu)
{
  // Don't add a user we already have
  ICQUser *u = gUserManager.FetchUser(nu->getUin(), LOCK_R);
  if (u != NULL)
  {
    gUserManager.DropUser(u);
    gLog.Warn("%sUser %ld already on contact list.\n", L_WARNxSTR, nu->getUin());
    return;
  }

  gUserManager.AddUser(nu);
  SaveUserList();

  if (m_nUDPSocketDesc != -1) icqAddUser(nu->getUin());

  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_ADD, nu->getUin()));
}

//-----RemoveUserFromList-------------------------------------------------------
void CICQDaemon::RemoveUserFromList(unsigned long _nUin)
{
  gUserManager.RemoveUser(_nUin);
  SaveUserList();
  //icqRemoveUser(id);

  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REMOVE, _nUin));
}


//-----ChangeUserStatus-------------------------------------------------------
void CICQDaemon::ChangeUserStatus(ICQUser *u, unsigned long s)
{
  if (s == ICQ_STATUS_OFFLINE)
    u->setStatusOffline();
  else
    u->setStatus(s);
  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                  USER_STATUS, u->getUin()));
}


//-----AddUserEvent-----------------------------------------------------------
void CICQDaemon::AddUserEvent(ICQUser *u, CUserEvent *e)
{
  if (u->User()) e->AddToHistory(u, D_RECEIVER);
  // Don't log a user event if this user is on the ignore list
  if (u->IgnoreList()) return;
  u->AddEvent(e);
  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_EVENTS,
                                  u->getUin()));
}



/*----------------------------------------------------------------------------
 * CICQDaemon::SendExpectEvent
 *
 * Sends an event and expects a reply.  Packages the given information into
 * an event structure and sticks it on the pending events queue.  Then signals
 * that it's there.
 *--------------------------------------------------------------------------*/
ICQEvent *CICQDaemon::SendExpectEvent(int _nSD, CPacket *packet, EConnect _eConnect)
{
  return SendExpectEvent(_nSD, packet, _eConnect, 0, NULL);
}

ICQEvent *CICQDaemon::SendExpectEvent(int _nSD, CPacket *packet, EConnect _eConnect,
                                      unsigned long _nDestinationUin, CUserEvent *ue)
{
  ICQEvent *e = new ICQEvent(_nSD, packet, _eConnect, _nDestinationUin, ue);

  pthread_mutex_lock(&mutex_pendingevents);
  m_lxPendingEvents.push_back(e);
  pthread_mutex_unlock(&mutex_pendingevents);
  DEBUG_THREADS("[SendExpectEvent] Throwing pending event.\n");
  pthread_cond_signal(&cond_pendingevents);

  return (e);
}


/*------------------------------------------------------------------------------
 * CICQDaemon::SendEvent
 *
 * Sends an event without expecting a reply, does not create an event
 * structure, and does not attempt a connection if the socket is invalid.
 * Can possibly block on send, but this is ok as it is never called from the
 * gui thread.
 * Note that the user who owns the given socket is probably read-locked at
 * this point.
 *----------------------------------------------------------------------------*/
void CICQDaemon::SendEvent(int _nSD, CPacket &p)
{
  //CBuffer buffer(p.getBuffer());
  INetSocket *s = gSocketManager.FetchSocket(_nSD);
  if (s == NULL) return;
  s->Send(p.getBuffer());
  gSocketManager.DropSocket(s);
}


/*------------------------------------------------------------------------------
 * DoneEvent
 *
 * Marks the given event as done and removes it from the running events list.
 * Then calls PushDoneEvent on the event.
 *----------------------------------------------------------------------------*/
ICQEvent *CICQDaemon::DoneEvent(ICQEvent *e, EEventResult _eResult)
{
  pthread_mutex_lock(&mutex_runningevents);
  list<ICQEvent *>::iterator iter;
  bool bFound = false;
  for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); iter++)
  {
    if (e == *iter)
    {
      bFound = true;
      m_lxRunningEvents.erase(iter);
      break;
    }
  }
  //bool bFound = (iter == m_lxRunningEvents.end());
  pthread_mutex_unlock(&mutex_runningevents);

  // If we didn't find the event, it must have already been removed, we are too late
  if (!bFound) return (NULL);

  e->m_eResult = _eResult;

  // Check if we should cancel a processing thread
  if (!pthread_equal(e->thread_send, pthread_self()))
    pthread_cancel(e->thread_send);

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
ICQEvent *CICQDaemon::DoneEvent(int _nSD, unsigned long _nSequence, EEventResult _eResult)
{
  pthread_mutex_lock(&mutex_runningevents);
  ICQEvent *e = NULL;
  list<ICQEvent *>::iterator iter;
  for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); iter++)
  {
    if ((*iter)->CompareEvent(_nSD, _nSequence) )
    {
      e = *iter;
      m_lxRunningEvents.erase(iter);
      break;
    }
  }
  pthread_mutex_unlock(&mutex_runningevents);

  // If we didn't find the event, it must have already been removed, we are too late
  if (e == NULL) return (NULL);

  e->m_eResult = _eResult;

  // Check if we should cancel a processing thread
  if (!pthread_equal(e->thread_send, pthread_self()))
    pthread_cancel(e->thread_send);

  return(e);
}


/*------------------------------------------------------------------------------
 * PushDoneEvent
 *
 * Takes the given event, moves it event into the done event queue,
 * signalling it's presence there.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PushDoneEvent(ICQEvent *e)
{
  pthread_mutex_lock(&mutex_doneevents);
  m_qxDoneEvents.push_back(e);
  pthread_mutex_unlock(&mutex_doneevents);
  pthread_cond_signal(&cond_doneevents);
}


/*------------------------------------------------------------------------------
 * DoneExtendedEvent
 *
 * Tracks down the relevant extended event, removes it from the list, and
 * returns it, marking the result as appropriate.
 *----------------------------------------------------------------------------*/
ICQEvent *CICQDaemon::DoneExtendedEvent(const unsigned short _nCommand, const unsigned short _nSubSequence, EEventResult _eResult)
{
  pthread_mutex_lock(&mutex_extendedevents);
  ICQEvent *e = NULL;
  list<ICQEvent *>::iterator iter;
  for (iter = m_lxExtendedEvents.begin(); iter != m_lxExtendedEvents.end(); iter++)
  {
    if ((*iter)->m_nSubSequence == _nSubSequence && (*iter)->m_nCommand == _nCommand)
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


ICQEvent *CICQDaemon::DoneExtendedEvent(ICQEvent *e, EEventResult _eResult)
{
  pthread_mutex_lock(&mutex_extendedevents);
  list<ICQEvent *>::iterator iter;
  for (iter = m_lxExtendedEvents.begin(); iter != m_lxExtendedEvents.end(); iter++)
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
  return(e);
}


/*------------------------------------------------------------------------------
 * PushExtendedEvent
 *
 * Takes the given event, moves it event into the extended event queue.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PushExtendedEvent(ICQEvent *e)
{
  pthread_mutex_lock(&mutex_extendedevents);
  m_lxExtendedEvents.push_back(e);
  pthread_mutex_unlock(&mutex_extendedevents);
}


/*------------------------------------------------------------------------------
 * PushPluginEvent
 *
 * Sticks the given event into the gui event queue.  Then signals that it is
 * there by sending data on the pipe.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PushPluginEvent(ICQEvent *e)
{
  vector<CPlugin *>::iterator iter;
  pthread_mutex_lock(&mutex_plugins);
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    if ((*iter)->CompareThread(e->thread_plugin))
    {
      (*iter)->PushEvent(e);
      break;
    }
  }
  // If no plugin got the event, then just delete it
  if (iter == m_vPlugins.end()) delete e;
  pthread_mutex_unlock(&mutex_plugins);
}


/*------------------------------------------------------------------------------
 * PushPluginSignal
 *
 * Sticks the given event into the gui signal queue.  Then signals that it is
 * there by sending data on the pipe.
 *----------------------------------------------------------------------------*/
void CICQDaemon::PushPluginSignal(CICQSignal *s)
{
  vector<CPlugin *>::iterator iter;
  pthread_mutex_lock(&mutex_plugins);
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    if ( (*iter)->CompareMask(s->Signal()) )
      (*iter)->PushSignal(new CICQSignal(s));
  }
  pthread_mutex_unlock(&mutex_plugins);
  delete s;
}


/*------------------------------------------------------------------------------
 * PopPluginSignal
 *
 * Pops an event from the gui signal queue.
 *----------------------------------------------------------------------------*/
CICQSignal *CICQDaemon::PopPluginSignal(void)
{
  vector<CPlugin *>::iterator iter;
  CICQSignal *s = NULL;
  pthread_mutex_lock(&mutex_plugins);
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    if ( (*iter)->CompareThread(pthread_self()) )
    {
      s = (*iter)->PopSignal();
      break;
    }
  }
  pthread_mutex_unlock(&mutex_plugins);
  return s;
}

/*------------------------------------------------------------------------------
 * PopPluginEvent
 *
 * Pops an event from the gui event queue.
 *----------------------------------------------------------------------------*/
ICQEvent *CICQDaemon::PopPluginEvent(void)
{
  vector<CPlugin *>::iterator iter;
  ICQEvent *e = NULL;
  pthread_mutex_lock(&mutex_plugins);
  for (iter = m_vPlugins.begin(); iter != m_vPlugins.end(); iter++)
  {
    if ( (*iter)->CompareThread(pthread_self()) )
    {
      e = (*iter)->PopEvent();
      break;
    }
  }
  pthread_mutex_unlock(&mutex_plugins);
  return e;
}


//-----CICQDaemon::CancelEvent---------------------------------------------------------
void CICQDaemon::CancelEvent(ICQEvent *e)
{
  if (!DoneEvent(e, EVENT_CANCELLED) && !DoneExtendedEvent(e, EVENT_CANCELLED)) return;

  if (e->m_nSubCommand == ICQ_CMDxSUB_CHAT)
    icqChatRequestCancel(e->m_nDestinationUin, e->m_nSequence);
  else if (e->m_nSubCommand == ICQ_CMDxSUB_FILE)
    icqFileTransferCancel(e->m_nDestinationUin, e->m_nSequence);

  delete e;
}


//-----updateAllUsers-------------------------------------------------------------------------
void CICQDaemon::UpdateAllUsers()
{
/*
  for (unsigned short i = 0; i < getNumUsers(); i++)
  {
    icqUserBasicInfo(getUser(i));
    icqUserExtInfo(getUser(i));
  }
*/
  gLog.Warn("%sThis feature does not work right now.\n", L_WARNxSTR);
}


//-----ParseFE------------------------------------------------------------------
void CICQDaemon::ParseFE(char *szBuffer, char ***szSubStr, int nMaxSubStr)
{
   char *pcEnd = szBuffer, *pcStart;
   unsigned short i = 0;
   bool bDone = false;
   // Clear the character pointers
   memset(*szSubStr, 0, nMaxSubStr * sizeof(char *));

   while (!bDone && i < nMaxSubStr)
   {
      pcStart = pcEnd;
      while (*pcEnd != '\0' && (unsigned char)*pcEnd != (unsigned char)0xFE)
        pcEnd++;
      if (*pcEnd == '\0')
         bDone = true;
      else // we are at an FE boundary
         *pcEnd++ = '\0';
      (*szSubStr)[i++] = pcStart;
   }
}


//-----ProcessFifo--------------------------------------------------------------
void CICQDaemon::ProcessFifo(char *_szBuf)
{
#ifdef USE_FIFO
  char *szCommand, *szRawArgs;

  // Make the command and data variables point to the relevant data in the buf
  szCommand = szRawArgs = _szBuf;
  while (*szRawArgs != '\0' && !isspace(*szRawArgs)) szRawArgs++;
  if (szRawArgs != '\0')
  {
    *szRawArgs = '\0';
    szRawArgs++;
    while (isspace(*szRawArgs)) szRawArgs++;
  }
  if (szRawArgs[strlen(szRawArgs) - 1] == '\n') szRawArgs[strlen(szRawArgs) - 1] = '\0';

  gLog.Info("%sReceived command \"%s\" with arguments \"%s\".\n", L_FIFOxSTR,
            szCommand, szRawArgs);

  char *szProcessedArgs = new char[strlen(szRawArgs) + 1];
  AddNewLines(szProcessedArgs, szRawArgs);
  char *szArgs = szProcessedArgs;

  // Process the command
  if (strcasecmp(szCommand, "status") == 0)
  {
    char *szStatus = szArgs;
    if (*szStatus == '\0')
    {
      gLog.Warn("%sFifo \"status\" command with no argument.\n", L_WARNxSTR);
      goto fifo_done;
    }
    while (!isspace(*szArgs) && *szArgs != '\0') szArgs++;
    if (*szArgs != '\0')
    {
      *szArgs = '\0';
      szArgs++;
    }
    // Determine the status to go to
    ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
    unsigned long nStatus = o->getStatusFlags();
    bool b = o->getStatusOffline();
    gUserManager.DropOwner();

    if (szStatus[0] == '*')
    {
      szStatus++;
      nStatus |= ICQ_STATUS_FxPRIVATE;
    }
    if (strcasecmp(szStatus, "online") == 0)
      nStatus |= ICQ_STATUS_ONLINE;
    else if (strcasecmp(szStatus, "away") == 0)
      nStatus |= ICQ_STATUS_AWAY;
    else if (strcasecmp(szStatus, "na") == 0)
      nStatus |= ICQ_STATUS_NA;
    else if (strcasecmp(szStatus, "occupied") == 0)
      nStatus |= ICQ_STATUS_OCCUPIED;
    else if (strcasecmp(szStatus, "dnd") == 0)
      nStatus |= ICQ_STATUS_DND;
    else if (strcasecmp(szStatus, "ffc") == 0)
      nStatus |= ICQ_STATUS_FREEFORCHAT;
    else if (strcasecmp(szStatus, "offline") == 0)
      nStatus = ICQ_STATUS_OFFLINE;
    else
    {
      gLog.Warn("%sFifo \"status\" command with invalid status \"%s\".\n",
                L_WARNxSTR, szStatus);
      return;
    }

    if (nStatus == ICQ_STATUS_OFFLINE)
    {
      if (!b) icqLogoff(false);
    }
    else
    {
      if (b)
        icqLogon(nStatus);
      else
        icqSetStatus(nStatus);
    }

    // Now set the auto response
    while (isspace(*szArgs) && *szArgs != '\0') szArgs++;
    if (*szArgs != '\0')
    {
      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
      o->SetAutoResponse(szArgs);
      gUserManager.DropOwner();
    }
  }
  else if (strcasecmp(szCommand, "auto_response") == 0)
  {
    if (*szArgs != '\0')
    {
      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
      o->SetAutoResponse(szArgs);
      gUserManager.DropOwner();
    }
    else
    {
      gLog.Warn("%sFifo \"auto_response\" command with no argument.\n", L_WARNxSTR);
    }
  }
  else if (strcasecmp(szCommand, "message") == 0)
  {
    if (*szArgs == '\0')
    {
      gLog.Warn("%sFifo \"message\" with no UIN.\n", L_WARNxSTR);
      goto fifo_done;
    }
    char *szUin = szArgs;
    while(!isspace(*szArgs) && *szArgs != '\0') szArgs++;
    if (*szArgs == '\0')
    {
      gLog.Warn("%sFifo \"message\" with no message.\n", L_WARNxSTR);
      goto fifo_done;
    }
    *szArgs = '\0';
    szArgs++;
    unsigned long nUin = atoi(szUin);
    icqSendMessage(nUin, szArgs, false, false);
  }
  else if (strcasecmp(szCommand, "url") == 0)
  {
    if (*szArgs == '\0')
    {
      gLog.Warn("%sFifo \"url\" with no UIN.\n", L_WARNxSTR);
      goto fifo_done;
    }
    char *szUin = szArgs;
    while(!isspace(*szArgs) && *szArgs != '\0') szArgs++;
    if (*szArgs == '\0')
    {
      gLog.Warn("%sFifo \"url\" with no URL.\n", L_WARNxSTR);
      goto fifo_done;
    }
    *szArgs = '\0';
    szArgs++;
    unsigned long nUin = atoi(szUin);
    char *szUrl = szArgs;
    while(!isspace(*szArgs) && *szArgs != '\0') szArgs++;
    if (*szArgs == '\0')
    {
      gLog.Warn("%sFifo \"url\" with no description.\n", L_WARNxSTR);
      goto fifo_done;
    }
    *szArgs = '\0';
    szArgs++;
    icqSendUrl(nUin, szUrl, szArgs, false, false);
  }
  else if (strcasecmp(szCommand, "redirect") == 0)
  {
    if (*szArgs != '\0')
    {
      if (!Redirect(szArgs))
      {
        gLog.Warn("%sRedirection to \"%s\" failed:\n%s%s.\n", L_WARNxSTR,
                  szArgs, L_BLANKxSTR, strerror(errno));
      }
      else
        gLog.Info("%sOutput redirected to \"%s\".\n", L_INITxSTR, szArgs);
    }
    else
    {
      gLog.Warn("%sFifo \"redirect\" command with no argument.\n", L_WARNxSTR);
    }
   }
  else if (strcasecmp(szCommand, "exit") == 0)
  {
    Shutdown();
  }
  else if (strcasecmp(szCommand, "help") == 0)
  {
    gLog.Info("%sFifo Help:\n"
              "%sstatus [*]<status> [auto response]\n"
              "%sauto_response <auto response>\n"
              "%smessage <uin> <message>\n"
              "%surl <uin> <url> <description>\n"
              "%sredirect <device>\n"
              "%sexit\n", L_FIFOxSTR, L_BLANKxSTR, L_BLANKxSTR, L_BLANKxSTR,
              L_BLANKxSTR, L_BLANKxSTR, L_BLANKxSTR);
  }
  else
  {
    gLog.Warn("%sUnknown fifo command \"%s\".\n", L_WARNxSTR, szCommand);
  }

fifo_done:
  delete [] szProcessedArgs;
#endif //USE_FIFO

}

