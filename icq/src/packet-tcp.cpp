/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "packet-tcp.h"

#include <boost/foreach.hpp>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include <cerrno>

#include <licq/byteorder.h>
#include <licq/color.h>
#include <licq/contactlist/usermanager.h>
#include <licq/translator.h>
#include <licq/logging/log.h>
#include <licq/version.h>

#include "buffer.h"
#include "defines.h"
#include "gettext.h"
#include "icq.h"
#include "socket.h"
#include "owner.h"
#include "user.h"

using namespace LicqIcq;
using Licq::UserId;
using Licq::gLog;
using Licq::gTranslator;
using std::list;
using std::string;

//=====TCP===================================================================
static unsigned char client_check_data[] = {
  "As part of this software beta version Mirabilis is "
  "granting a limited access to the ICQ network, "
  "servers, directories, listings, information and databases (\""
  "ICQ Services and Information\"). The "
  "ICQ Service and Information may databases (\""
  "ICQ Services and Information\"). The "
  "ICQ Service and Information may\0"
};


#define DEBUG_ENCRYPTION(x)
//#define DEBUG_ENCRYPTION(x) fprintf(stderr, x)

void LicqIcq::Encrypt_Client(CBuffer* pkt, unsigned long version)
{
  unsigned long B1, M1, check;
  unsigned int i;
  unsigned char X1, X2, X3;
  unsigned char* buf = (unsigned char*)pkt->getDataStart() + 2;
  unsigned char bak[6];
  unsigned long offset;
  unsigned long size = pkt->getDataSize() - 2;

  if (version < 4)
    return;  // no encryption necessary.

  switch(version)
  {
    case 4:
    case 5:
      offset = 6;
      break;
    case 7:
    case 8:
    case 6:
    default:
      offset = 0;
  }

  pkt->log(Licq::Log::Debug, tr("Unencrypted (ICQ) TCP Packet (%lu bytes)"), size);

  // Fuck AOL
  if (version > 6)
  {
    buf += 1;
    size -= 1;
  }

  // calculate verification data
  M1 = (rand() % ((size < 255 ? size : 255)-10))+10;
  X1 = buf[M1] ^ 0xFF;
  X2 = rand() % 220;
  X3 = client_check_data[X2] ^ 0xFF;
  if(offset) {
    for(i=0;i<6;i++)  bak[i] = buf[i];
    B1 = (buf[offset+4]<<24)|(buf[offset+6]<<16)|(buf[2]<<8)|buf[0];
  }
  else
    B1 = (buf[4]<<24)|(buf[6]<<16)|(buf[4]<<8)|(buf[6]);

  // calculate checkcode
  check = (M1 << 24) | (X1 << 16) | (X2 << 8) | X3;
  check ^= B1;
  DEBUG_ENCRYPTION(("complete check %08lx\n", check));

  // main XOR key
  unsigned long key = 0x67657268 * size + check;

  // XORing the actual data
  for(i=0;i<(size+3)/4;i+=4){
    unsigned long hex = key + client_check_data[i&0xFF];
    buf[i+0] ^= hex&0xFF;buf[i+1] ^= (hex>>8)&0xFF;
    buf[i+2] ^= (hex>>16)&0xFF;buf[i+3] ^= (hex>>24)&0xFF;
  }

  // in TCPv4 are the first 6 bytes unencrypted
  // so restore them
  if(offset)  for(i=0;i<6;i++) buf[i] = bak[i];

  // storing the checkcode
  buf[offset+3] = (check>>24)&0xFF;
  buf[offset+2] = (check>>16)&0xFF;
  buf[offset+1] = (check>>8)&0xFF;
  buf[offset+0] = check&0xFF;
}


bool LicqIcq::Decrypt_Client(CBuffer* pkt, unsigned long version)
{
  unsigned long hex, key, B1, M1, check;
  unsigned int i;
  unsigned char X1, X2, X3;
  unsigned char* buf = (unsigned char*)pkt->getDataStart() + 2;
  unsigned char bak[6];
  unsigned long size = pkt->getDataSize() - 2;
  unsigned long offset;

  if(version < 4)
    return true;  // no decryption necessary.

  switch(version){
  case 4:
  case 5:
    offset = 6;
    break;
  case 7:
  case 8:
  case 6:
  default:
    offset = 0;
  }

  // Fuck AOL
  if (version > 6)
  {
    buf += 1;
    size -= 1;
  }

  // backup the first 6 bytes
  if(offset)
    for(i=0;i<6;i++)  bak[i] = buf[i];

  // retrieve checkcode
  check = (buf[offset+3]<<24)|(buf[offset+2]<<16)|(buf[offset+1]<<8)|(buf[offset+0]);

  DEBUG_ENCRYPTION(("size %d, check %08lx\n", size, check));

  // main XOR key
  key = 0x67657268 * size + check;

  for(i=4; i<(size+3)/4; i+=4) {
    hex = key + client_check_data[i&0xFF];
    buf[i+0] ^= hex&0xFF;buf[i+1] ^= (hex>>8)&0xFF;
    buf[i+2] ^= (hex>>16)&0xFF;buf[i+3] ^= (hex>>24)&0xFF;
  }

  // retrive validate data
  if(offset) {
    // in TCPv4 are the first 6 bytes unencrypted
    // so restore them
    for(i=0;i<6;i++) buf[i] = bak[i];
    B1 = (buf[offset+4]<<24)|(buf[offset+6]<<16)|(buf[2]<<8)|buf[0];
  }
  else
    B1 = (buf[4]<<24) | (buf[6]<<16) | (buf[4]<<8) | (buf[6]<<0);

  // special decryption
  B1 ^= check;

  // validate packet
  M1 = (B1 >> 24) & 0xFF;
  if(M1 < 10 || M1 >= size) {
    DEBUG_ENCRYPTION(("range check failed, M1 is %02x, returning false\n", M1));
    return false;
  }

  X1 = buf[M1] ^ 0xFF;
  if(((B1 >> 16) & 0xFF) != X1) {
    DEBUG_ENCRYPTION(("M1 is %02x\n", M1));
    DEBUG_ENCRYPTION(("calculated X1 (%02lx) != %02x\n", X1, (B1 >> 16) & 0xFF));
    return false;
  }

  X2 = ((B1 >> 8) & 0xFF);
  if(X2 < 220) {
    X3 = client_check_data[X2] ^ 0xFF;
    if((B1 & 0xFF) != X3) {
      DEBUG_ENCRYPTION(("calculated X3 (%02x) does not match B1 (%02x)\n", X3, B1 & 0xFF));
      return false;
    }
  }

  pkt->log(Licq::Log::Debug, tr("Decrypted (ICQ) TCP Packet (%lu bytes)"), size);

  return true;
}

CPacketTcp_Handshake::CPacketTcp_Handshake()
  : buffer(NULL)
{
  // Empty
}

CPacketTcp_Handshake::~CPacketTcp_Handshake()
{
  delete buffer;
}

CPacketTcp_Handshake_v2::CPacketTcp_Handshake_v2(unsigned long nLocalPort)
{
  m_nLocalPort = nLocalPort;

  m_nSize = 28;
  buffer = new CBuffer(m_nSize);
  buffer->packUInt16LE(m_nSize-2); // Packet length

  buffer->PackChar(ICQ_CMDxTCP_HANDSHAKE);
  buffer->PackUnsignedLong(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(m_nLocalPort);
  buffer->PackUnsignedLong(gIcqProtocol.icqOwnerUin());
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(gIcqProtocol.directMode() ? MODE_DIRECT : MODE_INDIRECT);
  buffer->PackUnsignedLong(m_nLocalPort);
}


CPacketTcp_Handshake_v4::CPacketTcp_Handshake_v4(unsigned long nLocalPort)
{
  m_nLocalPort = nLocalPort;

  m_nSize = 28;
  buffer = new CBuffer(m_nSize);
  buffer->packUInt16LE(m_nSize-2); // Packet length

  buffer->PackChar(ICQ_CMDxTCP_HANDSHAKE);
  buffer->PackUnsignedLong(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(0x00000000);
  buffer->PackUnsignedLong(gIcqProtocol.icqOwnerUin());
  buffer->PackUnsignedLong(s_nLocalIp); // maybe should be 0
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(gIcqProtocol.directMode() ? MODE_DIRECT : MODE_INDIRECT);
  buffer->PackUnsignedLong(m_nLocalPort);
}


//=====PacketTcp_Handshake======================================================
CPacketTcp_Handshake_v6::CPacketTcp_Handshake_v6(unsigned long nDestinationUin,
   unsigned long /* nSessionId */, unsigned short nLocalPort)
{
  m_nDestinationUin = nDestinationUin;

  m_nSize = 46;
  buffer = new CBuffer(m_nSize);
  buffer->packUInt16LE(m_nSize-2); // Packet length

  buffer->PackChar(ICQ_CMDxTCP_HANDSHAKE);
  buffer->PackUnsignedShort(ICQ_VERSION_TCP);
  buffer->PackUnsignedShort(0x0027); //size
  buffer->PackUnsignedLong(m_nDestinationUin);
  buffer->PackUnsignedShort(0);
  buffer->PackUnsignedLong(nLocalPort);
  buffer->PackUnsignedLong(gIcqProtocol.icqOwnerUin());
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(gIcqProtocol.directMode() ? MODE_DIRECT : MODE_INDIRECT);
  buffer->PackUnsignedLong(nLocalPort == 0 ? s_nLocalPort : nLocalPort);

  char id[16];
  snprintf(id, 16, "%lu", nDestinationUin);
  UserId userId(gIcqProtocol.ownerId(), id);
  UserReadGuard u(userId);
  if (u.isLocked())
  {
    buffer->PackUnsignedLong(u->Cookie());
    m_nSessionId = u->Cookie();
  }
  else
  {
    m_nSessionId = 0;
    buffer->PackUnsignedLong(0);
  }

  buffer->PackUnsignedLong(0x00000050); // constant
  buffer->PackUnsignedLong(0x00000003); // constant
}


CPacketTcp_Handshake_v6::CPacketTcp_Handshake_v6(CBuffer *inbuf)
{
  inbuf->unpackUInt16LE(); // Packet length
  m_nHandshake = inbuf->UnpackChar();
  m_nVersionMajor = inbuf->UnpackUnsignedShort();
  m_nVersionMinor = inbuf->UnpackUnsignedShort();
  m_nDestinationUin = inbuf->UnpackUnsignedLong();
  inbuf->UnpackUnsignedLong();
  inbuf->UnpackUnsignedShort();
  m_nSourceUin = inbuf->UnpackUnsignedLong();
  m_nLocalIp = inbuf->UnpackUnsignedLong();
  m_nRealIp = inbuf->UnpackUnsignedLong();
  m_nMode = inbuf->UnpackChar();
  inbuf->UnpackUnsignedLong(); // port of some kind...?
  m_nSessionId = inbuf->UnpackUnsignedLong();
  //inbuf->UnpackUnsignedLong(); // constant
  //inbuf->UnpackUnsignedLong(); // constant
}


CPacketTcp_Handshake_v7::CPacketTcp_Handshake_v7(unsigned long nDestinationUin,
   unsigned long /* nSessionId */, unsigned short nLocalPort, unsigned long nId)
{
  m_nDestinationUin = nDestinationUin;

  m_nSize = 50;
  buffer = new CBuffer(m_nSize);
  buffer->packUInt16LE(m_nSize-2); // Packet length

  buffer->PackChar(ICQ_CMDxTCP_HANDSHAKE);
  buffer->PackUnsignedShort(ICQ_VERSION_TCP);
  buffer->PackUnsignedShort(0x002b); // size
  buffer->PackUnsignedLong(m_nDestinationUin);
  buffer->PackUnsignedShort(0);
  buffer->PackUnsignedLong(nLocalPort == 0 ? s_nLocalPort : nLocalPort);
  buffer->PackUnsignedLong(gIcqProtocol.icqOwnerUin());
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackChar(gIcqProtocol.directMode() ? MODE_DIRECT : MODE_INDIRECT);
  buffer->PackUnsignedLong(nLocalPort == 0 ? s_nLocalPort : nLocalPort);

  char id[16];
  snprintf(id, 16, "%lu", nDestinationUin);
  UserId userId(gIcqProtocol.ownerId(), id);
  UserReadGuard u(userId);
  if (u.isLocked())
  {
    buffer->PackUnsignedLong(u->Cookie());
    m_nSessionId = u->Cookie();
  }
  else
  {
    m_nSessionId = 0;
    buffer->PackUnsignedLong(0);
  }

  buffer->PackUnsignedLong(0x00000050); // constant
  buffer->PackUnsignedLong(0x00000003); // constant
  buffer->PackUnsignedLong(nId); // the connection id for reverse connect

}


CPacketTcp_Handshake_v7::CPacketTcp_Handshake_v7(CBuffer *inbuf)
{
  inbuf->unpackUInt16LE(); // Packet length
  m_nHandshake = inbuf->UnpackChar();
  m_nVersionMajor = inbuf->UnpackUnsignedShort();
  inbuf->UnpackUnsignedShort();  // Length
  m_nDestinationUin = inbuf->UnpackUnsignedLong();
  inbuf->UnpackUnsignedShort();
  inbuf->UnpackUnsignedLong();
  m_nSourceUin = inbuf->UnpackUnsignedLong();
  m_nRealIp = inbuf->UnpackUnsignedLong();
  m_nLocalIp = inbuf->UnpackUnsignedLong();
  m_nMode = inbuf->UnpackChar();
  inbuf->UnpackUnsignedLong();
  m_nSessionId = inbuf->UnpackUnsignedLong(); // Mmmm cookie
  inbuf->incDataPosRead(8);
  m_nId = inbuf->UnpackUnsignedLong();
}


CPacketTcp_Handshake_Ack::CPacketTcp_Handshake_Ack()
{
  m_nSize = 6;
  buffer = new CBuffer(m_nSize);
  buffer->packUInt16LE(m_nSize-2); // Packet length

  buffer->PackUnsignedLong(1);
}

CPacketTcp_Handshake_Confirm::CPacketTcp_Handshake_Confirm(int channel,
  unsigned short nSequence)
  : myChannel(channel)
{
  m_nSize = 35;
  buffer = new CBuffer(m_nSize);
  buffer->packUInt16LE(m_nSize-2); // Packet length

  const uint8_t* GUID;
  unsigned long nOurId;
  switch (channel)
  {
    case DcSocket::ChannelNormal:
    nOurId = 0x00000001;
    GUID = PLUGIN_NORMAL;
      break;
    case DcSocket::ChannelInfo:
    nOurId = 0x000003EB;
    GUID = PLUGIN_INFOxMANAGER;
      break;
    case DcSocket::ChannelStatus:
    nOurId = 0x000003EA;
    GUID = PLUGIN_STATUSxMANAGER;
    break;
    default:
      gLog.warning(tr("Channel %u is not implemented"), channel);
      return;
  }

  buffer->PackChar(0x03);
  buffer->PackUnsignedLong(0x0000000A);
  buffer->PackUnsignedLong(nOurId);
  buffer->PackUnsignedLong(nSequence);
  if (nSequence == 0)  //we are initiating the connection
  {
    buffer->Pack(GUID, 16);
    buffer->PackUnsignedLong(0x00040001);
  }
  else
  {
    buffer->Pack(GUID, 8);
    buffer->PackUnsignedLong(0x00040001);
    buffer->Pack(GUID + 8, 8);
  }
}

CPacketTcp_Handshake_Confirm::CPacketTcp_Handshake_Confirm(CBuffer *inbuf)
{
  inbuf->incDataPosRead(5); //skip over junk
  m_nId = inbuf->UnpackUnsignedLong(); // some sort of id??
  inbuf->UnpackUnsignedLong(); // 0 in incomming, our id in outgoing

  char GUID[16];
  for (int i = 0; i < 16; i ++)
    (*inbuf) >> GUID[i];

  if (memcmp(GUID, PLUGIN_NORMAL, 16) == 0)
    myChannel = DcSocket::ChannelNormal;
  else if (memcmp(GUID, PLUGIN_INFOxMANAGER, 16) == 0)
    myChannel = DcSocket::ChannelInfo;
  else if (memcmp(GUID, PLUGIN_STATUSxMANAGER, 16) == 0)
    myChannel = DcSocket::ChannelStatus;
  else
  {
    gLog.warning(tr("Unknown channel GUID."));
    myChannel = DcSocket::ChannelUnknown;
  }
}

//=====PacketTcp================================================================
Licq::Buffer* CPacketTcp::Finalize(Licq::INetSocket *s)
{
  // Set the local port in the tcp packet now
  if (s != NULL && LocalPortOffset() != NULL)
  {
    LocalPortOffset()[0] = s->getLocalPort() & 0xFF;
    LocalPortOffset()[1] = (s->getLocalPort() >> 8) & 0xFF;
  }

  Encrypt_Client(buffer, m_nVersion);
  return buffer;
}

CPacketTcp::CPacketTcp(unsigned long _nCommand, unsigned short _nSubCommand, int channel,
    const string& message, bool _bAccept, unsigned short nLevel, User* user)
  : myChannel(channel)
{
  // Setup the message type and status fields using our online status
  Licq::OwnerReadGuard o(gIcqProtocol.ownerId());
  unsigned short s;
  if (user->statusToUser() != Licq::User::OfflineStatus)
    s = IcqProtocol::icqStatusFromStatus(user->statusToUser());
  else
    s = IcqProtocol::icqStatusFromStatus(o->status());
  m_nLevel = nLevel;
  m_nVersion = user->ConnectionVersion();
  if (m_nVersion >= 7)
  {
    if (nLevel & ICQ_TCPxMSG_URGENT)
    {
      nLevel &= ~ICQ_TCPxMSG_URGENT;
      nLevel |= ICQ_TCPxMSG_URGENT2;
    }
    else if (nLevel & ICQ_TCPxMSG_LIST)
    {
      nLevel &= ~ICQ_TCPxMSG_LIST;
      nLevel |= ICQ_TCPxMSG_LIST2; 
    }
  }

  switch(_nCommand)
  {
    case ICQ_CMDxTCP_CANCEL:
    case ICQ_CMDxTCP_START:
    {
      m_nStatus = 0;
      m_nMsgType = nLevel;
      if (m_nVersion >= 7)
      {
        m_nStatus = s;
        break;
      }

      switch (s)
      {
        case ICQ_STATUS_AWAY: m_nMsgType |= ICQ_TCPxMSG_FxAWAY; break;
        case ICQ_STATUS_NA: m_nMsgType |= ICQ_TCPxMSG_FxNA; break;
        case ICQ_STATUS_DND: m_nMsgType |= ICQ_TCPxMSG_FxDND; break;
        case ICQ_STATUS_OCCUPIED: m_nMsgType |= ICQ_TCPxMSG_FxOCCUPIED; break;
        case ICQ_STATUS_ONLINE:
        case ICQ_STATUS_FREEFORCHAT:
        default: m_nMsgType |= ICQ_TCPxMSG_FxONLINE; break;
      }
      if (o->isInvisible())
        m_nMsgType |= ICQ_TCPxMSG_FxINVISIBLE;
      break;
    }

    case ICQ_CMDxTCP_ACK:
    {
      m_nMsgType = ICQ_TCPxMSG_AUTOxREPLY;
      if (!_bAccept)
        m_nStatus = ICQ_TCPxACK_REFUSE;
      // If we are accepting a chat or file request then always say we are online
      else if (nLevel == ICQ_TCPxMSG_URGENT ||
               nLevel == ICQ_TCPxMSG_URGENT2 ||
               _nSubCommand == ICQ_CMDxSUB_CHAT ||
               _nSubCommand == ICQ_CMDxSUB_FILE)
        m_nStatus = ICQ_TCPxACK_ONLINE;
      else
      {
        switch (s)
        {
          case ICQ_STATUS_AWAY: m_nStatus = ICQ_TCPxACK_AWAY; break;
          case ICQ_STATUS_NA: m_nStatus = ICQ_TCPxACK_NA; break;
          case ICQ_STATUS_DND:
            m_nStatus = (!user->customAutoResponse().empty() && _nSubCommand == ICQ_CMDxTCP_READxDNDxMSG)
              ? ICQ_TCPxACK_DNDxCAR : ICQ_TCPxACK_DND;
            break;
          case ICQ_STATUS_OCCUPIED:
            m_nStatus = (!user->customAutoResponse().empty() && _nSubCommand == ICQ_CMDxTCP_READxOCCUPIEDxMSG)
              ? ICQ_TCPxACK_OCCUPIEDxCAR : ICQ_TCPxACK_OCCUPIED;
            break;
          case ICQ_STATUS_ONLINE:
          case ICQ_STATUS_FREEFORCHAT:
          default: m_nStatus = ICQ_TCPxACK_ONLINE; break;
        }
      }
      break;
    }
  }
  o.unlock();

  m_nSourceUin = gIcqProtocol.icqOwnerUin();
  m_nCommand = _nCommand;
  m_nSubCommand = _nSubCommand;
  myMessage = message;
  m_nLocalPort = user->LocalPort();

  // don't increment the sequence if this is an ack and cancel packet
  if (m_nCommand == ICQ_CMDxTCP_START) m_nSequence = user->Sequence(true);

  // Buffer and size are set by InitBuffer
  m_nSize = 0;
  buffer = NULL;
}

CPacketTcp::~CPacketTcp()
{
  delete buffer;
}


void CPacketTcp::InitBuffer()
{
  switch (m_nVersion)
  {
    case 6:
      InitBuffer_v6();
      break;
    case 4:
    case 5:
      InitBuffer_v4();
      break;
    case 2:
    case 3:
      InitBuffer_v2();
      break;
    case 7:
    case 8:
    default:
      InitBuffer_v7();
      break;
  }
}

void CPacketTcp::PostBuffer()
{
  switch (m_nVersion)
  {
    case 7:
    case 8:
      PostBuffer_v7();
      break;
    case 6:
      PostBuffer_v6();
      break;
    case 4:
    case 5:
      PostBuffer_v4();
      break;
    case 2:
    case 3:
      PostBuffer_v2();
      break;
  }

  if (buffer->getDataSize() != m_nSize)
  {
    gLog.warning(tr("Packet length (%lu) different than expected (%i)"),
        buffer->getDataSize(), m_nSize);
    *((uint16_t*)buffer->getDataStart()) = LE_16(buffer->getDataSize() - 2);
  }
}


void CPacketTcp::InitBuffer_v2()
{
  m_nSize += 35 + myMessage.size() + 4;
  if (m_nVersion != 2)
    m_nSize += 3;
  buffer = new CBuffer(m_nSize);
  buffer->packUInt16LE(m_nSize-2); // Packet length

  buffer->PackUnsignedLong(m_nSourceUin);
  buffer->PackUnsignedShort(m_nVersion == 2 ? 2 : ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(m_nCommand);
  buffer->PackUnsignedLong(m_nSourceUin);
  buffer->PackUnsignedShort(m_nSubCommand);
  buffer->pack(myMessage);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  m_szLocalPortOffset = buffer->getDataPosWrite();
  buffer->PackUnsignedLong(m_nLocalPort);
  buffer->PackChar(gIcqProtocol.directMode() ? MODE_DIRECT : MODE_INDIRECT);
  buffer->PackUnsignedShort(m_nStatus);
  buffer->PackUnsignedShort(m_nMsgType);
}

void CPacketTcp::PostBuffer_v2()
{
  buffer->PackUnsignedLong(m_nSequence);
  // several V2 clients don't like our extension, so we omit it for them
  if (m_nVersion != 2)
  {
    buffer->PackChar('L');
    buffer->PackUnsignedShort(LICQ_VERSION);
  }
}


void CPacketTcp::InitBuffer_v4()
{
  m_nSize += 39 + myMessage.size() + 7;
  buffer = new CBuffer(m_nSize);
  buffer->packUInt16LE(m_nSize-2); // Packet length

  buffer->PackUnsignedLong(m_nSourceUin);
  buffer->PackUnsignedShort(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(0x00000000);  // Checksum
  buffer->PackUnsignedLong(m_nCommand);
  buffer->PackUnsignedLong(m_nSourceUin);
  buffer->PackUnsignedShort(m_nSubCommand);
  buffer->pack(myMessage);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  m_szLocalPortOffset = buffer->getDataPosWrite();
  buffer->PackUnsignedLong(m_nLocalPort);
  buffer->PackChar(gIcqProtocol.directMode() ? MODE_DIRECT : MODE_INDIRECT);
  buffer->PackUnsignedShort(m_nStatus);
  buffer->PackUnsignedShort(m_nMsgType);
}

void CPacketTcp::PostBuffer_v4()
{
  buffer->PackUnsignedLong(m_nSequence);
  buffer->PackChar('L');
  buffer->PackUnsignedShort(LICQ_VERSION);
}


void CPacketTcp::InitBuffer_v6()
{
  m_nSize += 32 + myMessage.size() + 0;
  buffer = new CBuffer(m_nSize);
  buffer->packUInt16LE(m_nSize-2); // Packet length

  buffer->PackUnsignedLong(0); // Checksum
  buffer->PackUnsignedShort(m_nCommand);
  buffer->PackUnsignedShort(0x000E);  //???
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedLong(0); // Always zero, probably decryption validation
  buffer->PackUnsignedLong(0); //   ""
  buffer->PackUnsignedLong(0); //   ""
  buffer->PackUnsignedShort(m_nSubCommand);
  buffer->PackUnsignedShort(m_nStatus);
  buffer->PackUnsignedShort(m_nMsgType);
  buffer->PackUnsignedShort(myMessage.size());
  buffer->pack(myMessage);

  m_szLocalPortOffset = NULL;
}

void CPacketTcp::PostBuffer_v6()
{
// don't break ICQ2000
//   buffer->PackChar('L');
//   buffer->PackUnsignedShort(LICQ_VERSION);
}

void CPacketTcp::InitBuffer_v7()
{
  m_nSize += 31;
  if (channel() == DcSocket::ChannelNormal)
    m_nSize += 2 + myMessage.size();
  else
    m_nSize += 3;
  buffer = new CBuffer(m_nSize);
  buffer->packUInt16LE(m_nSize-2); // Packet length

  buffer->PackChar(0x02);
  buffer->PackUnsignedLong(0); // Checksum
  buffer->PackUnsignedShort(m_nCommand);
  buffer->PackUnsignedShort((channel() == DcSocket::ChannelNormal) ? 0x000E : 0x0012);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedShort(m_nSubCommand);
  buffer->PackUnsignedShort(m_nStatus);
  buffer->PackUnsignedShort((channel() == DcSocket::ChannelNormal) ? m_nMsgType : m_nLevel);

  if (channel() == DcSocket::ChannelNormal)
  {
    buffer->PackUnsignedShort(myMessage.size());
    buffer->pack(myMessage);
  }
  else
  {
    buffer->PackUnsignedShort(1);
    buffer->PackChar(myMessage[0]);
  }

  m_szLocalPortOffset = NULL;
}

void CPacketTcp::PostBuffer_v7()
{
}


//-----Message------------------------------------------------------------------
CPT_Message::CPT_Message(const string& message, unsigned short nLevel, bool bMR,
    const Licq::Color* pColor, User* pUser, bool isUtf8)
  : CPacketTcp(ICQ_CMDxTCP_START,
       ICQ_CMDxSUB_MSG | (bMR ? ICQ_CMDxSUB_FxMULTIREC : 0),
        DcSocket::ChannelNormal,
        message, true, nLevel, pUser)
{
  if (m_nVersion >= 6)
  {
    m_nSize += 8;
    if (isUtf8)
      m_nSize += 4 + sizeof(ICQ_CAPABILITY_UTF8_STR)-1;
  }
  InitBuffer();
  if (m_nVersion >= 6)
  {
    if (pColor == NULL)
    {
      buffer->PackUnsignedLong(0x00000000);
      buffer->PackUnsignedLong(0x00FFFFFF);
    }
    else
    {
      buffer->PackUnsignedLong(pColor->foreground());
      buffer->PackUnsignedLong(pColor->background());
    }

    if (isUtf8)
    {
      buffer->PackUnsignedLong(sizeof(ICQ_CAPABILITY_UTF8_STR)-1);
      buffer->Pack(ICQ_CAPABILITY_UTF8_STR, sizeof(ICQ_CAPABILITY_UTF8_STR)-1);
    }
  }
  PostBuffer();
}

//-----Url----------------------------------------------------------------------
CPT_Url::CPT_Url(const string& message, unsigned short nLevel, bool bMR,
    const Licq::Color* pColor, User* pUser)
  : CPacketTcp(ICQ_CMDxTCP_START,
       ICQ_CMDxSUB_URL | (bMR ? ICQ_CMDxSUB_FxMULTIREC : 0),
        DcSocket::ChannelNormal,
        message, true, nLevel, pUser)
{
  if (m_nVersion >= 6)
    m_nSize += 8;
  InitBuffer();
  if (m_nVersion >= 6)
  {
    if (pColor == NULL)
    {
      buffer->PackUnsignedLong(0x00000000);
      buffer->PackUnsignedLong(0x00FFFFFF);
    }
    else
    {
      buffer->PackUnsignedLong(pColor->foreground());
      buffer->PackUnsignedLong(pColor->background());
    }
  }
  PostBuffer();
}


//-----ContactList-----------------------------------------------------------
CPT_ContactList::CPT_ContactList(const string& message, unsigned short nLevel, bool bMR,
    const Licq::Color* pColor, User* pUser)
  : CPacketTcp(ICQ_CMDxTCP_START,
       ICQ_CMDxSUB_CONTACTxLIST | (bMR ? ICQ_CMDxSUB_FxMULTIREC : 0),
        DcSocket::ChannelNormal,
        message, true, nLevel, pUser)
{
  if (m_nVersion >= 6)
    m_nSize += 8;
  InitBuffer();
  if (m_nVersion >= 6)
  {
    if (pColor == NULL)
    {
      buffer->PackUnsignedLong(0x00000000);
      buffer->PackUnsignedLong(0x00FFFFFF);
    }
    else
    {
      buffer->PackUnsignedLong(pColor->foreground());
      buffer->PackUnsignedLong(pColor->background());
    }
  }
  PostBuffer();
}


//-----ReadAwayMessage----------------------------------------------------------
CPT_ReadAwayMessage::CPT_ReadAwayMessage(User* _cUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxTCP_READxAWAYxMSG,
        DcSocket::ChannelNormal, "", true, ICQ_TCPxMSG_AUTOxREPLY, _cUser)
{
  // Properly set the subcommand to get the correct away message
  unsigned status = _cUser->status();
  if (status & Licq::User::DoNotDisturbStatus)
    m_nSubCommand = ICQ_CMDxTCP_READxDNDxMSG;
  else if (status & Licq::User::OccupiedStatus)
    m_nSubCommand = ICQ_CMDxTCP_READxOCCUPIEDxMSG;
  else if (status & Licq::User::NotAvailableStatus)
    m_nSubCommand = ICQ_CMDxTCP_READxNAxMSG;
  else if (status & Licq::User::AwayStatus)
    m_nSubCommand = ICQ_CMDxTCP_READxAWAYxMSG;
  else if (status & Licq::User::FreeForChatStatus)
    m_nSubCommand = ICQ_CMDxTCP_READxFFCxMSG;
  else
    m_nSubCommand = ICQ_CMDxTCP_READxAWAYxMSG;

  if (m_nVersion == 6)
    m_nSize += 8;
  InitBuffer();
  if (m_nVersion == 6)
  {
    buffer->PackUnsignedLong(0xFFFFFFFF);
    buffer->PackUnsignedLong(0xFFFFFFFF);
  }
  PostBuffer();
}

//-----ChatRequest--------------------------------------------------------------
CPT_ChatRequest::CPT_ChatRequest(const string& message, const string& chatUsers,
   unsigned short nPort, unsigned short nLevel, User* pUser, bool bICBM)
  : CPacketTcp(ICQ_CMDxTCP_START, bICBM ? ICQ_CMDxSUB_ICBM : ICQ_CMDxSUB_CHAT,
        DcSocket::ChannelNormal,
        bICBM ? "" : message, true, nLevel, pUser)
{
  m_nSize += 2 + chatUsers.size() + 1 + 8;
  if (bICBM)
    m_nSize += 47 + message.size() + 21;

  InitBuffer();

  if (bICBM)
  {
    buffer->PackUnsignedShort(0x3A);
    buffer->PackUnsignedLongBE(0xBFF720B2);
    buffer->PackUnsignedLongBE(0x378ED411);
    buffer->PackUnsignedLongBE(0xBD280004);
    buffer->PackUnsignedLongBE(0xAC96D905);
    buffer->PackUnsignedShort(0);

    buffer->PackUnsignedLong(21);
    buffer->Pack("Send / Start ICQ Chat", 21);
 
    buffer->PackUnsignedLongBE(0x00000100);
    buffer->PackUnsignedLongBE(0x00010000);
    buffer->PackUnsignedLongBE(0);
    buffer->PackUnsignedShortBE(0);
    buffer->PackChar(0);

    buffer->PackUnsignedLong(message.size() + chatUsers.size() + 15);

    buffer->PackUnsignedLong(message.size());

    if (!message.empty())
      buffer->pack(message);

    buffer->packString(chatUsers);

    buffer->PackUnsignedShortBE(nPort);
    buffer->PackUnsignedShort(0);
    buffer->PackUnsignedShort(nPort);
    buffer->PackUnsignedShort(0);
  }
  else
  {
    buffer->packString(chatUsers);
    buffer->packUInt16BE(nPort);
    buffer->packUInt16BE(0);
    buffer->PackUnsignedLong(nPort);
  }

  PostBuffer();
}


//-----FileTransfer--------------------------------------------------------------
CPT_FileTransfer::CPT_FileTransfer(const list<string>& lFileList, const string& filename,
    const string& description, unsigned short nLevel, User* _cUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_FILE, DcSocket::ChannelNormal,
        description, true, nLevel, _cUser),
    m_lFileList(lFileList.begin(), lFileList.end())
{
  m_bValid = false;
  m_nFileSize = 0;

  list<string>::iterator it;
  for (it = m_lFileList.begin(); it != m_lFileList.end(); ++it)
  {
    // Check file exists and get size
    struct stat buf;
    if (!(it->empty() || stat(it->c_str(), &buf) < 0))
    {
       m_nFileSize += buf.st_size;
       m_bValid = true;
    }
  }

  // Remove path from filename (if it exists)
  myFilename = filename;
  size_t posSlash = myFilename.rfind('/');
  if (posSlash != string::npos)
    myFilename.erase(0, posSlash+1);

	if (!m_bValid)  return;

  m_nSize += 15 + myFilename.size();
  InitBuffer();

  buffer->PackUnsignedLong(0);
  buffer->packString(myFilename);
  buffer->PackUnsignedLong(m_nFileSize);
  buffer->PackUnsignedLong(0);

  PostBuffer();
}


//-----Key------------------------------------------------------------------
CPT_OpenSecureChannel::CPT_OpenSecureChannel(User* _cUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_SECURExOPEN,
        DcSocket::ChannelNormal,
       "", true, ICQ_TCPxMSG_NORMAL, _cUser)
{
  InitBuffer();
  PostBuffer();
}


CPT_CloseSecureChannel::CPT_CloseSecureChannel(User* _cUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_SECURExCLOSE,
        DcSocket::ChannelNormal,
       "", true, ICQ_TCPxMSG_NORMAL, _cUser)
{
  InitBuffer();
  PostBuffer();
}


//+++++Ack++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CPT_Ack::CPT_Ack(unsigned short _nSubCommand, unsigned short _nSequence,
    bool _bAccept, bool l, User* pUser)
  : CPacketTcp(ICQ_CMDxTCP_ACK, _nSubCommand, DcSocket::ChannelNormal,
      "", _bAccept, l ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL, pUser)
{
  m_nSequence = _nSequence;
  Licq::OwnerReadGuard o(gIcqProtocol.ownerId());

  // don't sent out AutoResponse if we're online
  // it could contain stuff the other site shouldn't be able to read
  // also some clients always pop up the auto response
  // window when they receive one, annoying for them..
  if(((pUser->statusToUser() != Licq::User::OfflineStatus &&
      pUser->statusToUser() != Licq::User::OnlineStatus)  ?
      pUser->statusToUser() : o->status()) != Licq::User::OfflineStatus)
  {
    myMessage = pUser->usprintf(o->autoResponse(), Licq::User::usprintf_quotepipe, true);

    if (!pUser->customAutoResponse().empty())
    {
      myMessage += "\r\n--------------------\r\n";
      myMessage += pUser->usprintf(gTranslator.fromUtf8(pUser->customAutoResponse()),
          Licq::User::usprintf_quotepipe, true);
    }
  }
  else
    myMessage.clear();

  myMessage = IcqProtocol::pipeInput(myMessage);
}

CPT_Ack::~CPT_Ack()
{
  // Empty
}

//-----AckGeneral---------------------------------------------------------------
CPT_AckGeneral::CPT_AckGeneral(unsigned short nCmd, unsigned short nSequence,
    bool bAccept, bool nLevel, User* pUser)
  : CPT_Ack(nCmd, nSequence, bAccept, nLevel, pUser)
{
  if (m_nVersion >= 6)
    m_nSize += 8;
  InitBuffer();
  if (m_nVersion == 6)
  {
    buffer->PackUnsignedLong(0x00000000);
    buffer->PackUnsignedLong(0x00000000);
  }
  else if (m_nVersion >= 7)
	{
    buffer->PackUnsignedLong(0x00000000);
    buffer->PackUnsignedLong(0x00FFFFFF);
  }

  PostBuffer();
}


//-----AckKey---------------------------------------------------------------
CPT_AckOpenSecureChannel::CPT_AckOpenSecureChannel(unsigned short nSequence,
    bool ok, User* pUser)
  : CPT_Ack(ICQ_CMDxSUB_SECURExOPEN, nSequence, true, true, pUser)
{
  myMessage = (ok ? "1" : "");
  InitBuffer();
  PostBuffer();
}


CPT_AckOldSecureChannel::CPT_AckOldSecureChannel(unsigned short nSequence,
    User* pUser)
  : CPT_Ack(ICQ_CMDxSUB_SECURExOPEN, nSequence, true, true, pUser)
{
  myMessage.clear();
  if (m_nVersion == 6)
    m_nSize += 8;

  InitBuffer();
  if (m_nVersion == 6)
  {
    buffer->PackUnsignedLong(0x00000000);
    buffer->PackUnsignedLong(0x00000000);
  }
  PostBuffer();
}


CPT_AckCloseSecureChannel::CPT_AckCloseSecureChannel(unsigned short nSequence,
    User* pUser)
  : CPT_Ack(ICQ_CMDxSUB_SECURExCLOSE, nSequence, true, true, pUser)
{
  myMessage.clear();
  InitBuffer();
  PostBuffer();
}



#if 0
//-----AckMessage---------------------------------------------------------------
CPT_AckMessage::CPT_AckMessage(unsigned short _nSequence, bool _bAccept,
    bool nLevel, User* _cUser)
  : CPT_Ack(ICQ_CMDxSUB_MSG, _nSequence, _bAccept, nLevel, _cUser)
{
  InitBuffer();
  PostBuffer();
}



//-----AckReadAwayMessage-------------------------------------------------------
CPT_AckReadAwayMessage::CPT_AckReadAwayMessage(unsigned short _nSubCommand,
    unsigned short _nSequence, bool _bAccept, User* _cUser)
  : CPT_Ack(_nSubCommand, _nSequence, _bAccept, false, _cUser)
{
  InitBuffer();
  PostBuffer();
}


//-----AckUrl-------------------------------------------------------------------
CPT_AckUrl::CPT_AckUrl(unsigned short _nSequence, bool _bAccept, bool nLevel,
    User* _cUser)
  : CPT_Ack(ICQ_CMDxSUB_URL, _nSequence, _bAccept, nLevel, _cUser)
{
  InitBuffer();
  PostBuffer();
}


//-----AckContactList--------------------------------------------------------
CPT_AckContactList::CPT_AckContactList(unsigned short _nSequence, bool _bAccept,
    bool nLevel, User* _cUser)
  : CPT_Ack(ICQ_CMDxSUB_CONTACTxLIST, _nSequence, _bAccept, nLevel, _cUser)
{
  InitBuffer();
  PostBuffer();
}
#endif

//-----AckChatRefuse------------------------------------------------------------
CPT_AckChatRefuse::CPT_AckChatRefuse(const string& reason,
    unsigned short _nSequence, User *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_CHAT, _nSequence, false, false, _cUser)
{
  myMessage = reason;
  char temp_1[11] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  m_nSize += 11;
  InitBuffer();
  buffer->Pack(temp_1, 11);
  PostBuffer();
}


//-----AckChatAccept------------------------------------------------------------
CPT_AckChatAccept::CPT_AckChatAccept(unsigned short _nPort, const string& clients,
    unsigned short _nSequence, User* _cUser, bool bICBM)
  : CPT_Ack(bICBM ? ICQ_CMDxSUB_ICBM : ICQ_CMDxSUB_CHAT, _nSequence, true, true, _cUser)
{
  m_nPort = _nPort;
  m_nStatus = ICQ_TCPxACK_ONLINE;

  m_nSize += 11 + clients.size();
  if (bICBM)
    m_nSize += 68;

  InitBuffer();

  if (bICBM)
  {
    buffer->PackUnsignedShort(0x3A);
    buffer->PackUnsignedLongBE(0xBFF720B2);
    buffer->PackUnsignedLongBE(0x378ED411);
    buffer->PackUnsignedLongBE(0xBD280004);
    buffer->PackUnsignedLongBE(0xAC96D905);
    buffer->PackUnsignedShort(0);

    buffer->PackUnsignedLong(21);
    buffer->Pack("Send / Start ICQ Chat", 21);
 
    buffer->PackUnsignedLongBE(0x00000100);
    buffer->PackUnsignedLongBE(0x00010000);
    buffer->PackUnsignedLongBE(0);
    buffer->PackUnsignedShortBE(0);
    buffer->PackChar(0);

    buffer->PackUnsignedLong(15 + clients.size());

    buffer->PackUnsignedLong(0);

    buffer->packString(clients);

    buffer->PackUnsignedShortBE(m_nPort);
    buffer->PackUnsignedShort(0);
    buffer->PackUnsignedShort(m_nPort);
    buffer->PackUnsignedShort(0);
  }
  else
  {
    buffer->PackString("");
    buffer->packUInt16BE(m_nPort);
    buffer->packUInt16BE(0);
    buffer->PackUnsignedLong(m_nPort);
  }

  PostBuffer();
}


//-----AckFileRefuse------------------------------------------------------------
CPT_AckFileRefuse::CPT_AckFileRefuse(const string& reason,
    unsigned short _nSequence, User* _cUser)
  : CPT_Ack(ICQ_CMDxSUB_FILE, _nSequence, false, false, _cUser)
{
  myMessage = reason;

  m_nSize += 15;
  InitBuffer();

  buffer->PackUnsignedLong(0);
  buffer->PackString("");
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);

  PostBuffer();
}


//-----AckFileAccept------------------------------------------------------------
CPT_AckFileAccept::CPT_AckFileAccept(unsigned short _nPort,
    unsigned short _nSequence, User* _cUser)
  : CPT_Ack(ICQ_CMDxSUB_FILE, _nSequence, true, true, _cUser)
{
  m_nFileSize = 0;
  m_nPort = _nPort;
  m_nStatus = ICQ_TCPxACK_ONLINE;

  m_nSize += 15;
  InitBuffer();

  buffer->packUInt16BE(m_nPort);
  buffer->packUInt16BE(0);
  buffer->PackString("");
  buffer->PackUnsignedLong(m_nFileSize);
  buffer->PackUnsignedLong(m_nPort);

  PostBuffer();
}


//+++++Cancel+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CPT_Cancel::CPT_Cancel(unsigned short _nSubCommand, unsigned short _nSequence,
    User* _cUser)
  : CPacketTcp(ICQ_CMDxTCP_CANCEL, _nSubCommand, DcSocket::ChannelNormal, "", true, 0, _cUser)
{
  m_nSequence = _nSequence;
}



//-----CancelChat---------------------------------------------------------------
CPT_CancelChat::CPT_CancelChat(unsigned short _nSequence, User* _cUser)
  : CPT_Cancel(ICQ_CMDxSUB_CHAT, _nSequence, _cUser)
{
  char temp_1[11] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  m_nSize += 11;
  InitBuffer();
  buffer->Pack(temp_1, 11);
  PostBuffer();
}


//-----CancelFile---------------------------------------------------------------
CPT_CancelFile::CPT_CancelFile(unsigned short _nSequence, User* _cUser)
  : CPT_Cancel(ICQ_CMDxSUB_FILE, _nSequence, _cUser)
{
  m_nSize += 15;
  InitBuffer();

  buffer->PackUnsignedLong(0);
  buffer->PackString("");
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);

  PostBuffer();
}

//-----Send error reply------------------------------------------------------
CPT_PluginError::CPT_PluginError(User* _cUser, unsigned short nSequence,
    int channel)
  : CPacketTcp(ICQ_CMDxTCP_ACK, 0, channel, "\x03", true, 0, _cUser)
{
  m_nSequence = nSequence;

  InitBuffer();
  PostBuffer();
}

//-----Send info plugin request------------------------------------------------
CPT_InfoPluginReq::CPT_InfoPluginReq(User* _cUser, const uint8_t* GUID,
  unsigned long nTime)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_MSG, DcSocket::ChannelInfo, "", true, 0, _cUser)
{
  m_nSize += GUID_LENGTH + 4;
  memcpy(m_ReqGUID, GUID, GUID_LENGTH);

  InitBuffer();

  buffer->Pack(GUID, GUID_LENGTH);

  buffer->PackUnsignedLong(nTime);

  PostBuffer();
}

//----Reply to phone book request-----------------------------------------------
CPT_InfoPhoneBookResp::CPT_InfoPhoneBookResp(User* _cUser,
  unsigned short nSequence)
  : CPacketTcp(ICQ_CMDxTCP_ACK, 0, DcSocket::ChannelInfo, "\x01", true, ICQ_TCPxMSG_URGENT2, _cUser)
{
  OwnerReadGuard o(gIcqProtocol.ownerId());
  const Licq::IcqPhoneBookVector& book = o->getPhoneBook();

  unsigned long nLen = 4 + 4;
  BOOST_FOREACH(const struct Licq::PhoneBookEntry& entry, book)
  {
    nLen += 4 + entry.description.size() + 4 + entry.areaCode.size()
        + 4 + entry.phoneNumber.size() + 4 + entry.extension.size()
        + 4 + entry.country.size() + 4 + 4 + 4
        + 4 + entry.gateway.size() + 4 + 4 + 4 + 4;
  }

  m_nSize += 2 + 2 + 4 + 4 + nLen;
  m_nSequence = nSequence;
  InitBuffer();

  buffer->PackUnsignedShort(0);   //Unknown
  buffer->PackUnsignedShort(1);   //Unknown
  buffer->PackUnsignedLong(o->ClientInfoTimestamp());
  buffer->PackUnsignedLong(nLen);  //Bytes remaining in packet

  buffer->PackUnsignedLong(ICQ_PLUGIN_RESP_PHONExBOOK); //Response ID

  buffer->PackUnsignedLong(book.size());

  BOOST_FOREACH(const struct Licq::PhoneBookEntry& entry, book)
  {
    buffer->packString32LE(entry.description);
    buffer->packString32LE(entry.areaCode);
    buffer->packString32LE(entry.phoneNumber);
    buffer->packString32LE(entry.extension);
    buffer->packString32LE(entry.country);
    buffer->packUInt32LE(entry.nActive);
  }

  BOOST_FOREACH(const struct Licq::PhoneBookEntry& entry, book)
  {
    buffer->packUInt32LE(4 + 4 + entry.gateway.size() + 4 + 4 + 4 + 4);
    buffer->packUInt32LE(entry.nType);
    buffer->packString32LE(entry.gateway);
    buffer->packUInt32LE(entry.nGatewayType);
    buffer->packUInt32LE(entry.nSmsAvailable);
    buffer->packUInt32LE(entry.nRemoveLeading0s);
    buffer->packUInt32LE(entry.nPublish);
  }

  PostBuffer();
}

//----Reply to picture request--------------------------------------------------
CPT_InfoPictureResp::CPT_InfoPictureResp(User* _cUser, unsigned short nSequence)
  : CPacketTcp(ICQ_CMDxTCP_ACK, 0, DcSocket::ChannelInfo, "\x01", true, ICQ_TCPxMSG_URGENT2, _cUser)
{
  OwnerReadGuard o(gIcqProtocol.ownerId());
  string filename = o->pictureFileName();
  unsigned long nLen = 0, nFileLen = 0;
  int fd = -1;
  if (o->GetPicturePresent())
  {
    fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1)
    {
      gLog.error(tr("Unable to open picture file (%s): %s."),
          filename.c_str(), strerror(errno));
    }
    else
    {
      struct stat fi;
      if (fstat(fd, &fi) == -1)
      {
        gLog.error(tr("Unable to stat picture file (%s):%s."),
            filename.c_str(), strerror(errno));
      }
      else
      {
        nFileLen = fi.st_size;
        nLen = 4 + 4 + 1 + 4 + nFileLen;
      }
    }
  }

  m_nSize += 2 + 2 + 4 + 4 + nLen;
  m_nSequence = nSequence;
  InitBuffer();

  buffer->PackUnsignedShort(0);   //Unknown
  buffer->PackUnsignedShort(1);   //Unknown
  buffer->PackUnsignedLong(o->ClientInfoTimestamp());
  buffer->PackUnsignedLong(nLen);  //Bytes remaining in packet

  if (nLen != 0)
  {
    buffer->PackUnsignedLong(ICQ_PLUGIN_RESP_PICTURE); //Response ID

    buffer->PackUnsignedLong(1); //filename length
    buffer->PackChar('p'); //windows icq needs a filename

    buffer->PackUnsignedLong(nFileLen);

    char buf[8192];
    unsigned long nRead = 0;
    while (nRead < nFileLen)
    {
      unsigned long nToRead;
      if (sizeof(buf) < nFileLen - nRead)
        nToRead = sizeof(buf);
      else
        nToRead = nFileLen - nRead;

      ssize_t nBytesRead = read(fd, buf, nToRead);
      if (nBytesRead == -1)
      {
        gLog.error(tr("Failed to read file (%s): %s."),
            filename.c_str(), strerror(errno));
        break;
      }
      if (nBytesRead == 0)
      {
        gLog.error(tr("Premature end of file (%s): %s."),
            filename.c_str(), strerror(errno));
        break;
      }

      nRead += nBytesRead;
      for (ssize_t i = 0; i < nBytesRead; i++)
        buffer->PackChar(buf[i]);
    }

    if (nRead < nFileLen)
    {
      //failed to read as much as predicted, fill with 0s
      for (; nRead < nFileLen; nRead++)
        buffer->PackChar(0);
    }
  }

  if (fd != -1)
    close(fd);
}

//----Reply to plugin list request----------------------------------------------
CPT_InfoPluginListResp::CPT_InfoPluginListResp(User* _cUser, unsigned short nSequence)
  : CPacketTcp(ICQ_CMDxTCP_ACK, 0, DcSocket::ChannelInfo, "\x01", true, ICQ_TCPxMSG_URGENT2, _cUser)
{
  unsigned long num_plugins = sizeof(IcqProtocol::info_plugins)/sizeof(struct PluginList);

  unsigned long nLen;
  if (num_plugins == 0)
    nLen = 0;
  else
  {
    nLen = 4 + 4;
    for (unsigned long i = 0; i < num_plugins; i ++)
    {
      nLen += GUID_LENGTH + 2 + 2 + 4 + strlen(IcqProtocol::info_plugins[i].name)
          + 4 + strlen(IcqProtocol::info_plugins[i].description) + 4;
    }
  }

  m_nSize += 2 + 2 + 4 + 4 + nLen;
  m_nSequence = nSequence;
  InitBuffer();

  buffer->PackUnsignedShort(0);   //Unknown
  buffer->PackUnsignedShort(1);   //Unknown
  {
    OwnerReadGuard o(gIcqProtocol.ownerId());
    buffer->PackUnsignedLong(o->ClientInfoTimestamp());
  }
  buffer->PackUnsignedLong(nLen);  //Bytes remaining in packet
  if (nLen != 0)
  {
    buffer->PackUnsignedLong(ICQ_PLUGIN_RESP_INFOxLIST); //Response ID
    buffer->PackUnsignedLong(num_plugins);
    for (unsigned long i = 0; i < num_plugins; i++)
    {
      buffer->packRaw(IcqProtocol::info_plugins[i].guid, GUID_LENGTH);

      buffer->PackUnsignedShort(0); //Unknown
      buffer->PackUnsignedShort(1); //Unknown

      buffer->packString32LE(IcqProtocol::info_plugins[i].name,
          strlen(IcqProtocol::info_plugins[i].name));

      buffer->packString32LE(IcqProtocol::info_plugins[i].description,
          strlen(IcqProtocol::info_plugins[i].description));

      buffer->PackUnsignedLong(0);  //Unknown
    }
  }
  PostBuffer();
}

//-----Send status plugin request----------------------------------------------
CPT_StatusPluginReq::CPT_StatusPluginReq(User* _cUser, const uint8_t* GUID,
  unsigned long nTime)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_MSG, DcSocket::ChannelStatus, "", true, 0, _cUser)
{
  m_nSize += GUID_LENGTH + 4;
  memcpy(m_ReqGUID, GUID, GUID_LENGTH);

  InitBuffer();

  buffer->Pack(GUID, GUID_LENGTH);

  buffer->PackUnsignedLong(nTime);

  PostBuffer();
}

//----Reply to plugin list request----------------------------------------------
CPT_StatusPluginListResp::CPT_StatusPluginListResp(User* _cUser, unsigned short nSequence)
  : CPacketTcp(ICQ_CMDxTCP_ACK, 0, DcSocket::ChannelStatus, "\x01", true, 0, _cUser)
{
  unsigned long num_plugins = sizeof(IcqProtocol::status_plugins)/sizeof(struct PluginList);

  unsigned long nLen;
  if (num_plugins == 0)
    nLen = 0;
  else
  {
    nLen = 4 + 4;
    for (unsigned long i = 0; i < num_plugins; i ++)
    {
      nLen += GUID_LENGTH + 2 + 2 + 4 + strlen(IcqProtocol::status_plugins[i].name)
          + 4 + strlen(IcqProtocol::status_plugins[i].description) + 4;
    }
  }

  m_nSize += 13 + 4 + 4 + nLen;
  m_nSequence = nSequence;
  InitBuffer();

  buffer->PackUnsignedShort(0);   //Unknown
  buffer->PackUnsignedShort(1);   //Unknown
  buffer->PackUnsignedLong(0);    //Unknown
  buffer->PackUnsignedLong(0);    //Unknown
  buffer->PackChar(1);            //Unknown
  {
    OwnerReadGuard o(gIcqProtocol.ownerId());
    buffer->PackUnsignedLong(o->ClientStatusTimestamp());
  }
  buffer->PackUnsignedLong(nLen);  //Bytes remaining in packet
  if (nLen != 0)
  {
    buffer->PackUnsignedLong(ICQ_PLUGIN_RESP_STATUSxLIST); //Response ID
    buffer->PackUnsignedLong(num_plugins);
    for (unsigned long i = 0; i < num_plugins; i++)
    {
      buffer->packRaw(IcqProtocol::status_plugins[i].guid, GUID_LENGTH);

      buffer->PackUnsignedShort(0); //Unknown
      buffer->PackUnsignedShort(1); //Unknown

      buffer->packString32LE(IcqProtocol::status_plugins[i].name,
          strlen(IcqProtocol::status_plugins[i].name));

      buffer->packString32LE(IcqProtocol::status_plugins[i].description,
          strlen(IcqProtocol::status_plugins[i].description));

      buffer->PackUnsignedLong(0);  //Unknown
    }
  }
  PostBuffer();
}

//----Reply to status request--------------------------------------------------
CPT_StatusPluginResp::CPT_StatusPluginResp(User* _cUser,
  unsigned short nSequence,
  unsigned long nStatus)
  : CPacketTcp(ICQ_CMDxTCP_ACK, 0, DcSocket::ChannelStatus, "\x02", true, 0, _cUser)
{
  m_nSize += 2 + 2 + 4 + 4 + 1;
  m_nSequence = nSequence;
  InitBuffer();

  buffer->PackUnsignedShort(0);   //Unknown
  buffer->PackUnsignedShort(1);   //Unknown
  buffer->PackUnsignedLong(nStatus);
  {
    OwnerReadGuard o(gIcqProtocol.ownerId());
    buffer->PackUnsignedLong(o->ClientStatusTimestamp());
  }
  buffer->PackChar(1);            //Unknown

  PostBuffer();
}
