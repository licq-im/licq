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
void CICQDaemon::icqAddUser(unsigned long _nUin, bool _bServerOnly,
                            bool _bAuthRequired)
{
  // Server side list add, and update of group
  if (UseServerContactList())
  {
    CSrvPacketTcp *pStart = 0;
   
    if (_bServerOnly) // Export 
      pStart = new CPU_ExportContactStart();
    else
      pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
        ICQ_SNACxLIST_ROSTxEDITxSTART);

    SendEvent_Server(pStart);

    char szUin[13];
    snprintf(szUin, 12, "%lu", _nUin);
    szUin[12] = 0;

    pthread_mutex_lock(&mutex_modifyserverusers);
    m_lszModifyServerUsers.push_back(strdup(szUin));
    pthread_mutex_unlock(&mutex_modifyserverusers);

    CPU_AddToServerList *pAdd = new CPU_AddToServerList(szUin, ICQ_ROSTxNORMAL, _bServerOnly,
                                                        0, _bAuthRequired);
    gLog.Info("%sAdding %ld to server list...\n", L_SRVxSTR, _nUin);
    SendExpectEvent_Server(0, pAdd, NULL);
  }

  if (!_bServerOnly)
  {
    CSrvPacketTcp *p = new CPU_GenericUinList(_nUin, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
    gLog.Info("%sAlerting server to new user (#%ld)...\n", L_SRVxSTR,
              p->Sequence());
    SendExpectEvent_Server(_nUin, p, NULL);

    // update the users info from the server now or after they are added
    if (!UseServerContactList())
      icqUserBasicInfo(_nUin);
  }
}

//-----icqExportUsers-----------------------------------------------------------
void CICQDaemon::icqExportUsers(UinList &uins)
{
  if (!UseServerContactList())  return;

  CSrvPacketTcp *pStart = new CPU_ExportContactStart();
  SendEvent_Server(pStart);

  CSrvPacketTcp *pExport = new CPU_ExportToServerList(uins);
  gLog.Info("%sExporting users to server contact list...\n", L_SRVxSTR);
  SendExpectEvent_Server(0, pExport, NULL);
}

//-----icqAddGroup--------------------------------------------------------------
void CICQDaemon::icqAddGroup(const char *_szName)
{
  if (!UseServerContactList())  return;

  CSrvPacketTcp *pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
    ICQ_SNACxLIST_ROSTxEDITxSTART);
  SendEvent_Server(pStart);

  pthread_mutex_lock(&mutex_modifyserverusers);
  m_lszModifyServerUsers.push_back(strdup(_szName));
  pthread_mutex_unlock(&mutex_modifyserverusers);

  CPU_AddToServerList *pAdd = new CPU_AddToServerList(_szName, ICQ_ROSTxGROUP);
  int nGSID = pAdd->GetGSID();
  gLog.Info("%sAdding group %s (%d) to server list ...\n", L_SRVxSTR, _szName, nGSID);
  SendExpectEvent_Server(0, pAdd, NULL);
}

//-----icqChangeGroup-----------------------------------------------------------
void CICQDaemon::icqChangeGroup(unsigned long _nUin, unsigned short _nNewGroup,
                                unsigned short _nOldGSID)
{
  if (!UseServerContactList())  return;

  // Get their old SID
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
  int nSID = u->GetSID();
  gUserManager.DropUser(u);

  CSrvPacketTcp *pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
                                                ICQ_SNACxLIST_ROSTxEDITxSTART);
  SendEvent_Server(pStart);

  char szUin[13];
  snprintf(szUin, 12, "%ld", _nUin);
  szUin[12] = '\0';

  pthread_mutex_lock(&mutex_modifyserverusers);
  m_lszModifyServerUsers.push_back(strdup(szUin)); // add
  m_lszModifyServerUsers.push_back(strdup(szUin)); // remove
  pthread_mutex_unlock(&mutex_modifyserverusers);

  CPU_AddToServerList *pAdd = new CPU_AddToServerList(szUin, ICQ_ROSTxNORMAL,
                                                      false, _nNewGroup);
  gLog.Info("%sChanging group on server list for %s ...\n", L_SRVxSTR, szUin);
  SendExpectEvent_Server(0, pAdd, NULL);

  CSrvPacketTcp *pRemove = new CPU_RemoveFromServerList(szUin, _nOldGSID,
                                                        nSID, ICQ_ROSTxNORMAL);
  SendExpectEvent_Server(0, pRemove, NULL);
}

//-----icqExportGroups----------------------------------------------------------
void CICQDaemon::icqExportGroups(GroupList &groups)
{
  if (!UseServerContactList()) return;

  CSrvPacketTcp *pStart = new CPU_ExportContactStart();
  SendEvent_Server(pStart);

  CSrvPacketTcp *pExport = new CPU_ExportGroupsToServerList(groups);
  gLog.Info("%sExporting groups to server contact list...\n", L_SRVxSTR);
  SendExpectEvent_Server(0, pExport, NULL);
}

//-----icqRemoveUser-------------------------------------------------------
void CICQDaemon::icqRemoveUser(unsigned long _nUin)
{
  // Remove from the SSList and update groups
  if (UseServerContactList())
  {
    CSrvPacketTcp *pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
      ICQ_SNACxLIST_ROSTxEDITxSTART);
    SendEvent_Server(pStart);

    ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_W);
    unsigned short nGSID = u->GetGSID();
    unsigned short nSID = u->GetSID();
    char szUin[13];
    snprintf(szUin, 12, "%ld", _nUin);
    szUin[12] = '\0';
    u->SetGSID(0);
    gUserManager.DropUser(u);

    pthread_mutex_lock(&mutex_modifyserverusers);
    m_lszModifyServerUsers.push_back(strdup(szUin));
    pthread_mutex_unlock(&mutex_modifyserverusers);

    CSrvPacketTcp *pRemove = new CPU_RemoveFromServerList(szUin, nGSID, nSID, ICQ_ROSTxNORMAL);
    SendExpectEvent_Server(0, pRemove, NULL);
  }

  // Tell server they are no longer with us.
  CSrvPacketTcp *p = new CPU_GenericUinList(_nUin, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_REMOVExFROMxLIST);
  gLog.Info("%sAlerting server to remove user (#%ld)...\n", L_SRVxSTR,
            p->Sequence());
  SendExpectEvent_Server(_nUin, p, NULL);
}

//-----icqRemoveGroup----------------------------------------------------------
void CICQDaemon::icqRemoveGroup(const char *_szName)
{
  if (!UseServerContactList()) return;

  CSrvPacketTcp *pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
    ICQ_SNACxLIST_ROSTxEDITxSTART);
  SendEvent_Server(pStart);

  pthread_mutex_lock(&mutex_modifyserverusers);
  m_lszModifyServerUsers.push_back(strdup(_szName));
  pthread_mutex_unlock(&mutex_modifyserverusers);

  CSrvPacketTcp *pRemove = new CPU_RemoveFromServerList(_szName,
    gUserManager.GetIDFromGroup(_szName), 0, ICQ_ROSTxGROUP);
  gLog.Info("%sRemoving group from server side list (%s)...\n", L_SRVxSTR, _szName);
  SendExpectEvent_Server(0, pRemove, NULL);
}

//-----icqRenameGroup----------------------------------------------------------
void CICQDaemon::icqRenameGroup(const char *_szNewName, unsigned short _nGSID)
{
  if (!UseServerContactList() || !_nGSID) return;

  pthread_mutex_lock(&mutex_modifyserverusers);
  m_lszModifyServerUsers.push_back(strdup(_szNewName));
  pthread_mutex_unlock(&mutex_modifyserverusers);

  CSrvPacketTcp *pUpdate = new CPU_UpdateToServerList(_szNewName,
    ICQ_ROSTxGROUP, _nGSID);
  gLog.Info("%sRenaming group with id %d to %s...\n", L_SRVxSTR, _nGSID,
    _szNewName);
  SendExpectEvent_Server(0, pUpdate, NULL);
}

//-----icqRenameUser------------------------------------------------------------
void CICQDaemon::icqRenameUser(unsigned long _nUin, const char *_szOldAlias)
{
  if (!UseServerContactList()) return;

  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
  if (u == NULL) return;
  char *szNewAlias = u->GetAlias();
  gUserManager.DropUser(u);

  char szUin[13];
  snprintf(szUin, 12, "%lu", _nUin);
  szUin[12] = '\0';

  pthread_mutex_lock(&mutex_modifyserverusers);
  m_lszModifyServerUsers.push_back(strdup(szUin));
  pthread_mutex_unlock(&mutex_modifyserverusers);

  CSrvPacketTcp *pUpdate = new CPU_UpdateToServerList(szUin, ICQ_ROSTxNORMAL);
  gLog.Info("%sRenaming %s (%lu) to %s...\n", L_SRVxSTR, _szOldAlias, _nUin,
            szNewAlias);
  SendExpectEvent_Server(0, pUpdate, NULL);
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

//-----icqFetchAutoResponseServer-----------------------------------------------
unsigned long CICQDaemon::icqFetchAutoResponseServer(unsigned long _nUin)
{
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
  if (!u) return 0;
  
  int nCmd;
  switch (u->Status())
  {
  case ICQ_STATUS_AWAY:
    nCmd = ICQ_CMDxTCP_READxAWAYxMSG; break;
  case ICQ_STATUS_NA:
    nCmd = ICQ_CMDxTCP_READxNAxMSG; break;
  case ICQ_STATUS_DND:
    nCmd = ICQ_CMDxTCP_READxDNDxMSG; break;
  case ICQ_STATUS_OCCUPIED:
    nCmd = ICQ_CMDxTCP_READxOCCUPIEDxMSG; break;
  case ICQ_STATUS_FREEFORCHAT:
    nCmd = ICQ_CMDxTCP_READxFFCxMSG; break;
  default:
    nCmd = ICQ_CMDxTCP_READxAWAYxMSG; break;
  }
  
  CPU_ThroughServer *p = new CPU_ThroughServer(_nUin, nCmd, 0);
  gLog.Info("%sRequesting auto response from %s (%ld).\n", L_SRVxSTR,
  	u->GetAlias(), p->Sequence());
  gUserManager.DropUser(u);

  ICQEvent *result = SendExpectEvent_Server(_nUin, p, NULL);

  return result->EventId();
}

//-----icqSetRandomChatGroup----------------------------------------------------
unsigned long CICQDaemon::icqSetRandomChatGroup(unsigned long _nGroup)
{
  gLog.Warn("%sThis feature is currently not implemented.\n", L_WARNxSTR);

  return 0;
}

//-----icqRandomChatSearch------------------------------------------------------
unsigned long CICQDaemon::icqRandomChatSearch(unsigned long _nGroup)
{
  gLog.Warn("%sThis feature is currently not implemented.\n", L_WARNxSTR);

  return 0;
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
  m_bRegistering = true;
//  CPU_RegisterFirst *p = new CPU_RegisterFirst();
//  gLog.Info("%sRegistering a new user (#%ld)...\n", L_SRVxSTR, p->Sequence());
// SendEvent_Server(p);
  ConnectToLoginServer();
}

//-----ICQ::icqRegisterFinish------------------------------------------------
void CICQDaemon::icqRegisterFinish()
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  char *szPasswd = o->Password();
  gUserManager.DropOwner();

  CPU_RegisterFirst *pFirst = new CPU_RegisterFirst();
  SendEvent_Server(pFirst);

  CPU_Register *p = new CPU_Register(szPasswd);
  gLog.Info("%sRegistering a new user...\n", L_SRVxSTR);
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
  if (newStatus & ICQ_STATUS_DND)
    newStatus |= 0x10; // quick compat hack

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

//-----icqSetGeneralInfo----------------------------------------------------
unsigned long CICQDaemon::icqSetGeneralInfo(
                          const char *szAlias, const char *szFirstName,
                          const char *szLastName, const char *szEmailPrimary,
                          const char *szCity,
                          const char *szState, const char *szPhoneNumber,
                          const char *szFaxNumber, const char *szAddress,
                          const char *szCellularNumber, const char *szZipCode,
                          unsigned short nCountryCode, bool bHideEmail)
{
  CPU_Meta_SetGeneralInfo *p =
    new CPU_Meta_SetGeneralInfo(szAlias, szFirstName,
                                szLastName, szEmailPrimary,
                                szCity,
                                szState, szPhoneNumber,
                                szFaxNumber, szAddress,
                                szCellularNumber, szZipCode,
                                nCountryCode, bHideEmail);

  gLog.Info("%sUpdating general info (#%ld/#%d)...\n", L_SRVxSTR, p->Sequence(), p->SubSequence());

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  PushExtendedEvent(e);
  return e->EventId();
}

//-----icqSetEmailInfo---------------------------------------------------------
unsigned long CICQDaemon::icqSetEmailInfo(
                          const char *szEmailSecondary, const char *szEmailOld)
{
  CPU_Meta_SetEmailInfo *p =
    new CPU_Meta_SetEmailInfo(szEmailSecondary, szEmailOld);

  gLog.Info("%sUpdating additional E-Mail info (#%ld/#%d)...\n", L_SRVxSTR, p->Sequence(), p->SubSequence());

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  PushExtendedEvent(e);
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

  gLog.Info("%sUpdating more info (#%ld/#%d)...\n", L_SRVxSTR, p->Sequence(), p->SubSequence());

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  PushExtendedEvent(e);
  return e->EventId();
}

//-----icqSetWorkInfo--------------------------------------------------------
unsigned long CICQDaemon::icqSetWorkInfo(const char *_szCity, const char *_szState,
                                     const char *_szPhone,
                                     const char *_szFax, const char *_szAddress,
				     const char *_szZip, unsigned short _nCompanyCountry,
                                     const char *_szName, const char *_szDepartment,
                                     const char *_szPosition, const char *_szHomepage)
{
  CPU_Meta_SetWorkInfo *p =
    new CPU_Meta_SetWorkInfo(_szCity, _szState, _szPhone, _szFax, _szAddress,
                             _szZip, _nCompanyCountry, _szName, _szDepartment, _szPosition, _szHomepage);

  gLog.Info("%sUpdating work info (#%ld/#%d)...\n", L_SRVxSTR, p->Sequence(), p->SubSequence());

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  PushExtendedEvent(e);
  return e->EventId();
}

//-----icqSetAbout-----------------------------------------------------------
unsigned long CICQDaemon::icqSetAbout(const char *_szAbout)
{
  char *szAbout = gTranslator.NToRN(_szAbout);
  
  CPU_Meta_SetAbout *p = new CPU_Meta_SetAbout(szAbout);

  gLog.Info("%sUpdating about (#%ld/#%d)...\n", L_SRVxSTR, p->Sequence(), p->SubSequence());

  delete [] szAbout;

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
  delete [] sz;

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

//-----icqRequestAuth--------------------------------------------------------
void CICQDaemon::icqRequestAuth(unsigned long _nUin, const char *_szMessage)
{
  CSrvPacketTcp *p = new CPU_RequestAuth(_nUin, _szMessage);
  SendEvent_Server(p);
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

//-----icqSearchWhitePages--------------------------------------------------
unsigned long CICQDaemon::icqSearchWhitePages(const char *szFirstName,
    const char *szLastName, const char *szAlias, const char *szEmail,
    unsigned short nMinAge, unsigned short nMaxAge, char nGender,
    char nLanguage, const char *szCity, const char *szState,
    unsigned short nCountryCode, const char *szCoName, const char *szCoDept,
    const char *szCoPos, const char *szKeyword, bool bOnlineOnly)
{
  // Yes, there are a lot of extra options that you can search by.. but I
  // don't see a point for the hundreds of items that I can add..  just
  // use their web page for that shit - Jon
  CPU_SearchWhitePages *p = new CPU_SearchWhitePages(szFirstName, szLastName,
    szAlias, szEmail, nMinAge, nMaxAge, nGender, nLanguage, szCity, szState,
    nCountryCode, szCoName, szCoDept, szCoPos, szKeyword, bOnlineOnly);
  gLog.Info("%sStarting white pages search (#%ld/#%d)...\n", L_SRVxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  PushExtendedEvent(e);
  return e->EventId();
}

//-----icqSearchByUin----------------------------------------------------------
unsigned long CICQDaemon::icqSearchByUin(unsigned long nUin)
{
   CPU_SearchByUin *p = new CPU_SearchByUin(nUin);
   gLog.Info("%sStarting search by UIN for user (#%ld/#%d)...\n", L_SRVxSTR,
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
    case ICQ_CMDxSUB_CONTACTxLIST:
      gLog.Info("%sSending contact list through server (#%ld).\n", L_SRVxSTR, p->Sequence());
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

unsigned long CICQDaemon::icqSendSms(const char *szNumber, const char *szMessage,
				     unsigned long nUin)
{
  CEventSms *ue = new CEventSms(szNumber, szMessage, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, INT_VERSION);
  CPU_SendSms *p = new CPU_SendSms(szNumber, szMessage);
  gLog.Info("%sSending SMS through server (#%ld/#%d)...\n", L_SRVxSTR,
	    p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(nUin, p, ue);
  PushExtendedEvent(e);
  return e->EventId();
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
  if (m_bProxyEnabled)
    InitProxy();

  int r = ConnectToServer(m_szICQServer, m_nICQServerPort);

  write(pipe_newsocket[PIPE_WRITE], "S", 1);

  return r;
}

int CICQDaemon::ConnectToServer(const char* server, unsigned short port)
{
  SrvSocket *s = new SrvSocket(0);
  
  if (m_bProxyEnabled)
  {
    if (m_xProxy == NULL)
    {
      gLog.Warn("%sProxy server not properly configured.\n", L_ERRORxSTR);
      delete s;
      return (-1);
    }
    s->SetProxy(m_xProxy);
  } else if (m_xProxy != NULL)
  {
    delete m_xProxy;
    m_xProxy = NULL;
  }
  
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

  if (m_xProxy == NULL)
    gLog.Info("%sOpening socket to server.\n", L_SRVxSTR);
  else
    gLog.Info("%sOpening socket to server via proxy.\n", L_SRVxSTR);
  if (!s->OpenConnection())
  {
    char buf[128];
    gLog.Warn("%sUnable to connect to %s:%d:\n%s%s.\n", L_ERRORxSTR,
              s->RemoteIpStr(ipbuf), s->RemotePort(), L_BLANKxSTR,
              s->ErrorStr(buf, 128));
    delete s;
    return -1;
  }

  // Now get the internal ip from this socket
  CPacket::SetLocalIp(  NetworkIpToPacketIp(s->LocalIp() ));
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->SetIntIp(s->LocalIp());
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

//-----FindUinByCellular-------------------------------------------------------
unsigned long CICQDaemon::FindUinByCellular(const char *szCellular)
{
  char szParsedNumber1[16], szParsedNumber2[16];
  unsigned long nUin = 0;
  
  FOR_EACH_USER_START(LOCK_R)
  {
    ParseDigits(szParsedNumber1, pUser->GetCellularNumber(), 15);
    ParseDigits(szParsedNumber2, szCellular, 15);
    if (!strcmp(szParsedNumber1, szParsedNumber2))
	nUin = pUser->Uin();
  }
  FOR_EACH_USER_END

  return nUin;
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
    delete [] buf;
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
    if (m_bRegistering)
    {
      icqRegisterFinish();
      m_bRegistering = false;
    }
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

      gLog.Info("%sRequesting server contact list rights.\n", L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_REQUESTxRIGHTS);
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
    time_t nOnlineSince = 0;
    
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
      delete [] buf;
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
      ICQOwner *owner = gUserManager.FetchOwner(LOCK_W);
      owner->SetIp(realIP);
      gUserManager.DropOwner();

      char buf[32];
      gLog.Info("%sServer says we are at %s.\n", L_SRVxSTR, ip_ntoa(realIP, buf));
      //icqSetStatus(m_nDesiredStatus);
    }
    if (packet.getTLVLen(0x0003) == 4)
      nOnlineSince = packet.UnpackUnsignedLongTLV(0x0003);
    if (packet.getTLVLen(0x000c)) {
      gLog.Unknown("%sServer send us direct conn info,len: %d\n", L_UNKNOWNxSTR,
                   packet.getTLVLen(0x000c));
    }

    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    ChangeUserStatus(o, m_nDesiredStatus);
    o->SetOnlineSince(nOnlineSince);
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
    unsigned long junk1, intIP, userPort, nUin, timestamp, nCookie;
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

    if (packet.getTLVLen(0x000a) == 4) {
      unsigned long userIP = packet.UnpackUnsignedLongTLV(0x000a);
      if (userIP) {
        rev_e_long(userIP);
        userIP = PacketIpToNetworkIp(userIP);
        u->SetIp(userIP);
      }
    }

    if (packet.getTLVLen(0x0003) == 4) {
      time_t nOnlineSince = packet.UnpackUnsignedLongTLV(0x0003);
      u->SetOnlineSince(nOnlineSince);
    }

    if (packet.getTLVLen(0x000c) == 0x25) {
      CBuffer msg = packet.UnpackTLV(0x000c);

      intIP = msg.UnpackUnsignedLong();
      userPort = msg.UnpackUnsignedLongBE();
      mode = msg.UnpackChar();
      unsigned short tcpVersion = msg.UnpackUnsignedShortBE();
      nCookie = msg.UnpackUnsignedLongBE();
      junk1 = msg.UnpackUnsignedLongBE();
      junk1 = msg.UnpackUnsignedLongBE();
      timestamp = msg.UnpackUnsignedLongBE();  // will be licq version
      junk1 = msg.UnpackUnsignedLongBE();
      junk1 = msg.UnpackUnsignedLongBE();
      junk2 = msg.UnpackUnsignedShortBE();


      char szExtraInfo[28] = { 0 };
      if ((timestamp & 0xFFFF0000) == LICQ_WITHSSL)
        snprintf(szExtraInfo, 27, " [Licq %s/SSL]",
                 CUserEvent::LicqVersionToString(timestamp & 0xFFFF));
      else if ((timestamp & 0xFFFF0000) == LICQ_WITHOUTSSL)
        snprintf(szExtraInfo, 27, " [Licq %s]",
                 CUserEvent::LicqVersionToString(timestamp & 0xFFFF));
      else if (timestamp == 0xffffffff)
        strcpy(szExtraInfo, " [MIRANDA]");
      else
        strcpy(szExtraInfo, "");
      szExtraInfo[27] = '\0';

      if (u->StatusFull() != nNewStatus)
      {
        ChangeUserStatus(u, nNewStatus);
        gLog.Info("%s%s (%ld) changed status: %s (v%01x)%s.\n", L_SRVxSTR, u->GetAlias(),
                  nUin, u->StatusStr(), tcpVersion & 0x0F, szExtraInfo);
        if ( (nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS) )
          gLog.Unknown("%sUnknown status flag for %s (%ld): 0x%08lX\n",
                       L_UNKNOWNxSTR, u->GetAlias(), nUin, (nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS));
        nNewStatus &= ICQ_STATUS_FxUNKNOWNxFLAGS;
      }

      if (intIP)
      {
        intIP = PacketIpToNetworkIp(intIP);
        u->SetIntIp(intIP);
      }
      
      if (userPort)
        u->SetPort(userPort);

      u->SetVersion(tcpVersion);
      u->SetCookie(nCookie);
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
  /*unsigned long Flags =*/ packet.UnpackUnsignedLongBE();
  /*unsigned short nSubSequence =*/ packet.UnpackUnsignedShortBE();

  switch (nSubtype)
  {
  case ICQ_SNACxMSG_SERVERxMESSAGE:
  {
    unsigned long nMsgID[2], nUin;
    unsigned long nTimeSent;
    unsigned short mFormat, nMsgLen, nTLVs;

    nMsgID[0] = packet.UnpackUnsignedLongBE();
    nMsgID[1] = packet.UnpackUnsignedLongBE();
    nTimeSent   = time(0L);
    mFormat    = packet.UnpackUnsignedShortBE();
    nUin       = packet.UnpackUinString();

    if (nUin < 10000 && nUin != ICQ_UINxPAGER && nUin != ICQ_UINxSMS)
    {
      gLog.Warn("%sMessage through server with strange Uin: %04lx\n", L_WARNxSTR, nUin);
      break;
    }

    packet.UnpackUnsignedShortBE();  // warning level ?
    nTLVs = packet.UnpackUnsignedShortBE() + 1;
    if (nTLVs && !packet.readTLV())
    {
      gLog.Error("%sIvalid packet format in message through server\n", L_ERRORxSTR);
      return;
    }

    switch (mFormat)
    {
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
      CEventMsg *e = CEventMsg::Parse(szMsg, ICQ_CMDxRCV_SYSxMSGxONLINE, nTimeSent, 0);
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
      //I must admit, any server that does anything like this is a pile of shit
      CBuffer msgTxt = packet.UnpackTLV(5);
			if (msgTxt.getDataSize() == 0)  break;

      unsigned short nCancel = msgTxt.UnpackUnsignedShort();

      if (nCancel == 1)  break;

      msgTxt.incDataPosRead(24); // junk before tlv
      msgTxt.readTLV();

			CBuffer ackMsg = msgTxt.UnpackTLV(0x000A);
			if (ackMsg.getDataSize() == 0)  break;
			bool bIsAck = (ackMsg.UnpackUnsignedShortBE() == 2 ? true : false);

      CBuffer advMsg = msgTxt.UnpackTLV(0x2711);
			if (advMsg.getDataSize() == 0)  break;

      unsigned short nLen;
      unsigned char nMsgType, nMsgFlags;
     
      nLen = advMsg.UnpackUnsignedShort();
      advMsg.incDataPosRead(nLen - 2);
      unsigned short nSequence = advMsg.UnpackUnsignedShort();

      nLen = advMsg.UnpackUnsignedShort();
			if (nLen == 0x12) // what is this, a "you're on my list" statement?
				break;
      advMsg.incDataPosRead(nLen);

			unsigned long nMask = 0;
			nMsgType = advMsg.UnpackChar();
			nMsgFlags = advMsg.UnpackChar();
			if (nMsgFlags & 0x80)
				nMask |= ICQ_CMDxSUB_FxMULTIREC;
			advMsg.UnpackUnsignedLong();

      nLen = advMsg.UnpackUnsignedShort();
	
			// read the message in, excpet for DOS \r's
			char junkChar;
			char message[nLen+1];
			unsigned short j = 0;
			for (unsigned short i = 0; i < nLen; i++)
			{
				advMsg >> junkChar;
				if (junkChar != 0x0D)  message[j++] = junkChar;
			}
			message[j] = '\0'; // ensure null terminated

			// translate now
			gTranslator.ServerToClient(message);

			bool bNewUser = false;
			ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
			if (u == NULL)
			{
				u = new ICQUser(nUin);
				bNewUser = true;
			}

			ProcessMessage(u, advMsg, message, nMsgType, nMask, nMsgID,
										 nSequence, bIsAck, bNewUser);

			if (bNewUser) // can be changed in ProcessMessage
			{
				delete u;
				break;
			}

			gUserManager.DropUser(u);
			break;
    }
    case 4:
    //Version 5 (V5) protocol messages incapsulated in v7 packet.
    {
      CBuffer msgTxt = packet.UnpackTLV(0x0005);
      msgTxt.UnpackUnsignedLongBE();
      unsigned short nTypeMsg = msgTxt.UnpackUnsignedShort();
      unsigned long nMask = ((nTypeMsg & ICQ_CMDxSUB_FxMULTIREC) ? E_MULTIxREC : 0);
      nTypeMsg &= ~ICQ_CMDxSUB_FxMULTIREC;

      char *szMessage = NULL;

      if (nTypeMsg == ICQ_CMDxSUB_SMS)
      {
        msgTxt.incDataPosRead(21);

        unsigned short nTypeSMS = msgTxt.UnpackUnsignedShort();
        switch (nTypeSMS)
	{
          case 0x0000:
	    // SMS
	    break;
	  case 0x0002:
	    // SMS Receipt: Success (meanwhile, we handle it in a rather lame way)
	    gLog.Unknown("%sReceived SMS receipt indicating success.\n", L_UNKNOWNxSTR);
	    return;
	  case 0x0003:
	    // SMS Receipt : Failure
	    gLog.Unknown("%sReceived SMS receipt indicating failure.\n", L_UNKNOWNxSTR);
	    return;
	  default:
	    char *buf;
	    gLog.Unknown("%sUnknown SMS subtype (0x%04x):\n%s\n", L_UNKNOWNxSTR, nTypeSMS, packet.print(buf));
	    delete [] buf;
	    return;
        }

	unsigned long nTagLength = msgTxt.UnpackUnsignedLong();
	// Refuse irreasonable tag sizes
	if (nTagLength > 255)
	{
	  gLog.Unknown("%sInvalid tag in SMS message.", L_UNKNOWNxSTR);
	  return;
	}
	
	char* szTag = new char[nTagLength + 1];
	for (unsigned long i = 0; i < nTagLength; ++i)
	  szTag[i] = msgTxt.UnpackChar();
	szTag[nTagLength] = '\0';
        
	if (strcmp(szTag, "ICQSMS") != 0)
	{
          gLog.Unknown("%sUnknown tag in SMS message:\n%s\n", L_UNKNOWNxSTR, szTag);
          delete [] szTag;
          return;
        }
        delete [] szTag;

        msgTxt.incDataPosRead(3);

        msgTxt.UnpackUnsignedLong(); // length till end of the message (useless)
        unsigned long nSMSLength = msgTxt.UnpackUnsignedLong();
        // Refuse irreasonable SMS sizes (something must've went wrong)
        if (nSMSLength > 0x7fff)
	{
          gLog.Unknown("%sSMS message packet was too large (claimed size: %lu bytes)\n", L_UNKNOWNxSTR, nSMSLength);
          return;
        }
        szMessage = new char[nSMSLength+1];
        for (unsigned long i = 0; i < nSMSLength; ++i)
          szMessage[i] = msgTxt.UnpackChar();
        szMessage[nSMSLength] = '\0';
      }
      else
      {
        // new unpack the message
        nMsgLen = msgTxt.UnpackUnsignedShort();
        char* szMsg = new char[nMsgLen];
        for (int i = 0; i < nMsgLen; ++i)
          szMsg[i] = msgTxt.UnpackChar();

        szMessage = gTranslator.RNToN(szMsg);
        delete [] szMsg;
      }

      char *szType = NULL;
      unsigned short nTypeEvent = 0;
      CUserEvent *eEvent = NULL;
      
      switch(nTypeMsg)
      {
	case ICQ_CMDxSUB_MSG:
	{
	  CEventMsg *e = CEventMsg::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, nTimeSent, nMask);
	  szType = strdup("Message");
	  nTypeEvent = ON_EVENT_MSG;
	  eEvent = e;
	break;
	}
	case ICQ_CMDxSUB_URL:
	{
	  CEventUrl *e = CEventUrl::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, nTimeSent, nMask);
	  if (e == NULL)
	  {
	    char *buf;

	    gLog.Warn("%sInvalid URL message:\n%s\n", L_WARNxSTR, packet.print(buf));
	    delete [] buf;
	    break;
	  }
	  szType = strdup("URL");
	  nTypeEvent = ON_EVENT_URL;
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_AUTHxREQUEST:
	{
	  gLog.Info("%sAuthorization request from %ld.\n", L_SBLANKxSTR, nUin);

	  char **szFields = new char *[6];  // alias, first name, last name, email, auth, comment
 
	  if (!ParseFE(szMessage, &szFields, 6))
	  {
	    char *buf;

	    gLog.Warn("%sInvalid authorization request system message:\n%s\n", L_WARNxSTR,
									packet.print(buf));

	    delete [] buf;
	    delete [] szFields;
	    break;
	  }

	  // translating string with Translation Table
	  gTranslator.ServerToClient (szFields[0]);  // alias
	  gTranslator.ServerToClient (szFields[1]);  // first name
	  gTranslator.ServerToClient (szFields[2]);  // last name
	  gTranslator.ServerToClient (szFields[5]);  // comment

	  CEventAuthRequest *e = new CEventAuthRequest(nUin, szFields[0],
						       szFields[1], szFields[2],
						       szFields[3], szFields[5],
                                            	       ICQ_CMDxRCV_SYSxMSGxONLINE,
						       nTimeSent, 0);
	  delete [] szFields;	
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_AUTHxREFUSED:  // system message: authorization refused
	{
	  gLog.Info("%sAuthorization refused by %ld.\n", L_SBLANKxSTR, nUin);

	  // Translating string with Translation Table
	  gTranslator.ServerToClient(szMessage);

	  CEventAuthRefused *e = new CEventAuthRefused(nUin, szMessage,
						       ICQ_CMDxRCV_SYSxMSGxONLINE,
						       nTimeSent, 0);
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_AUTHxGRANTED:  // system message: authorized
	{
	  gLog.Info("%sAuthorization granted by %ld.\n", L_SBLANKxSTR, nUin);

	  // translating string with Translation Table
	  gTranslator.ServerToClient (szMessage);

	  CEventAuthGranted *e = new CEventAuthGranted(nUin, szMessage,
						       ICQ_CMDxRCV_SYSxMSGxONLINE,
						       nTimeSent, 0);
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_MSGxSERVER:
	{
	  gLog.Info("%sServer message.\n", L_BLANKxSTR);
	  
	  CEventServerMessage *e = CEventServerMessage::Parse(szMessage, ICQ_CMDxSUB_MSGxSERVER, nTimeSent, nMask);
	  if (e == NULL)
	  {
	    char *buf;
	    
	    gLog.Warn("%sInvalid Server Message:\n%s\n", L_WARNxSTR, packet.print(buf));
	    delete [] buf;
	    break;
	  }
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_ADDEDxTOxLIST: // system message: added to a contact list
	{
	  gLog.Info("%sUser %ld added you to their contact list.\n", L_SBLANKxSTR, nUin);

	  char **szFields = new char*[6]; // alias, first name, last name, email, auth, comment
          
	  if (!ParseFE(szMessage, &szFields, 6))
	  {
	    char *buf;
	    
	    gLog.Warn("%sInvalid added to list system message:\n%s\n", L_WARNxSTR,
								packet.print(buf));
	    delete [] buf;
	    delete [] szFields;
	    break;
	  }

	  // translating string with Translation Table
	  gTranslator.ServerToClient (szFields[0]);  // alias
	  gTranslator.ServerToClient (szFields[1]);  // first name
	  gTranslator.ServerToClient (szFields[2]);  // last name

	  CEventAdded *e = new CEventAdded(nUin, szFields[0], szFields[1],
					   szFields[2], szFields[3],
					   ICQ_CMDxRCV_SYSxMSGxONLINE,
					   nTimeSent, 0);
	  delete [] szFields;	
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_WEBxPANEL:
	{
	  gLog.Info("%sMessage through web panel.\n", L_SBLANKxSTR);

	  char **szFields = new char *[6];	// name, ?, ?, email, ?, message

	  if (!ParseFE(szMessage, &szFields, 6))
	  {
	    char *buf;
	    gLog.Warn("%sInvalid web panel system message:\n%s\n", L_WARNxSTR,
							    packet.print(buf));
	    delete [] buf;
	    delete [] szFields;
	    break;
	  }

	  // translating string with Translation Table
	  gTranslator.ServerToClient(szFields[0]);  // name
	  gTranslator.ServerToClient(szFields[3]);  // email
	  gTranslator.ServerToClient(szFields[5]);  // message

	  gLog.Info("%sFrom %s (%s).\n", L_SBLANKxSTR, szFields[0], szFields[3]);
	  CEventWebPanel *e = new CEventWebPanel(szFields[0], szFields[3],
						 szFields[5], ICQ_CMDxRCV_SYSxMSGxONLINE,
						 nTimeSent, 0);
	  delete [] szFields;	
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_EMAILxPAGER:
	{
	  gLog.Info("%sEmail pager message.\n", L_SBLANKxSTR);

	  char **szFields = new char *[6];	// name, ?, ?, email, ?, message
        
	  if (!ParseFE(szMessage, &szFields, 6))
	  {
	    char *buf;

	    gLog.Warn("%sInvalid email pager system message:\n%s\n", L_WARNxSTR,
							    packet.print(buf));
	    delete [] buf;
	    delete [] szFields;
	    break;
	  }

	  // translating string with Translation Table
	  gTranslator.ServerToClient(szFields[0]);  // name
	  gTranslator.ServerToClient(szFields[3]);  // email
	  gTranslator.ServerToClient(szFields[5]);  // message

	  gLog.Info("%sFrom %s (%s).\n", L_SBLANKxSTR, szFields[0], szFields[3]);
	  CEventEmailPager *e = new CEventEmailPager(szFields[0], szFields[3],
						     szFields[5], ICQ_CMDxRCV_SYSxMSGxONLINE,
						     nTimeSent, 0);
	  delete [] szFields;	
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_CONTACTxLIST:
	{
	  CEventContactList *e = CEventContactList::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE,nTimeSent, nMask);
	  if (e == NULL)
	  {
	    char *buf;

	    gLog.Warn("%sInvalid Contact List message:\n%s\n", L_WARNxSTR, packet.print(buf));
	    delete [] buf;
	    break;
	  }
	  szType = strdup("Contacts");
	  nTypeEvent = ON_EVENT_MSG;
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_SMS:
	{
	  CEventSms *e = CEventSms::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, nTimeSent, nMask);
	  if (e == NULL)
	  {
	    char *buf;

	    gLog.Warn("%sInvalid SMS message:\n%s\n", L_WARNxSTR, packet.print(buf));
	    delete [] buf;
	    break;
	  }
	  eEvent = e;
	  break;
	}
	default:
	{
	  char *szFE, *buf;

	  while ((szFE = strchr(szMessage, 0xFE)) != NULL) *szFE = '\n';
	  
	  gLog.Unknown("%sUnknown system message (0x%04x):\n%s\n", L_UNKNOWNxSTR,
						    nTypeMsg, packet.print(buf));
	  delete [] buf;
	  CEventUnknownSysMsg *e = new CEventUnknownSysMsg(nTypeMsg, ICQ_CMDxRCV_SYSxMSGxONLINE,
							   nUin, szMessage, nTimeSent, 0);

	  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
	  AddUserEvent(o, e);
	  gUserManager.DropOwner();
	}
      }

      if (eEvent)
	switch(nTypeMsg)
	{
	  case ICQ_CMDxSUB_MSG:
	  case ICQ_CMDxSUB_URL:
	  case ICQ_CMDxSUB_CONTACTxLIST:
	  {
	    // Lock the user to add the message to their queue
	    ICQUser* u = gUserManager.FetchUser(nUin, LOCK_W);
	    if (u == NULL)
	    {
	      if (Ignore(IGNORE_NEWUSERS))
	      {
		gLog.Info("%s%s from new user (%ld), ignoring.\n", L_SBLANKxSTR, szType, nUin);
		if (szType) free(szType);
		RejectEvent(nUin, eEvent);
		break;
	      }
	      gLog.Info("%s%s from new user (%ld).\n", L_SBLANKxSTR, szType, nUin);
	      AddUserToList(nUin);
	      u = gUserManager.FetchUser(nUin, LOCK_W);
	    }
	    else
	      gLog.Info("%s%s through server from %s (%ld).\n", L_SBLANKxSTR,
	    					szType, u->GetAlias(), nUin);
	    if (szType) free(szType);
	    if (AddUserEvent(u, eEvent))
	      m_xOnEventManager.Do(nTypeEvent, u);
	    gUserManager.DropUser(u);
	    break;
	  }
	  case ICQ_CMDxSUB_AUTHxREQUEST:
	  case ICQ_CMDxSUB_AUTHxREFUSED:
	  case ICQ_CMDxSUB_AUTHxGRANTED:
	  case ICQ_CMDxSUB_MSGxSERVER:
	  case ICQ_CMDxSUB_ADDEDxTOxLIST:
	  case ICQ_CMDxSUB_WEBxPANEL:
	  case ICQ_CMDxSUB_EMAILxPAGER:
	  {
	    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
	    if (AddUserEvent(o, eEvent))
	    {
	      gUserManager.DropOwner();
	      eEvent->AddToHistory(NULL, D_RECEIVER);
	      m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
	    }
	    else
	      gUserManager.DropOwner();
	    break;
	  }
	  case ICQ_CMDxSUB_SMS:
	  {
	    CEventSms *eSms = (CEventSms *)eEvent; 
	    unsigned long nUinSms = FindUinByCellular(eSms->Number());
	    
	    if (nUinSms != 0)
	    {
	      ICQUser* u = gUserManager.FetchUser(nUinSms, LOCK_W);
	      gLog.Info("%sSMS from %s - %s (%ld).\n", L_SBLANKxSTR, eSms->Number(), u->GetAlias(), nUinSms);
	      if (AddUserEvent(u, eEvent))
	        m_xOnEventManager.Do(ON_EVENT_SMS, u);
	      gUserManager.DropUser(u);
	    }
	    else
	    {  
	      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
	      gLog.Info("%sSMS from %s.\n", L_BLANKxSTR, eSms->Number());
	      if (AddUserEvent(o, eEvent))
	      {
	        gUserManager.DropOwner();
	        eEvent->AddToHistory(NULL, D_RECEIVER);
	        m_xOnEventManager.Do(ON_EVENT_SMS, NULL);
	      }
	      else
	        gUserManager.DropOwner();
	    }
	    break;
	  }
	}

      delete [] szMessage;
      break;
    }

    default:
      char *buf;
      
      gLog.Unknown("%sMessage through server with unknown format: %04hx\n%s\n",
				    L_ERRORxSTR, mFormat, packet.print(buf));
      delete [] buf;
      break;
    }
    break;
  }
  case ICQ_SNACxMSG_SERVERxREPLYxMSG:
  {
		unsigned short nFormat, nLen, nSequence, nMsgType, nAckFlags, nMsgFlags;
		unsigned long nUin, nMsgID;
		ICQUser *u = NULL;

	 	packet.incDataPosRead(4); // msg id
		nMsgID = packet.UnpackUnsignedLongBE(); // lower bits, what licq uses
		nFormat = packet.UnpackUnsignedShortBE();
		nUin = packet.UnpackUinString();

		u = gUserManager.FetchUser(nUin, LOCK_W);
		if (u == NULL)
		{
			gLog.Warn("%sUnexpected new user in subtype 0x%04x.\n", L_SRVxSTR,
								nSubtype);
			break;
		}

		packet.incDataPosRead(2);
		packet >> nLen;
		packet.incDataPosRead(nLen);
		packet >> nLen;
		nSequence = packet.UnpackUnsignedShortBE();
		packet.incDataPosRead(nLen-2);
		packet >> nMsgType >> nAckFlags >> nMsgFlags >> nLen;

		char szMessage[nLen];
		for (unsigned short i = 0; i < nLen; i++)
			packet >> szMessage[i];
		gTranslator.ServerToClient(szMessage);

		CExtendedAck *pExtendedAck;
		int nSubResult;
		if (nAckFlags == ICQ_TCPxACK_REFUSE)
		{
			pExtendedAck = new CExtendedAck(false, 0, szMessage);
			nSubResult = ICQ_TCPxACK_ACCEPT;
			gLog.Info("%sRefusal from %s (#%ld).\n", L_SRVxSTR, u->GetAlias(),
								nMsgID);
		}
		else
		{
			// Update the away message if it's changed
			if (strcmp(u->AutoResponse(), szMessage))
			{
				u->SetAutoResponse(szMessage);
				u->SetShowAwayMsg(*szMessage);
				gLog.Info("%sAuto response from %s (#%ld).\n", L_SRVxSTR, u->GetAlias(),
									nMsgID);
			}
			
			pExtendedAck = new CExtendedAck(false, 0, szMessage);
			nSubResult = ICQ_TCPxACK_RETURN;
		}

		gUserManager.DropUser(u);
		ICQEvent *e = DoneServerEvent(nMsgID, EVENT_ACKED);
		if (e)
		{
			e->m_pExtendedAck = pExtendedAck;
			e->m_nSubResult = nSubResult;
			ProcessDoneEvent(e);
			return;
		}
		else
		{
			gLog.Warn("%sAck for unknown event.\n", L_SRVxSTR);
			break;
		}

		break;
	}
	case ICQ_SNACxMSG_RIGHTSxGRANTED:
	{
		gLog.Info("%sReceived rights for Instant Messaging..\n", L_SRVxSTR);
		break;
	}
	default:
	{
		gLog.Warn("%sUnknown Message Family Subtype: %04hx\n", L_SRVxSTR, nSubtype);
		break;
	}
	}
}

//--------ProcessListFam--------------------------------------------
void CICQDaemon::ProcessListFam(CBuffer &packet, unsigned short nSubtype)
{
  /*unsigned short nFlags = */packet.UnpackUnsignedShortBE();
  unsigned long nSubSequence = packet.UnpackUnsignedLongBE();

  switch (nSubtype)
  {
    case ICQ_SNACxLIST_RIGHTSxGRANTED:
    {
      gLog.Info("%sServer granted Server Contact List.\n", L_SRVxSTR);
      if (UseServerContactList())
      {
        gLog.Info("%sRequesting Server Contact List.\n", L_SRVxSTR);

        CSrvPacketTcp *p = new CPU_RequestList();
        SendEvent_Server(p);
      }
      break;
    }

    case ICQ_SNACxLIST_ROSTxREPLY:
    {
      unsigned short nCount;
      unsigned long nTime;

      gLog.Info("%sReceived contact list.\n", L_SRVxSTR);

      packet.UnpackChar();  // unknown
      nCount = packet.UnpackUnsignedShortBE();

      for (unsigned short i = 0; i < nCount; i++)
      {
        char *szName;
        unsigned short nTag, nID, nType, nByteLen;

        // Can't use UnpackUinString because this may be a group name
        szName = packet.UnpackStringBE();
        nTag = packet.UnpackUnsignedShortBE();
        nID = packet.UnpackUnsignedShortBE();
        nType = packet.UnpackUnsignedShortBE();
        nByteLen = packet.UnpackUnsignedShortBE();

        if (nByteLen)
        {
          if (!packet.readTLV(-1, nByteLen))
          {
            gLog.Error("%sUnable to parse contact list TLV, aborting!\n",
											 L_ERRORxSTR);
            delete[] szName;
            return;
          }
        }

        switch (nType)
        {
          case ICQ_ROSTxNORMAL:
          case ICQ_ROSTxVISIBLE:
          case ICQ_ROSTxINVISIBLE:
          case ICQ_ROSTxIGNORE:
          {
            char *szNewName = packet.UnpackStringTLV(0x0131);
            unsigned long nUin = atoi(szName);
            unsigned short nInGroup = gUserManager.GetGroupFromID(nTag);
            AddUserToList(nUin, false); // Don't notify server

            ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
            if (u)
            {
              u->SetSID(nID);
              u->SetGSID(nTag);

              if (szNewName)
                u->SetAlias(szNewName, false);

              if (nType == ICQ_ROSTxINVISIBLE)  u->SetInvisibleList(true);
              else if (nType == ICQ_ROSTxVISIBLE) u->SetVisibleList(true);
              else if (nType == ICQ_ROSTxIGNORE) u->SetIgnoreList(true);

              // They aren't a new user if we added them to a server list
              u->SetNewUser(false);

              // Skip the call to AddUserToGroup
              u->AddToGroup(GROUPS_USER, nInGroup);
              gUserManager.DropUser(u);
            }

            gLog.Info("%sAdded %s (%ld) to list from server.\n", L_SRVxSTR,
              (szNewName ? szNewName : ""), nUin);
            if (szNewName)
              delete [] szNewName;

            break;
          }

          case ICQ_ROSTxGROUP:
          {
            if (szName[0] != '\0')
            {
              // Rename the group if we have it already or else add it
              unsigned short nGroup = gUserManager.GetGroupFromID(nTag);
              if (nGroup == gUserManager.NumGroups())
              {
                if (!gUserManager.AddGroup(szName, nTag))
                  gUserManager.ModifyGroupID(szName, nTag);
              }
              else
              {
                gUserManager.RenameGroup(nGroup, szName, false);
              }
            }
            break;
          }
        }  // switch (nType)

        packet.cleanupTLV();
        delete[] szName;
      } // for count

      // Update local info about contact list
      nTime = packet.UnpackUnsignedLongBE();
      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
      bool bFirstTime = o->GetSSTime() ? false: true;
      o->SetSSTime(nTime);
      o->SetSSCount(nCount);
      gUserManager.DropOwner();

      // Is this our first time?  Let's update server with local info!
      if (bFirstTime)
      {
        GroupList *g = gUserManager.LockGroupList(LOCK_R);
        GroupIDList *gID = gUserManager.LockGroupIDList(LOCK_R);

        GroupList groups;
        unsigned long n = 0;
        for (unsigned int i = 0; i < gID->size(); i++)
        {
          if ((*gID)[i] == 0)
          {
            n++;
            groups.push_back((*g)[i]);
          }
        }

        gUserManager.UnlockGroupList();
        gUserManager.UnlockGroupIDList();

        if (n)
        {
          pthread_mutex_lock(&mutex_modifyserverusers);
          m_lszModifyServerUsers.push_back(strdup("Top Level Group"));
          pthread_mutex_unlock(&mutex_modifyserverusers);
          icqExportGroups(groups);
        }

        // Export users on a per group basis for the update group packet to work
        g = gUserManager.LockGroupList(LOCK_R);
        for (unsigned int j = 1; j <= g->size(); j++)
        {
          UinList uins;
          n = 0;
          FOR_EACH_USER_START(LOCK_R)
          {
            if (pUser->GetInGroup(GROUPS_USER, j) && pUser->GetSID() == 0)
            {
              n++;
              uins.push_back(pUser->Uin());
            }
          }
          FOR_EACH_USER_END

          if (n)
          {
            pthread_mutex_lock(&mutex_modifyserverusers);
            m_lszModifyServerUsers.push_back(strdup((*g)[j-1]));
            pthread_mutex_unlock(&mutex_modifyserverusers);
            icqExportUsers(uins);
            n = 0;
          }
        }
        gUserManager.UnlockGroupList();
      }

      break;
    } // case rost reply

    case ICQ_SNACxLIST_UPDxACK:
    {
      packet.UnpackUnsignedLong(); // unknown
      packet.UnpackUnsignedLong(); // unknown TLV(1)

      ICQEvent *e = DoneServerEvent(nSubSequence, EVENT_SUCCESS);

      if (e == NULL)
      {
        gLog.Warn("%sServer list update ack without request.\n", L_SRVxSTR);
        break;
      }

      CSrvPacketTcp *pReply = 0;
      bool bHandled = false;
      unsigned short nError = 0;

      do
      {
        pthread_mutex_lock(&mutex_modifyserverusers);
        char *szPending = m_lszModifyServerUsers.front();
        m_lszModifyServerUsers.pop_front();
        pthread_mutex_unlock(&mutex_modifyserverusers);

        unsigned long nUin = 0;
        if (szPending)
          nUin = atol(szPending);

        nError = packet.UnpackUnsignedShortBE();

        switch (nError)
        {
          case 0: break; // No error

          case 0x0E:
            // no way
            //gLog.Warn("%s%s added to awaiting authorization group on server list\n",
            //           L_SRVxSTR, szPending);
            break;

          case 0x02:
            gLog.Error("%sUser/Group %s not found on server list.\n",
                       L_SRVxSTR,szPending);
            break;

          default:
            gLog.Error("%sUnknown error modyifing server list: 0x%02X\n",
                       L_SRVxSTR, nError & 0xFF);
        }

        if (nError && nError != 0x0E)
          break;

        switch (e->SubType())
        {
          case ICQ_SNACxLIST_ROSTxADD:
          case ICQ_SNACxLIST_ROSTxREM:
          {
            if (nError == 0x0E)
            {
              // If we don't send this, we can still add them
              // without any known side effects
              //pReply = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
              //  ICQ_SNACxLIST_ROSTxEDITxEND);
              //SendEvent_Server(pReply);

              icqAddUser(nUin, true, true);
              break;
            }

            if (bHandled == false)
            {
              bHandled = true;
              GroupList *g = gUserManager.LockGroupList(LOCK_R);
              pthread_mutex_lock(&mutex_modifyserverusers);
              char *szGroupName =
                strdup( (*g)[gUserManager.GetGroupFromID(
                e->ExtraInfo())-1] );
              m_lszModifyServerUsers.push_back(szGroupName);
              pthread_mutex_unlock(&mutex_modifyserverusers);
              gUserManager.UnlockGroupList();

              pReply = new CPU_UpdateToServerList(szGroupName, ICQ_ROSTxGROUP,
                                                  e->ExtraInfo());
              gLog.Info("%sUpdating group %s.\n", L_SRVxSTR, szGroupName);
              SendExpectEvent_Server(0, pReply, NULL);
            }

            ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
            unsigned short nSSCount = o->GetSSCount();
            // Skip the call to gUserManager.AddUserToGroup because
            // that will send a message out to the server AGAIN
            if (e->SubType() == ICQ_SNACxLIST_ROSTxADD)
            {
              ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
              if (u)
              {
                u->AddToGroup(GROUPS_USER, gUserManager.GetGroupFromID(
                              e->ExtraInfo()));
                gUserManager.DropUser(u);
              }
              o->SetSSCount(++nSSCount);
              
              // wait for them to be added before we get their info
              // which in turn will update their alias on the server
              icqUserBasicInfo(nUin); // since it isn't done in icqAddUser
            }
            else
            {
              if (nSSCount > 0)
                o->SetSSCount(--nSSCount);
            }
            gUserManager.DropOwner();
	    
            break;
          }

          case ICQ_SNACxLIST_ROSTxUPD_GROUP:
          {
            if (bHandled == false)
            {
              bHandled = true;
              if (nUin == 0)
              {
                pReply = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
                                               ICQ_SNACxLIST_ROSTxEDITxEND);
                gLog.Info("%sGroup %s updated successfully.\n", L_SRVxSTR,
                          szPending);
                SendEvent_Server(pReply);
              }
              else
              {
                gLog.Info("%sUser %s updated successfully.\n", L_SRVxSTR,
                          szPending);
              }
            }

            ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
            o->SetSSTime(time(NULL));
            gUserManager.DropOwner();

            break;
          }
        }
        free(szPending);
      } while (!packet.End());

      break;
    }

    default:
      gLog.Warn("%sUnknown List Family Subtype: %04hx\n", L_SRVxSTR, nSubtype);
      break;
  } // switch subtype
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
  char *tmp;

  switch (nSubtype)
  {
  case 0x0003: // multi-purpose sub-type
  {
    unsigned short nLen, nType, nId;

    if (!packet.readTLV()) {
      char *buf;
      gLog.Unknown("%sUnknown server response:\n%s\n", L_UNKNOWNxSTR,
         packet.print(buf));
      delete [] buf;
      break;
    }

    CBuffer msg = packet.UnpackTLV(0x0001);

    if (msg.Empty()) {
      char *buf;
      gLog.Unknown("%sUnknown server response:\n%s\n", L_UNKNOWNxSTR,
         packet.print(buf));
      delete [] buf;
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
      unsigned long nTimeSent;
      
      nUin = msg.UnpackUnsignedLong();

      sendTM.tm_year = msg.UnpackUnsignedShort() - 1900;
      sendTM.tm_mon = msg.UnpackChar() - 1;
      sendTM.tm_mday = msg.UnpackChar();
      sendTM.tm_hour = msg.UnpackChar();
      sendTM.tm_min = msg.UnpackChar();
      sendTM.tm_sec = 0;
      sendTM.tm_isdst = -1;
      nTimeSent = mktime(&sendTM);
      
      // Msg type & flags
      unsigned short nTypeMsg = msg.UnpackUnsignedShort();
      unsigned long nMask = ((nTypeMsg & ICQ_CMDxSUB_FxMULTIREC) ? E_MULTIxREC : 0);
      nTypeMsg &= ~ICQ_CMDxSUB_FxMULTIREC;
      
      char* szMessage = new char[msg.getDataMaxSize()];
      // 2 byte length little endian + string
      msg.UnpackString(szMessage, msg.getDataMaxSize());      
      char *szType = NULL;
      unsigned short nTypeEvent = 0;
      CUserEvent *eEvent = NULL;
            
      switch(nTypeMsg)
      {
        case ICQ_CMDxSUB_MSG:
	{
          CEventMsg *e = CEventMsg::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, nMask);
	  szType = strdup("Message");
	  nTypeEvent = ON_EVENT_MSG;
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_URL:
	{
          CEventUrl *e = CEventUrl::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, nMask);
	  if (e == NULL)
	  {
	    char *buf;
	    
	    gLog.Warn("%sInvalid offline URL message:\n%s\n", L_WARNxSTR, packet.print(buf));
	    delete [] buf;
	    break;
	  }
	  szType = strdup("URL");
	  nTypeEvent = ON_EVENT_URL;
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_AUTHxREQUEST:
	{
	  gLog.Info("%sOffline authorization request from %ld.\n", L_SBLANKxSTR, nUin);
	  
	  char **szFields = new char *[6];  // alias, first name, last name, email, auth, comment
          
	  if (!ParseFE(szMessage, &szFields, 6))
          {
            char *buf;
	    
            gLog.Warn("%sInvalid offline authorization request system message:\n%s\n", L_WARNxSTR,
		      packet.print(buf));
	    delete [] buf;
            delete [] szFields;
            break;
          }

          // translating string with Translation Table
          gTranslator.ServerToClient (szFields[0]);  // alias
          gTranslator.ServerToClient (szFields[1]);  // first name
          gTranslator.ServerToClient (szFields[2]);  // last name
          gTranslator.ServerToClient (szFields[5]);  // comment

          CEventAuthRequest *e = new CEventAuthRequest(nUin, szFields[0], szFields[1],
                                                       szFields[2], szFields[3], szFields[5],
                                            	       ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, 0);
	  delete [] szFields;	
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_AUTHxREFUSED:  // system message: authorization refused
        {
          gLog.Info("%sOffline authorization refused by %ld.\n", L_SBLANKxSTR, nUin);

          // Translating string with Translation Table
          gTranslator.ServerToClient(szMessage);

          CEventAuthRefused *e = new CEventAuthRefused(nUin, szMessage, ICQ_CMDxRCV_SYSxMSGxOFFLINE,
						       nTimeSent, 0);
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_AUTHxGRANTED:  // system message: authorized
        {
          gLog.Info("%sOffline authorization granted by %ld.\n", L_SBLANKxSTR, nUin);

          // translating string with Translation Table
          gTranslator.ServerToClient (szMessage);

          CEventAuthGranted *e = new CEventAuthGranted(nUin, szMessage, ICQ_CMDxRCV_SYSxMSGxOFFLINE,
                                                       nTimeSent, 0);
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_MSGxSERVER:
	{
	  gLog.Info("%sOffline server message.\n", L_BLANKxSTR);
	  
	  CEventServerMessage *e = CEventServerMessage::Parse(szMessage, ICQ_CMDxSUB_MSGxSERVER, nTimeSent, nMask);
	  if (e == NULL)
	  {
	    char *buf;

	    gLog.Warn("%sInvalid Server Message:\n%s\n", L_WARNxSTR, packet.print(buf));
	    delete [] buf;
	    break;
	  }
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_ADDEDxTOxLIST:  // system message: added to a contact list
        {
          gLog.Info("%sUser %ld added you to their contact list (offline).\n", L_SBLANKxSTR, nUin);

          char **szFields = new char *[6]; // alias, first name, last name, email, auth, comment
          
	  if (!ParseFE(szMessage, &szFields, 6))
          {
            char *buf;
	    
            gLog.Warn("%sInvalid offline added to list system message:\n%s\n", L_WARNxSTR,
                      packet.print(buf));
            delete [] buf;
            delete [] szFields;
            break;
          }

          // translating string with Translation Table
          gTranslator.ServerToClient (szFields[0]);  // alias
          gTranslator.ServerToClient (szFields[1]);  // first name
          gTranslator.ServerToClient (szFields[2]);  // last name

          CEventAdded *e = new CEventAdded(nUin, szFields[0], szFields[1],
                                           szFields[2], szFields[3],
                                           ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, 0);
	  delete [] szFields;	
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_WEBxPANEL:
        {
          gLog.Info("%sOffline message through web panel.\n", L_SBLANKxSTR);

          char **szFields = new char *[6];	// name, ?, ?, email, ?, message
        
	  if (!ParseFE(szMessage, &szFields, 6))
          {
    	    char *buf;
	    
            gLog.Warn("%sInvalid offline web panel system message:\n%s\n", L_WARNxSTR,
		      packet.print(buf));
            delete [] buf;
            delete [] szFields;
            break;
          }

	  // translating string with Translation Table
          gTranslator.ServerToClient(szFields[0]);  // name
          gTranslator.ServerToClient(szFields[3]);  // email
          gTranslator.ServerToClient(szFields[5]);  // message

          gLog.Info("%sFrom %s (%s).\n", L_SBLANKxSTR, szFields[0], szFields[3]);
          CEventWebPanel *e = new CEventWebPanel(szFields[0], szFields[3], szFields[5],
                                                 ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, 0);
	  delete [] szFields;	
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_EMAILxPAGER:
        {
          gLog.Info("%sEmail pager offline message.\n", L_SBLANKxSTR);

          char **szFields = new char *[6];	// name, ?, ?, email, ?, message
        
	  if (!ParseFE(szMessage, &szFields, 6))
          {
            char *buf;

            gLog.Warn("%sInvalid offline email pager system message:\n%s\n", L_WARNxSTR,
		      packet.print(buf));
            delete [] buf;
            delete [] szFields;
            break;
          }

	  // translating string with Translation Table
          gTranslator.ServerToClient(szFields[0]);  // name
          gTranslator.ServerToClient(szFields[3]);  // email
          gTranslator.ServerToClient(szFields[5]);  // message

          gLog.Info("%sFrom %s (%s).\n", L_SBLANKxSTR, szFields[0], szFields[3]);
          CEventEmailPager *e = new CEventEmailPager(szFields[0], szFields[3], szFields[5],
                                                     ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, 0);
	  delete [] szFields;	
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_CONTACTxLIST:
        {
	  CEventContactList *e = CEventContactList::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, nMask);
          if (e == NULL)
          {
            char *buf;
 
            gLog.Warn("%sInvalid offline Contact List message:\n%s\n", L_WARNxSTR,
		      packet.print(buf));
            delete [] buf;
            break;
          }
	  szType = strdup("Contacts");
	  nTypeEvent = ON_EVENT_MSG;
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_SMS:
	{
	  CEventSms *e = CEventSms::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, nTimeSent, nMask);
	  if (e == NULL)
          {
	    char *buf;

	    gLog.Warn("%sInvalid SMS message:\n%s\n", L_WARNxSTR, packet.print(buf));
	    delete [] buf;
	    break;
	  }
	  eEvent = e;
	  break;
	}
	default:
	{
	  char *szFE, *buf;

	  while ((szFE = strchr(szMessage, 0xFE)) != NULL) *szFE = '\n';
	  
          gLog.Unknown("%sUnknown offline system message (0x%04x):\n%s\n", L_UNKNOWNxSTR,
                       nTypeMsg, packet.print(buf));
          delete [] buf;
          CEventUnknownSysMsg *e = new CEventUnknownSysMsg(nTypeMsg, ICQ_CMDxRCV_SYSxMSGxOFFLINE,
                                                           nUin, szMessage, nTimeSent, 0);
          
	  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
          AddUserEvent(o, e);
          gUserManager.DropOwner();
	}
      }

      if (eEvent)
        switch(nTypeMsg)
        {
	  case ICQ_CMDxSUB_MSG:
	  case ICQ_CMDxSUB_URL:
	  case ICQ_CMDxSUB_CONTACTxLIST:
	  {
    	    // Lock the user to add the message to their queue
    	    ICQUser* u = gUserManager.FetchUser(nUin, LOCK_W);
    	    if (u == NULL)
    	    {
    	      if (Ignore(IGNORE_NEWUSERS))
    	      {
        	gLog.Info("%sOffline %s from new user (%ld), ignoring.\n", L_SBLANKxSTR, szType, nUin);
		if (szType) free(szType);
        	RejectEvent(nUin, eEvent);
        	break;
    	      }
    	      gLog.Info("%sOffline %s from new user (%ld).\n", L_SBLANKxSTR, szType, nUin);
    	      AddUserToList(nUin);
    	      u = gUserManager.FetchUser(nUin, LOCK_W);
    	    }
    	    else
    	      gLog.Info("%sOffline %s through server from %s (%ld).\n", L_SBLANKxSTR,
                        szType, u->GetAlias(), nUin);

            if (szType) free(szType);
    	    if (AddUserEvent(u, eEvent))
    	      m_xOnEventManager.Do(nTypeEvent, u);
    	    gUserManager.DropUser(u);
	    break;
	  }
	  case ICQ_CMDxSUB_AUTHxREQUEST:
	  case ICQ_CMDxSUB_AUTHxREFUSED:
	  case ICQ_CMDxSUB_AUTHxGRANTED:
	  case ICQ_CMDxSUB_MSGxSERVER:
	  case ICQ_CMDxSUB_ADDEDxTOxLIST:
	  case ICQ_CMDxSUB_WEBxPANEL:
	  case ICQ_CMDxSUB_EMAILxPAGER:
	  {
	    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
            if (AddUserEvent(o, eEvent))
	    {
              gUserManager.DropOwner();
              eEvent->AddToHistory(NULL, D_RECEIVER);
              m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
	    }
	    else
	      gUserManager.DropOwner();
            break;
	  }
	  case ICQ_CMDxSUB_SMS:
	  {
	    CEventSms *eSms = (CEventSms *)eEvent; 
	    unsigned long nUinSms = FindUinByCellular(eSms->Number());
	    
	    if (nUinSms != 0)
	    {
	      ICQUser* u = gUserManager.FetchUser(nUinSms, LOCK_W);
	      gLog.Info("%sOffline SMS from %s - %s (%ld).\n", L_SBLANKxSTR, eSms->Number(), u->GetAlias(), nUin);
	      if (AddUserEvent(u, eEvent))
	        m_xOnEventManager.Do(ON_EVENT_SMS, u);
	      gUserManager.DropUser(u);
	    }
	    else
	    {  
	      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
	      gLog.Info("%sOffline SMS from %s.\n", L_BLANKxSTR, eSms->Number());
	      if (AddUserEvent(o, eEvent))
	      {
	        gUserManager.DropOwner();
	        eEvent->AddToHistory(NULL, D_RECEIVER);
	        m_xOnEventManager.Do(ON_EVENT_SMS, NULL);
	      }
	      else
	        gUserManager.DropOwner();
	    }
	    break;
	  }
	}
	
      delete [] szMessage;
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
			unsigned short s = o->StatusFull();
    			gUserManager.DropOwner();
			// Set status to ensure the status flags are set
			icqSetStatus(s);
    			PushPluginEvent(e2);
    			DoneEvent(e, EVENT_SUCCESS);
    		}
    		else /* META_FAILURE */
    			gLog.Info("%sSecurity info not updated.\n", L_SRVxSTR);
      } else if (nSubtype == ICQ_CMDxMETA_GENERALxINFOxRSP) {
		if (nResult == META_SUCCESS) {
    			gLog.Info("%sGeneral info updated.\n", L_SRVxSTR);
 	   		ICQEvent *e = DoneExtendedServerEvent(nSubSequence, EVENT_SUCCESS);
    			ICQEvent *e2 = new ICQEvent(e);
    			e2->m_nCommand = ICQ_CMDxSND_META;
    			e2->m_nSubCommand = ICQ_CMDxMETA_GENERALxINFOxSET;
			CPU_Meta_SetGeneralInfo *p = (CPU_Meta_SetGeneralInfo *)e->m_pPacket;
    			ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
			
    			o->SetEnableSave(false);
			
			o->SetAlias( p->m_szAlias );
			o->SetFirstName( p->m_szFirstName );
			o->SetLastName( p->m_szLastName );
			o->SetEmailPrimary( p->m_szEmailPrimary );
			o->SetCity( p->m_szCity );
			o->SetState( p->m_szState );
			o->SetPhoneNumber( p->m_szPhoneNumber );
			o->SetFaxNumber( p->m_szFaxNumber );
			o->SetAddress( p->m_szAddress );
			o->SetCellularNumber( p->m_szCellularNumber );
			o->SetZipCode( p->m_szZipCode );
			o->SetCountryCode( p->m_nCountryCode );
			o->SetTimezone( p->m_nTimezone );
			o->SetHideEmail( p->m_nHideEmail ); // 0 = no, 1 = yes

			// translating string with Translation Table
			gTranslator.ServerToClient(o->GetAlias());
			gTranslator.ServerToClient(o->GetFirstName());
			gTranslator.ServerToClient(o->GetLastName());
			gTranslator.ServerToClient(o->GetEmailPrimary());
			gTranslator.ServerToClient(o->GetCity());
			gTranslator.ServerToClient(o->GetState());
			gTranslator.ServerToClient(o->GetPhoneNumber());
			gTranslator.ServerToClient(o->GetFaxNumber());
			gTranslator.ServerToClient(o->GetAddress());
		        gTranslator.ServerToClient(o->GetCellularNumber());
			gTranslator.ServerToClient(o->GetZipCode());

			// save the user infomation
			o->SetEnableSave(true);
			o->SaveGeneralInfo();
    			
			gUserManager.DropOwner();
    			PushPluginEvent(e2);
    			DoneEvent(e, EVENT_SUCCESS);
    		}
    		else /* META_FAILURE */
    			gLog.Info("%sGeneral info not updated.\n", L_SRVxSTR);    	
      } else if (nSubtype == ICQ_CMDxMETA_EMAILxINFOxRSP) {
		if (nResult == META_SUCCESS) {
    			gLog.Info("%sAdditional E-Mail info updated.\n", L_SRVxSTR);
 	   		ICQEvent *e = DoneExtendedServerEvent(nSubSequence, EVENT_SUCCESS);
    			ICQEvent *e2 = new ICQEvent(e);
    			e2->m_nCommand = ICQ_CMDxSND_META;
    			e2->m_nSubCommand = ICQ_CMDxMETA_EMAILxINFOxSET;
			CPU_Meta_SetEmailInfo *p = (CPU_Meta_SetEmailInfo *)e->m_pPacket;
    			ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
			
    			o->SetEnableSave(false);
			o->SetEmailSecondary( p->m_szEmailSecondary );
			o->SetEmailOld( p->m_szEmailOld );

			// translating string with Translation Table
			gTranslator.ServerToClient(o->GetEmailSecondary());
			gTranslator.ServerToClient(o->GetEmailOld());

			// save the user infomation
			o->SetEnableSave(true);
			o->SaveGeneralInfo();
    			
			gUserManager.DropOwner();
    			PushPluginEvent(e2);
    			DoneEvent(e, EVENT_SUCCESS);
    		}
    		else /* META_FAILURE */
    			gLog.Info("%sAdditional E-Mail info not updated.\n", L_SRVxSTR);    	
      } else if (nSubtype == ICQ_CMDxMETA_MORExINFOxRSP) {
		if (nResult == META_SUCCESS) {
    			gLog.Info("%sMore info updated.\n", L_SRVxSTR);
 	   		ICQEvent *e = DoneExtendedServerEvent(nSubSequence, EVENT_SUCCESS);
    			ICQEvent *e2 = new ICQEvent(e);
    			e2->m_nCommand = ICQ_CMDxSND_META;
    			e2->m_nSubCommand = ICQ_CMDxMETA_MORExINFOxSET;
			CPU_Meta_SetMoreInfo *p = (CPU_Meta_SetMoreInfo *)e->m_pPacket;
    			ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
			
			o->SetEnableSave(false);
			
			o->SetAge( p->m_nAge );
			o->SetGender( p->m_nGender );
			o->SetHomepage( p->m_szHomepage );
			o->SetBirthYear( p->m_nBirthYear );
			o->SetBirthMonth( p->m_nBirthMonth );
			o->SetBirthDay ( p->m_nBirthDay );
			o->SetLanguage1( p->m_nLanguage1 );
			o->SetLanguage2( p->m_nLanguage2 );
			o->SetLanguage3( p->m_nLanguage3 );

			// translating string with Translation Table
			gTranslator.ServerToClient(o->GetHomepage());

			// save the user infomation
			o->SetEnableSave(true);
			o->SaveMoreInfo();

			gUserManager.DropOwner();
    			PushPluginEvent(e2);
    			DoneEvent(e, EVENT_SUCCESS);
    		}
    		else /* META_FAILURE */
    			gLog.Info("%sMore info not updated.\n", L_SRVxSTR);    	
      } else if (nSubtype == ICQ_CMDxMETA_WORKxINFOxRSP) {
		if (nResult == META_SUCCESS) {
    			gLog.Info("%sWork info updated.\n", L_SRVxSTR);
 	   		ICQEvent *e = DoneExtendedServerEvent(nSubSequence, EVENT_SUCCESS);
    			ICQEvent *e2 = new ICQEvent(e);
    			e2->m_nCommand = ICQ_CMDxSND_META;
    			e2->m_nSubCommand = ICQ_CMDxMETA_WORKxINFOxSET;
			CPU_Meta_SetWorkInfo *p = (CPU_Meta_SetWorkInfo *)e->m_pPacket;
    			ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
			
			o->SetEnableSave(false);
			
			o->SetCompanyCity( p->m_szCity );
			o->SetCompanyState( p->m_szState );
			o->SetCompanyPhoneNumber( p->m_szPhoneNumber );
			o->SetCompanyFaxNumber( p->m_szFaxNumber );
			o->SetCompanyAddress( p->m_szAddress );
			o->SetCompanyZip( p->m_szZip );
			o->SetCompanyCountry( p->m_nCompanyCountry );
			o->SetCompanyName( p->m_szName );
			o->SetCompanyDepartment( p->m_szDepartment );
			o->SetCompanyPosition( p->m_szPosition );
			o->SetCompanyHomepage( p->m_szHomepage );

			// translating string with Translation Table
			gTranslator.ServerToClient(o->GetCompanyCity());
			gTranslator.ServerToClient(o->GetCompanyState());
			gTranslator.ServerToClient(o->GetCompanyPhoneNumber());
			gTranslator.ServerToClient(o->GetCompanyFaxNumber());
			gTranslator.ServerToClient(o->GetCompanyAddress());
			gTranslator.ServerToClient(o->GetCompanyZip());
			gTranslator.ServerToClient(o->GetCompanyName());
			gTranslator.ServerToClient(o->GetCompanyDepartment());
			gTranslator.ServerToClient(o->GetCompanyPosition());
			gTranslator.ServerToClient(o->GetCompanyHomepage());

			// save the user infomation
			o->SetEnableSave(true);
			o->SaveWorkInfo();

			gUserManager.DropOwner();
    			PushPluginEvent(e2);
    			DoneEvent(e, EVENT_SUCCESS);
    		}
    		else /* META_FAILURE */
    			gLog.Info("%sWork info not updated.\n", L_SRVxSTR);    	
      } else if (nSubtype == ICQ_CMDxMETA_ABOUTxRSP) {
		if (nResult == META_SUCCESS) {
    			gLog.Info("%sAbout updated.\n", L_SRVxSTR);
 	   		ICQEvent *e = DoneExtendedServerEvent(nSubSequence, EVENT_SUCCESS);
    			ICQEvent *e2 = new ICQEvent(e);
    			e2->m_nCommand = ICQ_CMDxSND_META;
    			e2->m_nSubCommand = ICQ_CMDxMETA_ABOUTxSET;
			CPU_Meta_SetAbout *p = (CPU_Meta_SetAbout *)e->m_pPacket;
    			ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
			
			char* msg = gTranslator.RNToN(p->m_szAbout);

			o->SetEnableSave(false);
			
			o->SetAbout( msg );
			delete [] msg;

			// translating string with Translation Table
			gTranslator.ServerToClient(o->GetAbout());

			// save the user infomation
			o->SetEnableSave(true);
			o->SaveAboutInfo();

			gUserManager.DropOwner();
    			PushPluginEvent(e2);
    			DoneEvent(e, EVENT_SUCCESS);
    		}
    		else /* META_FAILURE */
    			gLog.Info("%sAbout not updated.\n", L_SRVxSTR);    	
      } else if (nSubtype == ICQ_CMDxMETA_SENDxSMSxRSP) {
	      if (nResult == META_SUCCESS) {
    		char *szTag, *szXml, *szSmsResponse;
		
    		msg.UnpackUnsignedShortBE();
    		msg.UnpackUnsignedShortBE();
    		msg.UnpackUnsignedShortBE();

		szTag = msg.UnpackStringBE();
		szXml = msg.UnpackStringBE();
		
		szSmsResponse = GetXmlTag(szXml, "sms_response");
		if (szSmsResponse != NULL) {
		  char *szDeliverable;
		    
		  szDeliverable = GetXmlTag(szSmsResponse, "deliverable");
		  if (szDeliverable != NULL) {
		    if (!strcmp(szDeliverable, "Yes")) {
		      gLog.Info("%sSMS delivered.\n", L_SRVxSTR);
 	   	      ICQEvent *e = DoneExtendedServerEvent(nSubSequence, EVENT_ACKED);
    		      if (e != NULL) {
		        DoneEvent(e, EVENT_ACKED);
		        ProcessDoneEvent(e);
		      }
		    } else if (!strcmp(szDeliverable, "No")) {
		      char *szId, *szParam;
		      
		      szId = GetXmlTag(szSmsResponse, "id");
		      szParam = GetXmlTag(szSmsResponse, "param");
		      gLog.Warn("%sSMS not delivered, error #%s, param: %s\n", L_SRVxSTR, szId, szParam);
		      if (szId != NULL) free(szId);
		      if (szParam != NULL) free(szParam);
 	   	      ICQEvent *e = DoneExtendedServerEvent(nSubSequence, EVENT_FAILED);
    		      if (e != NULL) {
		        DoneEvent(e, EVENT_FAILED);
		        ProcessDoneEvent(e);
		      }
		    } else if (!strcmp(szDeliverable, "SMTP")) {
		      char *szFrom, *szTo, *szSubject;
		      
		      gLog.Info("%sSending SMS via SMTP not supported yet.\n", L_SRVxSTR);
		      
		      szFrom = GetXmlTag(szSmsResponse, "from");
		      if (szFrom != NULL) {
		        gLog.Info("%sFrom: %s\n", L_SRVxSTR, szFrom);
			free(szFrom);
		      }
		      szTo = GetXmlTag(szSmsResponse, "to");
		      if (szTo != NULL) {
		        gLog.Info("%sTo: %s\n", L_SRVxSTR, szTo);
			free(szTo);
		      }
		      szSubject = GetXmlTag(szSmsResponse, "subject");
		      if (szSubject != NULL) {
		        gLog.Info("%sSubject: %s\n", L_SRVxSTR, szSubject);
			free(szSubject);
		      }
		      ICQEvent *e = DoneExtendedServerEvent(nSubSequence, EVENT_FAILED);
    		      if (e != NULL) {
		        DoneEvent(e, EVENT_FAILED);
		        ProcessDoneEvent(e);
		      }
		    }
		    free(szDeliverable);
		  }
		  free(szSmsResponse);
		}
    		
		delete [] szTag;
		delete [] szXml;

    	      } else { /* META_FAILURE */
    		gLog.Info("%sSMS through server failed.\n", L_SRVxSTR);
		ICQEvent *e = DoneExtendedServerEvent(nSubSequence, EVENT_FAILED);
    		if (e != NULL) {
		  DoneEvent(e, EVENT_FAILED);
		  ProcessDoneEvent(e);
		}
	      }
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
        	
        s->m_szAlias = strdup(msg.UnpackString(szTemp, sizeof(szTemp)));
      	s->m_szFirstName = strdup(msg.UnpackString(szTemp, sizeof(szTemp)));
      	s->m_szLastName = strdup(msg.UnpackString(szTemp, sizeof(szTemp)));
      	s->m_szEmail = strdup(msg.UnpackString(szTemp, sizeof(szTemp)));
	s->m_nAuth = msg.UnpackChar(); // authorization required
	s->m_nStatus = msg.UnpackChar();
	msg.UnpackChar(); // unknown
	s->m_nGender = msg.UnpackChar(); // gender
	s->m_nAge = msg.UnpackChar(); // age
	msg.UnpackChar(); // unknown

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
      		nMore = msg.UnpackUnsignedLong();
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
        	gLog.Warn("%sFailed to update user info: %x.\n", L_WARNxSTR, nResult);
        	e = DoneExtendedServerEvent(nSubSequence, EVENT_FAILED);
		if (e != NULL) {
		  DoneEvent(e, EVENT_FAILED);
		  ProcessDoneEvent(e);
	        }
        	e = NULL;
		break;
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
	    u->SetEnableSave(false);
	    u->SetAlias( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetFirstName( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetLastName( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetEmailPrimary( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetCity( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetState( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetPhoneNumber( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetFaxNumber( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetAddress( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetCellularNumber( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetZipCode( tmp = msg.UnpackString() );
            delete[] tmp;
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
	    u->SetHomepage( tmp = msg.UnpackString() );
            delete[] tmp;
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
		    u->SetEmailSecondary( tmp = msg.UnpackString() );
                    delete[] tmp;
		    gTranslator.ServerToClient(u->GetEmailSecondary());
		}
		if(i == 1) {
		    u->SetEmailOld( tmp = msg.UnpackString() );
                    delete[] tmp;
		    gTranslator.ServerToClient(u->GetEmailOld());
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
	    u->SetCompanyCity( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetCompanyState( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetCompanyPhoneNumber( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetCompanyFaxNumber( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetCompanyAddress( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetCompanyZip( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetCompanyCountry( msg.UnpackUnsignedShort() );
	    u->SetCompanyName( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetCompanyDepartment( tmp = msg.UnpackString() );
            delete[] tmp;
	    u->SetCompanyPosition( tmp = msg.UnpackString() );
            delete[] tmp;
	    msg.UnpackUnsignedShort(); // unknown
	    u->SetCompanyHomepage( tmp = msg.UnpackString() );
            delete[] tmp;

	    // translating string with Translation Table
	    gTranslator.ServerToClient(u->GetCompanyCity());
	    gTranslator.ServerToClient(u->GetCompanyState());
	    gTranslator.ServerToClient(u->GetCompanyPhoneNumber());
	    gTranslator.ServerToClient(u->GetCompanyFaxNumber());
	    gTranslator.ServerToClient(u->GetCompanyAddress());
	    gTranslator.ServerToClient(u->GetCompanyZip());
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

	    u->SetEnableSave(false);
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
	    if (e != NULL) {
		DoneEvent(e, EVENT_SUCCESS);
		ProcessDoneEvent(e);
	    } else {
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

//--------ProcessNewUINFam-----------------------------------------------------
void CICQDaemon::ProcessNewUINFam(CBuffer &packet, unsigned short nSubtype)
{
  if (nSubtype != ICQ_SNACxNEW_UIN)
  {
    char *buf;
    gLog.Unknown("%sUnknow New UIN Family Subtype: %04hx\n%s\n", L_UNKNOWNxSTR,
    		nSubtype, packet.print(buf));
    delete [] buf;
    return;
  }

  packet.UnpackUnsignedShort(); // flags
  packet.UnpackUnsignedLong();  // id

  // 46 bytes of shit
  for (int x = 0; x < 11; x++)
    packet.UnpackUnsignedLong();
  packet.UnpackUnsignedShort();

  unsigned long nNewUin = packet.UnpackUnsignedLong();

  if (gUserManager.OwnerUin() != 0)
  {
    gLog.Warn("%sReceived new uin (%ld) when already have a uin (%ld).\n", L_WARNxSTR,
    	nNewUin, gUserManager.OwnerUin());
    return;
  }

  gLog.Info("%sReceived new uin: %ld\n", L_SRVxSTR, nNewUin);
  gUserManager.SetOwnerUin(nNewUin);

  // Reconnect now
  int nSD = m_nTCPSrvSocketDesc;
  m_nTCPSrvSocketDesc = -1;
  gSocketManager.CloseSocket(nSD);

  icqLogon(ICQ_STATUS_ONLINE);
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

  case ICQ_SNACxFAM_LIST:
    ProcessListFam(packet, nSubtype);
    break;

  case ICQ_SNACxFAM_BOS:
    ProcessBOSFam(packet, nSubtype);
    break;

  case ICQ_SNACxFAM_VARIOUS:
    ProcessVariousFam(packet, nSubtype);
    break;

  case ICQ_SNACxFAM_NEWUIN:
    ProcessNewUINFam(packet, nSubtype);
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
    PushPluginSignal(new CICQSignal(SIGNAL_LOGOFF, LOGOFF_RATE, 0));
    break;

  case 0x05:
    gLog.Error("%sInvalid UIN and password combination.\n", L_ERRORxSTR);
    m_eStatus = STATUS_OFFLINE_FORCED;
    m_bLoggingOn = false;
    PushPluginSignal(new CICQSignal(SIGNAL_LOGOFF, LOGOFF_PASSWORD, 0));
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
