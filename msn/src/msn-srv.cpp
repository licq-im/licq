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

#include <string>
#include <list>
#include <vector>

using namespace std;


void CMSN::ProcessServerPacket(CMSNBuffer &packet)
{
  char szCommand[4];
  CMSNPacket *pReply;
  
  // Build the entire packet
  if (!m_pPacketBuf)
    m_pPacketBuf = new CMSNBuffer(packet);
  else
    *m_pPacketBuf += packet;
    
  if (memcmp((void *)(&(m_pPacketBuf->getDataStart())[m_pPacketBuf->getDataSize() - 2]), "\x0D\x0A", 2))
    return;
  
  while (!m_pPacketBuf->End())
  {
    pReply = 0;
    m_pPacketBuf->UnpackRaw(szCommand, 3);
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
      m_pPacketBuf->SkipParameter(); // Seq
      string strServType = m_pPacketBuf->GetParameter();
      string strServer = m_pPacketBuf->GetParameter();
    
      if (strServType == "SB")
      {
        m_pPacketBuf->SkipParameter(); // 'CKI'
        string strCookie = m_pPacketBuf->GetParameter();
        
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
      m_pPacketBuf->SkipParameter(); // Seq
      string strType = m_pPacketBuf->GetParameter();
      
      if (strType == "OK")
      {
        m_pPacketBuf->SkipParameter(); // email account
        string strNick = m_pPacketBuf->GetParameter();
        gLog.Info("%s%s logged in.\n", L_MSNxSTR, strNick.c_str());
        
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
        m_pPacketBuf->SkipParameter(); // "S"
        string strParam = m_pPacketBuf->GetParameter();
      
        m_szCookie = strdup(strParam.c_str());

        //MSNGetServer();
        // Make an SSL connection to authenticate
        MSNAuthenticate(m_szCookie);
      }
    }
    else if (strCmd == "CHL")
    {
      m_pPacketBuf->SkipParameter(); // Seq
      string strHash = m_pPacketBuf->GetParameter();
      
      pReply = new CPS_MSNChallenge(strHash.c_str());
    }
    else if (strCmd == "SYN")
    {
      m_pPacketBuf->SkipParameter();
      string strVersion = m_pPacketBuf->GetParameter();
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
      string strUser = m_pPacketBuf->GetParameter();
      string strNick = m_pPacketBuf->GetParameter();
      string strLists = m_pPacketBuf->GetParameter();
      string strUserLists;

      int nLists = atoi(strLists.c_str());
      if (nLists & FLAG_CONTACT_LIST)
        strUserLists = m_pPacketBuf->GetParameter();
        
      if ((nLists & FLAG_CONTACT_LIST) &&
          !gUserManager.IsOnList(strUser.c_str(), MSN_PPID))
        m_pDaemon->AddUserToList(strUser.c_str(), MSN_PPID);

      ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
      if (u)
      {
        if (!u->KeepAliasOnUpdate())
        {
          string strDecodedNick = Decode(strNick);
          u->SetAlias(strDecodedNick.c_str());
          m_pDaemon->PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_GENERAL,
                            u->IdString(), u->PPID()));

        }
        u->SetEmailPrimary(strUser.c_str());
        u->SetNewUser(false);
        gUserManager.DropUser(u);
      }
    }
    else if (strCmd == "LSG")
    {
      // Add group
    }
    else if (strCmd == "ADD")
    {
      m_pPacketBuf->SkipParameter(); // What's this?
      string strList = m_pPacketBuf->GetParameter();
      string strVersion = m_pPacketBuf->GetParameter();
      string strUser = m_pPacketBuf->GetParameter();
      string strNick = m_pPacketBuf->GetParameter();
      m_nListVersion = atol(strVersion.c_str());
      
      if (strList == "RL")
      {
        gLog.Info("%sAuthorization request from %s.\n", L_MSNxSTR, strUser.c_str());
        
        CUserEvent *e = new CEventAuthRequest(strUser.c_str(), MSN_PPID,
          strNick.c_str(), "", "", "", "", ICQ_CMDxRCV_SYSxMSGxONLINE, time(0), 0);
      
        ICQOwner *o = gUserManager.FetchOwner(MSN_PPID, LOCK_W);
        if (m_pDaemon->AddUserEvent(o, e))
        {
          gUserManager.DropOwner(MSN_PPID);
          e->AddToHistory(NULL, D_RECEIVER);
          m_pDaemon->m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
        }
        else
          gUserManager.DropOwner(MSN_PPID);
      }
      else
      {
        gLog.Info("%sAdded %s to contact list.\n", L_MSNxSTR, strUser.c_str());
        
        ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
        if (u)
        {
          string strDecodedNick = Decode(strNick);
          u->SetAlias(strDecodedNick.c_str());
          m_pDaemon->PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_GENERAL,
                            u->IdString(), u->PPID()));
          gUserManager.DropUser(u);
        }
      }
    }
    else if (strCmd == "REM")
    {      
      m_pPacketBuf->SkipParameter(); // seq
      m_pPacketBuf->SkipParameter(); // list
      string strVersion = m_pPacketBuf->GetParameter();
      string strUser = m_pPacketBuf->GetParameter();
      m_nListVersion = atol(strVersion.c_str());
    
      gLog.Info("%sRemoved %s from contact list.\n", L_MSNxSTR, strUser.c_str()); 
    }
    else if (strCmd == "CHG")
    {
      m_pPacketBuf->SkipParameter(); // seq
      string strStatus = m_pPacketBuf->GetParameter();
      ICQUser *o = gUserManager.FetchOwner(MSN_PPID, LOCK_W);
      unsigned long nStatus;
      
      if (strStatus == "NLN")
        nStatus = ICQ_STATUS_ONLINE;
      else if (strStatus == "BSY")
        nStatus = ICQ_STATUS_DND;
      else if (strStatus == "HDN")
        nStatus = ICQ_STATUS_ONLINE | ICQ_STATUS_FxPRIVATE;
      else
        nStatus = ICQ_STATUS_AWAY;
        
      m_pDaemon->ChangeUserStatus(o, nStatus);
      m_nStatus = nStatus;
      gLog.Info("%sServer says we are now: %s\n", L_MSNxSTR, ICQUser::StatusToStatusStr(o->Status(), false));
      gUserManager.DropOwner(MSN_PPID);
    }
    else if (strCmd == "ILN" || strCmd == "NLN")
    {
      if (strCmd == "ILN")
        m_pPacketBuf->SkipParameter(); // seq
      string strStatus = m_pPacketBuf->GetParameter();
      string strUser = m_pPacketBuf->GetParameter();
      string strNick = m_pPacketBuf->GetParameter();

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
      string strUser = m_pPacketBuf->GetParameter();
      
      ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
      if (u)
      {
        gLog.Info("%s%s logged off.\n", L_MSNxSTR, u->GetAlias());
        m_pDaemon->ChangeUserStatus(u, ICQ_STATUS_OFFLINE);
      }
      gUserManager.DropUser(u);
    }
    else if (strCmd == "RNG")
    {
      string strSessionID = m_pPacketBuf->GetParameter();
      string strServer = m_pPacketBuf->GetParameter();
      m_pPacketBuf->SkipParameter(); // 'CKI'
      string strCookie = m_pPacketBuf->GetParameter();
      string strUser = m_pPacketBuf->GetParameter();
      
      MSNSBConnectAnswer(strServer, strSessionID, strCookie, strUser);
    }
    else if (strCmd == "MSG")
    {
      m_pPacketBuf->SkipParameter(); // 'Hotmail'
      m_pPacketBuf->SkipParameter(); // 'Hotmail' again
      m_pPacketBuf->SkipParameter(); // size
      m_pPacketBuf->SkipRN(); // Skip \r\n
      m_pPacketBuf->ParseHeaders();
      
      string strType = m_pPacketBuf->GetValue("Content-Type");
      
      if (strType.find("text/x-msmsgsprofile") != string::npos)
      {
        m_strMSPAuth = m_pPacketBuf->GetValue("MSPAuth");
        m_strSID = m_pPacketBuf->GetValue("sid");
        m_strKV = m_pPacketBuf->GetValue("kv");
        m_nSessionStart = time(0);
      }
      else if (strType.find("text/x-msmsgsinitialemailnotification") != string::npos)
      {
        // Email alert when we sign in
        
        // Get the next part..
        m_pPacketBuf->SkipRN();
        m_pPacketBuf->ParseHeaders();
      }
      else if (strType.find("text/x-msmsgsemailnotification") != string::npos)
      {
        // Email we get while signed in
        
        // Get the next part..
        m_pPacketBuf->SkipRN();
        m_pPacketBuf->ParseHeaders();
        
        string strFrom = m_pPacketBuf->GetValue("From");
        string strFromAddr = m_pPacketBuf->GetValue("From-Addr");
        string strSubject = m_pPacketBuf->GetValue("Subject");
        
        string strToHash = m_strMSPAuth + "9" + m_szPassword;
        unsigned char szDigest[16];
        char szHexOut[32];
        MD5((const unsigned char *)strToHash.c_str(), strToHash.size(), szDigest);
        for (int i = 0; i < 16; i++)
          sprintf(&szHexOut[i*2], "%02x", szDigest[i]);
    
        gLog.Info("%sNew email from %s (%s)\n", L_MSNxSTR, strFrom.c_str(), strFromAddr.c_str());
        CEventEmailAlert *pEmailAlert = new CEventEmailAlert(strFrom.c_str(), m_szUserName,
          strFromAddr.c_str(), strSubject.c_str(), time(0), m_strMSPAuth.c_str(), m_strSID.c_str(),
          m_strKV.c_str(), m_pPacketBuf->GetValue("id").c_str(),
          m_pPacketBuf->GetValue("Post-URL").c_str(), m_pPacketBuf->GetValue("Message-URL").c_str(),
          szHexOut, m_nSessionStart);
          
        ICQOwner *o = gUserManager.FetchOwner(MSN_PPID, LOCK_W);
        if (m_pDaemon->AddUserEvent(o, pEmailAlert))
        {
          gUserManager.DropOwner(MSN_PPID);
          pEmailAlert->AddToHistory(NULL, D_RECEIVER);
          m_pDaemon->m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
        }
        else
          gUserManager.DropOwner(MSN_PPID);
      }
    }
    
    // Get the next packet
    m_pPacketBuf->SkipPacket();
    
    if (pReply)
      SendPacket(pReply);
  }
  
  // Clear it out
  delete m_pPacketBuf;
  m_pPacketBuf = 0;
}

void CMSN::SendPacket(CMSNPacket *p)
{
  INetSocket *s = gSocketMan.FetchSocket(m_nServerSocket);
  SrvSocket *sock = static_cast<SrvSocket *>(s);
  sock->SendRaw(p->getBuffer());
  gSocketMan.DropSocket(sock);
  
  delete p;
}

void CMSN::MSNLogon(const char *_szServer, int _nPort, unsigned long _nStatus)
{
  if (_nStatus == ICQ_STATUS_OFFLINE)
    return;

  ICQOwner *o = gUserManager.FetchOwner(MSN_PPID, LOCK_R);
  if (!o)
  {
    gLog.Error("%sNo owner set.\n", L_MSNxSTR);
    return;
  }
  m_szUserName = strdup(o->IdString());
  m_szPassword = strdup(o->Password());
  gUserManager.DropOwner(MSN_PPID);
  
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

void CMSN::MSNLogoff()
{
  if (m_nServerSocket == -1) return;

  CMSNPacket *pSend = new CPS_MSNLogoff();
  SendPacket(pSend);
  m_nStatus = ICQ_STATUS_OFFLINE;
 
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
      gSocketMan.CloseSocket(pUser->SocketDesc(ICQ_CHNxNONE));
      pUser->ClearSocketDesc();
    }
    if (!pUser->StatusOffline())
      m_pDaemon->ChangeUserStatus(pUser, ICQ_STATUS_OFFLINE);
  }
  FOR_EACH_PROTO_USER_END
    
  ICQOwner *o = gUserManager.FetchOwner(MSN_PPID, LOCK_W);      
  m_pDaemon->ChangeUserStatus(o, ICQ_STATUS_OFFLINE);
  gUserManager.DropOwner(MSN_PPID);  
  //m_pDaemon->PushPluginSignal(new CICQSignal(SIGNAL_LOGOFF, 0, 0));   
}

void CMSN::MSNAddUser(char *szUser)
{
  CMSNPacket *pSend = new CPS_MSNAddUser(szUser, CONTACT_LIST);
  SendPacket(pSend);
}

void CMSN::MSNRemoveUser(char *szUser)
{
  CMSNPacket *pSend = new CPS_MSNRemoveUser(szUser, CONTACT_LIST);
  SendPacket(pSend);
}

void CMSN::MSNRenameUser(char *szUser)
{
  ICQUser *u = gUserManager.FetchUser(szUser, MSN_PPID, LOCK_R);
  if (!u) return;
  char *szNewNick = u->GetAlias();
  gUserManager.DropUser(u);

  string strNick(szNewNick);
  string strEncodedNick = Encode(strNick);
  CMSNPacket *pSend = new CPS_MSNRenameUser(szUser, strEncodedNick.c_str());
  SendPacket(pSend);
}

void CMSN::MSNGrantAuth(char *szUser)
{
  CMSNPacket *pSend = new CPS_MSNAddUser(szUser, ALLOW_LIST);
  SendPacket(pSend);
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
    
    if (pMSN->Connected())
      pMSN->MSNPing();

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();

    tv.tv_sec = 60;
    tv.tv_usec = 0;
    select(0, 0, 0, 0, &tv);

    pthread_testcancel();
  }  
  
  return 0;
}
