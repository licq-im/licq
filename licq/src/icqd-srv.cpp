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
#include <errno.h>
#include <assert.h>
#include <ctype.h>

#include <algorithm>

// Localization
#include "gettext.h"

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
#include "licq_countrycodes.h"
#include "licq_protoplugind.h"

void CICQDaemon::ProtoAddUser(const char *_szId, unsigned long _nPPID,
                              bool _bAuthRequired)
{
  if (_nPPID == LICQ_PPID)
    icqAddUser(_szId, _bAuthRequired);
  else
    PushProtoSignal(new CAddUserSignal(_szId, _bAuthRequired), _nPPID);
}

//-----icqAddUser----------------------------------------------------------
void CICQDaemon::icqAddUser(const char *_szId, bool _bAuthRequired)
{
  CSrvPacketTcp *p = new CPU_GenericUinList(_szId, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
  gLog.Info(tr("%sAlerting server to new user (#%hu)...\n"), L_SRVxSTR,
             p->Sequence());
  SendExpectEvent_Server(_szId, LICQ_PPID, p, NULL);

  // Server side list add, and update of group
  if (UseServerContactList())
  {
    icqAddUserServer(_szId, _bAuthRequired);
  }

  icqUserBasicInfo(_szId);
}

void CICQDaemon::icqAddUser(unsigned long _nUin, bool _bAuthRequired)
{
  char szUin[24];
  sprintf(szUin, "%lu", _nUin);

  icqAddUser(szUin, _bAuthRequired);
}


//-----icqAddUserServer--------------------------------------------------------
void CICQDaemon::icqAddUserServer(const char *_szId, bool _bAuthRequired)
{
  CSrvPacketTcp *pStart = 0;

  // Skip the authorization needed step
  // This has been reverted, it didn't work 100% of the time
  //if (_bAuthRequired)
  //  pStart = new CPU_ExportContactStart();
  //else
    pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
      ICQ_SNACxLIST_ROSTxEDITxSTART);
  SendEvent_Server(pStart);

  CPU_AddToServerList *pAdd = new CPU_AddToServerList(_szId, ICQ_ROSTxNORMAL,
    0, _bAuthRequired);
  gLog.Info(tr("%sAdding %s to server list...\n"), L_SRVxSTR, _szId);
  addToModifyUsers(pAdd->SubSequence(), _szId);
  SendExpectEvent_Server(0, pAdd, NULL);

  CSrvPacketTcp *pEnd = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
                                              ICQ_SNACxLIST_ROSTxEDITxEND);
  SendEvent_Server(pEnd);
}

void CICQDaemon::icqAddUserServer(unsigned long _nUin, bool _bAuthRequired)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", _nUin);
  szUin[12] = 0;

  icqAddUserServer(szUin, _bAuthRequired);
}

//-----CheckExport-------------------------------------------------------------
void CICQDaemon::CheckExport()
{
  // Export groups
  GroupList groups;
  GroupList *g = gUserManager.LockGroupList(LOCK_R);
  GroupIDList *gID = gUserManager.LockGroupIDList(LOCK_R);

  for (unsigned int i = 0; i < gID->size(); i++)
  {
    if ((*gID)[i] == 0)
    {
      groups.push_back((*g)[i]);
    }
  }

  gUserManager.UnlockGroupList();
  gUserManager.UnlockGroupIDList();

  if (groups.size())
    icqExportGroups(groups);

  // Just upload all of the users now
  UserStringList doneUsers;
  UserStringList users;
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_R)
  {
    // If they aren't a current server user and not in the ingore list,
    // let's import them!
    if (pUser->GetSID() == 0 && !pUser->IgnoreList())
    {
      UserStringList::const_iterator p = std::find(doneUsers.begin(), doneUsers.end(),
        pUser->IdString());

      if (p == doneUsers.end())
      {
        // Keep track of who has been done
        users.push_back(strdup(pUser->IdString()));
        doneUsers.push_back(strdup(pUser->IdString()));
      }
    }
  }
  FOR_EACH_PROTO_USER_END

  if (users.size())
  {
    icqExportUsers(users, ICQ_ROSTxNORMAL);
    icqUpdateServerGroups();
  }
  
  UserStringList::iterator it;
  for (it = doneUsers.begin(); it != doneUsers.end(); ++it)
    free(*it);

  // Export visible/invisible/ignore list
  UserStringList visibleUsers, invisibleUsers, ignoredUsers;
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_R)
  {
    if (pUser->IgnoreList() && pUser->GetSID() == 0)
    {
      ignoredUsers.push_back(strdup(pUser->IdString()));
    }
    else
    {
      if (pUser->InvisibleList() && pUser->GetInvisibleSID() == 0)
      {
        invisibleUsers.push_back(strdup(pUser->IdString()));
      }

      if (pUser->VisibleList() && pUser->GetVisibleSID() == 0)
      {
        visibleUsers.push_back(strdup(pUser->IdString()));
      }
    }
  }
  FOR_EACH_PROTO_USER_END

  if (visibleUsers.size())
    icqExportUsers(visibleUsers, ICQ_ROSTxVISIBLE);

  if (invisibleUsers.size())
    icqExportUsers(invisibleUsers, ICQ_ROSTxINVISIBLE);

  if (ignoredUsers.size())
    icqExportUsers(ignoredUsers, ICQ_ROSTxIGNORE);
}

//-----icqExportUsers----------------------------------------------------------
void CICQDaemon::icqExportUsers(UserStringList &users, unsigned short _nType)
{
  if (!UseServerContactList())  return;

  CSrvPacketTcp *pStart = new CPU_ExportContactStart();
  SendEvent_Server(pStart);

  CSrvPacketTcp *pExport = new CPU_ExportToServerList(users, _nType);
  gLog.Info(tr("%sExporting users to server contact list...\n"), L_SRVxSTR);
  addToModifyUsers(pExport->SubSequence(), "");
  SendEvent_Server(pExport);

  CSrvPacketTcp *pEnd = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
    ICQ_SNACxLIST_ROSTxEDITxEND);
  SendEvent_Server(pEnd);
}

//-----icqUpdateServerGroups----------------------------------------------------
void CICQDaemon::icqUpdateServerGroups()
{
  if (!UseServerContactList())  return;
  CSrvPacketTcp *pReply;
  
  pReply = new CPU_UpdateToServerList("", ICQ_ROSTxGROUP, 0);
  addToModifyUsers(pReply->SubSequence(), "");
  gLog.Info(tr("%sUpdating top level group.\n"), L_SRVxSTR);
  SendExpectEvent_Server(0, pReply, NULL);
 
  GroupList *g = gUserManager.LockGroupList(LOCK_R);
  GroupIDList *gID = gUserManager.LockGroupIDList(LOCK_R);

  for (unsigned int i = 0; i < gID->size(); i++)
  {
    if ((*gID)[i])
    {
      pReply = new CPU_UpdateToServerList((*g)[i], ICQ_ROSTxGROUP,
        (*gID)[i]);
      gLog.Info(tr("%sUpdating group %s.\n"), L_SRVxSTR, (*g)[i]);   
      addToModifyUsers(pReply->SubSequence(), "");
      SendExpectEvent_Server(0, pReply, NULL);
    }
  }

  gUserManager.UnlockGroupList();
  gUserManager.UnlockGroupIDList();       
}

//-----icqAddGroup--------------------------------------------------------------
void CICQDaemon::icqAddGroup(const char *_szName)
{
  if (!UseServerContactList())  return;

  CSrvPacketTcp *pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
    ICQ_SNACxLIST_ROSTxEDITxSTART);
  SendEvent_Server(pStart);

  CPU_AddToServerList *pAdd = new CPU_AddToServerList(_szName, ICQ_ROSTxGROUP);
  int nGSID = pAdd->GetGSID();
  gLog.Info(tr("%sAdding group %s (%d) to server list ...\n"), L_SRVxSTR, _szName, nGSID);
  addToModifyUsers(pAdd->SubSequence(), _szName);
  SendExpectEvent_Server(0L, pAdd, NULL);
}

//-----icqChangeGroup-----------------------------------------------------------
void CICQDaemon::icqChangeGroup(unsigned long _nUin, unsigned short _nNewGroup,
                                unsigned short _nOldGSID, unsigned short _nNewType,
                                unsigned short _nOldType)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", _nUin);
  szUin[12] = '\0';

  icqChangeGroup(szUin, LICQ_PPID, _nNewGroup, _nOldGSID, _nNewType, _nOldType);
}

void CICQDaemon::icqChangeGroup(const char *_szId, unsigned long _nPPID,
                                unsigned short _nNewGroup, unsigned short _nOldGSID,
                                unsigned short _nNewType, unsigned short _nOldType)
{
  if (!UseServerContactList())  return;

  // Get their old SID
  ICQUser *u = gUserManager.FetchUser(_szId, _nPPID, LOCK_R);
  int nSID = u->GetSID();
  gUserManager.DropUser(u);

  gLog.Info(tr("%sChanging group on server list for %s ...\n"), L_SRVxSTR, _szId);

  // Start transaction
  CSrvPacketTcp *pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
                                                ICQ_SNACxLIST_ROSTxEDITxSTART);
  SendEvent_Server(pStart);

  // Delete the user
  CSrvPacketTcp *pRemove = new CPU_RemoveFromServerList(_szId, _nOldGSID,
                                                        nSID, _nOldType);
  addToModifyUsers(pRemove->SubSequence(), _szId);
  SendExpectEvent_Server(0, pRemove, NULL);

  // Add the user, with the new group
  CPU_AddToServerList *pAdd = new CPU_AddToServerList(_szId, _nNewType,
                                                      _nNewGroup);
  addToModifyUsers(pAdd->SubSequence(), _szId);
  SendExpectEvent_Server(0, pAdd, NULL);

}

//-----icqExportGroups----------------------------------------------------------
void CICQDaemon::icqExportGroups(GroupList &groups)
{
  if (!UseServerContactList()) return;

  CSrvPacketTcp *pStart = new CPU_ExportContactStart();
  SendEvent_Server(pStart);

  CSrvPacketTcp *pExport = new CPU_ExportGroupsToServerList(groups);
  gLog.Info(tr("%sExporting groups to server contact list...\n"), L_SRVxSTR);
  // We lump all the groups into one packet, so the success/failure result will
  // be based on all of them. So a generic name should be fine, but then we need
  // to add a flag to signify if it is a real user/group or a generic one.
  addToModifyUsers(pExport->SubSequence(), "");
  SendExpectEvent_Server(0, pExport, NULL);
  
  CSrvPacketTcp *pEnd = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
    ICQ_SNACxLIST_ROSTxEDITxEND);
  SendEvent_Server(pEnd);
}

//-----icqCreatePDINFO---------------------------------------------------------
void CICQDaemon::icqCreatePDINFO()
{
  CSrvPacketTcp *pPDInfo = new CPU_AddPDINFOToServerList();
  gLog.Info(tr("%sAdding privacy information to server side list...\n"),
    L_SRVxSTR);
  SendEvent_Server(pPDInfo);
}

//-----icqRemoveUser-------------------------------------------------------
void CICQDaemon::ProtoRemoveUser(const char *_szId, unsigned long _nPPID)
{
  if (_nPPID == LICQ_PPID)
    icqRemoveUser(_szId);
  else
    PushProtoSignal(new CRemoveUserSignal(_szId), _nPPID);
}

void CICQDaemon::icqRemoveUser(const char *_szId)
{
  // Remove from the SSList and update groups
  if (UseServerContactList())
  {
    CSrvPacketTcp *pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
      ICQ_SNACxLIST_ROSTxEDITxSTART);
    SendEvent_Server(pStart);

    ICQUser *u = gUserManager.FetchUser(_szId, LICQ_PPID, LOCK_W);
    // When we remove a user, we remove them from all parts of the list:
    // Visible, Invisible and Ignore lists as well.
    unsigned short nGSID = u->GetGSID();
    unsigned short nSID = u->GetSID();
    unsigned short nVisibleSID = u->GetVisibleSID();
    unsigned short nInvisibleSID = u->GetInvisibleSID();
    bool bIgnored = u->IgnoreList();
    u->SetGSID(0);
    u->SetVisibleSID(0);
    u->SetInvisibleSID(0);
    u->SetVisibleList(false);
    u->SetInvisibleList(false);
    u->SaveLicqInfo();
    gUserManager.DropUser(u);

    CSrvPacketTcp *pRemove = 0;
    if (bIgnored)
      pRemove = new CPU_RemoveFromServerList(_szId, nGSID, nSID, ICQ_ROSTxIGNORE);
    else
      pRemove = new CPU_RemoveFromServerList(_szId, nGSID, nSID, ICQ_ROSTxNORMAL);
    addToModifyUsers(pRemove->SubSequence(), _szId);
    SendExpectEvent_Server(0, pRemove, NULL);

    if (nVisibleSID)
    {
      CSrvPacketTcp *pVisRemove = new CPU_RemoveFromServerList(_szId, 0,
        nVisibleSID, ICQ_ROSTxVISIBLE);
      /* XXX Check if we get an ack response on this packet
      pthread_mutex_lock(&mutex_modifyserverusers);
      m_lszModifyServerUsers[pVisRemove->SubSequence()] = _szId;
      pthread_mutex_unlock(&mutex_modifyserverusers);
      */
      SendEvent_Server(pVisRemove);
    }

    if (nInvisibleSID)
    {
      CSrvPacketTcp *pInvisRemove = new CPU_RemoveFromServerList(_szId, 0,
          nInvisibleSID, ICQ_ROSTxINVISIBLE);
      /* XXX Check if we get an ack response on this packet
      pthread_mutex_lock(&mutex_modifyserverusers);
      m_lszModifyServerUsers[pInvisRemove->SubSequence()] = _szId;
      pthread_mutex_unlock(&mutex_modifyserverusers);
      */
      SendEvent_Server(pInvisRemove);
    }
  }

  // Tell server they are no longer with us.
  CSrvPacketTcp *p = new CPU_GenericUinList(_szId, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_REMOVExFROMxLIST);
  gLog.Info(tr("%sAlerting server to remove user (#%hu)...\n"), L_SRVxSTR,
            p->Sequence());
  SendExpectEvent_Server(_szId, LICQ_PPID, p, NULL);
}

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
    snprintf(szUin, 12, "%lu", _nUin);
    szUin[12] = '\0';
    u->SetGSID(0);
    gUserManager.DropUser(u);

    CSrvPacketTcp *pRemove = new CPU_RemoveFromServerList(szUin, nGSID, nSID, ICQ_ROSTxNORMAL);
    addToModifyUsers(pRemove->SubSequence(), szUin);
    SendExpectEvent_Server(0, pRemove, NULL);
  }

  // Tell server they are no longer with us.
  CSrvPacketTcp *p = new CPU_GenericUinList(_nUin, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_REMOVExFROMxLIST);
  gLog.Info(tr("%sAlerting server to remove user (#%hu)...\n"), L_SRVxSTR,
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

  CSrvPacketTcp *pRemove = new CPU_RemoveFromServerList(_szName,
    gUserManager.GetIDFromGroup(_szName), 0, ICQ_ROSTxGROUP);
  gLog.Info(tr("%sRemoving group from server side list (%s)...\n"), L_SRVxSTR, _szName);
  addToModifyUsers(pRemove->SubSequence(), _szName);
  SendExpectEvent_Server(0, pRemove, NULL);
}

//-----icqRenameGroup----------------------------------------------------------
void CICQDaemon::icqRenameGroup(const char *_szNewName, unsigned short _nGSID)
{
  if (!UseServerContactList() || !_nGSID || m_nTCPSrvSocketDesc == -1) return;

  CSrvPacketTcp *pUpdate = new CPU_UpdateToServerList(_szNewName,
    ICQ_ROSTxGROUP, _nGSID);
  gLog.Info(tr("%sRenaming group with id %d to %s...\n"), L_SRVxSTR, _nGSID,
    _szNewName);
  addToModifyUsers(pUpdate->SubSequence(), _szNewName);
  SendExpectEvent_Server(0, pUpdate, NULL);
}

//-----icqRenameUser------------------------------------------------------------
void CICQDaemon::icqRenameUser(unsigned long _nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", _nUin);
  szUin[12] = '\0'; 

  icqRenameUser(szUin);
}

void CICQDaemon::ProtoRenameUser(const char *_szId, unsigned long _nPPID)
{
  if (_nPPID == LICQ_PPID)
    icqRenameUser(_szId);
  else
    PushProtoSignal(new CRenameUserSignal(_szId), _nPPID);
}

void CICQDaemon::icqRenameUser(const char *_szId)
{
  if (!UseServerContactList() || m_nTCPSrvSocketDesc == -1) return;

  ICQUser *u = gUserManager.FetchUser(_szId, LICQ_PPID, LOCK_R);
  if (u == NULL) return;
  char *szNewAlias = u->GetAlias();
  gUserManager.DropUser(u);

  CSrvPacketTcp *pUpdate = new CPU_UpdateToServerList(_szId, ICQ_ROSTxNORMAL);
  gLog.Info(tr("%sRenaming %s to %s...\n"), L_SRVxSTR, _szId, szNewAlias);
  addToModifyUsers(pUpdate->SubSequence(), _szId);
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
  gLog.Info(tr("%sAlerting user they were added (#%hu)...\n"), L_SRVxSTR, p->Sequence());
  SendExpectEvent_Server(_nUin, p, NULL);
}

//-----icqFetchAutoResponseServer-----------------------------------------------
unsigned long CICQDaemon::ProtoFetchAutoResponseServer(const char *_szId, unsigned long _nPPID)
{
  unsigned long nRet = 0;

  if (_nPPID == LICQ_PPID)
    nRet = icqFetchAutoResponseServer(_szId);

  return nRet;
}

unsigned long CICQDaemon::icqFetchAutoResponseServer(const char *_szId)
{
  CPU_CommonFamily *p = 0;

  if (isalpha(_szId[0]))
    p = new CPU_AIMFetchAwayMessage(_szId);
  else
  {
    ICQUser *u = gUserManager.FetchUser(_szId, LICQ_PPID, LOCK_R);
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
    gUserManager.DropUser(u);

    p = new CPU_ThroughServer(_szId, nCmd, 0);
  }

  if (!p) return 0;

  gLog.Info(tr("%sRequesting auto response from %s (%hu).\n"), L_SRVxSTR,
        _szId, p->Sequence());

  ICQEvent *result = SendExpectEvent_Server(_szId, LICQ_PPID, p, NULL);
  if (result != NULL)
    return result->EventId();
  return 0;
}

unsigned long CICQDaemon::icqFetchAutoResponseServer(unsigned long _nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", _nUin);
  szUin[12] = '\0';

  return icqFetchAutoResponseServer(szUin);
}

//-----icqSetRandomChatGroup----------------------------------------------------
unsigned long CICQDaemon::icqSetRandomChatGroup(unsigned long _nGroup)
{
  CPU_SetRandomChatGroup *p = new CPU_SetRandomChatGroup(_nGroup);
  gLog.Info(tr("%sSetting random chat group (#%hu)...\n"), L_SRVxSTR,
            p->Sequence());

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqRandomChatSearch------------------------------------------------------
unsigned long CICQDaemon::icqRandomChatSearch(unsigned long _nGroup)
{
  CPU_RandomChatSearch *p = new CPU_RandomChatSearch(_nGroup);
  gLog.Info(tr("%sSearching for random chat user (#%hu)...\n"), L_SRVxSTR,
            p->Sequence());

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

void CICQDaemon::icqRegister(const char *_szPasswd)
{
  if (m_szRegisterPasswd)
  {
    free(m_szRegisterPasswd);
    m_szRegisterPasswd = 0;
  }
    
  m_szRegisterPasswd = strdup(_szPasswd);
  m_bRegistering = true;
  m_nRegisterThreadId = pthread_self();
//  CPU_RegisterFirst *p = new CPU_RegisterFirst();
//  gLog.Info("%sRegistering a new user (#%hu)...\n", L_SRVxSTR, p->Sequence());
// SendEvent_Server(p);
  ConnectToLoginServer();
}

//-----ICQ::icqRegisterFinish------------------------------------------------
void CICQDaemon::icqRegisterFinish()
{
  CPU_RegisterFirst *pFirst = new CPU_RegisterFirst();
  SendEvent_Server(pFirst);

  CPU_Register *p = new CPU_Register(m_szRegisterPasswd);
  gLog.Info(tr("%sRegistering a new user...\n"), L_SRVxSTR);
  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  e->thread_plugin = m_nRegisterThreadId;
}

//-----ICQ::icqVerifyRegistration
void CICQDaemon::icqVerifyRegistration()
{
  CPU_RegisterFirst *pFirst = new CPU_RegisterFirst();
  SendEvent_Server(pFirst);
    
  CPU_VerifyRegistration *pVerify = new CPU_VerifyRegistration();
  gLog.Info(tr("%sRequesting verification image...\n"), L_SRVxSTR);
  SendEvent_Server(pVerify);
  
  m_nRegisterThreadId = pthread_self();
}

//-----ICQ::icqVerify--------------------------------------------------------
void CICQDaemon::icqVerify(const char *szVerification)
{
  CPU_SendVerification *p = new CPU_SendVerification(m_szRegisterPasswd,
    szVerification);
   gLog.Info(tr("%sSending verification for registration.\n"), L_SRVxSTR);
   
   SendExpectEvent_Server(0, p, NULL);
}

//-----ICQ::icqRelogon-------------------------------------------------------
void CICQDaemon::icqRelogon()
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

  icqLogoff();
  m_eStatus = STATUS_OFFLINE_MANUAL;

  icqLogon(status);

//  m_eStatus = STATUS_OFFLINE_FORCED;
}

//-----ProtoRequestInfo------------------------------------------------------
unsigned long CICQDaemon::ProtoRequestInfo(const char *_szId, unsigned long _nPPID)
{
  unsigned long nRet = 0;
  if (_nPPID == LICQ_PPID)
    nRet = icqRequestMetaInfo(_szId);
  else
    PushProtoSignal(new CRequestInfo(_szId), _nPPID);
  
  return nRet;
}

//-----icqRequestMetaInfo----------------------------------------------------
unsigned long CICQDaemon::icqRequestMetaInfo(const char *_szId)
{
  CPU_CommonFamily *p = 0;
  bool bIsAIM = isalpha(_szId[0]);
  if (bIsAIM)
    p = new CPU_RequestInfo(_szId);
  else
    p = new CPU_Meta_RequestAllInfo(_szId);
  gLog.Info(tr("%sRequesting meta info for %s (#%hu/#%d)...\n"), L_SRVxSTR, _szId,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(_szId, LICQ_PPID, p, NULL, !bIsAIM);
  if (e != NULL)
    return e->EventId();
  return 0;
}

unsigned long CICQDaemon::icqRequestMetaInfo(unsigned long nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = 0;

  return icqRequestMetaInfo(szUin);
}

//-----icqSetStatus-------------------------------------------------------------
unsigned long CICQDaemon::ProtoSetStatus(unsigned long _nPPID,
                                         unsigned short _nNewStatus)
{
  unsigned long nRet = 0;

  if (_nPPID == LICQ_PPID)
    nRet = icqSetStatus(_nNewStatus);
  else
    PushProtoSignal(new CChangeStatusSignal(_nNewStatus), _nPPID);

  return nRet;
}

unsigned long CICQDaemon::icqSetStatus(unsigned short newStatus)
{
  if (newStatus & ICQ_STATUS_DND)
    newStatus |= ICQ_STATUS_OCCUPIED; // quick compat hack

  // icq go wants the mask set when we truly are away
  if (newStatus & ICQ_STATUS_DND || newStatus & ICQ_STATUS_OCCUPIED ||
      newStatus & ICQ_STATUS_NA)
    newStatus |= ICQ_STATUS_AWAY;

  // Set the status flags
  ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  unsigned long s = o->AddStatusFlags(newStatus);
  unsigned long pfm = o->PhoneFollowMeStatus();
  bool Invisible = o->StatusInvisible();
  bool goInvisible = (newStatus & ICQ_STATUS_FxPRIVATE);
  bool isLogon = o->StatusOffline();
  int nPDINFO = o->GetPDINFO();
  gUserManager.DropOwner(LICQ_PPID);

  if (nPDINFO == 0)
  {
    icqCreatePDINFO();
  
    o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
    nPDINFO = o->GetPDINFO();
    gUserManager.DropOwner(LICQ_PPID);
  }
  
  if (goInvisible)
  {
    CPU_SetPrivacy *priv = new CPU_SetPrivacy(ICQ_PRIVACY_ALLOW_FOLLOWING);
    SendEvent_Server(priv);
    icqSendVisibleList();
  }
  else if (Invisible && !goInvisible)
  {
    CPU_SetPrivacy *priv = new CPU_SetPrivacy(ICQ_PRIVACY_BLOCK_FOLLOWING);
    SendEvent_Server(priv);
    icqSendInvisibleList();
  }

  CSrvPacketTcp* p;
  if (isLogon)
    p = new CPU_SetLogonStatus(s);
  else
    p = new CPU_SetStatus(s);

  gLog.Info(tr("%sChanging status to %s (#%hu)...\n"), L_SRVxSTR,
            ICQUser::StatusToStatusStr(newStatus, goInvisible), p->Sequence());
  m_nDesiredStatus = s;

  SendEvent_Server(p);

  if (pfm)
  {
    p = new CPU_UpdateStatusTimestamp(PLUGIN_FOLLOWxME, pfm, s);
    SendEvent_Server(p);
  }

  return 0;
}

//-----icqSetPassword--------------------------------------------------------
unsigned long CICQDaemon::icqSetPassword(const char *szPassword)
{
  CPU_SetPassword *p = new CPU_SetPassword(szPassword);
  gLog.Info(tr("%sUpdating password (#%hu/#%d)...\n"), L_SRVxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetGeneralInfo----------------------------------------------------
unsigned long CICQDaemon::ProtoSetGeneralInfo(unsigned long nPPID,
                          const char *szAlias, const char *szFirstName,
                          const char *szLastName, const char *szEmailPrimary,
                          const char *szCity,
                          const char *szState, const char *szPhoneNumber,
                          const char *szFaxNumber, const char *szAddress,
                          const char *szCellularNumber, const char *szZipCode,
                          unsigned short nCountryCode, bool bHideEmail)
{
  unsigned long nRet = 0;
  if (nPPID == LICQ_PPID)
    nRet = icqSetGeneralInfo(szAlias, szFirstName, szLastName, szEmailPrimary,
      szCity, szState, szPhoneNumber, szFaxNumber, szAddress,
      szCellularNumber, szZipCode, nCountryCode, bHideEmail);
  else
    PushProtoSignal(new CUpdateInfoSignal(szAlias, szFirstName, szLastName, szEmailPrimary,
      szCity, szState, szPhoneNumber, szFaxNumber, szAddress,
      szCellularNumber, szZipCode), nPPID);
      
  return nRet;
}
                          
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

  gLog.Info(tr("%sUpdating general info (#%hu/#%d)...\n"), L_SRVxSTR, p->Sequence(), p->SubSequence());

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetEmailInfo---------------------------------------------------------
unsigned long CICQDaemon::icqSetEmailInfo(
                          const char *szEmailSecondary, const char *szEmailOld)
{
return 0;
  CPU_Meta_SetEmailInfo *p =
    new CPU_Meta_SetEmailInfo(szEmailSecondary, szEmailOld);

  gLog.Info(tr("%sUpdating additional E-Mail info (#%hu/#%d)...\n"), L_SRVxSTR, p->Sequence(), p->SubSequence());

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
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

  gLog.Info(tr("%sUpdating more info (#%hu/#%d)...\n"), L_SRVxSTR, p->Sequence(), p->SubSequence());

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetInterestsInfo------------------------------------------------------
unsigned long CICQDaemon::icqSetInterestsInfo(const ICQUserCategory* interests)
{
  CPU_Meta_SetInterestsInfo *p = new CPU_Meta_SetInterestsInfo(interests);
  gLog.Info("%sUpdating Interests info (#%hu/#%d)..\n", L_SRVxSTR,
    p->Sequence(), p->SubSequence());

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetOrgBackInfo--------------------------------------------------------
unsigned long CICQDaemon::icqSetOrgBackInfo(const ICQUserCategory* orgs,
                                            const ICQUserCategory* background)
{
  CPU_Meta_SetOrgBackInfo *p =
    new CPU_Meta_SetOrgBackInfo(orgs, background);
  gLog.Info("%sUpdating Organizations/Backgrounds info (#%hu/#%d)..\n",
    L_SRVxSTR, p->Sequence(), p->SubSequence());

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetWorkInfo--------------------------------------------------------
unsigned long CICQDaemon::icqSetWorkInfo(const char *_szCity, const char *_szState,
                                     const char *_szPhone,
                                     const char *_szFax, const char *_szAddress,
                                     const char *_szZip, unsigned short _nCompanyCountry,
                                     const char *_szName, const char *_szDepartment,
                                     const char *_szPosition, unsigned short _nCompanyOccupation,
                                     const char *_szHomepage)
{
  CPU_Meta_SetWorkInfo *p =
    new CPU_Meta_SetWorkInfo(_szCity, _szState, _szPhone, _szFax, _szAddress,
                             _szZip, _nCompanyCountry, _szName, _szDepartment,
                             _szPosition, _nCompanyOccupation, _szHomepage);

  gLog.Info(tr("%sUpdating work info (#%hu/#%d)...\n"), L_SRVxSTR, p->Sequence(), p->SubSequence());

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetAbout-----------------------------------------------------------
unsigned long CICQDaemon::icqSetAbout(const char *_szAbout)
{
  char *szAbout = gTranslator.NToRN(_szAbout);

  CPU_Meta_SetAbout *p = new CPU_Meta_SetAbout(szAbout);

  gLog.Info(tr("%sUpdating about (#%hu/#%d)...\n"), L_SRVxSTR, p->Sequence(), p->SubSequence());

  delete [] szAbout;

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqAuthorizeGrant-------------------------------------------------------
unsigned long CICQDaemon::ProtoAuthorizeGrant(const char *szId,
  unsigned long nPPID, const char *szMessage)
{
  unsigned long nRet = 0;

  if (nPPID == LICQ_PPID)
    nRet = icqAuthorizeGrant(szId, szMessage);
  else
    PushProtoSignal(new CGrantAuthSignal(szId, szMessage), nPPID);
  return nRet;
}

unsigned long CICQDaemon::icqAuthorizeGrant(unsigned long nUin, const char *szMessage)
{
  char szUin[24];
  sprintf(szUin, "%lu", nUin);

  return icqAuthorizeGrant(szUin, szMessage);
}

unsigned long CICQDaemon::icqAuthorizeGrant(const char *szId,
  const char *szMessage)
{
  char *sz = NULL;
  if (szMessage != NULL)
  {
    sz = gTranslator.NToRN(szMessage);
    gTranslator.ClientToServer(sz);
  }
  CPU_ThroughServer *p = new CPU_ThroughServer(szId, ICQ_CMDxSUB_AUTHxGRANTED, sz);
  gLog.Info(tr("%sAuthorizing user %s (#%hu)...\n"), L_SRVxSTR, szId, p->Sequence());
  delete [] sz;

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqAuthorizeRefuse------------------------------------------------------
unsigned long CICQDaemon::ProtoAuthorizeRefuse(const char *szId,
  unsigned long nPPID, const char *szMessage)
{
  unsigned long nRet = 0;

  if (nPPID == LICQ_PPID)
    nRet = icqAuthorizeRefuse(szId, szMessage);
  else
    PushProtoSignal(new CRefuseAuthSignal(szId, szMessage), nPPID);

  return nRet;
}

unsigned long CICQDaemon::icqAuthorizeRefuse(unsigned long nUin, const char *szMessage)
{
  char szUin[24];
  sprintf(szUin, "%lu", nUin);
  return icqAuthorizeRefuse(szUin, szMessage);
}

unsigned long CICQDaemon::icqAuthorizeRefuse(const char *szId,
  const char *szMessage)
{
  char *sz = NULL;
  if (szMessage != NULL)
  {
    sz = gTranslator.NToRN(szMessage);
    gTranslator.ClientToServer(sz);
  }
  CPU_ThroughServer *p = new CPU_ThroughServer(szId, ICQ_CMDxSUB_AUTHxREFUSED, sz);
  gLog.Info(tr("%sRefusing authorization to user %s (#%hu)...\n"), L_SRVxSTR,
     szId, p->Sequence());
  delete [] sz;

  ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
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
  // Since ICQ5.1, the status change packet is sent first, which means it is
  // assumed that the set security info packet works.
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->SetEnableSave(false);
  o->SetAuthorization(bAuthorize);
  o->SetWebAware(bWebAware);
  o->SetHideIp(bHideIp);
  o->SetEnableSave(true);
  o->SaveLicqInfo();
  unsigned short s = o->StatusFull();
  gUserManager.DropOwner();
  // Set status to ensure the status flags are set
  icqSetStatus(s);

  // Now send the set security info packet
    CPU_Meta_SetSecurityInfo *p = new CPU_Meta_SetSecurityInfo(bAuthorize, bHideIp, bWebAware);
    gLog.Info(tr("%sUpdating security info (#%hu/#%d)...\n"), L_SRVxSTR, p->Sequence(), p->SubSequence());
    ICQEvent *e = SendExpectEvent_Server(0, p, NULL);
    if (e != NULL)
      return e->EventId();
    return 0;
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
  gLog.Info(tr("%sStarting white pages search (#%hu/#%d)...\n"), L_SRVxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(0, p, NULL, true);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSearchByUin----------------------------------------------------------
unsigned long CICQDaemon::icqSearchByUin(unsigned long nUin)
{
   CPU_SearchByUin *p = new CPU_SearchByUin(nUin);
   gLog.Info(tr("%sStarting search by UIN for user (#%hu/#%d)...\n"), L_SRVxSTR, 
             p->Sequence(), p->SubSequence());
   ICQEvent *e = SendExpectEvent_Server(0, p, NULL, true);
   if (e != NULL)
     return e->EventId();
   return 0;
}

//-----icqGetUserBasicInfo------------------------------------------------------
unsigned long CICQDaemon::icqUserBasicInfo(const char *_szId)
{
  CPU_CommonFamily *p = 0;
  bool bIsAIM = isalpha(_szId[0]);
  if (bIsAIM)
    p = new CPU_RequestInfo(_szId);
  else
    p = new CPU_Meta_RequestAllInfo(_szId);
  gLog.Info(tr("%sRequesting user info (#%hu/#%d)...\n"), L_SRVxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(_szId, LICQ_PPID, p, NULL, !bIsAIM);
  if (e != NULL)
    return e->EventId();
  return 0;
}

unsigned long CICQDaemon::icqUserBasicInfo(unsigned long _nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", _nUin);
  szUin[12] = 0;

  return icqUserBasicInfo(szUin);
}

//-----icqPing------------------------------------------------------------------
void CICQDaemon::icqPing()
{
  // pinging is necessary to avoid that masquerading
  // servers close the connection
   CPU_Ping *p = new CPU_Ping;
   SendEvent_Server(p);
}

//-----icqUpdateInfoTimestamp---------------------------------------------------
void CICQDaemon::icqUpdateInfoTimestamp(const char *GUID)
{
  CPU_UpdateInfoTimestamp *p = new CPU_UpdateInfoTimestamp(GUID);
  SendEvent_Server(p);
}

//-----icqUpdatePhoneBookTimestamp----------------------------------------------
void CICQDaemon::icqUpdatePhoneBookTimestamp()
{
  ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  o->SetClientInfoTimestamp(time(NULL));
  bool bOffline = o->StatusOffline();
  gUserManager.DropOwner();

  if (!bOffline)
    icqUpdateInfoTimestamp(PLUGIN_PHONExBOOK);
}

//-----icqUpdatePictureTimestamp------------------------------------------------
void CICQDaemon::icqUpdatePictureTimestamp()
{
  ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  o->SetClientInfoTimestamp(time(NULL));
  bool bOffline = o->StatusOffline();
  gUserManager.DropOwner();

  if (!bOffline)
    icqUpdateInfoTimestamp(PLUGIN_PICTURE);
}

//-----icqSetPhoneFollowMeStatus------------------------------------------------
void CICQDaemon::icqSetPhoneFollowMeStatus(unsigned long nNewStatus)
{
  ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  o->SetClientStatusTimestamp(time(NULL));
  o->SetPhoneFollowMeStatus(nNewStatus);
  bool bOffline = o->StatusOffline();
  gUserManager.DropOwner();

  if (!bOffline)
  {
    CPU_UpdateStatusTimestamp *p =
      new CPU_UpdateStatusTimestamp(PLUGIN_FOLLOWxME, nNewStatus);
    SendEvent_Server(p);
  }
}

//-----icqUpdateContactList-----------------------------------------------------
void CICQDaemon::icqUpdateContactList()
{
  unsigned short n = 0;
  UserStringList users;
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_W)
  {
    n++;
    users.push_back(strdup(pUser->IdString()));
    if (n == m_nMaxUsersPerPacket)
    {
      CSrvPacketTcp *p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
      gLog.Info(tr("%sUpdating contact list (#%hu)...\n"), L_SRVxSTR, p->Sequence());
      SendEvent_Server(p);
      users.erase(users.begin(), users.end());
      n = 0;
    }
    // Reset all users to offline
    if (!pUser->StatusOffline()) ChangeUserStatus(pUser, ICQ_STATUS_OFFLINE);
  }
  FOR_EACH_PROTO_USER_END
  if (n != 0)
  {
    CSrvPacketTcp *p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
    gLog.Info(tr("%sUpdating contact list (#%hu)...\n"), L_SRVxSTR, p->Sequence());
    SendEvent_Server(p);
  }
}
//-----icqTypingNotification---------------------------------------------------
void CICQDaemon::icqTypingNotification(const char *_szId, bool _bActive)
{
  if (m_bSendTN)
  {
    CSrvPacketTcp *p = new CPU_TypingNotification(_szId, _bActive);
    SendEvent_Server(p);
  }
}

//-----icqCheckInvisible--------------------------------------------------------
void CICQDaemon::icqCheckInvisible(const char *_szId)
{
  CSrvPacketTcp *p = new CPU_CheckInvisible(_szId);
  SendExpectEvent_Server(_szId, LICQ_PPID, p, NULL);
}

//-----icqSendVisibleList-------------------------------------------------------
void CICQDaemon::icqSendVisibleList()
{
  // send user info packet
  // Go through the entire list of users, checking if each one is on
  // the visible list
  UserStringList users;
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_R)
  {
    if (pUser->GetInGroup(GROUPS_SYSTEM, GROUP_VISIBLE_LIST) )
      users.push_back(strdup(pUser->IdString()));
  }
  FOR_EACH_PROTO_USER_END
  CSrvPacketTcp* p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxVISIBLExLIST);
  gLog.Info(tr("%sSending visible list (#%hu)...\n"), L_SRVxSTR, p->Sequence());
  SendEvent_Server(p);
}


//-----icqSendInvisibleList-----------------------------------------------------
void CICQDaemon::icqSendInvisibleList()
{
  UserStringList users;
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_R)
  {
    if (pUser->GetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST) )
      users.push_back(strdup(pUser->IdString()));
  }
  FOR_EACH_PROTO_USER_END

  CSrvPacketTcp* p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxINVISIBxLIST);
  gLog.Info(tr("%sSending invisible list (#%hu)...\n"), L_SRVxSTR, p->Sequence());
  SendEvent_Server(p);
}

//----- icqToggleVisibleList (deprecated!) -------------------------------------
void CICQDaemon::icqToggleVisibleList(unsigned long nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0';
  ProtoToggleVisibleList(szUin, LICQ_PPID);
}

//-----ProtoToggleVisibleList------------------------------------------------
void CICQDaemon::ProtoToggleVisibleList(const char* _szId, unsigned long _nPPID)
{
  ICQUser *u = gUserManager.FetchUser(_szId, _nPPID, LOCK_R);
  if (u == NULL) return;
  bool b = u->VisibleList();
  gUserManager.DropUser(u);

  if (b)
    if (_nPPID == LICQ_PPID)
      icqRemoveFromVisibleList(_szId, _nPPID);
    else
      PushProtoSignal(new CUnacceptUserSignal(_szId), _nPPID);
  else
    if (_nPPID == LICQ_PPID)
      icqAddToVisibleList(_szId, _nPPID);
    else
      PushProtoSignal(new CAcceptUserSignal(_szId), _nPPID);
}

//-----icqToggleInvisibleList (deprecated!)-------------------------------------
void CICQDaemon::icqToggleInvisibleList(unsigned long nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0';
  ProtoToggleInvisibleList(szUin, LICQ_PPID);
}

//-----ProtoToggleInvisibleList-------------------------------------------------
void CICQDaemon::ProtoToggleInvisibleList(const char *_szId, unsigned long _nPPID)
{
  ICQUser *u = gUserManager.FetchUser(_szId, _nPPID, LOCK_R);
  if (u == NULL) return;
  bool b = u->InvisibleList();
  gUserManager.DropUser(u);

  if (b)
    if (_nPPID == LICQ_PPID)
      icqRemoveFromInvisibleList(_szId, _nPPID);
    else
      PushProtoSignal(new CUnblockUserSignal(_szId), _nPPID);
  else
    if (_nPPID == LICQ_PPID)
      icqAddToInvisibleList(_szId, _nPPID);
    else
      PushProtoSignal(new CBlockUserSignal(_szId), _nPPID);
}

//-----icqToggleIgnoreList (deprecated!)----------------------------------------
void CICQDaemon::icqToggleIgnoreList(unsigned long nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0';
  icqToggleIgnoreList(szUin, LICQ_PPID);
}

//-----icqToggleIgnoreList------------------------------------------------------
void CICQDaemon::icqToggleIgnoreList(const char *_szId, unsigned long _nPPID)
{
  ICQUser *u = gUserManager.FetchUser(_szId, _nPPID, LOCK_R);
  if (u == NULL) return;
  bool b = u->IgnoreList();
  gUserManager.DropUser(u);

  if (_nPPID == LICQ_PPID)
  {
    if (b)
      icqAddToIgnoreList(_szId, _nPPID);
    else
      icqRemoveFromIgnoreList(_szId, _nPPID);
  }
  else
  {
    if (b)
      PushProtoSignal(new CIgnoreUserSignal(_szId), _nPPID);
    else
      PushProtoSignal(new CUnignoreUserSignal(_szId), _nPPID);
  }
}

//-----icqAddToVisibleList (deprecated!)----------------------------------------
void CICQDaemon::icqAddToVisibleList(unsigned long nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0';
  icqAddToVisibleList(szUin, LICQ_PPID);
}

//-----icqAddToVisibleList------------------------------------------------------
void CICQDaemon::icqAddToVisibleList(const char* _szId, unsigned long _nPPID)
{
  ICQUser *u = gUserManager.FetchUser(_szId, _nPPID, LOCK_W);
  if (u != NULL)
  {
    u->SetVisibleList(true);
    gUserManager.DropUser(u);
  }
  CSrvPacketTcp *p = new CPU_GenericUinList(_szId, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxVISIBLExLIST);
  gLog.Info(tr("%sAdding user %s to visible list (#%hu)...\n"), L_SRVxSTR, _szId,
     p->Sequence());
  SendEvent_Server(p);

  if (UseServerContactList())
  {
    CSrvPacketTcp *pAdd = new CPU_AddToServerList(_szId, ICQ_ROSTxVISIBLE);
    addToModifyUsers(pAdd->SubSequence(), _szId);
    SendExpectEvent_Server(0, pAdd, NULL);
  }
}

//-----icqRemoveFromVisibleList (deprecated!)-----------------------------------
void CICQDaemon::icqRemoveFromVisibleList(unsigned long nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0';
  icqRemoveFromVisibleList(szUin, LICQ_PPID);
}

//-----icqRemoveFromVisibleList-------------------------------------------------
void CICQDaemon::icqRemoveFromVisibleList(const char* _szId, unsigned long _nPPID)
{
  ICQUser *u = gUserManager.FetchUser(_szId, _nPPID, LOCK_W);
  if (u != NULL)
  {
    u->SetVisibleList(false);
    gUserManager.DropUser(u);
  }
  CSrvPacketTcp *p = new CPU_GenericUinList(_szId, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_REMxVISIBLExLIST);
  gLog.Info(tr("%sRemoving user %s from visible list (#%hu)...\n"), L_SRVxSTR, _szId,
     p->Sequence());
  SendEvent_Server(p);

  if (UseServerContactList())
  {
    u = gUserManager.FetchUser(_szId, _nPPID, LOCK_R);
    if (u != NULL)
    {
      CSrvPacketTcp *pRemove = new CPU_RemoveFromServerList(_szId, 0, u->GetVisibleSID(),
        ICQ_ROSTxVISIBLE);
      addToModifyUsers(pRemove->SubSequence(), _szId);
      SendExpectEvent_Server(_szId, _nPPID, pRemove, NULL);
      gUserManager.DropUser(u);
    }
  }
}

//-----icqAddToInvisibleList (deprecated!)--------------------------------------
void CICQDaemon::icqAddToInvisibleList(unsigned long nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0';
  icqAddToInvisibleList(szUin, LICQ_PPID);
}

//-----icqAddToInvisibleList----------------------------------------------------
void CICQDaemon::icqAddToInvisibleList(const char* _szId, unsigned long _nPPID)
{
  ICQUser *u = gUserManager.FetchUser(_szId, _nPPID, LOCK_W);
  if (u != NULL)
  {
    u->SetInvisibleList(true);
    gUserManager.DropUser(u);
  }
  CSrvPacketTcp *p = new CPU_GenericUinList(_szId, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxINVISIBxLIST);
  gLog.Info(tr("%sAdding user %s to invisible list (#%hu)...\n"), L_SRVxSTR, _szId,
     p->Sequence());
  SendEvent_Server(p);

  if (UseServerContactList())
  {
    CSrvPacketTcp *pAdd = new CPU_AddToServerList(_szId, ICQ_ROSTxINVISIBLE);
    addToModifyUsers(pAdd->SubSequence(), _szId);
    SendEvent_Server(pAdd);
  }
}

//-----icqRemoveFromInvisibleList (deprecated!)---------------------------------
void CICQDaemon::icqRemoveFromInvisibleList(unsigned long nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0';
  icqRemoveFromInvisibleList(szUin, LICQ_PPID);
}

//-----icqRemoveFromInvisibleList-----------------------------------------------
void CICQDaemon::icqRemoveFromInvisibleList(const char *_szId, unsigned long _nPPID)
{
  ICQUser *u = gUserManager.FetchUser(_szId, _nPPID, LOCK_W);
  if (u != NULL)
  {
    u->SetInvisibleList(false);
    gUserManager.DropUser(u);
  }
  CSrvPacketTcp *p = new CPU_GenericUinList(_szId, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_REMxINVISIBxLIST);
  gLog.Info(tr("%sRemoving user %s from invisible list (#%hu)...\n"), L_SRVxSTR, _szId,
     p->Sequence());
  SendEvent_Server(p);

  if (UseServerContactList())
  {
    u = gUserManager.FetchUser(_szId, _nPPID, LOCK_R);
    if (u)
    {
      CSrvPacketTcp *pRemove = new CPU_RemoveFromServerList(_szId, 0, u->GetInvisibleSID(),
        ICQ_ROSTxINVISIBLE);
      addToModifyUsers(pRemove->SubSequence(), _szId);
      SendEvent_Server(pRemove);
      gUserManager.DropUser(u);
    }
  }
}

//-----icqAddToIgnoreList (deprecated!)-----------------------------------------
void CICQDaemon::icqAddToIgnoreList(unsigned long nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0';
  icqAddToIgnoreList(szUin, LICQ_PPID);
}

//-----icqAddToIgnoreList-------------------------------------------------------
void CICQDaemon::icqAddToIgnoreList(const char *_szId, unsigned long /* _nPPID */)
{
  if (!UseServerContactList()) return;

  icqRemoveUser(_szId);
  CPU_AddToServerList *pAdd = new CPU_AddToServerList(_szId, ICQ_ROSTxIGNORE,
    0, false);
  SendEvent_Server(pAdd);
}

//-----icqRemoveFromIgnoreList (deprecated!)------------------------------------
void CICQDaemon::icqRemoveFromIgnoreList(unsigned long nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0';
  icqRemoveFromIgnoreList(szUin, LICQ_PPID);
}

//-----icqRemoveFromIgnoreList--------------------------------------------------
void CICQDaemon::icqRemoveFromIgnoreList(const char *_szId, unsigned long /* _nPPID */)
{
  if (!UseServerContactList()) return;

  icqRemoveUser(_szId);
  icqAddUser(_szId, false);
}

//-----icqClearServerList-------------------------------------------------------
void CICQDaemon::icqClearServerList()
{
  if (!UseServerContactList()) return;
  
  unsigned short n = 0;
  UserStringList users;
  
  // Delete all the users in groups
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_W)
  {
    n++;
    users.push_back(strdup(pUser->IdString()));
    if (n == m_nMaxUsersPerPacket)
    {
      gUserManager.DropUser(pUser);
      CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxNORMAL);
      gLog.Info(tr("%sDeleting server list users (#%hu)...\n"), L_SRVxSTR, p->Sequence());
      SendEvent_Server(p);
      users.erase(users.begin(), users.end());
      n = 0;
    }
  }
  FOR_EACH_PROTO_USER_END
  
  if (n != 0)
  {
    CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxNORMAL);
    gLog.Info(tr("%sDeleting server list users (#%hu)...\n"), L_SRVxSTR, p->Sequence());
    SendEvent_Server(p);
  }
  
  // Delete all the groups

  
  // Delete invisible users
  n = 0;
  users.erase(users.begin(), users.end());
    
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_W)
  {
    if (pUser->GetInvisibleSID())
    {
      n++;
      users.push_back(strdup(pUser->IdString()));
    }
    
    if (n == m_nMaxUsersPerPacket)
    {
      gUserManager.DropUser(pUser);
      CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxINVISIBLE);
      gLog.Info(tr("%sDeleting server list invisible list users (#%hu)...\n"),
        L_SRVxSTR, p->Sequence());
      SendEvent_Server(p);
      users.erase(users.begin(), users.end());
      n = 0;
    }
  }
  FOR_EACH_PROTO_USER_END
  
  if (n != 0)
  {
    CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxINVISIBLE);
    gLog.Info(tr("%sDeleting server list invisible list users (#%hu)...\n"),
      L_SRVxSTR, p->Sequence());
    SendEvent_Server(p);
  }
  
  // Delete visible users
  n = 0;
  users.erase(users.begin(), users.end());
  
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_W)
  {
    if (pUser->GetVisibleSID())
    {
      n++;
      users.push_back(strdup(pUser->IdString()));
    }
    
    if (n == m_nMaxUsersPerPacket)
    {
      gUserManager.DropUser(pUser);
      CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxVISIBLE);
      gLog.Info(tr("%sDeleting server list visible list users (#%hu)...\n"),
        L_SRVxSTR, p->Sequence());
      SendEvent_Server(p);
      users.erase(users.begin(), users.end());
      n = 0;
    }
  }
  FOR_EACH_PROTO_USER_END
  
  if (n != 0)
  {
    CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxVISIBLE);
    gLog.Info(tr("%sDeleting server list visible list users (#%hu)...\n"),
      L_SRVxSTR, p->Sequence());
    SendEvent_Server(p);
  }  
  
}

//-----icqSendThroughServer-----------------------------------------------------
ICQEvent* CICQDaemon::icqSendThroughServer(const char *szId,
  unsigned char format, char *_sMessage, CUserEvent* ue, unsigned short nCharset,
  size_t nMsgLen)
{
  ICQEvent* result;
  ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_R);
  bool bOffline = u->StatusOffline();
  gUserManager.DropUser(u);

  CPU_ThroughServer *p = new CPU_ThroughServer(szId, format, _sMessage, nCharset, bOffline, nMsgLen);

  switch (format)
  {
    case ICQ_CMDxSUB_MSG:
      gLog.Info(tr("%sSending message through server (#%hu).\n"), L_SRVxSTR, p->Sequence());
      break;
    case ICQ_CMDxSUB_URL:
      gLog.Info(tr("%sSending url through server (#%hu).\n"), L_SRVxSTR, p->Sequence());
      break;
    case ICQ_CMDxSUB_CONTACTxLIST:
      gLog.Info(tr("%sSending contact list through server (#%hu).\n"), L_SRVxSTR, p->Sequence());
      break;
    default:
      gLog.Info(tr("%sSending misc through server (#%hu).\n"), L_SRVxSTR, p->Sequence());
  }

  // If we are already shutting down, don't start any events
  if (m_bShuttingDown) return NULL;

  if (ue != NULL) ue->m_eDir = D_SENDER;
  ICQEvent *e = new ICQEvent(this, m_nTCPSrvSocketDesc, p, CONNECT_SERVER, szId,
    LICQ_PPID, ue);
  if (e == NULL) return 0;
  e->m_NoAck = true;

  result = SendExpectEvent(e, &ProcessRunningEvent_Server_tep);
  return result;
}

ICQEvent* CICQDaemon::icqSendThroughServer(unsigned long nUin, unsigned char format, char *_sMessage, CUserEvent* ue, unsigned short _nCharset)
{
  char szUin[24];
  sprintf(szUin, "%lu", nUin);
  return icqSendThroughServer(szUin, format, _sMessage, ue, _nCharset);
}

//-----icqSendSms---------------------------------------------------------------
unsigned long CICQDaemon::icqSendSms(const char *szNumber, const char *szMessage,
				     unsigned long nUin)
{
  CEventSms *ue = new CEventSms(szNumber, szMessage, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, INT_VERSION);
  CPU_SendSms *p = new CPU_SendSms(szNumber, szMessage);
  gLog.Info(tr("%sSending SMS through server (#%hu/#%d)...\n"), L_SRVxSTR,
            p->Sequence(), p->SubSequence());
  ICQEvent *e = SendExpectEvent_Server(nUin, p, ue);
  if (e != NULL)
    return e->EventId();
  return 0;
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
      (e->m_eResult == EVENT_ACKED || e->m_eResult == EVENT_SUCCESS) &&
      e->m_nSubResult != ICQ_TCPxACK_RETURN)
  {
    ICQUser *u = gUserManager.FetchUser(e->m_szId, e->m_nPPID, LOCK_R);
    if (u != NULL)
    {
      e->m_pUserEvent->AddToHistory(u, LICQ_PPID, D_SENDER);
      u->SetLastSentEvent();
      m_xOnEventManager.Do(ON_EVENT_MSGSENT, u);
      gUserManager.DropUser(u);
    }
    m_sStats[STATS_EventsSent].Inc();
  }

  // Process the event, first by channel
  if (e->m_nChannel == ICQ_CHNxNEW || e->m_nChannel == ICQ_CHNxPING)
  {
    delete e;
  }
  else if (e->m_nChannel == ICQ_CHNxNONE || e->m_nChannel == ICQ_CHNxINFO ||
           e->m_nChannel == ICQ_CHNxSTATUS)
  {
    if (e->m_nCommand == ICQ_CMDxTCP_START)
      PushPluginEvent(e);
    else
      delete e;
  }
  else if (e->m_nChannel == ICQ_CHNxDATA)
  {
    // Now process by command
    switch (e->m_nSNAC)
    {
      case MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER):
      case MAKESNAC(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST):
      case MAKESNAC(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_REMOVExFROMxLIST):
      case MAKESNAC(ICQ_SNACxFAM_NEWUIN, ICQ_SNACxREGISTER_USER):
      case MAKESNAC(ICQ_SNACxFAM_LOCATION, ICQ_SNACxREQUESTxUSERxINFO):
      case MAKESNAC(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_INFOxREQ):
        PushPluginEvent(e);
        break;

      // Other events
      case MAKESNAC(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA):
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

          case EVENT_ACKED:
            PushExtendedEvent(e);
            break;

          default:
            gLog.Error("%sInternal error: ProcessDoneEvents(): Invalid result "
                       "for extended event (%d).\n", L_ERRORxSTR, e->m_eResult);
            delete e;
            return;
        }

        break;
      }

      default:
      {
        gLog.Warn("%sInternal error: ProcessDoneEvents(): Unknown SNAC (0x%08lX).\n",
                  L_ERRORxSTR, e->m_nSNAC);
        delete e;
        return;
      }
    } // switch
  }
  else
  {
  }
}

//-----ICQ::Logon--------------------------------------------------------------
unsigned long CICQDaemon::ProtoLogon(unsigned long _nPPID, unsigned short _nLogonStatus)
{
  unsigned long nRet = 0;

  if (_nPPID == LICQ_PPID)
    nRet = icqLogon(_nLogonStatus);
  else
    PushProtoSignal(new CLogonSignal(_nLogonStatus), _nPPID);

  return nRet;
}

unsigned long CICQDaemon::icqLogon(unsigned short logonStatus)
{
  if (m_bLoggingOn)
  {
    gLog.Warn(tr(tr("%sAttempt to logon while already logged or logging on, logoff and try again.\n")), L_WARNxSTR);
    return 0;
  }
  ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o->IdString() == 0)
  {
    gUserManager.DropOwner();
    gLog.Error("%sNo registered user, unable to process logon attempt.\n", L_ERRORxSTR);
    return 0;
  }
  if (o->Password()[0] == '\0')
  {
    gUserManager.DropOwner();
    gLog.Error(tr("%sNo password set.  Edit ~/.licq/owner.Licq and fill in the password field.\n"), L_ERRORxSTR);
    return 0;
  }

  char *passwd = strdup(o->Password());
  char *user = strdup(o->IdString());
  unsigned long status = o->AddStatusFlags(logonStatus);
  gUserManager.DropOwner();
  CPU_Logon *p = new CPU_Logon(passwd, user, status);
  free(passwd);
  free(user);
  m_bOnlineNotifies = false;
  gLog.Info(tr("%sRequesting logon (#%hu)...\n"), L_SRVxSTR, p->Sequence());
  m_nServerSequence = 0;
  m_nDesiredStatus = status;
  m_bLoggingOn = true;
  m_tLogonTime = time(NULL);

  SendEvent_Server(p);

  return 0;
}

//-----ICQ::icqLogoff-----------------------------------------------------------
void CICQDaemon::ProtoLogoff(unsigned long _nPPID)
{
  if (_nPPID == LICQ_PPID)
    icqLogoff();
  else
    PushProtoSignal(new CLogoffSignal(), _nPPID);
}

void CICQDaemon::icqLogoff()
{
  // Kill the udp socket asap to avoid race conditions
  int nSD = m_nTCPSrvSocketDesc;
  m_nTCPSrvSocketDesc = -1;

  m_eStatus = STATUS_OFFLINE_MANUAL;

  if (nSD == -1 && !m_bLoggingOn)
  {
    gLog.Warn(tr("%sAttempt to logoff while not logged on.\n"), L_WARNxSTR);
    return;
  }

  m_bLoggingOn = false;

  gLog.Info(tr("%sLogging off.\n"), L_SRVxSTR);
  ICQEvent *cancelledEvent = NULL;

  if (nSD != -1)
  {
    CPU_Logoff p;
    cancelledEvent = new ICQEvent(this, nSD, &p, CONNECT_SERVER, 0, NULL);
    cancelledEvent->m_pPacket = NULL;
    cancelledEvent->m_bCancelled = true;
    SendEvent(nSD, p, true);
    gSocketManager.CloseSocket(nSD);
  }

  postLogoff(nSD, cancelledEvent);
}

void CICQDaemon::postLogoff(int nSD, ICQEvent *cancelledEvent)
{
  pthread_mutex_lock(&mutex_runningevents);
  pthread_mutex_lock(&mutex_sendqueue_server);
  pthread_mutex_lock(&mutex_extendedevents);
  pthread_mutex_lock(&mutex_cancelthread);
  pthread_mutex_lock(&mutex_reverseconnect);
  std::list<ICQEvent *>::iterator iter;

  // Cancel all events
  // Necessary since the end is always being modified
  unsigned long i = m_lxSendQueue_Server.size();
  for (iter = m_lxSendQueue_Server.begin(); i > 0; i--)
  {
    ICQEvent *e = *iter;
    gLog.Info("Event #%hu is still on the server queue!\n", e->Sequence());
    iter = m_lxSendQueue_Server.erase(iter);
    ICQEvent *cancelled = new ICQEvent(e);
    cancelled->m_bCancelled = true;
    m_lxSendQueue_Server.push_back(cancelled);
  }

  iter = m_lxRunningEvents.begin();
  while (iter != m_lxRunningEvents.end())
  {
    if ((*iter)->m_nSocketDesc == nSD || (*iter)->Channel() == ICQ_CHNxNEW)
    {
      ICQEvent *e = *iter;
      gLog.Info("Event #%hu is still on the running queue!\n", e->Sequence());
      iter = m_lxRunningEvents.erase(iter);
      if (e->thread_running && !pthread_equal(e->thread_send, pthread_self()))
      {
        pthread_cancel(e->thread_send);
        e->thread_running = false;
      }
      std::list<ICQEvent *>::iterator i;
      for (i = m_lxExtendedEvents.begin(); i != m_lxExtendedEvents.end(); ++i)
      {
        if (*i == e)
        {
          m_lxExtendedEvents.erase(i);
          break;
        }
      }
      CancelEvent(e);
    }
    else
      ++iter;
  }
  assert(m_lxExtendedEvents.empty());

  // Queue should be empty, might not be due to peer-to-peer events
  for (iter = m_lxRunningEvents.begin(); iter != m_lxRunningEvents.end(); ++iter)
    gLog.Info(tr("Event #%hu is still on queue!\n"), (*iter)->Sequence());

  if (cancelledEvent != NULL)
    m_lxSendQueue_Server.push_back(cancelledEvent);

  std::list<CReverseConnectToUserData *>::iterator rciter;
  for (rciter = m_lReverseConnect.begin(); rciter != m_lReverseConnect.end();
                                           ++rciter)
  {
    delete *rciter;
  }
  m_lReverseConnect.clear();

  pthread_mutex_unlock(&mutex_reverseconnect);    
  pthread_mutex_unlock(&mutex_cancelthread);
  pthread_mutex_unlock(&mutex_extendedevents);
  pthread_mutex_unlock(&mutex_sendqueue_server);
  pthread_mutex_unlock(&mutex_runningevents);

  // All extended event are a pointer that are also in the running events.
  // We do not need to clean these out.
#if 0
  // wipe out all extended events too...
  pthread_mutex_lock(&mutex_extendedevents);
  for (iter = m_lxExtendedEvents.begin(); iter != m_lxExtendedEvents.end(); ++iter)
  {
    (*iter)->m_eResult = EVENT_CANCELLED;
    ProcessDoneEvent(*iter);
  }
  m_lxExtendedEvents.erase(m_lxExtendedEvents.begin(), m_lxExtendedEvents.end());
  pthread_mutex_unlock(&mutex_extendedevents);
#endif

  // Mark all users as offline, this also updates the last seen
  // online field
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_W)
  {
    if (!pUser->StatusOffline())
      ChangeUserStatus(pUser, ICQ_STATUS_OFFLINE);
  }
  FOR_EACH_PROTO_USER_END

  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  if (o)
    ChangeUserStatus(o, ICQ_STATUS_OFFLINE);
  gUserManager.DropOwner();

  if (m_szRegisterPasswd)
  {
    free(m_szRegisterPasswd);
    m_szRegisterPasswd = 0;
  }

  PushPluginSignal(new CICQSignal(SIGNAL_LOGOFF, 0, 0, LICQ_PPID, 0, 0));
}

//-----ProtoTypingNotification-------------------------------------------------
void CICQDaemon::ProtoTypingNotification(const char *_szId,
  unsigned long _nPPID, bool _bActive, int nSocket)
{
  //TODO: Make for each plugin
  if (m_bSendTN)
  {
    if (_nPPID == LICQ_PPID)
      icqTypingNotification(_szId, _bActive);
    else 
      PushProtoSignal(new CTypingNotificationSignal(_szId, _bActive, nSocket), _nPPID);
  }
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----ConnectToServer---------------------------------------------------------

int CICQDaemon::ConnectToLoginServer()
{
  if (m_bProxyEnabled)
    InitProxy();

  // Which protocol plugin?
  int r = ConnectToServer(m_szICQServer, m_nICQServerPort);

  write(pipe_newsocket[PIPE_WRITE], "S", 1);

  return r;
}

int CICQDaemon::ConnectToServer(const char* server, unsigned short port)
{
  SrvSocket *s = new SrvSocket(gUserManager.OwnerUin());

  if (m_bProxyEnabled)
  {
    if (m_xProxy == NULL)
    {
      gLog.Warn(tr("%sProxy server not properly configured.\n"), L_ERRORxSTR);
      delete s;
      return (-1);
    }
    s->SetProxy(m_xProxy);
  }
  else if (m_xProxy != NULL)
  {
    delete m_xProxy;
    m_xProxy = NULL;
  }
  
  char ipbuf[32];

  if (m_xProxy == NULL)
  {
    gLog.Info(tr("%sResolving %s port %d...\n"), L_SRVxSTR, server, port);
    if (!s->SetRemoteAddr(server, port)) {
      char buf[128];
      gLog.Warn(tr("%sUnable to resolve %s:\n%s%s.\n"), L_ERRORxSTR,
                server, L_BLANKxSTR, s->ErrorStr(buf, 128));
      delete s;
      return (-1); // no route to host (not connected)
    }
    gLog.Info(tr("%sICQ server found at %s:%d.\n"), L_SRVxSTR,
	      s->RemoteIpStr(ipbuf), s->RemotePort());
  }
  else
  {
    // It doesn't matter if it resolves or not, the proxy should do it then
    s->SetRemoteAddr(server, port);
  }

  if (m_xProxy == NULL)
    gLog.Info(tr("%sOpening socket to server.\n"), L_SRVxSTR);
  else
    gLog.Info("%sOpening socket to server via proxy.\n", L_SRVxSTR);
  if (!s->OpenConnection())
  {
    char buf[128];
    gLog.Warn(tr("%sUnable to connect to %s:%d:\n%s%s.\n"), L_ERRORxSTR,
              s->RemoteIpStr(ipbuf), s->RemotePort(), L_BLANKxSTR,
              s->ErrorStr(buf, 128));
    delete s;
    return -1;
  }

  static pthread_mutex_t connect_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&connect_mutex);
  int nSocket;
  if (m_nTCPSrvSocketDesc != -1)
  {
    gLog.Warn(tr("%sConnection to server already exists, aborting.\n"), L_WARNxSTR);
    delete s;
    nSocket = -1;
  }
  else
  {
    // Now get the internal ip from this socket
    CPacket::SetLocalIp(  NetworkIpToPacketIp(s->LocalIp() ));
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    if (o)
      o->SetIntIp(s->LocalIp());
    gUserManager.DropOwner();

    gSocketManager.AddSocket(s);
    nSocket = m_nTCPSrvSocketDesc = s->Descriptor();
    gSocketManager.DropSocket(s);
  }

  pthread_mutex_unlock(&connect_mutex);

  return nSocket;
}

//-----FindUserForInfoUpdate-------------------------------------------------
ICQUser *CICQDaemon::FindUserForInfoUpdate(const char *szId, ICQEvent *e,
   const char *t)
{
  ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL)
  {
    // If the event is NULL as well then nothing we can do
    if (e == NULL)
    {
      gLog.Warn(tr("%sResponse to unknown %s info request for unknown user (%s).\n"),
                L_WARNxSTR, t, szId);
      return NULL;
    }
    // Check if we need to create the user
    if (e->m_pUnknownUser == NULL)
    {
      e->m_pUnknownUser = new ICQUser(szId, LICQ_PPID);
    }
    // If not, validate the uin
    else if (strcmp(e->m_pUnknownUser->IdString(), szId) != 0)
    {
      gLog.Error("%sInternal Error: Event contains wrong user.\n", L_ERRORxSTR);
      return NULL;
    }

    u = e->m_pUnknownUser;
    u->Lock(LOCK_W);
  }
  gLog.Info(tr("%sReceived %s information for %s (%s).\n"), L_SRVxSTR, t,
            u->GetAlias(), szId);
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
    gLog.Unknown(tr("%sUnknown server response:\n%s\n"), L_UNKNOWNxSTR,
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
      if (m_bVerify)
        icqVerifyRegistration();
      else
      {
        icqRegisterFinish();
        m_bRegistering = false;
      }
    }
    break;

  case ICQ_CHNxDATA:
    ProcessDataChannel(packet);
    break;

  case ICQ_CHNxERROR:
    gLog.Warn(tr("%sPacket on unhandled Channel 'Error' received!\n"), L_SRVxSTR);
    break;

  case ICQ_CHNxCLOSE:
    return ProcessCloseChannel(packet);
    break;

  default:
    gLog.Warn(tr("%sServer send unknown channel: %02x\n"), L_SRVxSTR, nChannel);
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

      gLog.Info(tr("%sServer says he's ready.\n"), L_SRVxSTR);

      gLog.Info(tr("%sSending our channel capability list...\n"), L_SRVxSTR);
      p = new CPU_ImICQ();
      SendEvent_Server(p);

      gLog.Info(tr("%sRequesting rate info...\n"), L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSUB_REQ_RATE_INFO);
      SendEvent_Server(p);

      // ask the user who we are (yeah we know already though)
      p = new CPU_GenericFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_GETxUSERxINFO);
      SendEvent_Server(p);

      break;
    }

  case ICQ_SNACxSRV_ACKxIMxICQ:
    {
      // ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
      // unsigned long nListTime = o->GetSSTime();
      // gUserManager.DropOwner(LICQ_PPID);

      CSrvPacketTcp* p;
      gLog.Info(tr("%sServer sent us channel capability list (ignoring).\n"), L_SRVxSTR);

      gLog.Info(tr("%sRequesting self info.\n"), L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_GETxUSERxINFO);
      SendEvent_Server(p);

      gLog.Info(tr("%sRequesting list rights.\n"), L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_REQUESTxRIGHTS);
      SendEvent_Server(p);

      gLog.Info(tr("%sRequesting list.\n"), L_SRVxSTR);
      p = new CPU_RequestList();
      SendExpectEvent_Server(0, p, NULL);

      gLog.Info(tr("%sRequesting location rights.\n"), L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxRIGHTS);
      SendEvent_Server(p);

      gLog.Info(tr("%sRequesting contact list rights.\n"), L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_REQUESTxRIGHTS);
      SendEvent_Server(p);

      gLog.Info(tr("%sRequesting Instant Messaging rights.\n"), L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_REQUESTxRIGHTS);
      SendEvent_Server(p);

      gLog.Info(tr("%sRequesting BOS rights.\n"), L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_REQUESTxRIGHTS);
      SendEvent_Server(p);

      break;
    }
  case ICQ_SNACxSUB_RATE_INFO:
  {
      gLog.Info(tr("%sServer sent us rate information.\n"), L_SRVxSTR);
      CSrvPacketTcp *p = new CPU_RateAck();
      SendEvent_Server(p);

      gLog.Info(tr("%sSetting ICQ Instant Messaging Mode.\n"), L_SRVxSTR);
      p = new CPU_ICQMode(1, 11); // enable typing notifications
      SendEvent_Server(p);
      p = new CPU_ICQMode(2, 3); // set default flags for all channels
      SendEvent_Server(p);
      p = new CPU_ICQMode(4, 3); // set default flags for all channels
      SendEvent_Server(p);

      gLog.Info(tr("%sSending capability settings (?)\n"),L_SRVxSTR);
      p = new CPU_CapabilitySettings();
      SendEvent_Server(p);

      break;
    }

  case ICQ_SNACxRCV_NAMExINFO:
  {
    unsigned short nUserClass, nLevel;
    unsigned long nUin, realIP;
    time_t nOnlineSince = 0;

    gLog.Info(tr("%sGot Name Info from Server\n"), L_SRVxSTR);

    nUin = packet.UnpackUinString();
    nLevel = packet.UnpackUnsignedShortBE();
    nUserClass = packet.UnpackUnsignedShortBE();

    gLog.Info("%sUIN: %lu level: %04hx Class: %04hx\n", L_SRVxSTR,
              nUin, nLevel, nUserClass );

    if (!packet.readTLV()) {
      char *buf;
      gLog.Unknown(tr("%sUnknown server response:\n%s\n"), L_UNKNOWNxSTR,
         packet.print(buf));
      delete [] buf;
      break;
    }

    // T(1) unknown
    // T(2) member since
    // T(3) online since
    // T(4) idle time
    // T(6) status code
    // T(A) IP
    // T(C) direct connection info
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
      gLog.Info(tr("%sServer says we are at %s.\n"), L_SRVxSTR, ip_ntoa(realIP, buf));
      //icqSetStatus(m_nDesiredStatus);
    }
    if (packet.getTLVLen(0x0003) == 4)
      nOnlineSince = packet.UnpackUnsignedLongTLV(0x0003);

    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    unsigned long nPFM = o->PhoneFollowMeStatus();
    // Workaround for the ICQ4.0 problem of it not liking the PFM flags
    if (nPFM != ICQ_PLUGIN_STATUSxINACTIVE)
      m_nDesiredStatus |= ICQ_STATUS_FxPFM;
    if (nPFM == ICQ_PLUGIN_STATUSxACTIVE)
      m_nDesiredStatus |= ICQ_STATUS_FxPFMxAVAILABLE;
    ChangeUserStatus(o, m_nDesiredStatus);
    o->SetOnlineSince(nOnlineSince);
    gLog.Info(tr("%sServer says we're now: %s\n"), L_SRVxSTR, ICQUser::StatusToStatusStr(o->Status(), o->StatusInvisible()));

    gUserManager.DropOwner();

    break;
  }

  case ICQ_SNACxSUB_RATE_WARNING:
  {
    // TODO: Inform the user:
    // we are sending fast, if we keep it up we will be kicked off
    break;
  }

  case ICQ_SNACxSUB_PAUSE:
  {
    // Server is going to disconnect/pause (maintainance?)
    // lets change servers and reconnect.
    gLog.Info(tr("%sServer is going to disconnect/pause. Lets reconnect to another one.\n"), L_SRVxSTR);
    icqRelogon();
    break;
  }

  default:
    gLog.Warn("%sUnknown Service Family Subtype: %04hx\n", L_SRVxSTR, nSubtype);
    break;
  }
}

//--------ProcessLocationFam-----------------------------------------------
void CICQDaemon::ProcessLocationFam(CBuffer &packet, unsigned short nSubtype)
{
  /*unsigned short nFlags = */packet.UnpackUnsignedShortBE();
  unsigned long nSubSequence = packet.UnpackUnsignedLongBE();

  switch (nSubtype)
  {
  case ICQ_SNAXxLOC_RIGHTSxGRANTED:
    gLog.Info(tr("%sReceived rights for Location Services.\n"), L_SRVxSTR);
    break;

  case ICQ_SNACxREPLYxUSERxINFO:
  {
    char *szId = packet.UnpackUserString();
    if (!szId) break;
    packet.UnpackUnsignedLongBE(); // Unknown
    
    if (!packet.readTLV())
    {
      gLog.Error("%sError during parsing user information packet!\n", L_ERRORxSTR);
      break;
    }
   
    char *szAwayMsg = packet.UnpackStringTLV(0x0004);
    if (szAwayMsg)
    {
      gLog.Info(tr("%sReceived away message for %s.\n"), L_SRVxSTR, szId);
      ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
      if (strcmp(szAwayMsg, u->AutoResponse()))
      {
        u->SetAutoResponse(szAwayMsg);
        u->SetShowAwayMsg(*szAwayMsg);
      }
      gUserManager.DropUser(u);

      ICQEvent *e = DoneServerEvent(nSubSequence, EVENT_SUCCESS);
      if (e)
        ProcessDoneEvent(e);
    }

    char *szInfo = packet.UnpackStringTLV(0x0002);
    if (szInfo)
    {
      gLog.Info(tr("%sReceived user information for %s.\n"), L_SRVxSTR, szId);
      ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
      u->SetEnableSave(false);
      u->SetAbout(szInfo);
      delete [] szInfo;

      // translating string with Translation Table
      gTranslator.ServerToClient(u->GetAbout());

      delete [] szId;
      szId = strdup(u->IdString());
      
      // save the user infomation
      u->SetEnableSave(true);
      u->SaveAboutInfo();
      gUserManager.DropUser(u);

      ICQEvent *e = DoneServerEvent(nSubSequence, EVENT_SUCCESS);
      if (e)
        ProcessDoneEvent(e);
      
      
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_ABOUT, szId, LICQ_PPID));
      free(szId);
    }

    break;
  }

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
    unsigned long junk1, intIP, userPort, nInfoTimestamp = 0,
                  nStatusPluginTimestamp = 0, nInfoPluginTimestamp = 0, nCookie,
                  nUserIP;
    time_t registeredTimestamp;
    unsigned short junk2;
    unsigned char mode;
    char *szId;

    junk1 = packet.UnpackUnsignedLongBE();
    junk2 = packet.UnpackUnsignedShortBE();
    szId = packet.UnpackUserString();

    junk1 = packet.UnpackUnsignedLongBE(); // tlvcount

    if (!packet.readTLV()) {
      gLog.Error("%sTLV Error\n", L_ERRORxSTR);
      return;
    }

//     userIP = packet.UnpackUnsignedLongTLV(0x0a, 1);
//     rev_e_long(userIP);
    ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
    if (u == NULL)
    {
      gLog.Warn(tr("%sUnknown user (%s) changed status.\n"), L_WARNxSTR,
                szId);
      delete [] szId;
      break;
    }
    delete [] szId;

    // 0 if not set -> Online
    unsigned long nNewStatus = 0;
    unsigned long nOldStatus = u->StatusFull();
    nUserIP = 0;

    // AIM status
    if (packet.getTLVLen(0x0001) == 2)
    {
      unsigned short nStatus = packet.UnpackUnsignedShortTLV(0x0001);
      if (nStatus & 0x0020)
        nNewStatus = ICQ_STATUS_AWAY;
      else
        nNewStatus = ICQ_STATUS_ONLINE;
    }

    // ICQ status
    if (packet.getTLVLen(0x0006))
      nNewStatus = packet.UnpackUnsignedLongTLV(0x0006);

    if (packet.getTLVLen(0x000a) == 4) {
      nUserIP = packet.UnpackUnsignedLongTLV(0x000a);
      if (nUserIP) {
        rev_e_long(nUserIP);
        nUserIP = PacketIpToNetworkIp(nUserIP);
      }
    }
    if( u->StatusOffline() || nUserIP )
      u->SetIp(nUserIP);

    if (packet.getTLVLen(0x0002) == 4)
    {
      registeredTimestamp = packet.UnpackUnsignedLongTLV(0x0002);
      u->SetRegisteredTime(registeredTimestamp);
    }

    if (packet.getTLVLen(0x0003) == 4)
    {
      time_t nOnlineSince = packet.UnpackUnsignedLongTLV(0x0003);
      u->SetOnlineSince(nOnlineSince);
    }

    if (packet.getTLVLen(0x0004) == 2)
    {
      unsigned short nIdleTime = packet.UnpackUnsignedShortTLV(0x0004);
      u->SetIdleSince(time(NULL) - (nIdleTime * 60));
    }
    else
      u->SetIdleSince(0);

    // Sometimes there ix 0x0005 instead of 0x0002. They appear to be mutually exclusive.
    if (packet.getTLVLen(0x0005) == 4)
    {
      registeredTimestamp = packet.UnpackUnsignedLongTLV(0x0005);
      u->SetRegisteredTime(registeredTimestamp);
    }

    if (packet.getTLVLen(0x000c) == 0x25)
    {
      CBuffer msg = packet.UnpackTLV(0x000c);

      intIP = msg.UnpackUnsignedLong();
      userPort = msg.UnpackUnsignedLongBE();
      mode = msg.UnpackChar();
      unsigned short tcpVersion = msg.UnpackUnsignedShortBE();
      nCookie = msg.UnpackUnsignedLongBE();
      msg.UnpackUnsignedLongBE();
      unsigned long tcount = msg.UnpackUnsignedLongBE();
      nInfoTimestamp = msg.UnpackUnsignedLongBE();  // will be licq version
      nStatusPluginTimestamp = msg.UnpackUnsignedLongBE();
      nInfoPluginTimestamp = msg.UnpackUnsignedLongBE();
      if (tcount == 0)
        nInfoTimestamp = nStatusPluginTimestamp = nInfoPluginTimestamp = 0;
      else if (tcount == 1)
        nStatusPluginTimestamp = nInfoPluginTimestamp = 0;
      else if (tcount == 2)
        nInfoPluginTimestamp = 0;
        
      junk2 = msg.UnpackUnsignedShortBE();

      char szExtraInfo[28] = { 0 };
      if ((nInfoTimestamp & 0xFFFF0000) == LICQ_WITHSSL)
        snprintf(szExtraInfo, 27, "Licq %s/SSL",
                 CUserEvent::LicqVersionToString(nInfoTimestamp & 0xFFFF));
      else if ((nInfoTimestamp & 0xFFFF0000) == LICQ_WITHOUTSSL)
        snprintf(szExtraInfo, 27, "Licq %s",
                 CUserEvent::LicqVersionToString(nInfoTimestamp & 0xFFFF));
      else if (nInfoTimestamp == 0xffffffff)
        strcpy(szExtraInfo, "MIRANDA");
      else if (nInfoTimestamp == 0xFFFFFF8F)
        strcpy(szExtraInfo, "StrICQ");
      else if (nInfoTimestamp == 0xFFFFFF42)
        strcpy(szExtraInfo, "mICQ");
      else if (nInfoTimestamp == 0xFFFFFF7F)
        strcpy(szExtraInfo, "&RQ");
      else if (nInfoTimestamp == 0xFFFFFFAB)
        strcpy(szExtraInfo, "YSM");
      else
        szExtraInfo[0]=0;
      szExtraInfo[27] = '\0';

      u->SetClientInfo(szExtraInfo[0] ? szExtraInfo : NULL);
      u->SetVersion(tcpVersion);
      
      if (nOldStatus != nNewStatus)
      {
        char *szClient = new char[strlen(szExtraInfo)+6];
        if (szExtraInfo[0])
          sprintf(szClient, " [%s].\n", szExtraInfo);
        else
          sprintf(szClient, ".\n");

        ChangeUserStatus(u, nNewStatus);
        gLog.Info(tr("%s%s (%s) changed status: %s (v%d)%s"),
                   L_SRVxSTR, u->GetAlias(), u->IdString(), u->StatusStr(),
                   tcpVersion & 0x0F, szClient);
        if ( (nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS) )
          gLog.Unknown("%sUnknown status flag for %s (%s): 0x%08lX\n",
                       L_UNKNOWNxSTR, u->GetAlias(), u->IdString(),
                       (nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS));
        nNewStatus &= ICQ_STATUS_FxUNKNOWNxFLAGS;
        u->SetAutoResponse(NULL);
        u->SetShowAwayMsg(false);

        delete [] szClient;
      }

      if (intIP)
      {
        intIP = PacketIpToNetworkIp(intIP);
        u->SetIntIp(intIP);
      }

      if (userPort)
        u->SetPort(userPort);

      u->SetCookie(nCookie);

      /* Mode 0 is used improperly by too many clients,
         don't bother warning, just set it to be indirect */
      if (mode == MODE_DENIED || mode == 6 || mode == 0 ||
          (nNewStatus & ICQ_STATUS_FxDIRECTxLISTED) ||
          (nNewStatus & ICQ_STATUS_FxDIRECTxAUTH)
        ) mode = MODE_INDIRECT;
      if (mode != MODE_DIRECT && mode != MODE_INDIRECT)
      {
        gLog.Unknown("%sUnknown peer-to-peer mode for %s (%s): %d\n", L_UNKNOWNxSTR,
                     u->GetAlias(), u->IdString(), mode);
        u->SetMode(MODE_DIRECT);
        u->SetSendServer(false);
      }
      else
      {
        u->SetMode(mode);
        u->SetSendServer(mode == MODE_INDIRECT);
      }
    }
    else // for AIM users
    {
      if (nOldStatus != nNewStatus)
      {
        ChangeUserStatus(u, nNewStatus);
        gLog.Info(tr("%s%s changed status: %s (AIM).\n"), L_SRVxSTR, u->GetAlias(),
                  u->StatusStr());
        if ( (nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS) )
          gLog.Unknown("%sUnknown status flag for %s: 0x%08lX\n",
                       L_UNKNOWNxSTR, u->GetAlias(),
                       nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS);
        u->SetAutoResponse(NULL);
        u->SetShowAwayMsg(false);
      }
    }

    if (u->OurClientStatusTimestamp() != nStatusPluginTimestamp &&
        nOldStatus == ICQ_STATUS_OFFLINE)
    {
      //we don't know what the new values are yet, so don't show anything
      u->SetPhoneFollowMeStatus(ICQ_PLUGIN_STATUSxINACTIVE);
      u->SetICQphoneStatus(ICQ_PLUGIN_STATUSxINACTIVE);
      u->SetSharedFilesStatus(ICQ_PLUGIN_STATUSxINACTIVE);
    }
    
    if (packet.hasTLV(0x000D))
    {
      CBuffer capBuf = packet.UnpackTLV(0x000D);
      int nCapSize = packet.getTLVLen(0x000D);
      char *caps = new char[nCapSize];    
      for (unsigned short i = 0; i < nCapSize; i++)
        capBuf >> caps[i];

      // Check if they support UTF8
      bool bUTF8 = false;

      // Check capability flags for their client version
      string version = "";
      char tmpVer[24];
      unsigned int ver1, ver2, ver3;

      for (int i = 0; i < (nCapSize / CAP_LENGTH); i++)
      {
        if (memcmp(caps+(i * CAP_LENGTH), ICQ_CAPABILITY_UTF8, CAP_LENGTH) == 0)
        {
          bUTF8 = true;
        }
        else if (memcmp(caps+(i * CAP_LENGTH), ICQ_CAPABILITY_LICQxVER,
                 strlen(ICQ_CAPABILITY_LICQxVER)) == 0)
        {
          char *verStr = caps+((i+1)*CAP_LENGTH-4);
          ver1 = verStr[0];
          ver2 = verStr[1]%100;
          ver3 = verStr[2];
          snprintf(tmpVer, sizeof(tmpVer)-1, "%u.%u.%u", ver1, ver2, ver3);
          version = "Licq v" + string(tmpVer);
          if (verStr[3] == 1)
            version += "/SSL";
        }
        else if (memcmp(caps+(i * CAP_LENGTH), ICQ_CAPABILITY_KOPETExVER,
                 strlen(ICQ_CAPABILITY_KOPETExVER)) == 0)
        {
          char *verStr = caps+((i+1)*CAP_LENGTH-4);
          ver1 = verStr[0];
          ver2 = verStr[1];
          ver3 = verStr[2]*100;
          ver3 += verStr[3];
          snprintf(tmpVer, sizeof(tmpVer)-1, "%u.%u.%u", ver1, ver2, ver3);
          version = "Kopete v" + string(tmpVer);
        }
        else if (memcmp(caps+(i * CAP_LENGTH), ICQ_CAPABILITY_SIMxVER,
                 strlen(ICQ_CAPABILITY_SIMxVER)) == 0)
        {
          version = "SIM";
        }
        else if (memcmp(caps+(i * CAP_LENGTH), ICQ_CAPABILITY_MICQxVER,
                 strlen(ICQ_CAPABILITY_MICQxVER)) == 0)
        {
          version = "mICQ";
        }
      }  
      delete [] caps;

      if (u)
      {
        u->SetSupportsUTF8(bUTF8);
        if (version != "")
          u->SetClientInfo(version.c_str());
      }
    }
    
    if (packet.hasTLV(0x0011))
    {
      CBuffer msg = packet.UnpackTLV(0x0011);
      char index = msg.UnpackChar();
      unsigned long nTime = msg.UnpackUnsignedLong(); //Timestamp

      char plugin[GUID_LENGTH];
      switch (index)
      {
      case 0x01: //Updated user info
        gLog.Info("%s%s updated info.\n", L_SRVxSTR, u->GetAlias());
        u->SetUserUpdated(false);
        break;

      case 0x02: //Updated info plugin settings
        msg.incDataPosRead(6); /* 02 00 01 00 01 00
                                  Don't know what those mean */
        for (unsigned int i = 0; i < sizeof(plugin); i ++)
          plugin[i] = msg.UnpackChar();

        if (memcmp(plugin, PLUGIN_PHONExBOOK, GUID_LENGTH) == 0)
          gLog.Info("%s%s updated phonebook\n", L_SRVxSTR, u->GetAlias());
        else if (memcmp(plugin, PLUGIN_PICTURE, GUID_LENGTH) == 0)
          gLog.Info("%s%s updated picture\n", L_SRVxSTR, u->GetAlias());
        else if (memcmp(plugin, PLUGIN_QUERYxINFO, GUID_LENGTH) == 0)
          gLog.Info("%s%s updated info plugin list\n", L_SRVxSTR,
                    u->GetAlias());
        else
          gLog.Warn("%sUnknown info plugin update from %s\n", L_WARNxSTR,
                    u->GetAlias());

        u->SetUserUpdated(false);

        break;

      case 0x03: //Updated status plugin settings
      {
        msg.incDataPosRead(6); /* 00 00 01 00 01 00 */
        for (unsigned int i = 0; i < sizeof(plugin); i ++)
          plugin[i] = msg.UnpackChar();

        msg.incDataPosRead(1);  /* 01 */

        unsigned long nPluginStatus = msg.UnpackUnsignedLong();
        const char* state;
        switch (nPluginStatus)
        {
          case ICQ_PLUGIN_STATUSxINACTIVE: state = "inactive"; break;
          case ICQ_PLUGIN_STATUSxACTIVE:   state = "active";   break;
          case ICQ_PLUGIN_STATUSxBUSY:     state = "busy";     break;
          default:                         state = "unknown";  break;
        }

        if (memcmp(plugin, PLUGIN_FOLLOWxME, GUID_LENGTH) == 0)
        {
            gLog.Info(tr("%s%s changed Phone \"Follow Me\" status to %s.\n"),
                                              L_SRVxSTR, u->GetAlias(), state);
            u->SetPhoneFollowMeStatus(nPluginStatus);
        }
        else if (memcmp(plugin, PLUGIN_FILExSERVER, GUID_LENGTH) == 0)
        {
            gLog.Info(tr("%s%s changed Shared Files Directory status to %s.\n"),
              L_SRVxSTR, u->GetAlias(), state);
            u->SetSharedFilesStatus(nPluginStatus);
        }
        else if (memcmp(plugin, PLUGIN_ICQxPHONE, GUID_LENGTH) == 0)
        {
            gLog.Info(tr("%s%s changed ICQphone status to %s.\n"), L_SRVxSTR,
              u->GetAlias(), state);
            u->SetICQphoneStatus(nPluginStatus);
        }

        // if status was up to date then we don't need to reask the user
        if (u->OurClientStatusTimestamp() == u->ClientStatusTimestamp() &&
            nOldStatus != ICQ_STATUS_OFFLINE)
          u->SetOurClientStatusTimestamp(nTime);


        // Which Plugin?
        PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                        USER_PLUGIN_STATUS, u->IdString(), u->PPID(), 0));

        break;
      }

      default:
        gLog.Warn(tr("%sUnknown index %d from %s.\n"), L_WARNxSTR, index,
        u->GetAlias());
      }
    }

    // maybe use this for auto update info later
    u->SetClientTimestamp(nInfoTimestamp);
    u->SetClientInfoTimestamp(nInfoPluginTimestamp);
    u->SetClientStatusTimestamp(nStatusPluginTimestamp);

    // We are no longer able to differentiate oncoming users from the
    // users that are on when we sign on.. so try the online since flag
    if ((m_bAlwaysOnlineNotify || (u->OnlineSince()+60 >= time(NULL))) &&
        nOldStatus == ICQ_STATUS_OFFLINE && u->OnlineNotify())
      m_xOnEventManager.Do(ON_EVENT_NOTIFY, u);
    gUserManager.DropUser(u);
    break;
  }
  case ICQ_SNACxSUB_OFFLINExLIST:
  {
    unsigned long junk1;
    unsigned short junk2;
    char *szId;
    bool bFake = false;

    junk1 = packet.UnpackUnsignedLongBE();
    junk2 = packet.UnpackUnsignedShortBE();
    szId = packet.UnpackUserString();

    if (packet.readTLV())
    {
      if (packet.hasTLV(0x0003) && packet.getTLVLen(0x0003) == 4)
        bFake = true;
    }

    // AIM users send this when they really do go offline, so skip it if it is
    // an AIM user
    if (bFake && isdigit(szId[0]))
    {
      ICQUser *user = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_R);
      //XXX Debug output
      //gLog.Error("%sIgnoring fake offline: %s (%s)\n", L_SRVxSTR,
      //    user->GetAlias(), szId);
      delete [] szId;
      gUserManager.DropUser(user);
      break;
    }


    ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
    if (u == NULL)
    {
      gLog.Warn(tr("%sUnknown user (%s) has gone offline.\n"), L_WARNxSTR, szId);
      delete [] szId;
      break;
    }
    delete [] szId;

    gLog.Info(tr("%s%s went offline.\n"), L_SRVxSTR, u->GetAlias());
    u->SetClientTimestamp(0);
    u->SetTyping(ICQ_TYPING_INACTIVEx0); 
    ChangeUserStatus(u, ICQ_STATUS_OFFLINE); 
    gUserManager.DropUser(u); 
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_TYPING, 
                                    u->IdString(), LICQ_PPID)); 
    break;
  }
  case ICQ_SNACxBDY_RIGHTSxGRANTED:
  {
    gLog.Info(tr("%sReceived rights for Contact List..\n"), L_SRVxSTR);

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
  /*unsigned short Flags =*/ packet.UnpackUnsignedShortBE();
  unsigned long nSubSequence = packet.UnpackUnsignedLongBE();

  switch (nSubtype)
  {
  case ICQ_SNACxMSG_ICBMxERROR:
  {
    unsigned short err = packet.UnpackUnsignedShortBE();

    ICQEvent *e = DoneServerEvent(nSubSequence, EVENT_ERROR);

    switch (err)
    {
    case 0x0004:
      gLog.Warn(tr("%sUser is offline.\n"), L_WARNxSTR);
      break;
    case 0x0009:
      gLog.Warn(tr("%sClient does not understand type-2 messages.\n"), L_WARNxSTR);
      break;
    case 0x000e:
      gLog.Warn(tr("%sPacket was malformed.\n"), L_WARNxSTR);
      break;
    case 0x0015:
      gLog.Info(tr("%sList overflow.\n"), L_WARNxSTR);
      break;
    default:
      gLog.Unknown(tr("%sUnknown ICBM error: 0x%04x.\n"), L_UNKNOWNxSTR, err);
    }
    
    if (e)
      ProcessDoneEvent(e);
    else
    {
      pthread_mutex_lock(&mutex_reverseconnect);
      bool bFound = false;
      std::list<CReverseConnectToUserData *>::iterator iter;
      for (iter = m_lReverseConnect.begin(); iter != m_lReverseConnect.end();
                                             ++iter)
      {
        if ((*iter)->nId == nSubSequence)
        {
          ICQUser *u = gUserManager.FetchUser((*iter)->nUin, LOCK_R);
          if (u == NULL)
            gLog.Warn("%sReverse connection from %lu failed.\n", L_WARNxSTR,
                      (*iter)->nUin);
          else
          {
            gLog.Warn("%sReverse connection from %s failed.\n", L_WARNxSTR,
                      u->GetAlias());
            gUserManager.DropUser(u);
          }

          (*iter)->bSuccess = false;
          (*iter)->bFinished = true;
          pthread_cond_broadcast(&cond_reverseconnect_done);
          bFound = true;
          break;
        }
      }
      pthread_mutex_unlock(&mutex_reverseconnect);

      if (!bFound)
        gLog.Warn("%sICBM error for unknown event.\n", L_WARNxSTR);
    }

    break;
  }
  case ICQ_SNACxMSG_SERVERxMESSAGE:
  {
    unsigned long nMsgID[2];
    unsigned long nTimeSent;
    unsigned short mFormat, nMsgLen, nTLVs, nSubEncoding;
    char *szId;

    nMsgID[0] = packet.UnpackUnsignedLongBE();
    nMsgID[1] = packet.UnpackUnsignedLongBE();
    nTimeSent   = time(0L);
    mFormat    = packet.UnpackUnsignedShortBE();
    szId       = packet.UnpackUserString();

    //TODO Check this again with new protocol plugin support
    //if (nUin < 10000 && nUin != ICQ_UINxPAGER && nUin != ICQ_UINxSMS)
    //{
    //  gLog.Warn("%sMessage through server with strange Uin: %04lx\n", L_WARNxSTR, nUin);
    //  break;
    //}

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

      unsigned short nEncoding = msgTxt.UnpackUnsignedShortBE();
      nSubEncoding = msgTxt.UnpackUnsignedShortBE();
      
      nMsgLen -= 4;

      char* szMessage = new char[nMsgLen+1];
      for (int i = 0; i < nMsgLen; i++)
        szMessage[i] = msgTxt.UnpackChar();
      
      szMessage[nMsgLen] = '\0';
      char* szMsg = 0;
      if (nEncoding == 2) // utf-8 or utf-16?
      {
        char *szTmpMsg = 0;
        szTmpMsg = gTranslator.FromUTF16(szMessage, nMsgLen);
        szMsg = gTranslator.RNToN(szTmpMsg);
        delete [] szTmpMsg;
      }
      else
        szMsg = gTranslator.RNToN(szMessage);
      delete [] szMessage;

      // now send the message to the user
      CEventMsg *e = CEventMsg::Parse(szMsg, ICQ_CMDxRCV_SYSxMSGxONLINE, nTimeSent, 0);
      delete [] szMsg;

      // Lock the user to add the message to their queue
      ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
      if (u == NULL)
      {
        if (Ignore(IGNORE_NEWUSERS))
        {
          gLog.Info(tr("%sMessage from new user (%s), ignoring.\n"), L_SBLANKxSTR, szId);
          //TODO
          RejectEvent(strtoul(szId, (char **)NULL, 10), e);
          break;
        }

        gLog.Info(tr("%sMessage from new user (%s).\n"),
                  L_SBLANKxSTR, szId);

        AddUserToList(szId, LICQ_PPID);
        u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
      }
      else
        gLog.Info(tr("%sMessage through server from %s (%s).\n"), L_SRVxSTR,
          u->GetAlias(), szId);
    
      u->SetTyping(ICQ_TYPING_INACTIVEx0);
      
      if (AddUserEvent(u, e))
        m_xOnEventManager.Do(ON_EVENT_MSG, u);
      gUserManager.DropUser(u);
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_TYPING, szId,
                                      LICQ_PPID));
      break;
    }
    case 2: // OSCAR's "Add ICBM parameter" message
    {
      //I must admit, any server that does anything like this is a pile of shit
      CBuffer msgTxt = packet.UnpackTLV(5);
      if (msgTxt.getDataSize() == 0) break;

      unsigned short nCancel = msgTxt.UnpackUnsignedShort();

      if (nCancel == 1)  break;

      msgTxt.incDataPosRead(8); // message ids again
      
      char cap[CAP_LENGTH];
      for (unsigned short i = 0; i < CAP_LENGTH; i++)
        msgTxt >> cap[i];

      msgTxt.readTLV();

      CBuffer ackMsg = msgTxt.UnpackTLV(0x000A);
      if (ackMsg.getDataSize() == 0)  break;
      bool bIsAck = (ackMsg.UnpackUnsignedShortBE() == 2 ? true : false);

      CBuffer advMsg = msgTxt.UnpackTLV(0x2711);
      if (advMsg.getDataSize() == 0)  break;
      
      // Check if they support UTF8
      bool bUTF8 = false;
      if (memcmp(cap, ICQ_CAPABILITY_UTF8, CAP_LENGTH) == 0)
        bUTF8 = true;
        
      ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
      if (u)
      {
        u->SetSupportsUTF8(bUTF8);
        gUserManager.DropUser(u);
      }
      
      if (memcmp(cap, ICQ_CAPABILITY_DIRECT, CAP_LENGTH) == 0)
      {
        // reverse connect request
        if (advMsg.getDataSize() != 27) break;

        unsigned long nUin, nIp, nPort, nFailedPort, nPort2, nId;
        unsigned short nVersion;
        char nMode;
        advMsg >> nUin >> nIp >> nPort >> nMode >> nFailedPort >> nPort2
               >> nVersion >> nId;

        /* nPort seems to always contain the port to connect to, nPort2 is a
           duplicate of nPort. But check anyway */
        if (nPort == 0)
          nPort = nPort2;

        pthread_t t;
        CReverseConnectToUserData *data = new CReverseConnectToUserData(
                               nUin, nId, ICQ_CHNxUNKNOWN, nIp, nPort,
                               nVersion, nFailedPort, nMsgID[0], nMsgID[1]);
        pthread_create(&t, NULL, &ReverseConnectToUser_tep, data);
        break;
      }
      else if (memcmp(cap, ICQ_CAPABILITY_SRVxRELAY, CAP_LENGTH) != 0) break;

      unsigned short nLen;
      unsigned short nMsgType, nStatus, nMsgFlags;

      nLen = advMsg.UnpackUnsignedShort();
      advMsg.UnpackUnsignedShort(); // tcp version
      
      char GUID[GUID_LENGTH];
      for (unsigned short i = 0; i < GUID_LENGTH; i++) // channel
        advMsg >> GUID[i];
        
      advMsg.incDataPosRead(nLen - 2 - GUID_LENGTH - 2);
      
      unsigned short nSequence = advMsg.UnpackUnsignedShort();

      nLen = advMsg.UnpackUnsignedShort();
      advMsg.incDataPosRead(nLen);

      // Get the message type, status, and flags
      unsigned long nMask = 0;
      advMsg >> nMsgType;

      if (memcmp(GUID, PLUGIN_NORMAL, GUID_LENGTH) != 0)
      {
        unsigned char nChannel = ICQ_CHNxUNKNOWN;

        if (memcmp(GUID, PLUGIN_INFOxMANAGER, GUID_LENGTH) == 0)
          nChannel = ICQ_CHNxINFO;
        else if (memcmp(GUID, PLUGIN_STATUSxMANAGER, GUID_LENGTH) == 0)
          nChannel = ICQ_CHNxSTATUS;

        bool bNewUser = false;
        ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
        if (u == NULL)
        {
          u = new ICQUser(szId, LICQ_PPID);
          bNewUser = true;
        }

        ProcessPluginMessage(advMsg, u, nChannel, bIsAck, nMsgID[0], nMsgID[1],
                             nSequence, NULL);

        if (bNewUser)
          delete u;
        else
          gUserManager.DropUser(u);

        break;
      }
 
      advMsg >> nStatus >> nMsgFlags;

      if (nMsgType & ICQ_CMDxSUB_FxMULTIREC)
      {
        nMask |= ICQ_CMDxSUB_FxMULTIREC;
        nMsgType &= ~ICQ_CMDxSUB_FxMULTIREC;
      }

      nLen = advMsg.UnpackUnsignedShort();

      // read the message in, except for DOS \r's
      char junkChar;
      char *message = new char[nLen+1];
      unsigned short j = 0;
      for (unsigned short i = 0; i < nLen; i++)
      {
        advMsg >> junkChar;
        if (junkChar != 0x0D)  message[j++] = junkChar;
      }
      message[j] = '\0'; // ensure null terminated

      char *szTmpMsg = gTranslator.RNToN(message);
      delete [] message;

      char *szMsg = parseRTF(szTmpMsg);
      delete [] szTmpMsg;

      // Seems to be misplaced, don't do it here
      //gTranslator.ServerToClient(szMsg);

      bool bNewUser = false;
      u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
      if (u == NULL)
      {
        u = new ICQUser(szId, LICQ_PPID);
        bNewUser = true;
      }

      u->SetTyping(ICQ_TYPING_INACTIVEx0);
      
      if (msgTxt.getTLVLen(0x0004) == 4)
      {
        unsigned long Ip = msgTxt.UnpackUnsignedLongTLV(0x0004);
        rev_e_long(Ip);
        u->SetIp(PacketIpToNetworkIp(Ip));
      }

      // Special status to us?
      if (!bIsAck && !bNewUser && nStatus != ICQ_STATUS_OFFLINE &&
          !(nStatus == ICQ_STATUS_ONLINE && u->Status() == ICQ_STATUS_FREEFORCHAT) &&
          nStatus != (u->Status() | (u->StatusInvisible() ? ICQ_STATUS_FxPRIVATE : 0)))
      {
        bool r = u->OfflineOnDisconnect() || u->StatusOffline();
        ChangeUserStatus(u, (u->StatusFull() & ICQ_STATUS_FxFLAGS) | nStatus);
        gLog.Info(tr("%s%s (%s) is %s to us.\n"), L_TCPxSTR, u->GetAlias(),
          u->IdString(), u->StatusStr());
        if (r) u->SetOfflineOnDisconnect(true);
      }

      if (u->Version() == 0x0A)
      {
        // We removed the conversion from before, but with this version we need
        // it back. Go figure.
        gTranslator.ClientToServer(szMsg);
      }

      if (!bNewUser)
        gUserManager.DropUser(u);

      // Handle it
      ProcessMessage(u, advMsg, szMsg, nMsgType, nMask, nMsgID,
                     nSequence, bIsAck, bNewUser);

      delete [] szMsg;
      if (bNewUser) // can be changed in ProcessMessage
      {
        delete u;
        break;
      }

      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_TYPING, szId,
                                      LICQ_PPID));
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
            gLog.Unknown(tr("%sReceived SMS receipt indicating success.\n"), L_UNKNOWNxSTR);
            return;
          case 0x0003:
            // SMS Receipt : Failure
            gLog.Unknown(tr("%sReceived SMS receipt indicating failure.\n"), L_UNKNOWNxSTR);
            return;
          default:
          {
            char *buf;
            gLog.Unknown("%sUnknown SMS subtype (0x%04x):\n%s\n", L_UNKNOWNxSTR,
              nTypeSMS, packet.print(buf));
            delete [] buf;
            return;
          }
        }

        unsigned long nTagLength = msgTxt.UnpackUnsignedLong();
        // Refuse irreasonable tag sizes
        if (nTagLength > 255)
        {
          gLog.Unknown(tr("%sInvalid tag in SMS message."), L_UNKNOWNxSTR);
          return;
        }

        char* szTag = new char[nTagLength + 1];
        for (unsigned long i = 0; i < nTagLength; ++i)
          szTag[i] = msgTxt.UnpackChar();
        szTag[nTagLength] = '\0';

        if (strcmp(szTag, "ICQSMS") != 0)
        {
          gLog.Unknown(tr("%sUnknown tag in SMS message:\n%s\n"), L_UNKNOWNxSTR,
            szTag);
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
          gLog.Unknown(tr("%sSMS message packet was too large (claimed size: %lu bytes)\n"),
            L_UNKNOWNxSTR, nSMSLength);
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
        char* szMsg = new char[nMsgLen+1];
        for (int i = 0; i < nMsgLen; ++i)
          szMsg[i] = msgTxt.UnpackChar();
        szMsg[nMsgLen] = '\0';

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
          szType = strdup(tr("Message"));
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
            gLog.Warn(tr("%sInvalid URL message:\n%s\n"), L_WARNxSTR, packet.print(buf));
            delete [] buf;
            break;
          }
          szType = strdup(tr("URL"));
          nTypeEvent = ON_EVENT_URL;
          eEvent = e;
          break;
        }
        case ICQ_CMDxSUB_AUTHxREQUEST:
        {
          gLog.Info(tr("%sAuthorization request from %s.\n"), L_SBLANKxSTR, szId);
          char **szFields = new char *[6];  // alias, first name, last name, email, auth, comment

          if (!ParseFE(szMessage, &szFields, 6))
          {
            char *buf;
            gLog.Warn(tr("%sInvalid authorization request system message:\n%s\n"), L_WARNxSTR,
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

          CEventAuthRequest *e = new CEventAuthRequest(szId, LICQ_PPID,
                                                       szFields[0], szFields[1],
                                                       szFields[2], szFields[3],
                                                       szFields[5],
                                                       ICQ_CMDxRCV_SYSxMSGxONLINE,
                                                       nTimeSent, 0);

          delete [] szFields;
          eEvent = e;
          break;
        }
        case ICQ_CMDxSUB_AUTHxREFUSED:  // system message: authorization refused
        {
          gLog.Info(tr("%sAuthorization refused by %s.\n"), L_SBLANKxSTR, szId);

          // Translating string with Translation Table
          gTranslator.ServerToClient(szMessage);

          CEventAuthRefused *e = new CEventAuthRefused(szId, LICQ_PPID, szMessage,
                                                       ICQ_CMDxRCV_SYSxMSGxONLINE, 
                                                       nTimeSent, 0);
          eEvent = e;
          break;
        }
        case ICQ_CMDxSUB_AUTHxGRANTED:  // system message: authorized
        {
          gLog.Info(tr("%sAuthorization granted by %s.\n"), L_SBLANKxSTR, szId);

          // translating string with Translation Table
          gTranslator.ServerToClient (szMessage);

          ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
          if (u)
          {
            u->SetAwaitingAuth(false);
            gUserManager.DropUser(u);
          }

          CEventAuthGranted *e = new CEventAuthGranted(szId, LICQ_PPID,
            szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, nTimeSent, 0);
          eEvent = e;
          break;
        }
        case ICQ_CMDxSUB_MSGxSERVER:
        {
          gLog.Info(tr("%sServer message.\n"), L_BLANKxSTR);

          CEventServerMessage *e = CEventServerMessage::Parse(szMessage,
            ICQ_CMDxSUB_MSGxSERVER, nTimeSent, nMask);
          if (e == NULL)
          {
            char *buf;

            gLog.Warn(tr("%sInvalid Server Message:\n%s\n"), L_WARNxSTR,
              packet.print(buf));
            delete [] buf;
            break;
          }
          eEvent = e;
          break;
        }
        case ICQ_CMDxSUB_ADDEDxTOxLIST: // system message: added to a contact list
        {
          gLog.Info(tr("%sUser %s added you to their contact list.\n"), L_SBLANKxSTR, szId);

          char **szFields = new char*[6]; // alias, first name, last name, email, auth, comment

          if (!ParseFE(szMessage, &szFields, 6))
          {
            char *buf;
            gLog.Warn(tr("%sInvalid added to list system message:\n%s\n"), L_WARNxSTR,
                      packet.print(buf));
            delete [] buf;
            delete [] szFields;
            break;
          }

          // translating string with Translation Table
          gTranslator.ServerToClient (szFields[0]);  // alias
          gTranslator.ServerToClient (szFields[1]);  // first name
          gTranslator.ServerToClient (szFields[2]);  // last name

          CEventAdded *e = new CEventAdded(szId, LICQ_PPID, szFields[0],
            szFields[1], szFields[2], szFields[3], ICQ_CMDxRCV_SYSxMSGxONLINE,
            nTimeSent, 0);
          delete [] szFields;
          eEvent = e;
          break;
        }
        case ICQ_CMDxSUB_WEBxPANEL:
        {
          gLog.Info(tr("%sMessage through web panel.\n"), L_SBLANKxSTR);

          char **szFields = new char *[6];	// name, ?, ?, email, ?, message

          if (!ParseFE(szMessage, &szFields, 6))
          {
            char *buf;
            gLog.Warn(tr("%sInvalid web panel system message:\n%s\n"), L_WARNxSTR,
              packet.print(buf));
            delete [] buf;
            delete [] szFields;
            break;
          }

          // translating string with Translation Table
          gTranslator.ServerToClient(szFields[0]);  // name
          gTranslator.ServerToClient(szFields[3]);  // email
          gTranslator.ServerToClient(szFields[5]);  // message

          gLog.Info(tr("%sFrom %s (%s).\n"), L_SBLANKxSTR, szFields[0], szFields[3]);
          CEventWebPanel *e = new CEventWebPanel(szFields[0], szFields[3],
                                                 szFields[5], ICQ_CMDxRCV_SYSxMSGxONLINE,
                                                 nTimeSent, 0);
          delete [] szFields;
          eEvent = e;
          break;
        }
        case ICQ_CMDxSUB_EMAILxPAGER:
        {
          gLog.Info(tr("%sEmail pager message.\n"), L_SBLANKxSTR);

          char **szFields = new char *[6];	// name, ?, ?, email, ?, message

          if (!ParseFE(szMessage, &szFields, 6))
          {
            char *buf;
            gLog.Warn(tr("%sInvalid email pager system message:\n%s\n"), L_WARNxSTR,
                      packet.print(buf));
            delete [] buf;
            delete [] szFields;
            break;
          }

          // translating string with Translation Table
          gTranslator.ServerToClient(szFields[0]);  // name
          gTranslator.ServerToClient(szFields[3]);  // email
          gTranslator.ServerToClient(szFields[5]);  // message

          gLog.Info(tr("%sFrom %s (%s).\n"), L_SBLANKxSTR, szFields[0], szFields[3]);
          CEventEmailPager *e = new CEventEmailPager(szFields[0], szFields[3],
                                                     szFields[5], ICQ_CMDxRCV_SYSxMSGxONLINE,
                                                     nTimeSent, 0);
          delete [] szFields;
          eEvent = e;
          break;
        }
        case ICQ_CMDxSUB_CONTACTxLIST:
        {
          CEventContactList *e = CEventContactList::Parse(szMessage,
            ICQ_CMDxRCV_SYSxMSGxONLINE,nTimeSent, nMask);
          if (e == NULL)
          {
            char *buf;
            gLog.Warn(tr("%sInvalid Contact List message:\n%s\n"), L_WARNxSTR,
              packet.print(buf));
            delete [] buf;
            break;
          }

          szType = strdup(tr("Contacts"));
          nTypeEvent = ON_EVENT_MSG;
          eEvent = e;
          break;
        }
        case ICQ_CMDxSUB_SMS:
        {
          CEventSms *e = CEventSms::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE,
            nTimeSent, nMask);
          if (e == NULL)
          {
            char *buf;
            gLog.Warn(tr("%sInvalid SMS message:\n%s\n"), L_WARNxSTR,
              packet.print(buf));
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

          gLog.Unknown(tr("%sUnknown system message (0x%04x):\n%s\n"), L_UNKNOWNxSTR,
                       nTypeMsg, packet.print(buf));
          delete [] buf;
          //TODO
          CEventUnknownSysMsg *e = new CEventUnknownSysMsg(nTypeMsg, ICQ_CMDxRCV_SYSxMSGxONLINE,
                                                           strtoul(szId, (char **)NULL, 10), 
                                                           szMessage, nTimeSent, 0);

          ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
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
	    ICQUser* u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
	    if (u == NULL)
	    {
	      if (Ignore(IGNORE_NEWUSERS))
	      {
          gLog.Info(tr("%s%s from new user (%s), ignoring.\n"), L_SBLANKxSTR, szType, szId);

          if (szType) free(szType);

          //TODO
          RejectEvent(strtoul(szId, (char **)NULL, 10), eEvent);
          break;
        }

	      gLog.Info(tr("%s%s from new user (%s).\n"), L_SBLANKxSTR, szType, szId);
	      AddUserToList(szId, LICQ_PPID);
	      u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
	    }
	    else
	      gLog.Info(tr("%s%s through server from %s (%s).\n"), L_SBLANKxSTR,
	    					szType, u->GetAlias(), u->IdString());

            u->SetTyping(ICQ_TYPING_INACTIVEx0);
            
	    if (szType) free(szType);
	    if (AddUserEvent(u, eEvent))
	      m_xOnEventManager.Do(nTypeEvent, u);
	    gUserManager.DropUser(u);
	    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_TYPING, szId,
                                            LICQ_PPID));
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
            ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_R);
            bool bIgnore = (u && u->IgnoreList());
            gUserManager.DropUser(u);

            if (bIgnore)
            {
              delete eEvent; // Processing stops here, needs to be deleted
              break;
            }

	    ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
	    if (AddUserEvent(o, eEvent))
	    {
	      gUserManager.DropOwner(LICQ_PPID);
	      eEvent->AddToHistory(NULL, LICQ_PPID, D_RECEIVER);
	      m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
	    }
	    else
	      gUserManager.DropOwner(LICQ_PPID);
	    break;
	  }
	  case ICQ_CMDxSUB_SMS:
	  {
	    CEventSms *eSms = (CEventSms *)eEvent;
      //TODO
	    unsigned long nUinSms = FindUinByCellular(eSms->Number());

	    if (nUinSms != 0)
	    {
        //TODO
	      ICQUser* u = gUserManager.FetchUser(nUinSms, LOCK_W);
	      gLog.Info(tr("%sSMS from %s - %s (%lu).\n"), L_SBLANKxSTR, eSms->Number(), 
                  u->GetAlias(), nUinSms);
	      if (AddUserEvent(u, eEvent))
	        m_xOnEventManager.Do(ON_EVENT_SMS, u);
	      gUserManager.DropUser(u);
	    }
	    else
	    {
	      ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
	      gLog.Info(tr("%sSMS from %s.\n"), L_BLANKxSTR, eSms->Number());
	      if (AddUserEvent(o, eEvent))
	      {
	        gUserManager.DropOwner();
	        eEvent->AddToHistory(NULL, LICQ_PPID, D_RECEIVER);
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

      gLog.Unknown(tr("%sMessage through server with unknown format: %04hx\n%s\n"),
				    L_ERRORxSTR, mFormat, packet.print(buf));
      delete [] buf;
      break;
    }
    delete [] szId;
    break;
  }
  case ICQ_SNACxMSG_SERVERxREPLYxMSG:
  {
		unsigned short nFormat, nLen, nSequence, nMsgType, nAckFlags, nMsgFlags;
		unsigned long nUin, nMsgID;
                CExtendedAck *pExtendedAck = 0;
		ICQUser *u = NULL;

	 	packet.incDataPosRead(4); // msg id
		nMsgID = packet.UnpackUnsignedLongBE(); // lower bits, what licq uses
		nFormat = packet.UnpackUnsignedShortBE();
		nUin = packet.UnpackUinString();

		u = gUserManager.FetchUser(nUin, LOCK_W);
		if (u == NULL)
		{
			gLog.Warn(tr("%sUnexpected new user in subtype 0x%04x.\n"), L_SRVxSTR,
								nSubtype);
			break;
		}

    pthread_mutex_lock(&mutex_reverseconnect);
    std::list<CReverseConnectToUserData *>::iterator iter;
    bool bFound = false;
    for (iter = m_lReverseConnect.begin(); iter != m_lReverseConnect.end();
                                           ++iter)
    {
      if ((*iter)->nId == nMsgID && (*iter)->nUin == nUin)
      {
        gLog.Warn("%sReverse connection from %s failed.\n", L_WARNxSTR,
                  u->GetAlias());
        (*iter)->bSuccess = false;
        (*iter)->bFinished = true;
        bFound = true;
        break;
      }
    }
    pthread_mutex_unlock(&mutex_reverseconnect);
    
    int nSubResult;
    if (bFound)
    {
      nSubResult = ICQ_TCPxACK_REFUSE;
      pExtendedAck = NULL;
      pthread_cond_broadcast(&cond_reverseconnect_done);
      gUserManager.DropUser(u);
      return;
    }

    packet.incDataPosRead(2);
    packet >> nLen;
    if (nLen == 0x0200)
    {
      gLog.Warn(tr("%s%s doesn't have a manager for this event.\n"), L_WARNxSTR,
        u->GetAlias());
      gUserManager.DropUser(u);  
      
      ICQEvent *e = DoneServerEvent(nMsgID, EVENT_ERROR);
      if (e)
      {
        ProcessDoneEvent(e);
        return;
      }
      else
      {
        gLog.Warn(tr("%sAck for unknown event.\n"), L_SRVxSTR);
        break;
      }
    }
    
    packet.UnpackUnsignedShort(); //tcp version
    
    char GUID[GUID_LENGTH];
    for (unsigned short i = 0; i < GUID_LENGTH; i++)
      packet >> GUID[i];
    
    packet.incDataPosRead(nLen - GUID_LENGTH - 2);
    packet >> nLen;
    nSequence = packet.UnpackUnsignedShortBE();
    packet.incDataPosRead(nLen - 2);
    packet >> nMsgType;
    
    if (memcmp(GUID, PLUGIN_NORMAL, GUID_LENGTH) != 0)
    {
      unsigned char nChannel = ICQ_CHNxUNKNOWN;
      if (memcmp(GUID, PLUGIN_INFOxMANAGER, GUID_LENGTH) == 0)
        nChannel = ICQ_CHNxINFO;
      else if (memcmp(GUID, PLUGIN_STATUSxMANAGER, GUID_LENGTH) == 0)
        nChannel = ICQ_CHNxSTATUS;
        
      ProcessPluginMessage(packet, u, nChannel, true, 0, nMsgID, nSequence, 0);
      
      gUserManager.DropUser(u);
      break;
    }
    
    packet >> nAckFlags >> nMsgFlags >> nLen;
    
    char szMessage[nLen + 1];
    for (unsigned short i = 0; i < nLen; i++)
      packet >> szMessage[i];
    szMessage[nLen] = '\0';
    gTranslator.ServerToClient(szMessage);
    
    if (nAckFlags == ICQ_TCPxACK_REFUSE)
    {
      pExtendedAck = new CExtendedAck(false, 0, szMessage);
      nSubResult = ICQ_TCPxACK_REFUSE;
      gLog.Info(tr("%sRefusal from %s (#%lu).\n"), L_SRVxSTR, u->GetAlias(),
        nMsgID);
    }
    else
    {
      // Update the away message if it's changed
      if (strcmp(u->AutoResponse(), szMessage))
      {
        u->SetAutoResponse(szMessage);
        u->SetShowAwayMsg(*szMessage);
        gLog.Info(tr("%sAuto response from %s (#%lu).\n"), L_SRVxSTR,
          u->GetAlias(), nMsgID);
      }

      if (nMsgType != ICQ_CMDxTCP_READxAWAYxMSG &&
          nMsgType != ICQ_CMDxTCP_READxOCCUPIEDxMSG &&
          nMsgType != ICQ_CMDxTCP_READxNAxMSG &&
          nMsgType != ICQ_CMDxTCP_READxDNDxMSG &&
          nMsgType != ICQ_CMDxTCP_READxFFCxMSG &&
          nAckFlags != ICQ_TCPxACK_ACCEPT &&
          nAckFlags != ICQ_TCPxACK_OCCUPIED &&
          nAckFlags != ICQ_TCPxACK_DND)
      {
        nSubResult = ICQ_TCPxACK_REFUSE;
      }
      else
      {
        nSubResult = ICQ_TCPxACK_RETURN;
      }

      pExtendedAck = new CExtendedAck(nSubResult == ICQ_TCPxACK_RETURN, 0,
                                      szMessage);
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
      gLog.Warn(tr("%sAck for unknown event.\n"), L_SRVxSTR);
      break;
    }

		break;
	}
	case ICQ_SNACxMSG_RIGHTSxGRANTED:
	{
		gLog.Info(tr("%sReceived rights for Instant Messaging..\n"), L_SRVxSTR);
		break;
	}
  case ICQ_SNACxMSG_SERVERxACK:
  {
    ICQEvent *e = DoneServerEvent(nSubSequence, EVENT_ACKED);
    if (e)
    {
      e->m_nSubResult = ICQ_TCPxACK_ACCEPT;
      ProcessDoneEvent(e);
    }
      
    gLog.Info(tr("%sMessage was sent to an offline user. It will be delivered"
                " when the user logs on.\n"), L_SRVxSTR);

    break;
  }
  case ICQ_SNACxMSG_TYPING:
  {
    packet.UnpackUnsignedLongBE(); // timestamp
    packet.UnpackUnsignedLongBE(); // message id
    packet.UnpackUnsignedShortBE(); // format (only seen 1)
    const char *szId = packet.UnpackUserString();
    unsigned short nTyping = packet.UnpackUnsignedShortBE();

    ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
    if (u == NULL)
    {
      gLog.Warn(tr("%sTyping status received for unknown user (%s).\n"),
        L_WARNxSTR, szId);
      break;
    }
    u->SetTyping(nTyping);
    gUserManager.DropUser(u);
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_TYPING, szId,
                                    LICQ_PPID));
    delete [] szId;
    break;
  }
	default:
	{
		gLog.Warn(tr("%sUnknown Message Family Subtype: %04hx\n"), L_SRVxSTR, nSubtype);
		break;
	}
	}
}

//--------ProcessListFam--------------------------------------------
void CICQDaemon::ProcessListFam(CBuffer &packet, unsigned short nSubtype)
{
  unsigned short nFlags = packet.UnpackUnsignedShortBE();
  unsigned long nSubSequence = packet.UnpackUnsignedLongBE();

  // First 8 bytes - unknown
  if (nSubtype != ICQ_SNACxLIST_ROSTxREPLY)
  {
    packet.UnpackUnsignedLong();
    packet.UnpackUnsignedLong();
  } 

  switch (nSubtype)
  {
    case ICQ_SNACxLIST_RIGHTSxGRANTED:
    {
      gLog.Info(tr("%sServer granted contact list rights.\n"), L_SRVxSTR);
      DoneServerEvent(nSubSequence, EVENT_SUCCESS);

      break;
    }

    case ICQ_SNACxLIST_ROSTxREPLY:
    {
      static unsigned short nCount;
      static bool sCheckExport = false;
      unsigned short nPacketCount;
      unsigned long nTime;

      if (nFlags & 0x0001)
      {
        if (!hasServerEvent(nSubSequence))
          gLog.Warn(tr("%sContact list without request.\n"), L_SRVxSTR);
        else
          gLog.Info(tr("%sReceived contact list.\n"), L_SRVxSTR);
      }
      else
      {
        // This is the last packet so mark it as done
        ICQEvent *e = DoneServerEvent(nSubSequence, EVENT_SUCCESS);

        if (e == NULL)
        {
          gLog.Warn(tr("%sContact list without request.\n"), L_SRVxSTR);
          break;
        }

        /* This isn't used anymore. At least with SSI Version 0.
        if (e->SNAC() == MAKESNAC(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_REQUESTxRIGHTS) ||
            e->SNAC() == MAKESNAC(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_REQUESTxROST))
        {
          packet.UnpackUnsignedLong();
          packet.UnpackUnsignedLong();
        }
        */

        gLog.Info(tr("%sReceived end of contact list.\n"), L_SRVxSTR);
      }

      m_bOnlineNotifies = true;

      packet.UnpackChar();  // SSI Version
      nPacketCount = packet.UnpackUnsignedShortBE();
      nCount += nPacketCount;

      for (unsigned short i = 0; i < nPacketCount; i++)
      {
        char *szId;
        unsigned short nTag, nID, nType, nByteLen;

        // Can't use UnpackUserString because this may be a group name
        szId = packet.UnpackStringBE();
        nTag = packet.UnpackUnsignedShortBE();
        nID = packet.UnpackUnsignedShortBE();
        nType = packet.UnpackUnsignedShortBE();
        nByteLen = packet.UnpackUnsignedShortBE();

        char *szUnicodeName = gTranslator.FromUnicode(szId);

        if (nByteLen)
        {
          if (!packet.readTLV(-1, nByteLen))
          {
            gLog.Error("%sUnable to parse contact list TLV, aborting!\n",
                L_ERRORxSTR);
            delete[] szId;
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
            if (!UseServerContactList()) break; 
            
            char *szNewName = packet.UnpackStringTLV(0x0131);
            char *szSMSNumber = packet.UnpackStringTLV(0x013A);
            bool bAwaitingAuth = packet.hasTLV(0x0066);

            bool isOnList = true;
            if (szId && !gUserManager.IsOnList(szId, LICQ_PPID))
            {
              isOnList = false;
              AddUserToList(szId, LICQ_PPID, false); // Don't notify server
            }

            char *szUnicodeAlias = szNewName ? strdup(szNewName) : 0;

            ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
            if (u)
            {
              u->SetGSID(nTag);

              if (szNewName)
              {
                if (szUnicodeAlias)
                {
                  u->SetAlias(szUnicodeAlias);
                }
              }

              if (szSMSNumber)
              {
                char *szUnicodeSMS = gTranslator.FromUnicode(szSMSNumber);
                if (szUnicodeSMS)
                {
                  u->SetCellularNumber(szUnicodeSMS);
                  delete [] szUnicodeSMS;
                }
              }

              u->SetAwaitingAuth(bAwaitingAuth);

              if (nType == ICQ_ROSTxINVISIBLE)
              {
                u->SetInvisibleList(true);
                u->SetInvisibleSID(nID);
              }
              else if (nType == ICQ_ROSTxVISIBLE)
              {
                u->SetVisibleList(true);
                u->SetVisibleSID(nID);
              }
              else
              {
                u->SetSID(nID);

                if (nType == ICQ_ROSTxNORMAL)
                {
                  // Save the group that they are in
                  u->AddToGroup(GROUPS_USER, gUserManager.GetGroupFromID(nTag));
                }
              }
              
              u->SetIgnoreList(nType == ICQ_ROSTxIGNORE);
              
              if (!isOnList)
              {
                // They aren't a new user if we added them to a server list
                u->SetNewUser(false);
              }

              PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_GENERAL,
                u->IdString(), u->PPID()));
              gUserManager.DropUser(u);
            }

            if (!isOnList)
            {
              gLog.Info(tr("%sAdded %s (%s) to list from server.\n"), L_SRVxSTR,
                (szUnicodeAlias ? szUnicodeAlias : ""), szId);
            }

            if (szUnicodeAlias)
              free(szUnicodeAlias);
            if (szNewName)
              delete [] szNewName;
            if (szSMSNumber)
              delete [] szSMSNumber;

            break;
          }

          case ICQ_ROSTxGROUP:
          {
            if (!UseServerContactList()) break; 
                      
            if (szId[0] != '\0' && nTag != 0)
            {
              // Rename the group if we have it already or else add it
              unsigned short nGroup = gUserManager.GetGroupFromID(nTag);
              if (nGroup == gUserManager.NumGroups())
              {
                if (!gUserManager.AddGroup(szUnicodeName, nTag))
                  gUserManager.ModifyGroupID(szUnicodeName, nTag);
              }
              else
              {
                gUserManager.RenameGroup(nGroup, szUnicodeName, false);
              }
              
              // This is bad, i don't think we want to call this at all..
              // it will add users to different groups that they werent even
              // assigned to
              //if (gUserManager.UpdateUsersInGroups())
              //{
              //  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST,
              //    LIST_ALL, 0));
              //}
            }
            else
            {
              gLog.Info(tr("%sGot Master Group record.\n"), L_SRVxSTR);
            }

            break;
          }

          case ICQ_ROSTxPDINFO:
          {
            unsigned char cPrivacySettings = packet.UnpackCharTLV(0x00CA);

            ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
            gLog.Info(tr("%sGot Privacy Setting.\n"), L_SRVxSTR);
            o->SetPDINFO(nID);
            if (cPrivacySettings == ICQ_PRIVACY_ALLOW_FOLLOWING)
              ChangeUserStatus(o, o->StatusFull() | ICQ_STATUS_FxPRIVATE);
            gUserManager.DropOwner(LICQ_PPID);
            break;
          }
        }  // switch (nType)

        if (szUnicodeName)
          delete [] szUnicodeName;

        packet.cleanupTLV();
        if (szId)
          delete[] szId;
      } // for count

      // First time we get this packet, check to upload our local list
      if (sCheckExport == false)
      {
        sCheckExport = true;
        CheckExport();
      }
      
      // Update local info about contact list
      nTime = packet.UnpackUnsignedLongBE();
      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
      o->SetSSTime(nTime);
      o->SetSSCount(nCount);
      gUserManager.DropOwner();

      gLog.Info(tr("%sActivate server contact list.\n"), L_SRVxSTR);
      CSrvPacketTcp *p = new CPU_GenericFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxACK);
      SendEvent_Server(p);

      
      break;
    } // case rost reply

    case ICQ_SNACxLIST_ROSTxSYNCED:
    {
      DoneServerEvent(nSubSequence, EVENT_SUCCESS);

      gLog.Info(tr("%sContact list is synchronized.\n"), L_SRVxSTR);
      // The server says we are up to date, let's double check
      CheckExport();

      gLog.Info(tr("%sActivate server contact list.\n"), L_SRVxSTR);
      CSrvPacketTcp *p = new CPU_GenericFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxACK);
      SendEvent_Server(p);

      break;
    }

    case ICQ_SNACxLIST_UPDxACK:
    {
      if (!UseServerContactList()) break;
      
      ICQEvent *e = DoneServerEvent(nSubSequence, EVENT_SUCCESS);

      if (e == NULL)
      {
        gLog.Warn(tr("%sServer list update ack without request.\n"), L_SRVxSTR);
        break;
      }

      ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      unsigned long nListTime = o->GetSSTime();
      o->SetSSTime(time(0));
      gUserManager.DropOwner(LICQ_PPID);

      CSrvPacketTcp *pReply = 0;
      bool bHandled = false;
      unsigned short nError = 0;

      do
      {
        pthread_mutex_lock(&mutex_modifyserverusers);
        std::map<unsigned long, std::string>::iterator mapIter = m_lszModifyServerUsers.find(nSubSequence);
        pthread_mutex_unlock(&mutex_modifyserverusers);

        if (mapIter == m_lszModifyServerUsers.end())
        {
          gLog.Info(tr("%sSkipping invalid server side list ack (%ld).\n"),
              L_SRVxSTR, nSubSequence);
          break;
        }
        std::string pending = mapIter->second;

        nError = packet.UnpackUnsignedShortBE();

        switch (nError)
        {
          case 0: break; // No error

          case 0x0E:
            gLog.Info(tr("%s%s added to awaiting authorization group on server list.\n"),
                       L_SRVxSTR, pending.c_str());
            break;

          case 0x02:
            gLog.Warn(tr("%sUser/Group %s not found on server list.\n"),
                       L_WARNxSTR, pending.c_str());
            break;

          default:
            gLog.Warn(tr("%sUnknown error modifying server list: 0x%02X (ID: %s)\n"),
                         L_ERRORxSTR, nError & 0xFF, pending.c_str());
        }

        if (nError && nError != 0x0E)
          break;

        switch (e->SubType())
        {
          case ICQ_SNACxLIST_ROSTxADD:
          case ICQ_SNACxLIST_ROSTxREM:
          {
            if (nError == 0x0E && e->SubType() == ICQ_SNACxLIST_ROSTxADD)
            {
              if (pending.length())
                icqAddUserServer(pending.c_str(), true);
              break;
            }

            bool bTopLevelUpdated = false;

            if (bHandled == false)
            {
              bHandled = true;
              int n = gUserManager.GetGroupFromID(e->ExtraInfo());
              if (n < 1 && e->ExtraInfo() != 0)
                break;


              GroupList *g = gUserManager.LockGroupList(LOCK_R);
              std::string groupName;
              if (e->ExtraInfo() == 0)
                groupName = ""; // top level
              else
                groupName = (*g)[n-1];
              gUserManager.UnlockGroupList();

              // Start editing server list
              CSrvPacketTcp *pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
                                                            ICQ_SNACxLIST_ROSTxEDITxSTART);
              SendEvent_Server(pStart);

              if (e->ExtraInfo() == 0)
              {
                bTopLevelUpdated = true;
                if (nListTime == 0)
                  pReply = new CPU_AddToServerList("", ICQ_ROSTxGROUP, 0, false, true);
                else
                  pReply = new CPU_UpdateToServerList("", ICQ_ROSTxGROUP, 0);
                gLog.Info(tr("%sUpdating top level group.\n"), L_SRVxSTR);
              }
              else
              {
                pReply = new CPU_UpdateToServerList(groupName.c_str(), ICQ_ROSTxGROUP,
                                                    e->ExtraInfo());
                gLog.Info(tr("%sUpdating group %s.\n"), L_SRVxSTR, groupName.c_str());
              }
              addToModifyUsers(pReply->SubSequence(), groupName);
              SendExpectEvent_Server(0, pReply, NULL);

              // Finish editing server list
              CSrvPacketTcp *pEnd = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
                                                          ICQ_SNACxLIST_ROSTxEDITxEND);
              SendEvent_Server(pEnd);
            }


            // Skip the call to gUserManager.AddUserToGroup because
            // that will send a message out to the server AGAIN
            if (e->SubType() == ICQ_SNACxLIST_ROSTxADD && !bTopLevelUpdated)
            {
              ICQUser *u = gUserManager.FetchUser(pending.c_str(), LICQ_PPID, LOCK_R);
              if (u)
              {
                u->AddToGroup(GROUPS_USER, gUserManager.GetGroupFromID(
                              e->ExtraInfo()));
                gUserManager.DropUser(u);
                PushPluginSignal(new CICQSignal(SIGNAL_ADDxSERVERxLIST, 0,
                                                pending.c_str(), LICQ_PPID));
              }
            }

            break;
          }

          case ICQ_SNACxLIST_ROSTxUPD_GROUP:
          {
            if (bHandled == false)
            {
              bHandled = true;
              gLog.Info(tr("%sUpdated %s successfully.\n"), L_SRVxSTR,
                        pending.c_str());
                
              if (nError == 0x0E)
              {
                pReply = new CPU_UpdateToServerList(pending.c_str(),
                  ICQ_ROSTxNORMAL, 0, true);
                addToModifyUsers(pReply->SubSequence(), pending);
                SendExpectEvent_Server(0, pReply, NULL);
              }
            }

            break;
          }
        }

        // Remove it from the map now that we will process it
        m_lszModifyServerUsers.erase(mapIter);
      } while (!packet.End());

      break;
    }

    case ICQ_SNACxLIST_AUTHxREQxSRV:
    {
      char *szId = packet.UnpackUserString();
      ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_R);
      bool bIgnore = (u && u->IgnoreList());
      gUserManager.DropUser(u);

      if (bIgnore)
      {
        delete [] szId;
        break;
      }

      gLog.Info(tr("%sAuthorization request from %s.\n"), L_SRVxSTR, szId);

      unsigned short nMsgLen;
      packet >> nMsgLen;
      char *szMsg = new char[nMsgLen+1];
      for (int i = 0; i < nMsgLen; i++)
        packet >> szMsg[i];
      szMsg[nMsgLen] = '\0';
      
      CEventAuthRequest *e = new CEventAuthRequest(szId, LICQ_PPID, "", "", "", "", nMsgLen ? szMsg : "",
                                                   ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);

      ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      if (AddUserEvent(o, e))
      {
        gUserManager.DropOwner(LICQ_PPID);
        e->AddToHistory(NULL, LICQ_PPID, D_RECEIVER);
        m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
      }
      else
        gUserManager.DropOwner(LICQ_PPID);
      
      delete [] szId;
      delete [] szMsg;
      break;
    }

    case ICQ_SNACxLIST_AUTHxRESPONS: // The resonse to our authorization request
    {
      char *szId = packet.UnpackUserString();
      unsigned char granted;

      packet >> granted;
      unsigned short nMsgLen;
      packet >> nMsgLen;
      char *szMsg = new char[nMsgLen+1];
      for (int i = 0; i < nMsgLen; i++)
       packet >> szMsg[i];
      szMsg[nMsgLen] = '\0';

      gLog.Info(tr("%sAuthorization %s by %s.\n"), L_SRVxSTR,
         granted ? "granted" : "refused", szId);

      CUserEvent *eEvent;
      if (granted)
      {
         eEvent = new CEventAuthGranted(szId, LICQ_PPID, szMsg,
           ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);

         ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
         if (u)
         {
           u->SetAwaitingAuth(false);
           gUserManager.DropUser(u);
         }
      }
      else
      {
        eEvent = new CEventAuthRefused(szId, LICQ_PPID, szMsg,
            ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);
      }

      ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      if (AddUserEvent(o, eEvent))
      {
        gUserManager.DropOwner(LICQ_PPID);
        eEvent->AddToHistory(NULL, LICQ_PPID, D_RECEIVER);
        m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
      }
      else
        gUserManager.DropOwner(LICQ_PPID);

      delete [] szId;
      delete [] szMsg;
      break;
    }

    case ICQ_SNACxLIST_AUTHxADDED: // You were added to a contact list
    {
      char *szId = packet.UnpackUserString();
      gLog.Info(tr("%sUser %s added you to their contact list.\n"), L_SRVxSTR,
                szId);

      CEventAdded *e = new CEventAdded(szId, LICQ_PPID, "", "", "", "",
                                       ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);
      ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      if (AddUserEvent(o, e))
      {
        gUserManager.DropOwner(LICQ_PPID);
        e->AddToHistory(NULL, LICQ_PPID, D_RECEIVER);
        m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
      }
      else
        gUserManager.DropOwner(LICQ_PPID);

      delete [] szId;
      break;
    }
    
    default:
      gLog.Warn(tr("%sUnknown List Family Subtype: %04hx\n"), L_SRVxSTR, nSubtype);
      break;
  } // switch subtype
}

//--------ProcessBosFam---------------------------------------------
void CICQDaemon::ProcessBOSFam(CBuffer& /* packet */, unsigned short nSubtype)
{
  switch (nSubtype)
  {
  case ICQ_SNACxBOS_RIGHTSxGRANTED:
  {
    gLog.Info(tr("%sReceived BOS rights.\n"), L_SRVxSTR);

    icqSetStatus(m_nDesiredStatus);

    gLog.Info(tr("%sSending client ready...\n"), L_SRVxSTR);
    CSrvPacketTcp *p = new CPU_ClientReady();
    SendEvent_Server(p);

    gLog.Info(tr("%sSending offline message request...\n"), L_SRVxSTR);
    p = new CPU_RequestSysMsg;
    SendEvent_Server(p);

    m_eStatus = STATUS_ONLINE;
    m_bLoggingOn = false;
    // ### FIX subsequence !!
    ICQEvent *e = DoneExtendedServerEvent(0, EVENT_SUCCESS);
    if (e != NULL) ProcessDoneEvent(e);
    PushPluginSignal(new CICQSignal(SIGNAL_LOGON, 0, 0));

    //icqSetStatus(m_nDesiredStatus);
    break;
  }
  default:
    gLog.Warn(tr("%sUnknown BOS Family Subtype: %04hx\n"), L_SRVxSTR, nSubtype);
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
      gLog.Unknown(tr("%sUnknown server response:\n%s\n"), L_UNKNOWNxSTR,
         packet.print(buf));
      delete [] buf;
      break;
    }

    CBuffer msg = packet.UnpackTLV(0x0001);

    if (msg.Empty()) {
      char *buf;
      gLog.Unknown(tr("%sUnknown server response:\n%s\n"), L_UNKNOWNxSTR,
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

      ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
      nTimeSent = mktime(&sendTM) - o->SystemTimeGMTOffset();
      gUserManager.DropOwner();
      
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
	  szType = strdup(tr("Message"));
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
	    
	    gLog.Warn(tr("%sInvalid offline URL message:\n%s\n"), L_WARNxSTR, packet.print(buf));
	    delete [] buf;
	    break;
	  }
	  szType = strdup(tr("URL"));
	  nTypeEvent = ON_EVENT_URL;
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_AUTHxREQUEST:
	{
	  gLog.Info(tr("%sOffline authorization request from %lu.\n"), L_SBLANKxSTR, nUin);
	  
	  char **szFields = new char *[6];  // alias, first name, last name, email, auth, comment
          
	  if (!ParseFE(szMessage, &szFields, 6))
          {
            char *buf;
	    
            gLog.Warn(tr("%sInvalid offline authorization request system message:\n%s\n"), L_WARNxSTR,
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
          gLog.Info(tr("%sOffline authorization refused by %lu.\n"), L_SBLANKxSTR, nUin);

          // Translating string with Translation Table
          gTranslator.ServerToClient(szMessage);

          CEventAuthRefused *e = new CEventAuthRefused(nUin, szMessage, ICQ_CMDxRCV_SYSxMSGxOFFLINE,
						       nTimeSent, 0);
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_AUTHxGRANTED:  // system message: authorized
        {
          gLog.Info(tr("%sOffline authorization granted by %lu.\n"), L_SBLANKxSTR, nUin);

          // translating string with Translation Table
          gTranslator.ServerToClient (szMessage);

          ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
          if (u)
          {
            u->SetAwaitingAuth(false);
            gUserManager.DropUser(u);
          }

          CEventAuthGranted *e = new CEventAuthGranted(nUin, szMessage, ICQ_CMDxRCV_SYSxMSGxOFFLINE,
                                                       nTimeSent, 0);
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_MSGxSERVER:
	{
	  gLog.Info(tr("%sOffline server message.\n"), L_BLANKxSTR);
	  
	  CEventServerMessage *e = CEventServerMessage::Parse(szMessage, ICQ_CMDxSUB_MSGxSERVER, nTimeSent, nMask);
	  if (e == NULL)
	  {
	    char *buf;

	    gLog.Warn(tr("%sInvalid Server Message:\n%s\n"), L_WARNxSTR, packet.print(buf));
	    delete [] buf;
	    break;
	  }
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_ADDEDxTOxLIST:  // system message: added to a contact list
        {
          gLog.Info(tr("%sUser %lu added you to their contact list (offline).\n"), L_SBLANKxSTR, nUin);

          char **szFields = new char *[6]; // alias, first name, last name, email, auth, comment
          
	  if (!ParseFE(szMessage, &szFields, 6))
          {
            char *buf;
	    
            gLog.Warn(tr("%sInvalid offline added to list system message:\n%s\n"), L_WARNxSTR,
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
          gLog.Info(tr("%sOffline message through web panel.\n"), L_SBLANKxSTR);

          char **szFields = new char *[6];	// name, ?, ?, email, ?, message
        
	  if (!ParseFE(szMessage, &szFields, 6))
          {
    	    char *buf;
	    
            gLog.Warn(tr("%sInvalid offline web panel system message:\n%s\n"), L_WARNxSTR,
                      packet.print(buf));
            delete [] buf;
            delete [] szFields;
            break;
          }

          // translating string with Translation Table
          gTranslator.ServerToClient(szFields[0]);  // name
          gTranslator.ServerToClient(szFields[3]);  // email
          gTranslator.ServerToClient(szFields[5]);  // message

          gLog.Info(tr("%sFrom %s (%s).\n"), L_SBLANKxSTR, szFields[0], szFields[3]);
          CEventWebPanel *e = new CEventWebPanel(szFields[0], szFields[3], szFields[5],
                                                 ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, 0);
          delete [] szFields;	
          eEvent = e;
          break;
    }
	case ICQ_CMDxSUB_EMAILxPAGER:
        {
          gLog.Info(tr("%sEmail pager offline message.\n"), L_SBLANKxSTR);

          char **szFields = new char *[6];	// name, ?, ?, email, ?, message
        
	  if (!ParseFE(szMessage, &szFields, 6))
          {
            char *buf;

            gLog.Warn(tr("%sInvalid offline email pager system message:\n%s\n"), L_WARNxSTR,
		      packet.print(buf));
            delete [] buf;
            delete [] szFields;
            break;
          }

	  // translating string with Translation Table
          gTranslator.ServerToClient(szFields[0]);  // name
          gTranslator.ServerToClient(szFields[3]);  // email
          gTranslator.ServerToClient(szFields[5]);  // message

          gLog.Info(tr("%sFrom %s (%s).\n"), L_SBLANKxSTR, szFields[0], szFields[3]);
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
 
            gLog.Warn(tr("%sInvalid offline Contact List message:\n%s\n"), L_WARNxSTR,
		      packet.print(buf));
            delete [] buf;
            break;
          }
	  szType = strdup(tr("Contacts"));
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

	    gLog.Warn(tr("%sInvalid SMS message:\n%s\n"), L_WARNxSTR, packet.print(buf));
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
        	gLog.Info(tr("%sOffline %s from new user (%lu), ignoring.\n"), L_SBLANKxSTR, szType, nUin);
		if (szType) free(szType);
        	RejectEvent(nUin, eEvent);
        	break;
    	      }
    	      gLog.Info(tr("%sOffline %s from new user (%lu).\n"), L_SBLANKxSTR, szType, nUin);
    	      AddUserToList(nUin);
    	      u = gUserManager.FetchUser(nUin, LOCK_W);
    	    }
    	    else
    	      gLog.Info(tr("%sOffline %s through server from %s (%lu).\n"), L_SBLANKxSTR,
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
            ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
            bool bIgnore = false;
            if (u)
            {
              bIgnore = u->IgnoreList();
              gUserManager.DropUser(u);
            }

            if (bIgnore)
            {
              delete eEvent; // Processing stops here, needs to be deleted
              gLog.Info("Ignored!");
              break;
            }

	    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
            if (AddUserEvent(o, eEvent))
	    {
              gUserManager.DropOwner();
              eEvent->AddToHistory(NULL, LICQ_PPID, D_RECEIVER);
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
	      gLog.Info(tr("%sOffline SMS from %s - %s (%lu).\n"), L_SBLANKxSTR, eSms->Number(), u->GetAlias(), nUin);
	      if (AddUserEvent(u, eEvent))
	        m_xOnEventManager.Do(ON_EVENT_SMS, u);
	      gUserManager.DropUser(u);
	    }
	    else
	    {  
	      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
	      gLog.Info(tr("%sOffline SMS from %s.\n"), L_BLANKxSTR, eSms->Number());
	      if (AddUserEvent(o, eEvent))
	      {
	        gUserManager.DropOwner();
	        eEvent->AddToHistory(NULL, LICQ_PPID, D_RECEIVER);
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
      gLog.Info(tr("%sEnd of Offline messages (nId: %04hx).\n"), L_SRVxSTR, nId);
      CSrvPacketTcp* p = new CPU_SysMsgDoneAck(nId);
      SendEvent_Server(p);

      break;
    }
    case 0x07DA:
    case 0x07D0: // XXX Now I see this one, huh?
    {
      unsigned short nSubtype;
      unsigned char nResult;
      ICQEvent *pEvent = NULL;
      nSubtype = msg.UnpackUnsignedShort();
      nResult = msg.UnpackChar();
      char *szType = NULL;

      if (nSubtype == ICQ_CMDxMETA_PASSWORDxRSP)
      {
        szType = strdup(tr("Password change"));
        pEvent = DoneServerEvent(nSubSequence,
          nResult == META_SUCCESS ? EVENT_SUCCESS : EVENT_FAILED);
        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
          o->SetEnableSave(false);
          o->SetPassword(((CPU_SetPassword *)pEvent->m_pPacket)->m_szPassword);
          o->SetEnableSave(true);
          o->SaveLicqInfo();
          gUserManager.DropOwner();
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_SECURITYxRSP)
      {
        szType = strdup(tr("Security info"));
        pEvent = DoneServerEvent(nSubSequence,
          nResult == META_SUCCESS ? EVENT_SUCCESS : EVENT_FAILED);
      }
      else if (nSubtype == ICQ_CMDxMETA_GENERALxINFOxRSP)
      {
        szType = strdup(tr("General info"));
        pEvent = DoneServerEvent(nSubSequence,
          nResult == META_SUCCESS ? EVENT_SUCCESS : EVENT_FAILED);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetGeneralInfo *p = (CPU_Meta_SetGeneralInfo *)pEvent->m_pPacket;
          ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
          o->SetEnableSave(false);
          o->SetAlias(p->m_szAlias);
          o->SetFirstName(p->m_szFirstName);
          o->SetLastName(p->m_szLastName);
          o->SetEmailPrimary(p->m_szEmailPrimary);
          o->SetCity(p->m_szCity);
          o->SetState(p->m_szState);
          o->SetPhoneNumber(p->m_szPhoneNumber);
          o->SetFaxNumber(p->m_szFaxNumber);
          o->SetAddress(p->m_szAddress);
          o->SetCellularNumber(p->m_szCellularNumber);
          o->SetZipCode(p->m_szZipCode);
          o->SetCountryCode(p->m_nCountryCode);
          o->SetTimezone(p->m_nTimezone);
          o->SetHideEmail(p->m_nHideEmail); // 0 = no, 1 = yes

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
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_EMAILxINFOxRSP)
      {
        szType = strdup(tr("E-mail info"));
        pEvent = DoneServerEvent(nSubSequence,
          nResult == META_SUCCESS ? EVENT_SUCCESS : EVENT_FAILED);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetEmailInfo *p = (CPU_Meta_SetEmailInfo *)pEvent->m_pPacket;
          ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
          o->SetEnableSave(false);
          o->SetEmailSecondary(p->m_szEmailSecondary);
          o->SetEmailOld(p->m_szEmailOld);

          // translating string with Translation Table
          gTranslator.ServerToClient(o->GetEmailSecondary());
          gTranslator.ServerToClient(o->GetEmailOld());

          // save the user infomation
          o->SetEnableSave(true);
          o->SaveGeneralInfo();
          gUserManager.DropOwner();
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_MORExINFOxRSP)
      {
        szType = strdup(tr("More info"));
        pEvent = DoneServerEvent(nSubSequence,
          nResult == META_SUCCESS ? EVENT_SUCCESS : EVENT_FAILED);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetMoreInfo *p = (CPU_Meta_SetMoreInfo *)pEvent->m_pPacket;
          ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
          o->SetEnableSave(false);
          o->SetAge(p->m_nAge);
          o->SetGender(p->m_nGender);
          o->SetHomepage(p->m_szHomepage);
          o->SetBirthYear(p->m_nBirthYear);
          o->SetBirthMonth(p->m_nBirthMonth);
          o->SetBirthDay (p->m_nBirthDay);
          o->SetLanguage1(p->m_nLanguage1);
          o->SetLanguage2(p->m_nLanguage2);
          o->SetLanguage3(p->m_nLanguage3);

          // translating string with Translation Table
          gTranslator.ServerToClient(o->GetHomepage());

          // save the user infomation
          o->SetEnableSave(true);
          o->SaveMoreInfo();
          gUserManager.DropOwner();
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_INTERESTSxINFOxRSP)
      {
        szType = strdup("Interests info");
        pEvent = DoneServerEvent(nSubSequence,
          nResult == META_SUCCESS ? EVENT_SUCCESS : EVENT_FAILED);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetInterestsInfo *p =
                           (CPU_Meta_SetInterestsInfo *)pEvent->m_pPacket;
          ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
          o->SetEnableSave(false);
          unsigned short cat;
          const char *descr;
          o->m_Interests->Clean();
          for (int i = 0; p->m_Interests->Get(i, &cat, &descr); i++)
          {
            char *tmp = strdup(descr);
            gTranslator.ServerToClient(tmp);
            o->m_Interests->AddCategory(cat, tmp);
            free(tmp);
          }
          o->SetEnableSave(true);
          o->SaveInterestsInfo();
          gUserManager.DropOwner();
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_WORKxINFOxRSP)
      {
        szType = strdup(tr("Work info"));
        pEvent = DoneServerEvent(nSubSequence,
          nResult == META_SUCCESS ? EVENT_SUCCESS : EVENT_FAILED);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetWorkInfo *p = (CPU_Meta_SetWorkInfo *)pEvent->m_pPacket;
          ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
          o->SetEnableSave(false);
          o->SetCompanyCity(p->m_szCity);
          o->SetCompanyState(p->m_szState);
          o->SetCompanyPhoneNumber(p->m_szPhoneNumber);
          o->SetCompanyFaxNumber(p->m_szFaxNumber);
          o->SetCompanyAddress(p->m_szAddress);
          o->SetCompanyZip(p->m_szZip);
          o->SetCompanyCountry(p->m_nCompanyCountry);
          o->SetCompanyName(p->m_szName);
          o->SetCompanyDepartment(p->m_szDepartment);
          o->SetCompanyPosition(p->m_szPosition);
          o->SetCompanyOccupation(p->m_nCompanyOccupation);
          o->SetCompanyHomepage(p->m_szHomepage);

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
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_ABOUTxRSP)
      {
        szType = strdup(tr("About"));
        pEvent = DoneServerEvent(nSubSequence,
          nResult == META_SUCCESS ? EVENT_SUCCESS : EVENT_FAILED);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetAbout *p = (CPU_Meta_SetAbout *)pEvent->m_pPacket;
          ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
          char* msg = gTranslator.RNToN(p->m_szAbout);
          o->SetEnableSave(false);
          o->SetAbout(msg);
          delete [] msg;

          // translating string with Translation Table
          gTranslator.ServerToClient(o->GetAbout());

          // save the user infomation
          o->SetEnableSave(true);
          o->SaveAboutInfo();
          gUserManager.DropOwner();
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_SENDxSMSxRSP)
      {
        // this one sucks, it could be sms or organization response
        pEvent = DoneServerEvent(nSubSequence,
          nResult == META_SUCCESS ? EVENT_SUCCESS : EVENT_FAILED);

        if (pEvent != NULL &&
            pEvent->m_pPacket->SubCommand() == ICQ_CMDxMETA_ORGBACKxINFOxSET)
        {
          szType = strdup("Organizations/Background info");

          if (nResult == META_SUCCESS)
          {
            CPU_Meta_SetOrgBackInfo *p =
                             (CPU_Meta_SetOrgBackInfo *)pEvent->m_pPacket;
            ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
            o->SetEnableSave(false);
            unsigned short cat;
            const char *descr;
            o->m_Organizations->Clean();
            for (int i = 0; p->m_Orgs->Get(i, &cat, &descr); i++)
            {
              char *tmp = strdup(descr);
              gTranslator.ServerToClient(tmp);
              o->m_Organizations->AddCategory(cat, tmp);
              free(tmp);
            }
            o->SetEnableSave(true);
            o->SaveOrganizationsInfo();

            o->m_Backgrounds->Clean();
            for (int i = 0; p->m_Background->Get(i, &cat, &descr); i++)
            {
              char *tmp = strdup(descr);
              gTranslator.ServerToClient(tmp);
              o->m_Backgrounds->AddCategory(cat, tmp);
              free(tmp);
            }
            o->SetEnableSave(true);
            o->SaveBackgroundsInfo();
            gUserManager.DropOwner();
          }
        }
        else if (pEvent != NULL &&
                pEvent->m_pPacket->SubCommand() == ICQ_CMDxMETA_SENDxSMS)
        {
          char *szTag, *szXml, *szSmsResponse;
  
          msg.UnpackUnsignedShortBE();
          msg.UnpackUnsignedShortBE();
          msg.UnpackUnsignedShortBE();
  
          szTag = msg.UnpackStringBE();
          szXml = msg.UnpackStringBE();
          szSmsResponse = GetXmlTag(szXml, "sms_response");
  
          if (szSmsResponse != NULL)
          {
            char *szDeliverable;
            szDeliverable = GetXmlTag(szSmsResponse, "deliverable");
  
            if (szDeliverable != NULL)
            {
              if (!strcmp(szDeliverable, "Yes"))
              {
                gLog.Info(tr("%sSMS delivered.\n"), L_SRVxSTR);
                if (pEvent)
                {
                  pEvent->m_eResult = EVENT_SUCCESS;
                  ProcessDoneEvent(pEvent);
                }
              }
              else if (!strcmp(szDeliverable, "No"))
              {
                char *szId, *szParam;
  
                szId = GetXmlTag(szSmsResponse, "id");
                szParam = GetXmlTag(szSmsResponse, "param");
                gLog.Warn(tr("%sSMS not delivered, error #%s, param: %s\n"),
                  L_SRVxSTR, szId, szParam);
  
                if (szId != NULL) free(szId);
                if (szParam != NULL) free(szParam);
                if (pEvent)
                {
                  pEvent->m_eResult = EVENT_FAILED;
                  ProcessDoneEvent(pEvent);
                }
              }
              else if (!strcmp(szDeliverable, "SMTP"))
              {
                char *szFrom, *szTo, *szSubject;
  
                szFrom = GetXmlTag(szSmsResponse, "from");
                szTo = GetXmlTag(szSmsResponse, "to");
                szSubject = GetXmlTag(szSmsResponse, "subject");
                gLog.Info(tr("%sSending SMS via SMTP not supported yet.\n"),
                  L_SRVxSTR);
  
                if (szFrom != NULL)
                {
                  gLog.Info(tr("%sFrom: %s\n"), L_SRVxSTR, szFrom);
                  free(szFrom);
                }
  
                if (szTo != NULL)
                {
                  gLog.Info(tr("%sTo: %s\n"), L_SRVxSTR, szTo);
                  free(szTo);
                }
  
                if (szSubject != NULL)
                {
                  gLog.Info(tr("%sSubject: %s\n"), L_SRVxSTR, szSubject);
                  free(szSubject);
                }
  
                if (pEvent)
                {
                  pEvent->m_eResult = EVENT_FAILED;
                  ProcessDoneEvent(pEvent);
                }
              }
              else
              {
                char *buf;
                gLog.Info(tr("%sUnknown SMS response:\n%s"), L_UNKNOWNxSTR,
                          packet.print(buf));
  
                delete [] buf;
  
                if (pEvent)
                {
                  pEvent->m_eResult = EVENT_FAILED;
                  ProcessDoneEvent(pEvent);
                }
              }
  
              free(szDeliverable);
            }
            else
            {
              gLog.Info(tr("%sUndeliverable SMS.\n"), L_SRVxSTR);
              if (pEvent)
              {
                pEvent->m_eResult = EVENT_FAILED;
                ProcessDoneEvent(pEvent);
              }
            }
  
            free(szSmsResponse);
          }
  
          delete [] szTag;
          delete [] szXml;
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_SETxRANDxCHATxRSP)
      {
        szType = strdup(tr("Random chat group"));
        pEvent = DoneServerEvent(nSubSequence,
          nResult == META_SUCCESS ? EVENT_SUCCESS : EVENT_FAILED);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
          o->SetRandomChatGroup(((CPU_SetRandomChatGroup *)pEvent->m_pPacket)->Group());
          gUserManager.DropOwner();
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_RANDOMxUSERxRSP)
      {
        ICQEvent *e = DoneServerEvent(nSubSequence,
                        nResult == META_SUCCESS ? EVENT_SUCCESS : EVENT_FAILED);

        if (e != NULL && nResult == META_SUCCESS)
        {
          unsigned long nUin, nIp;
          char nMode;
          msg >> nUin;
          char szUin[14];
          snprintf(szUin, sizeof(szUin), "%lu", nUin);
          gLog.Info(tr("%sRandom chat user found (%s).\n"), L_SRVxSTR, szUin);
          ICQUser *u = gUserManager.FetchUser(szUin, LICQ_PPID, LOCK_W);
          bool bNewUser = false;
          if (u == NULL)
          {
             AddUserToList(szUin, LICQ_PPID, false, true);
             u = gUserManager.FetchUser(szUin, LICQ_PPID, LOCK_W);
             bNewUser = true;
          }

          msg.UnpackUnsignedShort(); // chat group

          nIp = msg.UnpackUnsignedLongBE();
          nIp = PacketIpToNetworkIp(nIp);
          u->SetIpPort(nIp, msg.UnpackUnsignedLong());

          nIp = msg.UnpackUnsignedLongBE();
          nIp = PacketIpToNetworkIp(nIp);
          u->SetIntIp(nIp);

          msg >> nMode;
          u->SetMode(nMode);
          if (nMode != MODE_DIRECT)
            u->SetSendServer(true);

          u->SetVersion(msg.UnpackUnsignedShort());

          gUserManager.DropUser(u);

          if (bNewUser)
          {
            icqRequestMetaInfo(nUin);
          }

          e->m_pSearchAck = new CSearchAck(nUin);
        }
        else
          gLog.Info(tr("%sNo random chat user found.\n"), L_SRVxSTR);
        ProcessDoneEvent(e);
      }
      else if (nSubtype == ICQ_CMDxMETA_WPxINFOxSET_RSP)
      {
        ICQEvent *e = DoneServerEvent(nSubSequence,
                                      nResult == 0x0A ? EVENT_SUCCESS : EVENT_FAILED);
        if (e == NULL)
        {
          gLog.Info(tr("%sReceived info update ack, without updating info.\n"), L_SRVxSTR);
          break;
        }

        gLog.Info(tr("%sUpdated information successfully.\n"), L_SRVxSTR);
                  
        ProcessDoneEvent(e);
      }
      else if (nSubtype == 0x0001)
      {
        ICQEvent *e = DoneServerEvent(nSubSequence, EVENT_FAILED);
        gLog.Info(tr("%sSMS failed to send.\n"), L_SRVxSTR);
        if (e != NULL)
        ProcessDoneEvent(e);
      }
      // Search results need to be processed differently
      else if (nSubtype == 0x0190 || nSubtype == 0x019a ||
               nSubtype == 0x01a4 || nSubtype == 0x01ae)
      {
        ICQEvent *e = NULL;

        if (nResult == 0x32) // No results found
        {
          gLog.Info(tr("%sWhitePages search found no users.\n"), L_SRVxSTR);
          e = DoneExtendedServerEvent(nSubSequence, EVENT_SUCCESS);
          ICQEvent *e2 = new ICQEvent(e);
          e2->m_pSearchAck = NULL; // Search ack is null lets plugins know no results found
          e2->m_nCommand = ICQ_CMDxSND_META;
          e2->m_nSubCommand = ICQ_CMDxMETA_SEARCHxWPxLAST_USER;
          PushPluginEvent(e2);
          DoneEvent(e, EVENT_SUCCESS);break;
        }

        e = DoneExtendedServerEvent(nSubSequence, EVENT_ACKED);
        if (e == NULL)
        {
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
        //TODO: Find out what these unknowns are. The first UnpackChar has been unknown for a long time, the others
        //seem fairly new.
        msg.UnpackChar(); // unknown
        msg.UnpackChar();
        msg.UnpackUnsignedLong();
        msg.UnpackUnsignedLong();
        msg.UnpackUnsignedLong();

        // translating string with Translation Table
        gTranslator.ServerToClient(s->m_szAlias);
        gTranslator.ServerToClient(s->m_szFirstName);
        gTranslator.ServerToClient(s->m_szLastName);

        gLog.Info("%s%s (%lu) <%s %s, %s>\n", L_SBLANKxSTR, s->m_szAlias, nFoundUin,
                  s->m_szFirstName, s->m_szLastName, s->m_szEmail);

        ICQEvent *e2 = new ICQEvent(e);
        // JON: Hack it so it is backwards compatible with plugins for now.
        e2->m_nCommand = ICQ_CMDxSND_META;
        e2->m_pSearchAck = s;
        if (nSubtype & 0x0008)
        {
          unsigned long nMore = 0;
          e2->m_nSubCommand = ICQ_CMDxMETA_SEARCHxWPxLAST_USER;
          nMore = msg.UnpackUnsignedLong();
          // No more subtraction by 1, and now it seems to always be 0
          e2->m_pSearchAck->m_nMore = nMore;
          e2->m_eResult = EVENT_SUCCESS;
        }
        else
        {
          e2->m_nSubCommand = ICQ_CMDxMETA_SEARCHxWPxFOUND;
          e2->m_pSearchAck->m_nMore = 0;
        }

        PushPluginEvent(e2);

        if (nSubtype & 0x0008)
          DoneEvent(e, EVENT_SUCCESS); // Remove it from the running event list
        else
          PushExtendedEvent(e);
      }
      else
      {
        ICQEvent *e = NULL;
        ICQUser *u = NULL;
        char *szId = 0;
        bool multipart = false;

        if ((nResult == 0x32) || (nResult == 0x14) || (nResult == 0x1e))
        {
          // error: empty result or nonexistent user (1E =  readonly???)
          gLog.Warn(tr("%sFailed to update user info: %x.\n"), L_WARNxSTR, nResult);
          e = DoneExtendedServerEvent(nSubSequence, EVENT_FAILED);
          if (e)
          {
            DoneEvent(e, EVENT_FAILED);
            ProcessDoneEvent(e);
          }
          e = NULL;
          break;
        }
        else
        {
          // Find the relevant event
          e = DoneExtendedServerEvent(nSubSequence, EVENT_SUCCESS);
          if (e == NULL)
          {
            gLog.Warn("%sUnmatched extended event (%d)!\n", L_WARNxSTR, nSubSequence);
              break;
          }
          szId = e->Id();

          u = FindUserForInfoUpdate(szId, e, "extended");
          if (u == NULL)
          {
            gLog.Warn(tr("%scan't find user for updating!\n"), L_WARNxSTR);
            break;
          }
        }

        switch (nSubtype)
        {
        case ICQ_CMDxMETA_GENERALxINFO:
        {   
          gLog.Info(tr("%sGeneral info on %s (%s).\n"), L_SRVxSTR, u->GetAlias(), u->IdString());

          // main home info
          u->SetEnableSave(false);
          if (u->m_bKeepAliasOnUpdate && !gUserManager.FindOwner(szId, LICQ_PPID))
            tmp = msg.UnpackString(); // Skip the alias, user wants to keep his own.
          else
          {
            tmp = msg.UnpackString();
            char *szUTFAlias = tmp ? gTranslator.ToUnicode(tmp, u->UserEncoding()) : 0;
            u->SetAlias(szUTFAlias);
            //printf("Alias: %s\n", szUTFAlias);
          }
          if (tmp)
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
          u->SetAuthorization( !msg.UnpackChar() );
          unsigned char nStatus = msg.UnpackChar(); // Web aware status

          if (gUserManager.FindOwner(u->IdString(), u->PPID()) != 0)
          {
            static_cast<ICQOwner *>(u)->SetWebAware(nStatus);
            /* this unpack is inside the if statement since it appears only
               for the owner request */
            u->SetHideEmail( msg.UnpackChar() );
          }
          else
          {
          /* TODO maybe implement web aware (could be useful for viewing info of
             users not on contact list) */
          }

/*
          unsigned short tmp = msg.UnpackChar();
          if (tmp)
            gLog.Error("%sConnection flags??? %x\n", L_ERRORxSTR, cf); */
 
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

          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_GENERAL, u->IdString(), u->PPID()));
          break;
        }
        case ICQ_CMDxMETA_MORExINFO:
        {
          gLog.Info(tr("%sMore info on %s (%s).\n"), L_SRVxSTR, u->GetAlias(), u->IdString());

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

          //TODO
/*
          if (unsigned short tmp = msg.UnpackUnsignedShort()) //??
            gLog.Error("Unknown value %x\n", tmp);
          char *city = msg.UnpackString();        //Originally from city
          char *state = msg.UnpackString();        //Originally from state
          const struct SCountry *sc = GetCountryByCode(msg.UnpackUnsignedShort()); //Originally from country
          char *country = "unknown";
          if (sc)
            country = sc->szName;
          char *mstatus = "unknown";
          switch (msg.UnpackChar())          // marital status: 
          {
            case 0:  mstatus = "unspecified"; break;
            case 10: mstatus = "Single"; break;
            case 11: mstatus = "In a long-term relationship"; break;
            case 12: mstatus = "Engaged"; break;
            case 20: mstatus = "Married"; break;
            case 30: mstatus = "Divorced"; break;
            case 31: mstatus = "Separated"; break;
            case 40: mstatus = "Widowed"; break;
          }
          gLog.Info("%s%s status is %s, originally from: %s, %s, %s\n",
                    L_WARNxSTR, u->GetAlias(), mstatus, city, state, country);
          */
          
          // translating string with Translation Table
          gTranslator.ServerToClient(u->GetHomepage());

          // save the user infomation
          u->SetEnableSave(true);
          u->SaveMoreInfo();

          PushExtendedEvent(e);
          multipart = true;

          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_MORE, u->IdString(), u->PPID()));
          break;
        }
        case ICQ_CMDxMETA_EMAILxINFO:
        {

          gLog.Info(tr("%sEmail info on %s (%s).\n"), L_SRVxSTR, u->GetAlias(), u->IdString());

          u->SetEnableSave(false);
          int nEmail = (int)msg.UnpackChar();
          for(int i = 0; i < 2; i++)
          {
            if (i < nEmail)
            {
              msg.UnpackChar(); // publish email, not yet implemented
              tmp = msg.UnpackString();
            }
            else
            {
              tmp = new char[1];
              tmp[0] = '\0';
            }
              
            if(i == 0)
            {
              u->SetEmailSecondary(tmp);
              gTranslator.ServerToClient(u->GetEmailSecondary());
            }
            else if(i == 1)
            {
              u->SetEmailOld(tmp);
              gTranslator.ServerToClient(u->GetEmailOld());
            }
            delete[] tmp;
          }
          
          // save the user infomation
          u->SetEnableSave(true);
          u->SaveGeneralInfo();

          PushExtendedEvent(e);
          multipart = true;
          
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_EXT, u->IdString(), u->PPID()));
          break;
        }

        case ICQ_CMDxMETA_HOMEPAGExINFO:
        {
          gLog.Info("%sHomepage info on %s (%s).\n", L_SRVxSTR, u->GetAlias(),
            u->IdString());          
          
          u->SetEnableSave(false);
          
          unsigned char categoryPresent = msg.UnpackChar();
          u->SetHomepageCatPresent(categoryPresent);
          
          if (categoryPresent)
          {
            u->SetHomepageCatCode(msg.UnpackUnsignedShort());

            char *rawmsg = msg.UnpackString();
            char *msg = gTranslator.RNToN(rawmsg);
            delete [] rawmsg;

            u->SetHomepageDesc(msg);
            delete [] msg;

            gTranslator.ServerToClient(u->GetHomepageDesc());
          }
          
          u->SetICQHomepagePresent(msg.UnpackChar());

          // save the user infomation
          u->SetEnableSave(true);
          u->SaveHomepageInfo();

          PushExtendedEvent(e);
          multipart = true;
          
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_HP, u->IdString(), u->PPID()));
          break;
        }

        case ICQ_CMDxMETA_WORKxINFO:
        
          gLog.Info(tr("%sWork info on %s (%s).\n"), L_SRVxSTR, u->GetAlias(), u->IdString());
          
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
          u->SetCompanyOccupation(msg.UnpackUnsignedShort());
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

          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_WORK, u->IdString(), u->PPID()));

          break;

        case ICQ_CMDxMETA_ABOUT:
        {
          gLog.Info(tr("%sAbout info on %s (%s).\n"), L_SRVxSTR, u->GetAlias(), u->IdString());

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

          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_ABOUT, u->IdString(), u->PPID()));

          break;
        }

        case ICQ_CMDxMETA_INTERESTSxINFO:
        {
          unsigned i, n;
          bool nRet = true;

          gLog.Info("%sPersonal Interests info on %s (%s).\n", L_SRVxSTR,
                    u->GetAlias(), u->IdString());

          u->SetEnableSave(false);
          u->m_Interests->Clean();
          n = msg.UnpackChar();

          for (i = 0; nRet && i < n; i++)
          { 
            unsigned short cat = msg.UnpackUnsignedShort();
            tmp = msg.UnpackString();
            gTranslator.ServerToClient(tmp);
            nRet = u->m_Interests->AddCategory(cat, tmp);
            delete [] tmp;
          }

          // save the user infomation
          u->SetEnableSave(true);
          u->SaveInterestsInfo();
          
          PushExtendedEvent(e);
          multipart = true;
          
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_MORE2,
                           u->IdString(), u->PPID()));
          break;
        }

        case ICQ_CMDxMETA_PASTxINFO:
        {
          // past background info - last one received
          unsigned i, n;
          bool nRet = true;

          gLog.Info("%sOrganizations/Past Background info on %s (%s).\n",
                    L_SRVxSTR, u->GetAlias(), u->IdString());

          u->SetEnableSave(false);

          u->m_Backgrounds->Clean();
 
          n = msg.UnpackChar();

          for (i = 0; nRet && i < n; i++)
          {
            unsigned short cat = msg.UnpackUnsignedShort();
            tmp = msg.UnpackString();
            gTranslator.ServerToClient(tmp);
            nRet = u->m_Backgrounds->AddCategory(cat, tmp);
            delete [] tmp;
          }

          // save the user infomation
          u->SetEnableSave(true);
          u->SaveBackgroundsInfo();

          //---- Organizations
          u->SetEnableSave(false);
          u->m_Organizations->Clean();
          nRet = true;
          n = msg.UnpackChar();

          for(i = 0; nRet && i < n; i++)
          {
            unsigned short cat = msg.UnpackUnsignedShort();
            tmp = msg.UnpackString();
            gTranslator.ServerToClient(tmp);
            nRet = u->m_Organizations->AddCategory(cat, tmp);
            delete [] tmp;
          }

          // our user info is now up to date
          u->SetOurClientTimestamp(u->ClientTimestamp());

          // save the user infomation
          u->SetEnableSave(true);
          u->SaveOrganizationsInfo();
          u->SaveLicqInfo();

          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_MORE2,
                           u->IdString(), u->PPID()));

          break;
        }

        default:
        {
          char* buf;

          gLog.Unknown(tr("%sunknown info: %04hx\n%s\n"), L_UNKNOWNxSTR,
                       nSubtype, packet.print(buf));
          delete [] buf;
        }
        }

        if (!multipart) {
          if (e) {
            DoneEvent(e, EVENT_SUCCESS);
            ProcessDoneEvent(e);
          } else {
            gLog.Warn(tr("%sResponse to unknown extended info request for %s (%s).\n"),
                      L_WARNxSTR, u->GetAlias(), szId);
          }
        }

        PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_EXT, u->IdString(), u->PPID()));
        gUserManager.DropUser(u);
      }

      if (szType)
      {
        if (pEvent)
        {
          if (pEvent->Result() == EVENT_SUCCESS)
            gLog.Info(tr("%s%s updated.\n"), L_SRVxSTR, szType);
          else
            gLog.Info(tr("%s%s update failed.\n"), L_SRVxSTR, szType);
          ProcessDoneEvent(pEvent);
        }
        else
          gLog.Info(tr("%sUnexpected result for %s.\n"), L_SRVxSTR, szType);

        free(szType);
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
    gLog.Unknown(tr("%sUnknown Various Family Subtype: %04hx\n%s\n"), L_UNKNOWNxSTR,
                 nSubtype, packet.print(buf));
    delete [] buf;
    break;
  }
}

//--------ProcessNewUINFam-----------------------------------------------------
void CICQDaemon::ProcessNewUINFam(CBuffer &packet, unsigned short nSubtype)
{
  /*unsigned long Flags =*/ packet.UnpackUnsignedLongBE();
  unsigned short nSubSequence = packet.UnpackUnsignedShortBE();

  switch (nSubtype)
  {
    case ICQ_SNACxNEW_UIN_ERROR:
    {
      if (m_szRegisterPasswd)
      {
        gLog.Warn(tr("%sVerification required. Reconnecting...\n"), L_WARNxSTR);

        ICQEvent *e = DoneServerEvent(nSubSequence, EVENT_ERROR);
        if (e)
          delete e;
        m_bVerify = true;
        
        // Reconnect now
        char *szPasswd = strdup(m_szRegisterPasswd); // gets deleted in postLogoff
        int nSD = m_nTCPSrvSocketDesc;
        m_nTCPSrvSocketDesc = -1;
        m_eStatus = STATUS_OFFLINE_MANUAL;
        m_bLoggingOn = false; 
        gSocketManager.CloseSocket(nSD);
        postLogoff(nSD, NULL);
        icqRegister(szPasswd);
        free(szPasswd);
      }
      else
      {
        ICQEvent *e = DoneServerEvent(nSubSequence, EVENT_ERROR);
        if (e)
          delete e;
        gLog.Error(tr("%sUnknown logon error. There appears to be an issue with the ICQ servers. Please try again later.\n"), L_ERRORxSTR);
      }

      break;
    }
    case ICQ_SNACxNEW_UIN:
    {

      ICQEvent *e = DoneServerEvent(nSubSequence, EVENT_SUCCESS);
      if (e)
        ProcessDoneEvent(e);

      m_bVerify = false;
      m_bRegistering = false;
      
      packet.UnpackUnsignedShort(); // flags
      packet.UnpackUnsignedLong();  // id

      // 40 bytes of shit
      for (int x = 0; x < 10; x++)
        packet.UnpackUnsignedLong();

      unsigned long nNewUin = packet.UnpackUnsignedLong();

      if (gUserManager.OwnerUin() != 0)
      {
        gLog.Warn(tr("%sReceived new uin (%lu) when already have a uin (%lu).\n"), L_WARNxSTR,
          nNewUin, gUserManager.OwnerUin());
        return;
      }

      gLog.Info(tr("%sReceived new uin: %lu\n"), L_SRVxSTR, nNewUin);
      char szUin[14];
      snprintf(szUin, sizeof(szUin), "%lu", nNewUin);
      gUserManager.AddOwner(szUin, LICQ_PPID);

      ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      if (o)
      {
        o->SetPassword(m_szRegisterPasswd);
        gUserManager.DropOwner(LICQ_PPID);
        free(m_szRegisterPasswd);
        m_szRegisterPasswd = 0;
        SaveConf();
      }

      PushPluginSignal(new CICQSignal(SIGNAL_NEW_OWNER, 0, szUin, LICQ_PPID));
      
      // Reconnect now
      int nSD = m_nTCPSrvSocketDesc;
      m_nTCPSrvSocketDesc = -1;
      m_eStatus = STATUS_OFFLINE_MANUAL;
      m_bLoggingOn = false; 
      gSocketManager.CloseSocket(nSD);
      postLogoff(nSD, NULL);
      icqLogon(ICQ_STATUS_ONLINE);
      break;
    }

    case ICQ_SNACxSEND_IMAGE:
    {
      packet.UnpackUnsignedShort(); // flags
      packet.UnpackUnsignedLong(); // id
      m_bVerify = false;
      
      if (!packet.readTLV())
      {
        char *buf;
        gLog.Unknown(tr("%sUnknown server response:\n%s\n"), L_UNKNOWNxSTR,
           packet.print(buf));
        delete [] buf;
        break;
      }

      char *szJPEG = packet.UnpackStringTLV(0x0002);
 
      // Save it in a file
      char szFilename[MAX_FILENAME_LEN];
      snprintf(szFilename, MAX_FILENAME_LEN, "%s/%s", BASE_DIR, "Licq_verify.jpg");
      FILE *fp = fopen(szFilename, "w");
      if (fp == 0)
      {
        gLog.Warn(tr("%sUnable to open file (%s):\n%s%s.\n"), L_WARNxSTR,
          szFilename, L_BLANKxSTR, strerror(errno));
        break;
      }
      
      fwrite(szJPEG, packet.getTLVLen(0x0002), 1, fp);
      fclose(fp);
      
      // Push a signal to the plugin to load the file
      gLog.Info("%sReceived verification image.\n", L_SRVxSTR);
      PushPluginSignal(new CICQSignal(SIGNAL_VERIFY_IMAGE, 0, 0, LICQ_PPID));
      break;
    }

    default:
    {
      char *buf;
      gLog.Unknown("%sUnknown New UIN Family Subtype: %04hx\n%s\n", L_UNKNOWNxSTR,
          nSubtype, packet.print(buf));
      delete [] buf;
      break;
    }
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
    fprintf(stderr, "nsd <0 !\n");
  }
  else {
    m_nTCPSrvSocketDesc = -1;
    gSocketManager.CloseSocket(nSD);
    postLogoff(nSD, NULL);
  }

  if (packet.getDataSize() == 0) {
    gLog.Info(tr("%sWe're logging off..\n"), L_SRVxSTR);
    m_eStatus = STATUS_OFFLINE_FORCED;
    m_bLoggingOn = false;
    return true;
  }

  if (!packet.readTLV()) {
    gLog.Error("%sError during parsing packet!\n", L_ERRORxSTR);
    return false;
  }

  unsigned short nError = packet.UnpackUnsignedShortTLV(0x0008);
  switch (nError) {
  case 0x1D:
  case 0x18:
    gLog.Error(tr("%sRate limit exceeded.\n"), L_ERRORxSTR);
    PushPluginSignal(new CICQSignal(SIGNAL_LOGOFF, LOGOFF_RATE, 0,
                                    LICQ_PPID, 0, 0));
    break;

  case 0x04:
  case 0x05:
    gLog.Error(tr("%sInvalid UIN and password combination.\n"), L_ERRORxSTR);
    PushPluginSignal(new CICQSignal(SIGNAL_LOGOFF, LOGOFF_PASSWORD, 0,
                                    LICQ_PPID, 0, 0));
    break;
    
  case 0x0C:
  case 0x0D:
  case 0x12:
  case 0x13:
  case 0x14:
  case 0x15:
  case 0x1A:
  case 0x1F:
    gLog.Error(tr("%sThe requested network service is temporarily unavailable.\n"
                  "Please try again later.\n"), L_ERRORxSTR);
    break;  

  case 0:
    break;

  default:
    gLog.Error("%sUnknown sign on error: 0x%02X.\n", L_ERRORxSTR, nError);
  }

  if (nError)
  {
    packet.cleanupTLV();
    m_eStatus = STATUS_OFFLINE_FORCED;
    m_bLoggingOn = false;
    return false;
  }

  nError = packet.UnpackUnsignedShortTLV(0x0009);
  switch (nError)
  {
  case 0x0001:
    if (ReconnectAfterUinClash())
    {
      gLog.Error(tr("%sYour ICQ number is used from another location.\n"), L_ERRORxSTR);
      m_eStatus = STATUS_OFFLINE_FORCED; // will try to reconnect
    }
    else 
    {
      gLog.Error(tr("%sYour ICQ number is used from another location.\n"
                    "Automatic reconnect is disabled.\n"), L_ERRORxSTR);
      m_eStatus = STATUS_OFFLINE_MANUAL; // don't reconnect
    }
    break;

  case 0:
    break;

  default:
    gLog.Error("%sUnknown runtime error form server: 0x%02X.\n", L_ERRORxSTR, nError);
    m_eStatus = STATUS_OFFLINE_FORCED;
  }

  if (nError)
  {
    packet.cleanupTLV();
    m_bLoggingOn = false;
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
    m_eStatus = STATUS_OFFLINE_FORCED;
    m_bLoggingOn = false;
    return false;
  }

  packet.cleanupTLV();

  char* ptr;
  if ( (ptr = strchr(szNewServer, ':')))
    *ptr++ = '\0';

  gLog.Info(tr("%sAuthenticated. Connecting to %s port %s.\n"), L_SRVxSTR, szNewServer, ptr);

  // Connect to the new server here and send our cookie
  ConnectToServer(szNewServer, ptr ? atoi(ptr) : 5190);

  // Send our cookie
  CPU_SendCookie *p = new CPU_SendCookie(szCookie, nCookieLen);
  SendEvent_Server(p);

  delete [] szNewServer;
  delete [] szCookie;

  return true;
}

void  CICQDaemon::addToModifyUsers(unsigned long unique_id, const std::string data)
{
  pthread_mutex_lock(&mutex_modifyserverusers);
  m_lszModifyServerUsers[unique_id] = data;
  pthread_mutex_unlock(&mutex_modifyserverusers);
}

int CICQDaemon::RequestReverseConnection(unsigned long nUin,
                                         unsigned long nData,
                                         unsigned long nLocalIP,
                                         unsigned short nLocalPort,
                                         unsigned short nRemotePort)
{
  if (nUin == gUserManager.OwnerUin()) return -1;

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u == NULL) return -1;


  CPU_ReverseConnect *p = new CPU_ReverseConnect(u, nLocalIP, nLocalPort,
                                                 nRemotePort);
  int nId = p->SubSequence();

  pthread_mutex_lock(&mutex_reverseconnect);

  m_lReverseConnect.push_back(
            new CReverseConnectToUserData(nUin, nId, nData, nLocalIP,
                                          nLocalPort, ICQ_VERSION_TCP,
                                          nRemotePort, 0, nId));
  pthread_mutex_unlock(&mutex_reverseconnect);

  gLog.Info("%sRequesting reverse connection from %s.\n", L_TCPxSTR,
            u->GetAlias());
  SendEvent_Server(p);
  
  gUserManager.DropUser(u);

  return nId;
}
 
