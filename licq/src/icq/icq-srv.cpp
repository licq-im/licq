// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#include "config.h"

#include "icq.h"

#include <boost/foreach.hpp>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>

#include <boost/scoped_array.hpp>

#include "licq/byteorder.h"
#include <licq/event.h>
#include <licq/icqchat.h>
#include <licq/icqfiletransfer.h>
#include <licq/oneventmanager.h>
#include <licq/pluginsignal.h>
#include <licq/proxy.h>
#include <licq/socket.h>
#include <licq/statistics.h>
#include <licq/translator.h>
#include <licq/userevents.h>
#include "licq_log.h"
#include "licq/version.h"

#include "../contactlist/owner.h"
#include "../contactlist/user.h"
#include "../contactlist/usermanager.h"
#include "../daemon.h"
#include "../gettext.h"
#include "../support.h"
#include "oscarservice.h"
#include "packet.h"

using namespace std;
using Licq::OnEventManager;
using Licq::StringList;
using Licq::gOnEventManager;
using Licq::gTranslator;
using LicqDaemon::Daemon;
using LicqDaemon::Owner;
using LicqDaemon::User;
using LicqDaemon::gDaemon;
using LicqDaemon::gUserManager;

//-----icqAddUser----------------------------------------------------------
void IcqProtocol::icqAddUser(const Licq::UserId& userId, bool _bAuthRequired, unsigned short groupId)
{
  CSrvPacketTcp *p = new CPU_GenericUinList(userId.accountId().c_str(), ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
  gLog.Info(tr("%sAlerting server to new user (#%hu)...\n"), L_SRVxSTR,
             p->Sequence());
  SendExpectEvent_Server(userId, p, NULL);

  // Server side list add, and update of group
  if (UseServerContactList())
    icqAddUserServer(userId, _bAuthRequired, groupId);

  icqUserBasicInfo(userId);
}

//-----icqAddUserServer--------------------------------------------------------
void IcqProtocol::icqAddUserServer(const Licq::UserId& userId, bool _bAuthRequired,
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

  CPU_AddToServerList *pAdd = new CPU_AddToServerList(userId.accountId().c_str(), ICQ_ROSTxNORMAL,
    groupId, _bAuthRequired);
  gLog.Info(tr("%sAdding %s to server list...\n"), L_SRVxSTR, userId.accountId().c_str());
  addToModifyUsers(pAdd->SubSequence(), userId.accountId());
  SendExpectEvent_Server(pAdd, NULL);

  CSrvPacketTcp *pEnd = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
                                              ICQ_SNACxLIST_ROSTxEDITxEND);
  SendEvent_Server(pEnd);
}

//-----CheckExport-------------------------------------------------------------
void IcqProtocol::CheckExport()
{
  // Export groups
  GroupNameMap groups;
  {
    Licq::GroupListGuard groupList(false);
    BOOST_FOREACH(const Licq::Group* group, **groupList)
    {
      Licq::GroupReadGuard g(group);
      if (g->serverId(LICQ_PPID) == 0)
        groups[g->id()] = g->name();
    }
  }

  if (groups.size() > 0)
    icqExportGroups(groups);

  // Just upload all of the users now
  list<Licq::UserId> users;
  {
    Licq::UserListGuard userList(LICQ_PPID);
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      Licq::UserReadGuard u(user);
      // If they aren't a current server user and not in the ignore list,
      // let's import them!
      if (u->GetSID() == 0 && !u->IgnoreList())
        users.push_back(u->id());
    }
  }

  if (users.size())
  {
    icqExportUsers(users, ICQ_ROSTxNORMAL);
    icqUpdateServerGroups();
  }

  // Export visible/invisible/ignore list
  list<Licq::UserId> visibleUsers, invisibleUsers, ignoredUsers;
  {
    Licq::UserListGuard userList(LICQ_PPID);
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      Licq::UserReadGuard pUser(user);
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
  }

  if (visibleUsers.size())
    icqExportUsers(visibleUsers, ICQ_ROSTxVISIBLE);

  if (invisibleUsers.size())
    icqExportUsers(invisibleUsers, ICQ_ROSTxINVISIBLE);

  if (ignoredUsers.size())
    icqExportUsers(ignoredUsers, ICQ_ROSTxIGNORE);
}

//-----icqExportUsers----------------------------------------------------------
void IcqProtocol::icqExportUsers(const list<Licq::UserId>& users, unsigned short _nType)
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
void IcqProtocol::icqUpdateServerGroups()
{
  if (!UseServerContactList())  return;
  CSrvPacketTcp *pReply;

  pReply = new CPU_UpdateToServerList("", ICQ_ROSTxGROUP, 0);
  addToModifyUsers(pReply->SubSequence(), "");
  gLog.Info(tr("%sUpdating top level group.\n"), L_SRVxSTR);
  SendExpectEvent_Server(pReply, NULL);

  Licq::GroupListGuard groupList(false);
  BOOST_FOREACH(const Licq::Group* group, **groupList)
  {
    Licq::GroupReadGuard pGroup(group);
    unsigned int gid = pGroup->serverId(LICQ_PPID);
    if (gid != 0)
    {
      const char* gname = pGroup->name().c_str();
      pReply = new CPU_UpdateToServerList(gname, ICQ_ROSTxGROUP, gid);
      gLog.Info(tr("%sUpdating group %s.\n"), L_SRVxSTR, gname);
      addToModifyUsers(pReply->SubSequence(), "");
      SendExpectEvent_Server(pReply, NULL);
    }
  }
}

//-----icqAddGroup--------------------------------------------------------------
void IcqProtocol::icqAddGroup(const string& groupName)
{
  if (!UseServerContactList())  return;

  CSrvPacketTcp *pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
    ICQ_SNACxLIST_ROSTxEDITxSTART);
  SendEvent_Server(pStart);

  CPU_AddToServerList *pAdd = new CPU_AddToServerList(groupName.c_str(), ICQ_ROSTxGROUP);
  int nGSID = pAdd->GetGSID();
  gLog.Info(tr("%sAdding group %s (%d) to server list ...\n"), L_SRVxSTR, groupName.c_str(), nGSID);
  addToModifyUsers(pAdd->SubSequence(), groupName);
  SendExpectEvent_Server(pAdd, NULL);
}

void IcqProtocol::icqChangeGroup(const Licq::UserId& userId,
                                unsigned short _nNewGroup, unsigned short _nOldGSID,
                                unsigned short _nNewType, unsigned short _nOldType)
{
  if (!UseServerContactList())
    return;

  if (_nNewGroup == 0)
  {
    gLog.Warn(tr("%sProtocol prohibits for users to be in the root group.\n"
                 "%sAborting group change attempt for %s.\n"),
        L_SRVxSTR, L_BLANKxSTR, userId.accountId().c_str());
    return;
  }

  // Get their old SID
  int nSID;
  {
    Licq::UserReadGuard u(userId);
    nSID = u->GetSID();
    gLog.Info(tr("%sChanging group on server list for %s (%s)...\n"),
        L_SRVxSTR, u->getAlias().c_str(), userId.accountId().c_str());
  }

  // Start transaction
  CSrvPacketTcp* pStart =
    new CPU_GenericFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxEDITxSTART);
  SendEvent_Server(pStart);

  // Delete the user
  if (_nOldGSID != 0)
  {
    // Don't attempt removing users from the root group, they can't be there
    CSrvPacketTcp* pRemove =
      new CPU_RemoveFromServerList(userId.accountId().c_str(), _nOldGSID, nSID, _nOldType);
    addToModifyUsers(pRemove->SubSequence(), userId.accountId());
    SendExpectEvent_Server(pRemove, NULL);
  }

  // Add the user, with the new group
  CPU_AddToServerList* pAdd = new CPU_AddToServerList(userId.accountId().c_str(), _nNewType, _nNewGroup);
  addToModifyUsers(pAdd->SubSequence(), userId.accountId());
  SendExpectEvent_Server(pAdd, NULL);
}

//-----icqExportGroups----------------------------------------------------------
void IcqProtocol::icqExportGroups(const GroupNameMap& groups)
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
void IcqProtocol::icqCreatePDINFO()
{
  CSrvPacketTcp *pPDInfo = new CPU_AddPDINFOToServerList();
  gLog.Info(tr("%sAdding privacy information to server side list...\n"),
    L_SRVxSTR);
  SendEvent_Server(pPDInfo);
}

void IcqProtocol::icqRemoveUser(const Licq::UserId& userId, bool ignored)
{
  // Remove from the SSList and update groups
  if (UseServerContactList())
  {
    CSrvPacketTcp *pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
      ICQ_SNACxLIST_ROSTxEDITxSTART);
    SendEvent_Server(pStart);

    unsigned short nGSID;
    unsigned short nSID;
    unsigned short nVisibleSID;
    unsigned short nInvisibleSID;
    bool bIgnored;
    {
      Licq::UserWriteGuard u(userId);
      // When we remove a user, we remove them from all parts of the list:
      // Visible, Invisible and Ignore lists as well.
      nGSID = u->GetGSID();
      nSID = u->GetSID();
      nVisibleSID = u->GetVisibleSID();
      nInvisibleSID = u->GetInvisibleSID();
      bIgnored = (u->IgnoreList() | ignored);
      u->SetGSID(0);
      u->SetVisibleSID(0);
      u->SetInvisibleSID(0);
      u->SetVisibleList(false);
      u->SetInvisibleList(false);
      u->SaveLicqInfo();
    }

    CSrvPacketTcp *pRemove = 0;
    if (bIgnored)
      pRemove = new CPU_RemoveFromServerList(userId.accountId().c_str(), nGSID, nSID, ICQ_ROSTxIGNORE);
    else
      pRemove = new CPU_RemoveFromServerList(userId.accountId().c_str(), nGSID, nSID, ICQ_ROSTxNORMAL);
    addToModifyUsers(pRemove->SubSequence(), userId.accountId());
    SendExpectEvent_Server(pRemove, NULL);

    if (nVisibleSID)
    {
      CSrvPacketTcp *pVisRemove = new CPU_RemoveFromServerList(userId.accountId().c_str(), 0,
        nVisibleSID, ICQ_ROSTxVISIBLE);
      /* XXX Check if we get an ack response on this packet
      pthread_mutex_lock(&mutex_modifyserverusers);
      m_lszModifyServerUsers[pVisRemove->SubSequence()] = userId.accountId();
      pthread_mutex_unlock(&mutex_modifyserverusers);
      */
      SendEvent_Server(pVisRemove);
    }

    if (nInvisibleSID)
    {
      CSrvPacketTcp *pInvisRemove = new CPU_RemoveFromServerList(userId.accountId().c_str(), 0,
          nInvisibleSID, ICQ_ROSTxINVISIBLE);
      /* XXX Check if we get an ack response on this packet
      pthread_mutex_lock(&mutex_modifyserverusers);
      m_lszModifyServerUsers[pInvisRemove->SubSequence()] = userId.accountId();
      pthread_mutex_unlock(&mutex_modifyserverusers);
      */
      SendEvent_Server(pInvisRemove);
    }
  }

  // Tell server they are no longer with us.
  CSrvPacketTcp *p = new CPU_GenericUinList(userId.accountId().c_str(), ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_REMOVExFROMxLIST);
  gLog.Info(tr("%sAlerting server to remove user (#%hu)...\n"), L_SRVxSTR,
            p->Sequence());
  SendExpectEvent_Server(userId, p, NULL);
}

//-----icqRemoveGroup----------------------------------------------------------
void IcqProtocol::icqRemoveGroup(const string& groupName)
{
  if (!UseServerContactList()) return;

  CSrvPacketTcp *pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
    ICQ_SNACxLIST_ROSTxEDITxSTART);
  SendEvent_Server(pStart);

  CSrvPacketTcp *pRemove = new CPU_RemoveFromServerList(groupName.c_str(),
      Licq::gUserManager.GetIDFromGroup(groupName), 0, ICQ_ROSTxGROUP);
  gLog.Info(tr("%sRemoving group from server side list (%s)...\n"), L_SRVxSTR, groupName.c_str());
  addToModifyUsers(pRemove->SubSequence(), groupName);
  SendExpectEvent_Server(pRemove, NULL);
}

//-----icqRenameGroup----------------------------------------------------------
void IcqProtocol::icqRenameGroup(const string& newName, unsigned short _nGSID)
{
  if (!UseServerContactList() || !_nGSID || m_nTCPSrvSocketDesc == -1) return;

  CSrvPacketTcp *pUpdate = new CPU_UpdateToServerList(newName.c_str(),
    ICQ_ROSTxGROUP, _nGSID);
  gLog.Info(tr("%sRenaming group with id %d to %s...\n"), L_SRVxSTR, _nGSID,
      newName.c_str());
  addToModifyUsers(pUpdate->SubSequence(), newName);
  SendExpectEvent_Server(pUpdate, NULL);
}

void IcqProtocol::icqRenameUser(const Licq::UserId& userId, const string& newAlias)
{
  if (!UseServerContactList() || m_nTCPSrvSocketDesc == -1) return;

  CSrvPacketTcp *pUpdate = new CPU_UpdateToServerList(userId.accountId().c_str(), ICQ_ROSTxNORMAL);
  gLog.Info(tr("%sRenaming %s to %s...\n"), L_SRVxSTR, userId.accountId().c_str(), newAlias.c_str());
  addToModifyUsers(pUpdate->SubSequence(), userId.accountId());
  SendExpectEvent_Server(pUpdate, NULL);
}

void IcqProtocol::icqAlertUser(const Licq::UserId& userId)
{
  if (userId.protocolId() != LICQ_PPID)
    return;

  char sz[MaxMessageSize];
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    sprintf(sz, "%s%c%s%c%s%c%s%c%c%c", o->GetAlias(), 0xFE, o->getFirstName().c_str(),
        0xFE, o->getLastName().c_str(), 0xFE, o->getEmail().c_str(), 0xFE,
        o->GetAuthorization() ? '0' : '1', 0xFE);
  }
  string accountId = userId.accountId();
  const char* id = accountId.c_str();
  CPU_ThroughServer *p = new CPU_ThroughServer(id, ICQ_CMDxSUB_ADDEDxTOxLIST, sz);
  gLog.Info(tr("%sAlerting user they were added (#%hu)...\n"), L_SRVxSTR, p->Sequence());
  SendExpectEvent_Server(userId, p, NULL);
}

void IcqProtocol::icqFetchAutoResponseServer(unsigned long eventId, const Licq::UserId& userId)
{
  CPU_CommonFamily *p = 0;

  if (isalpha(userId.accountId()[0]))
    p = new CPU_AIMFetchAwayMessage(userId.accountId().c_str());
  else
  {
    int nCmd;
    {
      Licq::UserReadGuard u(userId);
      if (!u.isLocked())
        return;

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
    }

    p = new CPU_ThroughServer(userId.accountId().c_str(), nCmd, string());
  }

  if (p == NULL)
    return;

  gLog.Info(tr("%sRequesting auto response from %s (%hu).\n"), L_SRVxSTR,
      userId.toString().c_str(), p->Sequence());

  SendExpectEvent_Server(eventId, userId, p, NULL);
}

//-----icqSetRandomChatGroup----------------------------------------------------
unsigned long IcqProtocol::icqSetRandomChatGroup(unsigned long _nGroup)
{
  CPU_SetRandomChatGroup *p = new CPU_SetRandomChatGroup(_nGroup);
  gLog.Info(tr("%sSetting random chat group (#%hu)...\n"), L_SRVxSTR,
            p->Sequence());

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqRandomChatSearch------------------------------------------------------
unsigned long IcqProtocol::icqRandomChatSearch(unsigned long _nGroup)
{
  CPU_RandomChatSearch *p = new CPU_RandomChatSearch(_nGroup);
  gLog.Info(tr("%sSearching for random chat user (#%hu)...\n"), L_SRVxSTR,
            p->Sequence());

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

void IcqProtocol::icqRegister(const string& passwd)
{
  myRegisterPasswd = passwd;
  m_bRegistering = true;
  m_nRegisterThreadId = pthread_self();
//  CPU_RegisterFirst *p = new CPU_RegisterFirst();
//  gLog.Info("%sRegistering a new user (#%hu)...\n", L_SRVxSTR, p->Sequence());
// SendEvent_Server(p);
  ConnectToLoginServer();
}

//-----ICQ::icqRegisterFinish------------------------------------------------
void IcqProtocol::icqRegisterFinish()
{
  CPU_RegisterFirst *pFirst = new CPU_RegisterFirst();
  SendEvent_Server(pFirst);

  CPU_Register *p = new CPU_Register(myRegisterPasswd.c_str());
  gLog.Info(tr("%sRegistering a new user...\n"), L_SRVxSTR);
  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    e->thread_plugin = m_nRegisterThreadId;
}

//-----ICQ::icqVerifyRegistration
void IcqProtocol::icqVerifyRegistration()
{
  CPU_RegisterFirst *pFirst = new CPU_RegisterFirst();
  SendEvent_Server(pFirst);
    
  CPU_VerifyRegistration *pVerify = new CPU_VerifyRegistration();
  gLog.Info(tr("%sRequesting verification image...\n"), L_SRVxSTR);
  SendEvent_Server(pVerify);
  
  m_nRegisterThreadId = pthread_self();
}

//-----ICQ::icqVerify--------------------------------------------------------
void IcqProtocol::icqVerify(const string& verification)
{
  CPU_SendVerification *p = new CPU_SendVerification(myRegisterPasswd.c_str(),
      verification.c_str());
   gLog.Info(tr("%sSending verification for registration.\n"), L_SRVxSTR);

   SendExpectEvent_Server(p, NULL);
}

//-----ICQ::icqRelogon-------------------------------------------------------
void IcqProtocol::icqRelogon()
{
  unsigned long status;

  if (m_eStatus == STATUS_ONLINE)
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    status = o->StatusFull();
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

//-----icqRequestMetaInfo----------------------------------------------------
unsigned long IcqProtocol::icqRequestMetaInfo(const Licq::UserId& userId)
{
  CPU_CommonFamily *p = 0;
  bool bIsAIM = isalpha(userId.accountId()[0]);
  if (bIsAIM)
    p = new CPU_RequestInfo(userId.accountId().c_str());
  else
    p = new CPU_Meta_RequestAllInfo(userId.accountId().c_str());
  gLog.Info(tr("%sRequesting meta info for %s (#%hu/#%d)...\n"), L_SRVxSTR,
      userId.toString().c_str(), p->Sequence(), p->SubSequence());
  Licq::Event* e = SendExpectEvent_Server(userId, p, NULL, !bIsAIM);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqRequestService--------------------------------------------------------
void IcqProtocol::icqRequestService(unsigned short nFam)
{
  CPU_CommonFamily *p = new CPU_RequestService(nFam);
  gLog.Info(tr("%sRequesting service socket for FAM 0x%02X (#%hu/#%d)...\n"),
            L_SRVxSTR, nFam, p->Sequence(), p->SubSequence());
  SendEvent_Server(p);
}

unsigned long IcqProtocol::icqSetStatus(unsigned short newStatus)
{
  if (newStatus & ICQ_STATUS_DND)
    newStatus |= ICQ_STATUS_OCCUPIED; // quick compat hack

  // icq go wants the mask set when we truly are away
  if (newStatus & ICQ_STATUS_DND || newStatus & ICQ_STATUS_OCCUPIED ||
      newStatus & ICQ_STATUS_NA)
    newStatus |= ICQ_STATUS_AWAY;

  // Set the status flags
  unsigned long s;
  unsigned long pfm;
  bool Invisible;
  bool goInvisible;
  bool isLogon;
  int nPDINFO;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    s = o->AddStatusFlags(newStatus);
    pfm = o->PhoneFollowMeStatus();
    Invisible = o->isInvisible();
    goInvisible = (newStatus & ICQ_STATUS_FxPRIVATE);
    isLogon = !o->isOnline();
    nPDINFO = o->GetPDINFO();
  }

  if (nPDINFO == 0)
  {
    icqCreatePDINFO();

    Licq::OwnerReadGuard o(LICQ_PPID);
    nPDINFO = o->GetPDINFO();
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
      Licq::User::statusToString(Licq::User::statusFromIcqStatus(newStatus)).c_str(), p->Sequence());
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
unsigned long IcqProtocol::icqSetPassword(const string& password)
{
  CPU_SetPassword *p = new CPU_SetPassword(password.c_str());
  gLog.Info(tr("%sUpdating password (#%hu/#%d)...\n"), L_SRVxSTR,
            p->Sequence(), p->SubSequence());
  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

unsigned long IcqProtocol::icqSetGeneralInfo(const string& alias, const string& firstName,
      const string& lastName, const string& emailPrimary, const string& city,
      const string& state, const string& phoneNumber, const string& faxNumber,
      const string& address, const string& cellularNumber, const string& zipCode,
      unsigned short countryCode, bool hideEmail)
{
  CPU_Meta_SetGeneralInfo* p = new CPU_Meta_SetGeneralInfo(alias.c_str(), firstName.c_str(),
      lastName.c_str(), emailPrimary.c_str(), city.c_str(), state.c_str(), phoneNumber.c_str(),
      faxNumber.c_str(), address.c_str(), cellularNumber.c_str(), zipCode.c_str(), countryCode, hideEmail);

  gLog.Info(tr("%sUpdating general info (#%hu/#%d)...\n"), L_SRVxSTR, p->Sequence(), p->SubSequence());

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetEmailInfo---------------------------------------------------------
unsigned long IcqProtocol::icqSetEmailInfo(const std::string& emailSecondary, const std::string& emailOld)
{
return 0;
  CPU_Meta_SetEmailInfo* p = new CPU_Meta_SetEmailInfo(emailSecondary.c_str(), emailOld.c_str());

  gLog.Info(tr("%sUpdating additional E-Mail info (#%hu/#%d)...\n"), L_SRVxSTR, p->Sequence(), p->SubSequence());

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetMoreInfo----------------------------------------------------
unsigned long IcqProtocol::icqSetMoreInfo(unsigned short age, char gender,
    const string& homepage, unsigned short birthYear, char birthMonth,
    char birthDay, char language1, char language2, char language3)
{
  CPU_Meta_SetMoreInfo* p = new CPU_Meta_SetMoreInfo(age, gender, homepage.c_str(),
      birthYear, birthMonth, birthDay, language1, language2, language3);

  gLog.Info(tr("%sUpdating more info (#%hu/#%d)...\n"), L_SRVxSTR, p->Sequence(), p->SubSequence());

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetInterestsInfo------------------------------------------------------
unsigned long IcqProtocol::icqSetInterestsInfo(const UserCategoryMap& interests)
{
  CPU_Meta_SetInterestsInfo *p = new CPU_Meta_SetInterestsInfo(interests);
  gLog.Info("%sUpdating Interests info (#%hu/#%d)..\n", L_SRVxSTR,
    p->Sequence(), p->SubSequence());

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetOrgBackInfo--------------------------------------------------------
unsigned long IcqProtocol::icqSetOrgBackInfo(const UserCategoryMap& orgs,
    const UserCategoryMap& background)
{
  CPU_Meta_SetOrgBackInfo *p =
    new CPU_Meta_SetOrgBackInfo(orgs, background);
  gLog.Info("%sUpdating Organizations/Backgrounds info (#%hu/#%d)..\n",
    L_SRVxSTR, p->Sequence(), p->SubSequence());

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetWorkInfo--------------------------------------------------------
unsigned long IcqProtocol::icqSetWorkInfo(const string& city, const string& state,
    const string& phone, const string& fax, const string& address, const string& zip,
    unsigned short companyCountry, const string& name, const string& department,
    const string& position, unsigned short companyOccupation, const string& homepage)
{
  CPU_Meta_SetWorkInfo* p = new CPU_Meta_SetWorkInfo(city.c_str(), state.c_str(),
      phone.c_str(), fax.c_str(), address.c_str(), zip.c_str(), companyCountry,
      name.c_str(), department.c_str(), position.c_str(), companyOccupation, homepage.c_str());

  gLog.Info(tr("%sUpdating work info (#%hu/#%d)...\n"), L_SRVxSTR, p->Sequence(), p->SubSequence());

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetAbout-----------------------------------------------------------
unsigned long IcqProtocol::icqSetAbout(const string& about)
{
  char *szAbout = gTranslator.NToRN(about.c_str());

  CPU_Meta_SetAbout *p = new CPU_Meta_SetAbout(szAbout);

  gLog.Info(tr("%sUpdating about (#%hu/#%d)...\n"), L_SRVxSTR, p->Sequence(), p->SubSequence());

  delete [] szAbout;

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

unsigned long IcqProtocol::icqAuthorizeGrant(const Licq::UserId& userId, const string& /* message */)
{
  const string accountId = userId.accountId();
  const char* szId = accountId.c_str();
  CPU_Authorize *p = new CPU_Authorize(szId);
  gLog.Info(tr("%sAuthorizing user %s\n"), L_SRVxSTR, szId);
  SendEvent_Server(p);

  return 0;
}

unsigned long IcqProtocol::icqAuthorizeRefuse(const Licq::UserId& userId, const string& message)
{
  const string accountId = userId.accountId();
  const char* szId = accountId.c_str();
  char *sz = NULL;
  if (!message.empty())
  {
    sz = gTranslator.NToRN(message.c_str());
    gTranslator.ClientToServer(sz);
  }
  CPU_ThroughServer* p = new CPU_ThroughServer(szId, ICQ_CMDxSUB_AUTHxREFUSED, sz == NULL ? string() : sz);
  gLog.Info(tr("%sRefusing authorization to user %s (#%hu)...\n"), L_SRVxSTR,
     szId, p->Sequence());
  delete [] sz;

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

void IcqProtocol::icqRequestAuth(const Licq::UserId& userId, const string& message)
{
  CSrvPacketTcp* p = new CPU_RequestAuth(userId.accountId().c_str(), message.c_str());
  SendEvent_Server(p);
}

//-----icqSetSecurityInfo----------------------------------------------------
unsigned long IcqProtocol::icqSetSecurityInfo(bool bAuthorize, bool bHideIp, bool bWebAware)
{
  // Since ICQ5.1, the status change packet is sent first, which means it is
  // assumed that the set security info packet works.
  unsigned short s;
  {
    Licq::OwnerWriteGuard o(LICQ_PPID);
    o->SetEnableSave(false);
    o->SetAuthorization(bAuthorize);
    o->SetWebAware(bWebAware);
    o->SetHideIp(bHideIp);
    o->SetEnableSave(true);
    o->SaveLicqInfo();
    s = o->StatusFull();
  }
  // Set status to ensure the status flags are set
  icqSetStatus(s);

  // Now send the set security info packet
    CPU_Meta_SetSecurityInfo *p = new CPU_Meta_SetSecurityInfo(bAuthorize, bHideIp, bWebAware);
    gLog.Info(tr("%sUpdating security info (#%hu/#%d)...\n"), L_SRVxSTR, p->Sequence(), p->SubSequence());
  Licq::Event* e = SendExpectEvent_Server(p, NULL);
    if (e != NULL)
      return e->EventId();
    return 0;
}

//-----icqSearchWhitePages--------------------------------------------------
unsigned long IcqProtocol::icqSearchWhitePages(const string& firstName, const string& lastName,
    const string& alias, const string& email, unsigned short minAge, unsigned short maxAge,
    char gender, char language, const string& city, const string& state,
    unsigned short countryCode, const string& coName, const string& coDept,
    const string& coPos, const string& keyword, bool onlineOnly)
{
  // Yes, there are a lot of extra options that you can search by.. but I
  // don't see a point for the hundreds of items that I can add..  just
  // use their web page for that shit - Jon
  CPU_SearchWhitePages* p = new CPU_SearchWhitePages(firstName.c_str(), lastName.c_str(),
      alias.c_str(), email.c_str(), minAge, maxAge, gender, language, city.c_str(),
      state.c_str(), countryCode, coName.c_str(), coDept.c_str(), coPos.c_str(),
      keyword.c_str(), onlineOnly);
  gLog.Info(tr("%sStarting white pages search (#%hu/#%d)...\n"), L_SRVxSTR,
            p->Sequence(), p->SubSequence());
  Licq::Event* e = SendExpectEvent_Server(p, NULL, true);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSearchByUin----------------------------------------------------------
unsigned long IcqProtocol::icqSearchByUin(unsigned long nUin)
{
   CPU_SearchByUin *p = new CPU_SearchByUin(nUin);
   gLog.Info(tr("%sStarting search by UIN for user (#%hu/#%d)...\n"), L_SRVxSTR, 
             p->Sequence(), p->SubSequence());
  Licq::Event* e = SendExpectEvent_Server(p, NULL, true);
   if (e != NULL)
     return e->EventId();
   return 0;
}

//-----icqGetUserBasicInfo------------------------------------------------------
unsigned long IcqProtocol::icqUserBasicInfo(const Licq::UserId& userId)
{
  CPU_CommonFamily *p = 0;
  bool bIsAIM = isalpha(userId.accountId()[0]);
  if (bIsAIM)
    p = new CPU_RequestInfo(userId.accountId().c_str());
  else
    p = new CPU_Meta_RequestAllInfo(userId.accountId().c_str());
  gLog.Info(tr("%sRequesting user info (#%hu/#%d)...\n"), L_SRVxSTR,
            p->Sequence(), p->SubSequence());
  Licq::Event* e = SendExpectEvent_Server(userId, p, NULL, !bIsAIM);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqPing------------------------------------------------------------------
void IcqProtocol::icqPing()
{
  // pinging is necessary to avoid that masquerading
  // servers close the connection
   CPU_Ping *p = new CPU_Ping;
   SendEvent_Server(p);
}

//-----icqUpdateInfoTimestamp---------------------------------------------------
void IcqProtocol::icqUpdateInfoTimestamp(const char *GUID)
{
  CPU_UpdateInfoTimestamp *p = new CPU_UpdateInfoTimestamp(GUID);
  SendEvent_Server(p);
}

//-----icqUpdatePhoneBookTimestamp----------------------------------------------
void IcqProtocol::icqUpdatePhoneBookTimestamp()
{
  bool bOffline;
  {
    Licq::OwnerWriteGuard o(LICQ_PPID);
    o->SetClientInfoTimestamp(time(NULL));
    bOffline = !o->isOnline();
  }

  if (!bOffline)
    icqUpdateInfoTimestamp(PLUGIN_PHONExBOOK);
}

//-----icqUpdatePictureTimestamp------------------------------------------------
void IcqProtocol::icqUpdatePictureTimestamp()
{
  bool bOffline;
  {
    Licq::OwnerWriteGuard o(LICQ_PPID);
    o->SetClientInfoTimestamp(time(NULL));
    bOffline = !o->isOnline();
  }

  if (!bOffline)
    icqUpdateInfoTimestamp(PLUGIN_PICTURE);
}

//-----icqSetPhoneFollowMeStatus------------------------------------------------
void IcqProtocol::icqSetPhoneFollowMeStatus(unsigned long nNewStatus)
{
  bool bOffline;
  {
    Licq::OwnerWriteGuard o(LICQ_PPID);
    o->SetClientStatusTimestamp(time(NULL));
    o->SetPhoneFollowMeStatus(nNewStatus);
    bOffline = !o->isOnline();
  }

  if (!bOffline)
  {
    CPU_UpdateStatusTimestamp *p =
      new CPU_UpdateStatusTimestamp(PLUGIN_FOLLOWxME, nNewStatus);
    SendEvent_Server(p);
  }
}

//-----icqUpdateContactList-----------------------------------------------------
void IcqProtocol::icqUpdateContactList()
{
  unsigned short n = 0;
  StringList users;
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_W)
  {
    n++;
    users.push_back(pUser->IdString());
    if (n == myMaxUsersPerPacket)
    {
      CSrvPacketTcp *p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
      gLog.Info(tr("%sUpdating contact list (#%hu)...\n"), L_SRVxSTR, p->Sequence());
      SendEvent_Server(p);
      users.clear();
      n = 0;
    }
    // Reset all users to offline
    if (pUser->isOnline())
      pUser->statusChanged(User::OfflineStatus);
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
void IcqProtocol::icqTypingNotification(const Licq::UserId& userId, bool _bActive)
{
  if (gDaemon.sendTypingNotification())
  {
    CSrvPacketTcp *p = new CPU_TypingNotification(userId.accountId().c_str(), _bActive);
    SendEvent_Server(p);
  }
}

//-----icqCheckInvisible--------------------------------------------------------
void IcqProtocol::icqCheckInvisible(const Licq::UserId& userId)
{
  CSrvPacketTcp *p = new CPU_CheckInvisible(userId.accountId().c_str());
  SendExpectEvent_Server(userId, p, NULL);
}

//-----icqSendVisibleList-------------------------------------------------------
void IcqProtocol::icqSendVisibleList()
{
  // send user info packet
  // Go through the entire list of users, checking if each one is on
  // the visible list
  StringList users;
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_R)
  {
    if (pUser->VisibleList())
      users.push_back(pUser->IdString());
  }
  FOR_EACH_PROTO_USER_END
  CSrvPacketTcp* p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxVISIBLExLIST);
  gLog.Info(tr("%sSending visible list (#%hu)...\n"), L_SRVxSTR, p->Sequence());
  SendEvent_Server(p);
}


//-----icqSendInvisibleList-----------------------------------------------------
void IcqProtocol::icqSendInvisibleList()
{
  StringList users;
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_R)
  {
    if (pUser->InvisibleList())
      users.push_back(pUser->IdString());
  }
  FOR_EACH_PROTO_USER_END

  CSrvPacketTcp* p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxINVISIBxLIST);
  gLog.Info(tr("%sSending invisible list (#%hu)...\n"), L_SRVxSTR, p->Sequence());
  SendEvent_Server(p);
}

//-----icqAddToVisibleList------------------------------------------------------
void IcqProtocol::icqAddToVisibleList(const Licq::UserId& userId)
{
  {
    Licq::UserWriteGuard u(userId);
    if (u.isLocked())
      u->SetVisibleList(true);
  }
  CSrvPacketTcp *p = new CPU_GenericUinList(userId.accountId().c_str(), ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxVISIBLExLIST);
  gLog.Info(tr("%sAdding user %s to visible list (#%hu)...\n"), L_SRVxSTR, userId.accountId().c_str(),
     p->Sequence());
  SendEvent_Server(p);

  if (UseServerContactList())
  {
    CSrvPacketTcp *pAdd = new CPU_AddToServerList(userId.accountId().c_str(), ICQ_ROSTxVISIBLE);
    addToModifyUsers(pAdd->SubSequence(), userId.accountId());
    SendExpectEvent_Server(pAdd, NULL);
  }
}

//-----icqRemoveFromVisibleList-------------------------------------------------
void IcqProtocol::icqRemoveFromVisibleList(const Licq::UserId& userId)
{
  {
    Licq::UserWriteGuard u(userId);
    if (u.isLocked())
      u->SetVisibleList(false);
  }
  CSrvPacketTcp *p = new CPU_GenericUinList(userId.accountId().c_str(), ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_REMxVISIBLExLIST);
  gLog.Info(tr("%sRemoving user %s from visible list (#%hu)...\n"), L_SRVxSTR,
      userId.toString().c_str(), p->Sequence());
  SendEvent_Server(p);

  if (UseServerContactList())
  {
    Licq::UserReadGuard u(userId);
    if (u.isLocked())
    {
      CSrvPacketTcp *pRemove = new CPU_RemoveFromServerList(userId.accountId().c_str(), 0, u->GetVisibleSID(),
        ICQ_ROSTxVISIBLE);
      addToModifyUsers(pRemove->SubSequence(), userId.accountId());
      SendExpectEvent_Server(userId, pRemove, NULL);
    }
  }
}

//-----icqAddToInvisibleList----------------------------------------------------
void IcqProtocol::icqAddToInvisibleList(const Licq::UserId& userId)
{
  {
    Licq::UserWriteGuard u(userId);
    if (u.isLocked())
      u->SetInvisibleList(true);
  }
  CSrvPacketTcp *p = new CPU_GenericUinList(userId.accountId().c_str(), ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxINVISIBxLIST);
  gLog.Info(tr("%sAdding user %s to invisible list (#%hu)...\n"), L_SRVxSTR, userId.toString().c_str(),
     p->Sequence());
  SendEvent_Server(p);

  if (UseServerContactList())
  {
    CSrvPacketTcp *pAdd = new CPU_AddToServerList(userId.accountId().c_str(), ICQ_ROSTxINVISIBLE);
    addToModifyUsers(pAdd->SubSequence(), userId.accountId());
    SendEvent_Server(pAdd);
  }
}

//-----icqRemoveFromInvisibleList-----------------------------------------------
void IcqProtocol::icqRemoveFromInvisibleList(const Licq::UserId& userId)
{
  {
    Licq::UserWriteGuard u(userId);
    if (u.isLocked())
      u->SetInvisibleList(false);
  }
  CSrvPacketTcp *p = new CPU_GenericUinList(userId.accountId().c_str(), ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_REMxINVISIBxLIST);
  gLog.Info(tr("%sRemoving user %s from invisible list (#%hu)...\n"), L_SRVxSTR, userId.toString().c_str(),
     p->Sequence());
  SendEvent_Server(p);

  if (UseServerContactList())
  {
    Licq::UserReadGuard u(userId);
    if (u.isLocked())
    {
      CSrvPacketTcp *pRemove = new CPU_RemoveFromServerList(userId.accountId().c_str(), 0, u->GetInvisibleSID(),
        ICQ_ROSTxINVISIBLE);
      addToModifyUsers(pRemove->SubSequence(), userId.accountId());
      SendEvent_Server(pRemove);
    }
  }
}

//-----icqAddToIgnoreList-------------------------------------------------------
void IcqProtocol::icqAddToIgnoreList(const Licq::UserId& userId)
{
  if (!UseServerContactList()) return;

  icqRemoveUser(userId);
  CPU_AddToServerList *pAdd = new CPU_AddToServerList(userId.accountId().c_str(), ICQ_ROSTxIGNORE,
    0, false);
  SendEvent_Server(pAdd);
}

//-----icqRemoveFromIgnoreList--------------------------------------------------
void IcqProtocol::icqRemoveFromIgnoreList(const Licq::UserId& userId)
{
  if (!UseServerContactList()) return;

  icqRemoveUser(userId, true);
  icqAddUser(userId, false);
}

//-----icqClearServerList-------------------------------------------------------
void IcqProtocol::icqClearServerList()
{
  if (!UseServerContactList()) return;

  unsigned short n = 0;
  StringList users;

  // Delete all the users in groups
  {
    Licq::UserListGuard userList(LICQ_PPID);
    BOOST_FOREACH(const Licq::User* u, **userList)
    {
      n++;
      users.push_back(u->accountId());
      if (n == myMaxUsersPerPacket)
      {
        CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxNORMAL);
        gLog.Info(tr("%sDeleting server list users (#%hu)...\n"), L_SRVxSTR, p->Sequence());
        SendEvent_Server(p);
        users.clear();
        n = 0;
      }
    }
  }

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

  {
    Licq::UserListGuard userList(LICQ_PPID);
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      {
        Licq::UserReadGuard pUser(user);
        if (pUser->GetInvisibleSID())
        {
          n++;
          users.push_back(pUser->accountId());
        }
      }

      if (n == myMaxUsersPerPacket)
      {
        CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxINVISIBLE);
        gLog.Info(tr("%sDeleting server list invisible list users (#%hu)...\n"),
            L_SRVxSTR, p->Sequence());
        SendEvent_Server(p);
        users.clear();
        n = 0;
      }
    }
  }

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

  {
    Licq::UserListGuard userList(LICQ_PPID);
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      {
        Licq::UserReadGuard pUser(user);
        if (pUser->GetVisibleSID())
        {
          n++;
          users.push_back(pUser->accountId());
        }
      }

      if (n == myMaxUsersPerPacket)
      {
        CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxVISIBLE);
        gLog.Info(tr("%sDeleting server list visible list users (#%hu)...\n"),
            L_SRVxSTR, p->Sequence());
        SendEvent_Server(p);
        users.clear();
        n = 0;
      }
    }
  }

  if (n != 0)
  {
    CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxVISIBLE);
    gLog.Info(tr("%sDeleting server list visible list users (#%hu)...\n"),
      L_SRVxSTR, p->Sequence());
    SendEvent_Server(p);
  }
}

//-----icqSendThroughServer-----------------------------------------------------
Licq::Event* IcqProtocol::icqSendThroughServer(unsigned long eventId, const Licq::UserId& userId,
    unsigned char format, const string& message, Licq::UserEvent* ue, unsigned short nCharset,
  size_t nMsgLen)
{
  Licq::Event* result;
  bool bOffline = true;
  {
    Licq::UserReadGuard u(userId);
    if (u.isLocked())
      bOffline = !u->isOnline();
  }

  CPU_ThroughServer* p = new CPU_ThroughServer(userId.accountId().c_str(), format, message, nCharset, bOffline, nMsgLen);

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
  if (gDaemon.shuttingDown())
    return NULL;

  if (ue != NULL)
    ue->setIsReceiver(false);
  Licq::Event* e = new Licq::Event(eventId, m_nTCPSrvSocketDesc, p, Licq::Event::ConnectServer, userId, ue);
  if (e == NULL) return 0;
  e->m_NoAck = true;

  result = SendExpectEvent(e, &ProcessRunningEvent_Server_tep);
  return result;
}

unsigned long IcqProtocol::icqSendSms(const Licq::UserId& userId,
    const string& number, const string& message)
{
  Licq::EventSms* ue = new Licq::EventSms(number, message, ICQ_CMDxSND_THRUxSERVER,
      Licq::EventSms::TimeNow, LICQ_VERSION);
  CPU_SendSms* p = new CPU_SendSms(number.c_str(), message.c_str());
  gLog.Info(tr("%sSending SMS through server (#%hu/#%d)...\n"), L_SRVxSTR,
      p->Sequence(), p->SubSequence());
  Licq::Event* e = SendExpectEvent_Server(userId, p, ue);
  if (e != NULL)
    return e->EventId();
  return 0;
}

/*------------------------------------------------------------------------------
 * ProcessDoneEvent
 *
 * Processes the given event possibly passes the result to the gui.
 *----------------------------------------------------------------------------*/
void IcqProtocol::ProcessDoneEvent(Licq::Event* e)
{
  // Write the event to the history file if appropriate
  if (e->m_pUserEvent != NULL &&
      (e->m_eResult == Licq::Event::ResultAcked || e->m_eResult == Licq::Event::ResultSuccess) &&
      e->m_nSubResult != ICQ_TCPxACK_RETURN)
  {
    Licq::UserWriteGuard u(e->userId());
    if (u.isLocked())
    {
      e->m_pUserEvent->AddToHistory(*u, false);
      u->SetLastSentEvent();
      gOnEventManager.performOnEvent(OnEventManager::OnEventMsgSent, *u);
    }
    Licq::gStatistics.increase(Licq::Statistics::EventsSentCounter);
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
      gDaemon.PushPluginEvent(e);
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
        gDaemon.PushPluginEvent(e);
        break;

      // Other events
      case MAKESNAC(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA):
      {
        switch (e->m_eResult)
        {
          case Licq::Event::ResultError:
          case Licq::Event::ResultTimedout:
          case Licq::Event::ResultFailed:
          case Licq::Event::ResultSuccess:
          case Licq::Event::ResultCancelled:
            gDaemon.PushPluginEvent(e);
            break;

          case Licq::Event::ResultAcked:
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

unsigned long IcqProtocol::icqLogon(unsigned short logonStatus)
{
  if (m_bLoggingOn)
  {
    gLog.Warn(tr(tr("%sAttempt to logon while already logged or logging on, logoff and try again.\n")), L_WARNxSTR);
    return 0;
  }
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    if (!o.isLocked() || o->accountId().empty())
    {
      gLog.Error("%sNo registered user, unable to process logon attempt.\n", L_ERRORxSTR);
      return 0;
    }
    if (o->password().empty())
    {
      gLog.Error(tr("%sNo password set.  Edit ~/.licq/owner.Licq and fill in the password field.\n"), L_ERRORxSTR);
      return 0;
    }

    m_nDesiredStatus = o->AddStatusFlags(logonStatus);
  }

  CPU_ConnectStart *startPacket = new CPU_ConnectStart();
  SendEvent_Server(startPacket);

  m_bOnlineNotifies = false;
  m_nServerSequence = 0;
  m_bLoggingOn = true;
  m_tLogonTime = time(NULL);
  m_bNeedSalt = true;

  return 0;
}

unsigned long IcqProtocol::icqRequestLogonSalt()
{
  if (m_bNeedSalt)
  {
    CPU_RequestLogonSalt* p;
    {
      Licq::OwnerReadGuard o(LICQ_PPID);
      p =  new CPU_RequestLogonSalt(o->accountId());
    }
    gLog.Info(tr("%sRequesting logon salt (#%hu)...\n"), L_SRVxSTR, p->Sequence());
    SendEvent_Server(p);
  }

  return 0;
}

void IcqProtocol::icqLogoff()
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
  Licq::Event* cancelledEvent = NULL;

  if (nSD != -1)
  {
    CPU_Logoff p;
    unsigned long eventId = gDaemon.getNextEventId();
    cancelledEvent = new Licq::Event(eventId, nSD, &p, Licq::Event::ConnectServer);
    cancelledEvent->m_pPacket = NULL;
    cancelledEvent->m_bCancelled = true;
    SendEvent(nSD, p, true);
    gSocketManager.CloseSocket(nSD);
  }

  postLogoff(nSD, cancelledEvent);
}

void IcqProtocol::postLogoff(int nSD, Licq::Event* cancelledEvent)
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
  std::list<Licq::Event*>::iterator iter;

  // Cancel all events
  // Necessary since the end is always being modified
  unsigned long i = m_lxSendQueue_Server.size();
  for (iter = m_lxSendQueue_Server.begin(); i > 0; i--)
  {
    Licq::Event* e = *iter;
    gLog.Info("Event #%hu is still on the server queue!\n", e->Sequence());
    iter = m_lxSendQueue_Server.erase(iter);
    Licq::Event* cancelled = new Licq::Event(e);
    cancelled->m_bCancelled = true;
    m_lxSendQueue_Server.push_back(cancelled);
  }

  iter = m_lxRunningEvents.begin();
  while (iter != m_lxRunningEvents.end())
  {
    if ((*iter)->m_nSocketDesc == nSD || (*iter)->Channel() == ICQ_CHNxNEW)
    {
      Licq::Event* e = *iter;
      gLog.Info("Event #%hu is still on the running queue!\n", e->Sequence());
      iter = m_lxRunningEvents.erase(iter);
      if (e->thread_running && !pthread_equal(e->thread_send, pthread_self()))
      {
        pthread_cancel(e->thread_send);
        e->thread_running = false;
      }
      std::list<Licq::Event*>::iterator i;
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
    (*iter)->m_eResult = Licq::Event::ResultCancelled;
    ProcessDoneEvent(*iter);
  }
  m_lxExtendedEvents.erase(m_lxExtendedEvents.begin(), m_lxExtendedEvents.end());
  pthread_mutex_unlock(&mutex_extendedevents);
#endif

  Licq::gUserManager.ownerStatusChanged(LICQ_PPID, Licq::User::OfflineStatus);

  myRegisterPasswd = "";

  gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalLogoff,
      Licq::PluginSignal::LogoffRequested, Licq::gUserManager.ownerUserId(LICQ_PPID)));

  // Mark all users as offline, this also updates the last seen
  // online field
  Licq::UserListGuard userList;
  BOOST_FOREACH(Licq::User* user, **userList)
  {
    Licq::UserWriteGuard pUser(user);
    if (pUser->isOnline())
      pUser->statusChanged(Licq::User::OfflineStatus);
  }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----ConnectToServer---------------------------------------------------------

int IcqProtocol::ConnectToLoginServer()
{
  if (gDaemon.proxyEnabled())
    InitProxy();

  // Which protocol plugin?
  int r = ConnectToServer(myIcqServer.c_str(), myIcqServerPort);

  myNewSocketPipe.putChar('S');

  return r;
}

int IcqProtocol::ConnectToServer(const char* server, unsigned short port)
{
  Licq::SrvSocket* s = new Licq::SrvSocket(Licq::gUserManager.ownerUserId(LICQ_PPID));

  if (gDaemon.proxyEnabled())
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
    {
      Licq::OwnerWriteGuard o(LICQ_PPID);
      if (o.isLocked())
        o->SetIntIp(s->getLocalIpInt());
    }

    gSocketManager.AddSocket(s);
    nSocket = m_nTCPSrvSocketDesc = s->Descriptor();
    gSocketManager.DropSocket(s);
  }

  pthread_mutex_unlock(&connect_mutex);

  return nSocket;
}

//-----FindUserForInfoUpdate-------------------------------------------------
Licq::User* IcqProtocol::FindUserForInfoUpdate(const Licq::UserId& userId, Licq::Event* e,
   const char *t)
{
  Licq::User* u = gUserManager.fetchUser(userId, LOCK_W);
  if (u == NULL)
  {
    // If the event is NULL as well then nothing we can do
    if (e == NULL)
    {
      gLog.Warn(tr("%sResponse to unknown %s info request for unknown user (%s).\n"),
          L_WARNxSTR, t, userId.toString().c_str());
      return NULL;
    }
    // Check if we need to create the user
    if (e->m_pUnknownUser == NULL)
    {
      e->m_pUnknownUser = new User(userId);
    }
    // If not, validate the uin
    else if (e->m_pUnknownUser->id() == userId)
    {
      gLog.Error("%sInternal Error: Event contains wrong user.\n", L_ERRORxSTR);
      return NULL;
    }

    u = e->m_pUnknownUser;
    u->lockWrite();
  }
  gLog.Info(tr("%sReceived %s information for %s (%s).\n"), L_SRVxSTR, t,
      u->getAlias().c_str(), userId.toString().c_str());
  return u;
}

string IcqProtocol::findUserByCellular(const string& cellular)
{
  char szParsedNumber1[16], szParsedNumber2[16];
  string id;

  FOR_EACH_USER_START(LOCK_R)
  {
    ParseDigits(szParsedNumber1, pUser->getCellularNumber().c_str(), 15);
    ParseDigits(szParsedNumber2, cellular.c_str(), 15);
    if (!strcmp(szParsedNumber1, szParsedNumber2))
      id = pUser->IdString();
  }
  FOR_EACH_USER_END

  return id;
}

//-----ProcessSrvPacket---------------------------------------------------------
bool IcqProtocol::ProcessSrvPacket(CBuffer& packet)
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

void IcqProtocol::ProcessServiceFam(CBuffer &packet, unsigned short nSubtype)
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
        nPort = myIcqServerPort;
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
      // unsigned long nListTime;
      // {
      //   Licq::OwnerReadGuard o(LICQ_PPID, LOCK_R);
      //   nListTime = o->GetSSTime();
      // }

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
        {
          Licq::OwnerWriteGuard o(LICQ_PPID);
          o->SetIp(realIP);
        }

      char buf[32];
        gLog.Info(tr("%sServer says we are at %s.\n"), L_SRVxSTR, Licq::ip_ntoa(realIP, buf));
      //icqSetStatus(m_nDesiredStatus);
    }
    if (packet.getTLVLen(0x0003) == 4)
      nOnlineSince = packet.UnpackUnsignedLongTLV(0x0003);

      Licq::OwnerWriteGuard o(LICQ_PPID);
    unsigned long nPFM = o->PhoneFollowMeStatus();
    // Workaround for the ICQ4.0 problem of it not liking the PFM flags
    m_nDesiredStatus &= ~(ICQ_STATUS_FxPFM | ICQ_STATUS_FxPFMxAVAILABLE);
    if (nPFM != ICQ_PLUGIN_STATUSxINACTIVE)
      m_nDesiredStatus |= ICQ_STATUS_FxPFM;
    if (nPFM == ICQ_PLUGIN_STATUSxACTIVE)
      m_nDesiredStatus |= ICQ_STATUS_FxPFMxAVAILABLE;
      ChangeUserStatus(*o, m_nDesiredStatus);
    o->SetOnlineSince(nOnlineSince);
      gLog.Info(tr("%sServer says we're now: %s\n"), L_SRVxSTR, o->statusString().c_str());
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
void IcqProtocol::ProcessLocationFam(CBuffer &packet, unsigned short nSubtype)
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
      Licq::UserId userId(szId, LICQ_PPID);
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
        {
          Licq::UserWriteGuard u(userId);
          if (szAwayMsg != u->autoResponse())
          {
            u->setAutoResponse(szAwayMsg);
            u->SetShowAwayMsg(*szAwayMsg);
          }
        }

        Licq::Event* e = DoneServerEvent(nSubSequence, Licq::Event::ResultSuccess);
      if (e)
        ProcessDoneEvent(e);
    }

    char *szInfo = packet.UnpackStringTLV(0x0002);
    if (szInfo)
    {
      gLog.Info(tr("%sReceived user information for %s.\n"), L_SRVxSTR, szId);
      gTranslator.ServerToClient(szInfo);
        {
          Licq::UserWriteGuard u(userId);
          u->SetEnableSave(false);
          u->setUserInfoString("About", szInfo);
          delete [] szInfo;

          // translating string with Translation Table

          delete [] szId;

          // save the user infomation
          u->SetEnableSave(true);
          u->saveUserInfo();
        }

        Licq::Event* e = DoneServerEvent(nSubSequence, Licq::Event::ResultSuccess);
      if (e)
        ProcessDoneEvent(e);

        gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
            Licq::PluginSignal::UserInfo, userId));
      }

    break;
  }

  default:
    gLog.Warn("%sUnknown Location Family Subtype: %04hx\n", L_SRVxSTR, nSubtype);
    break;
  }
}

//--------ProcessBuddyFam--------------------------------------------------
void IcqProtocol::ProcessBuddyFam(CBuffer &packet, unsigned short nSubtype)
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
      Licq::UserWriteGuard u(Licq::UserId(szId, LICQ_PPID));
      if (!u.isLocked())
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
    if (!u->isOnline() || nUserIP )
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
              Licq::UserEvent::licqVersionToString(nInfoTimestamp & 0xFFFF).c_str());
      else if ((nInfoTimestamp & 0xFFFF0000) == LICQ_WITHOUTSSL)
        snprintf(szExtraInfo, 27, "Licq %s",
              Licq::UserEvent::licqVersionToString(nInfoTimestamp & 0xFFFF).c_str());
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

        u->setClientInfo(szExtraInfo);
      u->SetVersion(tcpVersion);
      
      if (nOldStatus != nNewStatus)
      {
        char *szClient = new char[strlen(szExtraInfo)+6];
        if (szExtraInfo[0])
          sprintf(szClient, " [%s].\n", szExtraInfo);
        else
          sprintf(szClient, ".\n");

        ChangeUserStatus(*u, nNewStatus);
        gLog.Info(tr("%s%s (%s) changed status: %s (v%d)%s"),
              L_SRVxSTR, u->getAlias().c_str(), u->id().toString().c_str(),
              u->statusString().c_str(), tcpVersion & 0x0F, szClient);
        if ( (nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS) )
          gLog.Unknown("%sUnknown status flag for %s (%s): 0x%08lX\n",
                       L_UNKNOWNxSTR, u->GetAlias(), u->IdString(),
                       (nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS));
        nNewStatus &= ICQ_STATUS_FxUNKNOWNxFLAGS;
        u->setAutoResponse("");
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
        ChangeUserStatus(*u, nNewStatus);
        gLog.Info(tr("%s%s changed status: %s (AIM).\n"), L_SRVxSTR,
              u->getAlias().c_str(), u->statusString().c_str());
        if ( (nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS) )
          gLog.Unknown("%sUnknown status flag for %s: 0x%08lX\n",
                       L_UNKNOWNxSTR, u->GetAlias(),
                       nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS);
        u->setAutoResponse("");
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

        if (u.isLocked())
        {
        u->SetSupportsUTF8(bUTF8);
        if (version != "")
            u->setClientInfo(version);
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
            gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                Licq::PluginSignal::UserPluginStatus, u->id()));
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
              u->setBuddyIconHash(PrintHex(HashHex.get(), Hash.get(), HashLength));
              u->setBuddyIconType(IconType);
              u->setBuddyIconHashType(HashType);
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

      if (nOldStatus == ICQ_STATUS_OFFLINE)
        gOnEventManager.performOnEvent(OnEventManager::OnEventOnline, *u);
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
        Licq::UserReadGuard user(Licq::UserId(szId, LICQ_PPID));
      //XXX Debug output
      //gLog.Error("%sIgnoring fake offline: %s (%s)\n", L_SRVxSTR,
      //    user->GetAlias(), szId);
      delete [] szId;
      break;
    }

      Licq::UserWriteGuard u(Licq::UserId(szId, LICQ_PPID));
      if (!u.isLocked())
      {
      gLog.Warn(tr("%sUnknown user (%s) has gone offline.\n"), L_WARNxSTR, szId);
      delete [] szId;
      break;
    }
    delete [] szId;

    gLog.Info(tr("%s%s went offline.\n"), L_SRVxSTR, u->GetAlias());
    u->SetClientTimestamp(0);
      u->setIsTyping(false);
      u->statusChanged(User::OfflineStatus);
      gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
          Licq::PluginSignal::UserTyping, u->id()));
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
void IcqProtocol::ProcessMessageFam(CBuffer &packet, unsigned short nSubtype)
{
  /*unsigned short Flags =*/ packet.UnpackUnsignedShortBE();
  unsigned long nSubSequence = packet.UnpackUnsignedLongBE();

  switch (nSubtype)
  {
  case ICQ_SNACxMSG_ICBMxERROR:
  {
    unsigned short err = packet.UnpackUnsignedShortBE();

      Licq::Event* e = DoneServerEvent(nSubSequence, Licq::Event::ResultError);

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
            {
              Licq::UserReadGuard u(Licq::UserId((*iter)->myIdString, LICQ_PPID));
              if (!u.isLocked())
                gLog.Warn("%sReverse connection from %s failed.\n", L_WARNxSTR,
                    (*iter)->myIdString.c_str());
              else
                gLog.Warn("%sReverse connection from %s failed.\n", L_WARNxSTR,
                    u->getAlias().c_str());
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
      Licq::UserId userId(szId, LICQ_PPID);

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
          char* szMsg;
      // Get the user and allow adding unless we ignore new users
          {
            Licq::UserWriteGuard u(userId, !gDaemon.ignoreType(Daemon::IgnoreNewUsers));
            if (!u.isLocked())
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
            const char* szEncoding = ignore ? "" : u->userEncoding().c_str();
        char* szTmpMsg = gTranslator.FromUTF16(szMessage, szEncoding, nMsgLen);
        delete [] szMessage;
        szMessage = szTmpMsg;
      }

            szMsg = gTranslator.RNToN(szMessage);
      delete [] szMessage;

          // Unlock user mutex before parsing message so we don't block other threads
          //   for a long time since parser may blocks to prompt for GPG passphrase.
          }

      // now send the message to the user
          Licq::EventMsg* e = Licq::EventMsg::Parse(szMsg, ICQ_CMDxRCV_SYSxMSGxONLINE, nTimeSent, 0);
      delete [] szMsg;

      if (ignore)
      {
            gDaemon.rejectEvent(userId, e);
            break;
      }

          Licq::UserWriteGuard u(userId, !gDaemon.ignoreType(Daemon::IgnoreNewUsers));
          u->setIsTyping(false);

          if (gDaemon.addUserEvent(*u, e))
            gOnEventManager.performOnEvent(OnEventManager::OnEventMessage, *u);
          gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserTyping, u->id()));
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

          {
            Licq::UserWriteGuard u(userId);
            if (u.isLocked())
              u->SetSupportsUTF8(bUTF8);
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
            Licq::User* u = gUserManager.fetchUser(userId, LOCK_W);
        if (u == NULL)
        {
              u = new User(userId);
          bNewUser = true;
        }

        ProcessPluginMessage(advMsg, u, nChannel, bIsAck, nMsgID[0], nMsgID[1],
                             nSequence, NULL);

        if (bNewUser)
          delete u;
            else
              gUserManager.dropUser(u);

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
          Licq::User* u = gUserManager.fetchUser(userId, LOCK_W);
      if (u == NULL)
      {
            u = new User(userId);
        bNewUser = true;
      }

          u->setIsTyping(false);

      if (msgTxt.getTLVLen(0x0004) == 4)
      {
        unsigned long Ip = BE_32(msgTxt.UnpackUnsignedLongTLV(0x0004));
        u->SetIp(Ip);
      }

      // Special status to us?
      if (!bIsAck && !bNewUser && nStatus != ICQ_STATUS_OFFLINE &&
          !(nStatus == ICQ_STATUS_ONLINE && u->Status() == ICQ_STATUS_FREEFORCHAT) &&
          nStatus != (u->Status() | (u->isInvisible() ? ICQ_STATUS_FxPRIVATE : 0)))
      {
        bool r = u->OfflineOnDisconnect() || !u->isOnline();
        ChangeUserStatus(u, (u->StatusFull() & ICQ_STATUS_FxFLAGS) | nStatus);
        gLog.Info(tr("%s%s (%s) is %s to us.\n"), L_TCPxSTR, u->GetAlias(),
              u->id().toString().c_str(), u->statusString().c_str());
        if (r) u->SetOfflineOnDisconnect(true);
      }

      if (u->Version() == 0x0A)
      {
        // We removed the conversion from before, but with this version we need
        // it back. Go figure.
        gTranslator.ClientToServer(szMsg);
      }

          Licq::UserId userId = u->id();
      if (!bNewUser)
            gUserManager.dropUser(u);

      // Handle it
      ProcessMessage(u, advMsg, szMsg, nMsgType, nMask, nMsgID,
                     nSequence, bIsAck, bNewUser);

      delete [] szMsg;
      if (bNewUser) // can be changed in ProcessMessage
      {
        delete u;
        break;
      }

          gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserTyping, userId));
          break;
        }
    case 4:
    //Version 5 (V5) protocol messages incapsulated in v7 packet.
    {
      CBuffer msgTxt = packet.UnpackTLV(0x0005);
      msgTxt.UnpackUnsignedLongBE();
      unsigned short nTypeMsg = msgTxt.UnpackUnsignedShort();
          unsigned long nMask = ((nTypeMsg & ICQ_CMDxSUB_FxMULTIREC) ? (int)Licq::UserEvent::FlagMultiRec : 0);
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
      OnEventManager::OnEventType onEventType = OnEventManager::OnEventMessage;
          Licq::UserEvent* eEvent = NULL;

      switch(nTypeMsg)
      {
        case ICQ_CMDxSUB_MSG:
            {
              Licq::EventMsg* e = Licq::EventMsg::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, nTimeSent, nMask);
          szType = strdup(tr("Message"));
          onEventType = OnEventManager::OnEventMessage;
          eEvent = e;
          break;
        }
        case ICQ_CMDxSUB_URL:
            {
              Licq::EventUrl* e = Licq::EventUrl::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, nTimeSent, nMask);
          if (e == NULL)
          {
            char *buf;
            gLog.Warn(tr("%sInvalid URL message:\n%s\n"), L_WARNxSTR, packet.print(buf));
            delete [] buf;
            break;
          }
          szType = strdup(tr("URL"));
          onEventType = OnEventManager::OnEventUrl;
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

              Licq::EventAuthRequest* e = new Licq::EventAuthRequest(userId,
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

              Licq::EventAuthRefused* e = new Licq::EventAuthRefused(userId, szMessage,
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

              {
                Licq::UserWriteGuard u(userId);
                if (u.isLocked())
                  u->SetAwaitingAuth(false);
              }

              Licq::EventAuthGranted* e = new Licq::EventAuthGranted(userId,
            szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, nTimeSent, 0);
          eEvent = e;
          break;
        }
        case ICQ_CMDxSUB_MSGxSERVER:
        {
          gLog.Info(tr("%sServer message.\n"), L_BLANKxSTR);

              Licq::EventServerMessage* e = Licq::EventServerMessage::Parse(szMessage,
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

              Licq::EventAdded* e = new Licq::EventAdded(userId, szFields[0],
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
              Licq::EventWebPanel* e = new Licq::EventWebPanel(szFields[0], szFields[3],
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
              Licq::EventEmailPager* e = new Licq::EventEmailPager(szFields[0], szFields[3],
                                                     szFields[5], ICQ_CMDxRCV_SYSxMSGxONLINE,
                                                     nTimeSent, 0);
          delete [] szFields;
          eEvent = e;
          break;
        }
        case ICQ_CMDxSUB_CONTACTxLIST:
        {
              Licq::EventContactList* e = Licq::EventContactList::Parse(szMessage,
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
          onEventType = OnEventManager::OnEventMessage;
          eEvent = e;
          break;
        }
        case ICQ_CMDxSUB_SMS:
            {
              Licq::EventSms* e = Licq::EventSms::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE,
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
              Licq::EventUnknownSysMsg* e = new Licq::EventUnknownSysMsg(nTypeMsg, ICQ_CMDxRCV_SYSxMSGxONLINE,
                  userId, szMessage, nTimeSent, 0);

              Licq::OwnerWriteGuard o(LICQ_PPID);
              gDaemon.addUserEvent(*o, e);
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
                Licq::UserWriteGuard u(userId, !gDaemon.ignoreType(Daemon::IgnoreNewUsers));
                if (!u.isLocked())
                {
          gLog.Info(tr("%s%s from new user (%s), ignoring.\n"), L_SBLANKxSTR, szType, szId);

          if (szType) free(szType);

          //TODO
              gDaemon.rejectEvent(userId, eEvent);
              break;
	    }
	    else
	      gLog.Info(tr("%s%s through server from %s (%s).\n"), L_SBLANKxSTR,
	    					szType, u->GetAlias(), u->IdString());

                u->setIsTyping(false);

	    if (szType) free(szType);
                if (gDaemon.addUserEvent(*u, eEvent))
                  gOnEventManager.performOnEvent(onEventType, *u);
                gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserTyping, u->id()));
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
                bool bIgnore;
                {
                  Licq::UserReadGuard u(userId);
                  bIgnore = (u.isLocked() && u->IgnoreList());
                }

            if (bIgnore)
            {
              delete eEvent; // Processing stops here, needs to be deleted
              break;
            }

                Licq::OwnerWriteGuard o(LICQ_PPID);
                if (gDaemon.addUserEvent(*o, eEvent))
                {
                  eEvent->AddToHistory(*o, true);
                  gOnEventManager.performOnEvent(OnEventManager::OnEventSysMsg, *o);
                }
                break;
	  }
	  case ICQ_CMDxSUB_SMS:
	  {
                Licq::EventSms* eSms = dynamic_cast<Licq::EventSms*>(eEvent);
      //TODO
                string idSms = findUserByCellular(eSms->number());

      if (!idSms.empty())
      {
        //TODO
                  Licq::UserWriteGuard u(Licq::UserId(idSms.c_str(), LICQ_PPID));
                  gLog.Info(tr("%sSMS from %s - %s (%s).\n"), L_SBLANKxSTR, eSms->number().c_str(),
                      u->getAlias().c_str(), idSms.c_str());
                  if (gDaemon.addUserEvent(*u, eEvent))
                    gOnEventManager.performOnEvent(OnEventManager::OnEventSms, *u);
                }
                else
                {
                  Licq::OwnerWriteGuard o(LICQ_PPID);
                  gLog.Info(tr("%sSMS from %s.\n"), L_BLANKxSTR, eSms->number().c_str());
                  if (gDaemon.addUserEvent(*o, eEvent))
                  {
                    eEvent->AddToHistory(*o, true);
                    gOnEventManager.performOnEvent(OnEventManager::OnEventSms, *o);
	          }
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
      Licq::ExtendedData* pExtendedAck = 0;
      Licq::User* u = NULL;

	 	packet.incDataPosRead(4); // msg id
		nMsgID = packet.UnpackUnsignedLongBE(); // lower bits, what licq uses
		nFormat = packet.UnpackUnsignedShortBE();
		nUin = packet.UnpackUinString();
      char id[16];
      snprintf(id, 15, "%lu", nUin);

      u = gUserManager.fetchUser(Licq::UserId(id, LICQ_PPID), LOCK_W);
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
        gUserManager.dropUser(u);
        return;
      }

    packet.incDataPosRead(2);
    packet >> nLen;
    if (nLen == 0x0200)
    {
      gLog.Warn(tr("%s%s doesn't have a manager for this event.\n"), L_WARNxSTR,
        u->GetAlias());
        gUserManager.dropUser(u);

        Licq::Event* e = DoneServerEvent(nMsgID, Licq::Event::ResultError);
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

        gUserManager.dropUser(u);
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
        pExtendedAck = new Licq::ExtendedData(false, 0, szMessage);
      nSubResult = ICQ_TCPxACK_REFUSE;
      gLog.Info(tr("%sRefusal from %s (#%lu).\n"), L_SRVxSTR, u->GetAlias(),
        nMsgID);
    }
    else
    {
      // Update the away message if it's changed
      if (u->autoResponse() != szMessage)
      {
        u->setAutoResponse(szMessage);
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

        pExtendedAck = new Licq::ExtendedData(nSubResult == ICQ_TCPxACK_RETURN, 0, szMessage);
      }
        gUserManager.dropUser(u);
      delete [] szMessage;

      Licq::Event* e = DoneServerEvent(nMsgID, Licq::Event::ResultAcked);
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
      Licq::Event* e = DoneServerEvent(nSubSequence, Licq::Event::ResultAcked);
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

      Licq::UserWriteGuard u(Licq::UserId(szId, LICQ_PPID));
      if (!u.isLocked())
      {
        gLog.Warn(tr("%sTyping status received for unknown user (%s).\n"),
            L_WARNxSTR, szId);
        break;
      }
      u->setIsTyping(nTyping == ICQ_TYPING_ACTIVE);
      gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
          Licq::PluginSignal::UserTyping, u->id()));
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
void IcqProtocol::ProcessListFam(CBuffer &packet, unsigned short nSubtype)
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
      DoneServerEvent(nSubSequence, Licq::Event::ResultSuccess);

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

            Licq::TlvList list = packet.getTlvList();
            for (Licq::TlvList::iterator it = list.begin(); it != list.end(); it++)
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
              unsigned short nGroup = Licq::gUserManager.GetGroupFromID(nTag);
              char* szUnicodeName = gTranslator.FromUnicode(szId);

              if (nGroup == 0)
              {
                if (!Licq::gUserManager.AddGroup(szUnicodeName, nTag))
                  Licq::gUserManager.ModifyGroupID(szUnicodeName, nTag);
              }
              else
              {
                Licq::gUserManager.RenameGroup(nGroup, szUnicodeName, false);
              }
              
              if (szUnicodeName)
                delete[] szUnicodeName;

              // This is bad, i don't think we want to call this at all..
              // it will add users to different groups that they werent even
              // assigned to
              //if (Licq::gUserManager.UpdateUsersInGroups())
              //{
              //  gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalList,
              //      Licq::PluginSignal::ListInvalidate, 0));
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

            Licq::OwnerWriteGuard o(LICQ_PPID);
            gLog.Info(tr("%sGot Privacy Setting.\n"), L_SRVxSTR);
            o->SetPDINFO(nID);
            if (cPrivacySettings == ICQ_PRIVACY_ALLOW_FOLLOWING)
              ChangeUserStatus(*o, o->StatusFull() | ICQ_STATUS_FxPRIVATE);
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
      {
        Licq::OwnerWriteGuard o(LICQ_PPID);
        o->SetSSTime(nTime);
        o->SetSSCount(nCount);
      }

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
        DoneServerEvent(nSubSequence, Licq::Event::ResultSuccess);

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

      Licq::UserWriteGuard u(Licq::UserId(szId, LICQ_PPID));
      if (u.isLocked())
      {
        // First update their gsid/sid
        u->SetSID(sid);
        u->removeFromGroup(Licq::gUserManager.GetGroupFromID(u->GetGSID()));
        u->SetGSID(gsid);
        u->addToGroup(Licq::gUserManager.GetGroupFromID(gsid));

        // Now the the tlv of attributes to attach to the user
        Licq::TlvList tlvList = packet.getTlvList();
        Licq::TlvList::iterator iter;
        for (iter = tlvList.begin(); iter != tlvList.end(); ++iter)
          u->AddTLV(iter->second);

        u->SaveLicqInfo();
        gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
            Licq::PluginSignal::UserGroups, u->id()));
      }

      delete [] szId;

      break;
    }

    case ICQ_SNACxLIST_ROSTxSYNCED:
    {
      DoneServerEvent(nSubSequence, Licq::Event::ResultSuccess);

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

      Licq::Event* e = DoneServerEvent(nSubSequence, Licq::Event::ResultSuccess);

      if (e == NULL)
      {
        gLog.Warn(tr("%sServer list update ack without request.\n"), L_SRVxSTR);
        break;
      }

      unsigned long nListTime;
      {
        Licq::OwnerWriteGuard o(LICQ_PPID);
        nListTime = o->GetSSTime();
        o->SetSSTime(time(0));
      }

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
                icqAddUserServer(Licq::UserId(pending, LICQ_PPID), true);
              break;
            }

            bool bTopLevelUpdated = false;

            if (bHandled == false)
            {
              bHandled = true;
              int n = Licq::gUserManager.GetGroupFromID(e->ExtraInfo());
              if (n < 1 && e->ExtraInfo() != 0)
                break;

              std::string groupName = "";
              if (e->ExtraInfo() != 0)
              {
                Licq::GroupReadGuard group(n);
                if (group.isLocked())
                  groupName = group->name();
              }

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
              Licq::UserWriteGuard u(Licq::UserId(pending.c_str(), LICQ_PPID));
              if (u.isLocked())
              {
                u->addToGroup(Licq::gUserManager.GetGroupFromID(e->ExtraInfo()));
                gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalAddedToServer,
                    0, u->id()));
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
      Licq::UserId userId(szId, LICQ_PPID);
      bool bIgnore;
      {
        Licq::UserReadGuard u(userId);
        bIgnore = (u.isLocked() && u->IgnoreList());
      }

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

      Licq::EventAuthRequest* e = new Licq::EventAuthRequest(userId, "", "", "", "", nMsgLen ? szMsg : "",
                                                   ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);

      Licq::OwnerWriteGuard o(LICQ_PPID);
      if (gDaemon.addUserEvent(*o, e))
      {
        e->AddToHistory(*o, true);
        gOnEventManager.performOnEvent(OnEventManager::OnEventSysMsg, *o);
      }

      delete [] szId;
      delete [] szMsg;
      break;
    }

    case ICQ_SNACxLIST_AUTHxRESPONS: // The resonse to our authorization request
    {
      char *szId = packet.UnpackUserString();
      Licq::UserId userId(szId, LICQ_PPID);
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

      Licq::UserEvent* eEvent;
      if (granted)
      {
        eEvent = new Licq::EventAuthGranted(userId, szMsg,
           ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);

        Licq::UserWriteGuard u(userId);
        if (u.isLocked())
        {
           u->SetAwaitingAuth(false);
           u->RemoveTLV(0x0066);
        }
      }
      else
      {
        eEvent = new Licq::EventAuthRefused(userId, szMsg,
            ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);
      }

      Licq::OwnerWriteGuard o(LICQ_PPID);
      if (gDaemon.addUserEvent(*o, eEvent))
      {
        eEvent->AddToHistory(*o, true);
        gOnEventManager.performOnEvent(OnEventManager::OnEventSysMsg, *o);
      }

      delete [] szId;
      delete [] szMsg;
      break;
    }

    case ICQ_SNACxLIST_AUTHxADDED: // You were added to a contact list
    {
      char *szId = packet.UnpackUserString();
      Licq::UserId userId(szId, LICQ_PPID);
      gLog.Info(tr("%sUser %s added you to their contact list.\n"), L_SRVxSTR,
                szId);

      Licq::EventAdded* e = new Licq::EventAdded(userId, "", "", "", "",
                                       ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);
      {
        Licq::OwnerWriteGuard o(LICQ_PPID);
        if (gDaemon.addUserEvent(*o, e))
        {
          e->AddToHistory(*o, true);
          gOnEventManager.performOnEvent(OnEventManager::OnEventSysMsg, *o);
        }
      }

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
void IcqProtocol::ProcessBOSFam(CBuffer& /* packet */, unsigned short nSubtype)
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
      Licq::Event* e = DoneExtendedServerEvent(0, Licq::Event::ResultSuccess);
    if (e != NULL) ProcessDoneEvent(e);
      gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalLogon, 0));

    //icqSetStatus(m_nDesiredStatus);
    break;
  }
  default:
    gLog.Warn(tr("%sUnknown BOS Family Subtype: %04hx\n"), L_SRVxSTR, nSubtype);
    break;
  }
}

//--------ProcessVariousFam-----------------------------------------
void IcqProtocol::ProcessVariousFam(CBuffer &packet, unsigned short nSubtype)
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
          Licq::UserId userId(id, LICQ_PPID);

      sendTM.tm_year = msg.UnpackUnsignedShort() - 1900;
      sendTM.tm_mon = msg.UnpackChar() - 1;
      sendTM.tm_mday = msg.UnpackChar();
      sendTM.tm_hour = msg.UnpackChar();
      sendTM.tm_min = msg.UnpackChar();
      sendTM.tm_sec = 0;
      sendTM.tm_isdst = -1;

          {
            Licq::OwnerReadGuard o(LICQ_PPID);
            nTimeSent = mktime(&sendTM) - o->SystemTimeGMTOffset();
          }

      // Msg type & flags
      unsigned short nTypeMsg = msg.UnpackUnsignedShort();
          unsigned long nMask = ((nTypeMsg & ICQ_CMDxSUB_FxMULTIREC) ? (int)Licq::UserEvent::FlagMultiRec : 0);
      nTypeMsg &= ~ICQ_CMDxSUB_FxMULTIREC;
      
      char* szMessage = new char[msg.getDataMaxSize()];
      // 2 byte length little endian + string
      msg.UnpackString(szMessage, msg.getDataMaxSize());      
      char *szType = NULL;
          OnEventManager::OnEventType onEventType = OnEventManager::OnEventMessage;
          Licq::UserEvent* eEvent = NULL;

      switch(nTypeMsg)
      {
        case ICQ_CMDxSUB_MSG:
            {
              Licq::EventMsg* e = Licq::EventMsg::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, nMask);
	  szType = strdup(tr("Message"));
              onEventType = OnEventManager::OnEventMessage;
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_URL:
            {
              Licq::EventUrl* e = Licq::EventUrl::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, nMask);
	  if (e == NULL)
	  {
	    char *buf;
	    
	    gLog.Warn(tr("%sInvalid offline URL message:\n%s\n"), L_WARNxSTR, packet.print(buf));
	    delete [] buf;
	    break;
	  }
	  szType = strdup(tr("URL"));
              onEventType = OnEventManager::OnEventUrl;
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

              Licq::EventAuthRequest* e = new Licq::EventAuthRequest(userId,
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

              Licq::EventAuthRefused* e = new Licq::EventAuthRefused(userId,
                  szMessage, ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, 0);
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_AUTHxGRANTED:  // system message: authorized
        {
              gLog.Info(tr("%sOffline authorization granted by %s.\n"), L_SBLANKxSTR, id);

          // translating string with Translation Table
          gTranslator.ServerToClient (szMessage);

              {
                Licq::UserWriteGuard u(userId);
                if (u.isLocked())
                  u->SetAwaitingAuth(false);
              }

              Licq::EventAuthGranted* e = new Licq::EventAuthGranted(userId,
                  szMessage, ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, 0);
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_MSGxSERVER:
	{
	  gLog.Info(tr("%sOffline server message.\n"), L_BLANKxSTR);

              Licq::EventServerMessage* e = Licq::EventServerMessage::Parse(szMessage,
                  ICQ_CMDxSUB_MSGxSERVER, nTimeSent, nMask);
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

              Licq::EventAdded* e = new Licq::EventAdded(userId, szFields[0],
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
              Licq::EventWebPanel* e = new Licq::EventWebPanel(szFields[0], szFields[3], szFields[5],
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
              Licq::EventEmailPager* e = new Licq::EventEmailPager(szFields[0], szFields[3], szFields[5],
                                                     ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, 0);
	  delete [] szFields;	
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_CONTACTxLIST:
            {
              Licq::EventContactList* e = Licq::EventContactList::Parse(szMessage,
                  ICQ_CMDxRCV_SYSxMSGxOFFLINE, nTimeSent, nMask);
          if (e == NULL)
          {
            char *buf;
 
            gLog.Warn(tr("%sInvalid offline Contact List message:\n%s\n"), L_WARNxSTR,
		      packet.print(buf));
            delete [] buf;
            break;
          }
	  szType = strdup(tr("Contacts"));
              onEventType = OnEventManager::OnEventMessage;
	  eEvent = e;
	  break;
	}
	case ICQ_CMDxSUB_SMS:
            {
              Licq::EventSms* e = Licq::EventSms::Parse(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE, nTimeSent, nMask);
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
              Licq::EventUnknownSysMsg* e = new Licq::EventUnknownSysMsg(nTypeMsg,
                  ICQ_CMDxRCV_SYSxMSGxOFFLINE, userId, szMessage, nTimeSent, 0);

              Licq::OwnerWriteGuard o(LICQ_PPID);
              gDaemon.addUserEvent(*o, e);
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
                Licq::UserWriteGuard u(userId, !gDaemon.ignoreType(Daemon::IgnoreNewUsers));
                if (!u.isLocked())
                {
                    gLog.Info(tr("%sOffline %s from new user (%s), ignoring.\n"),
                        L_SBLANKxSTR, szType, id);
                    if (szType) free(szType);
                    gDaemon.rejectEvent(userId, eEvent);
                    break;
                }
                else
                  gLog.Info(tr("%sOffline %s through server from %s (%s).\n"),
                      L_SBLANKxSTR, szType, u->GetAlias(), id);

            if (szType) free(szType);
                if (gDaemon.addUserEvent(*u, eEvent))
                  gOnEventManager.performOnEvent(onEventType, *u);
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
                bool bIgnore = false;
                {
                  Licq::UserReadGuard u(userId);
                  if (u.isLocked())
                    bIgnore = u->IgnoreList();
                }

            if (bIgnore)
            {
              delete eEvent; // Processing stops here, needs to be deleted
              gLog.Info("Ignored!");
              break;
            }

                Licq::OwnerWriteGuard o(LICQ_PPID);
                if (gDaemon.addUserEvent(*o, eEvent))
	        {
                  eEvent->AddToHistory(*o, true);
                  gOnEventManager.performOnEvent(OnEventManager::OnEventSysMsg, *o);
                }
                break;
              }
	  case ICQ_CMDxSUB_SMS:
              {
                Licq::EventSms* eSms = dynamic_cast<Licq::EventSms*>(eEvent);
                string idSms = findUserByCellular(eSms->number());

                if (!idSms.empty())
                {
                  Licq::UserWriteGuard u(Licq::UserId(idSms.c_str(), LICQ_PPID));
                  gLog.Info(tr("%sOffline SMS from %s - %s (%s).\n"), L_SBLANKxSTR,
                      eSms->number().c_str(), u->getAlias().c_str(), id);
                  if (gDaemon.addUserEvent(*u, eEvent))
                    gOnEventManager.performOnEvent(OnEventManager::OnEventSms, *u);
                }
                else
                {
                  Licq::OwnerWriteGuard o(LICQ_PPID);
	          gLog.Info(tr("%sOffline SMS from %s.\n"), L_BLANKxSTR, eSms->number().c_str());
                  if (gDaemon.addUserEvent(*o, eEvent))
                  {
                    eEvent->AddToHistory(*o, true);
                    gOnEventManager.performOnEvent(OnEventManager::OnEventSms, *o);
                  }
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
          Licq::Event* pEvent = NULL;
      nSubtype = msg.UnpackUnsignedShort();
      nResult = msg.UnpackChar();
      char *szType = NULL;

      if (nSubtype == ICQ_CMDxMETA_PASSWORDxRSP)
      {
        szType = strdup(tr("Password change"));
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);
        if (pEvent != NULL && nResult == META_SUCCESS)
        {
              Licq::OwnerWriteGuard o(LICQ_PPID);
          o->SetEnableSave(false);
              o->setPassword(((CPU_SetPassword *)pEvent->m_pPacket)->m_szPassword);
          o->SetEnableSave(true);
          o->SaveLicqInfo();
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_SECURITYxRSP)
      {
        szType = strdup(tr("Security info"));
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);
      }
      else if (nSubtype == ICQ_CMDxMETA_GENERALxINFOxRSP)
      {
        szType = strdup(tr("General info"));
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

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

              Licq::OwnerWriteGuard o(LICQ_PPID);
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
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_EMAILxINFOxRSP)
      {
        szType = strdup(tr("E-mail info"));
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetEmailInfo *p = (CPU_Meta_SetEmailInfo *)pEvent->m_pPacket;

              // translating string with Translation Table
              gTranslator.ServerToClient(p->m_szEmailSecondary);
              gTranslator.ServerToClient(p->m_szEmailOld);

              Licq::OwnerWriteGuard o(LICQ_PPID);

          o->SetEnableSave(false);
              o->setUserInfoString("Email2", p->m_szEmailSecondary);
              o->setUserInfoString("Email0", p->m_szEmailOld);

          // save the user infomation
          o->SetEnableSave(true);
              o->saveUserInfo();
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_MORExINFOxRSP)
      {
        szType = strdup(tr("More info"));
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetMoreInfo *p = (CPU_Meta_SetMoreInfo *)pEvent->m_pPacket;

              // translating string with Translation Table
              gTranslator.ServerToClient(p->m_szHomepage);

              Licq::OwnerWriteGuard o(LICQ_PPID);
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
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_INTERESTSxINFOxRSP)
      {
        szType = strdup("Interests info");
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetInterestsInfo *p =
                           (CPU_Meta_SetInterestsInfo *)pEvent->m_pPacket;
              Licq::OwnerWriteGuard o(LICQ_PPID);
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
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_WORKxINFOxRSP)
      {
        szType = strdup(tr("Work info"));
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

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

              Licq::OwnerWriteGuard o(LICQ_PPID);
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
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_ABOUTxRSP)
      {
        szType = strdup(tr("About"));
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetAbout *p = (CPU_Meta_SetAbout *)pEvent->m_pPacket;
              Licq::OwnerWriteGuard o(LICQ_PPID);
          char* msg = gTranslator.RNToN(p->m_szAbout);
          gTranslator.ServerToClient(msg);
          o->SetEnableSave(false);
              o->setUserInfoString("About", msg);
          delete [] msg;

          // save the user infomation
          o->SetEnableSave(true);
              o->saveUserInfo();
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_SENDxSMSxRSP)
      {
        // this one sucks, it could be sms or organization response
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (pEvent != NULL &&
            pEvent->m_pPacket->SubCommand() == ICQ_CMDxMETA_ORGBACKxINFOxSET)
        {
          szType = strdup("Organizations/Background info");

          if (nResult == META_SUCCESS)
          {
            CPU_Meta_SetOrgBackInfo *p =
                             (CPU_Meta_SetOrgBackInfo *)pEvent->m_pPacket;
            Licq::OwnerWriteGuard o(LICQ_PPID);
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
                      pEvent->m_eResult = Licq::Event::ResultSuccess;
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
                      pEvent->m_eResult = Licq::Event::ResultFailed;
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
                      pEvent->m_eResult = Licq::Event::ResultFailed;
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
                      pEvent->m_eResult = Licq::Event::ResultFailed;
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
                    pEvent->m_eResult = Licq::Event::ResultFailed;
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
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
              Licq::OwnerWriteGuard o(LICQ_PPID);
          o->SetRandomChatGroup(((CPU_SetRandomChatGroup *)pEvent->m_pPacket)->Group());
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_RANDOMxUSERxRSP)
      {
            Licq::Event* e = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (e != NULL && nResult == META_SUCCESS)
        {
          unsigned long nUin, nIp;
          char nMode;
          msg >> nUin;
          char szUin[14];
          snprintf(szUin, sizeof(szUin), "%lu", nUin);
              Licq::UserId userId(szUin, LICQ_PPID);
          gLog.Info(tr("%sRandom chat user found (%s).\n"), L_SRVxSTR, szUin);
          bool bNewUser = false;

              {
                Licq::UserWriteGuard u(userId, true, &bNewUser);

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
              }

          if (bNewUser)
                icqRequestMetaInfo(userId);

              e->m_pSearchAck = new Licq::SearchData(userId);
            }
        else
          gLog.Info(tr("%sNo random chat user found.\n"), L_SRVxSTR);
        ProcessDoneEvent(e);
      }
      else if (nSubtype == ICQ_CMDxMETA_WPxINFOxSET_RSP)
      {
            Licq::Event* e = DoneServerEvent(nSubSequence,
                nResult == 0x0A ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);
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
            Licq::Event* e = DoneServerEvent(nSubSequence, Licq::Event::ResultFailed);
        gLog.Info(tr("%sSMS failed to send.\n"), L_SRVxSTR);
        if (e != NULL)
        ProcessDoneEvent(e);
      }
      // Search results need to be processed differently
      else if (nSubtype == 0x0190 || nSubtype == 0x019a ||
               nSubtype == 0x01a4 || nSubtype == 0x01ae)
      {
            Licq::Event* e = NULL;

        if (nResult == 0x32) // No results found
        {
          gLog.Info(tr("%sWhitePages search found no users.\n"), L_SRVxSTR);
              e = DoneExtendedServerEvent(nSubSequence, Licq::Event::ResultSuccess);
              Licq::Event* e2 = new Licq::Event(e);
          e2->m_pSearchAck = NULL; // Search ack is null lets plugins know no results found
          e2->m_nCommand = ICQ_CMDxSND_META;
          e2->m_nSubCommand = ICQ_CMDxMETA_SEARCHxWPxLAST_USER;
              gDaemon.PushPluginEvent(e2);
              DoneEvent(e, Licq::Event::ResultSuccess);
              break;
            }

            e = DoneExtendedServerEvent(nSubSequence, Licq::Event::ResultAcked);
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
            Licq::UserId foundUserId(foundAccount, LICQ_PPID);

            Licq::SearchData* s = new Licq::SearchData(foundUserId);

            msg.UnpackString(szTemp, sizeof(szTemp));
            gTranslator.ServerToClient(szTemp);
            s->myAlias = szTemp;
            msg.UnpackString(szTemp, sizeof(szTemp));
            gTranslator.ServerToClient(szTemp);
            s->myFirstName = szTemp;
            msg.UnpackString(szTemp, sizeof(szTemp));
            gTranslator.ServerToClient(szTemp);
            s->myLastName = szTemp;
            s->myEmail = msg.unpackString();
            s->myAuth = msg.UnpackChar(); // authorization required
            s->myStatus = msg.UnpackChar();
        msg.UnpackChar(); // unknown
            s->myGender = msg.UnpackChar(); // gender
            s->myAge = msg.UnpackChar(); // age
        //TODO: Find out what these unknowns are. The first UnpackChar has been unknown for a long time, the others
        //seem fairly new.
        msg.UnpackChar(); // unknown
        msg.UnpackChar();
        msg.UnpackUnsignedLong();
        msg.UnpackUnsignedLong();
        msg.UnpackUnsignedLong();

            gLog.Info("%s%s (%lu) <%s %s, %s>\n", L_SBLANKxSTR, s->alias().c_str(),
                nFoundUin, s->firstName().c_str(), s->lastName().c_str(), s->email().c_str());

            Licq::Event* e2 = new Licq::Event(e);
        // JON: Hack it so it is backwards compatible with plugins for now.
        e2->m_nCommand = ICQ_CMDxSND_META;
        e2->m_pSearchAck = s;
        if (nSubtype & 0x0008)
        {
          unsigned long nMore = 0;
          e2->m_nSubCommand = ICQ_CMDxMETA_SEARCHxWPxLAST_USER;
          nMore = msg.UnpackUnsignedLong();
          // No more subtraction by 1, and now it seems to always be 0
              e2->m_pSearchAck->myMore = nMore;
              e2->m_eResult = Licq::Event::ResultSuccess;
        }
        else
        {
          e2->m_nSubCommand = ICQ_CMDxMETA_SEARCHxWPxFOUND;
              e2->m_pSearchAck->myMore = 0;
        }

            gDaemon.PushPluginEvent(e2);

        if (nSubtype & 0x0008)
              DoneEvent(e, Licq::Event::ResultSuccess); // Remove it from the running event list
        else
          PushExtendedEvent(e);
      }
          else
          {
            Licq::Event* e = NULL;
        Licq::User* u = NULL;
        Licq::UserId userId;
        bool multipart = false;

        if ((nResult == 0x32) || (nResult == 0x14) || (nResult == 0x1e))
        {
          // error: empty result or nonexistent user (1E =  readonly???)
          gLog.Warn(tr("%sFailed to update user info: %x.\n"), L_WARNxSTR, nResult);
              e = DoneExtendedServerEvent(nSubSequence, Licq::Event::ResultFailed);
          if (e)
          {
                DoneEvent(e, Licq::Event::ResultFailed);
            ProcessDoneEvent(e);
          }
          e = NULL;
          break;
        }
        else
        {
          // Find the relevant event
              e = DoneExtendedServerEvent(nSubSequence, Licq::Event::ResultSuccess);
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
          if (!u->m_bKeepAliasOnUpdate || userId == Licq::gUserManager.ownerUserId(LICQ_PPID))
          {
                  char *szUTFAlias = tmp ? gTranslator.ToUnicode(tmp, u->userEncoding().c_str()) : 0;
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

                if (!u->isUser())
                {
                  dynamic_cast<Owner*>(u)->SetWebAware(nStatus);
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

                gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserBasic, u->id()));
                gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
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

                gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
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

                gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
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

                gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
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

                gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
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

                gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
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

                gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
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

                gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
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
                DoneEvent(e, Licq::Event::ResultSuccess);
            ProcessDoneEvent(e);
          } else {
            gLog.Warn(tr("%sResponse to unknown extended info request for %s (%s).\n"),
                      L_WARNxSTR, u->getAlias().c_str(), userId.toString().c_str());
          }
        }

            gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                Licq::PluginSignal::UserInfo, u->id()));
            gUserManager.dropUser(u);
          }

      if (szType)
      {
        if (pEvent)
            {
              if (pEvent->Result() == Licq::Event::ResultSuccess)
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
void IcqProtocol::ProcessAuthFam(CBuffer &packet, unsigned short nSubtype)
{
  /*unsigned long Flags =*/ packet.UnpackUnsignedLongBE();
  unsigned short nSubSequence = packet.UnpackUnsignedShortBE();

  switch (nSubtype)
  {
    case ICQ_SNACxNEW_UIN_ERROR:
    {
      if (!myRegisterPasswd.empty())
      {
        gLog.Warn(tr("%sVerification required. Reconnecting...\n"), L_WARNxSTR);

        Licq::Event* e = DoneServerEvent(nSubSequence, Licq::Event::ResultError);
        if (e)
          delete e;
        m_bVerify = true;
        
        // Reconnect now
        string passwd = myRegisterPasswd; // gets deleted in postLogoff
        int nSD = m_nTCPSrvSocketDesc;
        m_nTCPSrvSocketDesc = -1;
        m_eStatus = STATUS_OFFLINE_MANUAL;
        m_bLoggingOn = false; 
        gSocketManager.CloseSocket(nSD);
        postLogoff(nSD, NULL);
        icqRegister(passwd);
      }
      else
      {
        Licq::Event* e = DoneServerEvent(nSubSequence, Licq::Event::ResultError);
        if (e)
          delete e;
        gLog.Error(tr("%sUnknown logon error. There appears to be an issue with the ICQ servers. Please try again later.\n"), L_ERRORxSTR);
      }

      break;
    }
    case ICQ_SNACxNEW_UIN:
    {
      Licq::Event* e = DoneServerEvent(nSubSequence, Licq::Event::ResultSuccess);
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

      Licq::UserId oldOwnerId = Licq::gUserManager.ownerUserId(LICQ_PPID);
      if (!oldOwnerId.isValid())
      {
        gLog.Warn(tr("%sReceived new uin (%lu) when already have a uin (%s).\n"), L_WARNxSTR,
            nNewUin, oldOwnerId.toString().c_str());
        return;
      }

      gLog.Info(tr("%sReceived new uin: %lu\n"), L_SRVxSTR, nNewUin);
      char szUin[14];
      snprintf(szUin, sizeof(szUin), "%lu", nNewUin);
      Licq::UserId ownerId(szUin, LICQ_PPID);
      Licq::gUserManager.addOwner(ownerId);

      {
        Licq::OwnerWriteGuard o(LICQ_PPID);
        if (o.isLocked())
          o->setPassword(myRegisterPasswd);
      }
      myRegisterPasswd = "";
      gDaemon.SaveConf();

      gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalNewOwner,
          0, Licq::UserId(), LICQ_PPID));

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
      CPU_NewLogon* p;
      {
        Licq::OwnerReadGuard o(LICQ_PPID);
        p = new CPU_NewLogon(o->password().c_str(), o->accountId().c_str(), md5Salt);
      }
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
      string filename = Licq::gDaemon.baseDir() + "Licq_verify.jpg";
      FILE* fp = fopen(filename.c_str(), "w");
      if (fp == 0)
      {
        gLog.Warn(tr("%sUnable to open file (%s):\n%s%s.\n"), L_WARNxSTR,
            filename.c_str(), L_BLANKxSTR, strerror(errno));
        break;
      }
      
      fwrite(szJPEG, packet.getTLVLen(0x0002), 1, fp);
      fclose(fp);
      
      // Push a signal to the plugin to load the file
      gLog.Info("%sReceived verification image.\n", L_SRVxSTR);
      gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalVerifyImage,
          0, Licq::UserId(), LICQ_PPID));
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

void IcqProtocol::ProcessUserList()
{
  if (receivedUserList.empty())
    return;

  ContactUserListIter iter;

  for (iter = receivedUserList.begin(); iter != receivedUserList.end(); iter++)
  {
    Licq::UserId userId(iter->first, LICQ_PPID);
    CUserProperties* data = iter->second;

    if (!userId.isValid())
    {
      gLog.Warn(tr("%sEmpty User ID was received in the contact list.\n"),
          L_SRVxSTR);
      continue;
    }

    bool isOnList = Licq::gUserManager.userExists(userId);

    if (!isOnList)
    {
      Licq::gUserManager.addUser(userId, true, false, Licq::gUserManager.GetGroupFromID(data->groupId)); // Don't notify server
      gLog.Info(tr("%sAdded %s (%s) to list from server.\n"),
          L_SRVxSTR, (data->newAlias ? data->newAlias.get() : userId.toString().c_str()), userId.toString().c_str());
    }

    Licq::UserWriteGuard u(userId);
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
      u->addToGroup(Licq::gUserManager.GetGroupFromID(data->groupId));

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
    gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
        Licq::PluginSignal::UserBasic, u->id()));
    gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
        Licq::PluginSignal::UserGroups, u->id()));
    gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
        Licq::PluginSignal::UserSettings, u->id()));
  }

  receivedUserList.clear();
}

//--------ProcessDataChannel---------------------------------------------------

void IcqProtocol::ProcessDataChannel(CBuffer &packet)
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

bool IcqProtocol::ProcessCloseChannel(CBuffer &packet)
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
      gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalLogoff,
          Licq::PluginSignal::LogoffRate, Licq::gUserManager.ownerUserId(LICQ_PPID)));
      break;

  case 0x04:
  case 0x05:
    gLog.Error(tr("%sInvalid UIN and password combination.\n"), L_ERRORxSTR);
      gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalLogoff,
          Licq::PluginSignal::LogoffPassword, Licq::gUserManager.ownerUserId(LICQ_PPID)));
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

void IcqProtocol::addToModifyUsers(unsigned long unique_id, const std::string data)
{
  pthread_mutex_lock(&mutex_modifyserverusers);
  m_lszModifyServerUsers[unique_id] = data;
  pthread_mutex_unlock(&mutex_modifyserverusers);
}

int IcqProtocol::requestReverseConnection(const Licq::UserId& userId,
                                         unsigned long nData,
                                         unsigned long nLocalIP,
                                         unsigned short nLocalPort,
                                         unsigned short nRemotePort)
{
  if (Licq::gUserManager.isOwner(userId))
    return -1;

  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return -1;


  CPU_ReverseConnect *p = new CPU_ReverseConnect(*u, nLocalIP, nLocalPort,
                                                 nRemotePort);
  int nId = p->SubSequence();

  pthread_mutex_lock(&mutex_reverseconnect);

  m_lReverseConnect.push_back(new CReverseConnectToUserData(userId.accountId().c_str(), nId, nData,
      nLocalIP, nLocalPort, ICQ_VERSION_TCP, nRemotePort, 0, nId));
  pthread_mutex_unlock(&mutex_reverseconnect);

  gLog.Info("%sRequesting reverse connection from %s.\n", L_TCPxSTR,
            u->GetAlias());
  SendEvent_Server(p);

  return nId;
}
 
