/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2013 Licq developers <licq-dev@googlegroups.com>
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
#include <sys/time.h>
#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <vector>

#include <licq/logging/log.h>
#include <licq/socket.h>
#include <licq/conversation.h>
#include <licq/event.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/protocolsignal.h>

#include "msn.h"
#include "msnpacket.h"
#include "owner.h"
#include "pluginversion.h"
#include "user.h"

using namespace std;
using namespace LicqMsn;
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


CMSN::CMSN(Licq::ProtocolPlugin::Params& p)
  : Licq::ProtocolPlugin(p),
    myServerSocket(NULL),
    mySslSocket(NULL),
    m_vlPacketBucket(211)
{
  m_bWaitingPingReply = m_bCanPing = false;
  m_pPacketBuf = 0;
  m_pSSLPacket = 0;
  myStatus = Licq::User::OfflineStatus;
  myPassword = "";
  m_nSessionStart = 0;

}

CMSN::~CMSN()
{
  if (m_pPacketBuf)
    delete m_pPacketBuf;
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

void CMSN::StorePacket(SBuffer *_pBuf, int _nSock)
{
  if (_pBuf->m_bStored == false)
  {
    BufferList &b = m_vlPacketBucket[HashValue(_nSock)];
    b.push_front(_pBuf);
  }
}

void CMSN::RemovePacket(const Licq::UserId& userId, int _nSock, int nSize)
{
  BufferList &b = m_vlPacketBucket[HashValue(_nSock)];
  BufferList::iterator it;
  SBuffer *pNewBuf = 0;
  int nNewSize = 0;

  for (it = b.begin(); it != b.end(); it++)
  {
    if ((*it)->myUserId == userId)
    {
      // Found a packet that has part of another packet at the end
      // so we have to save it and put it back on the queue.
      if (nSize)
      {
	nNewSize = (*it)->m_pBuf->getDataSize() - nSize;
	if (nNewSize)
	{
	  pNewBuf = new SBuffer;
	  pNewBuf->myUserId = userId;
	  pNewBuf->m_pBuf = new CMSNBuffer(nNewSize);
	  pNewBuf->m_pBuf->packRaw((*it)->m_pBuf->getDataStart()+nSize, nNewSize);
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
}

SBuffer *CMSN::RetrievePacket(const Licq::UserId& userId, int _nSock)
{
  BufferList &b = m_vlPacketBucket[HashValue(_nSock)];
  BufferList::iterator it;
  for (it = b.begin(); it != b.end(); it++)
  {
    if ((*it)->myUserId == userId)
      return *it;
  }

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

void CMSN::HandlePacket(Licq::TCPSocket* sock, CMSNBuffer& packet, const Licq::UserId& userId)
{
  int _nSocket = sock->Descriptor();
  SBuffer* pBuf = RetrievePacket(userId, _nSocket);
  bool bProcess = false;

  if (pBuf)
    *(pBuf->m_pBuf) += packet;
  else
  {
    pBuf = new SBuffer;
    pBuf->m_pBuf = new CMSNBuffer(packet);
    pBuf->myUserId = userId;
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
            pPart->packRaw(pBuf->m_pBuf->getDataStart(), nFullSize);
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
            pPart->packRaw(pBuf->m_pBuf->getDataStart(), nFullSize);
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
      if (sock == myServerSocket)
        ProcessServerPacket(pPart ? pPart : pBuf->m_pBuf);
      else
        ProcessSBPacket(userId, pPart ? pPart : pBuf->m_pBuf, sock);
      RemovePacket(userId, _nSocket, nFullSize);
      if (pPart)
        delete pPart;
      else
        delete pBuf;
      pBuf = RetrievePacket(userId, _nSocket);
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
  // Ping server every 60s
  myMainLoop.addTimeout(60*1000, this, 0, false);

  myMainLoop.addRawFile(getReadPipe(), this);
  myMainLoop.run();

  // Close out now
  MSNLogoff();
  return 0;
}

void CMSN::destructor()
{
  delete this;
}

Licq::User* CMSN::createUser(const Licq::UserId& id, bool temporary)
{
  return new User(id, temporary);
}

Licq::Owner* CMSN::createOwner(const Licq::UserId& id)
{
  return new Owner(id);
}

void CMSN::rawFileEvent(int fd, int /*revents*/)
{
  char c;
  read(fd, &c, 1);
  switch (c)
  {
    case Licq::ProtocolPlugin::PipeSignal:
    {
      Licq::ProtocolSignal* s = popSignal();
      ProcessSignal(s);
      break;
    }

    case Licq::ProtocolPlugin::PipeShutdown:
    gLog.info("Exiting");
    myMainLoop.quit();
    break;
  }
}

void CMSN::ProcessSignal(Licq::ProtocolSignal* s)
{
  if (myServerSocket == NULL && s->signal() != Licq::ProtocolSignal::SignalLogon)
  {
    delete s;
    return;
  }

  switch (s->signal())
  {
    case Licq::ProtocolSignal::SignalLogon:
    {
      if (myServerSocket == NULL)
      {
        Licq::ProtoLogonSignal* sig = dynamic_cast<Licq::ProtoLogonSignal*>(s);
        Logon(sig->userId(), sig->status());
      }
      break;
    }
    case Licq::ProtocolSignal::SignalChangeStatus:
    {
      Licq::ProtoChangeStatusSignal* sig = dynamic_cast<Licq::ProtoChangeStatusSignal*>(s);
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
      Licq::ProtoAddUserSignal* sig = dynamic_cast<Licq::ProtoAddUserSignal*>(s);
      MSNAddUser(sig->userId());
      break;
    }
    case Licq::ProtocolSignal::SignalRemoveUser:
    {
      Licq::ProtoRemoveUserSignal* sig = dynamic_cast<Licq::ProtoRemoveUserSignal*>(s);
      MSNRemoveUser(sig->userId());
      break;
    }
    case Licq::ProtocolSignal::SignalRenameUser:
    {
      Licq::ProtoRenameUserSignal* sig = dynamic_cast<Licq::ProtoRenameUserSignal*>(s);
      MSNRenameUser(sig->userId());
      break;
    }
    case Licq::ProtocolSignal::SignalNotifyTyping:
    {
      Licq::ProtoTypingNotificationSignal* sig = dynamic_cast<Licq::ProtoTypingNotificationSignal*>(s);
      if (sig->active())
        MSNSendTypingNotification(sig->userId(), sig->convoId());
      break;
    }
    case Licq::ProtocolSignal::SignalSendMessage:
    {
      Licq::ProtoSendMessageSignal* sig = dynamic_cast<Licq::ProtoSendMessageSignal*>(s);
      MSNSendMessage(sig->eventId(), sig->userId(), sig->message(), sig->callerThread(), sig->convoId());
      break;
    }
    case Licq::ProtocolSignal::SignalGrantAuth:
    {
      Licq::ProtoGrantAuthSignal* sig = dynamic_cast<Licq::ProtoGrantAuthSignal*>(s);
      MSNGrantAuth(sig->userId());
      Licq::gPluginManager.pushPluginEvent(new Licq::Event(s));
      break;
    }
    case Licq::ProtocolSignal::SignalUpdateInfo:
    {
      string newAlias;
      {
        Licq::OwnerReadGuard o(s->userId());
        if (!o.isLocked())
          newAlias = o->getAlias();
      }
      MSNUpdateUser(newAlias);
      Licq::gPluginManager.pushPluginEvent(new Licq::Event(s));
      break;
    }
    case Licq::ProtocolSignal::SignalBlockUser:
    {
      Licq::ProtoBlockUserSignal* sig = dynamic_cast<Licq::ProtoBlockUserSignal*>(s);
      MSNBlockUser(sig->userId());
      break;
    }
    case Licq::ProtocolSignal::SignalUnblockUser:
    {
      Licq::ProtoUnblockUserSignal* sig = dynamic_cast<Licq::ProtoUnblockUserSignal*>(s);
      MSNUnblockUser(sig->userId());
      break;
    }
   
    default:
    {
      /* Unsupported action, if it has an eventId, cancel it */
      if (s->eventId() != 0)
        Licq::gPluginManager.pushPluginEvent(
            new Licq::Event(s, Licq::Event::ResultUnsupported));
      break;
    }
  }

  delete s;
}

void CMSN::socketEvent(Licq::INetSocket* inetSocket, int /*revents*/)
{
  Licq::TCPSocket* sock = dynamic_cast<Licq::TCPSocket*>(inetSocket);
  assert(sock != NULL);

  CMSNBuffer packet;
  bool recok = sock->receive(packet);

  if (sock == myServerSocket)
  {
    if (recok)
      HandlePacket(myServerSocket, packet, myOwnerId);
    else
    {
      // Time to reconnect
      gLog.info("Disconnected from server, reconnecting");
      sleep(1);
      closeSocket(myServerSocket, false);
      myServerSocket = NULL;
      Logon(myOwnerId, myStatus);
    }
  }

  else if (sock == mySslSocket)
  {
    if (recok)
      ProcessSSLServerPacket(packet);
  }

  else
  {
    //SB socket
    if (recok)
      HandlePacket(sock, packet, sock->userId());
    else
    {
      // Sometimes SB just drops connection without sending any BYE for the user(s) first
      // This seems to happen when other user is offical client
      killConversation(sock);
      closeSocket(sock);
    }
  }
}

void CMSN::closeSocket(Licq::TCPSocket* sock, bool clearUser)
{
  myMainLoop.removeSocket(sock);
  sock->CloseConnection();

  if (clearUser)
  {
    Licq::UserWriteGuard u(sock->userId());
    if (u.isLocked())
    {
      u->clearSocketDesc(sock);
      if (u->OfflineOnDisconnect())
        u->statusChanged(Licq::User::OfflineStatus);
    }
  }

  delete sock;
}

void CMSN::WaitDataEvent(CMSNDataEvent *_pEvent)
{
  m_lMSNEvents.push_back(_pEvent);
}

bool CMSN::RemoveDataEvent(CMSNDataEvent *pData)
{
  list<CMSNDataEvent *>::iterator it;
  for (it = m_lMSNEvents.begin(); it != m_lMSNEvents.end(); it++)
  {
    if ((*it)->userId() == pData->userId() &&
	(*it)->getSocket() == pData->getSocket())
    {
      // Close the socket
      int sockFd = pData->getSocket()->Descriptor();
      closeSocket(pData->getSocket());

      Licq::Conversation* convo = gConvoManager.getFromSocket(sockFd);
      if (convo != NULL)
        gConvoManager.remove(convo->id());

      m_lMSNEvents.erase(it);
      delete pData;
      pData = 0;
      break;
    }
  }

  return (pData == 0);
}

CMSNDataEvent* CMSN::FetchDataEvent(const Licq::UserId& userId, Licq::TCPSocket* sock)
{
  CMSNDataEvent *pReturn = 0;
  list<CMSNDataEvent *>::iterator it;
  for (it = m_lMSNEvents.begin(); it != m_lMSNEvents.end(); it++)
  {
    if ((*it)->userId() == userId && (*it)->getSocket() == sock)
    {
      pReturn = *it;
      break;
    }
  }

  if (!pReturn)
  {
    pReturn = FetchStartDataEvent(userId);
    if (pReturn)
      pReturn->setSocket(sock);
  }

  return pReturn;
}

CMSNDataEvent* CMSN::FetchStartDataEvent(const Licq::UserId& userId)
{
  CMSNDataEvent *pReturn = 0;
  list<CMSNDataEvent *>::iterator it;
  for (it = m_lMSNEvents.begin(); it != m_lMSNEvents.end(); it++)
  {
    if ((*it)->userId() == userId && (*it)->getSocket() == NULL)
    {
      pReturn = *it;
      break;
    }
  }

  return pReturn;  
}
