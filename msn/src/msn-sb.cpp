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

void CMSN::ProcessSBPacket(char *szUser, CMSNBuffer *packet, int nSock)
{
  char szCommand[4];
  CMSNPacket *pReply;
  bool bSkipPacket;
  
  while (!packet->End())
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

      m_pDaemon->PushPluginSignal(new
        CICQSignal(SIGNAL_CONVOxJOIN, 0, strdup(strUser.c_str()), MSN_PPID, nSock));
        
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
        ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
        if (u)
        {
          u->SetTyping(ICQ_TYPING_ACTIVE);
          gUserManager.DropUser(u);
          m_pDaemon->PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
            USER_TYPING, strUser.c_str(), MSN_PPID, nSock));
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
        
        CEventMsg *e = CEventMsg::Parse(szMsg, ICQ_CMDxRCV_SYSxMSGxOFFLINE, time(0), 0, nSock);
        ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
        if (u)
          u->SetTyping(0);
        if (m_pDaemon->AddUserEvent(u, e))
          m_pDaemon->m_xOnEventManager.Do(ON_EVENT_MSG, u);
        gUserManager.DropUser(u);
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
          ICQUser *u = gUserManager.FetchUser(e->m_szId, e->m_nPPID, LOCK_R);
          if (u != NULL)
          {
            e->m_pUserEvent->AddToHistory(u, D_SENDER);
            u->SetLastSentEvent();
            m_pDaemon->m_xOnEventManager.Do(ON_EVENT_MSGSENT, u);
            gUserManager.DropUser(u);
          }
          m_pDaemon->m_sStats[STATS_EventsSent].Inc();
        }
      }

      m_pDaemon->PushPluginEvent(e);
    }
    else if (strCmd == "USR")
    {
      SStartMessage *pStart = m_lStart.front();
      pReply = new CPS_MSNCall(pStart->m_szUser);
    }
    else if (strCmd == "JOI")
    {
      string strUser = packet->GetParameter();
      gLog.Info("%s%s joined the conversation.\n", L_MSNxSTR, strUser.c_str());

      m_pDaemon->PushPluginSignal(new
        CICQSignal(SIGNAL_CONVOxJOIN, 0, strdup(strUser.c_str()), MSN_PPID, nSock));
  
      SStartMessage *pStart = 0;
      StartList::iterator it;
      for (it = m_lStart.begin(); it != m_lStart.end(); it++)
      {
        if (strcmp((*it)->m_szUser, strUser.c_str()) == 0) // case insensitive perhaps?
        {
          pStart = *it;
          m_lStart.erase(it);
          break;
        }
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
    }
    else if (strCmd == "BYE")
    {
      // closed the window and connection
      string strUser = packet->GetParameter();
      gLog.Info("%sConnection with %s closed.\n", L_MSNxSTR, strUser.c_str());

      ICQUser *u = gUserManager.FetchUser(const_cast<char *>(strUser.c_str()), MSN_PPID, LOCK_W);
      if (u)
      {
        int nThisSock = u->SocketDesc(ICQ_CHNxNONE);
        u->ClearSocketDesc(ICQ_CHNxNONE);
        gUserManager.DropUser(u);
        INetSocket *s = gSocketMan.FetchSocket(nThisSock);
        gSocketMan.DropSocket(s);  
        gSocketMan.CloseSocket(nThisSock);
      }

      m_pDaemon->PushPluginSignal(new
        CICQSignal(SIGNAL_CONVOxLEAVE, 0, strdup(strUser.c_str()), MSN_PPID, nSock));
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
  
  delete packet;
}

void CMSN::Send_SB_Packet(string &strUser, CMSNPacket *p, int nSocket, bool bDelete)
{
  ICQUser *u = gUserManager.FetchUser(const_cast<char *>(strUser.c_str()), MSN_PPID, LOCK_R);
  if (!u) return;

  int nSock = nSocket != -1 ? nSocket : u->SocketDesc(ICQ_CHNxNONE);
  gUserManager.DropUser(u);  
  INetSocket *s = gSocketMan.FetchSocket(nSock);
  if (!s)
    s = gSocketMan.FetchSocket(nSocket);
  if (!s) return;
  TCPSocket *sock = static_cast<TCPSocket *>(s);
  sock->SendRaw(p->getBuffer());
  gSocketMan.DropSocket(sock);
  
  if (bDelete)
    delete p;
}

bool CMSN::MSNSBConnectStart(string &strServer, string &strCookie)
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
  
  SStartMessage *pStart = m_lStart.front();
  
  TCPSocket *sock = new TCPSocket(pStart->m_szUser, MSN_PPID);
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
    u->SetSocketDesc(sock);
    gUserManager.DropUser(u);
    m_pDaemon->PushPluginSignal(new CICQSignal(SIGNAL_SOCKET, 0, strdup(pStart->m_szUser),
      MSN_PPID, sock->Descriptor()));
  }
  gSocketMan.DropSocket(sock);
  
  CMSNPacket *pReply = new CPS_MSN_SBStart(strCookie.c_str(), m_szUserName);
  string strUser(pStart->m_szUser);
  Send_SB_Packet(strUser, pReply, nSocket);  

  return true;
}

bool CMSN::MSNSBConnectAnswer(string &strServer, string &strSessionId, string &strCookie,
                              string &strUser)
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
    u = new ICQUser(strUser.c_str(), MSN_PPID);
    u->SetSocketDesc(sock);
    m_pDaemon->AddUserToList(u);
    bNewUser = true;
  }
  gUserManager.DropUser(u);
  
  m_pDaemon->PushPluginSignal(new CICQSignal(SIGNAL_SOCKET, 0, strdup(strUser.c_str()),
      MSN_PPID, sock->Descriptor()));
      
  if (bNewUser)
    MSNAddUser(const_cast<char *>(strUser.c_str()));

  gSocketMan.DropSocket(sock);
  
  Send_SB_Packet(strUser, pReply, nSocket);
  
  return true;
}


void CMSN::MSNSendMessage(char *_szUser, char *_szMsg, pthread_t _tPlugin, int nSockDesc)
{
  string strUser(_szUser);
  
  ICQUser *u = gUserManager.FetchUser(_szUser, MSN_PPID, LOCK_R);
  if (!u) return;
  int nSocket = nSockDesc != -1 ? nSockDesc : u->SocketDesc(ICQ_CHNxNONE);
  bool bCantSend = u->StatusOffline() || (m_nStatus & ICQ_STATUS_FxPRIVATE);
  gUserManager.DropUser(u);
  
  CMSNPacket *pSend = new CPS_MSNMessage(_szMsg);
  CEventMsg *m = new CEventMsg(_szMsg, 0, TIME_NOW, 0);
  m->m_eDir = D_SENDER;
  ICQEvent *e = new ICQEvent(m_pDaemon, 0, pSend, CONNECT_SERVER, strdup(_szUser), MSN_PPID, m);
  e->thread_plugin = _tPlugin;  
  CICQSignal *s = new CICQSignal(SIGNAL_EVENTxID, 0, strdup(_szUser), MSN_PPID, e->EventId());
  
  if (bCantSend)
  {
    // MSN doesn't support sending to offline users or if we are invisible.
    // It'd be best to change the GUI, but Licq uses a
    // GUI plugin system.. so that would force more requirements
    // on the plugin. This allows the plugin to see that it has failed.
    m_pDaemon->PushPluginSignal(s);
    e->m_eResult = EVENT_FAILED;
    m_pDaemon->PushPluginEvent(e);
    if (m_nStatus & ICQ_STATUS_FxPRIVATE)
      gLog.Error("%sCannot send messages while Invisible.\n", L_ERRORxSTR);
    else
      gLog.Error("%sCannot send messages to offline MSN users.\n", L_ERRORxSTR);
    return;
  }
  
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
    m_lStart.push_back(p);
   
    SendPacket(pSB);
  }  
}

void CMSN::MSNSendTypingNotification(char *_szUser, int nSockDesc)
{
  string strUser(_szUser);
  CMSNPacket *pSend = new CPS_MSNTypingNotification(m_szUserName);
    
  if (nSockDesc > 0)
    Send_SB_Packet(strUser, pSend, nSockDesc, false);
#if 0
  else
  {
    // Must connect to the SB and call the user
    CMSNPacket *pSB = new CPS_MSNXfr();
      
    SStartMessage *p = new SStartMessage;
    p->m_pPacket = pSend;
    p->m_pEvent = 0;
    p->m_pSignal = 0;
    p->m_szUser = strdup(_szUser);
    m_lStart.push_back(p);
   
    SendPacket(pSB);    
  }
#endif
}

