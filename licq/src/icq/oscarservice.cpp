// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 2007-2010 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
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
#include <licq/event.h>
#include <licq/pluginsignal.h>
#include <licq/proxy.h>
#include <licq/socket.h>
#include <licq/logging/log.h>

#include "../daemon.h"
#include "../gettext.h"
#include "../support.h"
#include "icq.h"
#include "packet.h"

using namespace std;
using Licq::Buffer;
using Licq::gLog;
using LicqDaemon::gDaemon;

COscarService::COscarService(unsigned short Fam)
{
  myFam = Fam;
  mySocketDesc = -1;
  myProxy = NULL;
  myStatus = STATUS_UNINITIALIZED;
  myServer = NULL;
  pthread_mutex_init(&mutex_sendqueue, NULL);
  pthread_cond_init(&cond_sendqueue, NULL);
  pthread_mutex_init(&mutex_status, NULL);
  pthread_cond_init(&cond_status, NULL);
}

COscarService::~COscarService()
{
  if (myProxy) delete myProxy;
  if (myServer) free(myServer);
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

void COscarService::SetConnectCredential(char *Server, unsigned short Port,
                                         char *Cookie, unsigned short CookieLen)
{
  if (myServer) free(myServer);
  myServer = strdup(Server);
  myPort = Port;
  myCookie.reset(new char[CookieLen]);
  memcpy(myCookie.get(), Cookie, CookieLen);
  myCookieLen = CookieLen;
}

bool COscarService::SendPacket(CPacket *p)
{
  Licq::INetSocket* s = gSocketManager.FetchSocket(mySocketDesc);
  if (s == NULL) return false;
  Buffer *b = p->Finalize(s);
  if (!s->Send(b))
  {
    gLog.warning(tr("%sError sending event (FAM #%02X, Subtype #%02X, Sequence #%hu):\n%s%s.\n"),
              L_WARNxSTR, (unsigned short)((p->SNAC() >> 16) & 0xffff), (unsigned short)(p->SNAC() & 0xffff),
        p->Sequence(), L_BLANKxSTR, s->errorStr().c_str());
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
      gLog.info("Event #%hu is still on the service 0x%02X queue!\n", e->Sequence(), myFam);
      delete e;
    }
  }
  pthread_mutex_unlock(&mutex_sendqueue);
}

unsigned long COscarService::SendEvent(const Licq::UserId& userId,
                                       unsigned short SubType, bool Request)
{
  unsigned long eventId = gDaemon.getNextEventId();
  Licq::Event* e = new Licq::Event(eventId, mySocketDesc, NULL, Licq::Event::ConnectServer, userId);
  e->SetSubType(SubType);
  if (Request)
    gIcqProtocol.PushEvent(e);
  else
    e->SetNoAck(true);
  pthread_mutex_lock(&mutex_sendqueue);
  mySendQueue.push_back(e);
  pthread_cond_signal(&cond_sendqueue);
  pthread_mutex_unlock(&mutex_sendqueue);

  return eventId;
}

bool COscarService::SendBARTFam(Licq::Event* e)
{
  switch (e->SubType())
  {
    case ICQ_SNACxBART_DOWNLOADxREQUEST:
    {
      CPU_RequestBuddyIcon* p;
      {
        Licq::UserReadGuard u(e->userId());
        if (!u.isLocked())
          return false;
        p = new CPU_RequestBuddyIcon(u->accountId(),
            u->buddyIconType(), u->buddyIconHashType(), u->buddyIconHash(), myFam);
        gLog.info(tr("%sRequesting buddy icon for %s (#%hu/#%d)...\n"),
            L_SRVxSTR, u->GetAlias(), p->Sequence(), p->SubSequence());
      }
      e->AttachPacket(p);
      return (SendPacket(p));
    }
    
    default:
      gLog.warning(tr("%sEvent with unsupported subtype (%02X) for FAM %02X failed.\n"),
                L_WARNxSTR, e->SubType(), myFam);
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
    gLog.warning(tr("%sbad start code %d for packet in socket of service 0x%02X.\n"),
               L_WARNxSTR, startCode, myFam);
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
      gLog.info(tr("%sServer send us request for close service 0x%02X.\n"),
                L_SRVxSTR, myFam);
      return false;
      break;

    default:
      gLog.warning(tr("%sPacket from unhandled channel %02x for service 0x%02X.\n"),
                L_WARNxSTR, Channel, myFam);
      break;
  }

  return true;
}

void COscarService::ProcessNewChannel(Buffer& packet)
{
  unsigned long Version = packet.UnpackUnsignedLongBE();
  
  if (Version != 0x00000001)
  {
    gLog.warning(tr("%sPacket with wrong version (0x%08lx) from new channel for service 0x%02X.\n"),
              L_WARNxSTR, Version, myFam);
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
        gLog.warning(tr("%sUsupported family %04hx\n on data channel of service %02X.\n"),
                  L_WARNxSTR, Family, myFam);
      break;

    default:
      gLog.warning(tr("%sUnknown or usupported family %04hx\n on data channel of service %02X.\n"),
                L_WARNxSTR, Family, myFam);
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
      gLog.warning(tr("%sError #%02x.%02x in control FAM request (%ld) for service 0x%02X.\n"),
                L_WARNxSTR, err, suberr, RequestId, myFam);
      break;
    }

    case ICQ_SNACxSUB_READYxSERVER:
      gLog.info(tr("%sServer says he's ready for service 0x%02X.\n"),
                L_SRVxSTR, myFam);
      ChangeStatus(STATUS_SRV_READY_RECV);
      break;

    case ICQ_SNACxSRV_ACKxIMxICQ:
      gLog.info(tr("%sServer sent us channel capability list for service 0x%02X.\n"),
                L_SRVxSTR, myFam);
      ChangeStatus(STATUS_SRV_VER_RECV);
      break;

    case ICQ_SNACxSUB_RATE_INFO:
      gLog.info(tr("%sServer sent us rate-limits information for service 0x%02X.\n"),
                L_SRVxSTR, myFam);
      ChangeStatus(STATUS_SRV_RATE_RECV);
      break;

    default:
      gLog.warning(tr("%sUnknown or unsupported service FAM subtype 0x%02X for service 0x%02X.\n"),
                L_WARNxSTR, SubType, myFam);
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
      gLog.warning(tr("%sError #%02x.%02x in BART request (%ld) for service 0x%02X.\n"),
                    L_WARNxSTR, err, suberr, RequestId, myFam);

      Licq::Event* e = gIcqProtocol.DoneServerEvent(RequestId, Licq::Event::ResultError);
      if (e)
        gIcqProtocol.ProcessDoneEvent(e);
      break;
    }

    case ICQ_SNACxBART_DOWNLOADxREPLY:
    {
      char *Id = packet.UnpackUserString();
      Licq::UserId userId(Id, LICQ_PPID);
      Licq::UserWriteGuard u(userId);
      if (!u.isLocked())
      {
        gLog.warning(tr("%sBuddy icon for unknown user (%s).\n"),
                  L_WARNxSTR, Id);
        delete [] Id;
        break;
      }
      delete [] Id;

      unsigned short IconType = packet.UnpackUnsignedShortBE();
      char HashType = packet.UnpackChar();                     
      char HashLength = packet.UnpackChar();
      switch (IconType)
      {
        case BART_TYPExBUDDY_ICON_SMALL:
        case BART_TYPExBUDDY_ICON:
        {
          if (HashType == 1 && HashLength > 0 && HashLength <= 16)
          {
            boost::scoped_array<char> Hash(new char[HashLength]);
            boost::scoped_array<char> HashHex(new char[HashLength*2 + 1]);
            packet.UnpackBinBlock(Hash.get(), HashLength);
            packet.UnpackChar(); // unknown (command ?)
            packet.UnpackUnsignedShortBE(); // IconType once more
            packet.UnpackChar(); // HashType once more
            char HashLength2 = packet.UnpackChar(); // Hash once more
            packet.incDataPosRead(HashLength2); // Hash once more
            u->setOurBuddyIconHash(PrintHex(HashHex.get(), Hash.get(), HashLength));

            gLog.info(tr("%sBuddy icon reply for %s.\n"), L_SRVxSTR, u->GetAlias());
            unsigned short IconLen = packet.UnpackUnsignedShortBE();
            if (IconLen > 0) // do not create empty .pic files
            {
              int FD = open(u->pictureFileName().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 00664);
              if (FD == -1)
              {
                gLog.error(tr("%sUnable to open picture file (%s):\n%s%s.\n"),
                    L_ERRORxSTR, u->pictureFileName().c_str(), L_BLANKxSTR, strerror(errno));
                break;
              }

              boost::scoped_array<char> Icon(new char[IconLen]);
              packet.UnpackBinBlock(Icon.get(), IconLen);
              write(FD, Icon.get(), IconLen);
              close(FD);

              u->SetEnableSave(false);
              u->SetPicturePresent(true);
              u->SetEnableSave(true);
            }
            u->SavePictureInfo();
            gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                Licq::PluginSignal::UserPicture, u->id()));

            Licq::Event* e = gIcqProtocol.DoneServerEvent(RequestId, Licq::Event::ResultSuccess);
            if (e)
              gIcqProtocol.ProcessDoneEvent(e);
          }
          else
          {
            gLog.warning(tr("%sBuddy icon reply for %s with wrong or unsupported hashtype (%d) or hashlength (%d).\n"),
                      L_WARNxSTR, u->GetAlias(), HashType, HashLength);
            Licq::Event* e = gIcqProtocol.DoneServerEvent(RequestId, Licq::Event::ResultFailed);
            if (e)
              gIcqProtocol.ProcessDoneEvent(e);
          }
          break;
        }

        default:
        {
          gLog.warning(tr("%sBuddy icon reply for %s with wrong or unsupported icontype (0x%02x).\n"),
                    L_WARNxSTR, u->GetAlias(), IconType);
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
    gLog.warning(tr("%sGive up waiting for redirect reply while initializing service 0x%02X.\n"),
              L_WARNxSTR, myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }

  ChangeStatus(STATUS_CONNECTED);
  Licq::SrvSocket* s = new Licq::SrvSocket(Licq::gUserManager.ownerUserId(LICQ_PPID));
  gLog.info(tr("%sConnecting to separate server for service 0x%02X.\n"),
            L_SRVxSTR, myFam);
  if (gLicqDaemon->GetProxy() == NULL)
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
  if (!s->connectTo(string(myServer), myPort, myProxy))
  {
    gLog.warning(tr("%sCan't establish service 0x%02X socket.\n"),
               L_WARNxSTR, myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }
  mySocketDesc = s->Descriptor();
  gSocketManager.AddSocket(s);
  gSocketManager.DropSocket(s);
  // Alert the select thread that there is a new socket
  gIcqProtocol.myNewSocketPipe.putChar('S');

  string cookie(myCookie.get(), myCookieLen);
  CPU_SendCookie *p1 = new CPU_SendCookie(cookie, myFam);
  gLog.info(tr("%sSending cookie for service 0x%02X.\n"),
            L_SRVxSTR, myFam);
  if (!SendPacket(p1))
  {
    gLog.warning(tr("%sCan't send cookie while initializing service 0x%02X.\n"),
              L_WARNxSTR, myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }

  if (!WaitForStatus(STATUS_SRV_READY_RECV))
  {
    gLog.warning(tr("%sGive up waiting for server ready packet while initializing service 0x%02X.\n"),
              L_WARNxSTR, myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }
  
  unsigned short VerArray[2][2] = {{ 0x0001, 0x0004 },	// Service FAM
                                   { 0x0010, 0x0001 }};	// BART	FAM
  CPU_ImICQ *p2 = new CPU_ImICQ(VerArray, 2, myFam);
  gLog.info(tr("%sSending our families versions for service 0x%02X.\n"),
            L_SRVxSTR, myFam);
  if (!SendPacket(p2))
  {
    gLog.warning(tr("%sCan't send channel capability request while initializing service 0x%02X.\n"),
              L_WARNxSTR, myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }
  
  if (!WaitForStatus(STATUS_SRV_VER_RECV))
  {
    gLog.warning(tr("%sGive up waiting for channel capability list while initializing service 0x%02X.\n"),
              L_WARNxSTR, myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }

  CPU_GenericFamily *p3 = new CPU_GenericFamily(ICQ_SNACxFAM_SERVICE,
                                                ICQ_SNACxSUB_REQ_RATE_INFO, myFam);
  gLog.info(tr("%sSending request of rate-limits for service 0x%02X.\n"),
            L_SRVxSTR, myFam);
  if (!SendPacket(p3))
  {
    gLog.warning(tr("%sCan't send request for rate-limits while initializing service 0x%02X.\n"),
              L_WARNxSTR, myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }

  if (!WaitForStatus(STATUS_SRV_RATE_RECV))
  {
    gLog.warning(tr("%sGive up waiting for rate-limits while initializing service 0x%02X.\n"),
              L_WARNxSTR, myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }

  CPU_RateAck *p4 = new CPU_RateAck(myFam);
  gLog.info(tr("%sSending ack for rate-limits for service 0x%02X.\n"),
            L_SRVxSTR, myFam);
  if (!SendPacket(p4))
  {
    gLog.warning(tr("%sCan't send rate-limits ack while initializing service 0x%02X.\n"),
              L_WARNxSTR, myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }
  unsigned short VerArray2[2][4] = {{ 0x0001, 0x0004, 0x0110, 0x08e4 },  // Service FAM
                                    { 0x0010, 0x0001, 0x0110, 0x08e4 }}; // BART FAM
  CPU_ClientReady *p5 = new CPU_ClientReady(VerArray2, 2, myFam);
  gLog.info(tr("%sSending client ready for service 0x%02X.\n"),
            L_SRVxSTR, myFam);
  if (!SendPacket(p5))
  {
    gLog.warning(tr("%sCan't send client ready while initializing service 0x%02X.\n"),
              L_WARNxSTR, myFam);
    ChangeStatus(STATUS_UNINITIALIZED);
    return false;
  }

  ChangeStatus(STATUS_READY);
  return true;
}

void *OscarServiceSendQueue_tep(void *p)
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

      if (gLicqDaemon->Status() != STATUS_ONLINE)
      {
        gLog.warning(tr("%sCan't send event for service 0x%02X because we are not online.\n"),
                  L_WARNxSTR, os->myFam);
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
        gLog.info(tr("%sInitializing socket for service 0x%02X.\n"), L_SRVxSTR, os->myFam);
        if (!os->Initialize())
        {
          gLog.warning(tr("%sInitialization of socket for service 0x%02X failed, failing event\n"),
                    L_WARNxSTR, os->myFam);
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
          gLog.warning(tr("%sEvent for unknown or unsupported service 0x%02X failed.\n"),
                    L_WARNxSTR, os->myFam);
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
