// -*- c-basic-offset: 2 -*-
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "time-fix.h"

#include "licq_icqd.h"
#include "licq_translate.h"
#include "licq_packets.h"
#include "licq_socket.h"
#include "licq_user.h"
#include "licq_events.h"
#include "licq_log.h"
#include "licq_chat.h"
#include "licq_filetransfer.h"
#include "support.h"
#include "licq_message.h"

//-----icqAddUser----------------------------------------------------------
void CICQDaemon::icqAddUser(unsigned long _nUin)
{
  CSrvPacketTcp *p = new CPU_GenericUinList(_nUin, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
  gLog.Info("%sAlerting server to new user (#%ld)...\n", L_SRVxSTR,
            p->Sequence());
  SendExpectEvent_Server(_nUin, p, NULL);

  // update the users info from the server
  icqUserBasicInfo(_nUin);
}


//-----icqAlertUser-------------------------------------------------------------
void CICQDaemon::icqAlertUser(unsigned long _nUin)
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  char sz[MAX_MESSAGE_SIZE];
  sprintf(sz, "%s%c%s%c%s%c%s%c%c%c", o->GetAlias(), 0xFE, o->GetFirstName(),
          0xFE, o->GetLastName(), 0xFE, o->GetEmailPrimary(), 0xFE,
          o->GetAuthorization() ? '0' : '1', 0xFE);
  gUserManager.DropOwner();
  CPU_ThroughServer *p = new CPU_ThroughServer(_nUin, ICQ_CMDxSUB_ADDEDxTOxLIST, sz);
  gLog.Info("%sAlerting user they were added (#%ld)...\n", L_SRVxSTR, p->Sequence());
  SendExpectEvent_Server(_nUin, p, NULL);
}


//-----NextServer---------------------------------------------------------------
void CICQDaemon::SwitchServer()
{
  icqRelogon();
}


void CICQDaemon::icqRegister(const char *_szPasswd)
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->SetPassword(_szPasswd);
  gUserManager.DropOwner();
  CPU_Register *p = new CPU_Register(_szPasswd);
  gLog.Info("%sRegistering a new user (#%ld)...\n", L_SRVxSTR, p->Sequence());
  m_nServerAck = p->Sequence() - 1;
  m_nServerSequence = 0;
  SendExpectEvent_Server(0, p, NULL);
}


//-----ICQ::icqRelogon-------------------------------------------------------
void CICQDaemon::icqRelogon(bool bChangeServer)
{
  unsigned long status;

  if (m_eStatus == STATUS_ONLINE)
  {
    ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
    status = o->StatusFull();
    gUserManager.DropOwner();
  }
  else
  {
    status = m_nDesiredStatus;
  }

  if (bChangeServer)
    SwitchServer();
  else
    icqLogoff();
  m_eStatus = STATUS_OFFLINE_MANUAL;

  icqLogon(status);

  m_eStatus = STATUS_OFFLINE_FORCED;
}

//-----icqRequestMetaInfo----------------------------------------------------
unsigned long CICQDaemon::icqRequestMetaInfo(unsigned long nUin)
{
  CPU_Meta_RequestAllInfo *p = new CPU_Meta_RequestAllInfo(nUin);
  gLog.Info("%sRequesting meta info for %ld (#%ld/#%d)...\n", L_SRVxSTR, nUin,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(nUin, p, NULL);
  PushExtendedEvent(e);
  return e->EventId();
}

//-----icqSetStatus-------------------------------------------------------------
unsigned long CICQDaemon::icqSetStatus(unsigned short newStatus)
{
  // Set the status flags
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned long s = o->AddStatusFlags(newStatus);
  bool Invisible = o->StatusInvisible();
  bool goInvisible = (newStatus & ICQ_STATUS_FxPRIVATE);
  bool isLogon = o->StatusOffline();
  gUserManager.DropOwner();

  if (!Invisible && goInvisible)
    icqSendVisibleList();

  CSrvPacketTcp* p;
  if (isLogon)
    p = new CPU_SetLogonStatus(s);
  else
    p = new CPU_SetStatus(s);

  gLog.Info("%sChanging status to %s (#%ld)...\n", L_SRVxSTR,
            ICQUser::StatusToStatusStr(newStatus, goInvisible), p->Sequence());
  m_nDesiredStatus = s;

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  unsigned long eid = e->EventId();

  if (Invisible && !goInvisible)
    icqSendInvisibleList();

  return eid;
}

//-----icqSetPassword--------------------------------------------------------
unsigned long CICQDaemon::icqSetPassword(const char *szPassword)
{
  CPU_SetPassword *p = new CPU_SetPassword(szPassword);
  gLog.Info("%sUpdating password (#%ld/#%d)...\n", L_SRVxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  PushExtendedEvent(e);
  return e->EventId();
}

//-----icqAuthorizeGrant--------------------------------------------------------
unsigned long CICQDaemon::icqAuthorizeGrant(unsigned long nUin, const char *szMessage)
// authorize a user to add you to their contact list
{
  char *sz = NULL;
  if (szMessage != NULL)
  {
    sz = gTranslator.NToRN(szMessage);
    gTranslator.ClientToServer(sz);
  }
  CPU_ThroughServer *p = new CPU_ThroughServer(nUin, ICQ_CMDxSUB_AUTHxGRANTED, sz);
  gLog.Info("%sAuthorizing user %ld (#%ld)...\n", L_SRVxSTR, nUin, p->Sequence());
  delete sz;

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  return e->EventId();
}

//-----icqAuthorizeRefuse-------------------------------------------------------
unsigned long CICQDaemon::icqAuthorizeRefuse(unsigned long nUin, const char *szMessage)
// refuseto authorize a user to add you to their contact list
{
  char *sz = NULL;
  if (szMessage != NULL)
  {
    sz = gTranslator.NToRN(szMessage);
    gTranslator.ClientToServer(sz);
  }
  CPU_ThroughServer *p = new CPU_ThroughServer(nUin, ICQ_CMDxSUB_AUTHxREFUSED, sz);
  gLog.Info("%sRefusing authorization to user %ld (#%ld)...\n", L_SRVxSTR,
     nUin, p->Sequence());
  delete sz;

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  return e->EventId();
}

//-----icqSetSecurityInfo----------------------------------------------------
unsigned long CICQDaemon::icqSetSecurityInfo(bool bAuthorize, bool bHideIp, bool bWebAware)
{
    CPU_Meta_SetSecurityInfo *p = new CPU_Meta_SetSecurityInfo(bAuthorize, bHideIp, bWebAware);
    gLog.Info("%sUpdating security info (#%ld/#%d)...\n", L_SRVxSTR, p->Sequence(), p->SubSequence());
    ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
    PushExtendedEvent(e);
    return e->EventId();
}

//-----icqSearchByUin------------------------------------------------------------
unsigned long CICQDaemon::icqSearchByUin(unsigned long nUin)
{
   CPU_SearchByUin *p = new CPU_SearchByUin(nUin);
   gLog.Info("%sStarting search by UIN for user (#%ld/#%d)...\n", L_SRVxSTR,
   	p->Sequence(), p->SubSequence());
   ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
   PushExtendedEvent(e);
   return e->EventId();
}

//-----icqSearchByInfo-----------------------------------------------------------
unsigned long CICQDaemon::icqSearchByInfo(const char *nick, const char *first,
                                          const char *last, const char *email)
{
  CPU_SearchByInfo *p = new CPU_SearchByInfo(nick, first, last, email);
  gLog.Info("%sStarting search by info for user (#%ld/#%d)...\n", L_SRVxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  PushExtendedEvent(e);
  return e->EventId();
}

//-----icqSearchWhitePages--------------------------------------------------
unsigned long CICQDaemon::icqSearchWhitePages(const char *szFirstName,
    const char *szLastName, const char *szAlias, const char *szEmail,
    unsigned short nMinAge, unsigned short nMaxAge, char nGender,
    char nLanguage, const char *szCity, const char *szState,
    unsigned short nCountryCode, const char *szCoName, const char *szCoDept,
    const char *szCoPos, bool bOnlineOnly)
{
  // Yes, there are a lot of extra options that you can search by.. but I
  // don't see a point for the hundreds of items that I can add..  just
  // use their web page for that shit - Jon
  CPU_SearchWhitePages *p = new CPU_SearchWhitePages(szFirstName, szLastName,
    szAlias, szEmail, nMinAge, nMaxAge, nGender, nLanguage, szCity, szState,
    nCountryCode, szCoName, szCoDept, szCoPos, bOnlineOnly);
  gLog.Info("%sStarting white pages search (#%ld/#%d)...\n", L_SRVxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  PushExtendedEvent(e);
  return e->EventId();
}

//-----icqGetUserBasicInfo------------------------------------------------------
unsigned long CICQDaemon::icqUserBasicInfo(unsigned long _nUin)
{
  //CPU_Meta_RequestBasicInfo *p = new CPU_Meta_RequestBasicInfo(_nUin);
  CPU_Meta_RequestAllInfo *p = new CPU_Meta_RequestAllInfo(_nUin);
  gLog.Info("%sRequesting user info (#%ld/#%d)...\n", L_SRVxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(_nUin, p, NULL);
  PushExtendedEvent(e);
  return e->EventId();
}

//-----icqPing------------------------------------------------------------------
void CICQDaemon::icqPing()
{
  // pinging is necessary to avoid that masquerading
  // servers close the connection
   CPU_Ping *p = new CPU_Ping;
   SendEvent_Server(p);
}

//-----icqUpdateContactList-----------------------------------------------------
void CICQDaemon::icqUpdateContactList()
{
  unsigned short n = 0;
  UinList uins;
  FOR_EACH_USER_START(LOCK_W)
  {
    n++;
    uins.push_back(pUser->Uin());
    if (n == m_nMaxUsersPerPacket)
    {
      CSrvPacketTcp *p = new CPU_GenericUinList(uins, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
      gLog.Info("%sUpdating contact list (#%ld)...\n", L_SRVxSTR, p->Sequence());
      SendEvent_Server(p);
      uins.erase(uins.begin(), uins.end());
      n = 0;
    }
    // Reset all users to offline
    if (!pUser->StatusOffline()) ChangeUserStatus(pUser, ICQ_STATUS_OFFLINE);
  }
  FOR_EACH_USER_END
  if (n != 0)
  {
    CSrvPacketTcp *p = new CPU_GenericUinList(uins, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
    gLog.Info("%sUpdating contact list (#%ld)...\n", L_SRVxSTR, p->Sequence());
    SendEvent_Server(p);
  }
}

//-----icqSendVisibleList-------------------------------------------------------
void CICQDaemon::icqSendVisibleList()
{
  // send user info packet
  // Go through the entire list of users, checking if each one is on
  // the visible list
  UinList uins;
  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->GetInGroup(GROUPS_SYSTEM, GROUP_VISIBLE_LIST) )
      uins.push_back(pUser->Uin());
  }
  FOR_EACH_USER_END
  CSrvPacketTcp* p = new CPU_GenericUinList(uins, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxVISIBLExLIST);
  gLog.Info("%sSending visible list (#%ld)...\n", L_SRVxSTR, p->Sequence());
  SendEvent_Server(p);
}


//-----icqSendInvisibleList-----------------------------------------------------
void CICQDaemon::icqSendInvisibleList()
{
  UinList uins;
  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->GetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST) )
      uins.push_back(pUser->Uin());
  }
  FOR_EACH_USER_END

  CSrvPacketTcp* p = new CPU_GenericUinList(uins, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxINVISIBxLIST);
  gLog.Info("%sSending invisible list (#%ld)...\n", L_SRVxSTR, p->Sequence());
  SendEvent_Server(p);
}

//-----icqAddToVisibleList---------------------------------------------------
void CICQDaemon::icqToggleVisibleList(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
  bool b = u->VisibleList();
  gUserManager.DropUser(u);

  if (b)
    icqRemoveFromVisibleList(nUin);
  else
    icqAddToVisibleList(nUin);
}

void CICQDaemon::icqToggleInvisibleList(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
  bool b = u->InvisibleList();
  gUserManager.DropUser(u);

  if (b)
    icqRemoveFromInvisibleList(nUin);
  else
    icqAddToInvisibleList(nUin);
}

void CICQDaemon::icqAddToVisibleList(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u != NULL)
  {
    u->SetVisibleList(true);
    gUserManager.DropUser(u);
  }
  CSrvPacketTcp *p = new CPU_GenericUinList(nUin, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxVISIBLExLIST);
  gLog.Info("%sAdding user %ld to visible list (#%ld)...\n", L_SRVxSTR, nUin,
     p->Sequence());
  SendEvent_Server(p);
}

void CICQDaemon::icqRemoveFromVisibleList(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u != NULL)
  {
    u->SetVisibleList(false);
    gUserManager.DropUser(u);
  }
  CSrvPacketTcp *p = new CPU_GenericUinList(nUin, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_REMxVISIBLExLIST);
  gLog.Info("%sRemoving user %ld from visible list (#%ld)...\n", L_SRVxSTR, nUin,
     p->Sequence());
  SendEvent_Server(p);
}

void CICQDaemon::icqAddToInvisibleList(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u != NULL)
  {
    u->SetInvisibleList(true);
    gUserManager.DropUser(u);
  }
  CSrvPacketTcp *p = new CPU_GenericUinList(nUin, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxINVISIBxLIST);
  gLog.Info("%sAdding user %ld to invisible list (#%ld)...\n", L_SRVxSTR, nUin,
     p->Sequence());
  SendEvent_Server(p);
}

void CICQDaemon::icqRemoveFromInvisibleList(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u != NULL)
  {
    u->SetInvisibleList(false);
    gUserManager.DropUser(u);
  }
  CSrvPacketTcp *p = new CPU_GenericUinList(nUin, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_REMxINVISIBxLIST);
  gLog.Info("%sRemoving user %ld from invisible list (#%ld)...\n", L_SRVxSTR, nUin,
     p->Sequence());
  SendEvent_Server(p);
}

ICQEvent* CICQDaemon::icqSendThroughServer(unsigned long nUin, unsigned char format, char *_sMessage, CUserEvent* ue)
{
  ICQEvent* result;

  CPU_ThroughServer *p = new CPU_ThroughServer(nUin, format, _sMessage);
  
  switch (format)
  {
    case ICQ_CMDxSUB_MSG:
	gLog.Info("%sSending message through server (#%ld).\n", L_SRVxSTR, p->Sequence());
	break;	
    case ICQ_CMDxSUB_URL:
        gLog.Info("%sSending url through server (#%ld).\n", L_SRVxSTR, p->Sequence());
	break;
    default:
	gLog.Info("%sSending misc through server (#%ld).\n", L_SRVxSTR, p->Sequence());
  }

  // If we are already shutting down, don't start any events
  if (m_bShuttingDown) return NULL;

  if (ue != NULL) ue->m_eDir = D_SENDER;
  ICQEvent *e = new ICQEvent(this, m_nTCPSrvSocketDesc, p, CONNECT_SERVER, nUin, ue);
  e->m_NoAck = true;

  result = SendExpectEvent(e, &ProcessRunningEvent_Server_tep);

  return result;
}

/*------------------------------------------------------------------------------
 * ProcessDoneEvent
 *
 * Processes the given event possibly passes the result to the gui.
 *----------------------------------------------------------------------------*/
void CICQDaemon::ProcessDoneEvent(ICQEvent *e)
{
  // Write the event to the history file if appropriate
  if (e->m_pUserEvent != NULL &&
      e->m_eResult == EVENT_ACKED &&
      e->m_nSubResult != ICQ_TCPxACK_RETURN)
  {
    ICQUser *u = gUserManager.FetchUser(e->m_nDestinationUin, LOCK_R);
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
  case ICQ_CHNxNEW:
  case ICQ_CHNxPING:
  {
    delete e;
    break;
  }
  // Regular events
  case ICQ_CHNxDATA:
    // DAW uwww.. horrible
    e->m_nCommand = ICQ_CMDxSND_THRUxSERVER;
  case ICQ_CMDxTCP_START:
    PushPluginEvent(e);
    break;

  case ICQ_CMDxSND_SETxSTATUS:
    if (e->m_eResult == EVENT_ACKED)
    {
      // DAW FIXME
//       ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
//       ChangeUserStatus(o, ((CPU_SetStatus *)e->m_pPacket)->Status() );
//       gUserManager.DropOwner();
    }
    PushPluginEvent(e);
    break;

  case ICQ_CMDxSND_SETxRANDOMxCHAT:
    if (e->m_eResult == EVENT_ACKED)
    {
      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
      o->SetRandomChatGroup(((CPU_SetRandomChatGroup *)e->m_pPacket)->Group());
      gUserManager.DropOwner();
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
    gLog.Warn("%sInternal error: ProcessDoneEvents(): Unknown command (%04X).\n",
               L_ERRORxSTR, e->m_nCommand);
    delete e;
    return;
  }
}

//-----ICQ::Logon--------------------------------------------------------------

unsigned long CICQDaemon::icqLogon(unsigned short logonStatus)
{
  if (m_bLoggingOn)
  {
    gLog.Warn("%sAttempt to logon while already logged or logging on, logoff and try again.\n", L_WARNxSTR);
    return 0;
  }
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  if (o->Uin() == 0)
  {
    gUserManager.DropOwner();
    gLog.Error("%sNo registered user, unable to process logon attempt.\n", L_ERRORxSTR);
    return 0;
  }
  if (o->Password()[0] == '\0')
  {
    gUserManager.DropOwner();
    gLog.Error("%sNo password set.  Edit ~/.licq/owner.uin and fill in the password field.\n", L_ERRORxSTR);
    return 0;
  }
  char szUin[13];
  snprintf(szUin, 12, "%ld", o->Uin());
  szUin[12] = 0;
  char *passwd = strdup(o->Password());
  unsigned long status = o->AddStatusFlags(logonStatus);
  gUserManager.DropOwner();
  CPU_Logon *p = new CPU_Logon(passwd, szUin, status);
  free(passwd);
  m_bOnlineNotifies = false;
  gLog.Info("%sRequesting logon (#%ld)...\n", L_SRVxSTR, p->Sequence());
  m_nServerSequence = 0;
  m_nDesiredStatus = status;
  m_bLoggingOn = true;
  m_tLogonTime = time(NULL);
//  ICQEvent *e = SendEvent_Server(p);
  SendEvent_Server(p);
  return 0;
//   PushExtendedEvent(e);
//   return e->EventId();
}

//-----ICQ::icqLogoff-----------------------------------------------------------
void CICQDaemon::icqLogoff()
{
  // Kill the udp socket asap to avoid race conditions
  int nSD = m_nTCPSrvSocketDesc;
  m_nTCPSrvSocketDesc = -1;
  gLog.Info("%sLogging off.\n", L_SRVxSTR);
  CPU_Logoff p;
  SendEvent(nSD, p, true);
  gSocketManager.CloseSocket(nSD);

  m_eStatus = STATUS_OFFLINE_MANUAL;
  m_bLoggingOn = false;

  pthread_mutex_lock(&mutex_runningevents);
  std::list<ICQEvent *>::iterator iter = m_lxRunningEvents.begin();
  while (iter != m_lxRunningEvents.end())
  {
    if ((*iter)->m_nSocketDesc == nSD)
    {
      if (!pthread_equal((*iter)->thread_send, pthread_self()))
        pthread_cancel((*iter)->thread_send);
      //(*iter)->m_eResult = EVENT_CANCELLED;
      //ProcessDoneEvent(*iter);
      CancelEvent(*iter);
      iter = m_lxRunningEvents.erase(iter);
    }
    else
      iter++;
  }
  pthread_mutex_unlock(&mutex_runningevents);

  // wipe out all extended events too...
  pthread_mutex_lock(&mutex_extendedevents);
  for (iter = m_lxExtendedEvents.begin(); iter != m_lxExtendedEvents.end(); iter++)
  {
    (*iter)->m_eResult = EVENT_CANCELLED;
    ProcessDoneEvent(*iter);
  }
  m_lxExtendedEvents.erase(m_lxExtendedEvents.begin(), m_lxExtendedEvents.end());
  pthread_mutex_unlock(&mutex_extendedevents);

  // Mark all users as offline, this also updates the last seen
  // online field
  FOR_EACH_USER_START(LOCK_W)
  {
    if (!pUser->StatusOffline())
      ChangeUserStatus(pUser, ICQ_STATUS_OFFLINE);
  }
  FOR_EACH_USER_END

  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  ChangeUserStatus(o, ICQ_STATUS_OFFLINE);
  gUserManager.DropOwner();
  PushPluginSignal(new CICQSignal(SIGNAL_LOGOFF, 0, 0));
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----ConnectToServer---------------------------------------------------------

int CICQDaemon::ConnectToLoginServer()
{
  int r = ConnectToServer("login.icq.com", DEFAULT_SERVER_PORT);

  write(pipe_newsocket[PIPE_WRITE], "S", 1);

  return r;
}

int CICQDaemon::ConnectToServer(const char* server, unsigned short port)
{
  SrvSocket *s = new SrvSocket(0);

  gLog.Info("%sResolving %s port %d...\n", L_SRVxSTR, server, port);
  if (!s->SetRemoteAddr(server, port)) {
    char buf[128];
    gLog.Warn("%sUnable to resolve %s:\n%s%s.\n", L_ERRORxSTR,
              server, L_BLANKxSTR, s->ErrorStr(buf, 128));
    delete s;
    return (-1); // no route to host (not connected)
  }
  char ipbuf[32];
  gLog.Info("%sICQ server found at %s:%d.\n", L_SRVxSTR,
	      s->RemoteIpStr(ipbuf), s->RemotePort());

  gLog.Info("%sOpening socket to server.\n", L_SRVxSTR);
  if (!s->OpenConnection())
  {
    char buf[128];
    gLog.Warn("%sUnable to connect to %s:%d:\n%s%s.\n", L_ERRORxSTR,
              s->RemoteIpStr(ipbuf), s->RemotePort(), L_BLANKxSTR,
              s->ErrorStr(buf, 128));
    delete s;
    return -1;
  }

  // Now get the real ip from this socket
  CPacket::SetLocalIp(  NetworkIpToPacketIp(s->LocalIp() ));
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->SetIp(s->LocalIp());
  gUserManager.DropOwner();

  gSocketManager.AddSocket(s);
  m_nTCPSrvSocketDesc = s->Descriptor();
  gSocketManager.DropSocket(s);

  return m_nTCPSrvSocketDesc;
}

//-----FindUserForInfoUpdate-------------------------------------------------
ICQUser *CICQDaemon::FindUserForInfoUpdate(unsigned long nUin, ICQEvent *e,
   const char *t)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u == NULL)
  {
    // If the event is NULL as well then nothing we can do
    if (e == NULL)
    {
      gLog.Warn("%sResponse to unknown %s info request for unknown user (%ld).\n",
                L_WARNxSTR, t, nUin);
      return NULL;
    }
    // Check if we need to create the user
    if (e->m_pUnknownUser == NULL)
    {
      e->m_pUnknownUser = new ICQUser(nUin);
    }
    // If not, validate the uin
    else if (e->m_pUnknownUser->Uin() != nUin)
    {
      gLog.Error("%sInternal Error: Event contains wrong user.\n", L_ERRORxSTR);
      return NULL;
    }

    u = e->m_pUnknownUser;
    u->Lock(LOCK_W);
  }
  gLog.Info("%sReceived %s information for %s (%ld).\n", L_SRVxSTR, t,
            u->GetAlias(), nUin);
  return u;
}

//-----ProcessSrvPacket---------------------------------------------------------
bool CICQDaemon::ProcessSrvPacket(CBuffer& packet)
{
  unsigned short nLen;
  unsigned short  nSequence;
  char startCode, nChannel;

  // read in the standard server TCP header info
  packet >> startCode;

  if (startCode != 0x2a)
  {
    gLog.Warn("%sProcessSrvPacket bad start code: %d.\n", L_WARNxSTR, startCode);
    char *buf;
    gLog.Unknown("%sUnknown server response:\n%s\n", L_UNKNOWNxSTR,
                 packet.print(buf));
    delete buf;
    return false;
  }

  packet >> nChannel
         >> nSequence
         >> nLen;

  // Eww
  rev_e_short(nSequence);
  rev_e_short(nLen);

  switch (nChannel)
  {
  case ICQ_CHNxNEW:
    // DAW can we really ignore those packets??
    //gLog.Warn("%sPacket on unhandled Channel 'New' received!\n", L_SRVxSTR);
    break;

  case ICQ_CHNxDATA:
    ProcessDataChannel(packet);
    break;

  case ICQ_CHNxERROR:
    gLog.Warn("%sPacket on unhandled Channel 'Error' received!\n", L_SRVxSTR);
    break;

  case ICQ_CHNxCLOSE:
    return ProcessCloseChannel(packet);
    break;

  default:
    gLog.Warn("%sServer send unknown channel: %02x\n", L_SRVxSTR, nChannel);
    break;
  }

  return true;
}

//--------ProcessServiceFam----------------------------------------------------

void CICQDaemon::ProcessServiceFam(CBuffer &packet, unsigned short nSubtype)
{
  packet.UnpackUnsignedShortBE(); // flags
  packet.UnpackUnsignedLongBE(); // sequence

  switch (nSubtype)
  {
  case ICQ_SNACxSUB_READYxSERVER:
    {
      CSrvPacketTcp* p;

      gLog.Info("%sServer says he's ready.\n", L_SRVxSTR);

      gLog.Info("%sSending our channel capability list...\n", L_SRVxSTR);
      p = new CPU_ImICQ();
      SendEvent_Server(p);

      gLog.Info("%sRequesting rate info...\n", L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSUB_REQ_RATE_INFO);
      SendEvent_Server(p);

      // ask the user who we are (yeah we know already though)
      p = new CPU_GenericFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_GETxUSERxINFO);
      SendEvent_Server(p);

      break;
    }

  case ICQ_SNACxSRV_ACKxIMxICQ:
    {
      CSrvPacketTcp* p;
      gLog.Info("%sServer sent us channel capability list (ignoring).\n", L_SRVxSTR);

      gLog.Info("%sRequesting location rights.\n", L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxRIGHTS);
      SendEvent_Server(p);

      gLog.Info("%sRequesting contact list rights.\n", L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_REQUESTxRIGHTS);
      SendEvent_Server(p);

      gLog.Info("%sRequesting Instant Messaging rights.\n", L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_REQUESTxRIGHTS);
      SendEvent_Server(p);

      gLog.Info("%sRequesting BOS rights.\n", L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_REQUESTxRIGHTS);
      SendEvent_Server(p);

      break;
    }
  case ICQ_SNACxSUB_RATE_INFO:
  {
      gLog.Info("%sServer sent us rate information.\n", L_SRVxSTR);
      CSrvPacketTcp *p = new CPU_RateAck();
      SendEvent_Server(p);

      gLog.Info("%sSetting ICQ Instant Messaging Mode.\n", L_SRVxSTR);
      p = new CPU_ICQMode();
      SendEvent_Server(p);

      gLog.Info("%sSending capability settings (?)\n",L_SRVxSTR);
      p = new CPU_CapabilitySettings();
      SendEvent_Server(p);

      icqSetStatus(m_nDesiredStatus);

      gLog.Info("%sSending client ready...\n", L_SRVxSTR);
      p = new CPU_ClientReady();
      SendEvent_Server(p);

      gLog.Info("%sSending offline message request...\n", L_SRVxSTR);
      p = new CPU_RequestSysMsg;
      SendEvent_Server(p);

      m_eStatus = STATUS_ONLINE;
      m_bLoggingOn = false;
      // ### FIX subsequence !!
      ICQEvent *e = DoneExtendedServerEvent(0, EVENT_SUCCESS);
      if (e != NULL) ProcessDoneEvent(e);
      PushPluginSignal(new CICQSignal(SIGNAL_LOGON, 0, 0));

      break;
    }

  case ICQ_SNACxRCV_NAMExINFO:
  {
    unsigned short nUserClass, nLevel;
    unsigned long nUin, realIP;

    gLog.Info("%sGot Name Info from Server\n", L_SRVxSTR);

    nUin = packet.UnpackUinString();
    nLevel = packet.UnpackUnsignedShortBE();
    nUserClass = packet.UnpackUnsignedShortBE();

    gLog.Info("%sUIN: %ld level: %04hx Class: %04hx\n", L_SRVxSTR,
              nUin, nLevel, nUserClass );

    if (!packet.readTLV()) {
      char *buf;
      gLog.Unknown("%sUnknown server response:\n%s\n", L_UNKNOWNxSTR,
         packet.print(buf));
      delete buf;
      break;
    }

    // T(1) unknown
    // T(2) member since
    // T(3) online since
    // T(4) idle time ?
    // T(6) status code
    // T(A) IP
    // T(C) direct connection info (???)
    if (packet.getTLVLen(0x0006) == 4)
      m_nDesiredStatus = packet.UnpackUnsignedLongTLV(0x0006);
    if (packet.getTLVLen(0x000a) == 4) {
      realIP = packet.UnpackUnsignedLongTLV(0x000a);
      rev_e_long(realIP);
      realIP = PacketIpToNetworkIp(realIP);
      CPacket::SetRealIp(NetworkIpToPacketIp(realIP));

      char buf[32];
      gLog.Info("%sServer says we are at %s.\n", L_SRVxSTR, ip_ntoa(realIP, buf));
      //icqSetStatus(m_nDesiredStatus);
    }
    if (packet.getTLVLen(0x000c)) {
      gLog.Unknown("%sServer send us direct conn info,len: %d\n", L_UNKNOWNxSTR,
                   packet.getTLVLen(0x000c));
    }

    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    ChangeUserStatus(o, m_nDesiredStatus);
    gLog.Info("%sServer says we're now: %s\n", L_SRVxSTR, ICQUser::StatusToStatusStr(o->Status(), o->StatusInvisible()));
    gUserManager.DropOwner();

    break;
  }
  default:
    gLog.Warn("%sUnknown Service Family Subtype: %04hx\n", L_SRVxSTR, nSubtype);
    break;
  }
}

//--------ProcessLocationFam-----------------------------------------------
void CICQDaemon::ProcessLocationFam(const CBuffer &packet, unsigned short nSubtype)
{
  switch (nSubtype)
  {
  case ICQ_SNAXxLOC_RIGHTSxGRANTED:
    gLog.Info("%sReceived rights for Location Services\n", L_SRVxSTR);
    break;

  default:
    gLog.Warn("%sUnknown Location Family Subtype: %04hx\n", L_SRVxSTR, nSubtype);
    break;
  }
}

//--------ProcessBuddyFam--------------------------------------------------
void CICQDaemon::ProcessBuddyFam(CBuffer &packet, unsigned short nSubtype)
{
  switch (nSubtype)
  {
  case ICQ_SNACxSUB_ONLINExLIST:
  {
    unsigned long junk1, realIP, userPort, nUin, timestamp;
    unsigned short junk2;
    unsigned char mode;

    junk1 = packet.UnpackUnsignedLongBE();
    junk2 = packet.UnpackUnsignedShortBE();
    nUin  = packet.UnpackUinString();

    junk1 = packet.UnpackUnsignedLongBE(); // tlvcount

    if (!packet.readTLV()) {
      gLog.Error("%sfuck\n", L_ERRORxSTR);
      return;
    }

//     userIP = packet.UnpackUnsignedLongTLV(0x0a, 1);
//     rev_e_long(userIP);

    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
      gLog.Warn("%sUnknown user (%ld) changed status.\n", L_WARNxSTR,
                nUin);
      break;
    }
    // 0 if not set -> Online
    unsigned long nNewStatus = packet.UnpackUnsignedLongTLV(0x0006);
    if (u->StatusFull() != nNewStatus) {
      ChangeUserStatus(u, nNewStatus);
      gLog.Info("%s%s (%ld) changed status: %s.\n", L_SRVxSTR, u->GetAlias(), nUin, u->StatusStr());
      if ( (nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS) )
        gLog.Unknown("%sUnknown status flag for %s (%ld): 0x%08lX\n",
                     L_UNKNOWNxSTR, u->GetAlias(), nUin, (nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS));
      nNewStatus &= ICQ_STATUS_FxUNKNOWNxFLAGS;
    }

    if (packet.getTLVLen(0x000a) == 4) {
      unsigned long userIP = packet.UnpackUnsignedLongTLV(0x000a);
      rev_e_long(userIP);
      userIP = PacketIpToNetworkIp(userIP);
      u->SetIpPort(userIP, u->Port());
    }

    if (packet.getTLVLen(0x000c) == 0x25) {
      CBuffer msg = packet.UnpackTLV(0x000c);

      realIP = msg.UnpackUnsignedLong();
      userPort = msg.UnpackUnsignedLongBE();
      mode = msg.UnpackChar();
      unsigned short tcpVersion = msg.UnpackUnsignedShortBE();
      /*unsigned long nTCPSession =*/ msg.UnpackUnsignedLongBE();
      junk1 = msg.UnpackUnsignedLongBE();  // direct connection cookie
      junk1 = msg.UnpackUnsignedLongBE();
      timestamp = msg.UnpackUnsignedLongBE();  // will be licq version
      junk1 = msg.UnpackUnsignedLongBE();
      if (u->ClientTimestamp() != timestamp ||
          u->Version() != tcpVersion) {
        if((timestamp & 0xFFFF0000) == LICQ_WITHSSL)
          gLog.Info("%s%s (%ld) changed connection info: v%01x [Licq %s/SSL].\n",
                    L_SRVxSTR, u->GetAlias(), nUin, tcpVersion & 0x0F,
                    CUserEvent::LicqVersionToString(timestamp & 0xFFFF));
      else if((timestamp & 0xFFFF0000) == LICQ_WITHOUTSSL)
        gLog.Info("%s%s (%ld) changed connection inf:o v%01x [Licq %s].\n",
                  L_SRVxSTR, u->GetAlias(), nUin, tcpVersion & 0x0F,
                  CUserEvent::LicqVersionToString(timestamp & 0xFFFF));
        else if(timestamp == 0xffffffff)
          gLog.Info("%s%s (%ld) changed connection info: v%01x [MIRANDA].\n",
                    L_SRVxSTR, u->GetAlias(), nUin, tcpVersion & 0x0F);
        else
          gLog.Info("%s%s (%ld) changed connection info: v%01x.\n",
                    L_SRVxSTR, u->GetAlias(), nUin, tcpVersion & 0x0F);


      }
      realIP = PacketIpToNetworkIp(realIP);
      u->SetIpPort(u->Ip(), userPort);
      u->SetRealIp(realIP);
      u->SetVersion(tcpVersion);
      u->SetClientTimestamp(timestamp);

      // What is mode 1?  We can't connect direct...
      if (mode == 1 || mode == 6 ||
          (nNewStatus & ICQ_STATUS_FxDIRECTxLISTED) ||
          (nNewStatus & ICQ_STATUS_FxDIRECTxAUTH)
        ) mode = MODE_INDIRECT;
      if (mode != MODE_DIRECT && mode != MODE_INDIRECT)
      {
        gLog.Unknown("%sUnknown peer-to-peer mode for %s (%ld): %d\n", L_UNKNOWNxSTR,
                     u->GetAlias(), u->Uin(), mode);
        u->SetMode(MODE_DIRECT);
        u->SetSendServer(false);
      }
      else
      {
        u->SetMode(mode);
        u->SetSendServer(mode == MODE_INDIRECT);
      }
      u->SetAutoResponse(NULL);
      u->SetShowAwayMsg(false);
    }
    if ((m_bOnlineNotifies || m_bAlwaysOnlineNotify) && u->OnlineNotify())
      m_xOnEventManager.Do(ON_EVENT_NOTIFY, u);
    gUserManager.DropUser(u);
    break;
  }
  case ICQ_SNACxSUB_OFFLINExLIST:
  {
    unsigned long junk1, nUin;
    unsigned short junk2;

    junk1 = packet.UnpackUnsignedLongBE();
    junk2 = packet.UnpackUnsignedShortBE();

    nUin = packet.UnpackUinString();

    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
      gLog.Warn("%sUnknown user (%ld) has gone offline.\n", L_WARNxSTR, nUin);
      break;
    }
    gLog.Info("%s%s (%ld) went offline.\n", L_SRVxSTR, u->GetAlias(), nUin);
    u->SetClientTimestamp(0);
    ChangeUserStatus(u, ICQ_STATUS_OFFLINE);
    gUserManager.DropUser(u);
    break;
  }
  case ICQ_SNACxBDY_RIGHTSxGRANTED:
  {
    gLog.Info("%sReceived rights for Contact List..\n", L_SRVxSTR);

    icqUpdateContactList();
    break;
  }

  default:
    gLog.Warn("%sUnknown Buddy Family Subtype: %04hx\n", L_SRVxSTR, nSubtype);
    break;
  }
}

//--------ProcessMessageFam------------------------------------------------
void CICQDaemon::ProcessMessageFam(CBuffer &packet, unsigned short nSubtype)
{
  // DAW unknown..
  packet.UnpackUnsignedLongBE();
  packet.UnpackUnsignedShortBE();

  switch (nSubtype)
  {
  case ICQ_SNACxMSG_SERVERxMESSAGE:
  {
    unsigned long nMsgID[2], nUin;
    unsigned long timeSent;
    unsigned short mFormat, nMsgLen, nTLVs;

    nMsgID[0] = packet.UnpackUnsignedLongBE();
    nMsgID[1] = packet.UnpackUnsignedLongBE();
    timeSent   = time(0L);
    mFormat    = packet.UnpackUnsignedShortBE();
    nUin       = packet.UnpackUinString();

    if (nUin < 10000) {
      gLog.Warn("%sMessage through server with strange Uin: %04lx\n", L_WARNxSTR, nUin);
      break;
    }

    packet.UnpackUnsignedShortBE(); // warning level ?
    nTLVs = packet.UnpackUnsignedShortBE();
    if (nTLVs && !packet.readTLV()) {
      gLog.Error("Fuck2\n");
      return;
    }

    switch(mFormat) {
    case 1:
    {
      CBuffer msg = packet.UnpackTLV(0x0002);

      // TLVs in TLV
      // type: 05 01: ???
      //       01 01: 4 bytes flags? + message text
      msg.readTLV();
      CBuffer msgTxt = msg.UnpackTLV(0x0101);
      nMsgLen = msgTxt.getDataSize();

      msgTxt.UnpackUnsignedLongBE(); // always ? 0
      nMsgLen -= 4;

      char* szMessage = new char[nMsgLen+1];
      for (int i = 0; i < nMsgLen; ++i)
        szMessage[i] = msgTxt.UnpackChar();

      szMessage[nMsgLen] = '\0';
      char* szMsg = gTranslator.RNToN(szMessage);
      delete [] szMessage;

      // now send the message to the user
      CEventMsg *e = CEventMsg::Parse(szMsg, ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
      delete [] szMsg;

      // Lock the user to add the message to their queue
      ICQUser* u = gUserManager.FetchUser(nUin, LOCK_W);
      if (u == NULL)
      {
        if (Ignore(IGNORE_NEWUSERS))
        {
          gLog.Info("%sMessage from new user (%ld), ignoring.\n", L_SBLANKxSTR, nUin);
          RejectEvent(nUin, e);
          break;
        }
        gLog.Info("%sMessage from new user (%ld).\n",
                  L_SBLANKxSTR, nUin);
        AddUserToList(nUin);
        u = gUserManager.FetchUser(nUin, LOCK_W);
      }
      else
        gLog.Info("%sMessage through server from %s (%ld).\n", L_SBLANKxSTR,
                  u->GetAlias(), nUin);

      if (AddUserEvent(u, e))
        m_xOnEventManager.Do(ON_EVENT_MSG, u);
      gUserManager.DropUser(u);
      break;
    }
    case 2: // OSCAR's "Add ICBM parameter" message
    {
      char *buf;
      gLog.Unknown("%sAdd ICBM message!\n%s\n",L_UNKNOWNxSTR,packet.print(buf));
      delete [] buf;
      break;
    }
    case 4:
    //Version 5 (V5) protocol messages incapsulated in v7 packet.
    {
      CBuffer msgTxt = packet.UnpackTLV(0x0005);
      msgTxt.UnpackUnsignedLongBE();
      unsigned short newCommand = msgTxt.UnpackUnsignedShort();
      unsigned long nMask = ((newCommand & ICQ_CMDxSUB_FxMULTIREC) ? E_MULTIxREC : 0);
      newCommand &= ~ICQ_CMDxSUB_FxMULTIREC;

      // new unpack the message
      nMsgLen = msgTxt.UnpackUnsignedShort();
      char* szMsg = new char[nMsgLen];
      for (int i = 0; i < nMsgLen; ++i)
        szMsg[i] = msgTxt.UnpackChar();

      char* szMessage = gTranslator.RNToN(szMsg);
      delete [] szMsg;

      switch(newCommand) {
      case ICQ_CMDxSUB_MSG:  // system message: message through the server
      {
        CEventMsg *e = CEventMsg::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, nMask);

        // Lock the user to add the message to their queue
        ICQUser* u = gUserManager.FetchUser(nUin, LOCK_W);
        if (u == NULL)
        {
          if (Ignore(IGNORE_NEWUSERS))
          {
            gLog.Info("%sMessage from new user (%ld), ignoring.\n", L_SBLANKxSTR, nUin);
            RejectEvent(nUin, e);
            break;
          }
          gLog.Info("%sMessage from new user (%ld).\n",
                    L_SBLANKxSTR, nUin);
          AddUserToList(nUin);
          u = gUserManager.FetchUser(nUin, LOCK_W);
        }
        else
          gLog.Info("%sMessage through server from %s (%ld).\n", L_SBLANKxSTR,
                    u->GetAlias(), nUin);

        if (AddUserEvent(u, e))
          m_xOnEventManager.Do(ON_EVENT_MSG, u);
        gUserManager.DropUser(u);
        break;
      }
      case ICQ_CMDxSUB_URL:  // system message: url through the server
      {
        CEventUrl *e = CEventUrl::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, nMask);
        if (e == NULL)
        {
          char *buf;
          gLog.Warn("%sInvalid URL message:\n%s\n", L_WARNxSTR, packet.print(buf));
          delete []buf;
          break;
        }

        ICQUser* u = gUserManager.FetchUser(nUin, LOCK_W);
        if (u == NULL)
        {
          if (Ignore(IGNORE_NEWUSERS))
          {
            gLog.Info("%sURL from new user (%ld), ignoring.\n", L_SBLANKxSTR, nUin);
            RejectEvent(nUin, e);
            break;
          }
          gLog.Info("%sURL from new user (%ld).\n", L_SBLANKxSTR, nUin);
          AddUserToList(nUin);
          u = gUserManager.FetchUser(nUin, LOCK_W);
        }
        else
          gLog.Info("%sURL through server from %s (%ld).\n", L_SBLANKxSTR,
                    u->GetAlias(), nUin);

        if (AddUserEvent(u, e))
          m_xOnEventManager.Do(ON_EVENT_URL, u);
        gUserManager.DropUser(u);
        break;
      }
      case ICQ_CMDxSUB_AUTHxREQUEST:  // system message: authorisation request
      {
        gLog.Info("%sAuthorization request from %ld.\n", L_SBLANKxSTR, nUin);

        char **szFields = new char*[6];  // alias, first name, last name, email, auth, reason
        if (!ParseFE(szMessage, &szFields, 6))
        {
          char *buf;
          gLog.Warn("%sInvalid authorization request system message:\n%s\n", L_WARNxSTR,
                    packet.print(buf));
          delete []buf;
          delete []szFields;
          break;
        }

        // translating string with Translation Table
        gTranslator.ServerToClient (szFields[0]);
        gTranslator.ServerToClient (szFields[1]);
        gTranslator.ServerToClient (szFields[2]);
        gTranslator.ServerToClient (szFields[5]);

        CEventAuthRequest *e = new CEventAuthRequest(nUin, szFields[0], szFields[1],
                                                     szFields[2], szFields[3], szFields[5],
                                                     ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
        ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
        AddUserEvent(o, e);
        gUserManager.DropOwner();
        e->AddToHistory(NULL, D_RECEIVER);
        m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
        delete[] szFields;
        break;
      }
      case ICQ_CMDxSUB_AUTHxGRANTED:  // system message: authorized
      {
        gLog.Info("%sAuthorization granted by %ld.\n", L_SBLANKxSTR, nUin);

        // translating string with Translation Table
        gTranslator.ServerToClient (szMessage);

        CEventAuthGranted *e = new CEventAuthGranted(nUin, szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE,
                                                     timeSent, 0);
        ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
        AddUserEvent(o, e);
        gUserManager.DropOwner();
        e->AddToHistory(NULL, D_RECEIVER);
        m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
        break;
      }
      case ICQ_CMDxSUB_AUTHxREFUSED:  // system message : authorization refused
      {
        gLog.Info("%sAuthorization refused by %ld.\n", L_SBLANKxSTR, nUin);

        // Translating string with Translation Table
        gTranslator.ServerToClient(szMessage);

        CEventAuthRefused *e = new CEventAuthRefused(nUin, szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE,
                                                     timeSent, 0);

        ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
        AddUserEvent(o, e);
        gUserManager.DropOwner();
        e->AddToHistory(NULL, D_RECEIVER);
        m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
        break;
      }

      case ICQ_CMDxSUB_ADDEDxTOxLIST:  // system message: added to a contact list
      {
        gLog.Info("%sUser %ld added you to their contact list.\n", L_SBLANKxSTR,
                  nUin);

        char **szFields = new char*[6]; // alias, first name, last name, email, auth, text
        if (!ParseFE(szMessage, &szFields, 6))
        {
          char *buf;
          gLog.Warn("%sInvalid added to list system message:\n%s\n", L_WARNxSTR,
                    packet.print(buf));
          delete []buf;
          delete []szFields;
          break;
        }

        // translating string with Translation Table
        gTranslator.ServerToClient(szFields[0]);  // alias
        gTranslator.ServerToClient(szFields[1]);  // first name
        gTranslator.ServerToClient(szFields[2]);  // last name
        gLog.Info("%s%s (%s %s), %s\n", L_SRVxSTR, szFields[0], szFields[1],
                  szFields[2], szFields[3]);

        CEventAdded *e = new CEventAdded(nUin, szFields[0], szFields[1],
                                         szFields[2], szFields[3],
                                         ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
        ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
        AddUserEvent(o, e);
        gUserManager.DropOwner();
        e->AddToHistory(NULL, D_RECEIVER);
        m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
        delete[] szFields;

        // See if we should send a welcome greeting
        //...TODO

        break;
      }
      case ICQ_CMDxSUB_CONTACTxLIST:
      {
        CEventContactList *e = CEventContactList::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, nMask);
        if (e == NULL)
        {
          char *buf;
          gLog.Warn("%sInvalid Contact List message:\n%s\n", L_WARNxSTR, packet.print(buf));
          delete []buf;
          break;
        }

        ICQUser* u = gUserManager.FetchUser(nUin, LOCK_W);
        if (u == NULL)
        {
          if (Ignore(IGNORE_NEWUSERS))
          {
            gLog.Info("%s%d Contacts from new user (%ld), ignoring.\n",
                      L_SBLANKxSTR, e->Contacts().size(), nUin);
            RejectEvent(nUin, e);
            break;
          }
          gLog.Info("%s%d Contacts from new user (%ld).\n",
                    L_SBLANKxSTR, e->Contacts().size(), nUin);
          AddUserToList(nUin);
          u = gUserManager.FetchUser(nUin, LOCK_W);
        }
        else
          gLog.Info("%s%d Contacts through server from %s (%ld).\n",
                    L_SBLANKxSTR,e->Contacts().size(), u->GetAlias(), nUin);

        if (AddUserEvent(u, e))
          m_xOnEventManager.Do(ON_EVENT_MSG, u);
        gUserManager.DropUser(u);
        break;
      }

      default:
      {
        char *szFE;
        while((szFE = strchr(szMessage, 0xFE)) != NULL) *szFE = '\n';

        char *buf;
        gLog.Unknown("%sUnknown system message (0x%04x):\n%s\n", L_UNKNOWNxSTR,
                   newCommand, packet.print(buf));
        delete [] buf;
        CEventUnknownSysMsg *e =
          new CEventUnknownSysMsg(newCommand, ICQ_CMDxRCV_SYSxMSGxONLINE,
                                  nUin, szMessage, timeSent, 0);
        ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
        AddUserEvent(o, e);
        gUserManager.DropOwner();
      }
      }

      delete [] szMessage;
    }

    default:
      char *buf;
      gLog.Unknown("%sMessage through server with unknown format: %04hx\n%s\n", L_ERRORxSTR,
                   mFormat, packet.print(buf));
      delete [] buf;
      break;
    }
    break;
  }
  case ICQ_SNAXxMSG_RIGHTSxGRANTED:
    gLog.Info("%sReceived rights for Instant Messaging..\n", L_SRVxSTR);
    break;
  default:
    gLog.Warn("%sUnknown Message Family Subtype: %04hx\n", L_SRVxSTR, nSubtype);
    break;
  }
}

//--------ProcessBosFam---------------------------------------------
void CICQDaemon::ProcessBOSFam(CBuffer &packet, unsigned short nSubtype)
{
  switch (nSubtype)
  {
  case ICQ_SNACxBOS_RIGHTSxGRANTED:
    gLog.Info("%sReceived BOS rights.\n", L_SRVxSTR);
    //icqSetStatus(m_nDesiredStatus);
    break;

  default:
    gLog.Warn("%sUnknown BOS Family Subtype: %04hx\n", L_SRVxSTR, nSubtype);
    break;
  }
}

//--------ProcessVariousFam-----------------------------------------
void CICQDaemon::ProcessVariousFam(CBuffer &packet, unsigned short nSubtype)
{
  /*unsigned long Flags =*/ packet.UnpackUnsignedLongBE();
  unsigned short nSubSequence = packet.UnpackUnsignedShortBE();

  switch (nSubtype)
  {
  case 0x0003: // multi-purpose sub-type
  {
    unsigned short nLen, nType, nId;

    if (!packet.readTLV()) {
      char *buf;
      gLog.Unknown("%sUnknown server response:\n%s\n", L_UNKNOWNxSTR,
         packet.print(buf));
      delete buf;
      break;
    }

    CBuffer msg = packet.UnpackTLV(0x0001);

    if (msg.Empty()) {
      char *buf;
      gLog.Unknown("%sUnknown server response:\n%s\n", L_UNKNOWNxSTR,
         packet.print(buf));
      delete buf;
      break;
    }

    nLen = msg.UnpackUnsignedShortBE();
    msg.UnpackUnsignedLong(); // own UIN
    nType = msg.UnpackUnsignedShort();
    nId = msg.UnpackUnsignedShortBE(); // req-id, which we use to match requests for info and replies

    switch(nType) {
    case 0x0041:
    {
      struct tm sendTM;
      unsigned long nUin;
      nUin = msg.UnpackUnsignedLong();

      sendTM.tm_year = msg.UnpackUnsignedShort() - 1900;
      sendTM.tm_mon = msg.UnpackChar() - 1;
      sendTM.tm_mday = msg.UnpackChar();
      sendTM.tm_hour = msg.UnpackChar();
      sendTM.tm_min = msg.UnpackChar();
      sendTM.tm_sec = msg.UnpackChar();
      sendTM.tm_isdst = -1;
      msg.UnpackChar(); // flags

      // DAW fix the timezone problem
      char* szMessage = new char[msg.getDataMaxSize()];
      msg.UnpackString(szMessage); // 2 byte length little endian + string
      gLog.Info("%sgot Offline Message through server:\n", L_SRVxSTR);

      // now send the message to the user
      CEventMsg *e = CEventMsg::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, mktime(&sendTM), 0);
      delete [] szMessage;

      // Lock the user to add the message to their queue
      ICQUser* u = gUserManager.FetchUser(nUin, LOCK_W);
      if (u == NULL)
      {
        if (Ignore(IGNORE_NEWUSERS))
        {
          gLog.Info("%sMessage from new user (%ld), ignoring.\n", L_SBLANKxSTR, nUin);
          RejectEvent(nUin, e);
          break;
        }
        gLog.Info("%sMessage from new user (%ld).\n",
                  L_SBLANKxSTR, nUin);
        AddUserToList(nUin);
        u = gUserManager.FetchUser(nUin, LOCK_W);
      }
      else
        gLog.Info("%sMessage through server from %s (%ld).\n", L_SBLANKxSTR,
                  u->GetAlias(), nUin);

      if (AddUserEvent(u, e))
        m_xOnEventManager.Do(ON_EVENT_MSG, u);
      gUserManager.DropUser(u);

      break;
    }
    case 0x0042:
    {
      gLog.Info("%sEnd of Offline messages (nId: %04hx).\n", L_SRVxSTR, nId);
      CSrvPacketTcp* p = new CPU_SysMsgDoneAck(nId);
      SendEvent_Server(p);

      break;
    }
    case 0x07DA:
    {
      unsigned short nSubtype;
      unsigned char nResult;
      nSubtype = msg.UnpackUnsignedShort();
      nResult = msg.UnpackChar();

      if (nSubtype == ICQ_CMDxMETA_PASSWORDxRSP) {
    		if (nResult == META_SUCCESS) {
    			gLog.Info("%sPassword updated.\n", L_SRVxSTR);
 	   			ICQEvent *e = DoneExtendedServerEvent(nSubSequence, EVENT_SUCCESS);
    			ICQEvent *e2 = new ICQEvent(e);
    			e2->m_nCommand = ICQ_CMDxSND_META;
    			e2->m_nSubCommand = ICQ_CMDxMETA_PASSWORDxSET;
    			ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    			o->SetEnableSave(false);
    			o->SetPassword(((CPU_SetPassword *)e->m_pPacket)->m_szPassword);
    			o->SetEnableSave(true);
    			o->SaveLicqInfo();
    			gUserManager.DropOwner();
    			PushPluginEvent(e2);
    			DoneEvent(e, EVENT_SUCCESS);
    		}
    		else /* META_FAILURE */
    			gLog.Info("%sPassword not updated.\n", L_SRVxSTR);    	
      } else if (nSubtype == ICQ_CMDxMETA_SECURITYxRSP) {
		if (nResult == META_SUCCESS) {
    			gLog.Info("%sSecurity info updated.\n", L_SRVxSTR);
 	   		ICQEvent *e = DoneExtendedServerEvent(nSubSequence, EVENT_SUCCESS);
    			ICQEvent *e2 = new ICQEvent(e);
    			e2->m_nCommand = ICQ_CMDxSND_META;
    			e2->m_nSubCommand = ICQ_CMDxMETA_SECURITYxSET;
			CPU_Meta_SetSecurityInfo *p = (CPU_Meta_SetSecurityInfo *)e->m_pPacket;
    			ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    			o->SetEnableSave(false);
			o->SetAuthorization(p->Authorization());
			o->SetWebAware(p->WebAware());
			o->SetHideIp(p->HideIp());
    			o->SetEnableSave(true);
    			o->SaveLicqInfo();
    			gUserManager.DropOwner();
    			PushPluginEvent(e2);
    			DoneEvent(e, EVENT_SUCCESS);
    		}
    		else /* META_FAILURE */
    			gLog.Info("%sSecurity info not updated.\n", L_SRVxSTR);    	
      }
      // Search results need to be processed differently
      else if (nSubtype == 0x0190 || nSubtype == 0x019a || nSubtype == 0x01a4 || nSubtype == 0x01ae) {
      	ICQEvent *e = NULL;
      	
      	if (nResult == 0x32) { // No results found
      		gLog.Info("%sWP search found no users.\n", L_SRVxSTR);
      		e = DoneExtendedServerEvent(nSubSequence, EVENT_SUCCESS);
      		ICQEvent *e2 = new ICQEvent(e);
      		e2->m_pSearchAck = NULL; // Search ack is null lets plugins know no results found
      		e2->m_nCommand = ICQ_CMDxSND_META;
      	  e2->m_nSubCommand = ICQ_CMDxMETA_SEARCHxWPxLAST_USER;
      	  PushPluginEvent(e2);
      	  DoneEvent(e, EVENT_SUCCESS);
      	  break;
      	}
      	
     		e = DoneExtendedServerEvent(nSubSequence, EVENT_ACKED);
      		
      	if (e == NULL) {
      		gLog.Warn("%sUnmatched extended event (%d)!\n", L_WARNxSTR, nSubSequence);
      		break;
        }

        unsigned long nFoundUin;
        char szTemp[64];
        	
        msg.UnpackUnsignedShort(); // length of the rest of the packet.
        nFoundUin = msg.UnpackUnsignedLong();
        CSearchAck *s = new CSearchAck(nFoundUin);;
        	
        s->m_szAlias = strdup(msg.UnpackString(szTemp));
      	s->m_szFirstName = strdup(msg.UnpackString(szTemp));
      	s->m_szLastName = strdup(msg.UnpackString(szTemp));
      	s->m_szEmail = strdup(msg.UnpackString(szTemp));
      	msg.UnpackChar(); // authorization required
        s->m_nStatus = msg.UnpackChar();

      	// translating string with Translation Table
      	gTranslator.ServerToClient(s->m_szAlias);
      	gTranslator.ServerToClient(s->m_szFirstName);
      	gTranslator.ServerToClient(s->m_szLastName);
      	
      	gLog.Info("%s%s (%ld) <%s %s, %s>\n", L_SBLANKxSTR, s->m_szAlias, nFoundUin,
               s->m_szFirstName, s->m_szLastName, s->m_szEmail);

      	ICQEvent *e2 = new ICQEvent(e);
      	// JON: Hack it so it is backwards compatible with plugins for now.
      	e2->m_nCommand = ICQ_CMDxSND_META;
      	e2->m_pSearchAck = s;
      	if (nSubtype & 0x0008) {
      		unsigned long nMore = 0;
      		e2->m_nSubCommand = ICQ_CMDxMETA_SEARCHxWPxLAST_USER;
      		msg >> nMore;
      		e2->m_pSearchAck->m_nMore = nMore;
      		e2->m_eResult = EVENT_SUCCESS;
      	} else {
      		e2->m_nSubCommand = ICQ_CMDxMETA_SEARCHxWPxFOUND;
      		e2->m_pSearchAck->m_nMore = 0;
     	}
     		
      	PushPluginEvent(e2);
      	
      	if (nSubtype & 0x0008)
      		DoneEvent(e, EVENT_SUCCESS); // Remove it from the running event list
      	else
      		PushExtendedEvent(e);
      }
      else {
      	ICQEvent *e = NULL;
      	ICQUser *u = NULL;
      	unsigned long nUin = 0;
	bool multipart = false;
      	
      	if ((nResult == 0x32) || (nResult == 0x14) || (nResult == 0x1e)) {
				// error: empty result or nonexistent user (1E =  readonly???)
        	gLog.Info("%sFailed to update detail info: %x.\n", L_SRVxSTR, nResult);
        	e = DoneExtendedServerEvent(nSubSequence, EVENT_FAILED);
        	if (e != NULL) ProcessDoneEvent(e);
      	} else {
        	// Find the relevant event
        	e = DoneExtendedServerEvent(nSubSequence, EVENT_SUCCESS);
        	if (e == NULL) {
          		gLog.Warn("%sUnmatched extended event (%d)!\n", L_WARNxSTR, nSubSequence);
          		break;
        	}
        	nUin = e->Uin();

        	u = FindUserForInfoUpdate( nUin, e, "extended");
        	if (u == NULL) {
        		gLog.Warn("%scan't find user for updating!\n", L_WARNxSTR);
        		break;
       		}
       	}

      	switch (nSubtype) {
        case ICQ_CMDxMETA_GENERALxINFO:

	    gLog.Info("%sGeneral info on %s (%ld).\n", L_SRVxSTR, u->GetAlias(), u->Uin());

	    // main home info
	    u->SetAlias( msg.UnpackString() );
	    u->SetFirstName( msg.UnpackString() );
	    u->SetLastName( msg.UnpackString() );
	    u->SetEmailPrimary( msg.UnpackString() );
	    u->SetCity( msg.UnpackString() );
	    u->SetState( msg.UnpackString() );
	    u->SetPhoneNumber( msg.UnpackString() );
	    u->SetFaxNumber( msg.UnpackString() );
	    u->SetAddress( msg.UnpackString() );
	    u->SetCellularNumber( msg.UnpackString() );
	    u->SetZipCode( msg.UnpackString() );
	    u->SetCountryCode( msg.UnpackUnsignedShort() );
	    u->SetTimezone( msg.UnpackChar() );
	    u->SetHideEmail( msg.UnpackChar() ); // 0 = no, 1 = yes

	    // translating string with Translation Table
	    gTranslator.ServerToClient(u->GetAlias());
	    gTranslator.ServerToClient(u->GetFirstName());
	    gTranslator.ServerToClient(u->GetLastName());
	    gTranslator.ServerToClient(u->GetEmailPrimary());
	    gTranslator.ServerToClient(u->GetCity());
	    gTranslator.ServerToClient(u->GetState());
	    gTranslator.ServerToClient(u->GetPhoneNumber());
	    gTranslator.ServerToClient(u->GetFaxNumber());
	    gTranslator.ServerToClient(u->GetAddress());
	    gTranslator.ServerToClient(u->GetCellularNumber());
	    gTranslator.ServerToClient(u->GetZipCode());

	    // save the user infomation
	    u->SetEnableSave(true);
	    u->SaveGeneralInfo();

	    PushExtendedEvent(e);
	    multipart = true;

	    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_GENERAL, u->Uin()));

	    break;
	
	case ICQ_CMDxMETA_MORExINFO:

	    gLog.Info("%sMore info on %s (%ld).\n", L_SRVxSTR, u->GetAlias(), u->Uin());

	    u->SetEnableSave(false);
	    u->SetAge( msg.UnpackUnsignedShort() );
	    u->SetGender( msg.UnpackChar() );
	    u->SetHomepage( msg.UnpackString() );
	    u->SetBirthYear( msg.UnpackUnsignedShort() );
	    u->SetBirthMonth( msg.UnpackChar() );
	    u->SetBirthDay (msg.UnpackChar() );
	    u->SetLanguage1( msg.UnpackChar() );
	    u->SetLanguage2( msg.UnpackChar() );
	    u->SetLanguage3( msg.UnpackChar() );

	    // translating string with Translation Table
	    gTranslator.ServerToClient(u->GetHomepage());

	    // save the user infomation
	    u->SetEnableSave(true);
	    u->SaveMoreInfo();

	    PushExtendedEvent(e);
	    multipart = true;

	    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_MORE, u->Uin()));

	    break;

        case ICQ_CMDxMETA_EMAILxINFO:
        {
	    gLog.Info("%sEmail info on %s (%ld).\n", L_SRVxSTR, u->GetAlias(), u->Uin());

	    u->SetEnableSave(false);
	    // let's just grab the first 2 for now
	    int nEmail = (int)msg.UnpackChar();
	    if (nEmail > 2) nEmail = 2;
	    for(int i = 0; i < nEmail; i++) {
		msg.UnpackChar(); // publish email, not yet implemented
		if(i == 0) {
		    u->SetEmailPrimary( msg.UnpackString() );
		    gTranslator.ServerToClient(u->GetEmailPrimary());
		}
		if(i == 1) {
		    u->SetEmailSecondary( msg.UnpackString() );
		    gTranslator.ServerToClient(u->GetEmailSecondary());
		}
	    }

	    // save the user infomation
	    u->SetEnableSave(true);
	    u->SaveGeneralInfo();

	    PushExtendedEvent(e);
	    multipart = true;

	    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_EXT, u->Uin()));

	    break;
	}

        case 0x010E:
        {
	    // unknown

	    char* buf;

	    gLog.Unknown("%sunknown info: %04hx\n%s\n", L_UNKNOWNxSTR,
				    nSubSequence, packet.print(buf));
	    delete [] buf;

	    msg.UnpackUnsignedShortBE(); // WORD 0

	    PushExtendedEvent(e);
	    multipart = true;

	    break;
        }

        case ICQ_CMDxMETA_WORKxINFO:

	    gLog.Info("%sWork info on %s (%ld).\n", L_SRVxSTR, u->GetAlias(), u->Uin());

	    u->SetEnableSave(false);
	    u->SetCompanyCity( msg.UnpackString() );
	    u->SetCompanyState( msg.UnpackString() );
	    u->SetCompanyPhoneNumber( msg.UnpackString() );
	    u->SetCompanyFaxNumber( msg.UnpackString() );
	    u->SetCompanyAddress( msg.UnpackString() );  // u->SetCompanyStreet( msg.UnpackString() );
	    msg.UnpackString();        // u->SetCompanyZip( msg.UnpackString() );
	    msg.UnpackUnsignedShort(); // u->SetCompanyCountry( msg.UnpackUnsignedShort() );
	    u->SetCompanyName( msg.UnpackString() );
	    u->SetCompanyDepartment( msg.UnpackString() );
	    u->SetCompanyPosition( msg.UnpackString() );
	    msg.UnpackUnsignedShort(); // unknown
	    u->SetCompanyHomepage( msg.UnpackString() );

	    // translating string with Translation Table
	    gTranslator.ServerToClient(u->GetCompanyCity());
	    gTranslator.ServerToClient(u->GetCompanyState());
	    gTranslator.ServerToClient(u->GetCompanyPhoneNumber());
	    gTranslator.ServerToClient(u->GetCompanyFaxNumber());
	    gTranslator.ServerToClient(u->GetCompanyAddress());
	    gTranslator.ServerToClient(u->GetCompanyName());
	    gTranslator.ServerToClient(u->GetCompanyDepartment());
	    gTranslator.ServerToClient(u->GetCompanyPosition());
	    gTranslator.ServerToClient(u->GetCompanyHomepage());

	    // save the user infomation
	    u->SetEnableSave(true);
	    u->SaveWorkInfo();

	    PushExtendedEvent(e);
	    multipart = true;

	    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_WORK, u->Uin()));

	    break;
        
        case ICQ_CMDxMETA_ABOUT:
        {
	    gLog.Info("%sAbout info on %s (%ld).\n", L_SRVxSTR, u->GetAlias(), u->Uin());

	    char* rawmsg = msg.UnpackString();
	    char* msg = gTranslator.RNToN(rawmsg);
	    delete [] rawmsg;

	    u->SetAbout( msg );
	    delete [] msg;

	    // translating string with Translation Table
	    gTranslator.ServerToClient(u->GetAbout());

	    // save the user infomation
	    u->SetEnableSave(true);
	    u->SaveAboutInfo();

	    PushExtendedEvent(e);
	    multipart = true;

	    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_ABOUT, u->Uin()));

	    break;
	}

	case 0x00F0:
        {
	    // personal interests info

	    char * buf;

	    gLog.Unknown("%spersonal interests: %04hx\n%s\n", L_UNKNOWNxSTR,
				    nSubSequence, packet.print(buf));
	    delete [] buf;

	    PushExtendedEvent(e);
	    multipart = true;

	    break;
        }

        case ICQ_CMDxMETA_PASTxINFO:
        {
	    // past background info

	    char* buf;

	    gLog.Unknown("%spast backgrounds: %04hx\n%s\n", L_UNKNOWNxSTR,
				    nSubSequence, packet.print(buf));
	    delete [] buf;

	    break;
        }

        default: {
          char* buf;

          gLog.Unknown("%sunknown info: %04hx\n%s\n", L_UNKNOWNxSTR,
                       nSubtype, packet.print(buf));
          delete [] buf;
      	}
        }

	if (!multipart) {
	    if (e != NULL)
		ProcessDoneEvent(e);
	    else {
		gLog.Warn("%sResponse to unknown extended info request for %s (%ld).\n",
		L_WARNxSTR, u->GetAlias(), nUin);
	    }
	}

     	PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_EXT, u->Uin()));
       	gUserManager.DropUser(u);
      }
      break;
    }
    default: {
      char *buf;
      gLog.Unknown("%sUnknown SNAC 15,03 response type: %04hx\n%s\n", L_UNKNOWNxSTR,
                   nType, packet.print(buf));
      delete [] buf;
      break;
    }
    }
    break;
  }
  default:
    char *buf;
    gLog.Unknown("%sUnknown Various Family Subtype: %04hx\n%s\n", L_UNKNOWNxSTR,
                 nSubtype, packet.print(buf));
    delete [] buf;
    break;
  }
}

//--------ProcessDataChannel---------------------------------------------------

void CICQDaemon::ProcessDataChannel(CBuffer &packet)
{
  unsigned short nFamily, nSubtype;

  packet >> nFamily >> nSubtype;
  rev_e_short(nFamily);
  rev_e_short(nSubtype);

  switch (nFamily)
  {
  case ICQ_SNACxFAM_SERVICE:
    ProcessServiceFam(packet, nSubtype);
    break;

  case ICQ_SNACxFAM_LOCATION:
    ProcessLocationFam(packet, nSubtype);
    break;

  case ICQ_SNACxFAM_BUDDY:
    ProcessBuddyFam(packet, nSubtype);
    break;

  case ICQ_SNACxFAM_MESSAGE:
    ProcessMessageFam(packet, nSubtype);
    break;

  case ICQ_SNACxFAM_BOS:
    ProcessBOSFam(packet, nSubtype);
    break;

  case ICQ_SNACxFAM_VARIOUS:
    ProcessVariousFam(packet, nSubtype);
    break;

  default:
    gLog.Warn("%sUnknown Family on data channel: %04hx\n", L_SRVxSTR, nFamily);
    break;
  }
}

//--------ProcessCloseChannel--------------------------------------------------

bool CICQDaemon::ProcessCloseChannel(CBuffer &packet)
{
  int nSD = m_nTCPSrvSocketDesc;
  if (nSD < 0) {
    printf("nsd <0 !\n");
  }
  else {
    m_nTCPSrvSocketDesc = -1;
    gSocketManager.CloseSocket(nSD);
  }

  if (packet.getDataSize() == 0) {
    gLog.Info("%sWe're logging off..\n", L_SRVxSTR);
    return true;
  }

  if (!packet.readTLV()) {
    gLog.Error("%sError during parsing packet!\n", L_ERRORxSTR);
    return false;
  }

  unsigned short nError = packet.UnpackUnsignedShortTLV(0x0008);
  switch (nError) {
  case 0x18:
    gLog.Error("%sRate limit exceeded.\n", L_ERRORxSTR);
    m_eStatus = STATUS_OFFLINE_FORCED;
    m_bLoggingOn = false;

    break;

  case 0x05:
    gLog.Error("%sInvalid UIN and password combination.\n", L_ERRORxSTR);
    m_eStatus = STATUS_OFFLINE_FORCED;
    m_bLoggingOn = false;
    break;

  case 0:
    break;

  default:
    gLog.Error("%sUnknown sign on error: 0x%02X.\n", L_ERRORxSTR, nError);
  }

  if (nError)
  {
    packet.cleanupTLV();
    return false;
  }

  nError = packet.UnpackUnsignedShortTLV(0x0009);
  switch (nError)
  {
  case 0x0001:
    gLog.Error("%sYour ICQ number is used from another location.\n", L_ERRORxSTR);
    m_eStatus = STATUS_OFFLINE_FORCED;
    m_bLoggingOn = false;
    icqLogoff();
    break;

  case 0:
    break;

  default:
    gLog.Error("%sUnknown runtime error form server: 0x%02X.\n", L_ERRORxSTR, nError);
  }

  if (nError)
  {
    packet.cleanupTLV();
    return false;
  }

  char *szNewServer = packet.UnpackStringTLV(0x0005);
  char *szCookie = packet.UnpackStringTLV(0x0006);
  int nCookieLen = packet.getTLVLen(0x0006);

  if (!szNewServer || !szCookie)
  {
    gLog.Error("%sUnable to sign on: NewServer: %s, cookie: %s.\n", L_ERRORxSTR,
               szNewServer ? szNewServer : "(null)", szCookie ? szCookie : "(null)");
    if (szNewServer) delete [] szNewServer;
    if (szCookie) delete [] szCookie;
    return false;
  }

  packet.cleanupTLV();

  char* ptr;
  if ( (ptr = strchr(szNewServer, ':')))
    *ptr++ = '\0';

  gLog.Info("%sAuthenticated. Connecting to %s port %s.\n", L_SRVxSTR, szNewServer, ptr);

  // Connect to the new server here and send our cookie
  ConnectToServer(szNewServer, ptr ? atoi(ptr) : 5190);

  // Send our cookie
  CPU_SendCookie *p = new CPU_SendCookie(szCookie, nCookieLen);
  SendEvent_Server(p);

  delete [] szNewServer;
  delete [] szCookie;

  return true;
}
