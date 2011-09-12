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

#include <cctype>
#include <cstdio>
#include <cstring>
#include <list>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <vector>

#include <licq/logging/log.h>
#include <licq/socket.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/conversation.h>
#include <licq/event.h>
#include <licq/inifile.h>
#include <licq/protocolsignal.h>

#include "msn.h"
#include "msnpacket.h"
#include "pluginversion.h"

using namespace std;
using Licq::gConvoManager;
using Licq::gLog;

#ifndef HAVE_STRNDUP

#include <cstdlib>
#include <cstring>

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
Licq::SocketManager gSocketMan;

void *MSNPing_tep(void *);


CMSN::CMSN(Licq::ProtocolPlugin::Params& p)
  : Licq::ProtocolPlugin(p),
    m_vlPacketBucket(211)
{
  m_bExit = false;
  m_bWaitingPingReply = m_bCanPing = false;
  m_nSSLSocket = m_nServerSocket = m_nNexusSocket = -1;
  m_pPacketBuf = 0;
  m_pNexusBuff = 0;
  m_pSSLPacket = 0;
  myStatus = Licq::User::OfflineStatus;
  m_szUserName = 0;
  myPassword = "";
  m_nSessionStart = 0;

}

CMSN::~CMSN()
{
  if (m_pPacketBuf)
    delete m_pPacketBuf;
  if (m_szUserName)
    free(m_szUserName);

  saveConfig();
}

std::string CMSN::name() const
{
  return "MSN";
}

std::string CMSN::version() const
{
  return PLUGIN_VERSION_STRING;
}

unsigned long CMSN::protocolId() const
{
  return MSN_PPID;
}

unsigned long CMSN::capabilities() const
{
  return Licq::ProtocolPlugin::CanSendMsg |
      Licq::ProtocolPlugin::CanSendAuth |
      Licq::ProtocolPlugin::CanSendAuthReq;
}

std::string CMSN::defaultServerHost() const
{
  return "messenger.hotmail.com";
}

int CMSN::defaultServerPort() const
{
  return 1863;
}

bool CMSN::init(int, char**)
{
  return true;
}


void CMSN::saveConfig()
{
  // Config file
  Licq::IniFile msnConf("licq_msn.conf");
  msnConf.loadFile();
  msnConf.setSection("network");
  msnConf.set("ListVersion", m_nListVersion);
  msnConf.writeFile();
}

void CMSN::StorePacket(SBuffer *_pBuf, int _nSock)
{
  if (_pBuf->m_bStored == false)
  {
    pthread_mutex_lock(&mutex_Bucket);
    BufferList &b = m_vlPacketBucket[HashValue(_nSock)];
    b.push_front(_pBuf);
    pthread_mutex_unlock(&mutex_Bucket);
  }
}

void CMSN::RemovePacket(const string& _strUser, int _nSock, int nSize)
{
  pthread_mutex_lock(&mutex_Bucket);
  BufferList &b = m_vlPacketBucket[HashValue(_nSock)];
  BufferList::iterator it;
  SBuffer *pNewBuf = 0;
  int nNewSize = 0;

  for (it = b.begin(); it != b.end(); it++)
  {
    if ((*it)->m_strUser == _strUser)
    {
      // Found a packet that has part of another packet at the end
      // so we have to save it and put it back on the queue.
      if (nSize)
      {
	nNewSize = (*it)->m_pBuf->getDataSize() - nSize;
	if (nNewSize)
	{
	  pNewBuf = new SBuffer;
	  pNewBuf->m_strUser = _strUser;
	  pNewBuf->m_pBuf = new CMSNBuffer(nNewSize);
	  pNewBuf->m_pBuf->Pack((*it)->m_pBuf->getDataStart()+nSize,
				nNewSize);
	  pNewBuf->m_bStored = true;
	}			   
      }
      
      b.erase(it);
      break;
    }
  }

  // Now add it here
  if (pNewBuf)
    b.push_front(pNewBuf);
  pthread_mutex_unlock(&mutex_Bucket);
}

SBuffer *CMSN::RetrievePacket(const string &_strUser, int _nSock)
{
  pthread_mutex_lock(&mutex_Bucket);
  BufferList &b = m_vlPacketBucket[HashValue(_nSock)];
  BufferList::iterator it;
  for (it = b.begin(); it != b.end(); it++)
  {
    if ((*it)->m_strUser == _strUser)
    {
      pthread_mutex_unlock(&mutex_Bucket);
      return *it;
    }
  }
  pthread_mutex_unlock(&mutex_Bucket);

  return 0;
}
  
Licq::Event *CMSN::RetrieveEvent(unsigned long _nTag)
{
  Licq::Event *e = 0;

  list<Licq::Event*>::iterator it;
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

void CMSN::HandlePacket(int _nSocket, CMSNBuffer &packet, const char* _szUser)
{
  SBuffer *pBuf = RetrievePacket(_szUser, _nSocket);
  bool bProcess = false;

  if (pBuf)
    *(pBuf->m_pBuf) += packet;
  else
  {
    pBuf = new SBuffer;
    pBuf->m_pBuf = new CMSNBuffer(packet);
    pBuf->m_strUser = _szUser;
    pBuf->m_bStored = false;
  }

  do
  {
    char *szNeedle;
    CMSNBuffer *pPart = 0;
    unsigned long nFullSize = 0;
    bProcess = false;

    if ((szNeedle = strstr((char *)pBuf->m_pBuf->getDataStart(),
                           "\r\n")))
    {
      bool isMSG = (memcmp(pBuf->m_pBuf->getDataStart(), "MSG", 3) == 0);

      if (/*memcmp(pBuf->m_pBuf->getDataStart(), "MSG", 3) == 0*/ isMSG ||
          memcmp(pBuf->m_pBuf->getDataStart(), "NOT", 3) == 0)
      {
        pBuf->m_pBuf->SkipParameter(); // MSG, NOT
        if (isMSG)
        {
          pBuf->m_pBuf->SkipParameter(); // Hotmail
          pBuf->m_pBuf->SkipParameter(); // Hotmail
        }
        string strSize = pBuf->m_pBuf->GetParameter();
        int nSize = atoi(strSize.c_str());

        // Cut the packet instead of passing it all along
        // we might receive 1 full packet and part of another.
        if (nSize <= (pBuf->m_pBuf->getDataPosWrite() - pBuf->m_pBuf->getDataPosRead()))
        {
          nFullSize = nSize + pBuf->m_pBuf->getDataPosRead() - pBuf->m_pBuf->getDataStart() + 1;
          if (pBuf->m_pBuf->getDataSize() > nFullSize)
          {
            // Hack to save the part and delete the rest
            if (pBuf->m_bStored == false)
            {
              StorePacket(pBuf, _nSocket);
              pBuf->m_bStored = true;
            }

            // We have a packet, with part of another one at the end
            pPart = new CMSNBuffer(nFullSize);
            pPart->Pack(pBuf->m_pBuf->getDataStart(), nFullSize);
          }
          bProcess = true;
        }
      }
      else //if(memcmp(pBuf->m_pBuf->getDataStart(), "ACK", 3) == 0)
      {
        int nSize = szNeedle - pBuf->m_pBuf->getDataStart() +2;

        // Cut the packet instead of passing it all along
        // we might receive 1 full packet and part of another.
        if (nSize <= (pBuf->m_pBuf->getDataPosWrite() - pBuf->m_pBuf->getDataPosRead()))
        {
          nFullSize = nSize + pBuf->m_pBuf->getDataPosRead() - pBuf->m_pBuf->getDataStart();

          if (pBuf->m_pBuf->getDataSize() > nFullSize)
          {
            // Hack to save the part and delete the rest
            if (pBuf->m_bStored == false)
            {
              StorePacket(pBuf, _nSocket);
              pBuf->m_bStored = true;
            }

            // We have a packet, with part of another one at the end
            pPart = new CMSNBuffer(nFullSize);
            pPart->Pack(pBuf->m_pBuf->getDataStart(), nFullSize);
          }
          bProcess = true;
        }
      }

      if (!bProcess)
      {
        // Save it
        StorePacket(pBuf, _nSocket);
        pBuf->m_bStored = true;
      }

      pBuf->m_pBuf->Reset();
    }
    else
    {
      // Need to save it, doens't have much data yet
      StorePacket(pBuf, _nSocket);
      pBuf->m_bStored = true;
      bProcess = false;
    }

    if (bProcess)
    {
      // Handle it, and then remove it from the queue
      if (m_nServerSocket == _nSocket)
        ProcessServerPacket(pPart ? pPart : pBuf->m_pBuf);
      else
        ProcessSBPacket(const_cast<char *>(_szUser), pPart ? pPart : pBuf->m_pBuf,
                        _nSocket);
      RemovePacket(_szUser, _nSocket, nFullSize);
      if (pPart)
        delete pPart;
      else
        delete pBuf;
      pBuf = RetrievePacket(_szUser, _nSocket);
    }
    else
      pBuf = 0;

  } while (pBuf);
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
  Licq::Conversation* convo = gConvoManager.getFromSocket(_nSocket);
  return (convo != NULL ? convo->id() : 0);
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

int CMSN::run()
{
  int nNumDesc;
  int nCurrent; 
  fd_set f;

  // Config file
  Licq::IniFile msnConf("licq_msn.conf");
  msnConf.loadFile();
  msnConf.setSection("network");
  msnConf.get("ListVersion", m_nListVersion, 0);

  pthread_mutex_init(&mutex_StartList, 0);
  pthread_mutex_init(&mutex_MSNEventList, 0);
  pthread_mutex_init(&mutex_ServerSocket, 0);
  pthread_mutex_init(&mutex_Bucket, 0);

  int nResult = pthread_create(&m_tMSNPing, NULL, &MSNPing_tep, this);
  if (nResult)
  {
    gLog.error("Unable to start ping thread: %s", strerror(nResult));
  }

  int m_nPipe = getReadPipe();
  nResult = 0;
  
  while (!m_bExit)
  {
    pthread_mutex_lock(&mutex_ServerSocket);
    FD_ZERO(&f);
    f = gSocketMan.socketSet();
    nNumDesc = gSocketMan.LargestSocket() + 1;
 
    if (m_nPipe != -1)
    {
      FD_SET(m_nPipe, &f);
      if (m_nPipe >= nNumDesc)
        nNumDesc = m_nPipe + 1;
    }

    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    nResult = select(nNumDesc, &f, NULL, NULL, &tv);
    pthread_mutex_unlock(&mutex_ServerSocket);
  
    if (nResult == 0)
    {
      // We need to call select to get a context switch to make
      // the ping thread be notified that the mutex has been unlocked.
      tv.tv_sec = 1; tv.tv_usec = 0;
      select(0, NULL, NULL, NULL, &tv);
    }

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
          Licq::INetSocket* s = gSocketMan.FetchSocket(m_nServerSocket);
          Licq::TCPSocket* sock = static_cast<Licq::TCPSocket*>(s);
          if (sock->RecvRaw())
          {
            CMSNBuffer packet(sock->RecvBuffer());
            sock->ClearRecvBuffer();
            gSocketMan.DropSocket(sock);

            HandlePacket(m_nServerSocket, packet, m_szUserName);
          }
          else
          {
            // Time to reconnect
            gLog.info("Disconnected from server, reconnecting");
            sleep(1);
            int nSD = m_nServerSocket;
            m_nServerSocket = -1;
            gSocketMan.DropSocket(sock);
            gSocketMan.CloseSocket(nSD);
            Logon(myStatus);
          }
        }
        
        else if (nCurrent == m_nNexusSocket)
        {
          Licq::INetSocket* s = gSocketMan.FetchSocket(m_nNexusSocket);
          Licq::TCPSocket* sock = static_cast<Licq::TCPSocket*>(s);
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
          Licq::INetSocket* s = gSocketMan.FetchSocket(m_nSSLSocket);
          Licq::TCPSocket* sock = static_cast<Licq::TCPSocket*>(s);
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
          Licq::INetSocket* s = gSocketMan.FetchSocket(nCurrent);
          Licq::TCPSocket* sock = static_cast<Licq::TCPSocket*>(s);
          if (sock && sock->RecvRaw())
          {
            CMSNBuffer packet(sock->RecvBuffer());
            sock->ClearRecvBuffer();
            char *szUser = strdup(sock->userId().accountId().c_str());
            gSocketMan.DropSocket(sock);

            HandlePacket(nCurrent, packet, szUser);
            
	    free(szUser);
	  }
	  else
	  {
            // Sometimes SB just drops connection without sending any BYE for the user(s) first
            // This seems to happen when other user is offical client
	    if (sock)
	      gSocketMan.DropSocket(sock);
	    gSocketMan.CloseSocket(nCurrent);

            // Clean up any conversations that was associated with the socket
            killConversation(nCurrent);
	  }
	}
      }

      nCurrent++;
    }
  }
  
  // Close out now
  pthread_cancel(m_tMSNPing);
  MSNLogoff();
  pthread_join(m_tMSNPing, NULL);
  return 0;
}

void CMSN::destructor()
{
  delete this;
}

void CMSN::ProcessPipe()
{
  char buf[16];
  read(getReadPipe(), buf, 1);
  switch (buf[0])
  {
    case Licq::ProtocolPlugin::PipeSignal:
    {
      Licq::ProtocolSignal* s = popSignal();
      ProcessSignal(s);
      break;
    }

    case Licq::ProtocolPlugin::PipeShutdown:
    gLog.info("Exiting");
    m_bExit = true;
    break;
  }
}

void CMSN::ProcessSignal(Licq::ProtocolSignal* s)
{
  if (m_nServerSocket < 0 && s->signal() != Licq::ProtocolSignal::SignalLogon)
  {
    delete s;
    return;
  }

  switch (s->signal())
  {
    case Licq::ProtocolSignal::SignalLogon:
    {
      if (m_nServerSocket < 0)
      {
        Licq::ProtoLogonSignal* sig = static_cast<Licq::ProtoLogonSignal*>(s);
        Logon(sig->status());
      }
      break;
    }
    case Licq::ProtocolSignal::SignalChangeStatus:
    {
      Licq::ProtoChangeStatusSignal* sig = static_cast<Licq::ProtoChangeStatusSignal*>(s);
      MSNChangeStatus(sig->status());
      break;
    }
    case Licq::ProtocolSignal::SignalLogoff:
    {
      MSNLogoff();
      break;
    }
    case Licq::ProtocolSignal::SignalAddUser:
    {
      Licq::ProtoAddUserSignal* sig = static_cast<Licq::ProtoAddUserSignal*>(s);
      MSNAddUser(sig->userId());
      break;
    }
    case Licq::ProtocolSignal::SignalRemoveUser:
    {
      Licq::ProtoRemoveUserSignal* sig = static_cast<Licq::ProtoRemoveUserSignal*>(s);
      MSNRemoveUser(sig->userId());
      break;
    }
    case Licq::ProtocolSignal::SignalRenameUser:
    {
      Licq::ProtoRenameUserSignal* sig = static_cast<Licq::ProtoRenameUserSignal*>(s);
      MSNRenameUser(sig->userId());
      break;
    }
    case Licq::ProtocolSignal::SignalNotifyTyping:
    {
      Licq::ProtoTypingNotificationSignal* sig = static_cast<Licq::ProtoTypingNotificationSignal*>(s);
      if (sig->active())
        MSNSendTypingNotification(sig->userId(), sig->convoId());
      break;
    }
    case Licq::ProtocolSignal::SignalSendMessage:
    {
      Licq::ProtoSendMessageSignal* sig = static_cast<Licq::ProtoSendMessageSignal*>(s);
      MSNSendMessage(sig->eventId(), sig->userId(), sig->message(), sig->callerThread(), sig->convoId());
      break;
    }
    case Licq::ProtocolSignal::SignalGrantAuth:
    {
      Licq::ProtoGrantAuthSignal* sig = static_cast<Licq::ProtoGrantAuthSignal*>(s);
      MSNGrantAuth(sig->userId());
      break;
    }
    case Licq::ProtocolSignal::SignalRefuseAuth:
    {
//      Licq::ProtoRefuseAuthSignal* sig = static_cast<Licq::ProtoRefuseAuthSignal*>(s);
      break;
    }
    case Licq::ProtocolSignal::SignalRequestInfo:
    {
//      Licq::ProtoRequestInfo* sig = static_cast<Licq::ProtoRequestInfo*>(s);
      break;
    }
    case Licq::ProtocolSignal::SignalUpdateInfo:
    {
      string newAlias;
      {
        Licq::OwnerReadGuard o(MSN_PPID);
        if (o.isLocked())
          newAlias = o->getAlias();
      }
      MSNUpdateUser(newAlias);
      break;
    }
    case Licq::ProtocolSignal::SignalBlockUser:
    {
      Licq::ProtoBlockUserSignal* sig = static_cast<Licq::ProtoBlockUserSignal*>(s);
      MSNBlockUser(sig->userId());
      break;
    }
    case Licq::ProtocolSignal::SignalUnblockUser:
    {
      Licq::ProtoUnblockUserSignal* sig = static_cast<Licq::ProtoUnblockUserSignal*>(s);
      MSNUnblockUser(sig->userId());
      break;
    }
   
    default:
      break;  //Do nothing now...
  }

  delete s;
}

void CMSN::WaitDataEvent(CMSNDataEvent *_pEvent)
{
  pthread_mutex_lock(&mutex_MSNEventList);
  m_lMSNEvents.push_back(_pEvent);
  pthread_mutex_unlock(&mutex_MSNEventList);
}

bool CMSN::RemoveDataEvent(CMSNDataEvent *pData)
{
  list<CMSNDataEvent *>::iterator it;
  pthread_mutex_lock(&mutex_MSNEventList);
  for (it = m_lMSNEvents.begin(); it != m_lMSNEvents.end(); it++)
  {
    if ((*it)->getUser() == pData->getUser() &&
	(*it)->getSocket() == pData->getSocket())
    {
      // Close the socket
      gSocketMan.CloseSocket(pData->getSocket());

      Licq::Conversation* convo = gConvoManager.getFromSocket(pData->getSocket());
      if (convo != NULL)
        gConvoManager.remove(convo->id());

      m_lMSNEvents.erase(it);
      delete pData;
      pData = 0;
      break;
    }
  }
  pthread_mutex_unlock(&mutex_MSNEventList);

  return (pData == 0);
}

CMSNDataEvent *CMSN::FetchDataEvent(const string &_strUser, int _nSocket)
{
  CMSNDataEvent *pReturn = 0;
  list<CMSNDataEvent *>::iterator it;
  pthread_mutex_lock(&mutex_MSNEventList);
  for (it = m_lMSNEvents.begin(); it != m_lMSNEvents.end(); it++)
  {
    if ((*it)->getUser() == _strUser && (*it)->getSocket() == _nSocket)
    {
      pReturn = *it;
      break;
    }
  }

  if (!pReturn)
  {
    pReturn = FetchStartDataEvent(_strUser);
    if (pReturn)
      pReturn->setSocket(_nSocket);
  }
  pthread_mutex_unlock(&mutex_MSNEventList);

  return pReturn;
}

CMSNDataEvent *CMSN::FetchStartDataEvent(const string &_strUser)
{
  CMSNDataEvent *pReturn = 0;
  list<CMSNDataEvent *>::iterator it;
  for (it = m_lMSNEvents.begin(); it != m_lMSNEvents.end(); it++)
  {
    if ((*it)->getUser() == _strUser && (*it)->getSocket() == -1)
    {
      pReturn = *it;
      break;
    }
  }

  return pReturn;  
}
