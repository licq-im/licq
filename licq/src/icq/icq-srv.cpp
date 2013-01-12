/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2013 Licq developers <licq-dev@googlegroups.com>
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

#include <licq/byteorder.h>
#include <licq/contactlist/group.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/icq/chat.h>
#include <licq/icq/filetransfer.h>
#include <licq/oneventmanager.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/protocolsignal.h>
#include <licq/proxy.h>
#include <licq/statistics.h>
#include <licq/translator.h>
#include <licq/userevents.h>
#include <licq/logging/log.h>
#include <licq/version.h>

#include "../gettext.h"
#include "icqprotocolplugin.h"
#include "oscarservice.h"
#include "owner.h"
#include "packet-srv.h"
#include "socket.h"
#include "user.h"

using namespace std;
using namespace LicqIcq;
using Licq::Daemon;
using Licq::IcqPluginActive;
using Licq::IcqPluginBusy;
using Licq::IcqPluginInactive;
using Licq::Log;
using Licq::OnEventData;
using Licq::StringList;
using Licq::UserCategoryMap;
using Licq::gDaemon;
using Licq::gLog;
using Licq::gPluginManager;
using Licq::gOnEventManager;
using Licq::gTranslator;

//-----icqAddUser----------------------------------------------------------
void IcqProtocol::icqAddUser(const Licq::UserId& userId, bool _bAuthRequired)
{
  CSrvPacketTcp* p = new CPU_GenericUinList(userId.accountId(), ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
  gLog.info(tr("Alerting server to new user (#%hu)..."), p->Sequence());
  SendExpectEvent_Server(userId, p, NULL);

  int groupId;
  {
    UserReadGuard u(userId);
    groupId = u->serverGroup();
  }

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

  // Create group if it doesn't already exist in server list
  bool serverGroupExists = false;
  string groupName;
  {
    Licq::GroupReadGuard group(groupId);
    if (group.isLocked())
    {
      serverGroupExists = (group->serverId(LICQ_PPID) != 0);
      groupName = group->name();
    }
  }
  if (!serverGroupExists)
  {
    CPU_AddToServerList* pAdd = new CPU_AddToServerList(groupName, ICQ_ROSTxGROUP);
    int nGSID = pAdd->GetGSID();
    gLog.info(tr("Adding group %s (%d) to server list ..."), groupName.c_str(), nGSID);
    addToModifyUsers(pAdd->SubSequence(), groupName);
    SendExpectEvent_Server(pAdd, NULL);
  }

  CPU_AddToServerList* pAdd = new CPU_AddToServerList(userId.accountId(), ICQ_ROSTxNORMAL,
    groupId, _bAuthRequired);
  gLog.info(tr("Adding %s to server list..."), userId.accountId().c_str());
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
      UserReadGuard u(dynamic_cast<const User*>(user));
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
      UserReadGuard pUser(dynamic_cast<const User*>(user));
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
  gLog.info(tr("Exporting users to server contact list..."));
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
  gLog.info(tr("Updating top level group."));
  SendExpectEvent_Server(pReply, NULL);

  Licq::GroupListGuard groupList(false);
  BOOST_FOREACH(const Licq::Group* group, **groupList)
  {
    Licq::GroupReadGuard pGroup(group);
    unsigned int gid = pGroup->serverId(LICQ_PPID);
    if (gid != 0)
    {
      pReply = new CPU_UpdateToServerList(pGroup->name(), ICQ_ROSTxGROUP, gid);
      gLog.info(tr("Updating group %s."), pGroup->name().c_str());
      addToModifyUsers(pReply->SubSequence(), "");
      SendExpectEvent_Server(pReply, NULL);
    }
  }
}

void IcqProtocol::icqChangeGroup(const Licq::UserId& userId)
{
  if (!UseServerContactList())
    return;

  // Get their old SID
  int nSID;
  int oldGSID;
  int newGroup;
  {
    UserReadGuard u(userId);
    nSID = u->GetSID();
    oldGSID = u->GetGSID();
    newGroup = u->serverGroup();
    gLog.info(tr("Changing group on server list for %s (%s)..."),
        u->getAlias().c_str(), userId.accountId().c_str());
  }

  // Start transaction
  CSrvPacketTcp* pStart =
    new CPU_GenericFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxEDITxSTART);
  SendEvent_Server(pStart);

  // Delete the user
  if (oldGSID != 0)
  {
    // Don't attempt removing users from the root group, they can't be there
    CSrvPacketTcp* pRemove =
        new CPU_RemoveFromServerList(userId.accountId(), oldGSID, nSID, ICQ_ROSTxNORMAL);
    addToModifyUsers(pRemove->SubSequence(), userId.accountId());
    SendExpectEvent_Server(pRemove, NULL);
  }

  // Add the user, with the new group
  CPU_AddToServerList* pAdd = new CPU_AddToServerList(userId.accountId(), ICQ_ROSTxNORMAL, newGroup);
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
  gLog.info(tr("Exporting groups to server contact list..."));
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
  gLog.info(tr("Adding privacy information to server side list..."));
  SendEvent_Server(pPDInfo);
}

void IcqProtocol::icqRemoveUser(const Licq::UserId& userId, bool ignored)
{
  {
    UserReadGuard u(userId);
    if (u->NotInList())
      return;
  }

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
      UserWriteGuard u(userId);
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
      u->save(Licq::User::SaveLicqInfo);
    }

    CSrvPacketTcp *pRemove = new CPU_RemoveFromServerList(userId.accountId(),
        nGSID, nSID, (bIgnored ? ICQ_ROSTxIGNORE : ICQ_ROSTxNORMAL));
    addToModifyUsers(pRemove->SubSequence(), userId.accountId());
    SendExpectEvent_Server(pRemove, NULL);

    if (nVisibleSID)
    {
      CSrvPacketTcp* pVisRemove = new CPU_RemoveFromServerList(userId.accountId(), 0,
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
      CSrvPacketTcp* pInvisRemove = new CPU_RemoveFromServerList(userId.accountId(), 0,
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
  CSrvPacketTcp* p = new CPU_GenericUinList(userId.accountId(), ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_REMOVExFROMxLIST);
  gLog.info(tr("Alerting server to remove user (#%hu)..."), p->Sequence());
  SendExpectEvent_Server(userId, p, NULL);
}

//-----icqRemoveGroup----------------------------------------------------------
void IcqProtocol::icqRemoveGroup(const Licq::ProtoRemoveGroupSignal* ps)
{
  if (!UseServerContactList()) return;

  CSrvPacketTcp *pStart = new CPU_GenericFamily(ICQ_SNACxFAM_LIST,
    ICQ_SNACxLIST_ROSTxEDITxSTART);
  SendEvent_Server(pStart);

  CSrvPacketTcp* pRemove = new CPU_RemoveFromServerList(ps->groupName(),
      ps->groupServerId(), 0, ICQ_ROSTxGROUP);
  gLog.info(tr("Removing group from server side list (%s)..."), ps->groupName().c_str());
  addToModifyUsers(pRemove->SubSequence(), ps->groupName());
  SendExpectEvent_Server(pRemove, NULL);
}

//-----icqRenameGroup----------------------------------------------------------
void IcqProtocol::icqRenameGroup(const Licq::ProtoRenameGroupSignal* ps)
{
  if (!UseServerContactList() || m_nTCPSrvSocketDesc == -1)
    return;

  string newName;
  unsigned short serverId;
  {
    Licq::GroupReadGuard group(ps->groupId());
    if (!group.isLocked())
      return;
    newName = group->name();
    serverId = group->serverId(LICQ_PPID);
  }

  if (serverId == 0)
    return;

  CSrvPacketTcp* pUpdate = new CPU_UpdateToServerList(newName, ICQ_ROSTxGROUP, serverId);
  gLog.info(tr("Renaming group with id %d to %s..."), serverId, newName.c_str());
  addToModifyUsers(pUpdate->SubSequence(), newName);
  SendExpectEvent_Server(pUpdate, NULL);
}

void IcqProtocol::icqRenameUser(const Licq::UserId& userId)
{
  if (!UseServerContactList() || m_nTCPSrvSocketDesc == -1) return;

  string newAlias;
  {
    UserReadGuard u(userId);
    newAlias = u->getAlias();
  }

  CSrvPacketTcp* pUpdate = new CPU_UpdateToServerList(userId.accountId(), ICQ_ROSTxNORMAL);
  gLog.info(tr("Renaming %s to %s..."), userId.accountId().c_str(), newAlias.c_str());
  addToModifyUsers(pUpdate->SubSequence(), userId.accountId());
  SendExpectEvent_Server(pUpdate, NULL);
}

void IcqProtocol::icqAlertUser(const Licq::UserId& userId)
{
  if (userId.protocolId() != LICQ_PPID)
    return;

  string s;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    s = o->getAlias() + '\xFE' + o->getFirstName() + '\xFE' + o->getLastName() +
        '\xFE' + o->getEmail() + '\xFE' + (o->GetAuthorization() ? '0' : '1') + '\xFE';
  }
  CPU_ThroughServer* p = new CPU_ThroughServer(userId.accountId(), ICQ_CMDxSUB_ADDEDxTOxLIST, s);
  gLog.info(tr("Alerting user they were added (#%hu)..."), p->Sequence());
  SendExpectEvent_Server(userId, p, NULL);
}

unsigned long IcqProtocol::icqFetchAutoResponseServer(const Licq::UserId& userId)
{
  CPU_CommonFamily *p = 0;

  if (isalpha(userId.accountId()[0]))
    p = new CPU_AIMFetchAwayMessage(userId.accountId());
  else
  {
    int nCmd;
    {
      Licq::UserReadGuard u(userId);
      if (!u.isLocked())
        return 0;

      unsigned status = u->status();
      if (status & Licq::User::DoNotDisturbStatus)
        nCmd = ICQ_CMDxTCP_READxDNDxMSG;
      else if (status & Licq::User::OccupiedStatus)
        nCmd = ICQ_CMDxTCP_READxOCCUPIEDxMSG;
      else if (status & Licq::User::NotAvailableStatus)
        nCmd = ICQ_CMDxTCP_READxNAxMSG;
      else if (status & Licq::User::AwayStatus)
        nCmd = ICQ_CMDxTCP_READxAWAYxMSG;
      else if (status & Licq::User::FreeForChatStatus)
        nCmd = ICQ_CMDxTCP_READxFFCxMSG;
      else
        nCmd = ICQ_CMDxTCP_READxAWAYxMSG;
    }

    p = new CPU_ThroughServer(userId.accountId(), nCmd, string());
  }

  if (p == NULL)
    return 0;

  gLog.info(tr("Requesting auto response from %s (%hu)."),
      userId.toString().c_str(), p->Sequence());

  Licq::Event* e = SendExpectEvent_Server(userId, p, NULL);
  return (e != NULL ? e->EventId() : 0);
}

//-----icqSetRandomChatGroup----------------------------------------------------
unsigned long IcqProtocol::setRandomChatGroup(unsigned chatGroup)
{
  CPU_SetRandomChatGroup *p = new CPU_SetRandomChatGroup(chatGroup);
  gLog.info(tr("Setting random chat group (#%hu)..."), p->Sequence());

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqRandomChatSearch------------------------------------------------------
unsigned long IcqProtocol::randomChatSearch(unsigned chatGroup)
{
  CPU_RandomChatSearch *p = new CPU_RandomChatSearch(chatGroup);
  gLog.info(tr("Searching for random chat user (#%hu)..."), p->Sequence());

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
//  gLog.info(tr("Registering a new user (#%hu)..."), p->Sequence());
// SendEvent_Server(p);
  ConnectToLoginServer();
}

//-----ICQ::icqRegisterFinish------------------------------------------------
void IcqProtocol::icqRegisterFinish()
{
  CPU_RegisterFirst *pFirst = new CPU_RegisterFirst();
  SendEvent_Server(pFirst);

  CPU_Register* p = new CPU_Register(myRegisterPasswd);
  gLog.info(tr("Registering a new user..."));
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
  gLog.info(tr("Requesting verification image..."));
  SendEvent_Server(pVerify);
  
  m_nRegisterThreadId = pthread_self();
}

//-----ICQ::icqVerify--------------------------------------------------------
void IcqProtocol::icqVerify(const string& verification)
{
  CPU_SendVerification* p = new CPU_SendVerification(myRegisterPasswd, verification);
  gLog.info(tr("Sending verification for registration."));

   SendExpectEvent_Server(p, NULL);
}

//-----ICQ::icqRelogon-------------------------------------------------------
void IcqProtocol::icqRelogon()
{
  unsigned long status;

  if (m_eStatus == STATUS_ONLINE)
  {
    OwnerReadGuard o;
    status = addStatusFlags(icqStatusFromStatus(o->status()), *o);
  }
  else
  {
    status = m_nDesiredStatus;
  }

  icqLogoff();
  m_eStatus = STATUS_OFFLINE_MANUAL;

  m_nDesiredStatus = status;
  icqLogon();

//  m_eStatus = STATUS_OFFLINE_FORCED;
}

//-----icqRequestMetaInfo----------------------------------------------------
void IcqProtocol::icqRequestMetaInfo(const Licq::UserId& userId, const Licq::ProtocolSignal* ps)
{
  CPU_CommonFamily *p = 0;
  bool bIsAIM = isalpha(userId.accountId()[0]);
  if (bIsAIM)
    p = new CPU_RequestInfo(userId.accountId());
  else
    p = new CPU_Meta_RequestAllInfo(userId.accountId());
  gLog.info(tr("Requesting meta info for %s (#%hu/#%d)..."),
      userId.toString().c_str(), p->Sequence(), p->SubSequence());
  SendExpectEvent_Server(ps, userId, p, NULL, !bIsAIM);
}

//-----icqRequestService--------------------------------------------------------
void IcqProtocol::icqRequestService(unsigned short nFam)
{
  CPU_CommonFamily *p = new CPU_RequestService(nFam);
  gLog.info(tr("Requesting service socket for FAM 0x%02X (#%hu/#%d)..."),
      nFam, p->Sequence(), p->SubSequence());
  SendEvent_Server(p);
}

unsigned long IcqProtocol::setStatus(unsigned newStatus)
{
  return icqSetStatus(icqStatusFromStatus(newStatus));
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
  unsigned pfm;
  bool Invisible;
  bool goInvisible;
  bool isLogon;
  int nPDINFO;
  {
    OwnerReadGuard o;
    s = addStatusFlags(newStatus, *o);
    pfm = o->phoneFollowMeStatus();
    Invisible = o->isInvisible();
    goInvisible = (newStatus & ICQ_STATUS_FxPRIVATE);
    isLogon = !o->isOnline();
    nPDINFO = o->GetPDINFO();
  }

  if (nPDINFO == 0)
  {
    icqCreatePDINFO();

    OwnerReadGuard o;
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

  gLog.info(tr("Changing status to %s (#%hu)..."),
      Licq::User::statusToString(statusFromIcqStatus(newStatus)).c_str(), p->Sequence());
  m_nDesiredStatus = s;

  SendEvent_Server(p);

  if (pfm != IcqPluginInactive)
  {
    p = new CPU_UpdateStatusTimestamp(PLUGIN_FOLLOWxME,
        (pfm == IcqPluginBusy ? ICQ_PLUGIN_STATUSxBUSY : ICQ_PLUGIN_STATUSxACTIVE), s);
    SendEvent_Server(p);
  }

  return 0;
}

//-----icqSetPassword--------------------------------------------------------
unsigned long IcqProtocol::icqSetPassword(const string& password)
{
  CPU_SetPassword* p = new CPU_SetPassword(password);
  gLog.info(tr("Updating password (#%hu/#%d)..."), p->Sequence(), p->SubSequence());
  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

void IcqProtocol::icqSetGeneralInfo(const Licq::ProtocolSignal* ps)
{
  string alias, firstName, lastName, emailPrimary, address, city, state, zipCode;
  string phoneNumber, faxNumber, cellularNumber;
  unsigned short countryCode;
  bool hideEmail;

  {
    OwnerReadGuard owner(ps->userId());
    if (!owner.isLocked())
      return;

    alias = owner->getAlias();
    firstName = owner->getFirstName();
    lastName = owner->getLastName();
    emailPrimary = owner->getUserInfoString("Email1");
    address = owner->getUserInfoString("Address");
    city = owner->getUserInfoString("City");
    state = owner->getUserInfoString("State");
    zipCode = owner->getUserInfoString("Zipcode");
    phoneNumber = owner->getUserInfoString("PhoneNumber");
    faxNumber = owner->getUserInfoString("FaxNumber");
    cellularNumber = owner->getUserInfoString("CellularNumber");
    countryCode = owner->getUserInfoUint("Country");
    hideEmail = owner->getUserInfoBool("HideEmail");
  }

  CPU_Meta_SetGeneralInfo* p = new CPU_Meta_SetGeneralInfo(alias, firstName,
      lastName, emailPrimary, city, state, phoneNumber,
      faxNumber, address, cellularNumber, zipCode, countryCode, hideEmail);

  gLog.info(tr("Updating general info (#%hu/#%d)..."), p->Sequence(), p->SubSequence());

  SendExpectEvent_Server(ps, p, NULL);
}

//-----icqSetEmailInfo---------------------------------------------------------
unsigned long IcqProtocol::icqSetEmailInfo(const std::string& emailSecondary, const std::string& emailOld)
{
return 0;
  CPU_Meta_SetEmailInfo* p = new CPU_Meta_SetEmailInfo(emailSecondary, emailOld);

  gLog.info(tr("Updating additional E-Mail info (#%hu/#%d)..."), p->Sequence(), p->SubSequence());

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
  CPU_Meta_SetMoreInfo* p = new CPU_Meta_SetMoreInfo(age, gender, homepage,
      birthYear, birthMonth, birthDay, language1, language2, language3);

  gLog.info(tr("Updating more info (#%hu/#%d)..."), p->Sequence(), p->SubSequence());

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetInterestsInfo------------------------------------------------------
unsigned long IcqProtocol::icqSetInterestsInfo(const UserCategoryMap& interests)
{
  CPU_Meta_SetInterestsInfo *p = new CPU_Meta_SetInterestsInfo(interests);
  gLog.info(tr("Updating Interests info (#%hu/#%d).."), p->Sequence(), p->SubSequence());

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
  gLog.info(tr("Updating Organizations/Backgrounds info (#%hu/#%d).."),
      p->Sequence(), p->SubSequence());

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
  CPU_Meta_SetWorkInfo* p = new CPU_Meta_SetWorkInfo(city, state, phone, fax, address,
      zip, companyCountry, name, department, position, companyOccupation, homepage);

  gLog.info(tr("Updating work info (#%hu/#%d)..."), p->Sequence(), p->SubSequence());

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

//-----icqSetAbout-----------------------------------------------------------
unsigned long IcqProtocol::icqSetAbout(const string& about)
{
  CPU_Meta_SetAbout *p = new CPU_Meta_SetAbout(gTranslator.returnToDos(about));

  gLog.info(tr("Updating about (#%hu/#%d)..."), p->Sequence(), p->SubSequence());

  Licq::Event* e = SendExpectEvent_Server(p, NULL);
  if (e != NULL)
    return e->EventId();
  return 0;
}

void IcqProtocol::icqAuthorizeGrant(const Licq::ProtocolSignal* ps)
{
  CPU_Authorize* p = new CPU_Authorize(ps->userId().accountId());
  gLog.info(tr("Authorizing user %s."), ps->userId().accountId().c_str());
  SendEvent_Server(p, ps);
}

void IcqProtocol::icqAuthorizeRefuse(const Licq::ProtoRefuseAuthSignal* ps)
{
  string userEncoding = getUserEncoding(ps->userId());
  CPU_ThroughServer* p = new CPU_ThroughServer(ps->userId().accountId(), ICQ_CMDxSUB_AUTHxREFUSED,
      gTranslator.returnToDos(gTranslator.fromUtf8(ps->message())));
  gLog.info(tr("Refusing authorization to user %s (#%hu)..."),
      ps->userId().accountId().c_str(), p->Sequence());

  SendExpectEvent_Server(ps, p, NULL);
}

void IcqProtocol::icqRequestAuth(const Licq::UserId& userId, const string& message)
{
  string userEncoding = getUserEncoding(userId);
  CSrvPacketTcp* p = new CPU_RequestAuth(userId.accountId(),
      gTranslator.fromUtf8(message, userEncoding));
  SendEvent_Server(p);
}

//-----icqSetSecurityInfo----------------------------------------------------
unsigned long IcqProtocol::icqSetSecurityInfo(bool bAuthorize, bool bWebAware)
{
  // Since ICQ5.1, the status change packet is sent first, which means it is
  // assumed that the set security info packet works.
  unsigned short s;
  {
    OwnerWriteGuard o;
    o->SetEnableSave(false);
    o->SetAuthorization(bAuthorize);
    o->SetWebAware(bWebAware);
    o->SetEnableSave(true);
    o->save(Licq::User::SaveLicqInfo);
    s = addStatusFlags(icqStatusFromStatus(o->status()), *o);
  }
  // Set status to ensure the status flags are set
  icqSetStatus(s);

  // Now send the set security info packet
    CPU_Meta_SetSecurityInfo *p = new CPU_Meta_SetSecurityInfo(bAuthorize, bWebAware);
  gLog.info(tr("Updating security info (#%hu/#%d)..."), p->Sequence(), p->SubSequence());
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
  CPU_SearchWhitePages* p = new CPU_SearchWhitePages(firstName, lastName,
      alias, email, minAge, maxAge, gender, language, city, state,
      countryCode, coName, coDept, coPos, keyword, onlineOnly);
  gLog.info(tr("Starting white pages search (#%hu/#%d)..."),
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
  gLog.info(tr("Starting search by UIN for user (#%hu/#%d)..."), p->Sequence(), p->SubSequence());
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
    p = new CPU_RequestInfo(userId.accountId());
  else
    p = new CPU_Meta_RequestAllInfo(userId.accountId());
  gLog.info(tr("Requesting user info (#%hu/#%d)..."), p->Sequence(), p->SubSequence());
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
void IcqProtocol::icqUpdateInfoTimestamp(const uint8_t* GUID)
{
  CPU_UpdateInfoTimestamp *p = new CPU_UpdateInfoTimestamp(GUID);
  SendEvent_Server(p);
}

//-----icqUpdatePhoneBookTimestamp----------------------------------------------
void IcqProtocol::icqUpdatePhoneBookTimestamp()
{
  bool bOffline;
  {
    OwnerWriteGuard o;
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
    OwnerWriteGuard o;
    o->SetClientInfoTimestamp(time(NULL));
    bOffline = !o->isOnline();
  }

  if (!bOffline)
    icqUpdateInfoTimestamp(PLUGIN_PICTURE);
}

//-----icqSetPhoneFollowMeStatus------------------------------------------------
void IcqProtocol::icqSetPhoneFollowMeStatus(unsigned newStatus)
{
  bool bOffline;
  {
    OwnerWriteGuard o;
    o->SetClientStatusTimestamp(time(NULL));
    o->setPhoneFollowMeStatus(newStatus);
    bOffline = !o->isOnline();
  }

  if (!bOffline)
  {
    unsigned long nNewStatus;
    switch (newStatus)
    {
      case IcqPluginActive: nNewStatus = ICQ_PLUGIN_STATUSxACTIVE; break;
      case IcqPluginBusy: nNewStatus = ICQ_PLUGIN_STATUSxBUSY; break;
      default: nNewStatus = ICQ_PLUGIN_STATUSxINACTIVE; break;
    }
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

  {
    Licq::UserListGuard userList(LICQ_PPID);
    BOOST_FOREACH(Licq::User* user, **userList)
    {
      Licq::UserWriteGuard u(user);
      n++;
      users.push_back(u->accountId());
      if (n == myMaxUsersPerPacket)
      {
        CSrvPacketTcp *p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
        gLog.info(tr("Updating contact list (#%hu)..."), p->Sequence());
        SendEvent_Server(p);
        users.clear();
        n = 0;
      }
      // Reset all users to offline
      if (u->isOnline())
        u->statusChanged(User::OfflineStatus);
    }
  }
  if (n != 0)
  {
    CSrvPacketTcp *p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
    gLog.info(tr("Updating contact list (#%hu)..."), p->Sequence());
    SendEvent_Server(p);
  }
}
//-----icqTypingNotification---------------------------------------------------
void IcqProtocol::icqTypingNotification(const Licq::UserId& userId, bool _bActive)
{
  if (gDaemon.sendTypingNotification())
  {
    CSrvPacketTcp* p = new CPU_TypingNotification(userId.accountId(), _bActive);
    SendEvent_Server(p);
  }
}

//-----icqCheckInvisible--------------------------------------------------------
void IcqProtocol::icqCheckInvisible(const Licq::UserId& userId)
{
  CSrvPacketTcp* p = new CPU_CheckInvisible(userId.accountId());
  SendExpectEvent_Server(userId, p, NULL);
}

//-----icqSendVisibleList-------------------------------------------------------
void IcqProtocol::icqSendVisibleList()
{
  // send user info packet
  // Go through the entire list of users, checking if each one is on
  // the visible list
  StringList users;
  {
    Licq::UserListGuard userList(LICQ_PPID);
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      Licq::UserReadGuard u(user);
      if (u->VisibleList())
        users.push_back(u->accountId());
    }
  }
  CSrvPacketTcp* p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxVISIBLExLIST);
  gLog.info(tr("Sending visible list (#%hu)..."), p->Sequence());
  SendEvent_Server(p);
}


//-----icqSendInvisibleList-----------------------------------------------------
void IcqProtocol::icqSendInvisibleList()
{
  StringList users;
  {
    Licq::UserListGuard userList(LICQ_PPID);
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      Licq::UserReadGuard u(user);
      if (u->InvisibleList())
        users.push_back(u->accountId());
    }
  }

  CSrvPacketTcp* p = new CPU_GenericUinList(users, ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxINVISIBxLIST);
  gLog.info(tr("Sending invisible list (#%hu)..."), p->Sequence());
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
  CSrvPacketTcp* p = new CPU_GenericUinList(userId.accountId(), ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxVISIBLExLIST);
  gLog.info(tr("Adding user %s to visible list (#%hu)..."), userId.accountId().c_str(),
     p->Sequence());
  SendEvent_Server(p);

  if (UseServerContactList())
  {
    CSrvPacketTcp* pAdd = new CPU_AddToServerList(userId.accountId(), ICQ_ROSTxVISIBLE);
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
  CSrvPacketTcp* p = new CPU_GenericUinList(userId.accountId(), ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_REMxVISIBLExLIST);
  gLog.info(tr("Removing user %s from visible list (#%hu)..."),
      userId.toString().c_str(), p->Sequence());
  SendEvent_Server(p);

  if (UseServerContactList())
  {
    UserReadGuard u(userId);
    if (u.isLocked())
    {
      CSrvPacketTcp* pRemove = new CPU_RemoveFromServerList(userId.accountId(), 0, u->GetVisibleSID(),
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
  CSrvPacketTcp* p = new CPU_GenericUinList(userId.accountId(), ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_ADDxINVISIBxLIST);
  gLog.info(tr("Adding user %s to invisible list (#%hu)..."), userId.toString().c_str(),
     p->Sequence());
  SendEvent_Server(p);

  if (UseServerContactList())
  {
    CSrvPacketTcp* pAdd = new CPU_AddToServerList(userId.accountId(), ICQ_ROSTxINVISIBLE);
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
  CSrvPacketTcp* p = new CPU_GenericUinList(userId.accountId(), ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_REMxINVISIBxLIST);
  gLog.info(tr("Removing user %s from invisible list (#%hu)..."), userId.toString().c_str(),
     p->Sequence());
  SendEvent_Server(p);

  if (UseServerContactList())
  {
    UserReadGuard u(userId);
    if (u.isLocked())
    {
      CSrvPacketTcp* pRemove = new CPU_RemoveFromServerList(userId.accountId(), 0, u->GetInvisibleSID(),
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
  CPU_AddToServerList* pAdd = new CPU_AddToServerList(userId.accountId(), ICQ_ROSTxIGNORE,
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
        gLog.info(tr("Deleting server list users (#%hu)..."), p->Sequence());
        SendEvent_Server(p);
        users.clear();
        n = 0;
      }
    }
  }

  if (n != 0)
  {
    CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxNORMAL);
    gLog.info(tr("Deleting server list users (#%hu)..."), p->Sequence());
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
        UserReadGuard pUser(dynamic_cast<const User*>(user));
        if (pUser->GetInvisibleSID())
        {
          n++;
          users.push_back(pUser->accountId());
        }
      }

      if (n == myMaxUsersPerPacket)
      {
        CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxINVISIBLE);
        gLog.info(tr("Deleting server list invisible list users (#%hu)..."), p->Sequence());
        SendEvent_Server(p);
        users.clear();
        n = 0;
      }
    }
  }

  if (n != 0)
  {
    CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxINVISIBLE);
    gLog.info(tr("Deleting server list invisible list users (#%hu)..."), p->Sequence());
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
        UserReadGuard pUser(dynamic_cast<const User*>(user));
        if (pUser->GetVisibleSID())
        {
          n++;
          users.push_back(pUser->accountId());
        }
      }

      if (n == myMaxUsersPerPacket)
      {
        CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxVISIBLE);
        gLog.info(tr("Deleting server list visible list users (#%hu)..."), p->Sequence());
        SendEvent_Server(p);
        users.clear();
        n = 0;
      }
    }
  }

  if (n != 0)
  {
    CSrvPacketTcp *p = new CPU_ClearServerList(users, ICQ_ROSTxVISIBLE);
    gLog.info(tr("Deleting server list visible list users (#%hu)..."), p->Sequence());
    SendEvent_Server(p);
  }
}

//-----icqSendThroughServer-----------------------------------------------------
Licq::Event* IcqProtocol::icqSendThroughServer(pthread_t caller, unsigned long eventId, const Licq::UserId& userId,
    unsigned char format, const string& message, Licq::UserEvent* ue, unsigned short nCharset)
{
  Licq::Event* result;
  bool bOffline = true;
  {
    Licq::UserReadGuard u(userId);
    if (u.isLocked())
      bOffline = !u->isOnline();
  }

  CPU_ThroughServer* p = new CPU_ThroughServer(userId.accountId(), format, message, nCharset, bOffline);

  switch (format)
  {
    case ICQ_CMDxSUB_MSG:
      gLog.info(tr("Sending message through server (#%hu)."), p->Sequence());
      break;
    case ICQ_CMDxSUB_URL:
      gLog.info(tr("Sending url through server (#%hu)."), p->Sequence());
      break;
    case ICQ_CMDxSUB_CONTACTxLIST:
      gLog.info(tr("Sending contact list through server (#%hu)."), p->Sequence());
      break;
    default:
      gLog.info(tr("Sending misc through server (#%hu)."), p->Sequence());
  }

  // If we are already shutting down, don't start any events
  if (gDaemon.shuttingDown())
    return NULL;

  Licq::Event* e = new Licq::Event(caller, eventId, m_nTCPSrvSocketDesc, p, Licq::Event::ConnectServer, userId, ue);
  e->myCommand = eventCommandFromPacket(p);
  e->m_NoAck = true;

  result = SendExpectEvent(e, &ProcessRunningEvent_Server_tep);
  return result;
}

unsigned long IcqProtocol::icqSendSms(const Licq::UserId& userId,
    const string& number, const string& message)
{
  Licq::EventSms* ue = new Licq::EventSms(number, message,
      Licq::EventSms::TimeNow, LICQ_VERSION | Licq::EventSms::FlagSender);
  CPU_SendSms* p = new CPU_SendSms(number, message);
  gLog.info(tr("Sending SMS through server (#%hu/#%d)..."), p->Sequence(), p->SubSequence());
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
      e->subResult() != Licq::Event::SubResultReturn)
  {
    Licq::UserWriteGuard u(e->userId());
    if (u.isLocked())
    {
      e->m_pUserEvent->AddToHistory(*u, false);
      u->SetLastSentEvent();
      gOnEventManager.performOnEvent(OnEventData::OnEventMsgSent, *u);
    }
    Licq::gStatistics.increase(Licq::Statistics::EventsSentCounter);
  }

  CSrvPacketTcp* srvPacket = dynamic_cast<CSrvPacketTcp*>(e->m_pPacket);

  // Process the event, first by channel
  if (srvPacket == NULL)
  {
    if (e->flags() & Licq::Event::FlagDirect)
      gPluginManager.pushPluginEvent(e);
    else
      delete e;
  }
  else if (srvPacket->icqChannel() == ICQ_CHNxNEW || srvPacket->icqChannel() == ICQ_CHNxPING)
  {
    delete e;
  }
  else if (srvPacket->icqChannel() == ICQ_CHNxDATA)
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
        gPluginManager.pushPluginEvent(e);
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
            gPluginManager.pushPluginEvent(e);
            break;

          case Licq::Event::ResultAcked:
            PushExtendedEvent(e);
            break;

          default:
            gLog.error(tr("Internal error: ProcessDoneEvents(): Invalid result "
                "for extended event (%d)."), e->m_eResult);
            delete e;
            return;
        }

        break;
      }

      default:
      {
        gLog.warning(tr("Internal error: ProcessDoneEvents(): Unknown SNAC (0x%08lX)."),
            e->m_nSNAC);
        delete e;
        return;
      }
    } // switch
  }
  else
  {
  }
}

unsigned long IcqProtocol::logon(unsigned logonStatus)
{
  if (m_bLoggingOn)
  {
    gLog.warning(tr("Attempt to logon while already logged or logging on, logoff and try again."));
    return 0;
  }
  {
    OwnerReadGuard o;
    if (!o.isLocked())
    {
      gLog.error(tr("No registered user, unable to process logon attempt."));
      return 0;
    }
    if (o->password().empty())
    {
      gLog.error(tr("No password set.  Edit ~/.licq/owner.Licq and fill in the password field."));
      return 0;
    }

    m_nDesiredStatus = addStatusFlags(icqStatusFromStatus(logonStatus), *o);
  }

  return icqLogon();
}

unsigned long IcqProtocol::icqLogon()
{
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
    gLog.info(tr("Requesting logon salt (#%hu)..."), p->Sequence());
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
    return;

  m_bLoggingOn = false;

  gLog.info(tr("Logging off."));
  Licq::Event* cancelledEvent = NULL;

  if (nSD != -1)
  {
    CPU_Logoff p;
    cancelledEvent = new Licq::Event(nSD, &p, Licq::Event::ConnectServer);
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
    gLog.info("Event #%hu is still on the server queue!\n", e->Sequence());
    iter = m_lxSendQueue_Server.erase(iter);
    Licq::Event* cancelled = new Licq::Event(e);
    cancelled->m_bCancelled = true;
    m_lxSendQueue_Server.push_back(cancelled);
  }

  iter = m_lxRunningEvents.begin();
  while (iter != m_lxRunningEvents.end())
  {
    CSrvPacketTcp* srvPacket = dynamic_cast<CSrvPacketTcp*>((*iter)->m_pPacket);
    if ((*iter)->m_nSocketDesc == nSD || (srvPacket != NULL && srvPacket->icqChannel() == ICQ_CHNxNEW))
    {
      Licq::Event* e = *iter;
      gLog.info("Event #%hu is still on the running queue!\n", e->Sequence());
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
    gLog.info(tr("Event #%hu is still on queue!\n"), (*iter)->Sequence());

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

  {
    Licq::OwnerWriteGuard o(LICQ_PPID);
    if (o.isLocked())
      o->statusChanged(Licq::User::OfflineStatus);
  }

  myRegisterPasswd = "";

  gPluginManager.pushPluginSignal(new Licq::PluginSignal(
      Licq::PluginSignal::SignalLogoff,
      Licq::PluginSignal::LogoffRequested, Licq::gUserManager.ownerUserId(LICQ_PPID)));

  // Mark all users as offline, this also updates the last seen
  // online field
  Licq::UserListGuard userList(LICQ_PPID);
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

  // Try and get server address from owner, but use default in case we're registering
  string serverHost;
  int serverPort = 0;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    if (o.isLocked())
    {
      serverHost = o->serverHost();
      serverPort = o->serverPort();
    }
  }

  if (serverHost.empty())
    serverHost = gIcqProtocolPlugin->defaultServerHost();
  if (serverPort <= 0)
    serverPort = gIcqProtocolPlugin->defaultServerPort();

  // Which protocol plugin?
  int r = ConnectToServer(serverHost, serverPort);

  myNewSocketPipe.putChar('S');

  return r;
}

int IcqProtocol::ConnectToServer(const string& server, unsigned short port)
{
  SrvSocket* s = new SrvSocket(Licq::gUserManager.ownerUserId(LICQ_PPID));

  if (gDaemon.proxyEnabled())
  {
    if (m_xProxy == NULL)
    {
      gLog.warning(tr("Proxy server not properly configured."));
      delete s;
      return (-1);
    }
  }
  else if (m_xProxy != NULL)
  {
    delete m_xProxy;
    m_xProxy = NULL;
  }

  if (!s->connectTo(server, port, m_xProxy))
  {
    delete s;
    return -1;
  }

  static pthread_mutex_t connect_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&connect_mutex);
  int nSocket;
  if (m_nTCPSrvSocketDesc != -1)
  {
    gLog.warning(tr("Connection to server already exists, aborting."));
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

string IcqProtocol::parseDigits(const string& number)
{
  string ret;

  for (size_t i = 0; i < number.size(); ++i)
    if (isdigit(number[i]))
      ret += number[i];
  return ret;
}

string IcqProtocol::findUserByCellular(const string& cellular)
{
  string parsedCellular = parseDigits(cellular);

  Licq::UserListGuard userList;
  BOOST_FOREACH(const Licq::User* user, **userList)
  {
    Licq::UserReadGuard u(user);

    if (parseDigits(u->getCellularNumber()) == parsedCellular)
      return u->accountId();
  }

  return "";
}

//-----ProcessSrvPacket---------------------------------------------------------
bool IcqProtocol::ProcessSrvPacket(Buffer& packet)
{
  unsigned short nLen;
  unsigned short  nSequence;
  char startCode, nChannel;

  // read in the standard server TCP header info
  packet >> startCode;

  if (startCode != 0x2a)
  {
    gLog.warning("ProcessSrvPacket bad start code: %d", startCode);
    packet.log(Log::Unknown, tr("Unknown server response"));
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
      gLog.warning(tr("Packet on unhandled Channel 'Error' received!"));
      break;

  case ICQ_CHNxCLOSE:
    return ProcessCloseChannel(packet);
    break;

    default:
      gLog.warning(tr("Server send unknown channel: %02x"), nChannel);
      break;
  }

  return true;
}

//--------ProcessServiceFam----------------------------------------------------

void IcqProtocol::ProcessServiceFam(Buffer& packet, unsigned short nSubtype)
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
      gLog.error(tr("Error parsing SNAC header"));
      return;
    }
  }

  switch (nSubtype)
  {
    case ICQ_SNACxSUB_READYxSERVER:
    {
      CSrvPacketTcp* p;

      gLog.info(tr("Server says he's ready."));

      gLog.info(tr("Sending our channel capability list..."));
      p = new CPU_ImICQ();
      SendEvent_Server(p);

      gLog.info(tr("Requesting rate info..."));
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
        gLog.warning(tr("Error during parsing service redirect packet!"));
        break;
      }
      if (packet.getTLVLen(0x000D) == 2)
        nFam = packet.UnpackUnsignedShortTLV(0x000D);

      gLog.info(tr("Redirect for service 0x%02X received."), nFam);

      string server = packet.unpackTlvString(0x0005);
      string cookie = packet.unpackTlvString(0x0006);
      if (server.empty() || cookie.empty())
      {
        gLog.warning(tr("Invalid servername (%s) or cookie (%s) in service redirect packet!"),
            server.c_str(), cookie.c_str());
        break;
      }
      unsigned short nCookieLen = packet.getTLVLen(0x0006);
      cookie.resize(nCookieLen);

      unsigned short nPort;
      size_t pos = server.find(':');
      if (pos != string::npos)
      {
        nPort = atoi(server.c_str() + pos + 1);
        server.resize(pos);
      }
      else
      {
        Licq::OwnerReadGuard o(LICQ_PPID);
        nPort = o->serverPort();
      }

      switch (nFam)
      {
        case ICQ_SNACxFAM_BART:
          if (m_xBARTService)
          {
            m_xBARTService->setConnectCredential(server, nPort, cookie);
            m_xBARTService->ChangeStatus(STATUS_SERVICE_REQ_ACKED);
            break;
          }
          else
          {
            gLog.warning(tr("Service redirect packet for unallocated BART service."));
            break;
          }

        default:
          gLog.warning(tr("Service redirect packet for unhandled service 0x%02X."), nFam);
      }
      break;
    }

    case ICQ_SNACxSRV_ACKxIMxICQ:
    {
      // unsigned long nListTime;
      // {
      //   Licq::OwnerReadGuard o(LICQ_PPID);
      //   nListTime = o->GetSSTime();
      // }

      CSrvPacketTcp* p;
      gLog.info(tr("Server sent us channel capability list (ignoring)."));

      gLog.info(tr("Requesting self info."));
      p = new CPU_GenericFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_GETxUSERxINFO);
      SendEvent_Server(p);

      gLog.info(tr("Requesting list rights."));
      p = new CPU_ListRequestRights();
      SendExpectEvent_Server(p, NULL);

      gLog.info(tr("Requesting roster rights."));
      p = new CPU_RequestList();
      SendEvent_Server(p);

      gLog.info(tr("Requesting location rights."));
      p = new CPU_GenericFamily(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxRIGHTS);
      SendEvent_Server(p);

      gLog.info(tr("Requesting contact list rights."));
      p = new CPU_GenericFamily(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_REQUESTxRIGHTS);
      SendEvent_Server(p);

      gLog.info(tr("Requesting Instant Messaging rights."));
      p = new CPU_GenericFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_REQUESTxRIGHTS);
      SendEvent_Server(p);

      gLog.info(tr("Requesting BOS rights."));
      p = new CPU_GenericFamily(ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_REQUESTxRIGHTS);
      SendEvent_Server(p);

      break;
    }

    case ICQ_SNACxSUB_RATE_INFO:
    {
      gLog.info(tr("Server sent us rate information."));
      CSrvPacketTcp *p = new CPU_RateAck();
      SendEvent_Server(p);

      gLog.info(tr("Setting ICQ Instant Messaging Mode."));
      p = new CPU_ICQMode(1, 11); // enable typing notifications
      SendEvent_Server(p);
      p = new CPU_ICQMode(2, 3); // set default flags for all channels
      SendEvent_Server(p);
      p = new CPU_ICQMode(4, 3); // set default flags for all channels
      SendEvent_Server(p);

      gLog.info(tr("Sending capability settings (?)"));
      p = new CPU_CapabilitySettings();
      SendEvent_Server(p);

      break;
    }

  case ICQ_SNACxRCV_NAMExINFO:
  {
    unsigned short evil, tlvBlocks;
      unsigned long realIP;
    time_t nOnlineSince = 0;

      gLog.info(tr("Got Name Info from Server"));

      string id = packet.unpackByteString();
    evil = packet.UnpackUnsignedShortBE();
    tlvBlocks = packet.UnpackUnsignedShortBE();

      gLog.info(tr("UIN: %s Evil: %04hx"), id.c_str(), evil);

    if (!packet.readTLV(tlvBlocks)) {
      packet.log(Log::Unknown, tr("Unknown server response"));
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
          OwnerWriteGuard o;
          o->SetIp(realIP);
        }

        char buf[32];
        gLog.info(tr("Server says we are at %s."), Licq::ip_ntoa(realIP, buf));
      //icqSetStatus(m_nDesiredStatus);
    }
    if (packet.getTLVLen(0x0003) == 4)
      nOnlineSince = packet.UnpackUnsignedLongTLV(0x0003);

      OwnerWriteGuard o;
      unsigned pfm = o->phoneFollowMeStatus();
    // Workaround for the ICQ4.0 problem of it not liking the PFM flags
    m_nDesiredStatus &= ~(ICQ_STATUS_FxPFM | ICQ_STATUS_FxPFMxAVAILABLE);
      if (pfm != IcqPluginInactive)
      m_nDesiredStatus |= ICQ_STATUS_FxPFM;
      if (pfm == IcqPluginActive)
      m_nDesiredStatus |= ICQ_STATUS_FxPFMxAVAILABLE;
      ChangeUserStatus(*o, m_nDesiredStatus, nOnlineSince);
      gLog.info(tr("Server says we're now: %s"), o->statusString().c_str());
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
    gLog.info(tr("Server is going to disconnect/pause. Lets reconnect to another one."));
    icqRelogon();
    break;
  }
    case ICQ_SNACxSRV_MOTD:
    {
      // MOTD from server, and some unknown flags
#if 0
      int type = packet.unpackUInt16BE();
      if (!packet.readTLV())
      {
        gLog.warning(tr("Error parsing MOTD packet!"));
        break;
      }
      if (packet.getTLVLen(0x0002) == 2)
        ; // Unknown
      if (packet.getTLVLen(0x0003) == 2)
        ; // Unknown
      if (packet.hasTLV(0x000B))
        string motd = packet.unpackTlvString(0x000B);
#endif
      break;
    }

    case ICQ_SNACxSRV_URLS:
    {
      // Server sends a bunch of ICQ URLs at logon, just ignore them
#if 0
      if (!packet.readTLV())
      {
        gLog.warning(tr("Error parsing URL packet!"));
        break;
      }
      // Each TLV contains an URL as a string for various ICQ web services
#endif
      break;
    }
    case ICQ_SNACxSRV_EXTSTATUS:
    {
      OwnerWriteGuard o;
      processIconHash(*o, packet);
      break;
    }

  default:
    gLog.warning(tr("Unknown Service Family Subtype: %04hx"), nSubtype);
    break;
  }
}

void IcqProtocol::processIconHash(User* u, Buffer& packet)
{
  while (packet.remainingDataToRead() >= 4)
  {
    int type = packet.unpackUInt16BE();
    int flags = packet.unpackUInt8();
    int length = packet.unpackUInt8();

    switch (type)
    {
      case BART_TYPExBUDDY_ICON_PHOTO:
      {
        // TODO: Handle photo item
        break;
      }
      case BART_TYPExBUDDY_ICON:
      case BART_TYPExBUDDY_ICON_SMALL:
      case BART_TYPExBUDDY_ICON_FLASH:
      {
        if (type == 1 && length > 0 && length <= 16)
        {
          string hash = packet.unpackRawString(length);
          u->setBuddyIconHash(hash);
          u->setBuddyIconType(type);
          u->setBuddyIconHashType(flags);
          u->save(Licq::User::SavePictureInfo);
          continue;
        }
        break;
      }
      case BART_TYPExSTATUS_STR:
      {
        // TODO: Handle status string
        break;
      }
      case BART_TYPExSTATUS_STR_TIMESTAMP:
      {
        // TODO: Handle status string timestamp
        break;
      }
      case BART_TYPExSTATUS_MOOD:
      {
        // TODO: Handle status mood item
        break;
      }
      case BART_TYPExITUNES_LINK:
      {
        // TODO: Handle iTunes music store link
        break;
      }
      default:
        gLog.warning(tr("Unknown Extended Status Data type 0x%04x flags 0x%02x length 0x%02x"),
            type, flags, length);
    }

    // Unhandled part, skip the data
    packet.incDataPosRead(length);
  }
}

//--------ProcessLocationFam-----------------------------------------------
void IcqProtocol::ProcessLocationFam(Buffer& packet, unsigned short nSubtype)
{
  /*unsigned short nFlags = */packet.UnpackUnsignedShortBE();
  unsigned long nSubSequence = packet.UnpackUnsignedLongBE();

  switch (nSubtype)
  {
    case ICQ_SNAXxLOC_RIGHTSxGRANTED:
      gLog.info(tr("Received rights for Location Services."));
      break;

  case ICQ_SNACxREPLYxUSERxINFO:
  {
      string id = packet.unpackByteString();
      Licq::UserId userId(id, LICQ_PPID);
    packet.UnpackUnsignedLongBE(); // Unknown
    
    if (!packet.readTLV())
    {
      gLog.error(tr("Error during parsing user information packet!"));
      break;
    }

      if (packet.hasTLV(0x0004))
      {
        string awayMsg = packet.unpackTlvString(0x0004);
        gLog.info(tr("Received away message for %s."), id.c_str());
        {
          Licq::UserWriteGuard u(userId);
          string awayMsgUtf8 = gTranslator.toUtf8(awayMsg, u->userEncoding());
          if (awayMsgUtf8 != u->autoResponse())
          {
            u->setAutoResponse(awayMsgUtf8);
            u->SetShowAwayMsg(!awayMsgUtf8.empty());
          }
        }

        Licq::Event* e = DoneServerEvent(nSubSequence, Licq::Event::ResultSuccess);
      if (e)
        ProcessDoneEvent(e);
    }

      if (packet.hasTLV(0x0002))
      {
        string info = packet.unpackTlvString(0x0002);
        gLog.info(tr("Received user information for %s."), id.c_str());
        {
          Licq::UserWriteGuard u(userId);
          u->SetEnableSave(false);
          u->setUserInfoString("About", gTranslator.toUtf8(info, u->userEncoding()));

          // save the user infomation
          u->SetEnableSave(true);
          u->save(Licq::User::SaveUserInfo);
        }

        Licq::Event* e = DoneServerEvent(nSubSequence, Licq::Event::ResultSuccess);
      if (e)
        ProcessDoneEvent(e);

        gPluginManager.pushPluginSignal(new Licq::PluginSignal(
            Licq::PluginSignal::SignalUser,
            Licq::PluginSignal::UserInfo, userId));
      }

      break;
    }

    default:
      gLog.warning(tr("Unknown Location Family Subtype: %04hx"), nSubtype);
      break;
  }
}

//--------ProcessBuddyFam--------------------------------------------------
void IcqProtocol::ProcessBuddyFam(Buffer& packet, unsigned short nSubtype)
{
  switch (nSubtype)
  {
  case ICQ_SNACxSUB_ONLINExLIST:
  {
    unsigned long intIP, userPort, nInfoTimestamp = 0,
                  nStatusPluginTimestamp = 0, nInfoPluginTimestamp = 0, nCookie,
                  nUserIP;
    time_t registeredTimestamp;
    unsigned char mode;

    packet.UnpackUnsignedLongBE();
    packet.UnpackUnsignedShortBE();
      string id = packet.unpackByteString();

    packet.UnpackUnsignedLongBE(); // tlvcount

    if (!packet.readTLV()) {
        gLog.error(tr("TLV Error"));
        return;
      }

//     userIP = packet.UnpackUnsignedLongTLV(0x0a, 1);
//      userIP = BSWAP_32(userIP);
      UserWriteGuard u(id);
      if (!u.isLocked())
      {
        gLog.warning(tr("Unknown user (%s) changed status."), id.c_str());
      break;
    }

    // 0 if not set -> Online
    unsigned long nNewStatus = 0;
      unsigned long nOldStatus = addStatusFlags(icqStatusFromStatus(u->status()), *u);
    nUserIP = 0;

    // AIM status
      int userClass;
    if (packet.getTLVLen(0x0001) == 2)
    {
        userClass = packet.UnpackUnsignedShortTLV(0x0001);
        if (userClass & 0x0020)
        nNewStatus = ICQ_STATUS_AWAY;
      else
        nNewStatus = ICQ_STATUS_ONLINE;
    }
      else
      {
        userClass = (isalpha(id[0]) ? 0x10 : 0x50);
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

      time_t onlineSince = 0;
    if (packet.getTLVLen(0x0003) == 4)
        onlineSince = packet.UnpackUnsignedLongTLV(0x0003);

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

      if (packet.getTLVLen(0x0029) == 4)
      {
        time_t awaySince = packet.UnpackUnsignedLongTLV(0x0029);
        u->setAwaySince(awaySince);
      }
      else if ((nNewStatus & 0xFFFF) == ICQ_STATUS_OFFLINE || (nNewStatus & 0xFFFF) == ICQ_STATUS_ONLINE)
      {
        // User is not away
        u->setAwaySince(0);
      }
      else
      {
        // User is away but no time given by server, assume it happened now
        u->setAwaySince(time(NULL));
      }

      unsigned tcpVersion = 0;
      unsigned webPort = 0;
    if (packet.getTLVLen(0x000c) == 0x25)
    {
      CBuffer msg = packet.UnpackTLV(0x000c);

      intIP = msg.UnpackUnsignedLong();
      userPort = msg.UnpackUnsignedLongBE();
      mode = msg.UnpackChar();
        tcpVersion = msg.UnpackUnsignedShortBE();
      nCookie = msg.UnpackUnsignedLongBE();
        webPort = msg.UnpackUnsignedLongBE();
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
        
      msg.UnpackUnsignedShortBE();

      u->SetVersion(tcpVersion);

        if (nOldStatus != (nNewStatus & ~ICQ_STATUS_FxUNKNOWNxFLAGS))
        {
          ChangeUserStatus(*u, nNewStatus, onlineSince);
          gLog.info(tr("%s (%s) changed status: %s (v%d)."),
              u->getAlias().c_str(), u->id().toString().c_str(),
              u->statusString().c_str(), tcpVersion & 0x0F);
        if ( (nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS) )
          gLog.unknown("Unknown status flag for %s (%s): 0x%08lX",
                u->getAlias().c_str(), u->accountId().c_str(),
                       (nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS));
        nNewStatus &= ICQ_STATUS_FxUNKNOWNxFLAGS;
        u->setAutoResponse("");
        u->SetShowAwayMsg(false);
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
        gLog.unknown("Unknown peer-to-peer mode for %s (%s): %d",
              u->getAlias().c_str(), u->accountId().c_str(), mode);
        u->setDirectMode(true);
        u->SetSendServer(false);
      }
      else
      {
        u->setDirectMode(mode == MODE_DIRECT);
        u->SetSendServer(mode == MODE_INDIRECT);
      }
    }
    else // for AIM users
      {
        if (nOldStatus != (nNewStatus & ~ICQ_STATUS_FxUNKNOWNxFLAGS))
        {
          ChangeUserStatus(*u, nNewStatus, onlineSince);
          gLog.info(tr("%s changed status: %s (AIM)."),
              u->getAlias().c_str(), u->statusString().c_str());
        if ( (nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS) )
            gLog.unknown(tr("Unknown status flag for %s: 0x%08lX"),
                u->getAlias().c_str(), nNewStatus & ICQ_STATUS_FxUNKNOWNxFLAGS);
        u->setAutoResponse("");
        u->SetShowAwayMsg(false);
      }
    }

    if (u->OurClientStatusTimestamp() != nStatusPluginTimestamp &&
        nOldStatus == ICQ_STATUS_OFFLINE)
    {
      //we don't know what the new values are yet, so don't show anything
        u->setPhoneFollowMeStatus(IcqPluginInactive);
        u->setIcqPhoneStatus(IcqPluginInactive);
        u->setSharedFilesStatus(IcqPluginInactive);
      }

      string caps;

      // Short capabilities
      if (packet.hasTLV(0x0019))
      {
        Buffer capbuf = packet.UnpackTLV(0x0019);
        while (capbuf.remainingDataToRead() >= 2)
        {
          // Convert to full capabilities
          uint8_t fullcap[16] = { 0x09, 0x46, 0, 0, 0x4C, 0x7F, 0x11,
              0xD1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 };
          fullcap[2] = capbuf.unpackUInt8();
          fullcap[3] = capbuf.unpackUInt8();
          caps.append((const char*)fullcap, 16);
        }
      }

      // Full capabilities
    if (packet.hasTLV(0x000D))
    {
      CBuffer capBuf = packet.UnpackTLV(0x000D);
        caps += packet.unpackRawString(capBuf.remainingDataToRead());
      }

      // Check if they support UTF8
      bool bUTF8 = false;
      for (size_t i = 0; i < (caps.size() / CAP_LENGTH); i++)
      {
          if (memcmp(caps.c_str()+(i * CAP_LENGTH), ICQ_CAPABILITY_UTF8, CAP_LENGTH) == 0)
          {
          bUTF8 = true;
        }
        }

        if (u.isLocked())
        {
        u->SetSupportsUTF8(bUTF8);
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
            gLog.info(tr("%s updated info."), u->getAlias().c_str());
        u->SetUserUpdated(false);
        break;

      case 0x02: //Updated info plugin settings
        msg.incDataPosRead(6); /* 02 00 01 00 01 00
                                  Don't know what those mean */
        for (unsigned int i = 0; i < sizeof(plugin); i ++)
          plugin[i] = msg.UnpackChar();

        if (memcmp(plugin, PLUGIN_PHONExBOOK, GUID_LENGTH) == 0)
              gLog.info(tr("%s updated phonebook"), u->getAlias().c_str());
        else if (memcmp(plugin, PLUGIN_PICTURE, GUID_LENGTH) == 0)
              gLog.info(tr("%s updated picture"), u->getAlias().c_str());
        else if (memcmp(plugin, PLUGIN_QUERYxINFO, GUID_LENGTH) == 0)
              gLog.info(tr("%s updated info plugin list"), u->getAlias().c_str());
        else
          gLog.warning(tr("Unknown info plugin update from %s."), u->getAlias().c_str());

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
            unsigned pluginStatus;
        switch (nPluginStatus)
        {
              case ICQ_PLUGIN_STATUSxINACTIVE:
                state = "inactive";
                pluginStatus = IcqPluginInactive;
                break;
              case ICQ_PLUGIN_STATUSxACTIVE:
                state = "active";
                pluginStatus = IcqPluginActive;
                break;
              case ICQ_PLUGIN_STATUSxBUSY:
                state = "busy";
                pluginStatus = IcqPluginBusy;
                break;
              default:
                state = "unknown";
                pluginStatus = IcqPluginInactive;
                break;
            }

        if (memcmp(plugin, PLUGIN_FOLLOWxME, GUID_LENGTH) == 0)
        {
              gLog.info(tr("%s changed Phone \"Follow Me\" status to %s."),
                  u->getAlias().c_str(), state);
              u->setPhoneFollowMeStatus(pluginStatus);
            }
        else if (memcmp(plugin, PLUGIN_FILExSERVER, GUID_LENGTH) == 0)
        {
              gLog.info(tr("%s changed Shared Files Directory status to %s."),
                  u->getAlias().c_str(), state);
              u->setSharedFilesStatus(pluginStatus);
            }
        else if (memcmp(plugin, PLUGIN_ICQxPHONE, GUID_LENGTH) == 0)
        {
              gLog.info(tr("%s changed ICQphone status to %s."),
                  u->getAlias().c_str(), state);
              u->setIcqPhoneStatus(pluginStatus);
            }

        // if status was up to date then we don't need to reask the user
        if (u->OurClientStatusTimestamp() == u->ClientStatusTimestamp() &&
            nOldStatus != ICQ_STATUS_OFFLINE)
          u->SetOurClientStatusTimestamp(nTime);


        // Which Plugin?
            gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                Licq::PluginSignal::SignalUser,
                Licq::PluginSignal::UserPluginStatus, u->id()));
            break;
          }

        default:
          gLog.warning(tr("Unknown index %d from %s."), index, u->getAlias().c_str());
      }
    }

    if (packet.hasTLV(0x001d))	// Server-stored buddy icon information
      {
        Buffer iconData = packet.UnpackTLV(0x001d);
        processIconHash(*u, iconData);
      }

    // maybe use this for auto update info later
    u->SetClientTimestamp(nInfoTimestamp);
    u->SetClientInfoTimestamp(nInfoPluginTimestamp);
    u->SetClientStatusTimestamp(nStatusPluginTimestamp);

      if ((nInfoTimestamp & 0xFFFF0000) == LICQ_WITHSSL)
        u->setSecureChannelSupport(Licq::User::SecureChannelSupported);
      else if ((nInfoTimestamp & 0xFFFF0000) == LICQ_WITHOUTSSL)
        u->setSecureChannelSupport(Licq::User::SecureChannelNotSupported);
      else
        u->setSecureChannelSupport(Licq::User::SecureChannelUnknown);

      if (nOldStatus == ICQ_STATUS_OFFLINE || u->clientInfo().empty())
      {
        string userClient = detectUserClient(caps.c_str(), caps.size(), userClass, tcpVersion,
            nInfoTimestamp, nInfoPluginTimestamp, nStatusPluginTimestamp,
            onlineSince, webPort);
        if (!userClient.empty())
        {
          gLog.info(tr("Identified user client as %s"), userClient.c_str());
          u->setClientInfo(userClient);
        }
      }

      break;
    }
  case ICQ_SNACxSUB_OFFLINExLIST:
  {
    bool bFake = false;

    packet.UnpackUnsignedLongBE();
    packet.UnpackUnsignedShortBE();
      string id = packet.unpackByteString();

    if (packet.readTLV())
    {
      if (packet.hasTLV(0x0003) && packet.getTLVLen(0x0003) == 4)
        bFake = true;
    }

    // AIM users send this when they really do go offline, so skip it if it is
    // an AIM user
    if (bFake && isdigit(id[0]))
    {
        Licq::UserReadGuard user(Licq::UserId(id, LICQ_PPID));
      //XXX Debug output
      //gLog.error(tr("Ignoring fake offline: %s (%s)"),
        //    user->getAlias().c_str(), id.c_str());
        break;
      }

      UserWriteGuard u(id);
      if (!u.isLocked())
      {
        gLog.warning(tr("Unknown user (%s) has gone offline."), id.c_str());
        break;
      }

      // Server told us something we already know
      if (u->status() == User::OfflineStatus)
        break;

      gLog.info(tr("%s went offline."), u->getAlias().c_str());
    u->SetClientTimestamp(0);
      u->setIsTyping(false);
      u->statusChanged(User::OfflineStatus);
      gPluginManager.pushPluginSignal(new Licq::PluginSignal(
          Licq::PluginSignal::SignalUser,
          Licq::PluginSignal::UserTyping, u->id()));
      break;
    }
    case ICQ_SNACxBDY_RIGHTSxGRANTED:
    {
      gLog.info(tr("Received rights for Contact List.."));

    icqUpdateContactList();
    break;
  }

    default:
      gLog.warning(tr("Unknown Buddy Family Subtype: %04hx"), nSubtype);
      break;
  }
}

//--------ProcessMessageFam------------------------------------------------
void IcqProtocol::ProcessMessageFam(Buffer& packet, unsigned short nSubtype)
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
          gLog.warning(tr("User is offline."));
          break;
        case 0x0009:
          gLog.warning(tr("Client does not understand type-2 messages."));
          break;
        case 0x000e:
          gLog.warning(tr("Packet was malformed."));
          break;
        case 0x0015:
          gLog.info(tr("List overflow."));
          break;
    default:
      gLog.unknown(tr("Unknown ICBM error: 0x%04x"), err);
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
                gLog.warning(tr("Reverse connection from %s failed."), (*iter)->myIdString.c_str());
              else
                gLog.warning(tr("Reverse connection from %s failed."), u->getAlias().c_str());
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
          gLog.warning(tr("ICBM error for unknown event."));
      }

    break;
  }
  case ICQ_SNACxMSG_SERVERxMESSAGE:
  {
    unsigned long nMsgID[2];
    unsigned long nTimeSent;
    unsigned short mFormat, nMsgLen, nTLVs;

    nMsgID[0] = packet.UnpackUnsignedLongBE();
    nMsgID[1] = packet.UnpackUnsignedLongBE();
    nTimeSent   = time(0L);
    mFormat    = packet.UnpackUnsignedShortBE();
      string id = packet.unpackByteString();
      Licq::UserId userId(id, LICQ_PPID);

    //TODO Check this again with new protocol plugin support
    //if (nUin < 10000 && nUin != ICQ_UINxPAGER && nUin != ICQ_UINxSMS)
    //{
    //  gLog.warning(tr("Message through server with strange Uin: %04lx"), nUin);
    //  break;
    //}

    packet.UnpackUnsignedShortBE();  // warning level ?
    nTLVs = packet.UnpackUnsignedShortBE() + 1;
    if (nTLVs && !packet.readTLV())
    {
        gLog.error(tr("Invalid packet format in message through server"));
        return;
      }

    switch (mFormat)
    {
        case 1:
        {
          Buffer msg = packet.UnpackTLV(0x0002);

      // TLVs in TLV
      // type: 05 01: ???
      //       01 01: 4 bytes flags? + message text
      msg.readTLV();
      CBuffer msgTxt = msg.UnpackTLV(0x0101);
      nMsgLen = msgTxt.getDataSize();

      unsigned short nEncoding = msgTxt.UnpackUnsignedShortBE();
      msgTxt.UnpackUnsignedShortBE(); // Sub encoding
      
      nMsgLen -= 4;

          string message = msgTxt.unpackRawString(nMsgLen);

      bool ignore = false;
          string userEncoding;
      // Get the user and allow adding unless we ignore new users
          {
            Licq::UserWriteGuard u(userId, !gDaemon.ignoreType(Daemon::IgnoreNewUsers));
            if (!u.isLocked())
            {
              gLog.info(tr("Message from new user (%s), ignoring"), id.c_str());
          //TODO
          ignore = true;
            }
            else
            {
              userEncoding = u->userEncoding();
              gLog.info(tr("Message through server from %s (%s)."),
                  u->getAlias().c_str(), id.c_str());
            }
          }

          switch (nEncoding)
          {
            case 2: // UCS-2 Big Endian encoding
              message = gTranslator.toUtf8(message, "UCS-2BE");
              break;
            case 0: // US-ASCII
            case 3: // Local 8bit encoding
            default:
              message = gTranslator.toUtf8(message, (ignore ? "" : userEncoding));
          }
          message = gTranslator.returnToUnix(message);

      // now send the message to the user
          Licq::EventMsg* e = new Licq::EventMsg(message, nTimeSent, 0);

      if (ignore)
      {
            gDaemon.rejectEvent(userId, e);
            break;
      }

          Licq::UserWriteGuard u(userId, !gDaemon.ignoreType(Daemon::IgnoreNewUsers));
          u->setIsTyping(false);

          if (gDaemon.addUserEvent(*u, e))
            gOnEventManager.performOnEvent(OnEventData::OnEventMessage, *u);
          gPluginManager.pushPluginSignal(new Licq::PluginSignal(
              Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserTyping, u->id()));
          break;
        }
    case 2: // OSCAR's "Add ICBM parameter" message
    {
      //I must admit, any server that does anything like this is a pile of shit
          Buffer msgTxt = packet.UnpackTLV(5);
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
            UserWriteGuard u(userId);
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

        {
          UserWriteGuard u(Licq::UserId(id, LICQ_PPID));
          if (u.isLocked())
            u->SetIpPort(nIp, nPort);
        }

        pthread_t t;
        CReverseConnectToUserData *data = new CReverseConnectToUserData(
                id, nId, DcSocket::ChannelUnknown, nIp, nPort,
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
            int channel;
        if (memcmp(GUID, PLUGIN_INFOxMANAGER, GUID_LENGTH) == 0)
              channel = DcSocket::ChannelInfo;
        else if (memcmp(GUID, PLUGIN_STATUSxMANAGER, GUID_LENGTH) == 0)
              channel = DcSocket::ChannelStatus;
            else
              channel = DcSocket::ChannelUnknown;

        bool bNewUser = false;
            UserWriteGuard u(userId, true, &bNewUser);

            processPluginMessage(advMsg, *u, channel, bIsAck, nMsgID[0], nMsgID[1],
                             nSequence, NULL);

        if (bNewUser)
            {
              u.unlock();
              Licq::gUserManager.removeLocalUser(userId);
            }

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

          string szTmpMsg = gTranslator.returnToUnix(message);
      delete [] message;

          char* szMsg = strdup(parseRtf(szTmpMsg).c_str());

      bool bNewUser = false;
          UserWriteGuard u(userId, true, &bNewUser);

          u->setIsTyping(false);

      if (msgTxt.getTLVLen(0x0004) == 4)
      {
        unsigned long Ip = BE_32(msgTxt.UnpackUnsignedLongTLV(0x0004));
        u->SetIp(Ip);
      }

      if (msgTxt.getTLVLen(0x0005) == 2)
        u->SetPort(BE_16(msgTxt.UnpackUnsignedShortTLV(0x0005)));

/*
The following code is supposed to check if user has set a different status toward us
However it seems to always think contact is online instead of away/occupied/etc...

      // Special status to us?
      if (!bIsAck && !bNewUser && nStatus != ICQ_STATUS_OFFLINE &&
            !(nStatus == ICQ_STATUS_ONLINE && u->singleStatus() == Licq::User::FreeForChatStatus) &&
            nStatus != (icqStatusFromStatus(u->status()) | (u->isInvisible() ? ICQ_STATUS_FxPRIVATE : 0)))
        {
        bool r = u->OfflineOnDisconnect() || !u->isOnline();
          u->statusChanged(statusFromIcqStatus(nStatus));
        gLog.info(tr("%s (%s) is %s to us."), u->getAlias().c_str(),
              u->id().toString().c_str(), u->statusString().c_str());
        if (r) u->SetOfflineOnDisconnect(true);
      }
*/

      // Handle it
          ProcessMessage(*u, advMsg, szMsg, nMsgType, nMask, nMsgID,
                     nSequence, bIsAck, bNewUser);

      delete [] szMsg;
          u.unlock();
      if (bNewUser) // can be changed in ProcessMessage
      {
        Licq::gUserManager.removeLocalUser(userId);
        break;
      }

          gPluginManager.pushPluginSignal(new Licq::PluginSignal(
              Licq::PluginSignal::SignalUser,
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

          string message;

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
            gLog.unknown(tr("Received SMS receipt indicating success"));
            return;
          case 0x0003:
            // SMS Receipt : Failure
            gLog.unknown(tr("Received SMS receipt indicating failure"));
            return;
          default:
            packet.log(Log::Unknown, "Unknown SMS subtype (0x%04x)", nTypeSMS);
            return;
        }

        string tag = msgTxt.unpackLongStringLE();
        if (tag != "ICQSMS")
        {
          gLog.unknown(tr("Unknown tag in SMS message: %s"), tag.c_str());
          return;
        }

        msgTxt.incDataPosRead(3);

        msgTxt.UnpackUnsignedLong(); // length till end of the message (useless)
            message = msgTxt.unpackLongStringLE();
          }
          else
          {
            message = gTranslator.returnToUnix(msgTxt.unpackShortStringLE());
          }

          processServerMessage(nTypeMsg, packet, userId, message, nTimeSent, nMask);
          break;
        }
      default:
        packet.log(Log::Unknown, tr("Message through server with unknown format: %04hx"),
                   mFormat);
        break;
    }
    break;
  }
  case ICQ_SNACxMSG_SERVERxREPLYxMSG:
  {
		unsigned short nLen, nSequence, nMsgType, nAckFlags, nMsgFlags;
      unsigned long nMsgID;
      Licq::ExtendedData* pExtendedAck = 0;

	 	packet.incDataPosRead(4); // msg id
		nMsgID = packet.UnpackUnsignedLongBE(); // lower bits, what licq uses
		packet.UnpackUnsignedShortBE(); // Format
      string id = packet.unpackByteString();

      UserWriteGuard u(id);
      if (!u.isLocked())
      {
        gLog.warning(tr("Unexpected new user in subtype 0x%04x."), nSubtype);
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
          gLog.warning(tr("Reverse connection from %s failed."), u->getAlias().c_str());
        (*iter)->bSuccess = false;
        (*iter)->bFinished = true;
        bFound = true;
        break;
      }
    }
    pthread_mutex_unlock(&mutex_reverseconnect);

      unsigned subResult;
    if (bFound)
    {
        subResult = Licq::Event::SubResultRefuse;
      pExtendedAck = NULL;
      pthread_cond_broadcast(&cond_reverseconnect_done);
        return;
      }

    packet.incDataPosRead(2);
    packet >> nLen;
    if (nLen == 0x0200)
    {
        gLog.warning(tr("%s doesn't have a manager for this event."), u->getAlias().c_str());
        u.unlock();

        Licq::Event* e = DoneServerEvent(nMsgID, Licq::Event::ResultError);
      if (e)
      {
        ProcessDoneEvent(e);
        return;
      }
        else
        {
          gLog.warning(tr("Ack for unknown event."));
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
        int channel;
      if (memcmp(GUID, PLUGIN_INFOxMANAGER, GUID_LENGTH) == 0)
          channel = DcSocket::ChannelInfo;
      else if (memcmp(GUID, PLUGIN_STATUSxMANAGER, GUID_LENGTH) == 0)
          channel = DcSocket::ChannelStatus;
        else
          channel = DcSocket::ChannelUnknown;

        processPluginMessage(packet, *u, channel, true, 0, nMsgID, nSequence, 0);

        break;
      }

      packet >> nAckFlags >> nMsgFlags;

      string message = gTranslator.toUtf8(packet.unpackShortStringLE(), u->userEncoding());

    if (nAckFlags == ICQ_TCPxACK_REFUSE)
      {
        pExtendedAck = new Licq::ExtendedData(false, 0, message);
        subResult = Licq::Event::SubResultRefuse;
        gLog.info(tr("Refusal from %s (#%lu)."), u->getAlias().c_str(), nMsgID);
      }
      else
      {
      // Update the away message if it's changed
      if (u->autoResponse() != message)
      {
        u->setAutoResponse(message);
        u->SetShowAwayMsg(!message.empty());
          gLog.info(tr("Auto response from %s (#%lu)."), u->getAlias().c_str(), nMsgID);
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
          subResult = Licq::Event::SubResultRefuse;
        }
        else
        {
          subResult = Licq::Event::SubResultReturn;
        }

        pExtendedAck = new Licq::ExtendedData(subResult == Licq::Event::SubResultReturn, 0, message);
      }
        u.unlock();

      Licq::Event* e = DoneServerEvent(nMsgID, Licq::Event::ResultAcked);
    if (e)
    {
      e->m_pExtendedAck = pExtendedAck;
      e->mySubResult = subResult;
      ProcessDoneEvent(e);
      return;
    }
      else
      {
        gLog.warning(tr("Ack for unknown event."));
        break;
      }

    break;
    }
    case ICQ_SNACxMSG_RIGHTSxGRANTED:
    {
      gLog.info(tr("Received rights for Instant Messaging.."));
      break;
    }
    case ICQ_SNACxMSG_SERVERxACK:
    {
      Licq::Event* e = DoneServerEvent(nSubSequence, Licq::Event::ResultAcked);
    if (e)
    {
        e->mySubResult = Licq::Event::SubResultAccept;
      ProcessDoneEvent(e);
    }

      gLog.info(tr("Message was sent to an offline user. It will be delivered"
          " when the user logs on."));
      break;
    }
  case ICQ_SNACxMSG_TYPING:
  {
    packet.UnpackUnsignedLongBE(); // timestamp
    packet.UnpackUnsignedLongBE(); // message id
    packet.UnpackUnsignedShortBE(); // format (only seen 1)
      string id = packet.unpackByteString();
    unsigned short nTyping = packet.UnpackUnsignedShortBE();

      Licq::UserWriteGuard u(Licq::UserId(id, LICQ_PPID));
      if (!u.isLocked())
      {
        gLog.warning(tr("Typing status received for unknown user (%s)."), id.c_str());
        break;
      }
      u->setIsTyping(nTyping == ICQ_TYPING_ACTIVE);
      gPluginManager.pushPluginSignal(new Licq::PluginSignal(
          Licq::PluginSignal::SignalUser,
          Licq::PluginSignal::UserTyping, u->id()));
    break;
  }
    default:
      gLog.warning(tr("Unknown Message Family Subtype: %04hx"), nSubtype);
      break;
  }
}

void IcqProtocol::processStatsFam(Buffer& packet, int subType)
{
  switch (subType)
  {
    case ICQ_SNACxSTATS_ERROR:
    {
      int errorCode = packet.unpackUInt16BE();
      gLog.warning(tr("Got server error for stats: 0x%02x"), errorCode);
      break;
    }
    case ICQ_SNACxSTATS_REPORT_INTERVAL:
    {
#if 0
      int interval = packet.unpackUInt16BE();
      gLog.info(tr("Server wants stats reports every %i hours"), interval);
#endif
      break;
    }
    case ICQ_SNACxSTATS_REPORT_ACK:
      // Server ack for stats data sent by us, no data
      break;

    default:
      gLog.warning(tr("Unknown Message Stats Subtype: 0x%04x"), subType);
      break;
  }
}

//--------ProcessListFam--------------------------------------------
void IcqProtocol::ProcessListFam(Buffer& packet, unsigned short nSubtype)
{
  unsigned short nFlags = packet.UnpackUnsignedShortBE();
  unsigned long nSubSequence = packet.UnpackUnsignedLongBE();

  if (nFlags & 0x8000)
  {
    unsigned short bytes = packet.UnpackUnsignedShortBE();
    if (!packet.readTLV(-1, bytes))
    {
      gLog.error(tr("Error parsing SNAC header."));
      return;
    }
  }

  switch (nSubtype)
  {
    case ICQ_SNACxLIST_RIGHTSxGRANTED:
    {
      gLog.info(tr("Server granted contact list rights."));
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
        string id = packet.unpackShortStringBE();
        unsigned short nTag = packet.UnpackUnsignedShortBE();
        unsigned short nID = packet.UnpackUnsignedShortBE();
        unsigned short nType = packet.UnpackUnsignedShortBE();

        unsigned short nByteLen = packet.UnpackUnsignedShortBE();

        if (nByteLen)
        {
          if (!packet.readTLV(-1, nByteLen))
          {
            gLog.error(tr("Unable to parse contact list TLV, aborting!"));
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
              receivedUserList.insert(make_pair(id, (CUserProperties*)NULL));

            ContactUserListIter iter = ret.first;
            if (ret.second) // we inserted a new NULL pair
              iter->second = new CUserProperties();

            CUserProperties* data = iter->second;

            TlvList list = packet.getTlvList();
            for (TlvList::iterator it = list.begin(); it != list.end(); it++)
              data->tlvs[it->first] = it->second;

#define COPYTLV(type, var) \
            if (packet.hasTLV(type)) \
              data->var = packet.unpackTlvString(type)

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

            if (id[0] != '\0' && nTag != 0)
            {
              int groupId = getGroupFromId(nTag);
              if (groupId != 0)
              {
                // Group exist, make sure it has the correct name
                Licq::gUserManager.RenameGroup(groupId, id, LICQ_PPID);
                break;
              }

              groupId = Licq::gUserManager.GetGroupFromName(id);
              if (groupId != 0)
              {
                // A local group has same name, save the server id for it
                Licq::gUserManager.setGroupServerId(groupId, LICQ_PPID, nTag);
                break;
              }

              groupId = Licq::gUserManager.AddGroup(id);
              if (groupId != 0)
              {
                // Local group created, save the server id for it
                Licq::gUserManager.setGroupServerId(groupId, LICQ_PPID, nTag);
                gLog.info(tr("Added group %s (%u) to list from server"), id.c_str(), nTag);
              }
            }
            else
            {
              gLog.info(tr("Got Master Group record."));
            }

            break;
          }

          case ICQ_ROSTxPDINFO:
          {
            unsigned char cPrivacySettings = packet.UnpackCharTLV(0x00CA);

            OwnerWriteGuard o;
            gLog.info(tr("Got Privacy Setting."));
            o->SetPDINFO(nID);
            if (cPrivacySettings == ICQ_PRIVACY_ALLOW_FOLLOWING)
              o->statusChanged(o->status() | Licq::User::InvisibleStatus);
            break;
          }
        }  // switch (nType)
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
        OwnerWriteGuard o;
        o->SetSSTime(nTime);
        o->SetSSCount(nCount);
      }

      if (nFlags & 0x0001)
      {
        if (!hasServerEvent(nSubSequence))
          gLog.warning(tr("Contact list without request."));
        else
          gLog.info(tr("Received contact list."));
      }
      else
      {
        // This is the last packet so mark it as done
        DoneServerEvent(nSubSequence, Licq::Event::ResultSuccess);

        gLog.info(tr("Received end of contact list."));

        ProcessUserList();

        gLog.info(tr("Activating server contact list."));
        CSrvPacketTcp *p = new CPU_GenericFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxACK);
        SendEvent_Server(p);
      }

      break;
    } // case rost reply

    case ICQ_SNACxLIST_ROSTxUPD_GROUP:
    {
      gLog.info(tr("Received updated contact information from server."));

      string id = packet.unpackShortStringBE();
      if (id.empty())
      {
        gLog.error(tr("Did not receive user ID."));
        break;
      }

      unsigned short gsid    = packet.UnpackUnsignedShortBE();
      unsigned short sid     = packet.UnpackUnsignedShortBE();
      /*unsigned short classid =*/ packet.UnpackUnsignedShortBE();

      unsigned short tlvBytes = packet.UnpackUnsignedShortBE();

      if (!packet.readTLV(-1, tlvBytes))
      {
        gLog.error(tr("Error during parsing packet!"));
        break;
      }

      UserWriteGuard u(id);
      if (u.isLocked())
      {
        // First update their gsid/sid
        u->SetSID(sid);
        u->removeFromGroup(getGroupFromId(u->GetGSID()));
        u->SetGSID(gsid);
        u->addToGroup(getGroupFromId(gsid));

        // Now the the tlv of attributes to attach to the user
        TlvList tlvList = packet.getTlvList();
        TlvList::iterator iter;
        for (iter = tlvList.begin(); iter != tlvList.end(); ++iter)
          u->AddTLV(iter->second);

        u->save(Licq::User::SaveLicqInfo);
        gPluginManager.pushPluginSignal(new Licq::PluginSignal(
            Licq::PluginSignal::SignalUser,
            Licq::PluginSignal::UserGroups, u->id()));
      }

      break;
    }

    case ICQ_SNACxLIST_ROSTxSYNCED:
    {
      DoneServerEvent(nSubSequence, Licq::Event::ResultSuccess);

      gLog.info(tr("Contact list is synchronized."));
      // The server says we are up to date, let's double check
      CheckExport();

      gLog.info(tr("Activate server contact list."));
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
        gLog.warning(tr("Server list update ack without request."));
        break;
      }

      unsigned long nListTime;
      {
        OwnerWriteGuard o;
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
          gLog.info(tr("Skipping invalid server side list ack (%ld)."), nSubSequence);
          break;
        }
        std::string pending = mapIter->second;

        nError = packet.UnpackUnsignedShortBE();

        switch (nError)
        {
          case 0: break; // No error

          case 0x0E:
            gLog.info(tr("%s added to awaiting authorization group on server list."),
                pending.c_str());
            break;

          case 0x02:
            gLog.warning(tr("User/Group %s not found on server list."),
                pending.c_str());
            break;

          default:
            gLog.warning(tr("Unknown error modifying server list: 0x%02X (ID: %s)"),
                nError & 0xFF, pending.c_str());
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
              int n = getGroupFromId(e->ExtraInfo());
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
                gLog.info(tr("Updating top level group."));
              }
              else
              {
                pReply = new CPU_UpdateToServerList(groupName, ICQ_ROSTxGROUP, e->ExtraInfo());
                gLog.info(tr("Updating group %s."), groupName.c_str());
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
                u->addToGroup(getGroupFromId(e->ExtraInfo()));
                gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                    Licq::PluginSignal::SignalAddedToServer, 0, u->id()));
              }
            }

            break;
          }

          case ICQ_SNACxLIST_ROSTxUPD_GROUP:
          {
            if (bHandled == false)
            {
              bHandled = true;
              gLog.info(tr("Updated %s successfully."),
                  pending.empty() ? tr("top level") : pending.c_str());
                
              if (nError == 0x0E)
              {
                pReply = new CPU_UpdateToServerList(pending, ICQ_ROSTxNORMAL, 0, true);
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
      string id = packet.unpackByteString();
      Licq::UserId userId(id, LICQ_PPID);
      string userEncoding = getUserEncoding(userId);
      bool bIgnore;
      {
        Licq::UserReadGuard u(userId);
        bIgnore = (u.isLocked() && u->IgnoreList());
      }

      if (bIgnore)
        break;

      gLog.info(tr("Authorization request from %s."), id.c_str());

      string msg = gTranslator.returnToUnix(gTranslator.toUtf8(packet.unpackShortStringLE(), userEncoding));
      Licq::EventAuthRequest* e = new Licq::EventAuthRequest(userId, "", "", "", "", msg,
          time(0), 0);

      Licq::OwnerWriteGuard o(LICQ_PPID);
      if (gDaemon.addUserEvent(*o, e))
      {
        e->AddToHistory(*o, true);
        gOnEventManager.performOnEvent(OnEventData::OnEventSysMsg, *o);
      }
      break;
    }

    case ICQ_SNACxLIST_AUTHxRESPONS: // The resonse to our authorization request
    {
      string id = packet.unpackByteString();
      Licq::UserId userId(id, LICQ_PPID);
      string userEncoding = getUserEncoding(userId);
      unsigned char granted;

      packet >> granted;
      string msg = gTranslator.returnToUnix(gTranslator.toUtf8(packet.unpackShortStringLE(), userEncoding));

      gLog.info(tr("Authorization %s by %s."), granted ? "granted" : "refused", id.c_str());

      Licq::UserEvent* eEvent;
      if (granted)
      {
        eEvent = new Licq::EventAuthGranted(userId, msg, time(0), 0);

        UserWriteGuard u(userId);
        if (u.isLocked())
        {
           u->SetAwaitingAuth(false);
           u->RemoveTLV(0x0066);
        }
      }
      else
      {
        eEvent = new Licq::EventAuthRefused(userId, msg, time(0), 0);
      }

      Licq::OwnerWriteGuard o(LICQ_PPID);
      if (gDaemon.addUserEvent(*o, eEvent))
      {
        eEvent->AddToHistory(*o, true);
        gOnEventManager.performOnEvent(OnEventData::OnEventSysMsg, *o);
      }
      break;
    }

    case ICQ_SNACxLIST_AUTHxADDED: // You were added to a contact list
    {
      string id = packet.unpackByteString();
      Licq::UserId userId(id, LICQ_PPID);
      gLog.info(tr("User %s added you to their contact list."), id.c_str());

      Licq::EventAdded* e = new Licq::EventAdded(userId, "", "", "", "",
          time(0), 0);
      {
        Licq::OwnerWriteGuard o(LICQ_PPID);
        if (gDaemon.addUserEvent(*o, e))
        {
          e->AddToHistory(*o, true);
          gOnEventManager.performOnEvent(OnEventData::OnEventSysMsg, *o);
        }
      }
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
      gLog.warning(tr("Unknown List Family Subtype: %04hx"), nSubtype);
      break;
  } // switch subtype
}

//--------ProcessBosFam---------------------------------------------
void IcqProtocol::ProcessBOSFam(Buffer& /* packet */, unsigned short nSubtype)
{
  switch (nSubtype)
  {
    case ICQ_SNACxBOS_RIGHTSxGRANTED:
    {
      gLog.info(tr("Received BOS rights."));

    icqSetStatus(m_nDesiredStatus);

      gLog.info(tr("Sending client ready..."));
    CSrvPacketTcp *p = new CPU_ClientReady();
    SendEvent_Server(p);

      gLog.info(tr("Sending offline message request..."));
    p = new CPU_RequestSysMsg;
    SendEvent_Server(p);

    m_eStatus = STATUS_ONLINE;
    m_bLoggingOn = false;
    // ### FIX subsequence !!
      Licq::Event* e = DoneExtendedServerEvent(0, Licq::Event::ResultSuccess);
    if (e != NULL) ProcessDoneEvent(e);
      gPluginManager.pushPluginSignal(new Licq::PluginSignal(
          Licq::PluginSignal::SignalLogon, 0, Licq::gUserManager.ownerUserId(LICQ_PPID)));

    //icqSetStatus(m_nDesiredStatus);
      break;
    }
    default:
      gLog.warning(tr("Unknown BOS Family Subtype: %04hx"), nSubtype);
      break;
  }
}

//--------ProcessVariousFam-----------------------------------------
void IcqProtocol::ProcessVariousFam(Buffer& packet, unsigned short nSubtype)
{
  /*unsigned long Flags =*/ packet.UnpackUnsignedLongBE();
  unsigned short nSubSequence = packet.UnpackUnsignedShortBE();

  switch (nSubtype)
  {
    case 0x0001: // Error
    {
      unsigned short errorcode = packet.UnpackUnsignedShortBE();
      gLog.info(tr("Meta request failed. Eventid %u, errorcode %u"), nSubSequence, errorcode);
      Licq::Event* pEvent = DoneServerEvent(nSubSequence, Licq::Event::ResultFailed);
      if (pEvent != NULL)
        ProcessDoneEvent(pEvent);
      break;
    }

  case 0x0003: // multi-purpose sub-type
  {
    unsigned short nType, nId;

    if (!packet.readTLV()) {
      packet.log(Log::Unknown, tr("Unknown server response"));
      break;
    }

    CBuffer msg = packet.UnpackTLV(0x0001);

    if (msg.Empty()) {
      packet.log(Log::Unknown, tr("Unknown server response"));
      break;
    }

    msg.UnpackUnsignedShortBE(); // Length
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
            nTimeSent = mktime(&sendTM) - o->systemTimezone();
          }

      // Msg type & flags
      unsigned short nTypeMsg = msg.UnpackUnsignedShort();
          unsigned long nMask = Licq::UserEvent::FlagOffline |
              ((nTypeMsg & ICQ_CMDxSUB_FxMULTIREC) ? (int)Licq::UserEvent::FlagMultiRec : 0);
      nTypeMsg &= ~ICQ_CMDxSUB_FxMULTIREC;

      // 2 byte length little endian + string
          string message = msg.unpackShortStringLE();
          processServerMessage(nTypeMsg, packet, userId, message, nTimeSent, nMask);
          break;
        }
        case 0x0042:
        {
          gLog.info(tr("End of Offline messages (nId: %04hx)."), nId);
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
          string type;

      if (nSubtype == ICQ_CMDxMETA_PASSWORDxRSP)
          {
            type = tr("Password change");
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);
        if (pEvent != NULL && nResult == META_SUCCESS)
        {
              Licq::OwnerWriteGuard o(LICQ_PPID);
          o->SetEnableSave(false);
              o->setPassword(((CPU_SetPassword *)pEvent->m_pPacket)->myPassword);
          o->SetEnableSave(true);
              o->save(Licq::Owner::SaveOwnerInfo);
            }
          }
      else if (nSubtype == ICQ_CMDxMETA_SECURITYxRSP)
          {
            type = tr("Security info");
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);
      }
      else if (nSubtype == ICQ_CMDxMETA_GENERALxINFOxRSP)
          {
            type = tr("General info");
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetGeneralInfo *p = (CPU_Meta_SetGeneralInfo *)pEvent->m_pPacket;

              OwnerWriteGuard o;
              o->SetEnableSave(false);
              o->setAlias(p->myAlias);
              o->setUserInfoString("FirstName", p->myFirstName);
              o->setUserInfoString("LastName", p->myLastName);
              o->setUserInfoString("Email1", p->myEmailPrimary);
              o->setUserInfoString("City", p->myCity);
              o->setUserInfoString("State", p->myState);
              o->setUserInfoString("PhoneNumber", p->myPhoneNumber);
              o->setUserInfoString("FaxNumber", p->myFaxNumber);
              o->setUserInfoString("Address", p->myAddress);
              o->setUserInfoString("CellularNumber", p->myCellularNumber);
              o->setUserInfoString("Zipcode", p->myZipCode);
              o->setUserInfoUint("Country", p->m_nCountryCode);
          o->SetTimezone(p->m_nTimezone);
              o->setUserInfoBool("HideEmail", p->m_nHideEmail); // 0 = no, 1 = yes

          // save the user infomation
          o->SetEnableSave(true);
              o->save(Licq::Owner::SaveUserInfo);
            }
          }
      else if (nSubtype == ICQ_CMDxMETA_EMAILxINFOxRSP)
          {
            type = tr("E-mail info");
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetEmailInfo *p = (CPU_Meta_SetEmailInfo *)pEvent->m_pPacket;

              Licq::OwnerWriteGuard o(LICQ_PPID);

          o->SetEnableSave(false);
              o->setUserInfoString("Email2", p->myEmailSecondary);
              o->setUserInfoString("Email0", p->myEmailOld);

          // save the user infomation
          o->SetEnableSave(true);
              o->save(Licq::Owner::SaveUserInfo);
            }
          }
      else if (nSubtype == ICQ_CMDxMETA_MORExINFOxRSP)
          {
            type = tr("More info");
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetMoreInfo *p = (CPU_Meta_SetMoreInfo *)pEvent->m_pPacket;

              Licq::OwnerWriteGuard o(LICQ_PPID);
          o->SetEnableSave(false);
              o->setUserInfoUint("Age", p->m_nAge);
              o->setUserInfoUint("Gender", p->m_nGender);
              o->setUserInfoString("Homepage", p->myHomepage);
              o->setUserInfoUint("BirthYear", p->m_nBirthYear);
              o->setUserInfoUint("BirthMonth", p->m_nBirthMonth);
              o->setUserInfoUint("BirthDay", p->m_nBirthDay);
              o->setUserInfoUint("Language1", p->m_nLanguage1);
              o->setUserInfoUint("Language2", p->m_nLanguage2);
              o->setUserInfoUint("Language3", p->m_nLanguage3);

          // save the user infomation
          o->SetEnableSave(true);
              o->save(Licq::Owner::SaveUserInfo);
            }
          }
      else if (nSubtype == ICQ_CMDxMETA_INTERESTSxINFOxRSP)
          {
            type = "Interests info";
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetInterestsInfo *p =
                           (CPU_Meta_SetInterestsInfo *)pEvent->m_pPacket;
              OwnerWriteGuard o;
          o->SetEnableSave(false);
              o->getInterests().clear();
              UserCategoryMap::iterator i;
              for (i = p->myInterests.begin(); i != p->myInterests.end(); ++i)
                o->getInterests()[i->first] = i->second;

          o->SetEnableSave(true);
              o->save(Licq::Owner::SaveUserInfo);
            }
          }
      else if (nSubtype == ICQ_CMDxMETA_WORKxINFOxRSP)
          {
            type = tr("Work info");
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetWorkInfo *p = (CPU_Meta_SetWorkInfo *)pEvent->m_pPacket;

              Licq::OwnerWriteGuard o(LICQ_PPID);
          o->SetEnableSave(false);
              o->setUserInfoString("CompanyCity", p->myCity);
              o->setUserInfoString("CompanyState", p->myState);
              o->setUserInfoString("CompanyPhoneNumber", p->myPhoneNumber);
              o->setUserInfoString("CompanyFaxNumber", p->myFaxNumber);
              o->setUserInfoString("CompanyAddress", p->myAddress);
              o->setUserInfoString("CompanyZip", p->myZip);
              o->setUserInfoUint("CompanyCountry", p->m_nCompanyCountry);
              o->setUserInfoString("CompanyName", p->myName);
              o->setUserInfoString("CompanyDepartment", p->myDepartment);
              o->setUserInfoString("CompanyPosition", p->myPosition);
              o->setUserInfoUint("CompanyOccupation", p->m_nCompanyOccupation);
              o->setUserInfoString("CompanyHomepage", p->myHomepage);

          // save the user infomation
          o->SetEnableSave(true);
              o->save(Licq::Owner::SaveUserInfo);
            }
          }
      else if (nSubtype == ICQ_CMDxMETA_ABOUTxRSP)
          {
            type = tr("About");
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
          CPU_Meta_SetAbout *p = (CPU_Meta_SetAbout *)pEvent->m_pPacket;
              Licq::OwnerWriteGuard o(LICQ_PPID);
          o->SetEnableSave(false);
              o->setUserInfoString("About", gTranslator.returnToUnix(p->myAbout));

          // save the user infomation
          o->SetEnableSave(true);
              o->save(Licq::Owner::SaveUserInfo);
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
              type = "Organizations/Background info";

          if (nResult == META_SUCCESS)
          {
            CPU_Meta_SetOrgBackInfo *p =
                             (CPU_Meta_SetOrgBackInfo *)pEvent->m_pPacket;
                OwnerWriteGuard o;
            o->SetEnableSave(false);
                o->getOrganizations().clear();
                UserCategoryMap::iterator i;
                for (i = p->myOrganizations.begin(); i != p->myOrganizations.end(); ++i)
                  o->getOrganizations()[i->first] = i->second;

                o->getBackgrounds().clear();
                for (i = p->myBackgrounds.begin(); i != p->myBackgrounds.end(); ++i)
                  o->getBackgrounds()[i->first] = i->second;

            o->SetEnableSave(true);
                o->save(Licq::Owner::SaveUserInfo);
              }
            }
        else if (pEvent != NULL &&
                pEvent->m_pPacket->SubCommand() == ICQ_CMDxMETA_SENDxSMS)
            {
          msg.UnpackUnsignedShortBE();
          msg.UnpackUnsignedShortBE();
          msg.UnpackUnsignedShortBE();
              msg.unpackShortStringBE(); // tag
              string xml = msg.unpackShortStringBE();
              string smsResponse = getXmlTag(xml, "sms_response");

              if (!smsResponse.empty())
              {
                string deliverable = getXmlTag(smsResponse, "deliverable");
                if (!deliverable.empty())
                {
                  if (deliverable == "Yes")
                  {
                    gLog.info(tr("SMS delivered."));
                    if (pEvent)
                    {
                      pEvent->m_eResult = Licq::Event::ResultSuccess;
                  ProcessDoneEvent(pEvent);
                }
                  }
                  else if (deliverable == "No")
                  {
                    string id = getXmlTag(smsResponse, "id");
                    string param = getXmlTag(smsResponse, "param");
                    gLog.warning(tr("SMS not delivered, error #%s, param: %s"),
                        id.c_str(), param.c_str());

                if (pEvent)
                    {
                      pEvent->m_eResult = Licq::Event::ResultFailed;
                  ProcessDoneEvent(pEvent);
                }
                  }
                  else if (deliverable ==  "SMTP")
                  {
                    string from = getXmlTag(smsResponse, "from");
                    string to = getXmlTag(smsResponse, "to");
                    string subject = getXmlTag(smsResponse, "subject");
                    gLog.info(tr("Sending SMS via SMTP not supported yet."));

                    if (!from.empty())
                      gLog.info(tr("From: %s"), from.c_str());

                    if (!to.empty())
                      gLog.info(tr("To: %s"), to.c_str());

                    if (!subject.empty())
                      gLog.info(tr("Subject: %s"), subject.c_str());

                    if (pEvent)
                    {
                      pEvent->m_eResult = Licq::Event::ResultFailed;
                  ProcessDoneEvent(pEvent);
                }
              }
                  else
                  {
                    packet.log(Log::Unknown, tr("Unknown SMS response"));

                if (pEvent)
                    {
                      pEvent->m_eResult = Licq::Event::ResultFailed;
                  ProcessDoneEvent(pEvent);
                }
              }
            }
                else
                {
                  gLog.info(tr("Undeliverable SMS."));
                  if (pEvent)
                  {
                    pEvent->m_eResult = Licq::Event::ResultFailed;
                ProcessDoneEvent(pEvent);
              }
            }
          }
        }
      }
      else if (nSubtype == ICQ_CMDxMETA_SETxRANDxCHATxRSP)
          {
            type = tr("Random chat group");
        pEvent = DoneServerEvent(nSubSequence,
                nResult == META_SUCCESS ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);

        if (pEvent != NULL && nResult == META_SUCCESS)
        {
              OwnerWriteGuard o;
              o->setRandomChatGroup(((CPU_SetRandomChatGroup *)pEvent->m_pPacket)->Group());
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
              gLog.info(tr("Random chat user found (%s)."), szUin);
          bool bNewUser = false;

              {
                UserWriteGuard u(userId, true, &bNewUser);

          msg.UnpackUnsignedShort(); // chat group

          nIp = LE_32(msg.UnpackUnsignedLongBE());
          u->SetIpPort(nIp, msg.UnpackUnsignedLong());

          nIp = LE_32(msg.UnpackUnsignedLongBE());
          u->SetIntIp(nIp);

          msg >> nMode;
          u->setDirectMode(nMode == MODE_DIRECT);
          if (nMode != MODE_DIRECT)
            u->SetSendServer(true);

          u->SetVersion(msg.UnpackUnsignedShort());
              }

          if (bNewUser)
                icqRequestMetaInfo(userId);

              e->m_pSearchAck = new Licq::SearchData(userId);
            }
            else
              gLog.info(tr("No random chat user found."));
        ProcessDoneEvent(e);
      }
      else if (nSubtype == ICQ_CMDxMETA_WPxINFOxSET_RSP)
      {
            Licq::Event* e = DoneServerEvent(nSubSequence,
                nResult == 0x0A ? Licq::Event::ResultSuccess : Licq::Event::ResultFailed);
        if (e == NULL)
        {
              gLog.info(tr("Received info update ack, without updating info."));
              break;
            }

            gLog.info(tr("Updated information successfully."));
        ProcessDoneEvent(e);
      }
      else if (nSubtype == 0x0001)
      {
            Licq::Event* e = DoneServerEvent(nSubSequence, Licq::Event::ResultFailed);
            gLog.info(tr("SMS failed to send."));
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
              gLog.info(tr("WhitePages search found no users."));
              e = DoneExtendedServerEvent(nSubSequence, Licq::Event::ResultSuccess);
              Licq::Event* e2 = new Licq::Event(e);
          e2->m_pSearchAck = NULL; // Search ack is null lets plugins know no results found
              e2->myCommand = Licq::Event::CommandSearch;
              e2->myFlags |= Licq::Event::FlagSearchDone;
              gPluginManager.pushPluginEvent(e2);
              DoneEvent(e, Licq::Event::ResultSuccess);
              break;
            }

            e = DoneExtendedServerEvent(nSubSequence, Licq::Event::ResultAcked);
        if (e == NULL)
        {
              gLog.warning(tr("Unmatched extended event (%d)!"), nSubSequence);
              break;
            }

        unsigned long nFoundUin;

        msg.UnpackUnsignedShort(); // length of the rest of the packet.
        nFoundUin = msg.UnpackUnsignedLong();
        char foundAccount[14];
        snprintf(foundAccount, sizeof(foundAccount), "%lu", nFoundUin);
            Licq::UserId foundUserId(foundAccount, LICQ_PPID);

            Licq::SearchData* s = new Licq::SearchData(foundUserId);

            s->myAlias = msg.unpackShortStringLE();
            s->myFirstName = msg.unpackShortStringLE();
            s->myLastName = msg.unpackShortStringLE();
            s->myEmail = msg.unpackShortStringLE();
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

            gLog.info("%s (%lu) <%s %s, %s>", s->alias().c_str(),
                nFoundUin, s->firstName().c_str(), s->lastName().c_str(), s->email().c_str());

            Licq::Event* e2 = new Licq::Event(e);
        // JON: Hack it so it is backwards compatible with plugins for now.
        e2->m_pSearchAck = s;
        if (nSubtype & 0x0008)
        {
          unsigned long nMore = 0;
              e2->myCommand = Licq::Event::CommandSearch;
              e2->myFlags |= Licq::Event::FlagSearchDone;
          nMore = msg.UnpackUnsignedLong();
          // No more subtraction by 1, and now it seems to always be 0
              e2->m_pSearchAck->myMore = nMore;
              e2->m_eResult = Licq::Event::ResultSuccess;
        }
        else
        {
              e2->myCommand = Licq::Event::CommandSearch;
              e2->m_pSearchAck->myMore = 0;
        }

            gPluginManager.pushPluginEvent(e2);

        if (nSubtype & 0x0008)
              DoneEvent(e, Licq::Event::ResultSuccess); // Remove it from the running event list
        else
          PushExtendedEvent(e);
      }
          else
          {
            Licq::Event* e = NULL;
        Licq::UserId userId;
        bool multipart = false;

        if ((nResult == 0x32) || (nResult == 0x14) || (nResult == 0x1e))
        {
          // error: empty result or nonexistent user (1E =  readonly???)
              gLog.warning(tr("Failed to update user info: %x."), nResult);
              e = DoneExtendedServerEvent(nSubSequence, Licq::Event::ResultFailed);
          if (e)
          {
                DoneEvent(e, Licq::Event::ResultFailed);
            ProcessDoneEvent(e);
          }
          e = NULL;
          break;
        }

            // Find the relevant event
            e = DoneExtendedServerEvent(nSubSequence, Licq::Event::ResultSuccess);
            if (e == NULL)
            {
              gLog.warning(tr("Unmatched extended event (%d)!"), nSubSequence);
              break;
            }

            userId = e->userId();
            UserWriteGuard u(userId, true);

            gLog.info(tr("Received extended information for %s (%s)."),
                u->getAlias().c_str(), userId.toString().c_str());


        switch (nSubtype)
        {
        case ICQ_CMDxMETA_GENERALxINFO:
              {
                gLog.info(tr("General info on %s (%s)."),
                    u->getAlias().c_str(), u->accountId().c_str());

          // main home info
          u->SetEnableSave(false);
                string alias = msg.unpackShortStringLE();
          // Skip the alias if user wants to keep his own.
          if (!u->m_bKeepAliasOnUpdate || userId == Licq::gUserManager.ownerUserId(LICQ_PPID))
          {
                  u->setAlias(alias);
                }
                u->setUserInfoString("FirstName", msg.unpackShortStringLE());
                u->setUserInfoString("LastName", msg.unpackShortStringLE());
                u->setUserInfoString("Email1", msg.unpackShortStringLE());
                u->setUserInfoString("City", msg.unpackShortStringLE());
                u->setUserInfoString("State", msg.unpackShortStringLE());
                u->setUserInfoString("PhoneNumber", msg.unpackShortStringLE());
                u->setUserInfoString("FaxNumber", msg.unpackShortStringLE());
                u->setUserInfoString("Address", msg.unpackShortStringLE());
                u->setUserInfoString("CellularNumber", msg.unpackShortStringLE());
                u->setUserInfoString("Zipcode", msg.unpackShortStringLE());
                u->setUserInfoUint("Country", msg.UnpackUnsignedShort() );
          u->SetTimezone( msg.UnpackChar() );
          u->SetAuthorization( !msg.UnpackChar() );
          unsigned char nStatus = msg.UnpackChar(); // Web aware status

                if (!u->isUser())
                {
                  dynamic_cast<Owner*>(*u)->SetWebAware(nStatus);
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
            gLog.error(tr("Connection flags??? %x"), cf); */

          // save the user infomation
          u->SetEnableSave(true);
                u->save(Licq::User::SaveUserInfo);

          PushExtendedEvent(e);
          multipart = true;

                gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                    Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserBasic, u->id()));
                gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                    Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
                break;
              }
        case ICQ_CMDxMETA_MORExINFO:
              {
                gLog.info(tr("More info on %s (%s)."),
                    u->getAlias().c_str(), u->accountId().c_str());

          u->SetEnableSave(false);
                u->setUserInfoUint("Age", msg.UnpackUnsignedShort());
                u->setUserInfoUint("Gender", msg.UnpackChar());
                u->setUserInfoString("Homepage", msg.unpackShortStringLE());
                u->setUserInfoUint("BirthYear", msg.UnpackUnsignedShort());
                u->setUserInfoUint("BirthMonth", msg.UnpackChar());
                u->setUserInfoUint("BirthDay", msg.UnpackChar());
                u->setUserInfoUint("Language1", msg.UnpackChar());
                u->setUserInfoUint("Language2", msg.UnpackChar());
                u->setUserInfoUint("Language3", msg.UnpackChar());

          //TODO
/*
          if (unsigned short tmp = msg.UnpackUnsignedShort()) //??
            gLog.error("Unknown value %x\n", tmp);
                string city = msg.unpackShortStringLE(); //Originally from city
                string state = msg.unpackShortStringLE(); //Originally from state
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
                gLog.info(tr("%s status is %s, originally from: %s, %s, %s"),
                    u->getAlias().c_str(), mstatus, city.c_str(), state.c_str(), country);
          */

          // save the user infomation
          u->SetEnableSave(true);
                u->save(Licq::User::SaveUserInfo);

          PushExtendedEvent(e);
          multipart = true;

                gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                    Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
                break;
              }
        case ICQ_CMDxMETA_EMAILxINFO:
              {
                gLog.info(tr("Email info on %s (%s)."),
                    u->getAlias().c_str(), u->accountId().c_str());

          u->SetEnableSave(false);
          int nEmail = (int)msg.UnpackChar();
                u->setUserInfoString("Email2", nEmail > 0 ? msg.unpackShortStringLE() : "");
                u->setUserInfoString("Email0", nEmail > 1 ? msg.unpackShortStringLE() : "");

          // save the user infomation
          u->SetEnableSave(true);
                u->save(Licq::User::SaveUserInfo);

          PushExtendedEvent(e);
          multipart = true;

                gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                    Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
                break;
              }
        case ICQ_CMDxMETA_HOMEPAGExINFO:
        {
                gLog.info(tr("Homepage info on %s (%s)."), u->getAlias().c_str(),
                    u->accountId().c_str());

          u->SetEnableSave(false);
          
          unsigned char categoryPresent = msg.UnpackChar();
                u->setUserInfoBool("HomepageCatPresent", categoryPresent);

          if (categoryPresent)
          {
                  u->setUserInfoUint("HomepageCatCode", msg.UnpackUnsignedShort());
                  u->setUserInfoString("HomepageDesc",
                      gTranslator.returnToUnix(msg.unpackShortStringLE()));
                }

                u->setUserInfoBool("ICQHomepagePresent", msg.UnpackChar());

          // save the user infomation
          u->SetEnableSave(true);
                u->save(Licq::User::SaveUserInfo);

          PushExtendedEvent(e);
          multipart = true;

                gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                    Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
                break;
              }

              case ICQ_CMDxMETA_WORKxINFO:
                gLog.info(tr("Work info on %s (%s)."),
                    u->getAlias().c_str(), u->accountId().c_str());

          u->SetEnableSave(false);
                u->setUserInfoString("CompanyCity", msg.unpackShortStringLE());
                u->setUserInfoString("CompanyState", msg.unpackShortStringLE());
                u->setUserInfoString("CompanyPhoneNumber", msg.unpackShortStringLE());
                u->setUserInfoString("CompanyFaxNumber", msg.unpackShortStringLE());
                u->setUserInfoString("CompanyAddress", msg.unpackShortStringLE());
                u->setUserInfoString("CompanyZip", msg.unpackShortStringLE());
                u->setUserInfoUint("CompanyCountry", msg.UnpackUnsignedShort());
                u->setUserInfoString("CompanyName", msg.unpackShortStringLE());
                u->setUserInfoString("CompanyDepartment", msg.unpackShortStringLE());
                u->setUserInfoString("CompanyPosition", msg.unpackShortStringLE());
                u->setUserInfoUint("CompanyOccupation", msg.UnpackUnsignedShort());
                u->setUserInfoString("CompanyHomepage", msg.unpackShortStringLE());

          // save the user infomation
          u->SetEnableSave(true);
                u->save(Licq::User::SaveUserInfo);

          PushExtendedEvent(e);
          multipart = true;

                gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                    Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
                break;

        case ICQ_CMDxMETA_ABOUT:
              {
                gLog.info(tr("About info on %s (%s)."),
                    u->getAlias().c_str(), u->accountId().c_str());

          u->SetEnableSave(false);
                u->setUserInfoString("About",
                    gTranslator.returnToUnix(msg.unpackShortStringLE()));

          // save the user infomation
          u->SetEnableSave(true);
                u->save(Licq::User::SaveUserInfo);

          PushExtendedEvent(e);
          multipart = true;

                gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                    Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
                break;
              }
        case ICQ_CMDxMETA_INTERESTSxINFO:
        {
          unsigned i, n;

          gLog.info(tr("Personal Interests info on %s (%s)."),
                    u->getAlias().c_str(), u->accountId().c_str());

          u->SetEnableSave(false);
                u->getInterests().clear();
          n = msg.UnpackChar();

                for (i = 0; i < n; ++i)
                {
            unsigned short cat = msg.UnpackUnsignedShort();
                  u->getInterests()[cat] = msg.unpackShortStringLE();
                }

          // save the user infomation
          u->SetEnableSave(true);
                u->save(Licq::User::SaveUserInfo);

          PushExtendedEvent(e);
          multipart = true;

                gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                    Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
                break;
              }
        case ICQ_CMDxMETA_PASTxINFO:
        {
          // past background info - last one received
          unsigned i, n;

                gLog.info(tr("Organizations/Past Background info on %s (%s)."),
                    u->getAlias().c_str(), u->accountId().c_str());

          u->SetEnableSave(false);

                u->getBackgrounds().clear();

          n = msg.UnpackChar();

                for (i = 0; i < n; ++i)
                {
            unsigned short cat = msg.UnpackUnsignedShort();
                  u->getBackgrounds()[cat] = msg.unpackShortStringLE();
                }

          //---- Organizations
                u->getOrganizations().clear();
          n = msg.UnpackChar();

                for (i = 0; i < n; ++i)
          {
            unsigned short cat = msg.UnpackUnsignedShort();
                  u->getOrganizations()[cat] = msg.unpackShortStringLE();;
                }

          // our user info is now up to date
          u->SetOurClientTimestamp(u->ClientTimestamp());

          // save the user infomation
          u->SetEnableSave(true);
                u->save(Licq::User::SaveUserInfo | Licq::User::SaveLicqInfo);

                gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                    Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
                break;
              }
            default:
              packet.log(Log::Unknown, tr("Unknown info: %04hx"), nSubtype);
          }

        if (!multipart) {
          if (e) {
                DoneEvent(e, Licq::Event::ResultSuccess);
            ProcessDoneEvent(e);
          } else {
                gLog.warning(tr("Response to unknown extended info request for %s (%s)."),
                    u->getAlias().c_str(), userId.toString().c_str());
              }
            }

            gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                Licq::PluginSignal::SignalUser,
                Licq::PluginSignal::UserInfo, u->id()));
          }

          if (!type.empty())
          {
        if (pEvent)
            {
              if (pEvent->Result() == Licq::Event::ResultSuccess)
                gLog.info(tr("%s updated."), type.c_str());
              else
                gLog.info(tr("%s update failed."), type.c_str());
          ProcessDoneEvent(pEvent);
        }
            else
              gLog.info(tr("Unexpected result for %s."), type.c_str());
          }
          break;
        }
        default:
          packet.log(Log::Unknown, "Unknown SNAC 15,03 response type: %04hx",
                     nType);
      }
      break;
    }
    default:
      packet.log(Log::Unknown, tr("Unknown Various Family Subtype: %04hx"),
                 nSubtype);
  }
}

//--------ProcessAuthFam----------------------------------------------------
void IcqProtocol::ProcessAuthFam(Buffer& packet, unsigned short nSubtype)
{
  /*unsigned long Flags =*/ packet.UnpackUnsignedLongBE();
  unsigned short nSubSequence = packet.UnpackUnsignedShortBE();

  switch (nSubtype)
  {
    case ICQ_SNACxNEW_UIN_ERROR:
    {
      if (!myRegisterPasswd.empty())
      {
        gLog.warning(tr("Verification required. Reconnecting..."));

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
        gLog.error(tr("Unknown logon error. There appears to be an issue with the ICQ servers. Please try again later."));
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
        gLog.warning(tr("Received new uin (%lu) when already have a uin (%s)."),
            nNewUin, oldOwnerId.toString().c_str());
        return;
      }

      gLog.info(tr("Received new uin: %lu"), nNewUin);
      char szUin[14];
      snprintf(szUin, sizeof(szUin), "%lu", nNewUin);
      Licq::UserId ownerId(szUin, LICQ_PPID);
      Licq::gUserManager.addOwner(ownerId);

      {
        Licq::OwnerWriteGuard o(LICQ_PPID);
        if (o.isLocked())
        {
          o->setPassword(myRegisterPasswd);
          o->save(Licq::Owner::SaveOwnerInfo);
        }
      }
      myRegisterPasswd = "";

      gPluginManager.pushPluginSignal(new Licq::PluginSignal(
          Licq::PluginSignal::SignalNewOwner,
          0, Licq::UserId(), LICQ_PPID));

      // Reconnect now
      int nSD = m_nTCPSrvSocketDesc;
      m_nTCPSrvSocketDesc = -1;
      m_eStatus = STATUS_OFFLINE_MANUAL;
      m_bLoggingOn = false; 
      gSocketManager.CloseSocket(nSD);
      postLogoff(nSD, NULL);
      logon(Licq::User::OnlineStatus);
      break;
    }

    case ICQ_SNACxAUTHxSALT_REPLY:
    {
      string md5Salt = packet.unpackShortStringBE();
      CPU_NewLogon* p;
      {
        Licq::OwnerReadGuard o(LICQ_PPID);
        p = new CPU_NewLogon(o->password(), o->accountId(), md5Salt);
      }
      gLog.info(tr("Sending md5 hashed password."));
      SendEvent_Server(p);
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
        packet.log(Log::Unknown, tr("Unknown server response"));
        break;
      }

      string jpeg = packet.unpackTlvString(0x0002);

      // Save it in a file
      string filename = Licq::gDaemon.baseDir() + "Licq_verify.jpg";
      FILE* fp = fopen(filename.c_str(), "w");
      if (fp == 0)
      {
        gLog.warning(tr("Unable to open file (%s): %s."),
            filename.c_str(), strerror(errno));
        break;
      }

      fwrite(jpeg.c_str(), packet.getTLVLen(0x0002), 1, fp);
      fclose(fp);
      
      // Push a signal to the plugin to load the file
      gLog.info(tr("Received verification image."));
      gPluginManager.pushPluginSignal(new Licq::PluginSignal(
          Licq::PluginSignal::SignalVerifyImage,
          0, Licq::UserId(), LICQ_PPID));
      break;
    }

    default:
      packet.log(Log::Unknown, "Unknown New UIN Family Subtype: %04hx",
                 nSubtype);
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
      gLog.warning(tr("Empty User ID was received in the contact list."));
      continue;
    }

    bool isOnList = Licq::gUserManager.userExists(userId);
    int groupId = getGroupFromId(data->groupId);

    if (!isOnList)
    {
      Licq::gUserManager.addUser(userId, true, false, groupId); // Don't notify server
      gLog.info(tr("Added %s (%s) to list from server."),
          (!data->newAlias.empty() ? data->newAlias.c_str() : userId.toString().c_str()), userId.toString().c_str());
    }

    UserWriteGuard u(userId);
    if (!u.isLocked())
      continue;

    // For now, just save all the TLVs. We should change this to have awaiting auth check
    // for the 0x0066 TLV, SMS number if it has the 0x013A TLV, etc
    u->SetTLVList(data->tlvs);

    if (!u->m_bKeepAliasOnUpdate)
      u->setAlias(data->newAlias);

    u->SetSID(data->normalSid);
    u->SetGSID(data->groupId);
    u->SetVisibleSID(data->visibleSid);
    u->SetVisibleList(data->visibleSid != 0);
    u->SetInvisibleSID(data->invisibleSid);
    u->SetInvisibleList(data->invisibleSid != 0);
    u->SetIgnoreList(data->inIgnoreList);

    if (isOnList)
      u->addToGroup(groupId);

    u->SetAwaitingAuth(data->awaitingAuth);

    if (!isOnList)
    {
      // They aren't a new user if we added them to a server list
      u->SetNewUser(false);
    }

    u->setUserInfoString("CellularNumber", data->newCellular);

    // Save GSID, SID and group memberships
    u->save(Licq::User::SaveLicqInfo);
    gPluginManager.pushPluginSignal(new Licq::PluginSignal(
        Licq::PluginSignal::SignalUser,
        Licq::PluginSignal::UserBasic, u->id()));
    gPluginManager.pushPluginSignal(new Licq::PluginSignal(
        Licq::PluginSignal::SignalUser,
        Licq::PluginSignal::UserGroups, u->id()));
    gPluginManager.pushPluginSignal(new Licq::PluginSignal(
        Licq::PluginSignal::SignalUser,
        Licq::PluginSignal::UserSettings, u->id()));
  }

  receivedUserList.clear();
}

//--------ProcessDataChannel---------------------------------------------------

void IcqProtocol::ProcessDataChannel(Buffer& packet)
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

    case ICQ_SNACxFAM_STATS:
      processStatsFam(packet, nSubtype);
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
      gLog.warning(tr("Unknown Family on data channel: %04hx"), nFamily);
      break;
  }
}

//--------ProcessCloseChannel--------------------------------------------------

bool IcqProtocol::ProcessCloseChannel(Buffer& packet)
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
    gLog.info(tr("We're logging off.."));
    m_eStatus = STATUS_OFFLINE_FORCED;
    m_bLoggingOn = false;
    return true;
  }

  if (!packet.readTLV()) {
    gLog.error(tr("Error during parsing packet!"));
    return false;
  }

  unsigned short nError = packet.UnpackUnsignedShortTLV(0x0008);
  switch (nError) {
    case 0x1D:
    case 0x18:
      gLog.error(tr("Rate limit exceeded."));
      gPluginManager.pushPluginSignal(new Licq::PluginSignal(
          Licq::PluginSignal::SignalLogoff,
          Licq::PluginSignal::LogoffRate, Licq::gUserManager.ownerUserId(LICQ_PPID)));
      break;

    case 0x04:
    case 0x05:
      gLog.error(tr("Invalid UIN and password combination."));
      gPluginManager.pushPluginSignal(new Licq::PluginSignal(
          Licq::PluginSignal::SignalLogoff,
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
      gLog.error(tr("The requested network service is temporarily unavailable. "
          "Please try again later."));
      break;

  case 0:
    break;

    default:
      gLog.error(tr("Unknown sign on error: 0x%02X."), nError);
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
    {
      OwnerReadGuard o;
      if (o->reconnectAfterUinClash())
      {
        gLog.error(tr("Your ICQ number is used from another location."));
      m_eStatus = STATUS_OFFLINE_FORCED; // will try to reconnect
    }
    else 
    {
        gLog.error(tr("Your ICQ number is used from another location. "
            "Automatic reconnect is disabled."));
      m_eStatus = STATUS_OFFLINE_MANUAL; // don't reconnect
    }
      break;
    }
  case 0:
    break;

    default:
      gLog.error(tr("Unknown runtime error form server: 0x%02X."), nError);
    m_eStatus = STATUS_OFFLINE_FORCED;
  }

  if (nError)
  {
    m_bLoggingOn = false;
    return false;
  }

  string newServer = packet.unpackTlvString(0x0005);
  string cookie = packet.unpackTlvString(0x0006);
  int nCookieLen = packet.getTLVLen(0x0006);

  if (newServer.empty() || cookie.empty())
  {
    gLog.error(tr("Unable to sign on: NewServer: %s, cookie: %s."),
        newServer.c_str(), cookie.c_str());
    m_eStatus = STATUS_OFFLINE_FORCED;
    m_bLoggingOn = false;
    return false;
  }

  size_t pos = newServer.find(':');
  int port = 5190;
  if (pos != string::npos)
  {
    port = atoi(newServer.c_str() + pos + 1);
    newServer.resize(pos);
  }

  gLog.info(tr("Authenticated. Connecting to %s port %i."), newServer.c_str(), port);

  // Connect to the new server here and send our cookie
  ConnectToServer(newServer, port);

  // Send our cookie
  cookie.resize(nCookieLen);
  CPU_SendCookie *p = new CPU_SendCookie(cookie);
  SendEvent_Server(p);

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

  UserWriteGuard u(userId);
  if (!u.isLocked())
    return -1;


  CPU_ReverseConnect *p = new CPU_ReverseConnect(*u, nLocalIP, nLocalPort,
                                                 nRemotePort);
  int nId = p->SubSequence();

  pthread_mutex_lock(&mutex_reverseconnect);

  m_lReverseConnect.push_back(new CReverseConnectToUserData(userId.accountId().c_str(), nId, nData,
      nLocalIP, nLocalPort, ICQ_VERSION_TCP, nRemotePort, 0, nId));
  pthread_mutex_unlock(&mutex_reverseconnect);

  gLog.info(tr("Requesting reverse connection from %s."), u->getAlias().c_str());
  SendEvent_Server(p);

  return nId;
}
 
