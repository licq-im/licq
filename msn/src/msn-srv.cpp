/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// written by Jon Keating <jon@licq.org>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "msn.h"
#include "msnpacket.h"
#include "licq_log.h"
#include "licq_message.h"

#include <openssl/md5.h>
#include <unistd.h>

#include <cassert>
#include <string>
#include <list>
#include <vector>

using namespace std;


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
        const char *szParam = strServer.c_str();
        char szNewServer[16];
        char *szPort;
        if ((szPort = strchr(szParam, ':')))
        {
          strncpy(szNewServer, szParam, szPort - szParam);
          szNewServer[szPort - szParam] = '\0';
          *szPort++ = '\0';
        }
        
        gSocketMan.CloseSocket(m_nServerSocket, false, true);
  
        // Make the new connection
        MSNLogon(szNewServer, atoi(szPort), m_nStatus);
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
        gLog.Info("%s%s logged in.\n", L_MSNxSTR, strDecodedNick.c_str());
       
        // Set our alias here
        ICQOwner *o = gUserManager.FetchOwner(MSN_PPID, LOCK_W);
        o->SetAlias(strDecodedNick.c_str());
        gUserManager.DropOwner(o);
         
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
      
      pReply = new CPS_MSNChangeStatus(m_nStatus);
      SendPacket(pReply);
      
      // Send our local list now
      //FOR_EACH_PROTO_USER_START(MSN_PPID, LOCK_R)
      //{
      //  pReply = new CPS_MSNAddUser(pUser->IdString());
      //  SendPacket(pReply);
      //}
      //FOR_EACH_PROTO_USER_END
      
      pReply = 0;
    }
    else if (strCmd == "LST")
    {
      // Add user
      string strUser = packet->GetParameter();
      string strNick = packet->GetParameter();
      string strLists = packet->GetParameter();
      string strUserLists;

      if (gUserManager.FindOwner(strUser.c_str(), MSN_PPID))
        return;

      int nLists = atoi(strLists.c_str());
      if (nLists & FLAG_CONTACT_LIST)
        strUserLists = packet->GetParameter();
        
      if ((nLists & FLAG_CONTACT_LIST) &&
          !gUserManager.IsOnList(strUser.c_str(), MSN_PPID))
        m_pDaemon->AddUserToList(strUser.c_str(), MSN_PPID);

      ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
      if (u)
      {
        u->SetEnableSave(false);
        u->SetUserEncoding("UTF-8"); 
        u->SetInvisibleList(nLists & FLAG_BLOCK_LIST);
        
        if (!u->KeepAliasOnUpdate())
        {
          string strDecodedNick = Decode(strNick);
          u->SetAlias(strDecodedNick.c_str());
          m_pDaemon->PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_GENERAL,
                            u->IdString(), u->PPID()));

        }
        u->SetEmailPrimary(strUser.c_str());
        string strURL = "http://members.msn.com/"+strUser;
        u->SetHomepage(strURL.c_str());
        u->SetNewUser(false);
        u->SetEnableSave(true);
        u->SaveLicqInfo();             
        gUserManager.DropUser(u);
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
      string strNick = packet->GetParameter();
      m_nListVersion = atol(strVersion.c_str());
      
      if (strList == "RL")
      {
        gLog.Info("%sAuthorization request from %s.\n", L_MSNxSTR, strUser.c_str());
        
        CUserEvent *e = new CEventAuthRequest(strUser.c_str(), MSN_PPID,
          strNick.c_str(), "", "", "", "", ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);
      
        ICQOwner *o = gUserManager.FetchOwner(MSN_PPID, LOCK_W);
        if (m_pDaemon->AddUserEvent(o, e))
        {
          gUserManager.DropOwner(o);
          e->AddToHistory(NULL, MSN_PPID, D_RECEIVER);
          m_pDaemon->m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
        }
        else
          gUserManager.DropOwner(o);
      }
      else
      {
        gLog.Info("%sAdded %s to contact list.\n", L_MSNxSTR, strUser.c_str());
        
        ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
        if (u)
        {
          if (!u->KeepAliasOnUpdate())
          {
            string strDecodedNick = Decode(strNick);
            u->SetAlias(strDecodedNick.c_str());
          }
          m_pDaemon->PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_GENERAL,
                            u->IdString(), u->PPID()));
          gUserManager.DropUser(u);
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
    
      gLog.Info("%sRemoved %s from contact list.\n", L_MSNxSTR, strUser.c_str()); 
    }
    else if (strCmd == "REA")
    {
      packet->SkipParameter(); // seq
      string strVersion = packet->GetParameter();
      string strUser = packet->GetParameter();
      string strNick = packet->GetParameter();
      
      m_nListVersion = atol(strVersion.c_str());
      if (strcmp(m_szUserName, strUser.c_str()) == 0)
      {
        ICQOwner *o = gUserManager.FetchOwner(MSN_PPID, LOCK_W);
        string strDecodedNick = Decode(strNick);
        o->SetAlias(strDecodedNick.c_str());
        gUserManager.DropOwner(o);
      }
      
      gLog.Info("%s%s renamed successfully.\n", L_MSNxSTR, strUser.c_str());
    }
    else if (strCmd == "CHG")
    {
      packet->SkipParameter(); // seq
      string strStatus = packet->GetParameter();
      ICQOwner* o = gUserManager.FetchOwner(MSN_PPID, LOCK_W);
      unsigned long nStatus;
      bool bHidden = false;
      
      if (strStatus == "NLN")
        nStatus = ICQ_STATUS_ONLINE;
      else if (strStatus == "BSY")
        nStatus = ICQ_STATUS_DND;
      else if (strStatus == "HDN")
      {
        nStatus = ICQ_STATUS_ONLINE | ICQ_STATUS_FxPRIVATE;
        bHidden = true;
      }
      else
        nStatus = ICQ_STATUS_AWAY;
        
      m_pDaemon->ChangeUserStatus(o, nStatus);
      m_nStatus = nStatus;
      gLog.Info("%sServer says we are now: %s\n", L_MSNxSTR, ICQUser::StatusToStatusStr(o->Status(), bHidden));
      gUserManager.DropOwner(o);
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

      unsigned short nStatus = ICQ_STATUS_AWAY;

      if (strStatus == "NLN")
        nStatus = ICQ_STATUS_ONLINE;
        
      ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
      if (u)
      {
        u->SetOnlineSince(time(NULL)); // Not in this protocol
        u->SetSendServer(true); // no direct connections

        if (!u->KeepAliasOnUpdate())
        {
          string strDecodedNick = Decode(strNick);
          u->SetAlias(strDecodedNick.c_str());
          m_pDaemon->PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_GENERAL,
                             u->IdString(), u->PPID()));

        }

	// Get the display picture here, so it can be shown with the notify
	if (strDecodedObject != u->GetPPField("MSNObject_DP"))
	{
	  u->SetPPField("MSNObject_DP", strDecodedObject);
	  if (strDecodedObject.size())
	  {
	    MSNGetDisplayPicture(u->IdString(), strDecodedObject);
	  }
	}

        gLog.Info("%s%s changed status (%s).\n", L_MSNxSTR, u->GetAlias(), strStatus.c_str());
        m_pDaemon->ChangeUserStatus(u, nStatus);

        if ((m_pDaemon->m_bAlwaysOnlineNotify || strCmd == "NLN") &&
            nStatus == ICQ_STATUS_ONLINE && u->OnlineNotify())
          m_pDaemon->m_xOnEventManager.Do(ON_EVENT_NOTIFY, u);
      }
      gUserManager.DropUser(u);
    }
    else if (strCmd == "FLN")
    {
      string strUser = packet->GetParameter();
      
      ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
      if (u)
      {
        gLog.Info("%s%s logged off.\n", L_MSNxSTR, u->GetAlias());
        m_pDaemon->ChangeUserStatus(u, ICQ_STATUS_OFFLINE);
      }
      gUserManager.DropUser(u);

      // Do we have a connection attempt to this user?
      StartList::iterator it;
      pthread_mutex_lock(&mutex_StartList);
      for (it = m_lStart.begin(); it != m_lStart.end(); it++)
      {
        if (*it && strcmp(strUser.c_str(), (*it)->m_szUser) == 0)
        {
          gLog.Info("%sRemoving connection attempt to %s.\n", L_MSNxSTR, strUser.c_str());
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
        
        string strToHash = m_strMSPAuth + "9" + m_szPassword;
        unsigned char szDigest[16];
        char szHexOut[32];
        MD5((const unsigned char *)strToHash.c_str(), strToHash.size(), szDigest);
        for (int i = 0; i < 16; i++)
          sprintf(&szHexOut[i*2], "%02x", szDigest[i]);
    
        gLog.Info("%sNew email from %s (%s)\n", L_MSNxSTR, strFrom.c_str(), strFromAddr.c_str());
        CEventEmailAlert *pEmailAlert = new CEventEmailAlert(strFrom.c_str(), m_szUserName,
          strFromAddr.c_str(), strSubject.c_str(), time(0), m_strMSPAuth.c_str(), m_strSID.c_str(),
          m_strKV.c_str(), packet->GetValue("id").c_str(),
          packet->GetValue("Post-URL").c_str(), packet->GetValue("Message-URL").c_str(),
          szHexOut, m_nSessionStart);
          
        ICQOwner *o = gUserManager.FetchOwner(MSN_PPID, LOCK_W);
        if (m_pDaemon->AddUserEvent(o, pEmailAlert))
        {
          gUserManager.DropOwner(o);
          pEmailAlert->AddToHistory(NULL, MSN_PPID, D_RECEIVER);
          m_pDaemon->m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
        }
        else
          gUserManager.DropOwner(o);
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
          gLog.Error("%sCannot send messages while invisible.\n", L_ERRORxSTR);
          pStart = *it;
          m_pDaemon->PushPluginSignal(pStart->m_pSignal);
          pStart->m_pEvent->m_eResult = EVENT_FAILED;
          m_pDaemon->PushPluginEvent(pStart->m_pEvent);
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
      gLog.Warn("%sUnhandled command (%s).\n", L_MSNxSTR, strCmd.c_str());
    }
    
    if (pReply)
      SendPacket(pReply);
  }
}

void CMSN::SendPacket(CMSNPacket *p)
{
  INetSocket *s = gSocketMan.FetchSocket(m_nServerSocket);
  SrvSocket *sock = static_cast<SrvSocket *>(s);
  assert(sock != NULL);
  if (!sock->SendRaw(p->getBuffer()))
    MSNLogoff(true);
  else
    gSocketMan.DropSocket(sock);
  
  delete p;
}

void CMSN::MSNLogon(const char *_szServer, int _nPort)
{
  MSNLogon(_szServer, _nPort, m_nOldStatus);
}

void CMSN::MSNLogon(const char *_szServer, int _nPort, unsigned long _nStatus)
{
  if (_nStatus == ICQ_STATUS_OFFLINE)
    return;

  const ICQOwner* o = gUserManager.FetchOwner(MSN_PPID, LOCK_R);
  if (!o)
  {
    gLog.Error("%sNo owner set.\n", L_MSNxSTR);
    return;
  }
  m_szUserName = strdup(o->IdString());
  m_szPassword = strdup(o->Password());
  gUserManager.DropOwner(o);

  SrvSocket *sock = new SrvSocket(m_szUserName, MSN_PPID);
  sock->SetRemoteAddr(_szServer, _nPort);
  char ipbuf[32];
  gLog.Info("%sServer found at %s:%d.\n", L_MSNxSTR, sock->RemoteIpStr(ipbuf), sock->RemotePort());
  
  if (!sock->OpenConnection())
  {
    gLog.Info("%sConnect failed.\n", L_MSNxSTR);
    delete sock;
    return;
  }
  
  gSocketMan.AddSocket(sock);
  m_nServerSocket = sock->Descriptor();
  gSocketMan.DropSocket(sock);
  
  CMSNPacket *pHello = new CPS_MSNVersion();
  SendPacket(pHello);
  m_nStatus = _nStatus;
}

void CMSN::MSNChangeStatus(unsigned long _nStatus)
{
  CMSNPacket *pSend = new CPS_MSNChangeStatus(_nStatus);
  SendPacket(pSend);
  m_nStatus = _nStatus;
}

void CMSN::MSNLogoff(bool bDisconnected)
{
  if (m_nServerSocket == -1) return;

  if (!bDisconnected)
  {
    CMSNPacket *pSend = new CPS_MSNLogoff();
    SendPacket(pSend);
  }
  
  m_nOldStatus = m_nStatus;
  m_nStatus = ICQ_STATUS_OFFLINE;
 
  // Don't try to send any more pings
  m_bCanPing = false;

  // Close the server socket
  INetSocket *s = gSocketMan.FetchSocket(m_nServerSocket);
  int nSD = m_nServerSocket;
  m_nServerSocket = -1;
  gSocketMan.DropSocket(s);
  gSocketMan.CloseSocket(nSD);

  
  // Close user sockets and update the daemon
  FOR_EACH_PROTO_USER_START(MSN_PPID, LOCK_W)
  {
    if (pUser->SocketDesc(ICQ_CHNxNONE) != -1)
    {
      gSocketMan.CloseSocket(pUser->SocketDesc(ICQ_CHNxNONE), false, true);
      pUser->ClearSocketDesc();
    }
    if (!pUser->StatusOffline())
      m_pDaemon->ChangeUserStatus(pUser, ICQ_STATUS_OFFLINE);
  }
  FOR_EACH_PROTO_USER_END
    
  ICQOwner *o = gUserManager.FetchOwner(MSN_PPID, LOCK_W);      
  m_pDaemon->ChangeUserStatus(o, ICQ_STATUS_OFFLINE);
  gUserManager.DropOwner(o);
  //m_pDaemon->PushPluginSignal(new CICQSignal(SIGNAL_LOGOFF, 0, 0));   
}

void CMSN::MSNAddUser(const char* szUser)
{
  ICQUser *u = gUserManager.FetchUser(szUser, MSN_PPID, LOCK_W);
  u->SetEnableSave(false);
  u->SetUserEncoding("UTF-8");
  u->SetEnableSave(true);
  u->SaveLicqInfo();       
  gUserManager.DropUser(u);
  
  CMSNPacket *pSend = new CPS_MSNAddUser(szUser, CONTACT_LIST);
  SendPacket(pSend);
}

void CMSN::MSNRemoveUser(const char* szUser)
{
  CMSNPacket *pSend = new CPS_MSNRemoveUser(szUser, CONTACT_LIST);
  SendPacket(pSend);
}

void CMSN::MSNRenameUser(const char* szUser)
{
  const ICQUser* u = gUserManager.FetchUser(szUser, MSN_PPID, LOCK_R);
  if (!u) return;
  string strNick = u->GetAlias();
  gUserManager.DropUser(u);

  string strEncodedNick = Encode(strNick);
  CMSNPacket *pSend = new CPS_MSNRenameUser(szUser, strEncodedNick.c_str());
  SendPacket(pSend);
}

void CMSN::MSNGrantAuth(const char* szUser)
{
  CMSNPacket *pSend = new CPS_MSNAddUser(szUser, ALLOW_LIST);
  SendPacket(pSend);
}

void CMSN::MSNUpdateUser(const char* szAlias)
{
  string strNick(szAlias);
  string strEncodedNick = Encode(strNick);
  CMSNPacket *pSend = new CPS_MSNRenameUser(m_szUserName, strEncodedNick.c_str());
  SendPacket(pSend);
}

void CMSN::MSNBlockUser(const char* szUser)
{
  ICQUser *u = gUserManager.FetchUser(szUser, MSN_PPID, LOCK_W);
  if (u)
  {
    u->SetInvisibleList(true);
    gUserManager.DropUser(u);
  }
  else
    return;
    
  CMSNPacket *pRem = new CPS_MSNRemoveUser(szUser, ALLOW_LIST);
  gLog.Info("%sRemoving user %s from the allow list.\n", L_MSNxSTR, szUser);
  SendPacket(pRem);
  CMSNPacket *pAdd = new CPS_MSNAddUser(szUser, BLOCK_LIST);
  gLog.Info("%sAdding user %s to the block list.\n", L_MSNxSTR, szUser);
  SendPacket(pAdd);
}

void CMSN::MSNUnblockUser(const char* szUser)
{
  ICQUser *u = gUserManager.FetchUser(szUser, MSN_PPID, LOCK_W);
  if (u)
  {
    u->SetInvisibleList(false);
    gUserManager.DropUser(u);
  }
  else
    return;
    
  CMSNPacket *pRem = new CPS_MSNRemoveUser(szUser, BLOCK_LIST);
  gLog.Info("%sRemoving user %s from the block list\n", L_MSNxSTR, szUser);
  SendPacket(pRem);
  CMSNPacket *pAdd = new CPS_MSNAddUser(szUser, ALLOW_LIST);
  gLog.Info("%sAdding user %s to the allow list.\n", L_MSNxSTR, szUser);
  SendPacket(pAdd);
}

void CMSN::MSNGetDisplayPicture(const string &strUser, const string &strMSNObject)
{
  // If we are invisible, this will result in an error, so don't allow it
  if (m_nStatus & ICQ_STATUS_FxPRIVATE)
    return;

  const char *szUser = const_cast<const char *>(strUser.c_str());
  CMSNPacket *pGetMSNDP = new CPS_MSNInvitation(szUser,
						m_szUserName,
						const_cast<char *>(strMSNObject.c_str()));
  CMSNP2PPacket *p = (CMSNP2PPacket *)(pGetMSNDP);
  CMSNDataEvent *pDataResponse = new CMSNDataEvent(MSN_DP_EVENT,
						   p->SessionId(), p->BaseId(),  strUser, 
                                                   m_szUserName, p->CallGUID(), this);
  WaitDataEvent(pDataResponse);
  gLog.Info("%sRequesting %s's display picture.\n", L_MSNxSTR, szUser);
  MSNSendInvitation(szUser, pGetMSNDP);
}


void CMSN::MSNPing()
{
  CMSNPacket *pSend = new CPS_MSNPing();
  SendPacket(pSend);
}

void *MSNPing_tep(void *p)
{
  pthread_detach(pthread_self());
  
  CMSN *pMSN = (CMSN *)p;
  
  struct timeval tv;

  while (true)
  {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    
    if (pMSN->WaitingPingReply())
    {
      pthread_mutex_lock(&(pMSN->mutex_ServerSocket));
      gLog.Info("%sPing timeout. Reconnecting...\n", L_MSNxSTR);
      pMSN->SetWaitingPingReply(false);
      pMSN->MSNLogoff();
      pMSN->MSNLogon(pMSN->serverAddress().c_str(), pMSN->serverPort());
      pthread_mutex_unlock(&(pMSN->mutex_ServerSocket));
    }
    else if (pMSN->CanSendPing())
    {
      pMSN->MSNPing();
      pMSN->SetWaitingPingReply(true);
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();

    tv.tv_sec = 60;
    tv.tv_usec = 0;
    select(0, 0, 0, 0, &tv);

    pthread_testcancel();
  }  
  
  return 0;
}
