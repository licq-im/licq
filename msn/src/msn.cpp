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

#ifndef HAVE_STRNDUP

#include <stdlib.h>

char *strndup(const char *s, size_t n)
{
  char *str;

  if (n < 1)
    return NULL;

  str = (char *)malloc(n + 1);
  if (!str)
    return NULL;

  memset(str, '\0', n + 1);
  strncpy(str, s, n);

  return str;
}
#endif // HAVE_STRNDUP

//Global socket manager
CSocketManager gSocketMan;

void *MSNPing_tep(void *);

CMSN::CMSN(CICQDaemon *_pDaemon, int _nPipe) : m_vlPacketBucket(211)
{
  m_pDaemon = _pDaemon;
  m_bExit = false;
  m_nPipe = _nPipe;
  m_nSSLSocket = m_nServerSocket = m_nNexusSocket = -1;
  m_pPacketBuf = 0;
  m_pNexusBuff = 0;
  m_pSSLPacket = 0;
  m_szUserName = 0;
  m_szPassword = 0;
  
  // Config file
  char szFileName[MAX_FILENAME_LEN];
  sprintf(szFileName, "%s/licq_msn.conf", BASE_DIR);
  CIniFile msnConf;
  if (!msnConf.LoadFile(szFileName))
  {
    FILE *f = fopen(szFileName, "w");
    fprintf(f, "[network]");
    fclose(f);
    msnConf.LoadFile(szFileName);
  }  
  
  msnConf.SetSection("network");
  
  msnConf.ReadNum("ListVersion", m_nListVersion, 0);
  
  msnConf.CloseFile();
}

CMSN::~CMSN()
{
  if (m_pPacketBuf)
    delete m_pPacketBuf;
  if (m_szUserName)
    free(m_szUserName);
  if (m_szPassword)
    free(m_szPassword);
    
  // Config file
  char szFileName[MAX_FILENAME_LEN];
  sprintf(szFileName, "%s/licq_msn.conf", BASE_DIR);
  CIniFile msnConf;
  if (msnConf.LoadFile(szFileName))
  {
    msnConf.SetSection("network");
    
    msnConf.WriteNum("ListVersion", m_nListVersion);
    msnConf.FlushFile();
    msnConf.CloseFile();
  }
}

void CMSN::StorePacket(SBuffer *_pBuf, int _nSock)
{
  BufferList &b = m_vlPacketBucket[HashValue(_nSock)];
  b.push_front(_pBuf);
}

void CMSN::RemovePacket(string _strUser, int _nSock)
{
  BufferList &b = m_vlPacketBucket[HashValue(_nSock)];
  BufferList::iterator it;
  for (it = b.begin(); it != b.end(); it++)
  {
    if ((*it)->m_strUser == _strUser)
    {
      b.erase(it);
      break;
    }
  }
}

SBuffer *CMSN::RetrievePacket(string _strUser, int _nSock)
{
  BufferList &b = m_vlPacketBucket[HashValue(_nSock)];
  BufferList::iterator it;
  for (it = b.begin(); it != b.end(); it++)
  {
    if ((*it)->m_strUser == _strUser)
    {
      return *it;
    }
  }
  
  return 0;
}
  
ICQEvent *CMSN::RetrieveEvent(unsigned long _nTag)
{
  ICQEvent *e = 0;
  
  list<ICQEvent *>::iterator it;
  for (it = m_pEvents.begin(); it != m_pEvents.end(); it++)
  {
    if ((*it)->Sequence() == _nTag)
    {
      e = *it;
      m_pEvents.erase(it);
      break;
    }
  }
  
  return e;
}

void CMSN::Run()
{
  int nNumDesc;
  int nCurrent; 
  fd_set f;

  int nResult = pthread_create(&m_tMSNPing, NULL, &MSNPing_tep, this);
  if (nResult)
  {
    gLog.Error("%sUnable to start ping thread:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR, strerror(nResult));
  }
  
  nResult = 0;
  
  while (!m_bExit)
  {
    f = gSocketMan.SocketSet();
    nNumDesc = gSocketMan.LargestSocket() + 1;
 
    if (m_nPipe != -1)
    {
      FD_SET(m_nPipe, &f);
      if (m_nPipe >= nNumDesc)
        nNumDesc = m_nPipe + 1;
    }

    nResult = select(nNumDesc, &f, NULL, NULL, NULL);
  
    nCurrent = 0;
    while (nResult > 0 && nCurrent < nNumDesc)
    {
      if (FD_ISSET(nCurrent, &f))
      {
        if (nCurrent == m_nPipe)
        {
          ProcessPipe();
        }
        
        else if (nCurrent == m_nServerSocket)
        {
          INetSocket *s = gSocketMan.FetchSocket(m_nServerSocket);
          TCPSocket *sock = static_cast<TCPSocket *>(s);
          if (sock->RecvRaw())
          {
            CMSNBuffer packet(sock->RecvBuffer());
            sock->ClearRecvBuffer();
            gSocketMan.DropSocket(sock);
            ProcessServerPacket(packet);
          }
          else
          {
            // Time to reconnect
            gLog.Info("%sDisconnected from server, reconnecting.\n", L_MSNxSTR);
            int nSD = m_nServerSocket;
            m_nServerSocket = -1;
            gSocketMan.DropSocket(sock);
            gSocketMan.CloseSocket(nSD);
            MSNLogon("messenger.hotmail.com", 1863);
          }
        }
        
        else if (nCurrent == m_nNexusSocket)
        {
          INetSocket *s = gSocketMan.FetchSocket(m_nNexusSocket);
          TCPSocket *sock = static_cast<TCPSocket *>(s);
          if (sock->SSLRecv())
          {
            CMSNBuffer packet(sock->RecvBuffer());
            sock->ClearRecvBuffer();
            gSocketMan.DropSocket(sock);
            ProcessNexusPacket(packet);
          }
        }

        else if (nCurrent == m_nSSLSocket)
        {
          INetSocket *s = gSocketMan.FetchSocket(m_nSSLSocket);
          TCPSocket *sock = static_cast<TCPSocket *>(s);
          if (sock->SSLRecv())
          {
            CMSNBuffer packet(sock->RecvBuffer());
            sock->ClearRecvBuffer();
            gSocketMan.DropSocket(sock);
            ProcessSSLServerPacket(packet);
          }
        }
        
        else
        {
          //SB socket
          INetSocket *s = gSocketMan.FetchSocket(nCurrent);
          TCPSocket *sock = static_cast<TCPSocket *>(s);
          if (sock->RecvRaw())
          {
            CMSNBuffer packet(sock->RecvBuffer());
            bool bProcess = false;
            sock->ClearRecvBuffer();
            char *szUser = strdup(sock->OwnerId());
            gSocketMan.DropSocket(sock);
            
            // Build the packet with last received portion
            string strUser(szUser);
            SBuffer *pBuf = RetrievePacket(strUser, nCurrent);
            if (pBuf)
            {
              *(pBuf->m_pBuf) += packet;
            }
            else
            {
              pBuf = new SBuffer;
              pBuf->m_pBuf = new CMSNBuffer(packet);
              pBuf->m_strUser = strUser;
            }
            
            // Do we have the entire packet?
            char *szNeedle;
            if ((szNeedle = strstr((char *)pBuf->m_pBuf->getDataStart(), "\r\n")))
            {
              // We have a basic packet, now check for a packet that has a payload
              if (memcmp(pBuf->m_pBuf->getDataStart(), "MSG", 3) == 0)
              {
                pBuf->m_pBuf->SkipParameter(); // command
                pBuf->m_pBuf->SkipParameter(); // user id
                pBuf->m_pBuf->SkipParameter(); // alias
                string strSize = pBuf->m_pBuf->GetParameter();
                int nSize = atoi(strSize.c_str());
                  
                // FIXME: Cut the packet instead of passing it all along
                // we might receive 1 full packet and part of another.
                if (nSize <= (pBuf->m_pBuf->getDataPosWrite() - pBuf->m_pBuf->getDataPosRead()))
                {
                  bProcess = true;
                }
                else
                {
                  StorePacket(pBuf, nCurrent);
                }
                
                pBuf->m_pBuf->Reset();  
              }
              else
                bProcess = true; // no payload
            }  
            
            if (bProcess)
            {
              ProcessSBPacket(szUser, pBuf->m_pBuf);
              RemovePacket(strUser, nCurrent);
              delete pBuf;
            }
            
            free(szUser);
          }
        }
      }

      nCurrent++;
    }

  }
  
  //pthread_cancel(m_tMSNPing);
}

void CMSN::ProcessPipe()
{
  char buf[16];
  read(m_nPipe, buf, 1);
  switch (buf[0])
  {
  case 'S':  // A signal is pending
    {
      CSignal *s = m_pDaemon->PopProtoSignal();
      ProcessSignal(s);
      break;
    }

  case 'X': // Bye
    gLog.Info("%sExiting.\n", L_MSNxSTR);
    m_bExit = true;
    break;
  }
}

void CMSN::ProcessSignal(CSignal *s)
{
  switch (s->Type())
  {
    case PROTOxLOGON:
    {
      if (m_nServerSocket < 0)
        MSNLogon("messenger.hotmail.com", 1863);
      break;
    }
    
    case PROTOxCHANGE_STATUS:
    {
      CChangeStatusSignal *sig = static_cast<CChangeStatusSignal *>(s);
      MSNChangeStatus(sig->Status());
      break;
    }
    
    case PROTOxLOGOFF:
    {
      MSNLogoff();
      break;
    }
    
    case PROTOxADD_USER:
    {
      CAddUserSignal *sig = static_cast<CAddUserSignal *>(s);
      MSNAddUser(sig->Id());
      break;
    }
    
    case PROTOxREM_USER:
    {
      CRemoveUserSignal *sig = static_cast<CRemoveUserSignal *>(s);
      MSNRemoveUser(sig->Id());
      break;
    }
    
    case PROTOxSENDxTYPING_NOTIFICATION:
    {
      CTypingNotificationSignal *sig =
        static_cast<CTypingNotificationSignal *>(s);
      if (sig->Active())
        MSNSendTypingNotification(sig->Id());
      break;
    }
    
    case PROTOxSENDxMSG:
    {
      CSendMessageSignal *sig = static_cast<CSendMessageSignal *>(s);
      MSNSendMessage(sig->Id(), sig->Message(), sig->Thread());
      break;
    }

    case PROTOxSENDxGRANTxAUTH:
    {
      CGrantAuthSignal *sig = static_cast<CGrantAuthSignal *>(s);
      MSNGrantAuth(sig->Id());
      break;
    }
  }
}

void CMSN::ProcessNexusPacket(CMSNBuffer &packet)
{
  bool bNew = false;
  if (m_pNexusBuff == 0)
  {
    m_pNexusBuff = new CMSNBuffer(packet);
    bNew = true;
  }

  char *ptr = packet.getDataStart() + packet.getDataSize() - 4;
  int x = memcmp(ptr, "\x0D\x0A\x0D\x0A", 4);
  if (x) return;
  else if (!bNew) *m_pNexusBuff += packet;

  char cTmp = 0;

  while (cTmp != '\r')
    *m_pNexusBuff >> cTmp;
  *m_pNexusBuff >> cTmp; // skip the \n as well

  m_pNexusBuff->ParseHeaders();
  
  char *szLogin = strstr(m_pNexusBuff->GetValue("PassportURLs").c_str(), "DALogin=");
  szLogin += 8; // skip to the tag
  char *szEndURL = strchr(szLogin, '/');
  char *szServer = strndup(szLogin, szEndURL - szLogin); // this is all we need
  char *szEnd = strchr(szLogin, ',');
  char *szURL = strndup(szEndURL, szEnd - szEndURL);

  MSNAuthenticate(m_szCookie);
}

void CMSN::ProcessSSLServerPacket(CMSNBuffer &packet)
{
  // Did we receive the entire packet?
  // I don't like doing this, is there a better way to see
  // if we get the entire packet at the socket level?
  // I couldn't find anything in the HTTP RFC about this packet
  // being broken up without any special HTTP headers
  if (m_pSSLPacket == 0)
    m_pSSLPacket = new CMSNBuffer(packet);

  char *ptr = packet.getDataStart() + packet.getDataSize() - 4;
  int x = memcmp(ptr, "\x0D\x0A\x0D\x0A", 4);
  if (m_pSSLPacket->getDataSize() != packet.getDataSize())
    *m_pSSLPacket += packet;
  
  if (x)  return;
  
  // Now process the packet
  char cTmp = 0;
  string strFirstLine = "";
  
  *m_pSSLPacket >> cTmp;
  while (cTmp != '\r')
  {
    strFirstLine += cTmp;
    *m_pSSLPacket >> cTmp;
  }
  
  *m_pSSLPacket >> cTmp; // skip \n as well
  
  // Authenticated
  if (strFirstLine == "HTTP/1.1 200 OK")
  {
    m_pSSLPacket->ParseHeaders();
    char *fromPP = strstr(m_pSSLPacket->GetValue("Authentication-Info").c_str(), "from-PP=");
    char *tag;

    if (fromPP == 0)
      tag = m_szCookie;
    else
    {
      fromPP+= 9; // skip to the tag
      char *endTag = strchr(fromPP, '\'');
      tag = strndup(fromPP, endTag - fromPP); // Thanks, this is all we need
    }

    CMSNPacket *pReply = new CPS_MSNSendTicket(tag);
    SendPacket(pReply);
    free(tag);
    m_szCookie = 0;
  }
  else if (strFirstLine == "HTTP/1.1 302 Object moved")
  {
    m_pSSLPacket->ParseHeaders();
    string strAuthHeader = m_pSSLPacket->GetValue("WWW-Authenticate");
    string strToSend = strAuthHeader.substr(strAuthHeader.find(" ") + 1, strAuthHeader.size() - strAuthHeader.find(" "));

    string strLocation = m_pSSLPacket->GetValue("Location");
    string::size_type pos = strLocation.find("/", 8);
    if (pos != string::npos)
    {
      string strHost = strLocation.substr(7, pos - 7);
      string strParam = strLocation.substr(pos, strLocation.size() - pos);
      gSocketMan.CloseSocket(m_nSSLSocket, false, true);
      m_nSSLSocket = -1;
      delete m_pSSLPacket;
      m_pSSLPacket = 0;

      gLog.Info("%sRedirecting to %s:443\n", L_MSNxSTR, strHost.c_str());
      MSNAuthenticateRedirect(strHost, strToSend);
      return;
    }
    else
      gLog.Error("%sMalformed location header.\n", L_MSNxSTR);
  }
  else if (strFirstLine == "HTTP/1.1 401 Unauthorized")
  {
    gLog.Error("%sInvalid password.\n", L_MSNxSTR);
  }
  else
  {
    gLog.Error("%sUnknown sign in error.\n", L_MSNxSTR);
  }
  
  gSocketMan.CloseSocket(m_nSSLSocket, false, true);
  m_nSSLSocket = -1;
  delete m_pSSLPacket;
  m_pSSLPacket = 0;
}

void CMSN::ProcessSBPacket(char *szUser, CMSNBuffer *packet)
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
            USER_TYPING, strUser.c_str(), MSN_PPID));
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
        
        CEventMsg *e = CEventMsg::Parse(szMsg, ICQ_CMDxRCV_SYSxMSGxOFFLINE, time(0), 0);
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
        Send_SB_Packet(strUser, pStart->m_pPacket, false);
        
        free(pStart->m_szUser);
        delete pStart;
      }
    }
    else if (strCmd == "BYE")
    {
      // closed the window and connection
      string strUser = packet->GetParameter();
      ICQUser *u = gUserManager.FetchUser(const_cast<char *>(strUser.c_str()), MSN_PPID, LOCK_W);
      if (u)
      {
        gLog.Info("%sConnection with %s closed.\n", L_MSNxSTR, strUser.c_str());
        int nSock = u->SocketDesc(ICQ_CHNxNONE);
        u->ClearSocketDesc(ICQ_CHNxNONE);
        gUserManager.DropUser(u);
        INetSocket *s = gSocketMan.FetchSocket(nSock);
        gSocketMan.DropSocket(s);  
        gSocketMan.CloseSocket(nSock);
      }
    }
  
    // Get the next packet
    if (bSkipPacket)
      packet->SkipPacket();
    
    if (pReply)
    {
      string strTo(szUser);
      Send_SB_Packet(strTo, pReply);
    }
  }
  
  delete packet;
}

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
        MSNLogon(szNewServer, atoi(szPort));
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
      
      pReply = new CPS_MSNChangeStatus(ICQ_STATUS_ONLINE);
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
        gLog.Info("%sAdded %s to contact list.\n", L_MSNxSTR, strUser.c_str());
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
      gLog.Info("%sServer says we are now: %s\n", L_MSNxSTR, ICQUser::StatusToStatusStr(o->Status(), false));
      gUserManager.DropOwner(MSN_PPID);
    }
    else if (strCmd == "ILN" || strCmd == "NLN")
    {
      if (strCmd == "ILN")
        m_pPacketBuf->SkipParameter(); // seq
      string strStatus = m_pPacketBuf->GetParameter();
      string strUser = m_pPacketBuf->GetParameter();
      unsigned short nStatus = ICQ_STATUS_AWAY;

      if (strStatus == "NLN")
        nStatus = ICQ_STATUS_ONLINE;
        
      ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
      if (u)
      {
        u->SetOnlineSince(time(NULL)); // Not in this protocol
        u->SetSendServer(true); // no direct connections
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

void CMSN::Send_SB_Packet(string &strUser, CMSNPacket *p, bool bDelete)
{
  ICQUser *u = gUserManager.FetchUser(const_cast<char *>(strUser.c_str()), MSN_PPID, LOCK_R);
  if (!u) return;

  int nSock = u->SocketDesc(ICQ_CHNxNONE);
  gUserManager.DropUser(u);  
  INetSocket *s = gSocketMan.FetchSocket(nSock);
  TCPSocket *sock = static_cast<TCPSocket *>(s);
  sock->SendRaw(p->getBuffer());
  gSocketMan.DropSocket(sock);
  
  if (bDelete)
    delete p;
}

void CMSN::MSNLogon(const char *_szServer, int _nPort)
{
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
}

void CMSN::MSNGetServer()
{
  TCPSocket *sock = new TCPSocket(m_szUserName, MSN_PPID);
  sock->SetRemoteAddr("nexus.passport.com", 443);
//  char ipbuf[32];
  if (!sock->OpenConnection())
  {
    delete sock;
    return;
  }
  
  if (!sock->SecureConnect())
  {
    delete sock;
    return;
  }

  gSocketMan.AddSocket(sock);
  m_nNexusSocket = sock->Descriptor();
  CMSNPacket *pHello = new CPS_MSNGetServer();
  sock->SSLSend(pHello->getBuffer());
  gSocketMan.DropSocket(sock);
}

void CMSN::MSNAuthenticateRedirect(string &strHost, string &strParam)
{
  TCPSocket *sock = new TCPSocket(m_szUserName, MSN_PPID);
  sock->SetRemoteAddr(strHost.c_str(), 443);
  char ipbuf[32];
  gLog.Info("%sAuthenticating to %s:%d\n", L_MSNxSTR, sock->RemoteIpStr(ipbuf),
sock->RemotePort());

  if (!sock->OpenConnection())
  {
    gLog.Error("%sConnection to %s failed.\n", L_MSNxSTR, sock->RemoteIpStr(ipbuf));
    delete sock;
    return;
  }

  if (!sock->SecureConnect())
  {
    gLog.Error("%sSSL connection failed.\n", L_MSNxSTR);
    delete sock;
    return;
  }

  gSocketMan.AddSocket(sock);
  m_nSSLSocket = sock->Descriptor();
  CMSNPacket *pHello = new CPS_MSNAuthenticate(m_szUserName, m_szPassword, strParam.c_str());
  sock->SSLSend(pHello->getBuffer());
  gSocketMan.DropSocket(sock);
}

void CMSN::MSNAuthenticate(char *szCookie)
{
  TCPSocket *sock = new TCPSocket(m_szUserName, MSN_PPID);
  sock->SetRemoteAddr("loginnet.passport.com", 443);
  char ipbuf[32];
  gLog.Info("%sAuthenticating to %s:%d\n", L_MSNxSTR, sock->RemoteIpStr(ipbuf), sock->RemotePort());
  
  if (!sock->OpenConnection())
  {
    gLog.Error("%sConnection to %s failed.\n", L_MSNxSTR, sock->RemoteIpStr(ipbuf));
    delete sock;
    free(szCookie);
    szCookie = 0;
    return;
  }
  
  if (!sock->SecureConnect())
  {
    gLog.Error("%sSSL connection failed.\n", L_MSNxSTR);   
    free(szCookie);
    szCookie = 0;
    delete sock;
    return;
  }
  
  gSocketMan.AddSocket(sock);
  m_nSSLSocket = sock->Descriptor();
  CMSNPacket *pHello = new CPS_MSNAuthenticate(m_szUserName, m_szPassword, szCookie);
  sock->SSLSend(pHello->getBuffer());
  gSocketMan.DropSocket(sock);

  free(szCookie);
  szCookie = 0;
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
  ICQUser *u = gUserManager.FetchUser(pStart->m_szUser, MSN_PPID, LOCK_W);
  if (u)
  {
    u->SetSocketDesc(sock);
    gUserManager.DropUser(u);
  }
  gSocketMan.DropSocket(sock);
  
  CMSNPacket *pReply = new CPS_MSN_SBStart(strCookie.c_str(), m_szUserName);
  string strUser(pStart->m_szUser);
  Send_SB_Packet(strUser, pReply);  

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
  ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
  if (u)
  {
    u->SetSocketDesc(sock);
    gUserManager.DropUser(u);
  }
  
  gSocketMan.DropSocket(sock);
  
  Send_SB_Packet(strUser, pReply);
  
  return true;
}

void CMSN::MSNSendMessage(char *_szUser, char *_szMsg, pthread_t _tPlugin)
{
  string strUser(_szUser);
  
  ICQUser *u = gUserManager.FetchUser(_szUser, MSN_PPID, LOCK_R);
  if (!u) return;
  int nSockDesc = u->SocketDesc(ICQ_CHNxNONE);
  gUserManager.DropUser(u);
  
  CMSNPacket *pSend = new CPS_MSNMessage(_szMsg);
  CEventMsg *m = new CEventMsg(_szMsg, 0, TIME_NOW, 0);
  m->m_eDir = D_SENDER;
  ICQEvent *e = new ICQEvent(m_pDaemon, 0, pSend, CONNECT_SERVER, strdup(_szUser), MSN_PPID, m);
  e->thread_plugin = _tPlugin;  
  CICQSignal *s = new CICQSignal(SIGNAL_EVENTxID, 0, strdup(_szUser), MSN_PPID, e->EventId());
  
  if (nSockDesc > 0)
  {
    m_pEvents.push_back(e);
    m_pDaemon->PushPluginSignal(s);
      
    Send_SB_Packet(strUser, pSend, false);
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

void CMSN::MSNSendTypingNotification(char *_szUser)
{
  ICQUser *u = gUserManager.FetchUser(_szUser, MSN_PPID, LOCK_R);
  if (!u) return;
  int nSockDesc = u->SocketDesc(ICQ_CHNxNONE);
  gUserManager.DropUser(u);
  
  string strUser(_szUser);
  CMSNPacket *pSend = new CPS_MSNTypingNotification(m_szUserName);
    
  if (nSockDesc > 0)
    Send_SB_Packet(strUser, pSend, false);
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

void CMSN::MSNChangeStatus(unsigned long _nStatus)
{
  CMSNPacket *pSend = new CPS_MSNChangeStatus(_nStatus);
  SendPacket(pSend);
}

void CMSN::MSNLogoff()
{
  CMSNPacket *pSend = new CPS_MSNLogoff();
  SendPacket(pSend);
 
  // Close the socket
  INetSocket *s = gSocketMan.FetchSocket(m_nServerSocket);
  int nSD = m_nServerSocket;
  m_nServerSocket = -1;
  gSocketMan.DropSocket(s);
  gSocketMan.CloseSocket(nSD);
  
  // Update the daemon
  FOR_EACH_PROTO_USER_START(MSN_PPID, LOCK_W)
  {
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
