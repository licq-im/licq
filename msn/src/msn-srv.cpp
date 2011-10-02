/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2011 Licq developers
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

#include "msn.h"
#include "msnpacket.h"
#include <licq/logging/log.h>

#include <boost/foreach.hpp>
#include <cassert>
#include <cstdio>
#include <string>
#include <list>
#include <unistd.h>
#include <vector>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/logging/logservice.h>
#include <licq/md5.h>
#include <licq/oneventmanager.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/socket.h>
#include <licq/userevents.h>

using namespace std;
using Licq::OnEventData;
using Licq::User;
using Licq::UserId;
using Licq::gLog;
using Licq::gOnEventManager;
using Licq::gUserManager;


void CMSN::ProcessServerPacket(CMSNBuffer *packet)
{
  char szCommand[4];
  CMSNPacket *pReply;
  
//while (!m_pPacketBuf->End())
  {
    pReply = 0;
    packet->UnpackRaw(szCommand, 3);
    string strCmd(szCommand);
    
    if (strCmd == "VER")
    {
      // Don't really care about this packet's data.
      pReply = new CPS_MSNClientVersion(m_szUserName);
    }
    else if (strCmd == "CVR")
    {
      // Don't really care about this packet's data.
      pReply = new CPS_MSNUser(m_szUserName);
    }
    else if (strCmd == "XFR")
    {
      //Time to transfer to a new server
      packet->SkipParameter(); // Seq
      string strServType = packet->GetParameter();
      string strServer = packet->GetParameter();
    
      if (strServType == "SB")
      {
        packet->SkipParameter(); // 'CKI'
        string strCookie = packet->GetParameter();
        
        MSNSBConnectStart(strServer, strCookie);
      }
      else
      {
        size_t sep = strServer.rfind(':');
        string host;
        int port = 0;
        if (sep != string::npos)
        {
          host = strServer.substr(0, sep);
          port = atoi(strServer.substr(sep+1).c_str());
        }

        gSocketMan.CloseSocket(m_nServerSocket, false, true);
  
        // Make the new connection
        Logon(myStatus, host.c_str(), port);
      }
    }
    else if (strCmd == "USR")
    {
      packet->SkipParameter(); // Seq
      string strType = packet->GetParameter();
      
      if (strType == "OK")
      {
        packet->SkipParameter(); // email account
        string strNick = packet->GetParameter();
        string strDecodedNick = Decode(strNick);
        gLog.info("%s logged in", strDecodedNick.c_str());
       
        // Set our alias here
        {
          Licq::OwnerWriteGuard o(MSN_PPID);
          o->setAlias(strDecodedNick);
        }

        // This cookie doesn't work anymore now that we are online
        if (m_szCookie)
        {
          free(m_szCookie);
          m_szCookie = 0;
        }

        pReply = new CPS_MSNSync(m_nListVersion);
      }
      else
      {
        packet->SkipParameter(); // "S"
        string strParam = packet->GetParameter();
      
        m_szCookie = strdup(strParam.c_str());

        //MSNGetServer();
        // Make an SSL connection to authenticate
        MSNAuthenticate(m_szCookie);
      }
    }
    else if (strCmd == "CHL")
    {
      packet->SkipParameter(); // Seq
      string strHash = packet->GetParameter();
      
      pReply = new CPS_MSNChallenge(strHash.c_str());
    }
    else if (strCmd == "SYN")
    {
      packet->SkipParameter();
      string strVersion = packet->GetParameter();
      m_nListVersion = atol(strVersion.c_str());
      saveConfig();

      MSNChangeStatus(myStatus);

      // Send our local list now
      //Licq::UserListGuard userList(MSN_PPID);
      //BOOST_FOREACH(const Licq::User* user, **userList)
      //{
      //  pReply = new CPS_MSNAddUser(user->accountId().c_str());
      //  SendPacket(pReply);
      //}
    }
    else if (strCmd == "LST")
    {
      // Add user
      string strUser = packet->GetParameter();
      string strNick = packet->GetParameter();
      string strLists = packet->GetParameter();
      string strUserLists;

      if (strUser == gUserManager.ownerUserId(MSN_PPID).accountId())
        return;

      int nLists = atoi(strLists.c_str());
      if (nLists & FLAG_CONTACT_LIST)
        strUserLists = packet->GetParameter();

      UserId userId(strUser, MSN_PPID);
      if (nLists & FLAG_CONTACT_LIST)
        gUserManager.addUser(userId, true, false);

      Licq::UserWriteGuard u(userId);
      if (u.isLocked())
      {
        u->SetEnableSave(false);
        u->setUserEncoding("UTF-8");
        u->SetInvisibleList(nLists & FLAG_BLOCK_LIST);
        
        if (!u->KeepAliasOnUpdate())
        {
          string strDecodedNick = Decode(strNick);
          u->setAlias(strDecodedNick);
          Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
              Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserBasic, u->id()));
        }
        u->setUserInfoString("Email1", strUser);
        string strURL = "http://members.msn.com/"+strUser;
        u->setUserInfoString("Homepage", strURL);
        u->SetNewUser(false);
        u->SetEnableSave(true);
        u->save(Licq::User::SaveLicqInfo);
        Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
            Licq::PluginSignal::SignalUser,
            Licq::PluginSignal::UserInfo, u->id()));
      }
    }
    else if (strCmd == "LSG")
    {
      // Add group
    }
    else if (strCmd == "ADD")
    {
      packet->SkipParameter(); // What's this?
      string strList = packet->GetParameter();
      string strVersion = packet->GetParameter();
      string strUser = packet->GetParameter();
      UserId userId(strUser, MSN_PPID);
      string strNick = packet->GetParameter();
      m_nListVersion = atol(strVersion.c_str());
      saveConfig();
      
      if (strList == "RL")
      {
        gLog.info("Authorization request from %s", strUser.c_str());

        Licq::UserEvent* e = new Licq::EventAuthRequest(userId,
            strNick.c_str(), "", "", "", "", time(0), 0);

        Licq::OwnerWriteGuard o(MSN_PPID);
        if (Licq::gDaemon.addUserEvent(*o, e))
        {
          e->AddToHistory(*o, true);
          gOnEventManager.performOnEvent(OnEventData::OnEventSysMsg, *o);
        }
      }
      else
      {
        gLog.info("Added %s to contact list", strUser.c_str());

        Licq::UserWriteGuard u(userId);
        if (u.isLocked())
        {
          if (!u->KeepAliasOnUpdate())
          {
            string strDecodedNick = Decode(strNick);
            u->setAlias(strDecodedNick);
          }
          Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
              Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserBasic, u->id()));
        }
      }
    }
    else if (strCmd == "REM")
    {      
      packet->SkipParameter(); // seq
      packet->SkipParameter(); // list
      string strVersion = packet->GetParameter();
      string strUser = packet->GetParameter();
      m_nListVersion = atol(strVersion.c_str());
      saveConfig();
      gLog.info("Removed %s from contact list", strUser.c_str()); 
    }
    else if (strCmd == "REA")
    {
      packet->SkipParameter(); // seq
      string strVersion = packet->GetParameter();
      string strUser = packet->GetParameter();
      string strNick = packet->GetParameter();
      
      m_nListVersion = atol(strVersion.c_str());
      saveConfig();
      if (strcmp(m_szUserName, strUser.c_str()) == 0)
      {
        Licq::OwnerWriteGuard o(MSN_PPID);
        string strDecodedNick = Decode(strNick);
        o->setAlias(strDecodedNick);
      }
      
      gLog.info("%s renamed successfully", strUser.c_str());
    }
    else if (strCmd == "CHG")
    {
      packet->SkipParameter(); // seq
      string strStatus = packet->GetParameter();
      unsigned status;

      if (strStatus == "NLN")
        status = User::OnlineStatus;
      else if (strStatus == "BSY")
        status = User::OnlineStatus | User::OccupiedStatus;
      else if (strStatus == "HDN")
        status = User::OnlineStatus | User::InvisibleStatus;
      else if (strStatus == "IDL")
        status = User::OnlineStatus | User::IdleStatus;
      else
        status = User::OnlineStatus | User::AwayStatus;

      gLog.info("Server says we are now: %s",
          User::statusToString(status, true, false).c_str());
      myStatus = status;

      Licq::OwnerWriteGuard o(MSN_PPID);
      if (o.isLocked())
        o->statusChanged(status);
    }
    else if (strCmd == "ILN" || strCmd == "NLN")
    {
      if (strCmd == "ILN")
        packet->SkipParameter(); // seq
      string strStatus = packet->GetParameter();
      string strUser = packet->GetParameter();
      string strNick = packet->GetParameter();
      string strClientId = packet->GetParameter();
      string strMSNObject = packet->GetParameter();
      string strDecodedObject = strMSNObject.size() ? Decode(strMSNObject) :"";

      unsigned status;
      if (strStatus == "NLN")
        status = User::OnlineStatus;
      else if (strStatus == "BSY")
        status = User::OnlineStatus | User::OccupiedStatus;
      else if (strStatus == "IDL")
        status = User::OnlineStatus | User::IdleStatus;
      else
        status = User::OnlineStatus | User::AwayStatus;

      Licq::UserWriteGuard u(UserId(strUser, MSN_PPID));
      if (u.isLocked())
      {
        u->SetSendServer(true); // no direct connections

        if (!u->KeepAliasOnUpdate())
        {
          string strDecodedNick = Decode(strNick);
          u->setAlias(strDecodedNick);
          Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
              Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserBasic, u->id()));
        }

	// Get the display picture here, so it can be shown with the notify
	if (strDecodedObject != u->GetPPField("MSNObject_DP"))
	{
	  u->SetPPField("MSNObject_DP", strDecodedObject);
	  if (strDecodedObject.size())
            MSNGetDisplayPicture(u->id(), strDecodedObject);
	}

        gLog.info("%s changed status (%s)", u->getAlias().c_str(), strStatus.c_str());
        u->statusChanged(status);
      }
    }
    else if (strCmd == "FLN")
    {
      UserId userId(packet->GetParameter(), MSN_PPID);

      {
        Licq::UserWriteGuard u(userId);
        if (u.isLocked())
        {
          gLog.info("%s logged off", u->getAlias().c_str());
          u->statusChanged(User::OfflineStatus);
        }
      }

      // Do we have a connection attempt to this user?
      StartList::iterator it;
      pthread_mutex_lock(&mutex_StartList);
      for (it = m_lStart.begin(); it != m_lStart.end(); it++)
      {
        if (*it && userId == (*it)->userId)
        {
          gLog.info("Removing connection attempt to %s", userId.toString().c_str());
//          SStartMessage *pStart = (*it);
          m_lStart.erase(it);
          break;
        }
      }
      pthread_mutex_unlock(&mutex_StartList);
    }
    else if (strCmd == "RNG")
    {
      string strSessionID = packet->GetParameter();
      string strServer = packet->GetParameter();
      packet->SkipParameter(); // 'CKI'
      string strCookie = packet->GetParameter();
      string strUser = packet->GetParameter();
      
      MSNSBConnectAnswer(strServer, strSessionID, strCookie, strUser);
    }
    else if (strCmd == "MSG")
    {
      packet->SkipParameter(); // 'Hotmail'
      packet->SkipParameter(); // 'Hotmail' again
      packet->SkipParameter(); // size
      packet->SkipRN(); // Skip \r\n
      packet->ParseHeaders();
      
      string strType = packet->GetValue("Content-Type");
      
      if (strType.find("text/x-msmsgsprofile") != string::npos)
      {
        m_strMSPAuth = packet->GetValue("MSPAuth");
        m_strSID = packet->GetValue("sid");
        m_strKV = packet->GetValue("kv");
        m_nSessionStart = time(0);

        // We might have another packet attached
        //packet->SkipRN();
      }
      else if (strType.find("text/x-msmsgsinitialemailnotification") != string::npos)
      {
        // Email alert when we sign in
        
        // Get the next part..
        packet->SkipRN();
        packet->ParseHeaders();
      }
      else if (strType.find("text/x-msmsgsemailnotification") != string::npos)
      {
        // Email we get while signed in
        
        // Get the next part..
        packet->SkipRN();
        packet->ParseHeaders();
        
        string strFrom = packet->GetValue("From");
        string strFromAddr = packet->GetValue("From-Addr");
        string strSubject = packet->GetValue("Subject");
        
        string strToHash = m_strMSPAuth + "9" + myPassword;
        unsigned char szDigest[16];
        char szHexOut[32];
        Licq::md5((const uint8_t*)strToHash.c_str(), strToHash.size(), szDigest);
        for (int i = 0; i < 16; i++)
          sprintf(&szHexOut[i*2], "%02x", szDigest[i]);
    
        gLog.info("New email from %s (%s)", strFrom.c_str(), strFromAddr.c_str());
        Licq::EventEmailAlert* pEmailAlert = new Licq::EventEmailAlert(strFrom.c_str(), m_szUserName,
          strFromAddr.c_str(), strSubject.c_str(), time(0), m_strMSPAuth.c_str(), m_strSID.c_str(),
          m_strKV.c_str(), packet->GetValue("id").c_str(),
          packet->GetValue("Post-URL").c_str(), packet->GetValue("Message-URL").c_str(),
          szHexOut, m_nSessionStart);

        Licq::OwnerWriteGuard o(MSN_PPID);
        if (Licq::gDaemon.addUserEvent(*o, pEmailAlert))
        {
          pEmailAlert->AddToHistory(*o, true);
          gOnEventManager.performOnEvent(OnEventData::OnEventSysMsg, *o);
        }
      }
    }
    else if (strCmd == "QNG")
    {
      m_bWaitingPingReply = false;
    }
    else if (strCmd == "913")
    {
      unsigned long nSeq = packet->GetParameterUnsignedLong();

      // Search pStart for this sequence, mark it as an error, send the
      // signals to the daemon and remove these item from the list.
      SStartMessage *pStart = 0;
      StartList::iterator it;
      pthread_mutex_lock(&mutex_StartList);
      for (it = m_lStart.begin(); it != m_lStart.end(); it++)
      {
        if ((*it)->m_nSeq == nSeq)
        {
          gLog.error("Cannot send messages while invisible");
          pStart = *it;
          pStart->m_pEvent->m_eResult = Licq::Event::ResultFailed;
          Licq::gPluginManager.pushPluginEvent(pStart->m_pEvent);
          m_lStart.erase(it);
          break; 
        }
      }     
      pthread_mutex_unlock(&mutex_StartList);
    }
    else if (strCmd == "GTC")
    {
    }
    else if (strCmd == "BLP")
    {
    }
    else if (strCmd == "PRP")
    {
    }    
    else if (strCmd == "QRY")
    {
      m_bCanPing = true;
    }
    else if (strCmd == "NOT")
    {
      // For the moment, skip the notification... consider it spam from MSN
      unsigned long nSize = packet->GetParameterUnsignedLong(); // size
      packet->SkipRN(); // Skip \r\n
      packet->Skip(nSize);
    }
    else
    {
      gLog.warning("Unhandled command (%s)", strCmd.c_str());
    }
    
    if (pReply)
      SendPacket(pReply);
  }
}

void CMSN::SendPacket(CMSNPacket *p)
{
  Licq::INetSocket* s = gSocketMan.FetchSocket(m_nServerSocket);
  Licq::SrvSocket* sock = static_cast<Licq::SrvSocket*>(s);
  assert(sock != NULL);
  if (!sock->SendRaw(p->getBuffer()))
    MSNLogoff(true);
  else
    gSocketMan.DropSocket(sock);
  
  delete p;
}

void CMSN::Logon(unsigned status, string host, int port)
{
  if (status == User::OfflineStatus)
    return;

  UserId myOwnerId;
  {
    Licq::OwnerReadGuard o(MSN_PPID);
    if (!o.isLocked())
    {
      gLog.error("No owner set");
      return;
    }
    m_szUserName = strdup(o->accountId().c_str());
    myOwnerId = o->id();
    myPassword = o->password();
    if (host.empty())
      host = o->serverHost();
    if (port == 0)
      port = o->serverPort();
  }

  Licq::SrvSocket* sock = new Licq::SrvSocket(myOwnerId);
  gLog.info("Server found at %s:%d", host.c_str(), port);

  if (!sock->connectTo(host, port))
  {
    gLog.info("Connect failed to %s", host.c_str());
    delete sock;
    return;
  }
  
  gSocketMan.AddSocket(sock);
  m_nServerSocket = sock->Descriptor();
  gSocketMan.DropSocket(sock);
  
  CMSNPacket *pHello = new CPS_MSNVersion();
  SendPacket(pHello);
  myStatus = status;
}

void CMSN::MSNChangeStatus(unsigned status)
{
  string msnStatus;
  if (status & User::InvisibleStatus)
  {
    msnStatus = "HDN";
    status = User::OnlineStatus | User::InvisibleStatus;
  }
  else if (status & User::FreeForChatStatus || status == User::OnlineStatus)
  {
    msnStatus = "NLN";
    status = User::OnlineStatus;
  }
  else if (status & (User::OccupiedStatus | User::DoNotDisturbStatus))
  {
    msnStatus = "BSY";
    status = User::OnlineStatus | User::OccupiedStatus;
  }
  else
  {
    msnStatus = "AWY";
    status = User::OnlineStatus | User::AwayStatus;
  }

  CMSNPacket* pSend = new CPS_MSNChangeStatus(msnStatus);
  SendPacket(pSend);
  myStatus = status;
}

void CMSN::MSNLogoff(bool bDisconnected)
{
  if (m_nServerSocket == -1) return;

  if (!bDisconnected)
  {
    CMSNPacket *pSend = new CPS_MSNLogoff();
    SendPacket(pSend);
  }

  myStatus = User::OfflineStatus;

  // Don't try to send any more pings
  m_bCanPing = false;

  // Close the server socket
  Licq::INetSocket* s = gSocketMan.FetchSocket(m_nServerSocket);
  int nSD = m_nServerSocket;
  m_nServerSocket = -1;
  gSocketMan.DropSocket(s);
  gSocketMan.CloseSocket(nSD);

  
  // Close user sockets and update the daemon
  {
    Licq::UserListGuard userList(MSN_PPID);
    BOOST_FOREACH(Licq::User* user, **userList)
    {
      Licq::UserWriteGuard u(user);
      if (u->normalSocketDesc() != -1)
      {
        gSocketMan.CloseSocket(u->normalSocketDesc(), false, true);
        u->clearAllSocketDesc();
      }
      if (u->isOnline())
        u->statusChanged(User::OfflineStatus);
    }
  }

  Licq::OwnerWriteGuard o(MSN_PPID);
  if (o.isLocked())
    o->statusChanged(Licq::User::OfflineStatus);
}

void CMSN::MSNAddUser(const UserId& userId)
{
  {
    Licq::UserWriteGuard u(userId);
    if (u.isLocked())
    {
      u->SetEnableSave(false);
      u->setUserEncoding("UTF-8");
      u->SetEnableSave(true);
      u->save(Licq::User::SaveLicqInfo);
    }
  }

  CMSNPacket* pSend = new CPS_MSNAddUser(userId.accountId().c_str(), CONTACT_LIST);
  SendPacket(pSend);
}

void CMSN::MSNRemoveUser(const UserId& userId)
{
  CMSNPacket* pSend = new CPS_MSNRemoveUser(userId.accountId().c_str(), CONTACT_LIST);
  SendPacket(pSend);
}

void CMSN::MSNRenameUser(const UserId& userId)
{
  string strNick;
  {
    Licq::UserReadGuard u(userId);
    if (!u.isLocked())
      return;
    strNick = u->getAlias();
  }

  string strEncodedNick = Encode(strNick);
  CMSNPacket* pSend = new CPS_MSNRenameUser(userId.accountId().c_str(), strEncodedNick.c_str());
  SendPacket(pSend);
}

void CMSN::MSNGrantAuth(const UserId& userId)
{
  CMSNPacket* pSend = new CPS_MSNAddUser(userId.accountId().c_str(), ALLOW_LIST);
  SendPacket(pSend);
}

void CMSN::MSNUpdateUser(const string& alias)
{
  string strEncodedNick = Encode(alias);
  CMSNPacket *pSend = new CPS_MSNRenameUser(m_szUserName, strEncodedNick.c_str());
  SendPacket(pSend);
}

void CMSN::MSNBlockUser(const UserId& userId)
{
  {
    Licq::UserWriteGuard u(userId);
    if (!u.isLocked())
      return;
    u->SetInvisibleList(true);
  }

  CMSNPacket* pRem = new CPS_MSNRemoveUser(userId.accountId().c_str(), ALLOW_LIST);
  gLog.info("Removing user %s from the allow list", userId.toString().c_str());
  SendPacket(pRem);
  CMSNPacket* pAdd = new CPS_MSNAddUser(userId.accountId().c_str(), BLOCK_LIST);
  gLog.info("Adding user %s to the block list", userId.toString().c_str());
  SendPacket(pAdd);
}

void CMSN::MSNUnblockUser(const UserId& userId)
{
  {
    Licq::UserWriteGuard u(userId);
    if (!u.isLocked())
      return;
    u->SetInvisibleList(false);
  }

  CMSNPacket* pRem = new CPS_MSNRemoveUser(userId.accountId().c_str(), BLOCK_LIST);
  gLog.info("Removing user %s from the block list", userId.toString().c_str());
  SendPacket(pRem);
  CMSNPacket* pAdd = new CPS_MSNAddUser(userId.accountId().c_str(), ALLOW_LIST);
  gLog.info("Adding user %s to the allow list", userId.toString().c_str());
  SendPacket(pAdd);
}

void CMSN::MSNGetDisplayPicture(const Licq::UserId& userId, const string &strMSNObject)
{
  // If we are invisible, this will result in an error, so don't allow it
  if (myStatus & User::InvisibleStatus)
    return;

  CMSNPacket *pGetMSNDP = new CPS_MSNInvitation(userId.accountId().c_str(),
						m_szUserName,
						const_cast<char *>(strMSNObject.c_str()));
  CMSNP2PPacket *p = (CMSNP2PPacket *)(pGetMSNDP);
  CMSNDataEvent *pDataResponse = new CMSNDataEvent(MSN_DP_EVENT,
      p->SessionId(), p->BaseId(), userId, m_szUserName, p->CallGUID(), this);
  WaitDataEvent(pDataResponse);
  gLog.info("Requesting %s's display picture", userId.toString().c_str());
  MSNSendInvitation(userId.accountId().c_str(), pGetMSNDP);
}


void CMSN::MSNPing()
{
  CMSNPacket *pSend = new CPS_MSNPing();
  SendPacket(pSend);
}

void *MSNPing_tep(void *p)
{
  CMSN *pMSN = (CMSN *)p;

  Licq::gLogService.createThreadLog("msn-ping");
  
  while (true)
  {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    
    if (pMSN->WaitingPingReply())
    {
      pthread_mutex_lock(&(pMSN->mutex_ServerSocket));
      gLog.info("Ping timeout, reconnecting...");
      pMSN->SetWaitingPingReply(false);
      unsigned status = pMSN->status();
      pMSN->MSNLogoff();
      pMSN->Logon(status);
      pthread_mutex_unlock(&(pMSN->mutex_ServerSocket));
    }
    else if (pMSN->CanSendPing())
    {
      pMSN->MSNPing();
      pMSN->SetWaitingPingReply(true);
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();

    struct timeval tv;
    tv.tv_sec = 60;
    tv.tv_usec = 0;
    select(0, 0, 0, 0, &tv);

    pthread_testcancel();
  }  
  
  return 0;
}
