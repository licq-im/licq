/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "oscarservice.h"

#include <boost/scoped_array.hpp>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <licq/buffer.h>
#include <licq/contactlist/usermanager.h>
#include <licq/byteorder.h>
#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/proxy.h>
#include <licq/logging/log.h>

#include "gettext.h"
#include "icq.h"
#include "packet-srv.h"
#include "socket.h"
#include "user.h"

using namespace LicqIcq;
using Licq::gLog;
using Licq::gDaemon;
using std::string;

COscarService::COscarService(unsigned short Fam)
{
  myFam = Fam;
  mySocketDesc = -1;
  myProxy = NULL;
  myStatus = STATUS_UNINITIALIZED;
  pthread_mutex_init(&mutex_sendqueue, NULL);
  pthread_cond_init(&cond_sendqueue, NULL);
  pthread_mutex_init(&mutex_status, NULL);
  pthread_cond_init(&cond_status, NULL);
}

COscarService::~COscarService()
{
  if (myProxy) delete myProxy;
}

void COscarService::ChangeStatus(EOscarServiceStatus s)
{
  pthread_mutex_lock(&mutex_status);
  myStatus = s;
  pthread_cond_signal(&cond_status);
  pthread_mutex_unlock(&mutex_status);
}

bool COscarService::WaitForStatus(EOscarServiceStatus s)
{
  pthread_mutex_lock(&mutex_status);

  struct timespec ts;
  ts.tv_nsec = 0;
  //wait for 120 seconds
  ts.tv_sec = time(NULL) + 120;
  
  if (pthread_cond_timedwait(&cond_status, &mutex_status, &ts) == ETIMEDOUT)
  {
    pthread_mutex_unlock(&mutex_status);
    return false;
  }
  if (myStatus == s)
  {
    pthread_mutex_unlock(&mutex_status);
    return true;
  }

  pthread_mutex_unlock(&mutex_status);
  return false;
}

void COscarService::setConnectCredential(const string& server,
    unsigned short port, const string& cookie)
{
  myServer = server;
  myPort = port;
  myCookie = cookie;
}

bool COscarService::SendPacket(CPacket *p)
{
  Licq::INetSocket* s = gSocketManager.FetchSocket(mySocketDesc);
  if (s == NULL) return false;
  Licq::Buffer* b = p->Finalize(s);
  if (!s->send(*b))
  {
    gLog.warning(tr("Error sending event (FAM #%02X, Subtype #%02X, Sequence #%hu): %s."),
        (unsigned short)((p->SNAC() >> 16) & 0xffff), (unsigned short)(p->SNAC() & 0xffff),
        p->Sequence(), s->errorStr().c_str());
    gSocketManager.DropSocket(s);
    delete b;
    return false;
  }
  gSocketManager.DropSocket(s);
  delete b;
  return true;
}

void COscarService::ClearQueue()
{
  pthread_mutex_lock(&mutex_sendqueue);
  std::list<Licq::Event*>::iterator iter;
  unsigned long i = mySendQueue.size();
  for (iter = mySendQueue.begin(); i > 0; i--)
  {
    Licq::Event* e = *iter;
    mySendQueue.erase(iter);
    if (e != NULL)
    {
      gLog.info(tr("Event #%hu is still on the service 0x%02X queue!"), e->Sequence(), myFam);
      delete e;
    }
  }
  pthread_mutex_unlock(&mutex_sendqueue);
}

void COscarService::SendEvent(pthread_t caller, unsigned long eventId,
    const Licq::UserId& userId, unsigned short SubType, bool Request)
{
  Licq::Event* e = new Licq::Event(caller, eventId, mySocketDesc, NULL, Licq::Event::ConnectServer, userId);
  e->SetSubType(SubType);
  if (Request)
    gIcqProtocol.PushEvent(e);
  else
    e->SetNoAck(true);
  pthread_mutex_lock(&mutex_sendqueue);
  mySendQueue.push_back(e);
  pthread_cond_signal(&cond_sendqueue);
  pthread_mutex_unlock(&mutex_sendqueue);
}

bool COscarService::SendBARTFam(Licq::Event* e)
{
  switch (e->SubType())
  {
    case ICQ_SNACxBART_DOWNLOADxREQUEST:
    {
      CPU_RequestBuddyIcon* p;
      {
        UserReadGuard u(e->userId());
        if (!u.isLocked())
          return false;
        p = new CPU_RequestBuddyIcon(u->accountId(),
            u->buddyIconType(), u->buddyIconHashType(), u->buddyIconHash(), myFam);
        gLog.info(tr("Requesting buddy icon for %s (#%hu/#%d)..."),
            u->getAlias().c_str(), p->Sequence(), p->SubSequence());
      }
      e->AttachPacket(p);
      return (SendPacket(p));
    }
    
    default:
      gLog.warning(tr("Event with unsupported subtype (%02X) for FAM %02X failed."),
          e->SubType(), myFam);
      return false;
  }

  return false;
}

bool COscarService::ProcessPacket(Buffer& packet)
{
  unsigned short Len;
  unsigned short Sequence;
  char startCode, Channel;

  // read in the serveice header info
  packet >> startCode;

  if (startCode != 0x2a)
  {
    gLog.warning(tr("bad start code %d for packet in socket of service 0x%02X."),
        startCode, myFam);
    return false;
  }

  packet >> Channel
         >> Sequence
         >> Len;

  Sequence = BSWAP_16(Sequence);
  Len = BSWAP_16(Len);

  switch (Channel)
  {
    case ICQ_CHNxNEW:
      ProcessNewChannel(packet);
      break;

    case ICQ_CHNxDATA:
      ProcessDataChannel(packet);
      break;

    case ICQ_CHNxCLOSE:
      gLog.info(tr("Server send us request for close service 0x%02X."), myFam);
      return false;
      break;

    default:
      gLog.warning(tr("Packet from unhandled channel %02x for service 0x%02X."),
          Channel, myFam);
      break;
  }

  return true;
}

void COscarService::ProcessNewChannel(Buffer& packet)
{
  unsigned long Version = packet.UnpackUnsignedLongBE();
  
  if (Version != 0x00000001)
  {
    gLog.warning(tr("Packet with wrong version (0x%08lx) from new channel for service 0x%02X."),
        Version, myFam);
  }
}
              
void COscarService::ProcessDataChannel(Buffer& packet)
{
  unsigned short Family, SubType, Flags;
  unsigned long RequestId;

  packet >> Family >> SubType >> Flags >> RequestId;
  Family = BSWAP_16(Family);
  SubType = BSWAP_16(SubType);
  Flags = BSWAP_16(Flags);
  RequestId = BSWAP_32(RequestId);

  if (Flags & 0x8000) // version of the family that this SNAC, just ignore it
  {
    unsigned short len = packet.UnpackUnsignedShortBE();
    packet.incDataPosRead(len);
  }

  switch (Family)
  {
    case ICQ_SNACxFAM_SERVICE:
      ProcessServiceFam(packet, SubType, RequestId);
      break;

    case ICQ_SNACxFAM_BART:
      if (myFam == ICQ_SNACxFAM_BART)
        ProcessBARTFam(packet, SubType, RequestId);
      else
        gLog.warning(tr("Unsupported family %04hx on data channel of service %02X."),
            Family, myFam);
      break;

    default:
      gLog.warning(tr("Unknown or usupported family %04hx on data channel of service %02X."),
          Family, myFam);
      break;
  }
}

void COscarService::ProcessServiceFam(Buffer& packet, unsigned short SubType,
                                      unsigned long RequestId)
{
  switch (SubType)
  {
    case ICQ_SNACxSUB_ERROR:
    {
      unsigned short err = packet.UnpackUnsignedShortBE();
      unsigned short suberr = 0;
      
      packet.readTLV();
      if (packet.getTLVLen(0x0008) == 2)
        suberr = packet.UnpackUnsignedShortTLV(0x0008);
      gLog.warning(tr("Error #%02x.%02x in control FAM request (%ld) for service 0x%02X."),
          err, suberr, RequestId, myFam);
      break;
    }

    case ICQ_SNACxSUB_READYxSERVER:
      gLog.info(tr("Server says he's ready for service 0x%02X."), myFam);
      ChangeStatus(STATUS_SRV_READY_RECV);
      break;

    case ICQ_SNACxSRV_ACKxIMxICQ:
      gLog.info(tr("Server sent us channel capability list for service 0x%02X."), myFam);
      ChangeStatus(STATUS_SRV_VER_RECV);
      break;

    case ICQ_SNACxSUB_RATE_INFO:
      gLog.info(tr("Server sent us rate-limits information for service 0x%02X."), myFam);
      ChangeStatus(STATUS_SRV_RATE_RECV);
      break;

    default:
      gLog.warning(tr("Unknown or unsupported service FAM subtype 0x%02X for service 0x%02X."),
          SubType, myFam);
      break;
  }
}

void COscarService::ProcessBARTFam(Buffer& packet, unsigned short SubType,
                                   unsigned long RequestId)
{
  switch (SubType)
  {
    case ICQ_SNACxBART_ERROR:
    {
      unsigned short err = packet.UnpackUnsignedShortBE();
      unsigned short suberr = 0;
      
      packet.readTLV();
      if (packet.getTLVLen(0x0008) == 2)
        suberr = packet.UnpackUnsignedShortTLV(0x0008);
      gLog.warning(tr("Error #%02x.%02x in BART request (%ld) for service 0x%02X."),
          err, suberr, RequestId, myFam);

      Licq::Event* e = gIcqProtocol.DoneServerEvent(RequestId, Licq::Event::ResultError);
      if (e)
        gIcqProtocol.ProcessDoneEvent(e);
      break;
    }

    case ICQ_SNACxBART_DOWNLOADxREPLY:
    {
      string id = packet.unpackByteString();
      UserWriteGuard u(Licq::UserId(gIcqProtocol.ownerId(), id));
      if (!u.isLocked())
      {
        gLog.warning(tr("Buddy icon for unknown user (%s)."), id.c_str());
        break;
      }

      unsigned short IconType = packet.UnpackUnsignedShortBE();
      char HashType = packet.UnpackChar();                     
      char HashLength = packet.UnpackChar();
      switch (IconType)
      {
        case BART_TYPExBUDDY_ICON_SMALL:
        case BART_TYPExBUDDY_ICON:
        {
          if ((HashType == 0 || HashType == 1) && HashLength > 0 && HashLength <= 16)
          {
            string hash = packet.unpackRawString(HashLength);
            packet.UnpackChar(); // unknown (command ?)
            packet.UnpackUnsignedShortBE(); // IconType once more
            packet.UnpackChar(); // HashType once more
            char HashLength2 = packet.UnpackChar(); // Hash once more
            packet.incDataPosRead(HashLength2); // Hash once more
            u->setOurBuddyIconHash(hash);

            gLog.info(tr("Buddy icon reply for %s."), u->getAlias().c_str());
            unsigned short IconLen = packet.UnpackUnsignedShortBE();
            if (IconLen > 0) // do not create empty .pic files
            {
              int FD = open(u->pictureFileName().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 00664);
              if (FD == -1)
              {
                gLog.error(tr("Unable to open picture file (%s): %s."),
                    u->pictureFileName().c_str(), strerror(errno));
                break;
              }

              string icon = packet.unpackRawString(IconLen);
              write(FD, icon.c_str(), IconLen);
              close(FD);

              u->SetEnableSave(false);
              u->SetPicturePresent(true);
              u->SetEnableSave(true);
            }
            u->save(Licq::User::SavePictureInfo);
            Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                Licq::PluginSignal::SignalUser,
                Licq::PluginSignal::UserPicture, u->id()));

            Licq::Event* e = gIcqProtocol.DoneServerEvent(RequestId, Licq::Event::ResultSuccess);
            if (e)
              gIcqProtocol.ProcessDoneEvent(e);
          }
          else
          {
            gLog.warning(tr("Buddy icon reply for %s with wrong or unsupported hashtype (%d) or hashlength (%d)."),
                u->getAlias().c_str(), HashType, HashLength);
            Licq::Event* e = gIcqProtocol.DoneServerEvent(RequestId, Licq::Event::ResultFailed);
            if (e)
              gIcqProtocol.ProcessDoneEvent(e);
          }
          break;
        }

        default:
        {
          gLog.warning(tr("Buddy icon reply for %s with wrong or unsupported icontype (0x%02x)."),
              u->getAlias().c_str(), IconType);
          Licq::Event* e = gIcqProtocol.DoneServerEvent(RequestId, Licq::Event::ResultFailed);
          if (e)
            gIcqProtocol.ProcessDoneEvent(e);
          break;
        }
      }
      break;
    }

    default:
      break;
  }
}

bool COscarService::Initialize()
{
  ChangeStatus(STATUS_SERVICE_REQ_SENT);
  gIcqProtocol.icqRequestService(myFam);

  if (!WaitForStatus(STATUS_SERVICE_REQ_ACKED))
  {
    gLog.warning(tr("Give up waiting for redirect reply while initializing service 0x%02X."),
        myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }

  ChangeStatus(STATUS_CONNECTED);
  SrvSocket* s = new SrvSocket(gIcqProtocol.ownerId());
  gLog.info(tr("Connecting to separate server for service 0x%02X."), myFam);
  if (gIcqProtocol.GetProxy() == NULL)
  {
    if (myProxy != NULL)
    {
      delete myProxy;
      myProxy = NULL;
    }
  }
  else
  {
    if (myProxy == NULL)
      myProxy = gDaemon.createProxy();
  }
  if (!s->connectTo(myServer, myPort, myProxy))
  {
    gLog.warning(tr("Can't establish service 0x%02X socket."), myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }
  mySocketDesc = s->Descriptor();
  gSocketManager.AddSocket(s);
  gSocketManager.DropSocket(s);
  // Alert the select thread that there is a new socket
  gIcqProtocol.myNewSocketPipe.putChar('S');

  CPU_SendCookie* p1 = new CPU_SendCookie(myCookie, myFam);
  gLog.info(tr("Sending cookie for service 0x%02X."), myFam);
  if (!SendPacket(p1))
  {
    gLog.warning(tr("Can't send cookie while initializing service 0x%02X."), myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }

  if (!WaitForStatus(STATUS_SRV_READY_RECV))
  {
    gLog.warning(tr("Give up waiting for server ready packet while initializing service 0x%02X."),
        myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }
  
  unsigned short VerArray[2][2] = {{ 0x0001, 0x0004 },	// Service FAM
                                   { 0x0010, 0x0001 }};	// BART	FAM
  CPU_ImICQ *p2 = new CPU_ImICQ(VerArray, 2, myFam);
  gLog.info(tr("Sending our families versions for service 0x%02X."), myFam);
  if (!SendPacket(p2))
  {
    gLog.warning(tr("Can't send channel capability request while initializing service 0x%02X."),
        myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }
  
  if (!WaitForStatus(STATUS_SRV_VER_RECV))
  {
    gLog.warning(tr("Give up waiting for channel capability list while initializing service 0x%02X."),
        myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }

  CPU_GenericFamily *p3 = new CPU_GenericFamily(ICQ_SNACxFAM_SERVICE,
                                                ICQ_SNACxSUB_REQ_RATE_INFO, myFam);
  gLog.info(tr("Sending request of rate-limits for service 0x%02X."), myFam);
  if (!SendPacket(p3))
  {
    gLog.warning(tr("Can't send request for rate-limits while initializing service 0x%02X."),
        myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }

  if (!WaitForStatus(STATUS_SRV_RATE_RECV))
  {
    gLog.warning(tr("Give up waiting for rate-limits while initializing service 0x%02X."),
        myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }

  CPU_RateAck *p4 = new CPU_RateAck(myFam);
  gLog.info(tr("Sending ack for rate-limits for service 0x%02X."), myFam);
  if (!SendPacket(p4))
  {
    gLog.warning(tr("Can't send rate-limits ack while initializing service 0x%02X."), myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }
  unsigned short VerArray2[2][4] = {{ 0x0001, 0x0004, 0x0110, 0x08e4 },  // Service FAM
                                    { 0x0010, 0x0001, 0x0110, 0x08e4 }}; // BART FAM
  CPU_ClientReady *p5 = new CPU_ClientReady(VerArray2, 2, myFam);
  gLog.info(tr("Sending client ready for service 0x%02X."), myFam);
  if (!SendPacket(p5))
  {
    gLog.warning(tr("Can't send client ready while initializing service 0x%02X."), myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }

  ChangeStatus(STATUS_READY);
  return true;
}

void* LicqIcq::OscarServiceSendQueue_tep(void *p)
{
  pthread_detach(pthread_self());
  
  COscarService *os = (COscarService *)p;

  while (true)
  {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_mutex_lock(&os->mutex_sendqueue);
    if (!os->mySendQueue.empty())
    {
      std::list<Licq::Event*>::iterator iter = os->mySendQueue.begin();
      Licq::Event* e = *iter;
      os->mySendQueue.erase(iter);
      pthread_mutex_unlock(&os->mutex_sendqueue);

      if (e->IsCancelled())
      {
        delete e;
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        continue;
      }

      if (gIcqProtocol.Status() != STATUS_ONLINE)
      {
        gLog.warning(tr("Can't send event for service 0x%02X because we are not online."),
            os->myFam);
        if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultError) != NULL)
          gIcqProtocol.ProcessDoneEvent(e);
        else
          delete e;
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        continue;
      }
      
      if (os->mySocketDesc == -1)
      {
        gLog.info(tr("Initializing socket for service 0x%02X."), os->myFam);
        if (!os->Initialize())
        {
          gLog.warning(tr("Initialization of socket for service 0x%02X failed, failing event."),
              os->myFam);
          if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultError) != NULL)
            gIcqProtocol.ProcessDoneEvent(e);
          else
            delete e;
          pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
          pthread_testcancel();
          continue;
        }
      }

      bool Sent;
      switch (os->myFam)
      {
        case ICQ_SNACxFAM_BART:
          Sent = os->SendBARTFam(e);
          break;

        default:
          gLog.warning(tr("Event for unknown or unsupported service 0x%02X failed."),
              os->myFam);
          Sent = false;
          break;
      }
 
      if (!Sent)
      {
        if (gIcqProtocol.DoneEvent(e, Licq::Event::ResultError) != NULL)
          gIcqProtocol.ProcessDoneEvent(e);
        else
          delete e;
      }

      if (e->NoAck())
          delete e;
      
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      continue;
    }
    else
    {
      pthread_cond_wait(&os->cond_sendqueue, &os->mutex_sendqueue);
      pthread_mutex_unlock(&os->mutex_sendqueue);
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
    }
  }

  pthread_exit(NULL);
}
