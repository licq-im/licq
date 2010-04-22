// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>

#include <boost/scoped_array.hpp>

// Localization
#include "gettext.h"

#include "time-fix.h"

#include "licq_byteorder.h"
#include "licq_icqd.h"
#include "licq_translate.h"
#include "licq_oscarservice.h"
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
#include "licq_proxy.h"

using namespace std;

void CICQDaemon::protoAddUser(const string& accountId, unsigned long ppid, int groupId)
{
  if (ppid == LICQ_PPID)
    icqAddUser(accountId.c_str(), false, groupId);
  else
    PushProtoSignal(new CAddUserSignal(accountId.c_str(), false), ppid);
}

//-----icqAddUser----------------------------------------------------------
void CICQDaemon::icqAddUser(const char *_szId, bool _bAuthRequired, unsigned short groupId)
{
  UserId userId = LicqUser::makeUserId(_szId, LICQ_PPID);
  CSrvPacketTcp *p = new CPU_GenericUinList(_szId, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
  gLog.Info(tr("%sAlerting server to new user (#%hu)...\n"), L_SRVxSTR,
             p->Sequence());
  SendExpectEvent_Server(userId, p, NULL);

  // Server side list add, and update of group
  if (UseServerContactList())
  {
    icqAddUserServer(_szId, _bAuthRequired, groupId);
  }

  icqUserBasicInfo(_szId);
}

//-----icqAddUserServer--------------------------------------------------------
void CICQDaemon::icqAddUserServer(const char *_szId, bool _bAuthRequired,
    unsigned short groupId)
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
    groupId, _bAuthRequired);
  gLog.Info(tr("%sAdding %s to server list...\n"), L_SRVxSTR, _szId);
  addToModifyUsers(pAdd->SubSequence(), _szId);
  SendExpectEvent_Server(pAdd, NULL);

  CSrvPacketTcp *pEnd = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
                                              ICQ_SNACxLIST_ROSTxEDITxEND);
  SendEvent_Server(pEnd);
}

//-----CheckExport-------------------------------------------------------------
void CICQDaemon::CheckExport()
{
  // Export groups
  GroupNameMap groups;
  FOR_EACH_GROUP_START(LOCK_R)
  {
    if (pGroup->icqGroupId() == 0)
      groups[pGroup->id()] = pGroup->name();
  }
  FOR_EACH_GROUP_END

  if (groups.size() > 0)
    icqExportGroups(groups);

  // Just upload all of the users now
  list<UserId> users;
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_R)
  {
    // If they aren't a current server user and not in the ingore list,
    // let's import them!
    if (pUser->GetSID() == 0 && !pUser->IgnoreList())
      users.push_back(pUser->id());
  }
  FOR_EACH_PROTO_USER_END

  if (users.size())
  {
    icqExportUsers(users, ICQ_ROSTxNORMAL);
    icqUpdateServerGroups();
  }

  // Export visible/invisible/ignore list
  list<UserId> visibleUsers, invisibleUsers, ignoredUsers;
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_R)
  {
    if (pUser->IgnoreList() && pUser->GetSID() == 0)
    {
      ignoredUsers.push_back(pUser->id());
    }
    else
    {
      if (pUser->InvisibleList() && pUser->GetInvisibleSID() == 0)
        invisibleUsers.push_back(pUser->id());

      if (pUser->VisibleList() && pUser->GetVisibleSID() == 0)
        visibleUsers.push_back(pUser->id());
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
void CICQDaemon::icqExportUsers(const list<UserId>& users, unsigned short _nType)
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
  SendExpectEvent_Server(pReply, NULL);

  FOR_EACH_GROUP_START(LOCK_R)
  {
    unsigned int gid = pGroup->icqGroupId();
    if (gid != 0)
    {
      const char* gname = pGroup->name().c_str();
      pReply = new CPU_UpdateToServerList(gname, ICQ_ROSTxGROUP, gid);
      gLog.Info(tr("%sUpdating group %s.\n"), L_SRVxSTR, gname);
      addToModifyUsers(pReply->SubSequence(), "");
      SendExpectEvent_Server(pReply, NULL);
    }
  }
  FOR_EACH_GROUP_END
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
  SendExpectEvent_Server(pAdd, NULL);
}

void CICQDaemon::icqChangeGroup(const char *_szId, unsigned long _nPPID,
                                unsigned short _nNewGroup, unsigned short _nOldGSID,
                                unsigned short _nNewType, unsigned short _nOldType)
{
  if (!UseServerContactList())
    return;

  if (_nNewGroup == 0)
  {
    gLog.Warn(tr("%sProtocol prohibits for users to be in the root group.\n"
                 "%sAborting group change attempt for %s.\n"),
        L_SRVxSTR, L_BLANKxSTR, _szId);
    return;
  }

  // Get their old SID
  const ICQUser* u = gUserManager.FetchUser(_szId, _nPPID, LOCK_R);
  const char* alias = u->GetAlias();
  int nSID = u->GetSID();
  gUserManager.DropUser(u);

  gLog.Info(tr("%sChanging group on server list for %s (%s)...\n"),
      L_SRVxSTR, alias, _szId);

  // Start transaction
  CSrvPacketTcp* pStart =
    new CPU_GenericFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxEDITxSTART);
  SendEvent_Server(pStart);

  // Delete the user
  if (_nOldGSID != 0)
  {
    // Don't attempt removing users from the root group, they can't be there
    CSrvPacketTcp* pRemove =
      new CPU_RemoveFromServerList(_szId, _nOldGSID, nSID, _nOldType);
    addToModifyUsers(pRemove->SubSequence(), _szId);
    SendExpectEvent_Server(pRemove, NULL);
  }

  // Add the user, with the new group
  CPU_AddToServerList* pAdd =
    new CPU_AddToServerList(_szId, _nNewType, _nNewGroup);
  addToModifyUsers(pAdd->SubSequence(), _szId);
  SendExpectEvent_Server(pAdd, NULL);
}

//-----icqExportGroups----------------------------------------------------------
void CICQDaemon::icqExportGroups(const GroupNameMap& groups)
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
  SendExpectEvent_Server(pExport, NULL);
  
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
void CICQDaemon::protoRemoveUser(const UserId& userId)
{
  const LicqUser* u = gUserManager.fetchUser(userId);
  if (u == NULL)
    return;
  unsigned long ppid = u->ppid();
  string accountId = u->accountId();
  bool tempUser = u->NotInList();
  gUserManager.DropUser(u);

  if (ppid == LICQ_PPID && !tempUser)
    icqRemoveUser(accountId.c_str());
  else if(ppid != LICQ_PPID)
    PushProtoSignal(new CRemoveUserSignal(accountId.c_str()), ppid);
}

void CICQDaemon::icqRemoveUser(const char *_szId, bool ignored)
{
  UserId userId = LicqUser::makeUserId(_szId, LICQ_PPID);
  // Remove from the SSList and update groups
  if (UseServerContactList())
  {
    CSrvPacketTcp *pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
      ICQ_SNACxLIST_ROSTxEDITxSTART);
    SendEvent_Server(pStart);

    LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);
    // When we remove a user, we remove them from all parts of the list:
    // Visible, Invisible and Ignore lists as well.
    unsigned short nGSID = u->GetGSID();
    unsigned short nSID = u->GetSID();
    unsigned short nVisibleSID = u->GetVisibleSID();
    unsigned short nInvisibleSID = u->GetInvisibleSID();
    bool bIgnored = (u->IgnoreList() | ignored);
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
    SendExpectEvent_Server(pRemove, NULL);

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
  SendExpectEvent_Server(userId, p, NULL);
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
  SendExpectEvent_Server(pRemove, NULL);
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
  SendExpectEvent_Server(pUpdate, NULL);
}

void CICQDaemon::updateUserAlias(const UserId& userId)
{
  const LicqUser* user = gUserManager.fetchUser(userId);
  if (user == NULL)
    return;
  string accountId = user->accountId();
  unsigned long ppid = user->ppid();
  string newAlias = user->GetAlias();
  gUserManager.DropUser(user);

  if (ppid == LICQ_PPID)
    icqRenameUser(accountId, newAlias);
  else
    PushProtoSignal(new CRenameUserSignal(accountId.c_str()), ppid);
}

void CICQDaemon::icqRenameUser(const string& accountId, const string& newAlias)
{
  if (!UseServerContactList() || m_nTCPSrvSocketDesc == -1) return;

  CSrvPacketTcp *pUpdate = new CPU_UpdateToServerList(accountId.c_str(), ICQ_ROSTxNORMAL);
  gLog.Info(tr("%sRenaming %s to %s...\n"), L_SRVxSTR, accountId.c_str(), newAlias.c_str());
  addToModifyUsers(pUpdate->SubSequence(), accountId.c_str());
  SendExpectEvent_Server(pUpdate, NULL);
}

void CICQDaemon::icqAlertUser(const UserId& userId)
{
  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  char sz[MAX_MESSAGE_SIZE];
  sprintf(sz, "%s%c%s%c%s%c%s%c%c%c", o->GetAlias(), 0xFE, o->getFirstName().c_str(),
      0xFE, o->getLastName().c_str(), 0xFE, o->getEmail().c_str(), 0xFE,
          o->GetAuthorization() ? '0' : '1', 0xFE);
  gUserManager.DropOwner(o);
  string accountId = LicqUser::getUserAccountId(userId);
  const char* id = accountId.c_str();
  CPU_ThroughServer *p = new CPU_ThroughServer(id, ICQ_CMDxSUB_ADDEDxTOxLIST, sz);
  gLog.Info(tr("%sAlerting user they were added (#%hu)...\n"), L_SRVxSTR, p->Sequence());
  SendExpectEvent_Server(userId, p, NULL);
}

unsigned long CICQDaemon::requestUserAutoResponse(const UserId& userId)
{
  const LicqUser* user = gUserManager.fetchUser(userId);
  if (user == NULL)
    return 0;
  string accountId = user->accountId();
  unsigned long ppid = user->ppid();
  gUserManager.DropUser(user);

  unsigned long nRet = 0;

  if (ppid == LICQ_PPID)
    nRet = icqFetchAutoResponseServer(accountId.c_str());

  return nRet;
}

unsigned long CICQDaemon::icqFetchAutoResponseServer(const char *_szId)
{
  UserId userId = LicqUser::makeUserId(_szId, LICQ_PPID);
  CPU_CommonFamily *p = 0;

  if (isalpha(_szId[0]))
    p = new CPU_AIMFetchAwayMessage(_szId);
  else
  {
    const LicqUser* u = gUserManager.fetchUser(userId);
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
      USERID_TOSTR(userId), p->Sequence());

  LicqEvent* result = SendExpectEvent_Server(userId, p, NULL);
  if (result != NULL)
    return result->EventId();
  return 0;
}

//-----icqSetRandomChatGroup----------------------------------------------------
unsigned long CICQDaemon::icqSetRandomChatGroup(unsigned long _nGroup)
{
  CPU_SetRandomChatGroup *p = new CPU_SetRandomChatGroup(_nGroup);
  gLog.Info(tr("%sSetting random chat group (#%hu)...\n"), L_SRVxSTR,
            p->Sequence());

  ICQEvent* e = SendExpectEvent_Server(p, NULL);
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

  ICQEvent* e = SendExpectEvent_Server(p, NULL);
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
  ICQEvent* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
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

   SendExpectEvent_Server(p, NULL);
}

//-----ICQ::icqRelogon-------------------------------------------------------
void CICQDaemon::icqRelogon()
{
  unsigned long status;

  if (m_eStatus == STATUS_ONLINE)
  {
    const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
    status = o->StatusFull();
    gUserManager.DropOwner(o);
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

unsigned long CICQDaemon::requestUserInfo(const UserId& userId)
{
  const LicqUser* user = gUserManager.fetchUser(userId);
  if (user == NULL)
    return 0;
  string accountId = user->accountId();
  unsigned long ppid = user->ppid();
  gUserManager.DropUser(user);

  unsigned long nRet = 0;
  if (ppid == LICQ_PPID)
    nRet = icqRequestMetaInfo(accountId.c_str());
  else
    PushProtoSignal(new CRequestInfo(accountId.c_str()), ppid);

  return nRet;
}

//-----icqRequestMetaInfo----------------------------------------------------
unsigned long CICQDaemon::icqRequestMetaInfo(const char *_szId)
{
  UserId userId = LicqUser::makeUserId(_szId, LICQ_PPID);
  CPU_CommonFamily *p = 0;
  bool bIsAIM = isalpha(_szId[0]);
  if (bIsAIM)
    p = new CPU_RequestInfo(_szId);
  else
    p = new CPU_Meta_RequestAllInfo(_szId);
  gLog.Info(tr("%sRequesting meta info for %s (#%hu/#%d)...\n"), L_SRVxSTR,
      USERID_TOSTR(userId), p->Sequence(), p->SubSequence());
  LicqEvent* e = SendExpectEvent_Server(userId, p, NULL, !bIsAIM);
  if (e != NULL)
    return e->EventId();
  return 0;
}

unsigned long CICQDaemon::requestUserPicture(const UserId& userId)
{
  const LicqUser* user = gUserManager.fetchUser(userId);
  if (user == NULL)
    return 0;
  string accountId = user->accountId();
  unsigned long ppid = user->ppid();
  size_t iconHashSize = strlen(user->BuddyIconHash());
  bool sendServer = (user->SocketDesc(ICQ_CHNxINFO) < 0);
  gUserManager.DropUser(user);

  unsigned long nRet = 0;

  if (ppid == LICQ_PPID)
  {
    if (UseServerSideBuddyIcons() && iconHashSize > 0)
      nRet = m_xBARTService->SendEvent(userId, ICQ_SNACxBART_DOWNLOADxREQUEST, true);
    else
      nRet = icqRequestPicture(accountId.c_str(), sendServer);
  }
  else
    PushProtoSignal(new CRequestPicture(accountId.c_str()), ppid);

  return nRet;
}

//-----icqRequestService--------------------------------------------------------
void CICQDaemon::icqRequestService(unsigned short nFam)
{
  CPU_CommonFamily *p = new CPU_RequestService(nFam);
  gLog.Info(tr("%sRequesting service socket for FAM 0x%02X (#%hu/#%d)...\n"),
            L_SRVxSTR, nFam, p->Sequence(), p->SubSequence());
  SendEvent_Server(p);
}

//-----icqSetStatus-------------------------------------------------------------
unsigned long CICQDaemon::protoSetStatus(const UserId& ownerId, unsigned short newStatus)
{
  bool isOffline;
  unsigned long ppid;

  {
    LicqUserReadGuard u(ownerId);
    if (!u.isLocked() || u->User())
      return 0;

    isOffline = u->StatusOffline();
    ppid = u->ppid();
  }

  unsigned long nRet = 0;

  if (newStatus == ICQ_STATUS_OFFLINE)
  {
    if (isOffline)
      return 0;

    if (ppid == LICQ_PPID)
      icqLogoff();
    else
      PushProtoSignal(new CLogoffSignal(), ppid);
  }
  else if(isOffline)
  {
    if (ppid == LICQ_PPID)
      nRet = icqLogon(newStatus);
    else
      PushProtoSignal(new CLogonSignal(newStatus), ppid);
  }
  else
  {
    if (ppid == LICQ_PPID)
      nRet = icqSetStatus(newStatus);
    else
      PushProtoSignal(new CChangeStatusSignal(newStatus), ppid);
  }

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
  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  unsigned long s = o->AddStatusFlags(newStatus);
  unsigned long pfm = o->PhoneFollowMeStatus();
  bool Invisible = o->StatusInvisible();
  bool goInvisible = (newStatus & ICQ_STATUS_FxPRIVATE);
  bool isLogon = o->StatusOffline();
  int nPDINFO = o->GetPDINFO();
  gUserManager.DropOwner(o);

  if (nPDINFO == 0)
  {
    icqCreatePDINFO();

    o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
    nPDINFO = o->GetPDINFO();
    gUserManager.DropOwner(o);
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
  ICQEvent* e = SendExpectEvent_Server(p, NULL);
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

  ICQEvent* e = SendExpectEvent_Server(p, NULL);
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

  ICQEvent* e = SendExpectEvent_Server(p, NULL);
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

  ICQEvent *e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetInterestsInfo------------------------------------------------------
unsigned long CICQDaemon::icqSetInterestsInfo(const UserCategoryMap& interests)
{
  CPU_Meta_SetInterestsInfo *p = new CPU_Meta_SetInterestsInfo(interests);
  gLog.Info("%sUpdating Interests info (#%hu/#%d)..\n", L_SRVxSTR,
    p->Sequence(), p->SubSequence());

  ICQEvent* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetOrgBackInfo--------------------------------------------------------
unsigned long CICQDaemon::icqSetOrgBackInfo(const UserCategoryMap& orgs,
    const UserCategoryMap& background)
{
  CPU_Meta_SetOrgBackInfo *p =
    new CPU_Meta_SetOrgBackInfo(orgs, background);
  gLog.Info("%sUpdating Organizations/Backgrounds info (#%hu/#%d)..\n",
    L_SRVxSTR, p->Sequence(), p->SubSequence());

  ICQEvent* e = SendExpectEvent_Server(p, NULL);
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

  ICQEvent* e = SendExpectEvent_Server(p, NULL);
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

  ICQEvent* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqAuthorizeGrant-------------------------------------------------------
unsigned long CICQDaemon::authorizeGrant(const UserId& userId, const string& message)
{
  unsigned long nRet = 0;
  unsigned long nPPID = LicqUser::getUserProtocolId(userId);
  string accountId = LicqUser::getUserAccountId(userId);

  if (nPPID == LICQ_PPID)
    nRet = icqAuthorizeGrant(userId, message);
  else
    PushProtoSignal(new CGrantAuthSignal(accountId.c_str(), message.c_str()), nPPID);
  return nRet;
}

unsigned long CICQDaemon::icqAuthorizeGrant(const UserId& userId, const string& /* message */)
{
  const string accountId = LicqUser::getUserAccountId(userId);
  const char* szId = accountId.c_str();
  CPU_Authorize *p = new CPU_Authorize(szId);
  gLog.Info(tr("%sAuthorizing user %s\n"), L_SRVxSTR, szId);
  SendEvent_Server(p);

  return 0;
}

//-----icqAuthorizeRefuse------------------------------------------------------
unsigned long CICQDaemon::authorizeRefuse(const UserId& userId, const string& message)
{
  unsigned long nRet = 0;
  unsigned long nPPID = LicqUser::getUserProtocolId(userId);
  string accountId = LicqUser::getUserAccountId(userId);

  if (nPPID == LICQ_PPID)
    nRet = icqAuthorizeRefuse(userId, message);
  else
    PushProtoSignal(new CRefuseAuthSignal(accountId.c_str(), message.c_str()), nPPID);

  return nRet;
}

unsigned long CICQDaemon::icqAuthorizeRefuse(const UserId& userId, const string& message)
{
  const string accountId = LicqUser::getUserAccountId(userId);
  const char* szId = accountId.c_str();
  char *sz = NULL;
  if (!message.empty())
  {
    sz = gTranslator.NToRN(message.c_str());
    gTranslator.ClientToServer(sz);
  }
  CPU_ThroughServer *p = new CPU_ThroughServer(szId, ICQ_CMDxSUB_AUTHxREFUSED, sz);
  gLog.Info(tr("%sRefusing authorization to user %s (#%hu)...\n"), L_SRVxSTR,
     szId, p->Sequence());
  delete [] sz;

  ICQEvent* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

void CICQDaemon::icqRequestAuth(const char* id, const char *_szMessage)
{
  CSrvPacketTcp* p = new CPU_RequestAuth(id, _szMessage);
  SendEvent_Server(p);
}

//-----icqSetSecurityInfo----------------------------------------------------
unsigned long CICQDaemon::icqSetSecurityInfo(bool bAuthorize, bool bHideIp, bool bWebAware)
{
  // Since ICQ5.1, the status change packet is sent first, which means it is
  // assumed that the set security info packet works.
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  o->SetEnableSave(false);
  o->SetAuthorization(bAuthorize);
  o->SetWebAware(bWebAware);
  o->SetHideIp(bHideIp);
  o->SetEnableSave(true);
  o->SaveLicqInfo();
  unsigned short s = o->StatusFull();
  gUserManager.DropOwner(o);
  // Set status to ensure the status flags are set
  icqSetStatus(s);

  // Now send the set security info packet
    CPU_Meta_SetSecurityInfo *p = new CPU_Meta_SetSecurityInfo(bAuthorize, bHideIp, bWebAware);
    gLog.Info(tr("%sUpdating security info (#%hu/#%d)...\n"), L_SRVxSTR, p->Sequence(), p->SubSequence());
    ICQEvent* e = SendExpectEvent_Server(p, NULL);
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
  ICQEvent *e = SendExpectEvent_Server(p, NULL, true);
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
   ICQEvent* e = SendExpectEvent_Server(p, NULL, true);
   if (e != NULL)
     return e->EventId();
   return 0;
}

//-----icqGetUserBasicInfo------------------------------------------------------
unsigned long CICQDaemon::icqUserBasicInfo(const char *_szId)
{
  UserId userId = LicqUser::makeUserId(_szId, LICQ_PPID);
  CPU_CommonFamily *p = 0;
  bool bIsAIM = isalpha(_szId[0]);
  if (bIsAIM)
    p = new CPU_RequestInfo(_szId);
  else
    p = new CPU_Meta_RequestAllInfo(_szId);
  gLog.Info(tr("%sRequesting user info (#%hu/#%d)...\n"), L_SRVxSTR,
            p->Sequence(), p->SubSequence());
  LicqEvent* e = SendExpectEvent_Server(userId, p, NULL, !bIsAIM);
  if (e != NULL)
    return e->EventId();
  return 0;
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
  gUserManager.DropOwner(o);

  if (!bOffline)
    icqUpdateInfoTimestamp(PLUGIN_PHONExBOOK);
}

//-----icqUpdatePictureTimestamp------------------------------------------------
void CICQDaemon::icqUpdatePictureTimestamp()
{
  ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  o->SetClientInfoTimestamp(time(NULL));
  bool bOffline = o->StatusOffline();
  gUserManager.DropOwner(o);

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
  gUserManager.DropOwner(o);

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
  StringList users;
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_W)
  {
    n++;
    users.push_back(pUser->IdString());
    if (n == m_nMaxUsersPerPacket)
    {
      CSrvPacketTcp *p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
      gLog.Info(tr("%sUpdating contact list (#%hu)...\n"), L_SRVxSTR, p->Sequence());
      SendEvent_Server(p);
      users.clear();
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
  UserId userId = LicqUser::makeUserId(_szId, LICQ_PPID);
  CSrvPacketTcp *p = new CPU_CheckInvisible(_szId);
  SendExpectEvent_Server(userId, p, NULL);
}

//-----icqSendVisibleList-------------------------------------------------------
void CICQDaemon::icqSendVisibleList()
{
  // send user info packet
  // Go through the entire list of users, checking if each one is on
  // the visible list
  StringList users;
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_R)
  {
    if (pUser->GetInGroup(GROUPS_SYSTEM, GROUP_VISIBLE_LIST) )
      users.push_back(pUser->IdString());
  }
  FOR_EACH_PROTO_USER_END
  CSrvPacketTcp* p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxVISIBLExLIST);
  gLog.Info(tr("%sSending visible list (#%hu)...\n"), L_SRVxSTR, p->Sequence());
  SendEvent_Server(p);
}


//-----icqSendInvisibleList-----------------------------------------------------
void CICQDaemon::icqSendInvisibleList()
{
  StringList users;
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_R)
  {
    if (pUser->GetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST) )
      users.push_back(pUser->IdString());
  }
  FOR_EACH_PROTO_USER_END

  CSrvPacketTcp* p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxINVISIBxLIST);
  gLog.Info(tr("%sSending invisible list (#%hu)...\n"), L_SRVxSTR, p->Sequence());
  SendEvent_Server(p);
}

//-----ProtoToggleVisibleList------------------------------------------------
void CICQDaemon::ProtoToggleVisibleList(const char* _szId, unsigned long _nPPID)
{
  const ICQUser* u = gUserManager.FetchUser(_szId, _nPPID, LOCK_R);
  if (u == NULL) return;
  bool b = u->VisibleList();
  gUserManager.DropUser(u);
  visibleListSet(LicqUser::makeUserId(_szId, _nPPID), !b);
}

void CICQDaemon::visibleListSet(const UserId& userId, bool visible)
{
  unsigned long _nPPID = LicqUser::getUserProtocolId(userId);
  string accountId = LicqUser::getUserAccountId(userId);

  if (!visible)
    if (_nPPID == LICQ_PPID)
      icqRemoveFromVisibleList(userId);
    else
      PushProtoSignal(new CUnacceptUserSignal(accountId.c_str()), _nPPID);
  else
    if (_nPPID == LICQ_PPID)
      icqAddToVisibleList(userId);
    else
      PushProtoSignal(new CAcceptUserSignal(accountId.c_str()), _nPPID);
}

//-----ProtoToggleInvisibleList-------------------------------------------------
void CICQDaemon::ProtoToggleInvisibleList(const char *_szId, unsigned long _nPPID)
{
  const ICQUser* u = gUserManager.FetchUser(_szId, _nPPID, LOCK_R);
  if (u == NULL) return;
  bool b = u->InvisibleList();
  gUserManager.DropUser(u);
  invisibleListSet(LicqUser::makeUserId(_szId, _nPPID), !b);
}

void CICQDaemon::invisibleListSet(const UserId& userId, bool invisible)
{
  unsigned long _nPPID = LicqUser::getUserProtocolId(userId);
  string accountId = LicqUser::getUserAccountId(userId);

  if (!invisible)
    if (_nPPID == LICQ_PPID)
      icqRemoveFromInvisibleList(userId);
    else
      PushProtoSignal(new CUnblockUserSignal(accountId.c_str()), _nPPID);
  else
    if (_nPPID == LICQ_PPID)
      icqAddToInvisibleList(userId);
    else
      PushProtoSignal(new CBlockUserSignal(accountId.c_str()), _nPPID);
}

//-----icqToggleIgnoreList------------------------------------------------------
void CICQDaemon::icqToggleIgnoreList(const char *_szId, unsigned long _nPPID)
{
  const ICQUser* u = gUserManager.FetchUser(_szId, _nPPID, LOCK_R);
  if (u == NULL) return;
  bool b = u->IgnoreList();
  gUserManager.DropUser(u);
  ignoreListSet(LicqUser::makeUserId(_szId, _nPPID), b);
}

void CICQDaemon::ignoreListSet(const UserId& userId, bool b)
{
  unsigned long _nPPID = LicqUser::getUserProtocolId(userId);
  string accountId = LicqUser::getUserAccountId(userId);

  if (_nPPID == LICQ_PPID)
  {
    if (b)
      icqAddToIgnoreList(userId);
    else
      icqRemoveFromIgnoreList(userId);
  }
  else
  {
    if (b)
      PushProtoSignal(new CIgnoreUserSignal(accountId.c_str()), _nPPID);
    else
      PushProtoSignal(new CUnignoreUserSignal(accountId.c_str()), _nPPID);
  }
}

//-----icqAddToVisibleList------------------------------------------------------
void CICQDaemon::icqAddToVisibleList(const UserId& userId)
{
  LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);
  string accountId = LicqUser::getUserAccountId(userId);
  const char* _szId = accountId.c_str();
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
    SendExpectEvent_Server(pAdd, NULL);
  }
}

//-----icqRemoveFromVisibleList-------------------------------------------------
void CICQDaemon::icqRemoveFromVisibleList(const UserId& userId)
{
  LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);
  string accountId = LicqUser::getUserAccountId(userId);
  const char* _szId = accountId.c_str();
  if (u != NULL)
  {
    u->SetVisibleList(false);
    gUserManager.DropUser(u);
  }
  CSrvPacketTcp *p = new CPU_GenericUinList(_szId, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_REMxVISIBLExLIST);
  gLog.Info(tr("%sRemoving user %s from visible list (#%hu)...\n"), L_SRVxSTR,
      USERID_TOSTR(userId), p->Sequence());
  SendEvent_Server(p);

  if (UseServerContactList())
  {
    u = gUserManager.fetchUser(userId);
    if (u != NULL)
    {
      CSrvPacketTcp *pRemove = new CPU_RemoveFromServerList(_szId, 0, u->GetVisibleSID(),
        ICQ_ROSTxVISIBLE);
      addToModifyUsers(pRemove->SubSequence(), _szId);
      SendExpectEvent_Server(userId, pRemove, NULL);
      gUserManager.DropUser(u);
    }
  }
}

//-----icqAddToInvisibleList----------------------------------------------------
void CICQDaemon::icqAddToInvisibleList(const UserId& userId)
{
  string accountId = LicqUser::getUserAccountId(userId);
  const char* _szId = accountId.c_str();
  LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);
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

//-----icqRemoveFromInvisibleList-----------------------------------------------
void CICQDaemon::icqRemoveFromInvisibleList(const UserId& userId)
{
  string accountId = LicqUser::getUserAccountId(userId);
  const char* _szId = accountId.c_str();
  LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);
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
    u = gUserManager.fetchUser(userId);
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

//-----icqAddToIgnoreList-------------------------------------------------------
void CICQDaemon::icqAddToIgnoreList(const UserId& userId)
{
  if (!UseServerContactList()) return;

  string accountId = LicqUser::getUserAccountId(userId);
  const char* _szId = accountId.c_str();
  icqRemoveUser(_szId);
  CPU_AddToServerList *pAdd = new CPU_AddToServerList(_szId, ICQ_ROSTxIGNORE,
    0, false);
  SendEvent_Server(pAdd);
}

//-----icqRemoveFromIgnoreList--------------------------------------------------
void CICQDaemon::icqRemoveFromIgnoreList(const UserId& userId)
{
  if (!UseServerContactList()) return;

  string accountId = LicqUser::getUserAccountId(userId);
  const char* _szId = accountId.c_str();
  icqRemoveUser(_szId, true);
  icqAddUser(_szId, false);
}

//-----icqClearServerList-------------------------------------------------------
void CICQDaemon::icqClearServerList()
{
  if (!UseServerContactList()) return;

  unsigned short n = 0;
  StringList users;

  // Delete all the users in groups
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_W)
  {
    n++;
    users.push_back(pUser->IdString());
    if (n == m_nMaxUsersPerPacket)
    {
      gUserManager.DropUser(pUser);
      CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxNORMAL);
      gLog.Info(tr("%sDeleting server list users (#%hu)...\n"), L_SRVxSTR, p->Sequence());
      SendEvent_Server(p);
      users.clear();
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
  users.clear();

  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_W)
  {
    if (pUser->GetInvisibleSID())
    {
      n++;
      users.push_back(pUser->IdString());
    }

    if (n == m_nMaxUsersPerPacket)
    {
      gUserManager.DropUser(pUser);
      CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxINVISIBLE);
      gLog.Info(tr("%sDeleting server list invisible list users (#%hu)...\n"),
        L_SRVxSTR, p->Sequence());
      SendEvent_Server(p);
      users.clear();
      n = 0;
      continue;
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
  users.clear();

  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_W)
  {
    if (pUser->GetVisibleSID())
    {
      n++;
      users.push_back(pUser->IdString());
    }

    if (n == m_nMaxUsersPerPacket)
    {
      gUserManager.DropUser(pUser);
      CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxVISIBLE);
      gLog.Info(tr("%sDeleting server list visible list users (#%hu)...\n"),
        L_SRVxSTR, p->Sequence());
      SendEvent_Server(p);
      users.clear();
      n = 0;
      continue;
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
    unsigned char format, const char *_sMessage, CUserEvent* ue, unsigned short nCharset,
  size_t nMsgLen)
{
  ICQEvent* result;
  UserId userId = LicqUser::makeUserId(szId, LICQ_PPID);
  bool bOffline = true;
  {
    LicqUserReadGuard u(userId);
    if (u.isLocked())
      bOffline = u->StatusOffline();
  }

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
  LicqEvent* e = new LicqEvent(this, m_nTCPSrvSocketDesc, p, CONNECT_SERVER, userId, ue);
  if (e == NULL) return 0;
  e->m_NoAck = true;

  result = SendExpectEvent(e, &ProcessRunningEvent_Server_tep);
  return result;
}

unsigned long CICQDaemon::icqSendSms(const char* id, unsigned long ppid,
    const char* number, const char* message)
{
  UserId userId = LicqUser::makeUserId(id, ppid);
  CEventSms* ue = new CEventSms(number, message, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, INT_VERSION);
  CPU_SendSms* p = new CPU_SendSms(number, message);
  gLog.Info(tr("%sSending SMS through server (#%hu/#%d)...\n"), L_SRVxSTR,
      p->Sequence(), p->SubSequence());
  ICQEvent* e = SendExpectEvent_Server(userId, p, ue);
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
    LicqUser* u = gUserManager.fetchUser(e->userId(), LOCK_W);
    if (u != NULL)
    {
      e->m_pUserEvent->AddToHistory(u, D_SENDER);
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
      case MAKESNAC(ICQ_SNACxFAM_AUTH, ICQ_SNACxREGISTER_USER):
      case MAKESNAC(ICQ_SNACxFAM_LOCATION, ICQ_SNACxREQUESTxUSERxINFO):
      case MAKESNAC(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_INFOxREQ):
      case MAKESNAC(ICQ_SNACxFAM_BART, ICQ_SNACxBART_DOWNLOADxREQUEST):
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

unsigned long CICQDaemon::icqLogon(unsigned short logonStatus)
{
  if (m_bLoggingOn)
  {
    gLog.Warn(tr(tr("%sAttempt to logon while already logged or logging on, logoff and try again.\n")), L_WARNxSTR);
    return 0;
  }
  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o->IdString() == 0)
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

  m_nDesiredStatus = o->AddStatusFlags(logonStatus);
  gUserManager.DropOwner(o);

  CPU_ConnectStart *startPacket = new CPU_ConnectStart();
  SendEvent_Server(startPacket);

  m_bOnlineNotifies = false;
  m_nServerSequence = 0;
  m_bLoggingOn = true;
  m_tLogonTime = time(NULL);
  m_bNeedSalt = true;

  return 0;
}

unsigned long CICQDaemon::icqRequestLogonSalt()
{
  if (m_bNeedSalt)
  {
    const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
    CPU_RequestLogonSalt *p =  new CPU_RequestLogonSalt(o->IdString());
    gUserManager.DropOwner(o);
    gLog.Info(tr("%sRequesting logon salt (#%hu)...\n"), L_SRVxSTR, p->Sequence());
    SendEvent_Server(p);
  }

  return 0;
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
    cancelledEvent = new LicqEvent(this, nSD, &p, CONNECT_SERVER);
    cancelledEvent->m_pPacket = NULL;
    cancelledEvent->m_bCancelled = true;
    SendEvent(nSD, p, true);
    gSocketManager.CloseSocket(nSD);
  }

  postLogoff(nSD, cancelledEvent);
}

void CICQDaemon::postLogoff(int nSD, ICQEvent *cancelledEvent)
{
  if (m_xBARTService)
  {
    if (m_xBARTService->GetSocketDesc() != -1)
    {
      gSocketManager.CloseSocket(m_xBARTService->GetSocketDesc());
      m_xBARTService->ResetSocket();
      m_xBARTService->ChangeStatus(STATUS_UNINITIALIZED);
      m_xBARTService->ClearQueue();
    }
  }
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

  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  if (o)
    ChangeUserStatus(o, ICQ_STATUS_OFFLINE);
  gUserManager.DropOwner(o);

  if (m_szRegisterPasswd)
  {
    free(m_szRegisterPasswd);
    m_szRegisterPasswd = 0;
  }

  pushPluginSignal(new LicqSignal(SIGNAL_LOGOFF, 0, gUserManager.ownerUserId(LICQ_PPID), LICQ_PPID));

  // Mark all users as offline, this also updates the last seen
  // online field
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_W)
  {
    if (!pUser->StatusOffline())
      ChangeUserStatus(pUser, ICQ_STATUS_OFFLINE);
  }
  FOR_EACH_PROTO_USER_END
}

void CICQDaemon::sendTypingNotification(const UserId& userId, bool active, int nSocket)
{
  const LicqUser* user = gUserManager.fetchUser(userId);
  if (user == NULL)
    return;
  string accountId = user->accountId();
  unsigned long ppid = user->ppid();
  gUserManager.DropUser(user);

  //TODO: Make for each plugin
  if (!m_bSendTN)
    return;

  if (ppid == LICQ_PPID)
    icqTypingNotification(accountId.c_str(), active);
  else
    PushProtoSignal(new CTypingNotificationSignal(accountId.c_str(), active, nSocket), ppid);
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
  SrvSocket* s = new SrvSocket(gUserManager.ownerUserId(LICQ_PPID));

  if (m_bProxyEnabled)
  {
    if (m_xProxy == NULL)
    {
      gLog.Warn(tr("%sProxy server not properly configured.\n"), L_ERRORxSTR);
      delete s;
      return (-1);
    }
  }
  else if (m_xProxy != NULL)
  {
    delete m_xProxy;
    m_xProxy = NULL;
  }

  if (!s->connectTo(string(server), port, m_xProxy))
  {
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
    CPacket::SetLocalIp(LE_32(s->getLocalIpInt()));
    ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
    if (o)
      o->SetIntIp(s->getLocalIpInt());
    gUserManager.DropOwner(o);

    gSocketManager.AddSocket(s);
    nSocket = m_nTCPSrvSocketDesc = s->Descriptor();
    gSocketManager.DropSocket(s);
  }

  pthread_mutex_unlock(&connect_mutex);

  return nSocket;
}

//-----FindUserForInfoUpdate-------------------------------------------------
LicqUser* CICQDaemon::FindUserForInfoUpdate(const UserId& userId, LicqEvent* e,
   const char *t)
{
  LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);
  if (u == NULL)
  {
    // If the event is NULL as well then nothing we can do
    if (e == NULL)
    {
      gLog.Warn(tr("%sResponse to unknown %s info request for unknown user (%s).\n"),
          L_WARNxSTR, t, USERID_TOSTR(userId));
      return NULL;
    }
    // Check if we need to create the user
    if (e->m_pUnknownUser == NULL)
    {
      e->m_pUnknownUser = new LicqUser(LicqUser::getUserAccountId(userId), LicqUser::getUserProtocolId(userId));
    }
    // If not, validate the uin
    else if (e->m_pUnknownUser->id() == userId)
    {
      gLog.Error("%sInternal Error: Event contains wrong user.\n", L_ERRORxSTR);
      return NULL;
    }

    u = e->m_pUnknownUser;
    u->Lock(LOCK_W);
  }
  gLog.Info(tr("%sReceived %s information for %s (%s).\n"), L_SRVxSTR, t,
      u->GetAlias(), USERID_TOSTR(userId));
  return u;
}

string CICQDaemon::FindUserByCellular(const char *szCellular)
{
  char szParsedNumber1[16], szParsedNumber2[16];
  string id;

  FOR_EACH_USER_START(LOCK_R)
  {
    ParseDigits(szParsedNumber1, pUser->getCellularNumber().c_str(), 15);
    ParseDigits(szParsedNumber2, szCellular, 15);
    if (!strcmp(szParsedNumber1, szParsedNumber2))
      id = pUser->IdString();
  }
  FOR_EACH_USER_END

  return id;
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
  nSequence = BSWAP_16(nSequence);
  nLen = BSWAP_16(nLen);

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
    else if (m_bLoggingOn)
    {
      icqRequestLogonSalt();
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
  unsigned short snacFlags = packet.UnpackUnsignedShortBE(); // flags
  packet.UnpackUnsignedLongBE(); // sequence

  // TODO The TLVs should be processed first, in a common area, instead of requiring
  // each case to do the same thing. However, the individual case's may depend on the tlv
  // coming to them, so leave this commented out for now and do some testing
  if (snacFlags & 0x8000)
  {
    unsigned short bytes = packet.UnpackUnsignedShortBE();
    if (!packet.readTLV(-1, bytes))
    {
      gLog.Error(tr("%sError parsing SNAC header\n"), L_SRVxSTR);
      return;
    }
  }

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

    case ICQ_SNACxSUB_REDIRECT:
    {
      unsigned short nFam = 0;

      if (!packet.readTLV())
      {
        gLog.Warn(tr("%sError during parsing service redirect packet!\n"), L_WARNxSTR);
        break;
      }
      if (packet.getTLVLen(0x000D) == 2)
        nFam = packet.UnpackUnsignedShortTLV(0x000D);

      gLog.Info(tr("%sRedirect for service 0x%02X received.\n"), L_SRVxSTR, nFam);

      char *szServer = packet.UnpackStringTLV(0x0005);
      char *szCookie = packet.UnpackStringTLV(0x0006);
      unsigned short nCookieLen = packet.getTLVLen(0x0006);
      if (!szServer || !szCookie)
      {
        gLog.Warn(tr("%sInvalid servername (%s) or cookie (%s) in service redirect packet!\n"),
                  L_WARNxSTR, szServer ? szServer : "(null)", szCookie ? szCookie : "(null)");
        if (szServer) delete [] szServer;
        if (szCookie) delete [] szCookie;
        break;
      }

      char *szPort = strchr(szServer, ':');
      unsigned short nPort;
      if (szPort)
      {
        *szPort++ = '\0';
        nPort = atoi(szPort);
      }
      else
      {
        nPort = m_nICQServerPort;
      }

      switch (nFam)
      {
        case ICQ_SNACxFAM_BART:
          if (m_xBARTService)
          {
            m_xBARTService->SetConnectCredential(szServer, nPort, szCookie, nCookieLen);
            m_xBARTService->ChangeStatus(STATUS_SERVICE_REQ_ACKED);
            break;
          }
          else
          {
            gLog.Warn(tr("%sService redirect packet for unallocated BART service.\n"),
                      L_WARNxSTR);
            break;
          }

        default:
          gLog.Warn(tr("%sService redirect packet for unhandled service 0x%02X.\n"),
                    L_WARNxSTR, nFam);
      }

      delete [] szServer;
      delete [] szCookie;
      break;
    }

    case ICQ_SNACxSRV_ACKxIMxICQ:
    {
      // const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
      // unsigned long nListTime = o->GetSSTime();
      // gUserManager.DropOwner(o);

      CSrvPacketTcp* p;
      gLog.Info(tr("%sServer sent us channel capability list (ignoring).\n"), L_SRVxSTR);

      gLog.Info(tr("%sRequesting self info.\n"), L_SRVxSTR);
      p = new CPU_GenericFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_GETxUSERxINFO);
      SendEvent_Server(p);

      gLog.Info(tr("%sRequesting list rights.\n"), L_SRVxSTR);
      p = new CPU_ListRequestRights();
      SendExpectEvent_Server(p, NULL);

      gLog.Info(tr("%sRequesting roster rights.\n"), L_SRVxSTR);
      p = new CPU_RequestList();
      SendEvent_Server(p);

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
    unsigned short evil, tlvBlocks;
    unsigned long nUin, realIP;
    time_t nOnlineSince = 0;

    gLog.Info(tr("%sGot Name Info from Server\n"), L_SRVxSTR);

    nUin = packet.UnpackUinString();
    evil = packet.UnpackUnsignedShortBE();
    tlvBlocks = packet.UnpackUnsignedShortBE();

    gLog.Info("%sUIN: %lu Evil: %04hx\n", L_SRVxSTR, nUin, evil);

    if (!packet.readTLV(tlvBlocks)) {
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
      realIP = BE_32(packet.UnpackUnsignedLongTLV(0x000a));
      CPacket::SetRealIp(LE_32(realIP));
      ICQOwner* owner = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      owner->SetIp(realIP);
      gUserManager.DropOwner(owner);

      char buf[32];
      gLog.Info(tr("%sServer says we are at %s.\n"), L_SRVxSTR, ip_ntoa(realIP, buf));
      //icqSetStatus(m_nDesiredStatus);
    }
    if (packet.getTLVLen(0x0003) == 4)
      nOnlineSince = packet.UnpackUnsignedLongTLV(0x0003);

    ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
    unsigned long nPFM = o->PhoneFollowMeStatus();
    // Workaround for the ICQ4.0 problem of it not liking the PFM flags
    m_nDesiredStatus &= ~(ICQ_STATUS_FxPFM | ICQ_STATUS_FxPFMxAVAILABLE);
    if (nPFM != ICQ_PLUGIN_STATUSxINACTIVE)
      m_nDesiredStatus |= ICQ_STATUS_FxPFM;
    if (nPFM == ICQ_PLUGIN_STATUSxACTIVE)
      m_nDesiredStatus |= ICQ_STATUS_FxPFMxAVAILABLE;
    ChangeUserStatus(o, m_nDesiredStatus);
    o->SetOnlineSince(nOnlineSince);
    gLog.Info(tr("%sServer says we're now: %s\n"), L_SRVxSTR, ICQUser::StatusToStatusStr(o->Status(), o->StatusInvisible()));

    gUserManager.DropOwner(o);

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
      gTranslator.ServerToClient(szInfo);
      ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
      u->SetEnableSave(false);
        u->setUserInfoString("About", szInfo);
      delete [] szInfo;

      // translating string with Translation Table

      delete [] szId;
        UserId userId = u->id();

      // save the user infomation
      u->SetEnableSave(true);
        u->saveUserInfo();
      gUserManager.DropUser(u);

      ICQEvent *e = DoneServerEvent(nSubSequence, EVENT_SUCCESS);
      if (e)
        ProcessDoneEvent(e);

        pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_ABOUT, userId));
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
//      userIP = BSWAP_32(userIP);
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
          nUserIP = BSWAP_32(nUserIP);
        nUserIP = LE_32(nUserIP);
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
        intIP = LE_32(intIP);
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
            pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_PLUGIN_STATUS, u->id()));
            break;
          }

      default:
        gLog.Warn(tr("%sUnknown index %d from %s.\n"), L_WARNxSTR, index,
        u->GetAlias());
      }
    }

    if (packet.hasTLV(0x001d))	// Server-stored buddy icon information
    {
      CBuffer BART_info = packet.UnpackTLV(0x001d);
      unsigned short IconType = BART_info.UnpackUnsignedShortBE();
      char HashType = BART_info.UnpackChar();
      char HashLength = BART_info.UnpackChar();
      
      switch (IconType)
      {
        case BART_TYPExBUDDY_ICON_SMALL:
        case BART_TYPExBUDDY_ICON:
        {
          if (HashType == 1 && HashLength > 0 && HashLength <= 16)
          {
            boost::scoped_array<char> Hash(new char[HashLength]);
            boost::scoped_array<char> HashHex(new char[HashLength*2 + 1]);
            
            BART_info.UnpackBinBlock(Hash.get(), HashLength);
            u->SetBuddyIconHash(PrintHex(HashHex.get(), Hash.get(), HashLength));
            u->SetBuddyIconType(IconType);
            u->SetBuddyIconHashType(HashType);
            u->SavePictureInfo();
          }
          break;
        }

        default:	// Unsupported types of BART
          gLog.Warn(tr("%sUnsupported type 0x%02X of buddy icon for %s.\n"),
                    L_WARNxSTR, IconType, u->GetAlias());
          break;
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
      const ICQUser* user = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_R);
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
      pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_TYPING, u->id()));
    gUserManager.DropUser(u); 
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
          const ICQUser* u = gUserManager.FetchUser((*iter)->myIdString.c_str(), LICQ_PPID, LOCK_R);
          if (u == NULL)
            gLog.Warn("%sReverse connection from %s failed.\n", L_WARNxSTR,
                (*iter)->myIdString.c_str());
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
      UserId userId = LicqUser::makeUserId(szId, LICQ_PPID);

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
      
      bool ignore = false;
      // Get the user and allow adding unless we ignore new users
          LicqUser* u = gUserManager.fetchUser(userId, LOCK_W, !Ignore(IGNORE_NEWUSERS));
      if (u == NULL)
      {
          gLog.Info(tr("%sMessage from new user (%s), ignoring.\n"), L_SBLANKxSTR, szId);
          //TODO
          ignore = true;
      }
      else
        gLog.Info(tr("%sMessage through server from %s (%s).\n"), L_SRVxSTR,
          u->GetAlias(), szId);
    
      if (nEncoding == 2) // utf-8 or utf-16?
      {
        const char* szEncoding = ignore ? "" : u->UserEncoding();
        char* szTmpMsg = gTranslator.FromUTF16(szMessage, szEncoding, nMsgLen);
        delete [] szMessage;
        szMessage = szTmpMsg;
      }

      char* szMsg = gTranslator.RNToN(szMessage);
      delete [] szMessage;

          // Unlock user mutex before parsing message so we don't block other threads
          //   for a long time since parser may blocks to prompt for GPG passphrase.
          gUserManager.DropUser(u);

      // now send the message to the user
      CEventMsg *e = CEventMsg::Parse(szMsg, ICQ_CMDxRCV_SYSxMSGxONLINE, nTimeSent, 0);
      delete [] szMsg;

      if (ignore)
      {
            RejectEvent(userId, e);
        break;
      }

          u = gUserManager.fetchUser(userId, LOCK_W, !Ignore(IGNORE_NEWUSERS));
      u->SetTyping(ICQ_TYPING_INACTIVEx0);
      
      if (AddUserEvent(u, e))
        m_xOnEventManager.Do(ON_EVENT_MSG, u);
          pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_TYPING, u->id()));
      gUserManager.DropUser(u);
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

          LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);
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

        char id[16];
        snprintf(id, 15, "%lu", nUin);

        pthread_t t;
        CReverseConnectToUserData *data = new CReverseConnectToUserData(
            id, nId, ICQ_CHNxUNKNOWN, nIp, nPort,
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
            LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);
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
          u = gUserManager.fetchUser(userId, LOCK_W);
      if (u == NULL)
      {
        u = new ICQUser(szId, LICQ_PPID);
        bNewUser = true;
      }

      u->SetTyping(ICQ_TYPING_INACTIVEx0);
      
      if (msgTxt.getTLVLen(0x0004) == 4)
      {
        unsigned long Ip = BE_32(msgTxt.UnpackUnsignedLongTLV(0x0004));
        u->SetIp(Ip);
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

          UserId userId = u->id();
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

          pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_TYPING, userId));
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

              CEventAuthRequest* e = new CEventAuthRequest(userId,
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

              CEventAuthRefused* e = new CEventAuthRefused(userId, szMessage,
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

              LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);
          if (u)
          {
            u->SetAwaitingAuth(false);
            gUserManager.DropUser(u);
          }

              CEventAuthGranted* e = new CEventAuthGranted(userId,
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

              CEventAdded* e = new CEventAdded(userId, szFields[0],
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
              CEventUnknownSysMsg* e = new CEventUnknownSysMsg(nTypeMsg, ICQ_CMDxRCV_SYSxMSGxONLINE,
                  userId, szMessage, nTimeSent, 0);

          ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
          AddUserEvent(o, e);
          gUserManager.DropOwner(o);
        }
      }

      if (eEvent)
	switch(nTypeMsg)
	{
	  case ICQ_CMDxSUB_MSG:
	  case ICQ_CMDxSUB_URL:
	  case ICQ_CMDxSUB_CONTACTxLIST:
	  {
                // Get the user and allow adding unless we ignore new users
                LicqUser* u = gUserManager.fetchUser(userId, LOCK_W, !Ignore(IGNORE_NEWUSERS));
	    if (u == NULL)
	    {
          gLog.Info(tr("%s%s from new user (%s), ignoring.\n"), L_SBLANKxSTR, szType, szId);

          if (szType) free(szType);

          //TODO
              RejectEvent(userId, eEvent);
              break;
	    }
	    else
	      gLog.Info(tr("%s%s through server from %s (%s).\n"), L_SBLANKxSTR,
	    					szType, u->GetAlias(), u->IdString());

            u->SetTyping(ICQ_TYPING_INACTIVEx0);
            
	    if (szType) free(szType);
	    if (AddUserEvent(u, eEvent))
	      m_xOnEventManager.Do(nTypeEvent, u);
                pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_TYPING, u->id()));
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
                const LicqUser* u = gUserManager.fetchUser(userId);
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
                eEvent->AddToHistory(o, D_RECEIVER);
	      gUserManager.DropOwner(o);
	      m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
	    }
	    else
	      gUserManager.DropOwner(o);
	    break;
	  }
	  case ICQ_CMDxSUB_SMS:
	  {
	    CEventSms *eSms = (CEventSms *)eEvent;
      //TODO
      string idSms = FindUserByCellular(eSms->Number());

      if (!idSms.empty())
      {
        //TODO
        ICQUser* u = gUserManager.FetchUser(idSms.c_str(), LICQ_PPID, LOCK_W);
        gLog.Info(tr("%sSMS from %s - %s (%s).\n"), L_SBLANKxSTR, eSms->Number(),
            u->GetAlias(), idSms.c_str());
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
                  eEvent->AddToHistory(o, D_RECEIVER);
	        gUserManager.DropOwner(o);
	        m_xOnEventManager.Do(ON_EVENT_SMS, NULL);
	      }
	      else
	        gUserManager.DropOwner(o);
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
      char id[16];
      snprintf(id, 15, "%lu", nUin);

      u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_W);
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
        if ((*iter)->nId == nMsgID && (*iter)->myIdString == id)
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

    char* szMessage = new char[nLen + 1];
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
      delete [] szMessage;

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
      pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_TYPING, u->id()));
    gUserManager.DropUser(u);
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

  if (nFlags & 0x8000)
  {
    unsigned short bytes = packet.UnpackUnsignedShortBE();
    if (!packet.readTLV(-1, bytes))
    {
      gLog.Error(tr("%sError parsing SNAC header\n"), L_SRVxSTR);
      return;
    }
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
      static unsigned short nCount = 0;
      static bool sCheckExport = false;

      m_bOnlineNotifies = true;

      packet.UnpackChar();  // SSI Version
      unsigned short nPacketCount = packet.UnpackUnsignedShortBE();
      nCount += nPacketCount;

      while (nPacketCount-- != 0)
      {
        // Can't use UnpackUserString because this may be a group name
        char* szId = packet.UnpackStringBE();
        unsigned short nTag = packet.UnpackUnsignedShortBE();
        unsigned short nID = packet.UnpackUnsignedShortBE();
        unsigned short nType = packet.UnpackUnsignedShortBE();

        unsigned short nByteLen = packet.UnpackUnsignedShortBE();

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
            if (!UseServerContactList())
              break;

            std::pair<ContactUserListIter, bool> ret =
              receivedUserList.insert(make_pair(szId, (CUserProperties*)NULL));

            ContactUserListIter iter = ret.first;
            if (ret.second) // we inserted a new NULL pair
              iter->second = new CUserProperties();

            CUserProperties* data = iter->second;

            TLVList list = packet.getTLVList();
            for (TLVListIter it = list.begin(); it != list.end(); it++)
              data->tlvs[it->first] = it->second;

#define COPYTLV(type, var) \
            if (packet.hasTLV(type)) \
              data->var.reset(packet.UnpackStringTLV(type))

            COPYTLV(0x0131, newAlias);
            COPYTLV(0x013A, newCellular);
#undef COPYTLV
            if (packet.hasTLV(0x0066))
              data->awaitingAuth = true;

            if (nTag != 0)
              data->groupId = nTag;

            if (nType == ICQ_ROSTxIGNORE)
              data->inIgnoreList = true;

            if (nID != 0)
            {
              if (nType == ICQ_ROSTxVISIBLE)
              {
                data->visibleSid = nID;
              }
              else if (nType == ICQ_ROSTxINVISIBLE)
              {
                data->invisibleSid = nID;
              }
              else
              {
                data->normalSid = nID;
              }
            }
            break;
          }

          case ICQ_ROSTxGROUP:
          {
            if (!UseServerContactList()) break; 

            if (szId[0] != '\0' && nTag != 0)
            {
              // Rename the group if we have it already or else add it
              unsigned short nGroup = gUserManager.GetGroupFromID(nTag);
              char* szUnicodeName = gTranslator.FromUnicode(szId);

              if (nGroup == 0)
              {
                if (!gUserManager.AddGroup(szUnicodeName, nTag))
                  gUserManager.ModifyGroupID(szUnicodeName, nTag);
              }
              else
              {
                gUserManager.RenameGroup(nGroup, szUnicodeName, false);
              }
              
              if (szUnicodeName)
                delete[] szUnicodeName;

              // This is bad, i don't think we want to call this at all..
              // it will add users to different groups that they werent even
              // assigned to
              //if (gUserManager.UpdateUsersInGroups())
              //{
              //  pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST,
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
            gUserManager.DropOwner(o);
            break;
          }
        }  // switch (nType)

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
      unsigned long nTime = packet.UnpackUnsignedLongBE();
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      o->SetSSTime(nTime);
      o->SetSSCount(nCount);
      gUserManager.DropOwner(o);

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
        DoneServerEvent(nSubSequence, EVENT_SUCCESS);

        gLog.Info(tr("%sReceived end of contact list.\n"), L_SRVxSTR);

        ProcessUserList();

        gLog.Info(tr("%sActivating server contact list.\n"), L_SRVxSTR);
        CSrvPacketTcp *p = new CPU_GenericFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxACK);
        SendEvent_Server(p);
      }

      break;
    } // case rost reply

    case ICQ_SNACxLIST_ROSTxUPD_GROUP:
    {
      gLog.Info(tr("%sReceived updated contact information from server.\n"), L_SRVxSTR);

      char *szId = packet.UnpackStringBE();
      if (szId == 0)
      {
        gLog.Error(tr("%sDid not receive user ID.\n"), L_SRVxSTR);
        break;
      }

      unsigned short gsid    = packet.UnpackUnsignedShortBE();
      unsigned short sid     = packet.UnpackUnsignedShortBE();
      /*unsigned short classid =*/ packet.UnpackUnsignedShortBE();

      unsigned short tlvBytes = packet.UnpackUnsignedShortBE();

      if (!packet.readTLV(-1, tlvBytes))
      {
        gLog.Error(tr("%sError during parsing packet!\n"), L_ERRORxSTR);
        break;
      }

      ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
      if (u)
      {
        // First update their gsid/sid
        u->SetSID(sid);
        u->RemoveFromGroup(GROUPS_USER, gUserManager.GetGroupFromID(u->GetGSID()));
        u->SetGSID(gsid);
        u->AddToGroup(GROUPS_USER, gUserManager.GetGroupFromID(gsid));

        // Now the the tlv of attributes to attach to the user
        TLVList tlvList = packet.getTLVList();
        TLVListIter iter;
        for (iter = tlvList.begin(); iter != tlvList.end(); ++iter)
        {
          TLVPtr tlv = iter->second;
          u->AddTLV(tlv);
        }

        u->SaveLicqInfo();
        pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_GENERAL, u->id()));
        gUserManager.DropUser(u);
      }

      delete [] szId;

      break;
    }

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
      gUserManager.DropOwner(o);

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


              LicqGroup* group = gUserManager.FetchGroup(n, LOCK_R);
              std::string groupName;
              if (e->ExtraInfo() == 0 || group == NULL)
                groupName = ""; // top level
              else
                groupName = group->name();
              if (group != NULL)
                gUserManager.DropGroup(group);

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
              SendExpectEvent_Server(pReply, NULL);

              // Finish editing server list
              CSrvPacketTcp *pEnd = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
                                                          ICQ_SNACxLIST_ROSTxEDITxEND);
              SendEvent_Server(pEnd);
            }


            // Skip the call to gUserManager.AddUserToGroup because
            // that will send a message out to the server AGAIN
            if (e->SubType() == ICQ_SNACxLIST_ROSTxADD && !bTopLevelUpdated)
            {
              ICQUser* u = gUserManager.FetchUser(pending.c_str(), LICQ_PPID, LOCK_W);
              if (u)
              {
                u->AddToGroup(GROUPS_USER, gUserManager.GetGroupFromID(
                              e->ExtraInfo()));
                pushPluginSignal(new LicqSignal(SIGNAL_ADDxSERVERxLIST, 0, u->id()));
                gUserManager.DropUser(u);
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
                  pending.empty() ? tr("top level") : pending.c_str());
                
              if (nError == 0x0E)
              {
                pReply = new CPU_UpdateToServerList(pending.c_str(),
                  ICQ_ROSTxNORMAL, 0, true);
                addToModifyUsers(pReply->SubSequence(), pending);
                SendExpectEvent_Server(pReply, NULL);
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
      UserId userId = LicqUser::makeUserId(szId, LICQ_PPID);
      const LicqUser* u = gUserManager.fetchUser(userId);
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

      CEventAuthRequest* e = new CEventAuthRequest(userId, "", "", "", "", nMsgLen ? szMsg : "",
                                                   ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);

      ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      if (AddUserEvent(o, e))
      {
        e->AddToHistory(o, D_RECEIVER);
        gUserManager.DropOwner(o);
        m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
      }
      else
        gUserManager.DropOwner(o);

      delete [] szId;
      delete [] szMsg;
      break;
    }

    case ICQ_SNACxLIST_AUTHxRESPONS: // The resonse to our authorization request
    {
      char *szId = packet.UnpackUserString();
      UserId userId = LicqUser::makeUserId(szId, LICQ_PPID);
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
         eEvent = new CEventAuthGranted(userId, szMsg,
           ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);

         LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);
         if (u)
         {
           u->SetAwaitingAuth(false);
           u->RemoveTLV(0x0066);
           gUserManager.DropUser(u);
         }
      }
      else
      {
        eEvent = new CEventAuthRefused(userId, szMsg,
            ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);
      }

      ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      if (AddUserEvent(o, eEvent))
      {
        eEvent->AddToHistory(o, D_RECEIVER);
        gUserManager.DropOwner(o);
        m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
      }
      else
        gUserManager.DropOwner(o);

      delete [] szId;
      delete [] szMsg;
      break;
    }

    case ICQ_SNACxLIST_AUTHxADDED: // You were added to a contact list
    {
      char *szId = packet.UnpackUserString();
      UserId userId = LicqUser::makeUserId(szId, LICQ_PPID);
      gLog.Info(tr("%sUser %s added you to their contact list.\n"), L_SRVxSTR,
                szId);

      CEventAdded* e = new CEventAdded(userId, "", "", "", "",
                                       ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);
      ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      if (AddUserEvent(o, e))
      {
        e->AddToHistory(o, D_RECEIVER);
        gUserManager.DropOwner(o);
        m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
      }
      else
        gUserManager.DropOwner(o);

      delete [] szId;
      break;
    }

    case ICQ_SNACxLIST_ROSTxEDITxSTART:
    case ICQ_SNACxLIST_ROSTxEDITxEND:
    {
      // Nothing to be done here.
      // Server just encloses contact list changes with these.
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
      pushPluginSignal(new LicqSignal(SIGNAL_LOGON, 0));

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
          char id[16];
          snprintf(id, 16, "%lu", nUin);
          UserId userId = LicqUser::makeUserId(id, LICQ_PPID);

      sendTM.tm_year = msg.UnpackUnsignedShort() - 1900;
      sendTM.tm_mon = msg.UnpackChar() - 1;
      sendTM.tm_mday = msg.UnpackChar();
      sendTM.tm_hour = msg.UnpackChar();
      sendTM.tm_min = msg.UnpackChar();
      sendTM.tm_sec = 0;
      sendTM.tm_isdst = -1;

          const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
      nTimeSent = mktime(&sendTM) - o->SystemTimeGMTOffset();
          gUserManager.DropOwner(o);

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
              gLog.Info(tr("%sOffline authorization request from %s.\n"), L_SBLANKxSTR, id);

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

              CEventAuthRequest* e = new CEventAuthRequest(userId,
                  szFields[0], szFields[1], szFields[2], szFields[3],
                  szFields[5], ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, 0);
              delete [] szFields;
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_AUTHxREFUSED:  // system message: authorization refused
        {
              gLog.Info(tr("%sOffline authorization refused by %s.\n"), L_SBLANKxSTR, id);

          // Translating string with Translation Table
          gTranslator.ServerToClient(szMessage);

              CEventAuthRefused* e = new CEventAuthRefused(userId,
                  szMessage, ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, 0);
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_AUTHxGRANTED:  // system message: authorized
        {
              gLog.Info(tr("%sOffline authorization granted by %s.\n"), L_SBLANKxSTR, id);

          // translating string with Translation Table
          gTranslator.ServerToClient (szMessage);

              LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);
          if (u)
          {
            u->SetAwaitingAuth(false);
            gUserManager.DropUser(u);
          }

              CEventAuthGranted* e = new CEventAuthGranted(userId,
                  szMessage, ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, 0);
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
              gLog.Info(tr("%sUser %s added you to their contact list (offline).\n"), L_SBLANKxSTR, id);

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

              CEventAdded* e = new CEventAdded(userId, szFields[0],
                  szFields[1], szFields[2], szFields[3],
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
              CEventUnknownSysMsg* e = new CEventUnknownSysMsg(nTypeMsg,
                  ICQ_CMDxRCV_SYSxMSGxOFFLINE, userId, szMessage, nTimeSent, 0);

              ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
          AddUserEvent(o, e);
              gUserManager.DropOwner(o);
	}
      }

      if (eEvent)
        switch(nTypeMsg)
        {
	  case ICQ_CMDxSUB_MSG:
	  case ICQ_CMDxSUB_URL:
	  case ICQ_CMDxSUB_CONTACTxLIST:
	  {
                // Get the user and allow adding unless we ignore new users
                LicqUser* u = gUserManager.fetchUser(userId, LOCK_W, !Ignore(IGNORE_NEWUSERS));
                if (u == NULL)
                {
                    gLog.Info(tr("%sOffline %s from new user (%s), ignoring.\n"),
                        L_SBLANKxSTR, szType, id);
                    if (szType) free(szType);
                    RejectEvent(userId, eEvent);
                    break;
                }
                else
                  gLog.Info(tr("%sOffline %s through server from %s (%s).\n"),
                      L_SBLANKxSTR, szType, u->GetAlias(), id);

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
                const LicqUser* u = gUserManager.fetchUser(userId);
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

                ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
            if (AddUserEvent(o, eEvent))
	    {
                  eEvent->AddToHistory(o, D_RECEIVER);
                  gUserManager.DropOwner(o);
              m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
	    }
	    else
                  gUserManager.DropOwner(o);
            break;
	  }
	  case ICQ_CMDxSUB_SMS:
	  {
                CEventSms* eSms = (CEventSms *)eEvent;
                string idSms = FindUserByCellular(eSms->Number());

                if (!idSms.empty())
                {
                  ICQUser* u = gUserManager.FetchUser(idSms.c_str(), LICQ_PPID, LOCK_W);
                  gLog.Info(tr("%sOffline SMS from %s - %s (%s).\n"), L_SBLANKxSTR,
                      eSms->Number(), u->GetAlias(), id);
	      if (AddUserEvent(u, eEvent))
	        m_xOnEventManager.Do(ON_EVENT_SMS, u);
	      gUserManager.DropUser(u);
	    }
	    else
                {
                  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
	      gLog.Info(tr("%sOffline SMS from %s.\n"), L_BLANKxSTR, eSms->Number());
	      if (AddUserEvent(o, eEvent))
	      {
	            eEvent->AddToHistory(o, D_RECEIVER);
                    gUserManager.DropOwner(o);
	        m_xOnEventManager.Do(ON_EVENT_SMS, NULL);
	      }
	      else
                    gUserManager.DropOwner(o);
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
              ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
          o->SetEnableSave(false);
          o->SetPassword(((CPU_SetPassword *)pEvent->m_pPacket)->m_szPassword);
          o->SetEnableSave(true);
          o->SaveLicqInfo();
              gUserManager.DropOwner(o);
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

              // translating string with Translation Table
              gTranslator.ServerToClient(p->m_szAlias);
              gTranslator.ServerToClient(p->m_szFirstName);
              gTranslator.ServerToClient(p->m_szLastName);
              gTranslator.ServerToClient(p->m_szEmailPrimary);
              gTranslator.ServerToClient(p->m_szCity);
              gTranslator.ServerToClient(p->m_szState);
              gTranslator.ServerToClient(p->m_szPhoneNumber);
              gTranslator.ServerToClient(p->m_szFaxNumber);
              gTranslator.ServerToClient(p->m_szAddress);
              gTranslator.ServerToClient(p->m_szCellularNumber);
              gTranslator.ServerToClient(p->m_szZipCode);

              ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
              o->SetEnableSave(false);
              o->setAlias(p->m_szAlias);
              o->setUserInfoString("FirstName", p->m_szFirstName);
              o->setUserInfoString("LastName", p->m_szLastName);
              o->setUserInfoString("Email1", p->m_szEmailPrimary);
              o->setUserInfoString("City", p->m_szCity);
              o->setUserInfoString("State", p->m_szState);
              o->setUserInfoString("PhoneNumber", p->m_szPhoneNumber);
              o->setUserInfoString("FaxNumber", p->m_szFaxNumber);
              o->setUserInfoString("Address", p->m_szAddress);
              o->setUserInfoString("CellularNumber", p->m_szCellularNumber);
              o->setUserInfoString("Zipcode", p->m_szZipCode);
              o->setUserInfoUint("Country", p->m_nCountryCode);
          o->SetTimezone(p->m_nTimezone);
              o->setUserInfoBool("HideEmail", p->m_nHideEmail); // 0 = no, 1 = yes

          // save the user infomation
          o->SetEnableSave(true);
              o->saveUserInfo();
              gUserManager.DropOwner(o);
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

              // translating string with Translation Table
              gTranslator.ServerToClient(p->m_szEmailSecondary);
              gTranslator.ServerToClient(p->m_szEmailOld);

              ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);

          o->SetEnableSave(false);
              o->setUserInfoString("Email2", p->m_szEmailSecondary);
              o->setUserInfoString("Email0", p->m_szEmailOld);

          // save the user infomation
          o->SetEnableSave(true);
              o->saveUserInfo();
              gUserManager.DropOwner(o);
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

              // translating string with Translation Table
              gTranslator.ServerToClient(p->m_szHomepage);

              ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
          o->SetEnableSave(false);
              o->setUserInfoUint("Age", p->m_nAge);
              o->setUserInfoUint("Gender", p->m_nGender);
              o->setUserInfoString("Homepage", p->m_szHomepage);
              o->setUserInfoUint("BirthYear", p->m_nBirthYear);
              o->setUserInfoUint("BirthMonth", p->m_nBirthMonth);
              o->setUserInfoUint("BirthDay", p->m_nBirthDay);
              o->setUserInfoUint("Language1", p->m_nLanguage1);
              o->setUserInfoUint("Language2", p->m_nLanguage2);
              o->setUserInfoUint("Language3", p->m_nLanguage3);

          // save the user infomation
          o->SetEnableSave(true);
              o->saveUserInfo();
              gUserManager.DropOwner(o);
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
              ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
          o->SetEnableSave(false);
              o->getInterests().clear();
              UserCategoryMap::iterator i;
              for (i = p->myInterests.begin(); i != p->myInterests.end(); ++i)
              {
                char* tmp = strdup(i->second.c_str());
                gTranslator.ServerToClient(tmp);
                o->getInterests()[i->first] = tmp;
            free(tmp);
          }
          o->SetEnableSave(true);
              o->saveUserInfo();
              gUserManager.DropOwner(o);
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

              // translating string with Translation Table
              gTranslator.ServerToClient(p->m_szCity);
              gTranslator.ServerToClient(p->m_szState);
              gTranslator.ServerToClient(p->m_szPhoneNumber);
              gTranslator.ServerToClient(p->m_szFaxNumber);
              gTranslator.ServerToClient(p->m_szAddress);
              gTranslator.ServerToClient(p->m_szZip);
              gTranslator.ServerToClient(p->m_szName);
              gTranslator.ServerToClient(p->m_szDepartment);
              gTranslator.ServerToClient(p->m_szPosition);
              gTranslator.ServerToClient(p->m_szHomepage);

              ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
          o->SetEnableSave(false);
              o->setUserInfoString("CompanyCity", p->m_szCity);
              o->setUserInfoString("CompanyState", p->m_szState);
              o->setUserInfoString("CompanyPhoneNumber", p->m_szPhoneNumber);
              o->setUserInfoString("CompanyFaxNumber", p->m_szFaxNumber);
              o->setUserInfoString("CompanyAddress", p->m_szAddress);
              o->setUserInfoString("CompanyZip", p->m_szZip);
              o->setUserInfoUint("CompanyCountry", p->m_nCompanyCountry);
              o->setUserInfoString("CompanyName", p->m_szName);
              o->setUserInfoString("CompanyDepartment", p->m_szDepartment);
              o->setUserInfoString("CompanyPosition", p->m_szPosition);
              o->setUserInfoUint("CompanyOccupation", p->m_nCompanyOccupation);
              o->setUserInfoString("CompanyHomepage", p->m_szHomepage);

          // save the user infomation
          o->SetEnableSave(true);
              o->saveUserInfo();
              gUserManager.DropOwner(o);
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
              ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
          char* msg = gTranslator.RNToN(p->m_szAbout);
          gTranslator.ServerToClient(msg);
          o->SetEnableSave(false);
              o->setUserInfoString("About", msg);
          delete [] msg;

          // save the user infomation
          o->SetEnableSave(true);
              o->saveUserInfo();
              gUserManager.DropOwner(o);
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
                o->getOrganizations().clear();
                UserCategoryMap::iterator i;
                for (i = p->myOrganizations.begin(); i != p->myOrganizations.end(); ++i)
                {
                  char *tmp = strdup(i->second.c_str());
              gTranslator.ServerToClient(tmp);
                  o->getOrganizations()[i->first] = tmp;
              free(tmp);
            }

                o->getBackgrounds().clear();
                for (i = p->myBackgrounds.begin(); i != p->myBackgrounds.end(); ++i)
                {
                  char *tmp = strdup(i->second.c_str());
              gTranslator.ServerToClient(tmp);
                  o->getBackgrounds()[i->first] = tmp;
              free(tmp);
            }
            o->SetEnableSave(true);
                o->saveUserInfo();
                gUserManager.DropOwner(o);
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
              ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
          o->SetRandomChatGroup(((CPU_SetRandomChatGroup *)pEvent->m_pPacket)->Group());
              gUserManager.DropOwner(o);
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
              UserId userId = LicqUser::makeUserId(szUin, LICQ_PPID);
          gLog.Info(tr("%sRandom chat user found (%s).\n"), L_SRVxSTR, szUin);
          bool bNewUser = false;
              LicqUser* u = gUserManager.fetchUser(userId, LOCK_W, true, &bNewUser);

          msg.UnpackUnsignedShort(); // chat group

          nIp = LE_32(msg.UnpackUnsignedLongBE());
          u->SetIpPort(nIp, msg.UnpackUnsignedLong());

          nIp = LE_32(msg.UnpackUnsignedLongBE());
          u->SetIntIp(nIp);

          msg >> nMode;
          u->SetMode(nMode);
          if (nMode != MODE_DIRECT)
            u->SetSendServer(true);

          u->SetVersion(msg.UnpackUnsignedShort());

          gUserManager.DropUser(u);

          if (bNewUser)
            icqRequestMetaInfo(szUin);

              e->m_pSearchAck = new CSearchAck(userId);
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
        char foundAccount[14];
        snprintf(foundAccount, sizeof(foundAccount), "%lu", nFoundUin);
            UserId foundUserId = LicqUser::makeUserId(foundAccount, LICQ_PPID);

            CSearchAck* s = new CSearchAck(foundUserId);

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
        UserId userId = USERID_NONE;
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
          userId = e->userId();

          u = FindUserForInfoUpdate(userId, e, "extended");
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
          tmp = msg.UnpackString();
          // Skip the alias if user wants to keep his own.
          if (!u->m_bKeepAliasOnUpdate || userId == gUserManager.ownerUserId(LICQ_PPID))
          {
            char *szUTFAlias = tmp ? gTranslator.ToUnicode(tmp, u->UserEncoding()) : 0;
            gTranslator.ServerToClient(szUTFAlias);
                  u->setAlias(szUTFAlias);
            //printf("Alias: %s\n", szUTFAlias);
          }
          if (tmp)
            delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("FirstName", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("LastName", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("Email1", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("City", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("State", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("PhoneNumber", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("FaxNumber", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("Address", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("CellularNumber", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("Zipcode", tmp);
          delete[] tmp;
                u->setUserInfoUint("Country", msg.UnpackUnsignedShort() );
          u->SetTimezone( msg.UnpackChar() );
          u->SetAuthorization( !msg.UnpackChar() );
          unsigned char nStatus = msg.UnpackChar(); // Web aware status

                if (!u->User())
                {
            static_cast<ICQOwner *>(u)->SetWebAware(nStatus);
            /* this unpack is inside the if statement since it appears only
               for the owner request */
                  u->setUserInfoBool("HideEmail", msg.UnpackChar());
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

          // save the user infomation
          u->SetEnableSave(true);
                u->saveUserInfo();

          PushExtendedEvent(e);
          multipart = true;

                pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_GENERAL, u->id()));
                break;
              }
        case ICQ_CMDxMETA_MORExINFO:
        {
          gLog.Info(tr("%sMore info on %s (%s).\n"), L_SRVxSTR, u->GetAlias(), u->IdString());

          u->SetEnableSave(false);
                u->setUserInfoUint("Age", msg.UnpackUnsignedShort());
                u->setUserInfoUint("Gender", msg.UnpackChar());
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("Homepage", tmp);
          delete[] tmp;
                u->setUserInfoUint("BirthYear", msg.UnpackUnsignedShort());
                u->setUserInfoUint("BirthMonth", msg.UnpackChar());
                u->setUserInfoUint("BirthDay", msg.UnpackChar());
                u->setUserInfoUint("Language1", msg.UnpackChar());
                u->setUserInfoUint("Language2", msg.UnpackChar());
                u->setUserInfoUint("Language3", msg.UnpackChar());

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

          // save the user infomation
          u->SetEnableSave(true);
                u->saveUserInfo();

          PushExtendedEvent(e);
          multipart = true;

                pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_MORE, u->id()));
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
              gTranslator.ServerToClient(tmp);
            }
            else
            {
              tmp = new char[1];
              tmp[0] = '\0';
            }

            if(i == 0)
                    u->setUserInfoString("Email2", tmp);
            else if(i == 1)
                    u->setUserInfoString("Email0", tmp);

            delete[] tmp;
          }

          // save the user infomation
          u->SetEnableSave(true);
                u->saveUserInfo();

          PushExtendedEvent(e);
          multipart = true;

                pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_EXT, u->id()));
                break;
              }
        case ICQ_CMDxMETA_HOMEPAGExINFO:
        {
          gLog.Info("%sHomepage info on %s (%s).\n", L_SRVxSTR, u->GetAlias(),
            u->IdString());          
          
          u->SetEnableSave(false);
          
          unsigned char categoryPresent = msg.UnpackChar();
                u->setUserInfoBool("HomepageCatPresent", categoryPresent);

          if (categoryPresent)
          {
                  u->setUserInfoUint("HomepageCatCode", msg.UnpackUnsignedShort());

            char *rawmsg = msg.UnpackString();
            char *msg = gTranslator.RNToN(rawmsg);
            delete [] rawmsg;

            gTranslator.ServerToClient(msg);
                  u->setUserInfoString("HomepageDesc", msg);
            delete [] msg;
          }

                u->setUserInfoBool("ICQHomepagePresent", msg.UnpackChar());

          // save the user infomation
          u->SetEnableSave(true);
                u->saveUserInfo();

          PushExtendedEvent(e);
          multipart = true;

                pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_HP, u->id()));
          break;
        }

        case ICQ_CMDxMETA_WORKxINFO:
          gLog.Info(tr("%sWork info on %s (%s).\n"), L_SRVxSTR, u->GetAlias(), u->IdString());

          u->SetEnableSave(false);
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("CompanyCity", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("CompanyState", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("CompanyPhoneNumber", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("CompanyFaxNumber", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("CompanyAddress", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("CompanyZip", tmp);
          delete[] tmp;
                u->setUserInfoUint("CompanyCountry", msg.UnpackUnsignedShort());
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("CompanyName", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("CompanyDepartment", tmp);
          delete[] tmp;
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("CompanyPosition", tmp);
          delete[] tmp;
                u->setUserInfoUint("CompanyOccupation", msg.UnpackUnsignedShort());
          gTranslator.ServerToClient( tmp = msg.UnpackString() );
                u->setUserInfoString("CompanyHomepage", tmp);
          delete[] tmp;

          // save the user infomation
          u->SetEnableSave(true);
                u->saveUserInfo();

          PushExtendedEvent(e);
          multipart = true;

                pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_WORK, u->id()));
          break;

        case ICQ_CMDxMETA_ABOUT:
        {
          gLog.Info(tr("%sAbout info on %s (%s).\n"), L_SRVxSTR, u->GetAlias(), u->IdString());

          char* rawmsg = msg.UnpackString();
          char* msg = gTranslator.RNToN(rawmsg);
          delete [] rawmsg;
          gTranslator.ServerToClient(msg);
          u->SetEnableSave(false);
                u->setUserInfoString("About", msg);
          delete [] msg;

          // save the user infomation
          u->SetEnableSave(true);
                u->saveUserInfo();

          PushExtendedEvent(e);
          multipart = true;

                pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_ABOUT, u->id()));
                break;
              }
        case ICQ_CMDxMETA_INTERESTSxINFO:
        {
          unsigned i, n;

          gLog.Info("%sPersonal Interests info on %s (%s).\n", L_SRVxSTR,
                    u->GetAlias(), u->IdString());

          u->SetEnableSave(false);
                u->getInterests().clear();
          n = msg.UnpackChar();

                for (i = 0; i < n; ++i)
                {
            unsigned short cat = msg.UnpackUnsignedShort();
            tmp = msg.UnpackString();
            gTranslator.ServerToClient(tmp);
                  u->getInterests()[cat] = tmp;
            delete [] tmp;
          }

          // save the user infomation
          u->SetEnableSave(true);
                u->saveUserInfo();

          PushExtendedEvent(e);
          multipart = true;

                pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_MORE2, u->id()));
                break;
              }
        case ICQ_CMDxMETA_PASTxINFO:
        {
          // past background info - last one received
          unsigned i, n;

          gLog.Info("%sOrganizations/Past Background info on %s (%s).\n",
                    L_SRVxSTR, u->GetAlias(), u->IdString());

          u->SetEnableSave(false);

                u->getBackgrounds().clear();

          n = msg.UnpackChar();

                for (i = 0; i < n; ++i)
                {
            unsigned short cat = msg.UnpackUnsignedShort();
            tmp = msg.UnpackString();
            gTranslator.ServerToClient(tmp);
                  u->getBackgrounds()[cat] = tmp;
            delete [] tmp;
          }

          //---- Organizations
                u->getOrganizations().clear();
          n = msg.UnpackChar();

                for (i = 0; i < n; ++i)
          {
            unsigned short cat = msg.UnpackUnsignedShort();
            tmp = msg.UnpackString();
            gTranslator.ServerToClient(tmp);
                  u->getOrganizations()[cat] = tmp;
            delete [] tmp;
          }

          // our user info is now up to date
          u->SetOurClientTimestamp(u->ClientTimestamp());

          // save the user infomation
          u->SetEnableSave(true);
                u->saveUserInfo();
          u->SaveLicqInfo();

                pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_MORE2, u->id()));
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
                      L_WARNxSTR, u->GetAlias(), USERID_TOSTR(userId));
          }
        }

            pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_EXT, u->id()));
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

//--------ProcessAuthFam----------------------------------------------------
void CICQDaemon::ProcessAuthFam(CBuffer &packet, unsigned short nSubtype)
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

      if (!gUserManager.OwnerId(LICQ_PPID).empty())
      {
        gLog.Warn(tr("%sReceived new uin (%lu) when already have a uin (%s).\n"), L_WARNxSTR,
          nNewUin, gUserManager.OwnerId(LICQ_PPID).c_str());
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
        gUserManager.DropOwner(o);
        free(m_szRegisterPasswd);
        m_szRegisterPasswd = 0;
        SaveConf();
      }

      pushPluginSignal(new LicqSignal(SIGNAL_NEW_OWNER, 0, USERID_NONE, LICQ_PPID));

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

    case ICQ_SNACxAUTHxSALT_REPLY:
    {
      char *md5Salt = packet.UnpackStringBE();
      const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
      CPU_NewLogon *p = new CPU_NewLogon(o->Password(), o->IdString(), md5Salt);
      gUserManager.DropOwner(o);
      gLog.Info(tr("%sSending md5 hashed password.\n"), L_SRVxSTR);
      SendEvent_Server(p);
      delete [] md5Salt;
      m_bNeedSalt = false;
      break;
    }

    case ICQ_SNACxAUTHxLOGON_REPLY:
    {
      ProcessCloseChannel(packet);
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
      pushPluginSignal(new LicqSignal(SIGNAL_VERIFY_IMAGE, 0, USERID_NONE, LICQ_PPID));
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

void CICQDaemon::ProcessUserList()
{
  if (receivedUserList.empty())
    return;

  ContactUserListIter iter;

  for (iter = receivedUserList.begin(); iter != receivedUserList.end(); iter++)
  {
    UserId userId = LicqUser::makeUserId(iter->first, LICQ_PPID);
    CUserProperties* data = iter->second;

    if (!USERID_ISVALID(userId))
    {
      gLog.Warn(tr("%sEmpty User ID was received in the contact list.\n"),
          L_SRVxSTR);
      continue;
    }

    bool isOnList = gUserManager.userExists(userId);

    if (!isOnList)
    {
      gUserManager.addUser(userId, true, false, gUserManager.GetGroupFromID(data->groupId)); // Don't notify server
      gLog.Info(tr("%sAdded %s (%s) to list from server.\n"),
          L_SRVxSTR, (data->newAlias ? data->newAlias.get() : USERID_TOSTR(userId)), USERID_TOSTR(userId));
    }

    LicqUserWriteGuard u(userId);
    if (!u.isLocked())
      continue;

    // For now, just save all the TLVs. We should change this to have awaiting auth check
    // for the 0x0066 TLV, SMS number if it has the 0x013A TLV, etc
    u->SetTLVList(data->tlvs);

    if (data->newAlias != NULL && !u->m_bKeepAliasOnUpdate)
      u->setAlias(data->newAlias.get());

    u->SetSID(data->normalSid);
    u->SetGSID(data->groupId);
    u->SetVisibleSID(data->visibleSid);
    u->SetVisibleList(data->visibleSid != 0);
    u->SetInvisibleSID(data->invisibleSid);
    u->SetInvisibleList(data->invisibleSid != 0);
    u->SetIgnoreList(data->inIgnoreList);

    if (isOnList)
      u->AddToGroup(GROUPS_USER, gUserManager.GetGroupFromID(data->groupId));

    u->SetAwaitingAuth(data->awaitingAuth);

    if (!isOnList)
    {
      // They aren't a new user if we added them to a server list
      u->SetNewUser(false);
    }

    if (data->newCellular != NULL)
    {
      char* tmp = gTranslator.FromUnicode(data->newCellular.get());
      if (tmp != NULL)
      {
        u->setUserInfoString("CellularNumber", tmp);
        delete[] tmp;
      }
    }

    // Save GSID, SID and group memberships
    u->SaveLicqInfo();
    pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_GENERAL, u->id()));
  }

  receivedUserList.clear();
}

//--------ProcessDataChannel---------------------------------------------------

void CICQDaemon::ProcessDataChannel(CBuffer &packet)
{
  unsigned short nFamily, nSubtype;

  packet >> nFamily >> nSubtype;
  nFamily = BSWAP_16(nFamily);
  nSubtype = BSWAP_16(nSubtype);

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

  case ICQ_SNACxFAM_AUTH:
    ProcessAuthFam(packet, nSubtype);
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
      pushPluginSignal(new LicqSignal(SIGNAL_LOGOFF, LOGOFF_RATE, gUserManager.ownerUserId(LICQ_PPID), LICQ_PPID));
      break;

  case 0x04:
  case 0x05:
    gLog.Error(tr("%sInvalid UIN and password combination.\n"), L_ERRORxSTR);
      pushPluginSignal(new LicqSignal(SIGNAL_LOGOFF, LOGOFF_PASSWORD, gUserManager.ownerUserId(LICQ_PPID), LICQ_PPID));
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

int CICQDaemon::RequestReverseConnection(const char* id,
                                         unsigned long nData,
                                         unsigned long nLocalIP,
                                         unsigned short nLocalPort,
                                         unsigned short nRemotePort)
{
  if (id == gUserManager.OwnerId(LICQ_PPID))
    return -1;

  ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_W);
  if (u == NULL) return -1;


  CPU_ReverseConnect *p = new CPU_ReverseConnect(u, nLocalIP, nLocalPort,
                                                 nRemotePort);
  int nId = p->SubSequence();

  pthread_mutex_lock(&mutex_reverseconnect);

  m_lReverseConnect.push_back(new CReverseConnectToUserData(id, nId, nData,
      nLocalIP, nLocalPort, ICQ_VERSION_TCP, nRemotePort, 0, nId));
  pthread_mutex_unlock(&mutex_reverseconnect);

  gLog.Info("%sRequesting reverse connection from %s.\n", L_TCPxSTR,
            u->GetAlias());
  SendEvent_Server(p);
  
  gUserManager.DropUser(u);

  return nId;
}
 
