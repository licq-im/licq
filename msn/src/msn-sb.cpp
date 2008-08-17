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
#include "licq_translate.h"


#include <openssl/md5.h>

#include <string>
#include <list>
#include <vector>

using namespace std;

void CMSN::ProcessSBPacket(char *szUser, CMSNBuffer *packet, int nSock)
{
  char szCommand[4];
  CMSNPacket *pReply;
  bool bSkipPacket;

  //while (!packet->End())
  {
    pReply = 0;
    bSkipPacket = true;
    packet->UnpackRaw(szCommand, 3);
    string strCmd(szCommand);
 
    if (strCmd == "IRO")
    {
      packet->SkipParameter(); // Seq
      packet->SkipParameter(); // current user to add
      packet->SkipParameter(); // total users in conversation
      string strUser = packet->GetParameter();

      ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_R);
      if (!u)
      {
        m_pDaemon->AddUserToList(strUser.c_str(), MSN_PPID, false, true);
        
        // MSN uses UTF-8 so we need to set this for all new users automatically
        ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
        if (u) // To be safe
        {
          u->SetEnableSave(false);
          u->SetUserEncoding("UTF-8");
          u->SetEnableSave(true);
          u->SaveLicqInfo();
        }
      }
      
      if (u)
        gUserManager.DropUser(u);

      // Add the user to the conversation
      if (!m_pDaemon->FindConversation(nSock))
        m_pDaemon->AddConversation(nSock, MSN_PPID);  
      m_pDaemon->AddUserConversation(nSock, strUser.c_str());
            
      // Notify the plugins of the new CID
      m_pDaemon->PushPluginSignal(new CICQSignal(SIGNAL_SOCKET, 0,
        strdup(strUser.c_str()), MSN_PPID, 0, SocketToCID(nSock)));

      m_pDaemon->PushPluginSignal(new
        CICQSignal(SIGNAL_CONVOxJOIN, 0, strdup(strUser.c_str()), MSN_PPID, 0, SocketToCID(nSock)));
        
      gLog.Info("%s%s joined the conversation.\n", L_MSNxSTR, strUser.c_str());
    }
    else if (strCmd == "ANS")
    {
      // just OK, ready to talk
      // we can ignore this
    }
    else if (strCmd == "MSG")
    {
      string strUser = packet->GetParameter();
      packet->SkipParameter(); // Nick
      string strSize = packet->GetParameter(); // Size
      int nSize = atoi(strSize.c_str()) + 1; // Make up for the \n
      unsigned long nBeforeParse = packet->getDataPosRead() - packet->getDataStart();
      packet->SkipPacket(); // Skip \r\n
      packet->ParseHeaders();
      unsigned long nAfterParse = packet->getDataPosRead() - packet->getDataStart();
      int nRead = nAfterParse - nBeforeParse;
      nSize -= nRead;

      string strType = packet->GetValue("Content-Type");
      if (strType == "text/x-msmsgscontrol")
      {
        packet->SkipRN();
        packet->SkipRN();
        packet->SkipRN();
        ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
        if (u)
        {
          u->SetTyping(ICQ_TYPING_ACTIVE);
          gUserManager.DropUser(u);
          m_pDaemon->PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
            USER_TYPING, strUser.c_str(), MSN_PPID, SocketToCID(nSock)));
        }
      }
      else if (strncmp(strType.c_str(), "text/plain", 10) == 0)
      {
        gLog.Info("%sMessage from %s.\n", L_MSNxSTR, strUser.c_str());

        bSkipPacket = false;  
        char szMsg[nSize + 1];
        int i;
        for (i = 0; i < nSize; i++)
          (*packet) >> szMsg[i];
        szMsg[i] = '\0';
        
        CEventMsg *e = CEventMsg::Parse(szMsg, ICQ_CMDxRCV_SYSxMSGxOFFLINE, time(0), 0, SocketToCID(nSock));
        ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
        if (u)
          u->SetTyping(0);
        if (m_pDaemon->AddUserEvent(u, e))
          m_pDaemon->m_xOnEventManager.Do(ON_EVENT_MSG, u);
        gUserManager.DropUser(u);
      }
      else if (strncmp(strType.c_str(), "text/x-msmsgsinvite", 19) == 0)
      {
        packet->SkipRN();
        packet->ParseHeaders();

        string application = packet->GetValue("Application-Name");
        string cookie = packet->GetValue("Invitation-Cookie");
        string command = packet->GetValue("Invitation-Command");

        if (command == "INVITE")
        {
          // Invitation for unknown application, tell inviter that we don't have it
          gLog.Info("%sInvitation from %s for unknown application (%s).\n", L_MSNxSTR, strUser.c_str(), application.c_str());
          pReply = new CPS_MSNCancelInvite(cookie, "REJECT_NOT_INSTALLED");
        }
      }
      else if (strncmp(strType.c_str(), "application/x-msnmsgrp2p", 24) == 0)
      {
	// Get the binary header
	/*
	  unsigned long nSessionID = packet->UnpackUnsignedLong();
	unsigned long nIdentifier = packet->UnpackUnsignedLong();
	unsigned long nOffset[2], nSize[2], nAckDataSize[2];
	nOffset[0] = packet->UnpackUnsignedLong();
	nOffset[1] = packet->UnpackUnsignedLong();
	nSize[0] = packet->UnpackUnsignedLong();
	nSize[1] = packet->UnpackUnsignedLong();
	unsigned long nLen = packet->UnpackUnsignedLong();
	unsigned long nFlag = packet->UnpackUnsignedLong();
	unsigned long nAckID = packet->UnpackUnsignedLong();
	unsigned long nAckUniqueID = packet->UnpackUnsignedLong();
	nAckDataSize[0] = packet->UnpackUnsignedLong();
	nAckDataSize[1] = packet->UnpackUnsignedLong();

	printf("%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
	      nSessionID, nIdentifier, nOffset[0], nOffset[1], nSize[0],
	      nSize[1], nLen, nFlag, nAckID, nAckUniqueID,
	      nAckDataSize[0], nAckDataSize[1]);
	*/

	CMSNDataEvent *p = FetchDataEvent(strUser, nSock);
	if (p)
	{
	  if (p->ProcessPacket(packet) > 0)
	  {
	    RemoveDataEvent(p);
	  }
	}
      }
      else
      {
        gLog.Info("%sMessage from %s with unknown content type (%s).\n", L_MSNxSTR, strUser.c_str(), strType.c_str());
      }
    }
    else if (strCmd == "ACK")
    {
      string strId = packet->GetParameter();
      unsigned long nSeq = (unsigned long)atoi(strId.c_str());
      ICQEvent *e = RetrieveEvent(nSeq);
      if (e)
      {
        e->m_eResult = EVENT_ACKED;
        if (e->m_pUserEvent)
        {
          CConversation *pConv = m_pDaemon->FindConversation(nSock);
          if (pConv)
          {
            for (int i = 0; i < pConv->NumUsers(); i++)
            {
              string strUser = pConv->GetUser(i);
              ICQUser* u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
              if (u)
              {
                e->m_pUserEvent->AddToHistory(u, MSN_PPID, D_SENDER);
                u->SetLastSentEvent();
                if (strcmp(u->IdString(), e->m_szId) == 0)
                  m_pDaemon->m_xOnEventManager.Do(ON_EVENT_MSGSENT, u);
                gUserManager.DropUser(u);
              }
            }
          }
          else
          {
            ICQUser* u = gUserManager.FetchUser(e->m_szId, e->m_nPPID, LOCK_W);
            if (u != NULL)
            {
              e->m_pUserEvent->AddToHistory(u, MSN_PPID, D_SENDER);
              u->SetLastSentEvent();
              m_pDaemon->m_xOnEventManager.Do(ON_EVENT_MSGSENT, u);
              gUserManager.DropUser(u);
            }
          }
          m_pDaemon->m_sStats[STATS_EventsSent].Inc();
        }
	m_pDaemon->PushPluginEvent(e);
      }
      else
      {
	// P2P response
	//packet->SkipRN();
      }
    }
    else if (strCmd == "USR")
    {
      SStartMessage *pStart = 0;
      pthread_mutex_lock(&mutex_StartList);
      StartList::iterator it;
      for (it = m_lStart.begin(); it != m_lStart.end(); it++)
      {
	if ((*it)->m_bConnecting == true)
	  continue;
	pStart = *it;
	break;
      }
      
      if (pStart)
      {
	pStart->m_bConnecting = true;
	pReply = new CPS_MSNCall(pStart->m_szUser);
	pStart->m_nSeq = pReply->Sequence();
      }
      pthread_mutex_unlock(&mutex_StartList);
    }
    else if (strCmd == "JOI")
    {
      string strUser = packet->GetParameter();
      gLog.Info("%s%s joined the conversation.\n", L_MSNxSTR, strUser.c_str());

      SStartMessage *pStart = 0;
      StartList::iterator it;
      pthread_mutex_lock(&mutex_StartList);
      for (it = m_lStart.begin(); it != m_lStart.end(); it++)
      {
        if (strcmp((*it)->m_szUser, strUser.c_str()) == 0) // case insensitive perhaps?
        {
          pStart = *it;
          m_lStart.erase(it);
          break;
        }
      }
      
      if ((pStart && pStart->m_bDataConnection == false) || pStart == 0)
      {
        // Add the user to the conversation
        if (!m_pDaemon->FindConversation(nSock))
          m_pDaemon->AddConversation(nSock, MSN_PPID);

        m_pDaemon->AddUserConversation(nSock, strUser.c_str());

        // Notify the plugins of the new CID
        m_pDaemon->PushPluginSignal(new CICQSignal(SIGNAL_SOCKET, 0,
          strdup(strUser.c_str()), MSN_PPID, 0, SocketToCID(nSock)));

        // Notify the plugins
        m_pDaemon->PushPluginSignal(new
          CICQSignal(SIGNAL_CONVOxJOIN, 0, strdup(strUser.c_str()), MSN_PPID, 0, SocketToCID(nSock)));
      }

      if (pStart)
      {
        if (pStart->m_pEvent)
          m_pEvents.push_back(pStart->m_pEvent);
        if (pStart->m_pSignal)
          m_pDaemon->PushPluginSignal(pStart->m_pSignal);
        
        string strUser(pStart->m_szUser);
        Send_SB_Packet(strUser, pStart->m_pPacket, nSock, false);
        
        free(pStart->m_szUser);
        delete pStart;
      }
      pthread_mutex_unlock(&mutex_StartList);
    }
    else if (strCmd == "BYE")
    {
      // closed the window and connection
      string strUser = packet->GetParameter();
      gLog.Info("%sConnection with %s closed.\n", L_MSNxSTR, strUser.c_str());
      
      m_pDaemon->PushPluginSignal(new
        CICQSignal(SIGNAL_CONVOxLEAVE, 0, strdup(strUser.c_str()), MSN_PPID, 0, SocketToCID(nSock)));

      m_pDaemon->RemoveUserConversation(nSock, strUser.c_str());
      CConversation *pConv = m_pDaemon->FindConversation(nSock);
    
      ICQUser *u = gUserManager.FetchUser(const_cast<char *>(strUser.c_str()), MSN_PPID, LOCK_W);
      if (u)
      {
        u->ClearSocketDesc(ICQ_CHNxNONE);
        if (pConv && !pConv->IsEmpty())
        {
          gUserManager.DropUser(u);
        }
        else
        {
          int nThisSock = u->SocketDesc(ICQ_CHNxNONE);
          gUserManager.DropUser(u);
          INetSocket *s = gSocketMan.FetchSocket(nThisSock);
          gSocketMan.DropSocket(s);  
          gSocketMan.CloseSocket(nSock);
          if (pConv)
            m_pDaemon->RemoveConversation(pConv->CID());
        }
      }
    }
    else if (strCmd == "217")
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
          gLog.Error("%sUser not online.\n", L_ERRORxSTR);
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
    else
    {
      gLog.Warn("%sUnhandled command (%s).\n", L_MSNxSTR, strCmd.c_str());
    }
  
    // Get the next packet
    if (bSkipPacket)
      packet->SkipPacket();
    
    if (pReply)
    {
      string strTo(szUser);
      Send_SB_Packet(strTo, pReply, nSock);
    }
  }
  
  //delete packet;
}

void CMSN::Send_SB_Packet(const string &strUser, CMSNPacket *p, int nSocket, bool bDelete)
{
  const ICQUser* u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_R);
  if (!u) return;

  int nSock = nSocket != -1 ? nSocket : u->SocketDesc(ICQ_CHNxNONE);
  gUserManager.DropUser(u);  
  INetSocket *s = gSocketMan.FetchSocket(nSock);
  if (!s)
    s = gSocketMan.FetchSocket(nSocket);
  if (!s) return;
  TCPSocket *sock = static_cast<TCPSocket *>(s);
  if (!sock->SendRaw(p->getBuffer()))
  {
    gLog.Info("%sConnection with %s lost.\n", L_MSNxSTR, strUser.c_str());

    m_pDaemon->PushPluginSignal(new
      CICQSignal(SIGNAL_CONVOxLEAVE, 0, strdup(strUser.c_str()), MSN_PPID, 0, SocketToCID(nSock)));

    m_pDaemon->RemoveUserConversation(nSock, strUser.c_str());
    CConversation *pConv = m_pDaemon->FindConversation(nSock);

    ICQUser* u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
    if (u)
    {
      u->ClearSocketDesc(ICQ_CHNxNONE);
      if (!pConv->IsEmpty())
      {
        gUserManager.DropUser(u);
      }
      else
      {
//        int nThisSock = u->SocketDesc(ICQ_CHNxNONE);
        gUserManager.DropUser(u);
        gSocketMan.DropSocket(s);
        gSocketMan.CloseSocket(nSock);
        if (pConv)
          m_pDaemon->RemoveConversation(pConv->CID());
      }
    }
  }
  else
    gSocketMan.DropSocket(sock);
  
  if (bDelete)
    delete p;
}

bool CMSN::MSNSBConnectStart(const string &strServer, const string &strCookie)
{
  const char *szParam = strServer.c_str();
  char szServer[16];
  char *szPort;
  if ((szPort = strchr(szParam, ':')))
  {
    strncpy(szServer, szParam, szPort - szParam);
    szServer[szPort - szParam] = '\0';
    *szPort++ = '\0';
  }

  SStartMessage *pStart = 0;  
  pthread_mutex_lock(&mutex_StartList);
  StartList::iterator it;
  for (it = m_lStart.begin(); it != m_lStart.end(); it++)
  {
    if ((*it)->m_bConnecting == true)
      continue;
    pStart = *it;
    break;
  }
  if (!pStart)
  {
    pthread_mutex_unlock(&mutex_StartList);
    return false;
  }
  //pStart->m_bConnecting = true;
  TCPSocket *sock = new TCPSocket(pStart->m_szUser, MSN_PPID);
  pthread_mutex_unlock(&mutex_StartList);

  sock->SetRemoteAddr(szServer, atoi(szPort));
  char ipbuf[32];
  gLog.Info("%sConnecting to SB at %s:%d.\n", L_MSNxSTR, sock->RemoteIpStr(ipbuf),
    sock->RemotePort());
  
  if (!sock->OpenConnection())
  {
    gLog.Error("%sConnection to SB at %s failed.\n", L_MSNxSTR, sock->RemoteIpStr(ipbuf));
    delete sock;
    return false;
  }
  
  gSocketMan.AddSocket(sock);
  int nSocket = sock->Descriptor();
  ICQUser *u = gUserManager.FetchUser(pStart->m_szUser, MSN_PPID, LOCK_W);
  if (u)
  {
    if (pStart->m_bDataConnection)
      sock->SetChannel(ICQ_CHNxINFO);
    u->SetSocketDesc(sock);
    gUserManager.DropUser(u);
  }
  gSocketMan.DropSocket(sock);
  
  CMSNPacket *pReply = new CPS_MSN_SBStart(strCookie.c_str(), m_szUserName);
  string strUser(pStart->m_szUser);
  Send_SB_Packet(strUser, pReply, nSocket);  

  return true;
}

bool CMSN::MSNSBConnectAnswer(const string& strServer, const string& strSessionId,
    const string& strCookie, const string& strUser)
{
  const char *szParam = strServer.c_str();
  char szServer[16];
  char *szPort;
  if ((szPort = strchr(szParam, ':')))
  {
    strncpy(szServer, szParam, szPort - szParam);
    szServer[szPort - szParam] = '\0';
    *szPort++ = '\0';
  }
  
  TCPSocket *sock = new TCPSocket(strUser.c_str(), MSN_PPID);
  sock->SetRemoteAddr(szServer, atoi(szPort));
  char ipbuf[32];
  gLog.Info("%sConnecting to SB at %s:%d.\n", L_MSNxSTR, sock->RemoteIpStr(ipbuf),
    sock->RemotePort());
  
  if (!sock->OpenConnection())
  {
    gLog.Error("%sConnection to SB at %s failed.\n", L_MSNxSTR, sock->RemoteIpStr(ipbuf));
    delete sock;
    return false;
  }
  
  gSocketMan.AddSocket(sock);
  CMSNPacket *pReply = new CPS_MSN_SBAnswer(strSessionId.c_str(),
    strCookie.c_str(), m_szUserName);
  bool bNewUser = false;
  int nSocket = sock->Descriptor();
  ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
  if (u)
  {
    u->SetSocketDesc(sock);
  }
  else
  {
    m_pDaemon->AddUserToList(strUser.c_str(), MSN_PPID, false, true);
    u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
    u->SetEnableSave(false);
    u->SetUserEncoding("UTF-8");
    u->SetSocketDesc(sock);
    u->SetEnableSave(true);
    u->SaveLicqInfo();
    bNewUser = true;
  }
  gUserManager.DropUser(u);
  gSocketMan.DropSocket(sock);
  
  Send_SB_Packet(strUser, pReply, nSocket);
  
  return true;
}

void CMSN::MSNSendInvitation(const char* _szUser, CMSNPacket* _pPacket)
{
  //const ICQUser* u = gUserManager.FetchUser(_szUser, MSN_PPID, LOCK_R);
  //if (!u) return;
  //gUserManager.DropUser(u);

  // Must connect to the SB and call the user
  CMSNPacket *pSB = new CPS_MSNXfr();
      
  SStartMessage *p = new SStartMessage;
  p->m_pPacket = _pPacket;
  p->m_pEvent = 0;
  p->m_pSignal = 0;
  p->m_szUser = strdup(_szUser);
  p->m_nSeq = pSB->Sequence();
  p->m_bConnecting = false;
  p->m_bDataConnection = true;
  pthread_mutex_lock(&mutex_StartList);
  m_lStart.push_back(p);
  pthread_mutex_unlock(&mutex_StartList);
  
  SendPacket(pSB);
}

void CMSN::MSNSendMessage(const char* _szUser, const char* _szMsg,
    pthread_t _tPlugin, unsigned long _nCID)
{
  string strUser(_szUser);
  int nSocket = -1;
  
  if (_nCID)
  {
    CConversation *pConv = m_pDaemon->FindConversation(_nCID);
    if (pConv)
      nSocket = pConv->Socket();
  } 

  const ICQUser* u = gUserManager.FetchUser(_szUser, MSN_PPID, LOCK_R);
  if (!u) return;
  gUserManager.DropUser(u);
  
  char *szRNMsg = gTranslator.NToRN(_szMsg);
  CMSNPacket *pSend = new CPS_MSNMessage(szRNMsg);
  CEventMsg *m = new CEventMsg(szRNMsg, 0, TIME_NOW, 0);
  m->m_eDir = D_SENDER;
  ICQEvent *e = new ICQEvent(m_pDaemon, 0, pSend, CONNECT_SERVER, strdup(_szUser), MSN_PPID, m);
  e->thread_plugin = _tPlugin;  
  CICQSignal *s = new CICQSignal(SIGNAL_EVENTxID, 0, strdup(_szUser), MSN_PPID, e->EventId());
  
  delete [] szRNMsg;

  if (nSocket > 0)
  {
    m_pEvents.push_back(e);
    m_pDaemon->PushPluginSignal(s);
      
    Send_SB_Packet(strUser, pSend, nSocket, false);
  }
  else
  {
    // Must connect to the SB and call the user
    CMSNPacket *pSB = new CPS_MSNXfr();
      
    SStartMessage *p = new SStartMessage;
    p->m_pPacket = pSend;
    p->m_pEvent = e;
    p->m_pSignal = s;
    p->m_szUser = strdup(_szUser);
    p->m_nSeq = pSB->Sequence();
    p->m_bConnecting = false;
    p->m_bDataConnection = false;
    pthread_mutex_lock(&mutex_StartList);
    m_lStart.push_back(p);
    pthread_mutex_unlock(&mutex_StartList);
   
    SendPacket(pSB);
  }  
}

void CMSN::MSNSendTypingNotification(const char* _szUser, unsigned long _nCID)
{
  string strUser(_szUser);
  CMSNPacket *pSend = new CPS_MSNTypingNotification(m_szUserName);
  int nSockDesc = -1;
  
  if (_nCID)
  {
    CConversation *pConv = m_pDaemon->FindConversation(_nCID);
    if (pConv)
      nSockDesc = pConv->Socket();
  } 
    
  if (nSockDesc > 0)
    Send_SB_Packet(strUser, pSend, nSockDesc);
}

