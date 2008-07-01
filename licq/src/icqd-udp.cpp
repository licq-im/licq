// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream.h>

#include "time-fix.h"

#include "licq_icqd.h"
#include "licq_translate.h"
#include "licq_log.h"
#include "licq_user.h"
#include "licq_packets.h"

using namespace std;

//-----ConnectToServer----------------------------------------------------------
int CICQDaemon::ConnectToServer()
{
  // no servers!
  if (icqServers.current() == NULL)
  {
    gLog.Error("%sInternal error: Attempt to connect to server when no servers are set.\n",
               L_ERRORxSTR);
    return (-1);
  }

  // try and set the destination
  gLog.Info("%sResolving %s...\n", L_UDPxSTR, icqServers.current()->name());
  UDPSocket *s = new UDPSocket(0);
  if (!s->SetRemoteAddr(icqServers.current()->name(), icqServers.current()->port()))
  {
    char buf[128];
    gLog.Warn("%sUnable to resolve %s:\n%s%s.\n", L_ERRORxSTR,
              icqServers.current()->name(), L_BLANKxSTR, s->ErrorStr(buf, 128));
    delete s;
    return (-1);  // no route to host (not connected)
  }
  char ipbuf[32];
  gLog.Info("%sICQ server found at %s:%d.\n", L_UDPxSTR,
             s->RemoteIpStr(ipbuf), s->RemotePort());

  gLog.Info("%sCreating local server.\n", L_UDPxSTR);
  if (!s->StartServer(0))
  {
    char buf[128];
    gLog.Warn("%sUnable to start udp server:\n%s%s.\n", L_ERRORxSTR,
              L_BLANKxSTR, s->ErrorStr(buf, 128));
    delete s;
    return -1;
  }
  gLog.Info("%sOpening socket to server.\n", L_UDPxSTR);
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
  CPacket::SetIps(s);
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  o->SetIp(s->LocalIp());
  gUserManager.DropOwner(o);

  gSocketManager.AddSocket(s);
  m_nUDPSocketDesc = s->Descriptor();
  gSocketManager.DropSocket(s);
  write(pipe_newsocket[PIPE_WRITE], "S", 1);

  return m_nUDPSocketDesc;
}


//-----icqAddUser----------------------------------------------------------
void CICQDaemon::icqAddUser(unsigned long _nUin)
{
  CPU_ContactList *p = new CPU_ContactList(_nUin);
  gLog.Info("%sAlerting server to new user (#%hu)...\n", L_UDPxSTR,
            p->Sequence());
  SendExpectEvent_Server(p);

  // update the users info from the server
  icqUserBasicInfo(_nUin);
}


//-----icqAlertUser-------------------------------------------------------------
void CICQDaemon::icqAlertUser(unsigned long _nUin)
{
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  char sz[MAX_MESSAGE_SIZE];
  sprintf(sz, "%s%c%s%c%s%c%s%c%c%c", o->GetAlias(), 0xFE, o->GetFirstName(),
          0xFE, o->GetLastName(), 0xFE, o->GetEmailPrimary(), 0xFE,
          o->GetAuthorization() ? '0' : '1', 0xFE);
  gUserManager.DropOwner(o);
  CPU_ThroughServer *p = new CPU_ThroughServer(_nUin, ICQ_CMDxSUB_ADDEDxTOxLIST, sz);
  gLog.Info("%sAlerting user they were added (#%hu)...\n", L_UDPxSTR, p->Sequence());
  SendExpectEvent_Server(p);
}


//-----NextServer---------------------------------------------------------------
void CICQDaemon::SwitchServer()
{
  icqLogoff();
  icqServers.next();
}


void CICQDaemon::icqRegister(const char *_szPasswd)
{
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  o->SetPassword(_szPasswd);
  gUserManager.DropOwner(o);
  CPU_Register *p = new CPU_Register(_szPasswd);
  gLog.Info("%sRegistering a new user (#%hu)...\n", L_UDPxSTR, p->Sequence());
  m_nServerAck = p->Sequence() - 1;
  m_nServerSequence = 0;
  SendExpectEvent_Server(p);
}


//-----ICQ::Logon---------------------------------------------------------------
unsigned long CICQDaemon::icqLogon(unsigned short logonStatus)
{
  if (m_bLoggingOn)
  {
    gLog.Warn("%sAttempt to logon while already logged or logging on, logoff and try again.\n", L_WARNxSTR);
    return 0;
  }
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o->Uin() == 0)
  {
    gUserManager.DropOwner(o);
    gLog.Error("%sNo registered user, unable to process logon attempt.\n", L_ERRORxSTR);
    return 0;
  }
  if (o->Password()[0] == '\0')
  {
    gUserManager.DropOwner(o);
    gLog.Error(tr("%sNo password set.  Edit ~/.licq/owner.Licq and fill in the password field.\n"), L_ERRORxSTR);
    return 0;
  }
  char *passwd = strdup(o->Password());
  unsigned long status = o->AddStatusFlags(logonStatus);
  gUserManager.DropOwner(o);
  INetSocket *s = gSocketManager.FetchSocket(m_nTCPSocketDesc);
  if (s == NULL) return 0;
  CPU_Logon *p = new CPU_Logon(s->LocalPort(), passwd, status);
  gSocketManager.DropSocket(s);
  free (passwd);
  m_bOnlineNotifies = false;
  gLog.Info("%sRequesting logon (#%hu)...\n", L_UDPxSTR, p->Sequence());
  m_nServerAck = p->Sequence() - 1;
  m_nServerSequence = 0;
  m_nDesiredStatus = status;
  m_bLoggingOn = true;
  m_tLogonTime = time(NULL);
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----ICQ::icqRelogon-------------------------------------------------------
void CICQDaemon::icqRelogon(bool bChangeServer)
{
  unsigned long status;

  if (m_eStatus == STATUS_ONLINE)
  {
    ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
    status = o->StatusFull();
    gUserManager.DropOwner(o);
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


//-----ICQ::icqLogoff-----------------------------------------------------------
void CICQDaemon::icqLogoff()
{
  // Kill the udp socket asap to avoid race conditions
  int nSD = m_nUDPSocketDesc;
  m_nUDPSocketDesc = -1;
  gLog.Info("%sLogging off.\n", L_UDPxSTR);
  CPU_Logoff p;
  if (SendEvent(nSD, p, true))
    gSocketManager.CloseSocket(nSD);

  m_eStatus = STATUS_OFFLINE_MANUAL;
  m_bLoggingOn = false;

  pthread_mutex_lock(&mutex_runningevents);
  list<ICQEvent *>::iterator iter = m_lxRunningEvents.begin();
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

  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  ChangeUserStatus(o, ICQ_STATUS_OFFLINE);
  gUserManager.DropOwner(o);
  PushPluginSignal(new CICQSignal(SIGNAL_LOGOFF, 0, 0));
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
      CPU_ContactList *p = new CPU_ContactList(uins);
      gLog.Info("%sUpdating contact list (#%hu)...\n", L_UDPxSTR, p->Sequence());
      SendExpectEvent_Server(p);
      uins.erase(uins.begin(), uins.end());
      n = 0;
    }
    // Reset all users to offline
    if (!pUser->StatusOffline()) ChangeUserStatus(pUser, ICQ_STATUS_OFFLINE);
  }
  FOR_EACH_USER_END
  if (n != 0)
  {
    CPU_ContactList *p = new CPU_ContactList(uins);
    gLog.Info("%sUpdating contact list (#%hu)...\n", L_UDPxSTR, p->Sequence());
    SendExpectEvent_Server(p);
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
  if (uins.size() == 0) return;

  CPU_VisibleList *p = new CPU_VisibleList(uins);
  gLog.Info("%sSending visible list (#%hu)...\n", L_UDPxSTR, p->Sequence());
  SendExpectEvent_Server(p);
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
  if (uins.size() == 0) return;

  CPU_InvisibleList *p = new CPU_InvisibleList(uins);
  gLog.Info("%sSending invisible list (#%hu)...\n", L_UDPxSTR, p->Sequence());
  SendExpectEvent_Server(p);
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
  CPU_ModifyViewList *p = new CPU_ModifyViewList(nUin, true, true);
  gLog.Info("%sAdding user %ld to visible list (#%hu)...\n", L_UDPxSTR, nUin,
     p->Sequence());
  SendExpectEvent_Server(p);
}

void CICQDaemon::icqRemoveFromVisibleList(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u != NULL)
  {
    u->SetVisibleList(false);
    gUserManager.DropUser(u);
  }
  CPU_ModifyViewList *p = new CPU_ModifyViewList(nUin, true, false);
  gLog.Info("%sRemoving user %ld from visible list (#%hu)...\n", L_UDPxSTR, nUin,
     p->Sequence());
  SendExpectEvent_Server(p);
}

void CICQDaemon::icqAddToInvisibleList(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u != NULL)
  {
    u->SetInvisibleList(true);
    gUserManager.DropUser(u);
  }
  CPU_ModifyViewList *p = new CPU_ModifyViewList(nUin, false, true);
  gLog.Info("%sAdding user %ld to invisible list (#%hu)...\n", L_UDPxSTR, nUin,
     p->Sequence());
  SendExpectEvent_Server(p);
}

void CICQDaemon::icqRemoveFromInvisibleList(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u != NULL)
  {
    u->SetInvisibleList(false);
    gUserManager.DropUser(u);
  }
  CPU_ModifyViewList *p = new CPU_ModifyViewList(nUin, false, false);
  gLog.Info("%sRemoving user %ld from invisible list (#%hu)...\n", L_UDPxSTR, nUin,
     p->Sequence());
  SendExpectEvent_Server(p);
}

//-----icqSearchByUin-----------------------------------------------------------
unsigned long CICQDaemon::icqSearchByUin(unsigned long nUin)
{
  CPU_SearchByUin *p = new CPU_SearchByUin(nUin);
  gLog.Info("%sStarting search by uin for user (#%hu/#%d)...\n", L_UDPxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}

//-----icqSetRandomChatGroup-------------------------------------------------
unsigned long CICQDaemon::icqSetRandomChatGroup(unsigned long nGroup)
{
  CPU_SetRandomChatGroup *p = new CPU_SetRandomChatGroup(nGroup);
  gLog.Info("%sSetting random chat group (#%hu)...\n", L_UDPxSTR,
            p->Sequence());
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqRandomChatSearch---------------------------------------------------
unsigned long CICQDaemon::icqRandomChatSearch(unsigned long nGroup)
{
  CPU_RandomChatSearch *p = new CPU_RandomChatSearch(nGroup);
  gLog.Info("%sSearching for random chat user (#%hu)...\n", L_UDPxSTR,
            p->Sequence());
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqPing------------------------------------------------------------------
void CICQDaemon::icqPing()
{
  CPU_Ping *p = new CPU_Ping;
  gLog.Info("%sPinging Server (#%hu)...\n", L_UDPxSTR, p->Sequence());
  SendExpectEvent_Server(p);
}


//-----icqSetStatus-------------------------------------------------------------
unsigned long CICQDaemon::icqSetStatus(unsigned short newStatus)
{
  // Set the status flags
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  unsigned long s = o->AddStatusFlags(newStatus);
  bool invisible = o->StatusInvisible();
  gUserManager.DropOwner(o);
  CPU_SetStatus *p = new CPU_SetStatus(s);
  gLog.Info("%sChanging status to %s (#%hu)...\n", L_UDPxSTR,
            ICQUser::StatusToStatusStr(newStatus, invisible), p->Sequence());
  m_nDesiredStatus = s;
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqGetUserBasicInfo------------------------------------------------------
unsigned long CICQDaemon::icqUserBasicInfo(unsigned long _nUin)
{
  CPU_GetUserBasicInfo *p = new CPU_GetUserBasicInfo(_nUin);
  gLog.Info("%sRequesting user info (#%hu/#%d)...\n", L_UDPxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqGetUserExtInfo--------------------------------------------------------
unsigned long CICQDaemon::icqUserExtendedInfo(unsigned long _nUin)
{
  CPU_GetUserExtInfo *p = new CPU_GetUserExtInfo(_nUin);
  gLog.Info("%sRequesting extended user info (#%hu/#%d)...\n", L_UDPxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqUpdatePersonalBasicInfo-----------------------------------------------
unsigned long CICQDaemon::icqUpdateBasicInfo(const char *_sAlias, const char *_sFirstName,
                                     const char *_sLastName, const char *_sEmail,
                                     bool _bAuthorization)
{
  CPU_UpdatePersonalBasicInfo *p =
    new CPU_UpdatePersonalBasicInfo(_sAlias, _sFirstName, _sLastName, _sEmail,
                                    (char)_bAuthorization);
  gLog.Info("%sUpdating personal information (#%hu/#%d)...\n", L_UDPxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqUpdatePersonalExtInfo-------------------------------------------------
unsigned long CICQDaemon::icqUpdateExtendedInfo(const char *_sCity, unsigned short _nCountry,
                                   const char *_sState, unsigned short _nAge,
                                   char _cSex, const char *_sPhone,
                                   const char *_sHomepage, const char *_sAbout,
                                   const char *szZipcode)
{
  CPU_UpdatePersonalExtInfo *p =
    new CPU_UpdatePersonalExtInfo(_sCity, _nCountry, _sState, _nAge, _cSex,
                                  _sPhone, _sHomepage, _sAbout, atol(szZipcode));
   gLog.Info("%sUpdating personal extended info (#%hu/#%d)...\n", L_UDPxSTR,
             p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqSetWorkInfo--------------------------------------------------------
unsigned long CICQDaemon::icqSetWorkInfo(const char *_szCity, const char *_szState,
                                     const char *_szPhone,
                                     const char *_szFax, const char *_szAddress,
                                     const char *_szName, const char *_szDepartment,
                                     const char *_szPosition, const char *_szHomepage)
{
  CPU_Meta_SetWorkInfo *p =
    new CPU_Meta_SetWorkInfo(_szCity, _szState, _szPhone, _szFax, _szAddress,
                             _szName, _szDepartment, _szPosition, _szHomepage);
  gLog.Info("%sUpdating work info (#%hu/#%d)...\n", L_UDPxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqSetGeneralInfo----------------------------------------------------
unsigned long CICQDaemon::icqSetGeneralInfo(
                          const char *szAlias, const char *szFirstName,
                          const char *szLastName, const char *szEmailPrimary,
                          const char *szEmailSecondary, const char *szEmailOld, const char *szCity,
                          const char *szState, const char *szPhoneNumber,
                          const char *szFaxNumber, const char *szAddress,
                          const char *szCellularNumber, const char *szZipCode,
                          unsigned short nCountryCode, bool bHideEmail)
{
  CPU_Meta_SetGeneralInfo *p =
    new CPU_Meta_SetGeneralInfo(szAlias, szFirstName,
                                szLastName, szEmailPrimary,
                                szEmailSecondary, szEmailOld, szCity,
                                szState, szPhoneNumber,
                                szFaxNumber, szAddress,
                                szCellularNumber, szZipCode,
                                nCountryCode, bHideEmail);

  gLog.Info("%sUpdating general info (#%hu/#%d)...\n", L_UDPxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqSetMoreInfo----------------------------------------------------
unsigned long CICQDaemon::icqSetMoreInfo(unsigned short nAge,
                              char nGender, const char *szHomepage,
                              unsigned short nBirthYear, char nBirthMonth,
                              char nBirthDay, char nLanguage1,
                              char nLanguage2, char nLanguage3)
{
  CPU_Meta_SetMoreInfo *p =
    new CPU_Meta_SetMoreInfo(nAge, nGender, szHomepage,
                             nBirthYear, nBirthMonth,
                             nBirthDay, nLanguage1,
                             nLanguage2, nLanguage3);

  gLog.Info("%sUpdating more info (#%hu/#%d)...\n", L_UDPxSTR,
            p->Sequence(), p->SubSequence());

  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqSetSecurityInfo----------------------------------------------------
unsigned long CICQDaemon::icqSetSecurityInfo(bool bAuthorize, bool bHideIp, bool bWebAware)
{
  CPU_Meta_SetSecurityInfo *p =
    new CPU_Meta_SetSecurityInfo(bAuthorize, bHideIp, bWebAware);
  gLog.Info("%sUpdating security info (#%hu/#%d)...\n", L_UDPxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqSetAbout-----------------------------------------------------------
unsigned long CICQDaemon::icqSetAbout(const char *_szAbout)
{
  char *szAbout = gTranslator.NToRN(_szAbout);
  CPU_Meta_SetAbout *p = new CPU_Meta_SetAbout(szAbout);
  gLog.Info("%sUpdating about (#%hu/#%d)...\n", L_UDPxSTR,
            p->Sequence(), p->SubSequence());
  delete [] szAbout;
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqSetPassword--------------------------------------------------------
unsigned long CICQDaemon::icqSetPassword(const char *szPassword)
{
  CPU_Meta_SetPassword *p = new CPU_Meta_SetPassword(szPassword);
  gLog.Info("%sUpdating password (#%hu/#%d)...\n", L_UDPxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqRequestMetaInfo----------------------------------------------------
unsigned long CICQDaemon::icqRequestMetaInfo(unsigned long nUin)
{
  CPU_Meta_RequestAllInfo *p = new CPU_Meta_RequestAllInfo(nUin);
  gLog.Info("%sRequesting meta info for %ld (#%hu/#%d)...\n", L_UDPxSTR, nUin,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


///-----icqAuthorize-------------------------------------------------------------
unsigned long CICQDaemon::icqAuthorizeGrant(unsigned long uinToAuthorize, const char *szMessage)
// authorize a user to add you to their contact list
{
  char *sz = NULL;
  if (szMessage != NULL)
  {
    sz = gTranslator.NToRN(szMessage);
    gTranslator.ClientToServer(sz);
  }
  CPU_ThroughServer *p = new CPU_ThroughServer(uinToAuthorize, ICQ_CMDxSUB_AUTHxGRANTED, sz);
  gLog.Info("%sAuthorizing user %ld (#%hu)...\n", L_UDPxSTR, uinToAuthorize,
     p->Sequence());
  delete sz;

  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}


//-----icqAuthorize-------------------------------------------------------------
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
  gLog.Info("%sRefusing authorization to user %ld (#%hu)...\n", L_UDPxSTR,
     nUin, p->Sequence());
  delete sz;

  ICQEvent *e = SendExpectEvent_Server(p);
  return e->EventId();
}




//-----icqRequestSystemMsg---------------------------------------------------
// request offline system messages
void CICQDaemon::icqRequestSystemMsg()
{
  CPU_RequestSysMsg *p = new CPU_RequestSysMsg;
  gLog.Info("%sSending offline message request (#%hu)...\n", L_UDPxSTR,
            p->Sequence());
  SendExpectEvent_Server(p);
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
  gLog.Info("%sReceived %s information for %s (%ld).\n", L_UDPxSTR, t,
            u->GetAlias(), nUin);
  return u;
}


//-----ProcessUdpPacket---------------------------------------------------------
unsigned short CICQDaemon::ProcessUdpPacket(UDPSocket *udp, unsigned short bMultiPacket)
{
  unsigned short version, nCommand, nSequence, nSubSequence,
                 junkShort;
  unsigned long nUin, nOwnerUin;
  char junkChar;

  CBuffer &packet = udp->RecvBuffer();

  // read in the standard UDP header info
  packet >> version;

#if ICQ_VERSION == 2
  if (version != 2)
#elif ICQ_VERSION == 4 || ICQ_VERSION == 5
  if (version != 3 && version != 5)
#endif
  {
    gLog.Warn("%sServer send bad version number: %d.\n", L_WARNxSTR, version);
    return(0xFFFF);
  }

#if ICQ_VERSION == 2
  packet >> nCommand
         >> nSequence;
#elif ICQ_VERSION == 4 || ICQ_VERSION == 5
  unsigned long nSessionId = 0, nCheckSum = 0;
  if (version == 5)
  {
    packet >> junkChar
           >> nSessionId
           >> nCommand
           >> nSequence
           >> nSubSequence
           >> nOwnerUin
           >> nCheckSum;
  }
  else if (version == 3)
  {
    packet >> nCommand
           >> nSequence
           >> nSubSequence
           >> nOwnerUin
           >> nCheckSum;
  }
#endif

  if (nCommand != ICQ_CMDxRCV_ACK && nCommand != ICQ_CMDxRCV_ERROR)
  {
    if (nSequence == m_nServerSequence + 1 ||
        (m_nServerSequence == 0 && nSequence == 0) ||
        (bMultiPacket && nSequence == m_nServerSequence) )
    {
      m_nServerSequence = nSequence;
    }
    else if (nSequence > m_nServerSequence + 1)
    {
      gLog.Warn("%sOut of order packet received, command %d (#%d, expecting #%d).\n",
                  L_WARNxSTR, nCommand, nSequence, m_nServerSequence + 1);
      return nCommand;
    }
    else
    {
      gLog.Warn("%sDuplicate packet received, command %d (#%d, expecting #%d).\n",
                L_WARNxSTR, nCommand, nSequence, m_nServerSequence + 1);
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      return nCommand;
    }
  }

  switch (nCommand)
  {
    case ICQ_CMDxRCV_MULTIxPACKET:  // Multi-packet
    {
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      unsigned char nPackets;
      unsigned short nLen;
      char *buf;
      packet >> nPackets;

      gLog.Info("%sMultiPacket (%d sub-packets).\n", L_UDPxSTR, nPackets);

      for (unsigned short i = 0; i < nPackets; i++)
      {
        packet >> nLen;
        buf = packet.getDataPosRead() + nLen;
        if (!ProcessUdpPacket(udp, nLen)) return false;
        packet.setDataPosRead(buf);
      }
      break;
    }
    case ICQ_CMDxRCV_USERxONLINE:   // initial user status packet
    {
      /* 02 00 6E 00 0B 00 8F 76 20 00 CD CD 77 90 3F 50 00 00 7F 00 00 01 04 00
         00 00 00 03 00 00 00 */
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      packet >> nUin;

      // find which user it is, verify we have them on our list
      ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
      if (u == NULL)
      {
         gLog.Warn("%sUnknown user (%ld) is online.\n", L_WARNxSTR, nUin);
         break;
      }

      // read in the relevant user information
      unsigned short userPort;
      unsigned long userIP, realIP, newStatus, tcpVersion;
      unsigned long timestamp;
      char mode;
      packet >> userIP
             >> userPort
             >> junkShort
             >> realIP
             >> mode
             >> newStatus  // initial status of user
             >> tcpVersion
      ;
      packet.UnpackUnsignedLong();
      packet.UnpackUnsignedLong();
      packet.UnpackUnsignedLong();
      timestamp = packet.UnpackUnsignedLong();

      if((timestamp & 0xFFFF0000) == LICQ_WITHSSL)
          gLog.Info("%s%s (%ld) went online (v%01lx) [Licq %s/SSL].\n",
                    L_UDPxSTR, u->GetAlias(), nUin, tcpVersion & 0x0F,
                    CUserEvent::LicqVersionToString(timestamp & 0xFFFF));
      else if((timestamp & 0xFFFF0000) == LICQ_WITHOUTSSL)
          gLog.Info("%s%s (%ld) went online (v%01lx) [Licq %s].\n",
                    L_UDPxSTR, u->GetAlias(), nUin, tcpVersion & 0x0F,
                    CUserEvent::LicqVersionToString(timestamp & 0xFFFF));
      else
          gLog.Info("%s%s (%ld) went online (v%01lx).\n", L_UDPxSTR, u->GetAlias(),
                    nUin, tcpVersion & 0x0F);

      // The packet class will spit out an ip in network order on a little
      // endian machine and in little-endian on a big endian machine
      userIP = PacketIpToNetworkIp(userIP);
      realIP = PacketIpToNetworkIp(realIP);
      u->SetIpPort(userIP, userPort);
      u->SetRealIp(realIP);
      // What is mode 1?  We can't connect direct...
      if (mode == 1 || mode == 6) mode = MODE_INDIRECT;
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
      u->SetVersion(tcpVersion);
      u->SetClientTimestamp(timestamp);
      ChangeUserStatus(u, newStatus);
      u->SetAutoResponse(NULL);
      u->SetShowAwayMsg(false);
      if ((m_bOnlineNotifies || m_bAlwaysOnlineNotify) && u->OnlineNotify())
        m_xOnEventManager.Do(ON_EVENT_NOTIFY, u);
      gUserManager.DropUser(u);

      /*
      // Test of going online bytes
      unsigned char c1 = 0;
      if (bMultiPacket == 0)
      {
        unsigned short i = 0;
        while (!packet.End())
        {
          packet >> c1;
          printf("%02X", c1);
          i++;
          if (i % 4 == 0) printf(" | ");
          if (i % 28 == 0) printf("\n");
        }
      }
      else
      {
        unsigned short j = 0;
        for (unsigned short i = 46; i < bMultiPacket; i++)
        {
          packet >> c1;
          printf("%02X", c1);
          j++;
          if (j % 4 == 0) printf(" | ");
          if (j % 28 == 0) { printf("\n"); }
        }
      }
      printf("\n");
      */
      break;
    }

    case ICQ_CMDxRCV_USERxOFFLINE:  // user just went offline packet
    {
      /* 02 00 78 00 06 00 ED 21 4E 00 */
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      packet >> nUin;

      // find which user it is, verify we have them on our list
      ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
      if (u == NULL)
      {
         gLog.Warn("%sUnknown user (%ld) has gone offline.\n", L_WARNxSTR, nUin);
         break;
      }
      gLog.Info("%s%s (%ld) went offline.\n", L_UDPxSTR, u->GetAlias(), nUin);
      u->SetClientTimestamp(0);
      ChangeUserStatus(u, ICQ_STATUS_OFFLINE);
      gUserManager.DropUser(u);
      break;
    }

    case ICQ_CMDxRCV_USERxINFO:   // user info packet
    {
      /* 02 00 18 01 6C 00 10 00 50 A5 82 00 08 00 48 61 63 6B 49 43 51 00 04 00
         46 6F 6F 00 04 00 42 61 72 00 15 00 68 61 63 6B 65 72 73 40 75 77 61 74
         65 72 6C 6F 6F 2E 63 61 00 00 00 00  */
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
  #if ICQ_VERSION == 2
      packet >> nSubSequence;  // corresponds to the sequence number from the user information request packet...totally irrelevant.
  #endif
      packet >> nUin;

      // Find the relevant event
      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_USERxGETINFO, nSubSequence, EVENT_SUCCESS);

      // Find which user it is, verify we have them on our list
      ICQUser *u = FindUserForInfoUpdate(nUin, e, "basic");
      if (u == NULL) break;

      // read in the four data fields; alias, first name, last name, and email address
      u->SetEnableSave(false);
      char temp[MAX_DATA_LEN], cAuthorization;
      u->SetAlias(packet.UnpackString(temp, sizeof(temp)));
      u->SetFirstName(packet.UnpackString(temp, sizeof(temp)));
      u->SetLastName(packet.UnpackString(temp, sizeof(temp)));
      u->SetEmailPrimary(packet.UnpackString(temp, sizeof(temp)));
      packet >> cAuthorization;
      u->SetAuthorization(cAuthorization == 0 ? true : false);

      // translating string with Translation Table
      gTranslator.ServerToClient(u->GetAlias());
      gTranslator.ServerToClient(u->GetFirstName());
      gTranslator.ServerToClient(u->GetLastName());

      // save the user infomation
      u->SetEnableSave(true);
      u->SaveBasicInfo();

      /*ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_USERxGETINFO, nSubSequence, EVENT_SUCCESS);*/
      if (e != NULL)
        ProcessDoneEvent(e);
      else
      {
        gLog.Warn("%sResponse to unknown basic info request for %s (%ld).\n",
           L_WARNxSTR, u->GetAlias(), nUin);
      }
      // Good question...do we send the signal when the user isn't on the list?
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_BASIC, u->Uin()));
      gUserManager.DropUser(u);
      break;
    }

    case ICQ_CMDxRCV_USERxDETAILS:   // user details packet
    {
      /* 02 00 22 01 09 00 0A 00 96 3D 44 00 04 00 3F 3F 3F 00 2E 00 FE 01 00 00
         FF FF 01 01 00 00 1C 00 68 74 74 70 3A 2F 2F 68 65 6D 2E 70 61 73 73 61
         67 65 6E 2E 73 65 2F 67 72 72 2F 00 1E 00 49 27 6D 20 6A 75 73 74 20 61
         20 67 69 72 6C 20 69 6E 20 61 20 77 6F 72 6C 64 2E 2E 2E 00 FF FF FF FF */

      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
  #if ICQ_VERSION == 2
      packet >> nSubSequence;  // corresponds to the sequence number from the user information request packet
  #endif
      packet >> nUin;

      // Find the event
      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_USERxGETDETAILS, nSubSequence, EVENT_SUCCESS);

      // find which user it is, verify we have them on our list
      ICQUser *u = FindUserForInfoUpdate(nUin, e, "extended");
      if (u == NULL) break;

      /*ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
      if (u == NULL)
      {
         gLog.Warn("%sExtended information on unknown user (%d).\n", L_WARNxSTR, nUin);
         break;
      }
      gLog.Info("%sReceived extended information for %s (%ld).\n", L_UDPxSTR,
                u->GetAlias(), nUin);*/

      u->SetEnableSave(false);
      char sTemp[MAX_MESSAGE_SIZE];

      u->SetCity(packet.UnpackString(sTemp, sizeof(sTemp)));
      u->SetCountryCode(packet.UnpackUnsignedShort());
      u->SetTimezone(packet.UnpackChar());
      u->SetState(packet.UnpackString(sTemp, sizeof(sTemp)));
      u->SetAge(packet.UnpackUnsignedShort());
      u->SetGender(packet.UnpackChar());
      u->SetPhoneNumber(packet.UnpackString(sTemp, sizeof(sTemp)));
      u->SetHomepage(packet.UnpackString(sTemp, sizeof(sTemp)));
      u->SetAbout(packet.UnpackString(sTemp, sizeof(sTemp)));
      sprintf(sTemp, "%ld", packet.UnpackUnsignedLong());
      u->SetZipCode(sTemp);

      // translating string with Translation Table
      gTranslator.ServerToClient(u->GetCity());
      gTranslator.ServerToClient(u->GetState());
      gTranslator.ServerToClient(u->GetPhoneNumber());
      gTranslator.ServerToClient(u->GetHomepage());
      gTranslator.ServerToClient(u->GetAbout());

      // save the user infomation
      u->SetEnableSave(true);
      u->SaveExtInfo();

      //ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_USERxGETDETAILS, nSubSequence, EVENT_SUCCESS);
      if (e != NULL)
        ProcessDoneEvent(e);
      else
      {
        gLog.Warn("%sResponse to unknown extended info request for %s (%ld).\n",
           L_WARNxSTR, u->GetAlias(), nUin);
      }
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_EXT, u->Uin()));
      gUserManager.DropUser(u);
      break;
    }

    case ICQ_CMDxRCV_UPDATEDxBASIC:
    {
      /* 02 00 B4 00 28 00 01 00 */
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
  #if ICQ_VERSION == 2
      packet >> nSubSequence;
  #endif
      gLog.Info("%sSuccessfully updated basic info.\n", L_UDPxSTR);
      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_UPDATExBASIC, nSubSequence, EVENT_SUCCESS);
      CPU_UpdatePersonalBasicInfo *p = (CPU_UpdatePersonalBasicInfo *)e->m_pPacket;
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      o->SetAlias(p->Alias());
      o->SetFirstName(p->FirstName());
      o->SetLastName(p->LastName());
      o->SetEmailPrimary(p->Email());
      o->SetAuthorization(p->Authorization());

      // translating string with Translation Table
      gTranslator.ServerToClient(o->GetAlias());
      gTranslator.ServerToClient(o->GetFirstName());
      gTranslator.ServerToClient(o->GetLastName());

      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                      USER_BASIC, o->Uin()));
      gUserManager.DropOwner(o);
      if (e != NULL) ProcessDoneEvent(e);
      break;
    }

    case ICQ_CMDxRCV_UPDATExBASICxFAIL:
    {
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      gLog.Info("%sFailed to update basic info.\n", L_UDPxSTR);
  #if ICQ_VERSION == 2
      packet >> nSubSequence;
  #endif
      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_UPDATExBASIC, nSubSequence, EVENT_FAILED);
      if (e != NULL) ProcessDoneEvent(e);
      break;
    }

    case ICQ_CMDxRCV_UPDATEDxDETAIL:
    {
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      gLog.Info("%sSuccessfully updated detail info.\n", L_UDPxSTR);
  #if ICQ_VERSION == 2
      packet >> nSubSequence;
  #endif
      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_UPDATExDETAIL, nSubSequence, EVENT_SUCCESS);
      CPU_UpdatePersonalExtInfo *p = (CPU_UpdatePersonalExtInfo *)e->m_pPacket;
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      o->SetCity(p->City());
      o->SetCountryCode(p->Country());
      o->SetState(p->State());
      o->SetAge(p->Age());
      o->SetGender(p->Sex());
      o->SetPhoneNumber(p->PhoneNumber());
      o->SetHomepage(p->Homepage());
      o->SetAbout(p->About());
      char sz[32];
      sprintf(sz, "%ld", p->Zipcode());
      o->SetZipCode(sz);
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                      USER_EXT, o->Uin()));
      gUserManager.DropOwner(o);
      if (e != NULL) ProcessDoneEvent(e);
      break;
    }

    case ICQ_CMDxRCV_UPDATExDETAILxFAIL:
    {
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      gLog.Info("%sFailed to update detail info.\n", L_UDPxSTR);
  #if ICQ_VERSION == 2
      packet >> nSubSequence;
  #endif
      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_UPDATExDETAIL, nSubSequence, EVENT_FAILED);
      if (e != NULL) ProcessDoneEvent(e);
      break;
    }

    case ICQ_CMDxRCV_USERxINVALIDxUIN:  // not a good uin
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      packet >> nUin;
      gLog.Info("%sInvalid UIN: %ld.\n", L_UDPxSTR, nUin);
      // we need to do something here, but I bet the command is included in the packet
      break;

    case ICQ_CMDxRCV_USERxSTATUS:  // user changed status packet
    {
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      packet >> nUin;

      // find which user it is, verify we have them on our list
      unsigned long nNewStatus;
      packet >> nNewStatus;
      ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
      if (u == NULL)
      {
        gLog.Warn("%sUnknown user (%ld) changed status.\n", L_WARNxSTR,
                  nUin);
        break;
      }
      ChangeUserStatus(u, nNewStatus);
      gLog.Info("%s%s (%ld) changed status: %s.\n", L_UDPxSTR,
                u->GetAlias(), nUin, u->StatusStr());
      nNewStatus &= ICQ_STATUS_FxUNKNOWNxFLAGS;
      if (nNewStatus)
      {
        gLog.Unknown("%sUnknown status flag for %s (%ld): 0x%08lX\n",
           L_UNKNOWNxSTR, u->GetAlias(), nUin, nNewStatus);
      }
      gUserManager.DropUser(u);
      break;
    }

    case ICQ_CMDxRCV_USERxLISTxDONE:  // end of user list
      /* 02 00 1C 02 05 00 8F 76 20 00 */
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      m_bOnlineNotifies = true;
      gLog.Info("%sLogon complete.\n", L_UDPxSTR);
      break;

    case ICQ_CMDxRCV_SEARCHxFOUND:  // user found in search
    {
      /* 02 00 8C 00 03 00 05 00 8F 76 20 00 0B 00 41 70 6F 74 68 65 6F 73 69 73
         00 07 00 47 72 61 68 61 6D 00 05 00 52 6F 66 66 00 13 00 67 72 6F 66 66 40 75
         77 61 74 65 72 6C 6F 6F 2E 63 61 00 01 02 */
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      gLog.Info("%sSearch found user:\n", L_UDPxSTR);

      char auth;
      char szTemp[64];
  #if ICQ_VERSION == 2
      packet >> nSubSequence;
  #endif
      packet >> nUin;
      CSearchAck *s = new CSearchAck(nUin);

      s->m_szAlias = strdup(packet.UnpackString(szTemp, sizeof(szTemp)));
      s->m_szFirstName = strdup(packet.UnpackString(szTemp, sizeof(szTemp)));
      s->m_szLastName = strdup(packet.UnpackString(szTemp, sizeof(szTemp)));
      s->m_szEmail = strdup(packet.UnpackString(szTemp, sizeof(szTemp)));

      // translating string with Translation Table
      gTranslator.ServerToClient(s->m_szAlias);
      gTranslator.ServerToClient(s->m_szFirstName);
      gTranslator.ServerToClient(s->m_szLastName);

      packet >> auth;

      // Their status... 0x00 = Offline, 0x01 = Online, 0x02 = Disabled
      s->m_nStatus = packet.UnpackChar();

      gLog.Info("%s%s (%ld) <%s %s, %s>\n", L_SBLANKxSTR, s->m_szAlias, nUin,
                s->m_szFirstName, s->m_szLastName, s->m_szEmail);

      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_SEARCHxINFO, nSubSequence, EVENT_ACKED);
      if (e == NULL)
        e = DoneExtendedEvent(ICQ_CMDxSND_SEARCHxUIN, nSubSequence, EVENT_ACKED);
      // We make as copy as each plugin will delete the events it gets
      if (e == NULL)
      {
        gLog.Warn("%sReceived search result when no search in progress.\n", L_WARNxSTR);
        delete s;
        break;
      }
      ICQEvent *e2 = new ICQEvent(e);
      e2->m_pSearchAck = s;
      e2->m_pSearchAck->m_nMore = 0;
      PushPluginEvent(e2);
      PushExtendedEvent(e);
      break;
    }

    case ICQ_CMDxRCV_SEARCHxDONE:  // user found in search
    {
      /* 02 00 A0 00 04 00 05 00 00*/
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);

      char more;
  #if ICQ_VERSION == 2
      packet >> nSubSequence;
  #endif
      packet >> more;
      gLog.Info("%sSearch finished.\n", L_UDPxSTR);
      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_SEARCHxINFO, nSubSequence, EVENT_SUCCESS);
      if (e == NULL)
        e = DoneExtendedEvent(ICQ_CMDxSND_SEARCHxUIN, nSubSequence, EVENT_SUCCESS);
      if (e == NULL)
      {
        gLog.Warn("%sReceived end of search when no search in progress.\n", L_WARNxSTR);
        break;
      }
      e->m_pSearchAck = new CSearchAck(0);
      e->m_pSearchAck->m_nMore = (more != 0 ? -1 : 0);
      ProcessDoneEvent(e);
      break;
    }

    case ICQ_CMDxRCV_RANDOMxUSERxFOUND:
    {
      unsigned long nUin, nIp, nRealIp, nStatus;
      unsigned short nPort, nJunk, nTcpVersion;
      char nMode;

      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      packet >> nUin;

      if (nUin == 0)
      {
        gLog.Info("%sNo random chat user found.\n", L_UDPxSTR);
        ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_RANDOMxSEARCH, nSubSequence, EVENT_FAILED);
        if (e == NULL)
        {
          gLog.Warn("%sReceived random chat user when no search in progress.\n", L_WARNxSTR);
          break;
        }
        ProcessDoneEvent(e);
        break;
      }

      gLog.Info("%sRandom chat user found (%ld).\n", L_UDPxSTR, nUin);
      packet >> nIp >> nPort >> nJunk >> nRealIp >> nMode
             >> nStatus >> nTcpVersion;
      nIp = PacketIpToNetworkIp(nIp);
      nRealIp = PacketIpToNetworkIp(nRealIp);

      ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
      bool bNewUser = false;
      if (u == NULL)
      {
        u = new ICQUser(nUin);
        bNewUser = true;
      }
      u->SetIpPort(nIp, nPort);
      u->SetRealIp(nRealIp);
      u->SetMode(nMode);
      u->SetVersion(nTcpVersion);
      if (nMode != MODE_DIRECT)
        u->SetSendServer(true);
      u->SetStatus(nStatus);
      u->SetAutoResponse(NULL);
      if (bNewUser)
      {
        AddUserToList(u);
        icqRequestMetaInfo(nUin);
      }
      gUserManager.DropUser(u);

      // Return the event to the plugin
      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_RANDOMxSEARCH, nSubSequence, EVENT_SUCCESS);
      if (e == NULL)
      {
        gLog.Warn("%sReceived random chat user when no search in progress.\n", L_WARNxSTR);
        break;
      }
      e->m_pSearchAck = new CSearchAck(nUin);
      ProcessDoneEvent(e);

      break;
    }

    case ICQ_CMDxRCV_SYSxMSGxDONE:  // end of system messages
    {
       /* 02 00 E6 00 04 00 50 A5 82 00 */
       if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
       gLog.Info("%sEnd of system messages.\n", L_UDPxSTR);
       m_bOnlineNotifies = true;

  #if ICQ_VERSION == 2
       CPU_SysMsgDoneAck *p = new CPU_SysMsgDoneAck(nSequence);
  #elif ICQ_VERSION == 4
       CPU_SysMsgDoneAck *p = new CPU_SysMsgDoneAck(nSequence, nSubSequence);
  #elif ICQ_VERSION == 5
       CPU_SysMsgDoneAck *p = new CPU_SysMsgDoneAck;
  #endif
       gLog.Info("%sAcknowledging system messages (#%hu)...\n", L_UDPxSTR, p->Sequence());
       SendExpectEvent_Server(p);
       break;
    }

    case ICQ_CMDxRCV_SYSxMSGxOFFLINE:  // offline system message
    {
      /* 02 00 DC 00 0A 00 EC C9 45 00 CE 07 04 17 04 21 01 00 3F 00 6E 6F 2C 20
         73 74 69 6C 6C 20 68 6F 70 69 6E 67 20 66 6F 72 20 74 68 65 20 72 65 63
         6F 72 64 20 63 6F 6D 70 61 6E 79 2C 20 62 75 74 20 79 6F 75 20 6E 65 76
         65 72 20 6B 6E 6F 77 2E 2E 2E 00 */
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      gLog.Info("%sOffline system message.\n", L_UDPxSTR);

      unsigned short yearSent, newCommand;
      char monthSent, daySent, hourSent, minSent;
      packet >> nUin
             >> yearSent
             >> monthSent
             >> daySent
             >> hourSent
             >> minSent
             >> newCommand;

      // prepare a structure containing the relevant time information
      struct tm sentTime;
      sentTime.tm_sec  = 0;
      sentTime.tm_min  = minSent;
      sentTime.tm_hour = hourSent;
      sentTime.tm_mday = daySent;
      sentTime.tm_mon  = monthSent - 1;
      sentTime.tm_year = yearSent - 1900;
      sentTime.tm_isdst= -1;

      time_t sentLocalTime = mktime(&sentTime);

      // Timezone fix
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
      int nTimezone = o->GetTimezone();
      gUserManager.DropOwner(o);

      if (nTimezone != TIMEZONE_UNKNOWN)
      {
        int nHourDiff = nTimezone / -2;
        bool bPlusHalfHour = (nTimezone % 2);

        sentLocalTime += nHourDiff * 3600;

        if (bPlusHalfHour && (nTimezone / 2) > 0)
          sentLocalTime -= 1800;
        else if (bPlusHalfHour && (nTimezone / 2) < 0)
          sentLocalTime += 1800;
      }

      // process the system message, sending the time it occured converted to a time_t structure
      ProcessSystemMessage(packet, nUin, newCommand, sentLocalTime);
      break;
    }

    case ICQ_CMDxRCV_SYSxMSGxONLINE:  // online system message
    {
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      gLog.Info("%sOnline system message.\n", L_UDPxSTR);

      unsigned short newCommand;
      packet >> nUin
             >> newCommand;
      // process the system message, sending the current time as a time_t structure
      ProcessSystemMessage(packet, nUin, newCommand, time(NULL));
      break;
    }

    case ICQ_CMDxRCV_META: // meta command
    {
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      unsigned short nMetaCommand;
      char nMetaResult;
      packet >> nMetaCommand
             >> nMetaResult;
      gLog.Info("%sMeta command (%d) %s (#%d).\n", L_UDPxSTR,
                nMetaCommand,
                nMetaResult == META_SUCCESS ? "success" : "failed", nSubSequence);
      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_META, nSubSequence,
                                      nMetaResult == META_SUCCESS ? EVENT_SUCCESS : EVENT_FAILED);
      if (e == NULL)
      {
        gLog.Warn("%sReceived meta result for unknown meta command.\n", L_WARNxSTR);
        break;
      }

      // META_FAIL on a ICQ_CMDxMETA_SEARCHxWPxLAST_USER means on users found
      if (nMetaResult != META_SUCCESS &&
          nMetaCommand != ICQ_CMDxMETA_SEARCHxWPxLAST_USER)
        ProcessDoneEvent(e);
      else
      {
        ProcessMetaCommand(packet, nMetaCommand, e);
        if (e->m_nSubResult == META_DONE)
          ProcessDoneEvent(e);
        else
          PushExtendedEvent(e);
      }
      break;
    }

    case ICQ_CMDxRCV_REVERSExTCP:
    {
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      unsigned long nUin, nIp;
      unsigned short nPort, nJunk, nPort2, nVersion;
      char cJunk;
      packet >> nUin >> nIp
             >> nPort >> nJunk
             >> cJunk
             >> nPort2 >> nJunk // port which they tried to connect to
             >> nJunk >> nJunk // nPort again
             >> nVersion;
      nIp = PacketIpToNetworkIp(nIp);
      gLog.Info("%sReverse tcp request from %ld (port %d).\n", L_UDPxSTR, nUin, nPort2);
#ifdef BLOCKING_REVERSE_CONNECT
      ReverseConnectToUser(nUin, nIp, nPort, VersionToUse(nVersion), nPort2);
#else
      pthread_t thread_reverseconnect;
      CReverseConnectToUserData *d =
       new CReverseConnectToUserData(nUin, nIp, nPort, VersionToUse(nVersion), nPort2);
      pthread_create(&thread_reverseconnect, NULL, &ReverseConnectToUser_tep, d);
#endif
      break;
    }

    case ICQ_CMDxRCV_SETxOFFLINE:  // we got put offline by mirabilis for some reason
      gLog.Info("%sKicked offline by server.\n", L_UDPxSTR);
      //icqRelogon();
      return false;
      break;

    case ICQ_CMDxRCV_ACK:  // icq acknowledgement
    {
      /* 02 00 0A 00 12 00 */
      gLog.Info("%sAck (#%d).\n", L_UDPxSTR, nSequence);
  #if ICQ_VERSION == 5
      pthread_mutex_lock(&mutex_serverack);
      if (nSequence >= m_nServerAck)
      {
        m_nServerAck = nSequence;
        pthread_cond_broadcast(&cond_serverack);
      }
      pthread_mutex_unlock(&mutex_serverack);
  #endif
      ICQEvent *e = DoneEvent(m_nUDPSocketDesc, nSequence, EVENT_ACKED);
      if (e != NULL) ProcessDoneEvent(e);
      break;
    }

    case ICQ_CMDxRCV_SERVERxDOWN: // server having problems
    {
      unsigned long d = packet.UnpackUnsignedLong();
      gLog.Info("%sServer problem: %ld\n", L_UDPxSTR, d);
      break;
    }

    case ICQ_CMDxRCV_ERROR:  // icq says go away
    {
      gLog.Info("%sServer says you are not logged on.\n", L_UDPxSTR);
      //icqRelogon();
      return false;
      break;
    }

    case ICQ_CMDxRCV_HELLO: // hello packet from mirabilis received on logon
    {
      /* 02 00 5A 00 00 00 8F 76 20 00 CD CD 76 10 02 00 01 00 05 00 00 00 00 00
         8C 00 00 00 F0 00 0A 00 0A 00 05 00 0A 00 01 */
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);

      packet.UnpackUnsignedLong();
      packet.UnpackUnsignedLong();
      packet.UnpackUnsignedLong();
      unsigned long nLocalIp = packet.UnpackUnsignedLong();
      CPacket::SetLocalIp(nLocalIp);

      char buf[32];
      gLog.Info("%sServer says hello, we are at %s.\n", L_UDPxSTR,
        ip_ntoa(nLocalIp, buf));

      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      ChangeUserStatus(o, m_nDesiredStatus);
      gUserManager.DropOwner(o);

      m_eStatus = STATUS_ONLINE;
      m_bLoggingOn = false;
      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_LOGON, 1, EVENT_SUCCESS);
      if (e != NULL) ProcessDoneEvent(e);
      PushPluginSignal(new CICQSignal(SIGNAL_LOGON, 0, 0));

      icqUpdateContactList();
      icqSendInvisibleList();
      icqSendVisibleList();
  #if ICQ_VERSION != 5
      icqRequestSystemMsg();
  #endif
      // Send an update status packet to force hideip/webpresence
      if (m_nDesiredStatus & ICQ_STATUS_FxFLAGS)
      {
        icqSetStatus(m_nDesiredStatus);
      }

      break;
    }

    case ICQ_CMDxRCV_WRONGxPASSWD:  // incorrect password sent in logon
    {
      /* 02 00 64 00 00 00 02 00 8F 76 20 00 */
      gLog.Error("%sIncorrect password.\n", L_ERRORxSTR);
      m_eStatus = STATUS_OFFLINE_FORCED;
      m_bLoggingOn = false;
      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_LOGON, 1, EVENT_FAILED);
      if (e != NULL) ProcessDoneEvent(e);
      break;
    }

    case ICQ_CMDxRCV_BUSY:  // server too busy to respond
    {
      gLog.Info("%sServer busy, try again in a few minutes.\n", L_UDPxSTR);
      m_eStatus = STATUS_OFFLINE_FORCED;
      m_bLoggingOn = false;
      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_LOGON, 1, EVENT_FAILED);
      if (e != NULL) ProcessDoneEvent(e);
      break;
    }

    case ICQ_CMDxRCV_NEWxUIN:  // received a new uin
    {
  #if ICQ_VERSION == 2
      unsigned short nTemp;
      packet >> nTemp >> nOwnerUin;
  #else
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
  #endif
      if (!gUserManager.OwnerId(LICQ_PPID).empty())
      {
        gLog.Warn("%sReceived new uin (%ld) when already have a uin (%s).\n",
                  L_WARNxSTR, nOwnerUin, gUserManager.OwnerId(LICQ_PPID).c_str());
        break;
      }
      gLog.Info("%sReceived new uin: %ld\n", L_UDPxSTR, nOwnerUin);
      char ownerId[24];
      sprintf(ownerId, "%lu", nOwnerUin);
      gUserManager.AddOwner(ownerId, LICQ_PPID);
      ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_REGISTERxUSER, 1, EVENT_SUCCESS);
      if (e != NULL) ProcessDoneEvent(e);
      // Logon as an ack
      icqLogon(ICQ_STATUS_ONLINE);
      break;
    }

    default:  // what the heck is this packet?  print it out
    {
      if (!bMultiPacket) AckUDP(nSequence, nSubSequence, udp);
      char *buf;
      gLog.Unknown("%sUnknown server command %d:\n%s\n", L_UNKNOWNxSTR,
         nCommand, packet.print(buf));
      delete [] buf;
      break;
    }
  }

  return(nCommand);
}


//-----CICQDaemon::ProcessSystemMessage-----------------------------------------
void CICQDaemon::ProcessSystemMessage(CBuffer &packet, unsigned long nUin,
                                      unsigned short newCommand, time_t timeSent)
{
  ICQUser *u;
  int i, j;
  char c;

  if (nUin == 0)
  {
    char *buf;
    gLog.Unknown("%sInvalid system message (UIN = 0):\n%s\n", L_UNKNOWNxSTR, packet.print(buf));
    delete [] buf;
  }

#if 0
  // !!! This causes problems for UINs that are > 0x07FFFFFF !!!
  // Check if uin is backwards, what the fuck is with icq99b?
  if (nUin > 0x07FFFFFF)
  {
    nUin = ((nUin & 0x000000FF) << 24) + ((nUin & 0x0000FF00) << 8) +
           ((nUin & 0x00FF0000) >> 8)  + ((nUin & 0xFF000000) >> 24);
  }
#endif

  // Swap high and low bytes for strange new icq99
  if ((newCommand > 0x00FF) && !(newCommand & ICQ_CMDxSUB_FxMULTIREC))
  {
    newCommand = ((newCommand & 0xFF00) >> 8) + ((newCommand & 0x00FF) << 8);
  }

  unsigned long nMask = ((newCommand & ICQ_CMDxSUB_FxMULTIREC) ? E_MULTIxREC : 0);
  newCommand &= ~ICQ_CMDxSUB_FxMULTIREC;

  // Read out the data
  unsigned short nLen;
  packet >> nLen;

  // read in the user data from the packet
  char szMessage[nLen + 1];
  for (i = 0, j = 0; i < nLen; i++)
  {
    packet >> c;
    if (c != 0x0D) szMessage[j++] = c;
  }

  switch(newCommand)
  {
    case ICQ_CMDxSUB_MSG:  // system message: message through the server
    {
      CEventMsg *e = CEventMsg::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, nMask);

      // Lock the user to add the message to their queue
      u = gUserManager.FetchUser(nUin, LOCK_W);
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

      u = gUserManager.FetchUser(nUin, LOCK_W);
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
      /* 02 00 04 01 08 00 8F 76 20 00 06 00 41 00 41 70 6F 74 68 65 6F 73 69 73
         FE 47 72 61 68 61 6D FE 52 6F 66 66 FE 67 72 6F 66 66 40 75 77 61 74 65
         72 6C 6F 6F 2E 63 61 FE 31 FE 50 6C 65 61 73 65 20 61 75 74 68 6F 72 69
         7A 65 20 6D 65 2E 00 */

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
       ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
       AddUserEvent(o, e);
       gUserManager.DropOwner(o);
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
       ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
       AddUserEvent(o, e);
       gUserManager.DropOwner(o);
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

       ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
       AddUserEvent(o, e);
       gUserManager.DropOwner(o);
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
      gLog.Info("%s%s (%s %s), %s\n", L_UDPxSTR, szFields[0], szFields[1],
               szFields[2], szFields[3]);

      CEventAdded *e = new CEventAdded(nUin, szFields[0], szFields[1],
                                       szFields[2], szFields[3],
                                       ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      AddUserEvent(o, e);
      gUserManager.DropOwner(o);
      e->AddToHistory(NULL, D_RECEIVER);
      m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
      delete[] szFields;

      // See if we should send a welcome greeting
      //...TODO

      break;
    }
    case ICQ_CMDxSUB_WEBxPANEL:
    {
      /* 02 00 04 01 28 00 0A 00 00 00 0D 00 49 00 6D 79 20 6E 61 6D 65 FE FE FE
         65 6D 61 69 6C FE 33 FE 53 65 6E 64 65 72 20 49 50 3A 20 32 30 39 2E 32
         33 39 2E 36 2E 31 33 0D 0A 53 75 62 6A 65 63 74 3A 20 73 75 62 6A 65 63
         74 0D 0A 74 68 65 20 6D 65 73 73 61 67 65 00 */
      gLog.Info("%sMessage through web panel.\n", L_SBLANKxSTR);

      char **szFields = new char*[6]; // name, ?, ?, email, ?, message
      if (!ParseFE(szMessage, &szFields, 6))
      {
        char *buf;
        gLog.Warn("%sInvalid web panel system message:\n%s\n", L_WARNxSTR,
                  packet.print(buf));
        delete []buf;
        delete []szFields;
        break;
      }

      gTranslator.ServerToClient(szFields[0]);
      gTranslator.ServerToClient(szFields[5]);

      gLog.Info("%sFrom %s (%s).\n", L_SBLANKxSTR, szFields[0], szFields[3]);
      CEventWebPanel *e = new CEventWebPanel(szFields[0], szFields[3], szFields[5],
                                             ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      if (AddUserEvent(o, e))
      {
        gUserManager.DropOwner(o);
        e->AddToHistory(NULL, D_RECEIVER);
        m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
      }
      else
        gUserManager.DropOwner(o);
      delete[] szFields;
      break;
    }
    case ICQ_CMDxSUB_EMAILxPAGER:
    {
      /* 02 00 04 01 07 00 0A 00 00 00 0E 00 5C 00 73 61 6D 40 65 75 2E 6F 72 67
         FE FE FE 73 61 6D 40 65 75 2E 6F 72 67 FE 33 FE 53 75 62 6A 65 63 74 3A
         20 5B 53 4C 41 53 48 44 4F 54 5D 0D 0A 5B 54 68 65 20 49 6E 74 65 72 6E
         65 74 5D 20 45 6E 6F 72 6D 6F 75 73 20 38 30 73 20 54 65 78 74 66 69 6C
         65 20 41 72 63 68 69 76 65 00 */
      gLog.Info("%sEmail pager message.\n", L_SBLANKxSTR);

      char **szFields = new char*[6]; // name, ?, ?, email, ?, message
      if (!ParseFE(szMessage, &szFields, 6))
      {
        char *buf;
        gLog.Warn("%sInvalid email pager system message:\n%s\n", L_WARNxSTR,
                  packet.print(buf));
        delete []buf;
        delete []szFields;
        break;
      }

      gTranslator.ServerToClient(szFields[0]);
      gTranslator.ServerToClient(szFields[5]);

      gLog.Info("%sFrom %s (%s).\n", L_SBLANKxSTR, szFields[0], szFields[3]);
      CEventEmailPager *e = new CEventEmailPager(szFields[0], szFields[3], szFields[5],
                                                 ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      if (AddUserEvent(o, e))
      {
        gUserManager.DropOwner(o);
        e->AddToHistory(NULL, D_RECEIVER);
        m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
      }
      else
        gUserManager.DropOwner(o);
      delete[] szFields;
      break;
    }

    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      /* 02 00 DC 00 1A 00 23 64 84 00 CF 07 06 15 13 2B 13 00 1E 00 32 FE 37 33
         39 37 38 35 33 FE 46 6C 75 6E 6B 69 FE 37 33 35 37 32 31 39 FE 55 68 75
         FE 00 */
      CEventContactList *e = CEventContactList::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, nMask);
      if (e == NULL)
      {
        char *buf;
        gLog.Warn("%sInvalid Contact List message:\n%s\n", L_WARNxSTR, packet.print(buf));
        delete []buf;
        break;
      }

      u = gUserManager.FetchUser(nUin, LOCK_W);
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
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      AddUserEvent(o, e);
      gUserManager.DropOwner(o);
    }
  } // switch
}



//-----CICQDaemon::ProcessMetaCommand-----------------------------------------
void CICQDaemon::ProcessMetaCommand(CBuffer &packet,
                                    unsigned short nMetaCommand,
                                    ICQEvent *e)
{
  ICQUser *u = NULL;
  char szTemp[MAX_DATA_LEN];
  unsigned long nUin;

  switch(nMetaCommand)
  {
    case ICQ_CMDxMETA_MORExINFO:
    case ICQ_CMDxMETA_WORKxINFO:
    case ICQ_CMDxMETA_GENERALxINFO:
    case ICQ_CMDxMETA_ABOUT:
    case ICQ_CMDxMETA_UNKNOWNx240:
    case ICQ_CMDxMETA_UNKNOWNx250:
    case ICQ_CMDxMETA_UNKNOWNx270:
    case ICQ_CMDxMETA_BASICxINFO:
    {
      e->m_nSubResult += nMetaCommand;
      nUin = ((CPU_Meta_RequestAllInfo *)e->m_pPacket)->Uin();
      u = FindUserForInfoUpdate(nUin, e, "meta");
      if (u == NULL) break;

      switch (nMetaCommand)
      {
        case ICQ_CMDxMETA_BASICxINFO:
        {
          gLog.Info("%sBasic info on %s (%ld).\n", L_SBLANKxSTR, u->GetAlias(), u->Uin());
          u->SetEnableSave(false);
          u->SetAlias(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetFirstName(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetLastName(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetEmailPrimary(packet.UnpackString(szTemp, sizeof(szTemp)));
          // FIXME how does this packet end?
          //u->SetAuthorization(!packet.UnpackChar());
          //packet.UnpackChar(); // 02, what the?
          //u->SetHideEmail(packet.UnpackChar());

          // translating string with Translation Table
          gTranslator.ServerToClient(u->GetAlias());
          gTranslator.ServerToClient(u->GetFirstName());
          gTranslator.ServerToClient(u->GetLastName());
      gTranslator.ServerToClient(u->GetEmailPrimary());

          u->SetEnableSave(true);
          u->SaveGeneralInfo();
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_BASIC, nUin));
          e->m_nSubResult = META_DONE;
          break;
        }

        case ICQ_CMDxMETA_GENERALxINFO:
        {
          gLog.Info("%sGeneral info on %s (%ld).\n", L_SBLANKxSTR, u->GetAlias(), u->Uin());
          u->SetEnableSave(false);
          u->SetAlias(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetFirstName(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetLastName(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetEmailPrimary(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetEmailSecondary(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetEmailOld(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetCity(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetState(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetPhoneNumber(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetFaxNumber(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetAddress(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetCellularNumber(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetZipCode(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetCountryCode(packet.UnpackUnsignedShort());
          u->SetTimezone(packet.UnpackChar());
          // FIXME how does this packet end?
          //u->SetAuthorization(!packet.UnpackChar());
          //packet.UnpackChar(); // 02, what the?
          //u->SetHideEmail(packet.UnpackChar());

          // translating string with Translation Table
          gTranslator.ServerToClient(u->GetAlias());
          gTranslator.ServerToClient(u->GetFirstName());
          gTranslator.ServerToClient(u->GetLastName());
      gTranslator.ServerToClient(u->GetEmailPrimary());
      gTranslator.ServerToClient(u->GetEmailSecondary());
      gTranslator.ServerToClient(u->GetEmailOld());
          gTranslator.ServerToClient(u->GetCity());
          gTranslator.ServerToClient(u->GetState());
          gTranslator.ServerToClient(u->GetPhoneNumber());
          gTranslator.ServerToClient(u->GetFaxNumber());
          gTranslator.ServerToClient(u->GetCellularNumber());
          gTranslator.ServerToClient(u->GetAddress());
          gTranslator.ServerToClient(u->GetZipCode());

          u->SetEnableSave(true);
          u->SaveGeneralInfo();
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                          USER_GENERAL, nUin));
          break;
        }
        case ICQ_CMDxMETA_WORKxINFO:
        {
          gLog.Info("%sWork info on %s (%ld).\n", L_SBLANKxSTR, u->GetAlias(), u->Uin());
          u->SetEnableSave(false);
          u->SetCompanyCity(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetCompanyState(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetCompanyPhoneNumber(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetCompanyFaxNumber(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetCompanyAddress(packet.UnpackString(szTemp, sizeof(szTemp)));
          // who knows..."256" typically
          packet.UnpackString(szTemp, sizeof(szTemp));
          packet.UnpackUnsignedShort(); // ??? 0x0000
          u->SetCompanyName(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetCompanyDepartment(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetCompanyPosition(packet.UnpackString(szTemp, sizeof(szTemp)));
          packet.UnpackUnsignedShort(); // ?? 0x0004
          u->SetCompanyHomepage(packet.UnpackString(szTemp, sizeof(szTemp)));

          // translating string with Translation Table
          gTranslator.ServerToClient(u->GetCompanyCity());
          gTranslator.ServerToClient(u->GetCompanyState());
          gTranslator.ServerToClient(u->GetCompanyAddress());
          gTranslator.ServerToClient(u->GetCompanyPhoneNumber());
          gTranslator.ServerToClient(u->GetCompanyFaxNumber());
          gTranslator.ServerToClient(u->GetCompanyName());
          gTranslator.ServerToClient(u->GetCompanyDepartment());
          gTranslator.ServerToClient(u->GetCompanyPosition());
          gTranslator.ServerToClient(u->GetCompanyHomepage());

          u->SetEnableSave(true);
          u->SaveWorkInfo();
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                          USER_WORK, nUin));
          break;
        }
        case ICQ_CMDxMETA_MORExINFO:
        {
          gLog.Info("%sMore info on %s (%ld).\n", L_SBLANKxSTR, u->GetAlias(), u->Uin());
          u->SetEnableSave(false);
          u->SetAge(packet.UnpackUnsignedShort());
          u->SetGender(packet.UnpackChar());
          u->SetHomepage(packet.UnpackString(szTemp, sizeof(szTemp)));
          u->SetBirthYear(packet.UnpackUnsignedShort());
          u->SetBirthMonth(packet.UnpackChar());
          u->SetBirthDay(packet.UnpackChar());
          u->SetLanguage1(packet.UnpackChar());
          u->SetLanguage2(packet.UnpackChar());
          u->SetLanguage3(packet.UnpackChar());

          // translating string with Translation Table
          gTranslator.ServerToClient(u->GetHomepage());

          u->SetEnableSave(true);
          u->SaveMoreInfo();
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                          USER_MORE, nUin));
          break;
        }
        case ICQ_CMDxMETA_ABOUT:
        {
          gLog.Info("%sAbout info on %s (%ld).\n", L_SBLANKxSTR, u->GetAlias(), u->Uin());
          u->SetAbout(packet.UnpackString(szTemp, sizeof(szTemp)));
          gTranslator.ServerToClient(u->GetAbout());
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                          USER_ABOUT, nUin));
          break;
        }
      }
      gUserManager.DropUser(u);
      break;
    }

    case ICQ_CMDxMETA_GENERALxINFOxRSP:
    {
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      CPU_Meta_SetGeneralInfo *p = (CPU_Meta_SetGeneralInfo *)e->m_pPacket;
      o->SetEnableSave(false);
      o->SetAlias(p->m_szAlias);
      o->SetFirstName(p->m_szFirstName);
      o->SetLastName(p->m_szLastName);
      o->SetEmailPrimary(p->m_szEmailPrimary);
      o->SetEmailOld(p->m_szEmailOld);
      o->SetEmailSecondary(p->m_szEmailSecondary);
      o->SetCity(p->m_szCity);
      o->SetState(p->m_szState);
      o->SetPhoneNumber(p->m_szPhoneNumber);
      o->SetFaxNumber(p->m_szFaxNumber);
      o->SetAddress(p->m_szAddress);
      o->SetCellularNumber(p->m_szCellularNumber);
      o->SetZipCode(p->m_szZipCode);
      o->SetCountryCode(p->m_nCountryCode);
      o->SetTimezone(p->m_nTimezone);
      o->SetHideEmail(p->m_nHideEmail == 1);

      gTranslator.ServerToClient(o->GetAlias());
      gTranslator.ServerToClient(o->GetFirstName());
      gTranslator.ServerToClient(o->GetLastName());
      gTranslator.ServerToClient(o->GetEmailPrimary());
      gTranslator.ServerToClient(o->GetEmailOld());
      gTranslator.ServerToClient(o->GetEmailSecondary());
      gTranslator.ServerToClient(o->GetCity());
      gTranslator.ServerToClient(o->GetState());
      gTranslator.ServerToClient(o->GetPhoneNumber());
      gTranslator.ServerToClient(o->GetFaxNumber());
      gTranslator.ServerToClient(o->GetAddress());
      gTranslator.ServerToClient(o->GetCellularNumber());

      o->SetEnableSave(true);
      o->SaveGeneralInfo();

      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                      USER_GENERAL, o->Uin()));
      gUserManager.DropOwner(o);
      e->m_nSubResult = META_DONE;
      break;
    }
    case ICQ_CMDxMETA_MORExINFOxRSP:
    {
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      CPU_Meta_SetMoreInfo *p = (CPU_Meta_SetMoreInfo *)e->m_pPacket;
      o->SetEnableSave(false);
      o->SetAge(p->m_nAge);
      o->SetGender(p->m_nGender);
      o->SetHomepage(p->m_szHomepage);
      o->SetBirthYear(p->m_nBirthYear);
      o->SetBirthMonth(p->m_nBirthMonth);
      o->SetBirthDay(p->m_nBirthDay);
      o->SetLanguage1(p->m_nLanguage1);
      o->SetLanguage2(p->m_nLanguage2);
      o->SetLanguage3(p->m_nLanguage3);

      gTranslator.ServerToClient(o->GetHomepage());

      o->SetEnableSave(true);
      o->SaveMoreInfo();

      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                      USER_MORE, o->Uin()));
      gUserManager.DropOwner(o);
      e->m_nSubResult = META_DONE;
      break;
    }
    case ICQ_CMDxMETA_WORKxINFOxRSP:
    {
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      CPU_Meta_SetWorkInfo *p = (CPU_Meta_SetWorkInfo *)e->m_pPacket;
      o->SetEnableSave(false);
      o->SetCompanyCity(p->m_szCity);
      o->SetCompanyState(p->m_szState);
      o->SetCompanyPhoneNumber(p->m_szPhoneNumber);
      o->SetCompanyFaxNumber(p->m_szFaxNumber);
      o->SetCompanyAddress(p->m_szAddress);
      o->SetCompanyName(p->m_szName);
      o->SetCompanyDepartment(p->m_szDepartment);
      o->SetCompanyPosition(p->m_szPosition);
      o->SetCompanyHomepage(p->m_szHomepage);

      gTranslator.ServerToClient(o->GetCompanyCity());
      gTranslator.ServerToClient(o->GetCompanyState());
      gTranslator.ServerToClient(o->GetCompanyPhoneNumber());
      gTranslator.ServerToClient(o->GetCompanyFaxNumber());
      gTranslator.ServerToClient(o->GetCompanyAddress());
      gTranslator.ServerToClient(o->GetCompanyName());
      gTranslator.ServerToClient(o->GetCompanyDepartment());
      gTranslator.ServerToClient(o->GetCompanyPosition());
      gTranslator.ServerToClient(o->GetCompanyHomepage());


      o->SetEnableSave(true);
      o->SaveWorkInfo();

      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                      USER_WORK, o->Uin()));
      gUserManager.DropOwner(o);
      e->m_nSubResult = META_DONE;
      break;
    }
    case ICQ_CMDxMETA_ABOUTxRSP:
    {
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      CPU_Meta_SetAbout *p = (CPU_Meta_SetAbout *)e->m_pPacket;
      o->SetAbout(p->m_szAbout);

//      gTranslator.ServerToClient(o->GetAbout());

      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                      USER_ABOUT, o->Uin()));
      gUserManager.DropOwner(o);
      e->m_nSubResult = META_DONE;
      break;
    }
    case ICQ_CMDxMETA_SECURITYxRSP:
    {
      CPU_Meta_SetSecurityInfo *p = (CPU_Meta_SetSecurityInfo *)e->m_pPacket;
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      o->SetEnableSave(false);
      o->SetAuthorization(p->Authorization());
      o->SetWebAware(p->WebAware());
      o->SetHideIp(p->HideIp());
      o->SetEnableSave(true);
      o->SaveLicqInfo();
      unsigned short s = o->StatusFull();
      gUserManager.DropOwner(o);
      // Set status to ensure the status flags are set
      icqSetStatus(s);
      e->m_nSubResult = META_DONE;
      break;
    }
    case ICQ_CMDxMETA_PASSWORDxRSP:
    {
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      o->SetEnableSave(false);
      o->SetPassword( ((CPU_Meta_SetPassword *)e->m_pPacket)->m_szPassword);
      o->SetEnableSave(true);
      o->SaveLicqInfo();
      gUserManager.DropOwner(o);
      e->m_nSubResult = META_DONE;
      break;
    }
    case ICQ_CMDxMETA_SEARCHxWPxFOUND:
    {
      gLog.Info("%sWP search found user:\n", L_UDPxSTR);

      unsigned long nUin;
      char auth;
      char szTemp[64];

      packet.UnpackUnsignedShort(); // Remove the 2 byte data length
      packet >> nUin;
      CSearchAck *s = new CSearchAck(nUin);

      // Get the data out of the packet
      s->m_szAlias = strdup(packet.UnpackString(szTemp, sizeof(szTemp)));
      s->m_szFirstName = strdup(packet.UnpackString(szTemp, sizeof(szTemp)));
      s->m_szLastName = strdup(packet.UnpackString(szTemp, sizeof(szTemp)));
      s->m_szEmail = strdup(packet.UnpackString(szTemp, sizeof(szTemp)));

      // Translating string with Translation Table
      gTranslator.ServerToClient(s->m_szAlias);
      gTranslator.ServerToClient(s->m_szFirstName);
      gTranslator.ServerToClient(s->m_szLastName);
      gTranslator.ServerToClient(s->m_szEmail);

      // Check to see if they want a request sent.. but we don't do that anywas.
      packet >> auth;

      // This will show their status
      // 0x00 - Offline
      // 0x01 - Online
      // 0x02 - Disabled
      s->m_nStatus = packet.UnpackChar();

      // Eat it.. always 0x00
      packet.UnpackChar();

      gLog.Info("%s%s (%ld) <%s %s %s>\n", L_SBLANKxSTR, s->m_szAlias, nUin,
                s->m_szFirstName, s->m_szLastName, s->m_szEmail);

      // Make a copy for the plugin
      if (e == NULL)
      {
        gLog.Warn("%sReceived search result when no search in progress.\n",
              L_WARNxSTR);
        break;
      }

      ICQEvent *e2 = new ICQEvent(e);
      e2->m_pSearchAck = s;
      e2->m_nSubCommand = ICQ_CMDxMETA_SEARCHxWPxFOUND;  // Sub-meta command
      e2->m_eResult = EVENT_ACKED;
      e2->m_pSearchAck->m_nMore = 0;
      PushPluginEvent(e2);

      break;
    }
    case ICQ_CMDxMETA_SEARCHxWPxLAST_USER:
    {
      if (packet.End())
      {
        gLog.Info("%sWP search found no users.\n", L_UDPxSTR);
        ICQEvent *e2 = new ICQEvent(e);
        e2->m_eResult = EVENT_SUCCESS;
        e2->m_pSearchAck = NULL;
        // It's a lie, but let's the plugin check for a
        // null search ack to see if no users were found
        e2->m_nSubCommand = ICQ_CMDxMETA_SEARCHxWPxLAST_USER;
        PushPluginEvent(e2);
        break;
      }

      gLog.Info("%sWP search found last user:\n", L_UDPxSTR);

      // We'll only get here if the result was 0x0A - Passed
      unsigned long nUin;
      char auth;
      char szTemp[64];

      packet.UnpackUnsignedShort(); // Remove the 2 byte data length
      packet >> nUin;
      CSearchAck *s = new CSearchAck(nUin);

      // Get the data out of the packet
      s->m_szAlias = strdup(packet.UnpackString(szTemp, sizeof(szTemp)));
      s->m_szFirstName = strdup(packet.UnpackString(szTemp, sizeof(szTemp)));
      s->m_szLastName = strdup(packet.UnpackString(szTemp, sizeof(szTemp)));
      s->m_szEmail = strdup(packet.UnpackString(szTemp, sizeof(szTemp)));

      // Translating string with Translation Table
      gTranslator.ServerToClient(s->m_szAlias);
      gTranslator.ServerToClient(s->m_szFirstName);
      gTranslator.ServerToClient(s->m_szLastName);
      gTranslator.ServerToClient(s->m_szEmail);

      // Check to see if they want a request sent.. but we don't do that anywas.
      packet >> auth;

      // This will show their status
      // 0x00 - Offline
      // 0x01 - Online
      // 0x02 - Disabled
      s->m_nStatus = packet.UnpackChar();

      // Eat it
      packet.UnpackChar();

      gLog.Info("%s%s (%ld) <%s %s %s>\n", L_SBLANKxSTR, s->m_szAlias, nUin,
        s->m_szFirstName, s->m_szLastName, s->m_szEmail);

      // Make a copy for the plugin
      if (e == NULL)
      {
        gLog.Warn("%sReceived search result when no search in progress.\n",
          L_WARNxSTR);
        break;
      }

      // More users..... It sure looks like this is the number of how many
      // more users were found.
      unsigned long nNumMore;
      packet >> nNumMore;

      ICQEvent *e2 = new ICQEvent(e);
      e2->m_pSearchAck = s;
      e2->m_nSubCommand = ICQ_CMDxMETA_SEARCHxWPxLAST_USER;  // Sub meta-command
      e2->m_eResult = EVENT_SUCCESS;
      e2->m_pSearchAck->m_nMore = nNumMore;
      PushPluginEvent(e2);

      break;
    }
    default:
    {
      char *buf;
      gLog.Unknown("%sUnknown meta command (0x%04x):\n%s\n", L_UNKNOWNxSTR,
                   nMetaCommand, packet.print(buf));
      delete [] buf;
      break;
    }
  }

}


//-----CICQDaemon::AckUDP--------------------------------------------------------------
void CICQDaemon::AckUDP(unsigned short _nSequence, unsigned short _nSubSequence, UDPSocket *udp)
// acknowledge whatever packet we received using the relevant sequence number
{
#if ICQ_VERSION == 2
  CPU_Ack p(_nSequence);
#elif ICQ_VERSION == 4 || ICQ_VERSION == 5
  CPU_Ack p(_nSequence, _nSubSequence);
#endif
  SendEvent(udp, p, true);
}


