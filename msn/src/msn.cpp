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
  m_nStatus = ICQ_STATUS_OFFLINE;
  m_szUserName = 0;
  m_szPassword = 0;
  m_nSessionStart = 0;
  
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

  // pthread stuff now
  pthread_mutex_init(&mutex_StartList, 0);
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

string CMSN::Decode(const string &strIn)
{
  string strOut = "";
  
  for (unsigned int i = 0; i < strIn.length(); i++)
  {
    if (strIn[i] == '%')
    {
      char szByte[3];
      szByte[0] = strIn[++i]; szByte[1] = strIn[++i]; szByte[2] = '\0';
      strOut += strtol(szByte, NULL, 16);
    }
    else
      strOut += strIn[i];
  }

  return strOut;
}

unsigned long CMSN::SocketToCID(int _nSocket)
{
  CConversation *pConv = m_pDaemon->FindConversation(_nSocket);
  return pConv ? pConv->CID() : 0;
}

string CMSN::Encode(const string &strIn)
{
  string strOut = "";

  for (unsigned int i = 0; i < strIn.length(); i++)
  {
    if (isalnum(strIn[i]))
      strOut += strIn[i];
    else
    {
      char szChar[4];
      sprintf(szChar, "%%%02X", strIn[i]);
      szChar[3] = '\0';
      strOut += szChar;
    }
  }

  return strOut;
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
            MSNLogon("messenger.hotmail.com", 1863, m_nStatus);
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
              ProcessSBPacket(szUser, pBuf->m_pBuf, sock->Descriptor());
              RemovePacket(strUser, nCurrent);
              delete pBuf;
            }
            
            free(szUser);
          }
          else
          {
            // Shouldn't get here, as we close the socket with a BYE command.
            // But just to be safe..
            gSocketMan.DropSocket(sock);
            gSocketMan.CloseSocket(nCurrent);
          }
        }
      }

      nCurrent++;
    }

  }
  
  
  // Close out now
  pthread_cancel(m_tMSNPing);
  MSNLogoff();
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
      {
        CLogonSignal *sig = static_cast<CLogonSignal *>(s);
        MSNLogon("messenger.hotmail.com", 1863, sig->LogonStatus());
      }
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
    
    case PROTOxRENAME_USER:
    {
      CRenameUserSignal *sig = static_cast<CRenameUserSignal *>(s);
      MSNRenameUser(sig->Id());
      break;
    }

    case PROTOxSENDxTYPING_NOTIFICATION:
    {
      CTypingNotificationSignal *sig =
        static_cast<CTypingNotificationSignal *>(s);
      if (sig->Active())
        MSNSendTypingNotification(sig->Id(), sig->CID());
      break;
    }
    
    case PROTOxSENDxMSG:
    {
      CSendMessageSignal *sig = static_cast<CSendMessageSignal *>(s);
      MSNSendMessage(sig->Id(), sig->Message(), sig->Thread(), sig->CID());
      break;
    }

    case PROTOxSENDxGRANTxAUTH:
    {
      CGrantAuthSignal *sig = static_cast<CGrantAuthSignal *>(s);
      MSNGrantAuth(sig->Id());
      break;
    }

    case PROTOxSENDxREFUSExAUTH:
    {
      CRefuseAuthSignal *sig = static_cast<CRefuseAuthSignal *>(s);
      break;
    }

    case PROTOxREQUESTxINFO:
    {
      CRequestInfo *sig = static_cast<CRequestInfo *>(s);
      break;
    }

    case PROTOxUPDATExINFO:
    {
      CUpdateInfoSignal *sig = static_cast<CUpdateInfoSignal *>(s);
      MSNUpdateUser(sig->Alias());
      break;
    }

    default:
      break;  //Do nothing now...
  }

  delete s;
}
