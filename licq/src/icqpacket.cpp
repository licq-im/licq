// -*- c-basic-offset: 2 -*-

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#include "time-fix.h"

#include "licq_packets.h"
#include "licq_icq.h"
#include "licq_translate.h"
#include "licq_log.h"
#include "licq_utility.h"
#include "licq_color.h"
#include "support.h"


unsigned short ReversePort(unsigned short p)
{
  return ((p >> 8) & 0xFF) + ((p & 0xFF) << 8);
}

#define DEBUG_ENCRYPTION(x)
//#define DEBUG_ENCRYPTION(x) printf x

#if ICQ_VERSION == 4
static unsigned char icq_check_data[256] = {
  0x0a, 0x5b, 0x31, 0x5d, 0x20, 0x59, 0x6f, 0x75,
  0x20, 0x63, 0x61, 0x6e, 0x20, 0x6d, 0x6f, 0x64,
  0x69, 0x66, 0x79, 0x20, 0x74, 0x68, 0x65, 0x20,
  0x73, 0x6f, 0x75, 0x6e, 0x64, 0x73, 0x20, 0x49,
  0x43, 0x51, 0x20, 0x6d, 0x61, 0x6b, 0x65, 0x73,
  0x2e, 0x20, 0x4a, 0x75, 0x73, 0x74, 0x20, 0x73,
  0x65, 0x6c, 0x65, 0x63, 0x74, 0x20, 0x22, 0x53,
  0x6f, 0x75, 0x6e, 0x64, 0x73, 0x22, 0x20, 0x66,
  0x72, 0x6f, 0x6d, 0x20, 0x74, 0x68, 0x65, 0x20,
  0x22, 0x70, 0x72, 0x65, 0x66, 0x65, 0x72, 0x65,
  0x6e, 0x63, 0x65, 0x73, 0x2f, 0x6d, 0x69, 0x73,
  0x63, 0x22, 0x20, 0x69, 0x6e, 0x20, 0x49, 0x43,
  0x51, 0x20, 0x6f, 0x72, 0x20, 0x66, 0x72, 0x6f,
  0x6d, 0x20, 0x74, 0x68, 0x65, 0x20, 0x22, 0x53,
  0x6f, 0x75, 0x6e, 0x64, 0x73, 0x22, 0x20, 0x69,
  0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6f,
  0x6e, 0x74, 0x72, 0x6f, 0x6c, 0x20, 0x70, 0x61,
  0x6e, 0x65, 0x6c, 0x2e, 0x20, 0x43, 0x72, 0x65,
  0x64, 0x69, 0x74, 0x3a, 0x20, 0x45, 0x72, 0x61,
  0x6e, 0x0a, 0x5b, 0x32, 0x5d, 0x20, 0x43, 0x61,
  0x6e, 0x27, 0x74, 0x20, 0x72, 0x65, 0x6d, 0x65,
  0x6d, 0x62, 0x65, 0x72, 0x20, 0x77, 0x68, 0x61,
  0x74, 0x20, 0x77, 0x61, 0x73, 0x20, 0x73, 0x61,
  0x69, 0x64, 0x3f, 0x20, 0x20, 0x44, 0x6f, 0x75,
  0x62, 0x6c, 0x65, 0x2d, 0x63, 0x6c, 0x69, 0x63,
  0x6b, 0x20, 0x6f, 0x6e, 0x20, 0x61, 0x20, 0x75,
  0x73, 0x65, 0x72, 0x20, 0x74, 0x6f, 0x20, 0x67,
  0x65, 0x74, 0x20, 0x61, 0x20, 0x64, 0x69, 0x61,
  0x6c, 0x6f, 0x67, 0x20, 0x6f, 0x66, 0x20, 0x61,
  0x6c, 0x6c, 0x20, 0x6d, 0x65, 0x73, 0x73, 0x61,
  0x67, 0x65, 0x73, 0x20, 0x73, 0x65, 0x6e, 0x74,
  0x20, 0x69, 0x6e, 0x63, 0x6f, 0x6d, 0x69, 0x6e,
};
#elif ICQ_VERSION == 5
static unsigned char icq_check_data[256] = {
  0x59, 0x60, 0x37, 0x6B, 0x65, 0x62, 0x46, 0x48,
  0x53, 0x61, 0x4C, 0x59, 0x60, 0x57, 0x5B, 0x3D,
  0x5E, 0x34, 0x6D, 0x36, 0x50, 0x3F, 0x6F, 0x67,
  0x53, 0x61, 0x4C, 0x59, 0x40, 0x47, 0x63, 0x39,
  0x50, 0x5F, 0x5F, 0x3F, 0x6F, 0x47, 0x43, 0x69,
  0x48, 0x33, 0x31, 0x64, 0x35, 0x5A, 0x4A, 0x42,
  0x56, 0x40, 0x67, 0x53, 0x41, 0x07, 0x6C, 0x49,
  0x58, 0x3B, 0x4D, 0x46, 0x68, 0x43, 0x69, 0x48,
  0x33, 0x31, 0x44, 0x65, 0x62, 0x46, 0x48, 0x53,
  0x41, 0x07, 0x6C, 0x69, 0x48, 0x33, 0x51, 0x54,
  0x5D, 0x4E, 0x6C, 0x49, 0x38, 0x4B, 0x55, 0x4A,
  0x62, 0x46, 0x48, 0x33, 0x51, 0x34, 0x6D, 0x36,
  0x50, 0x5F, 0x5F, 0x5F, 0x3F, 0x6F, 0x47, 0x63,
  0x59, 0x40, 0x67, 0x33, 0x31, 0x64, 0x35, 0x5A,
  0x6A, 0x52, 0x6E, 0x3C, 0x51, 0x34, 0x6D, 0x36,
  0x50, 0x5F, 0x5F, 0x3F, 0x4F, 0x37, 0x4B, 0x35,
  0x5A, 0x4A, 0x62, 0x66, 0x58, 0x3B, 0x4D, 0x66,
  0x58, 0x5B, 0x5D, 0x4E, 0x6C, 0x49, 0x58, 0x3B,
  0x4D, 0x66, 0x58, 0x3B, 0x4D, 0x46, 0x48, 0x53,
  0x61, 0x4C, 0x59, 0x40, 0x67, 0x33, 0x31, 0x64,
  0x55, 0x6A, 0x32, 0x3E, 0x44, 0x45, 0x52, 0x6E,
  0x3C, 0x31, 0x64, 0x55, 0x6A, 0x52, 0x4E, 0x6C,
  0x69, 0x48, 0x53, 0x61, 0x4C, 0x39, 0x30, 0x6F,
  0x47, 0x63, 0x59, 0x60, 0x57, 0x5B, 0x3D, 0x3E,
  0x64, 0x35, 0x3A, 0x3A, 0x5A, 0x6A, 0x52, 0x4E,
  0x6C, 0x69, 0x48, 0x53, 0x61, 0x6C, 0x49, 0x58,
  0x3B, 0x4D, 0x46, 0x68, 0x63, 0x39, 0x50, 0x5F,
  0x5F, 0x3F, 0x6F, 0x67, 0x53, 0x41, 0x25, 0x41,
  0x3C, 0x51, 0x54, 0x3D, 0x5E, 0x54, 0x5D, 0x4E,
  0x4C, 0x39, 0x50, 0x5F, 0x5F, 0x5F, 0x3F, 0x6F,
  0x47, 0x43, 0x69, 0x48, 0x33, 0x51, 0x54, 0x5D,
  0x6E, 0x3C, 0x31, 0x64, 0x35, 0x5A, 0x00, 0x00,
};
#endif

void Encrypt_Server(CBuffer *buffer)
{
#if ICQ_VERSION == 2
// No encryption in V2
#elif ICQ_VERSION == 4
  // This speeds things up if no one is logging packets
  if (gLog.LoggingPackets())
  {
    char *b;
    gLog.Packet("%sUnencrypted Packet:\n%s\n", L_PACKETxSTR, buffer->print(b));
    delete [] b;
  }

  unsigned long nCheckSum = 0;
  unsigned long l = buffer->getDataSize();
  unsigned char *buf = (unsigned char *)buffer->getDataStart();

  unsigned long chk1 = ( buf[8] << 24) |
                       ( buf[4] << 16 ) |
                       ( buf[2] << 8 ) |
                       ( buf[6] );
  unsigned short r1 = rand() % (l - 4);
  unsigned short r2 = rand() & 0xFF;
  unsigned long chk2 = ( r1 << 24 ) |
                       ( buf[r1] << 16 ) |
                       ( r2 << 8 ) |
                       ( icq_check_data[r2] );
  chk2 ^= 0x00FF00FF;
  nCheckSum = chk1 ^ chk2;
  unsigned long key = l * 0x66756b65 + nCheckSum;
  //printf("key: 0x%08x\n", key);

  unsigned long k = 0;
  unsigned short n = (l + 3) >> 2;
  for (unsigned short i = 0; i < n; i += 4)
  {
    // Skip check code field
    //*((unsigned long *)(buf + i)) ^= (key + icq_check_data[i & 0xff]);
    k = (key + icq_check_data[i & 0xff]);
    // Make it work on any endianness
    buf[i]   ^= (k      ) & 0xFF;
    buf[i+1] ^= (k >>  8) & 0xFF;
    buf[i+2] ^= (k >> 16) & 0xFF;
    buf[i+3] ^= (k >> 24) & 0xFF;
  }
  // Restore the version number
  buf[0] = 0x04;
  buf[1] = 0x00;
  // Stick in the checksum
  buf[16] = nCheckSum & 0xFF;
  buf[17] = (nCheckSum >> 8) & 0xFF;
  buf[18] = (nCheckSum >> 16) & 0xFF;
  buf[19] = (nCheckSum >> 24) & 0xFF;

#elif ICQ_VERSION == 5
  unsigned long l = buffer->getDataSize();
  unsigned long nCheckSum = 0;
  unsigned char *buf = (unsigned char *)buffer->getDataStart();

  // Make sure packet is long enough
  if (l == 24)
  {
    buffer->PackUnsignedLong(rand());
    l = buffer->getDataSize();
  }

  if (gLog.LoggingPackets())
  {
    char *b;
    gLog.Packet("%sUnencrypted Packet (%ld bytes):\n%s\n", L_PACKETxSTR, l,
                buffer->print(b));
    delete [] b;
  }

  // Calculate checkcode
  unsigned long chk1 = ( buf[8] << 24) |
                       ( buf[4] << 16 ) |
                       ( buf[2] << 8 ) |
                       ( buf[6] );
  DEBUG_ENCRYPTION(("chk1: %08lX\n", chk1));
  unsigned short r1 = 24 + rand() % (l - 24);
  unsigned short r2 = rand() & 0xFF;
  unsigned long chk2 = ( r1 << 24 ) |
                       ( buf[r1] << 16 ) |
                       ( r2 << 8 ) |
                       ( icq_check_data[r2] );
  DEBUG_ENCRYPTION(("chk2: %08lX\n", chk2));
  chk2 ^= 0x00FF00FF;
  nCheckSum = chk1 ^ chk2;
  unsigned long key = l * 0x68656C6C + nCheckSum;

  unsigned long k = 0;
  for (unsigned short i = 10; i < l; i += 4)
  {
    k = (key + icq_check_data[i & 0xff]);
    // Make it work on any endianness
    buf[i]   ^= (k      ) & 0xFF;
    buf[i+1] ^= (k >>  8) & 0xFF;
    buf[i+2] ^= (k >> 16) & 0xFF;
    buf[i+3] ^= (k >> 24) & 0xFF;
  }

  // Add in the checkcode
  DEBUG_ENCRYPTION(("checksum: %08lX\n", nCheckSum));
  unsigned long a1 = nCheckSum & 0x0000001F;
  unsigned long a2 = nCheckSum & 0x03E003E0;
  unsigned long a3 = nCheckSum & 0xF8000400;
  unsigned long a4 = nCheckSum & 0x0000F800;
  unsigned long a5 = nCheckSum & 0x041F0000;
  a1 <<= 0x0C;
  a2 <<= 0x01;
  a3 >>= 0x0A;
  a4 <<= 0x10;
  a5 >>= 0x0F;
  unsigned long nNewCheckSum = a1 + a2 + a3 + a4 + a5;

  // Stick in the checksum
  buf[20] = nNewCheckSum & 0xFF;
  buf[21] = (nNewCheckSum >> 8) & 0xFF;
  buf[22] = (nNewCheckSum >> 16) & 0xFF;
  buf[23] = (nNewCheckSum >> 24) & 0xFF;

#endif
}


//======Server TCP============================================================
bool CSrvPacketTcp::s_bRegistered = false;
unsigned short CSrvPacketTcp::s_nSequence = 0;
unsigned short CSrvPacketTcp::s_nSubSequence = 0;

CSrvPacketTcp::CSrvPacketTcp(unsigned char nChannel)
{
  m_nChannel = nChannel;
  m_nSequence = s_nSequence++;
  m_nSubSequence = s_nSubSequence++;

  buffer = NULL;
  m_nSize = 0;
  m_szSequenceOffset = NULL;
}

CSrvPacketTcp::~CSrvPacketTcp()
{
  if (buffer) delete buffer;
}

CBuffer *CSrvPacketTcp::Finalize(INetSocket *)
{
  //  m_szSequenceOffset
  if (!getBuffer()) return new CBuffer;
  return new CBuffer(*getBuffer());
}

void CSrvPacketTcp::InitBuffer()
{
  buffer = new CBuffer(m_nSize+6);
  buffer->PackChar(0x2a);
  buffer->PackChar(m_nChannel);
  m_szSequenceOffset = buffer->getDataPosWrite();
  buffer->PackUnsignedShortBE(m_nSequence);
  buffer->PackUnsignedShortBE(m_nSize);
}

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


void Encrypt_Client(CBuffer *pkt, unsigned long version)
{
  unsigned long B1, M1, check;
  unsigned int i;
  unsigned char X1, X2, X3;
  unsigned char *buf = (unsigned char*)pkt->getDataStart();
  unsigned char bak[6];
  unsigned long offset;
  unsigned long size = pkt->getDataSize();

  if (version < 4)
    return;  // no encryption necessary.

  switch(version)
  {
    case 4:
    case 5:
      offset = 6;
      break;
    case 6:
    case 7:
    default:
      offset = 0;
  }

  if (gLog.LoggingPackets())
  {
    char *b;
    gLog.Packet("%sUnencrypted (ICQ) TCP Packet (%ld bytes):\n%s\n", L_PACKETxSTR, size,
       pkt->print(b));
    delete [] b;
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


bool Decrypt_Client(CBuffer *pkt, unsigned long version)
{
  unsigned long hex, key, B1, M1, check;
  unsigned int i;
  unsigned char X1, X2, X3;
  unsigned char *buf = (unsigned char*)pkt->getDataStart();
  unsigned char bak[6];
  unsigned long size = pkt->getDataSize();
  unsigned long offset;

  if(version < 4)
    return true;  // no decryption necessary.

  switch(version) {
  case 4:
  case 5:
    offset = 6;
    break;
  case 6:
  case 7:
  default:
    offset = 0;
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

  if (gLog.LoggingPackets())
  {
    char *b;
    gLog.Packet("%sDecrypted (ICQ) TCP Packet (%ld bytes):\n%s\n", L_PACKETxSTR, size,
       pkt->print(b));
    delete [] b;
  }

  return true;
}



unsigned long CPacket::s_nLocalIp = 0;
unsigned long CPacket::s_nRealIp = 0;
unsigned short CPacket::s_nLocalPort = 0;
char CPacket::s_nMode = MODE_DIRECT;

//----SetIps-----------------------------------------------------------------
void CPacket::SetIps(INetSocket *s)
{
  if (s_nLocalIp == 0 || s_nLocalIp == s_nRealIp)
    s_nLocalIp = NetworkIpToPacketIp(s->LocalIp());
  s_nRealIp = NetworkIpToPacketIp(s->LocalIp());
}



//=====UDP======================================================================
unsigned short CPacketUdp::s_nSequence = 0;
unsigned short CPacketUdp::s_nSubSequence = 0;
unsigned long  CPacketUdp::s_nSessionId = 0;
bool CPacketUdp::s_bRegistered = false;

CBuffer *CPacketUdp::Finalize(INetSocket *)
{
  if (!getBuffer()) return new CBuffer;
  return new CBuffer(*getBuffer());
}

CPacketUdp::CPacketUdp(unsigned short _nCommand)
{
#if ICQ_VERSION == 2
  m_nVersion = 0x02;
#elif ICQ_VERSION == 4
  m_nVersion = 0x04;
  m_nRandom = rand();
  m_nZero = 0;
  m_nCheckSum = 0;
#elif ICQ_VERSION == 5
  m_nVersion = 0x05;
  m_nZero = 0;
  m_nSessionId = s_nSessionId;
  m_nCheckSum = 0;
#endif

  m_nCommand = _nCommand;
  // Fill in the sequences
  switch(m_nCommand)
  {
    case ICQ_CMDxSND_ACK:
    case ICQ_CMDxSND_LOGON:
    case ICQ_CMDxSND_REGISTERxUSER:
    {
      m_nSequence = 0;
      m_nSubSequence = 0;
      break;
    }
    case ICQ_CMDxSND_PING:
    {
      m_nSequence = s_nSequence++;
      m_nSubSequence = 0;
      break;
    }
    default:
    {
      m_nSequence = s_nSequence++;
      m_nSubSequence = s_nSubSequence++;
      break;
    }
  }

  buffer = NULL;
#if ICQ_VERSION == 2
  m_nSize = 10;
#elif ICQ_VERSION == 4
  m_nSize = 20;
#elif ICQ_VERSION == 5
  m_nSize = 24;
#endif
}


CPacketUdp::~CPacketUdp()
{
  if (buffer != NULL) delete buffer;
}


void CPacketUdp::InitBuffer()
{
  buffer = new CBuffer(m_nSize + 8);
#if ICQ_VERSION == 2
  buffer->PackUnsignedShort(m_nVersion);
  buffer->PackUnsignedShort(m_nCommand);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
#elif ICQ_VERSION == 4
  buffer->add(m_nVersion);
  buffer->add(m_nRandom);
  buffer->add(m_nZero);
  buffer->add(m_nCommand);
  buffer->add(m_nSequence);
  buffer->add(m_nSubSequence);
  buffer->add(gUserManager.OwnerUin());
  buffer->add(m_nCheckSum);
#elif ICQ_VERSION == 5
  buffer->PackUnsignedShort(m_nVersion);
  buffer->PackUnsignedLong(m_nZero);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedLong(m_nSessionId);
  buffer->PackUnsignedShort(m_nCommand);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedShort(m_nSubSequence);
  buffer->PackUnsignedLong(m_nCheckSum);
#endif
}

//-----Register----------------------------------------------------------------
#if ICQ_VERSION == 2 || ICQ_VERSION > 6
CPU_Register::CPU_Register(const char *_szPasswd)
{
  m_nVersion = ICQ_VERSION;
  m_nCommand = ICQ_CMDxSND_REGISTERxUSER;
  m_nSequence = 0x001;
  m_nPasswdLen = strlen(_szPasswd) + 1;
  m_szPasswd = strdup(_szPasswd);

  m_nSize = strlen (m_szPasswd) + 1 + 18;
  buffer = new CBuffer(m_nSize);

  buffer->PackUnsignedShort(m_nVersion);
  buffer->PackUnsignedShort(m_nCommand);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedShort(0x02);
  buffer->PackString(m_szPasswd);
  buffer->PackUnsignedShort(0x72);
  buffer->PackUnsignedShort(0x00);
}

CPU_Register::~CPU_Register()
{
  free (m_szPasswd);
  if (buffer != NULL) delete buffer;
}

#elif ICQ_VERSION == 4 || ICQ_VERSION == 5

CPU_Register::CPU_Register(const char *szPasswd)
  : CPacketUdp(ICQ_CMDxSND_REGISTERxUSER)
{
  m_nSize += strlen (szPasswd) + 1 + 20;

#if ICQ_VERSION == 5
  m_nSessionId = s_nSessionId = rand() & 0x3FFFFFFF;
#endif
  s_nSequence = rand() & 0x7FFF;
  m_nSequence = s_nSequence++;
  m_nSubSequence = s_nSubSequence = 1;
  s_bRegistered = true;

  InitBuffer();

  buffer->PackString(szPasswd);
  buffer->PackUnsignedLong(0x000000A0);
  buffer->PackUnsignedLong(0x00002461);
  buffer->PackUnsignedLong(0x00A00000);
  buffer->PackUnsignedLong(0x00000000);
}

#elif ICQ_VERSION >= 7

CPU_Register::CPU_Register(const char *szPasswd)
	: CPU_CommonFamily()
{
}

#endif

//-----Logon--------------------------------------------------------------------
CPU_Logon::CPU_Logon(const char *szPassword, const char *szUin, unsigned short _nLogonStatus)
  : CSrvPacketTcp(ICQ_CHNxNEW)
{
  char szEncPass[16];
  unsigned int j;

  if (!s_bRegistered) {
    s_nSequence = rand() & 0x7fff;
    s_bRegistered = true;
    m_nSequence = s_nSequence++;
  }
  m_nLogonStatus = _nLogonStatus;
  m_nTcpVersion = ICQ_VERSION_TCP;

  unsigned int uinlen = strlen(szUin);
  unsigned int pwlen = strlen(szPassword);

  m_nSize = uinlen + pwlen + 117;
  InitBuffer();

  // Encrypt our password here
  unsigned char xor_table[] = { 0xf3, 0x26, 0x81, 0xc4, 0x39, 0x86, 0xdb, 0x92,
			    0x71, 0xa3, 0xb9, 0xe6, 0x53, 0x7a, 0x95, 0x7c};
  for (j = 0; j < pwlen; j++)
    szEncPass[j] = (szPassword[j] ^ xor_table[j]);
  szEncPass[j] = 0;

  buffer->PackUnsignedLongBE(0x00000001);
  buffer->PackTLV(0x0001, uinlen, szUin);
  buffer->PackTLV(0x0002, pwlen, szEncPass);
  buffer->PackTLV(0x0003,  0x0033, "ICQ Inc. - Product of ICQ (TM).2001b.5.17.1.3642.85");

  // Static versioning
  buffer->PackUnsignedLongBE(0x00160002);
  buffer->PackUnsignedShortBE(0x010a);
  // Client version major (4 == ICQ2000, 5 == ICQ2001)
  buffer->PackUnsignedLongBE(0x00170002);
  buffer->PackUnsignedShortBE(0x0005);
  // Client version minor
  buffer->PackUnsignedLongBE(0x00180002);
  buffer->PackUnsignedShortBE(0x0011);
  buffer->PackUnsignedLongBE(0x00190002);
  buffer->PackUnsignedShortBE(0x0001);
  // Client version build
  buffer->PackUnsignedLongBE(0x001a0002);
  buffer->PackUnsignedShortBE(0x0e3a);
  buffer->PackUnsignedLongBE(0x00140004);
  buffer->PackUnsignedLongBE(0x00000055);

  // locale info, just use english, usa for now, i don't know what else they use
  buffer->PackTLV(0x000e, 0x0002, "us");
  buffer->PackTLV(0x000f, 0x0002, "en");
}

//-----SendCookie------------------------------------------------------------
CPU_SendCookie::CPU_SendCookie(const char *szCookie, int nLen)
  : CSrvPacketTcp(ICQ_CHNxNEW)
{
  m_szCookie = new char[nLen];
  memcpy(m_szCookie, szCookie, nLen);
  m_nSize = nLen + 8;
  s_nSequence = (rand() & 0x7fff);
  m_nSequence = s_nSequence++;
  InitBuffer();

  buffer->PackUnsignedLongBE(0x00000001);
  buffer->PackTLV(0x0006, nLen, m_szCookie);
}

CPU_SendCookie::~CPU_SendCookie()
{
  if (m_szCookie) delete [] m_szCookie;
}

//-----ImICQ-----------------------------------------------------------------
CPU_ImICQ::CPU_ImICQ()
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_IMxICQ)
{
  m_nSize += 36;

  InitBuffer();

  // setting communication parameters (key / value pair) ?
  // seems to say which channels should be enabled
  buffer->PackUnsignedLongBE(0x00010003);
  //buffer->PackUnsignedLongBE(0x00130002);
  buffer->PackUnsignedLongBE(0x00020001);
  buffer->PackUnsignedLongBE(0x00030001);
  buffer->PackUnsignedLongBE(0x00150001);
  buffer->PackUnsignedLongBE(0x00040001);
  buffer->PackUnsignedLongBE(0x00060001);
  buffer->PackUnsignedLongBE(0x00090001);
  buffer->PackUnsignedLongBE(0x000a0001);
  buffer->PackUnsignedLongBE(0x000b0001);
}

//-----ImICQ-----------------------------------------------------------------
CPU_ICQMode::CPU_ICQMode()
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SETxICQxMODE)
{
  m_nSize += 16;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x00000000);
  buffer->PackUnsignedLongBE(0x00031f40);
  buffer->PackUnsignedLongBE(0x03e703e7);
  buffer->PackUnsignedLongBE(0x00000000);
}

//-----RateAck-----------------------------------------------------------------
CPU_RateAck::CPU_RateAck()
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSND_RATE_ACK)
{
  m_nSize += 10;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x00010002);
  buffer->PackUnsignedLongBE(0x00030004);
  buffer->PackUnsignedShortBE(0x0005);
}

//-----UINSettings-----------------------------------------------------------
CPU_CapabilitySettings::CPU_CapabilitySettings()
  : CPU_CommonFamily(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_SETxUSERxINFO)
{
#if 0
  m_nSize += 68;
  InitBuffer();

  char data[0x40] = { 0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1,
		      0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,
		      0x09, 0x46, 0x13, 0x44, 0x4c, 0x7f, 0x11, 0xd1,
		      0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,
                      0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34,
                      0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x92,
                      0x2e, 0x7a, 0x64, 0x75, 0xfa, 0xdf, 0x4d, 0xc8,
                      0x88, 0x6f, 0xea, 0x35, 0x95, 0xfd, 0xb6, 0xdf
                    };

  buffer->PackTLV(0x05, 0x40, data);
#else

  m_nSize += 36;
  InitBuffer();

  char data[0x20] = { 0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1,
		      0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,
		      0x09, 0x49, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1,
		      0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
                    };

  buffer->PackTLV(0x05, 0x20, data);
#endif
}

//-----SetStatus----------------------------------------------------------------
CPU_SetStatus::CPU_SetStatus(unsigned long _nNewStatus)
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS)
{
  m_nNewStatus = _nNewStatus;

  m_nSize += 8;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x00060004);     // TLV
  buffer->PackUnsignedLongBE(m_nNewStatus);  // ICQ status
}

CPU_SetLogonStatus::CPU_SetLogonStatus(unsigned long _nNewStatus)
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS)
{
  m_nNewStatus = _nNewStatus;

  m_nSize += 55;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x00060004);     // TLV
  buffer->PackUnsignedLongBE(m_nNewStatus);  // ICQ status
  buffer->PackUnsignedLongBE(0x00080002);    // TLV
  buffer->PackUnsignedShortBE(0);            // error code ?
  buffer->PackUnsignedLongBE(0x000c0025);    // TLV
  buffer->PackUnsignedLong(s_nLocalIp);    // direct connection info
  buffer->PackUnsignedLongBE(s_nLocalPort);
  buffer->PackChar(MODE_DIRECT);
  buffer->PackUnsignedShortBE(ICQ_VERSION_TCP);
  buffer->PackUnsignedLongBE(0x00000000);    // local direction conn cookie
  buffer->PackUnsignedLongBE(0x00000050);
  buffer->PackUnsignedLongBE(0x00000003);

  // build date of the core DLL of Mirabilis ICQ
  // we use that to tell other users that we're Licq
  // ICQ99b:  0x385BFAAC;
#ifdef USE_OPENSSL
   buffer->PackUnsignedLongBE(LICQ_WITHSSL | INT_VERSION);
#else
   buffer->PackUnsignedLongBE(LICQ_WITHOUTSSL | INT_VERSION);
#endif
   // some kind of timestamp ?
  buffer->PackUnsignedLongBE(0x00000000);
  buffer->PackUnsignedLongBE(0x3b7248ed); // timestamp of some kind?
  buffer->PackUnsignedShortBE(0x0000);
}


//-----GenericUinList-----------------------------------------------------------
CPU_GenericUinList::CPU_GenericUinList(UinList &uins, unsigned short family, unsigned short Subtype)
  : CPU_CommonFamily(family, Subtype)
{
  char len[2];
  len[1] = '\0';
  char contacts[uins.size()*13+1];
  contacts[0] = '\0';

  for (UinList::iterator iter = uins.begin(); iter != uins.end(); iter++) {
    char uin[13];
    uin[12] = '\0';
    len[0] = snprintf(uin, 12, "%lu", *iter);
    strcat(contacts, len);
    strcat(contacts, uin);
  }

  m_nSize += strlen(contacts);
  InitBuffer();

  buffer->Pack(contacts, strlen(contacts));
}

CPU_GenericUinList::CPU_GenericUinList(unsigned long _nUin, unsigned short family, unsigned short Subtype)
  : CPU_CommonFamily(family, Subtype)
{
  char uin[13];
  uin[12] = '\0';
  int n = snprintf(uin, 12, "%lu", _nUin);

  m_nSize += n+1;
  InitBuffer();

  buffer->PackChar(n);
  buffer->Pack(uin, n);
}

CPU_GenericFamily::CPU_GenericFamily(unsigned short Family, unsigned short SubType)
  : CPU_CommonFamily(Family, SubType)
{
  m_nSize += 0;
  InitBuffer();
}

CPU_CommonFamily::CPU_CommonFamily(unsigned short Family, unsigned short SubType)
  : CSrvPacketTcp(ICQ_CHNxDATA)
{
  m_nSize += 10;

  m_nFamily = Family;
  m_nSubType = SubType;
}

void CPU_CommonFamily::InitBuffer()
{
  CSrvPacketTcp::InitBuffer();

  buffer->PackUnsignedShortBE(m_nFamily);
  buffer->PackUnsignedShortBE(m_nSubType);
  buffer->PackUnsignedLongBE(0x00000000); // flags
  buffer->PackUnsignedShortBE(m_nSubSequence);
}

CPU_ClientReady::CPU_ClientReady()
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSUB_READYxCLIENT)
{
#if 0
  m_nSize += 80;

  InitBuffer();
  buffer->PackUnsignedLongBE(0x00010003);
  buffer->PackUnsignedLongBE(0x0110047b);
  buffer->PackUnsignedLongBE(0x00130002);
  buffer->PackUnsignedLongBE(0x0110047b);
  buffer->PackUnsignedLongBE(0x00020001);

  buffer->PackUnsignedLongBE(0x0101047b);
  buffer->PackUnsignedLongBE(0x00030001);
  buffer->PackUnsignedLongBE(0x0110047b);
  buffer->PackUnsignedLongBE(0x00150001);
  buffer->PackUnsignedLongBE(0x0110047b);

  buffer->PackUnsignedLongBE(0x00040001);
  buffer->PackUnsignedLongBE(0x0110047b);
  buffer->PackUnsignedLongBE(0x00060001);
  buffer->PackUnsignedLongBE(0x0110047b);
  buffer->PackUnsignedLongBE(0x00090001);

  buffer->PackUnsignedLongBE(0x0110047b);
  buffer->PackUnsignedLongBE(0x000a0001);
  buffer->PackUnsignedLongBE(0x0110047b);
  buffer->PackUnsignedLongBE(0x000b0001);
  buffer->PackUnsignedLongBE(0x0110047b);
#else
  m_nSize += 64;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x00010003);
  buffer->PackUnsignedLongBE(0x0110028a);
  buffer->PackUnsignedLongBE(0x00020001);
  buffer->PackUnsignedLongBE(0x0110028a);
  buffer->PackUnsignedLongBE(0x00030001);
  buffer->PackUnsignedLongBE(0x0110028a);
  buffer->PackUnsignedLongBE(0x00150001);
  buffer->PackUnsignedLongBE(0x0110028a);
  buffer->PackUnsignedLongBE(0x00040001);
  buffer->PackUnsignedLongBE(0x0110028a);
  buffer->PackUnsignedLongBE(0x00060001);
  buffer->PackUnsignedLongBE(0x0110028a);
  buffer->PackUnsignedLongBE(0x00090001);
  buffer->PackUnsignedLongBE(0x0110028a);
  buffer->PackUnsignedLongBE(0x000a0001);
  buffer->PackUnsignedLongBE(0x0110028a);

#endif
}

CPU_AckNameInfo::CPU_AckNameInfo()
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSND_NAMExINFOxACK)
{
  m_nSize += 4;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x00000000);
}

//-----ThroughServer-------------------------------------------------------
CPU_ThroughServer::CPU_ThroughServer(unsigned long nDestinationUin, unsigned char msgType, char *szMessage)
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER)
{
  m_nDestinationUin = nDestinationUin;
  int msgLen = strlen(szMessage);
  char uin[13];
  uin[12] = '\0';
  int n = snprintf(uin, 12, "%lu", m_nDestinationUin);
	unsigned short nFormat = 0;
	int nTypeLen = 0, nTLVType = 0;
	CBuffer tlvData;
	
  switch (msgType)
  {
  case ICQ_CMDxSUB_MSG:
  	nTypeLen = 13+msgLen;
  	nFormat = 1;
  	break;
  	
  case ICQ_CMDxSUB_ADDEDxTOxLIST:
  	break;
  	
  case ICQ_CMDxSUB_URL:
  case ICQ_CMDxSUB_AUTHxGRANTED:
  case ICQ_CMDxSUB_AUTHxREFUSED:
  case ICQ_CMDxSUB_AUTHxREQUEST:
  	nTypeLen = 9+msgLen;
  	nFormat = 4;
  	break;

  default:
  	n = nTypeLen = msgLen = 0;
  	gLog.Warn("%sCommand not implemented yet (%04X).\n", L_BLANKxSTR, msgType);
  }

  m_nSize += 11 + nTypeLen + n + 8; // 11 all bytes pre-tlv
  																	//  8 fom tlv type, tlv len, and last 4 bytes
  InitBuffer();

 	buffer->PackUnsignedLongBE(0); // upper 4 bytes of message id
	buffer->PackUnsignedLongBE(0); // lower 4 bytes of message id
  buffer->PackUnsignedShortBE(nFormat); // message format
  buffer->PackChar(n);
  buffer->Pack(uin, n);

  tlvData.Create(nTypeLen);

  switch (nFormat)
  {
  case 1:
  	nTLVType = 0x02;
  	
  	tlvData.PackUnsignedLongBE(0x05010001);
	  tlvData.PackUnsignedShortBE(0x0101);
  	tlvData.PackChar(0x01);
	  tlvData.PackUnsignedShortBE(msgLen + 4);
  	tlvData.PackUnsignedLongBE(0);
	  tlvData.Pack(szMessage, msgLen);
	
  	break;
  	
  case 2:
  	nTLVType = 0x05;
  	
  	break;
  	
  case 4:
  	nTLVType = 0x05;
  	
		tlvData.PackUnsignedLong(gUserManager.OwnerUin());
		tlvData.PackChar(msgType);
		tlvData.PackChar(0); // message flags
		tlvData.PackLNTS(szMessage);
		
  	break;
  }

  buffer->PackTLV(nTLVType, nTypeLen, &tlvData);
  buffer->PackUnsignedLongBE(0x00060000); // tlv type: 6, tlv len: 0
}

CPU_AckThroughServer::CPU_AckThroughServer(unsigned long Uin, unsigned long timestamp1, unsigned long timestamp2,
                         unsigned short Cookie, unsigned char msgType, unsigned char msgFlags,
                         unsigned short len)
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, 0x0b)
{
  char szUin[13];
  int nUinLen;

  snprintf(szUin, 13, "%lu", Uin);
  nUinLen = strlen(szUin);

  m_nSize += 75 + nUinLen + len;

  InitBuffer();

  buffer->PackUnsignedLongBE(timestamp1);
  buffer->PackUnsignedLongBE(timestamp2);
  buffer->PackUnsignedShortBE(2);
  buffer->PackChar(nUinLen);
  buffer->Pack(szUin, nUinLen);
  buffer->PackUnsignedShortBE(0x03);
  buffer->PackUnsignedShort(0x1b);
  buffer->PackUnsignedShort(ICQ_VERSION_TCP);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedShortBE(0);
  buffer->PackUnsignedLong(3);
  buffer->PackChar(0);
  buffer->PackUnsignedShort(Cookie);
  buffer->PackUnsignedShort(0x0e);
  buffer->PackUnsignedShort(Cookie);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackChar(msgType);
  buffer->PackChar(msgFlags);
  buffer->PackUnsignedLongBE(0);

  buffer->PackUnsignedShort(1);
  buffer->PackChar(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0xffffff00);
}


//-----Ack-------------------------------------------------------------------
#if ICQ_VERSION == 2
CPU_Ack::CPU_Ack(unsigned short _nSequence) : CPacketUdp(ICQ_CMDxSND_ACK)
{
  m_nSequence = _nSequence;
  InitBuffer();
}
#elif ICQ_VERSION == 4 || ICQ_VERSION == 5
CPU_Ack::CPU_Ack(unsigned short _nSequence, unsigned short _nSubSequence)
  : CPacketUdp(ICQ_CMDxSND_ACK)
{
  m_nSequence = _nSequence;
  m_nSubSequence = _nSubSequence;
  InitBuffer();
}
#endif

//-----AddUser------------------------------------------------------------------
CPU_AddUser::CPU_AddUser(unsigned long _nAddedUin)
  : CPacketUdp(ICQ_CMDxSND_USERxADD)
{
  m_nAddedUin = _nAddedUin;

  m_nSize += 4;
  InitBuffer();

  buffer->PackUnsignedLong(m_nAddedUin);
}


//-----Logoff-------------------------------------------------------------------
CPU_Logoff::CPU_Logoff()
  : CSrvPacketTcp(ICQ_CHNxCLOSE)
{
  InitBuffer();
}

//-----SearchByInfo--------------------------------------------------------------
CPU_SearchByInfo::CPU_SearchByInfo(const char *szAlias, const char *szFirstName,
                                 const char *szLastName, const char *szEmail)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxSEARCH)
{
  int nLenEmail = strlen(szEmail)+1;
  m_nMetaCommand = 0x2905;
  m_nSize += 18 + nLenEmail;
  InitBuffer();

  gTranslator.ClientToServer((char *)szEmail);

  // Really TLV of type 0x0001
  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackUnsignedShortBE(14 + nLenEmail);
  buffer->PackUnsignedShort(12 + nLenEmail);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xd007);
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShortBE(0x2905);
  buffer->PackUnsignedShort(nLenEmail);
  buffer->Pack(szEmail, nLenEmail);
}

//-----SearchByUin--------------------------------------------------------------
CPU_SearchByUin::CPU_SearchByUin(unsigned long nUin)
  : CPacketUdp(ICQ_CMDxSND_SEARCHxUIN)
{

  m_nSize += 6;
  InitBuffer();

#if ICQ_VERSION == 2
  buffer->PackUnsignedShort(m_nSubSequence);
#endif
  buffer->PackUnsignedLong(nUin);
}


//-----SearchWhitePages---------------------------------------------------------
CPU_SearchWhitePages::CPU_SearchWhitePages(const char *szFirstName,
    const char *szLastName, const char *szAlias, const char *szEmail,
    unsigned short nMinAge, unsigned short nMaxAge, char nGender,
    char nLanguage, const char *szCity, const char *szState, unsigned short nCountryCode,
    const char *szCoName, const char *szCoDept, const char *szCoPos,
    bool bOnlineOnly)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxSEARCH)
{
	unsigned short nLenFName, nLenLName, nLenAlias, nLenEmail, nLenCity, nLenState,
								 nLenCoName, nLenCoDept, nLenCoPos, nDataLen;
								
	nLenFName  = strlen(szFirstName) + 1;
	nLenLName  = strlen(szLastName) + 1;
	nLenAlias  = strlen(szAlias) + 1;
	nLenEmail  = strlen(szEmail) + 1;
	nLenCity   = strlen(szCity) + 1;
	nLenState  = strlen(szState) + 1;
	nLenCoName = strlen(szCoName) + 1;
	nLenCoDept = strlen(szCoDept) + 1;
	nLenCoPos  = strlen(szCoPos) + 1;						
  nDataLen   = nLenFName + nLenLName + nLenAlias +
               nLenEmail + nLenCity + nLenState +
	             nLenCoName + nLenCoDept + nLenCoPos;
	m_nSize += nDataLen + 64;
  m_nMetaCommand = 0x3305;
  InitBuffer();

  // Fix invalid ages. Ensure that the plugin doesn't make an invalid request
  if (nMinAge != 0 || nMaxAge != 0)
  {
    if (nMinAge <= 18) nMinAge = 18;
    else if (nMinAge <= 23) nMinAge = 23;
    else if (nMinAge <= 30) nMinAge = 30;
    else if (nMinAge <= 40) nMinAge = 40;
    else if (nMinAge <= 50) nMinAge = 50;
    else nMinAge = 60;

    switch (nMinAge)
    {
    case 18: nMaxAge = 22;  break;
    case 23: nMaxAge = 29;  break;
    case 30: nMaxAge = 39;  break;
    case 40: nMaxAge = 49;  break;
    case 50: nMaxAge = 59;  break;
    case 60: nMaxAge = 120; break;
    }
  }

  gTranslator.ClientToServer((char *) szFirstName);
  gTranslator.ClientToServer((char *) szLastName);
  gTranslator.ClientToServer((char *) szAlias);
  gTranslator.ClientToServer((char *) szEmail);
  gTranslator.ClientToServer((char *) szCity);
  gTranslator.ClientToServer((char *) szState);
  gTranslator.ClientToServer((char *) szCoName);
  gTranslator.ClientToServer((char *) szCoDept);
  gTranslator.ClientToServer((char *) szCoPos);

  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackUnsignedShortBE(nDataLen+64 - 4);
  buffer->PackUnsignedShort(nDataLen+64 - 6);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xd007);
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShortBE(0x3305);
  buffer->PackLNTS(szFirstName);
	buffer->PackLNTS(szLastName);
	buffer->PackLNTS(szAlias);
	buffer->PackLNTS(szEmail);
	buffer->PackUnsignedShort(nMinAge);
	buffer->PackUnsignedShort(nMaxAge);
	buffer->PackChar(nGender);
	buffer->PackChar(nLanguage);
	buffer->PackLNTS(szCity);
	buffer->PackLNTS(szState);
	buffer->PackUnsignedShort(nCountryCode);
	buffer->PackLNTS(szCoName);
	buffer->PackLNTS(szCoDept);
	buffer->PackLNTS(szCoPos);

  // Ok, this stuff is Company Field, and all those interest crap
  // This will just leave them blank.. I don't see them as necessary
  buffer->PackChar(0x00);
  buffer->PackUnsignedShort(0x0000);

  for (unsigned short i = 0; i < 3; i++)
  {
    buffer->PackUnsignedShort(0x0001);
    buffer->PackChar(0x00);
    buffer->PackUnsignedShort(0x0000);
  }

  buffer->PackUnsignedShort(0x0001);
  buffer->PackChar(0x00);

  if (bOnlineOnly)
    buffer->PackChar(0x01);
  else
    buffer->PackChar(0x00);
}

//-----UpdatePersonalInfo-------------------------------------------------------
CPU_UpdatePersonalBasicInfo::CPU_UpdatePersonalBasicInfo(const char *szAlias,
                                                        const char *szFirstName,
                                                        const char *szLastName,
                                                        const char *szEmail,
                                                        bool bAuthorization)
  : CPacketUdp(ICQ_CMDxSND_UPDATExBASIC)
{

  m_nAuthorization = bAuthorization ? 0 : 1;

  m_nSize += 15 + strlen(szAlias) + strlen(szFirstName) +
             strlen(szLastName) + strlen(szEmail);
  InitBuffer();

#if ICQ_VERSION == 2
  buffer->PackUnsignedShort(m_nSubSequence);
#endif

  gTranslator.ClientToServer((char *) szAlias);
  gTranslator.ClientToServer((char *) szFirstName);
  gTranslator.ClientToServer((char *) szLastName);
  gTranslator.ClientToServer((char *) szEmail);

  m_szAlias = buffer->PackString(szAlias);
  m_szFirstName = buffer->PackString(szFirstName);
  m_szLastName = buffer->PackString(szLastName);
  m_szEmail = buffer->PackString(szEmail);
  buffer->PackChar(m_nAuthorization);

}


//-----UpdatePersonalExtInfo-------------------------------------------------------
CPU_UpdatePersonalExtInfo::CPU_UpdatePersonalExtInfo(const char *szCity,
                                                    unsigned short nCountry,
                                                    const char *szState,
                                                    unsigned short nAge,
                                                    char cSex,
                                                    const char *szPhone,
                                                    const char *szHomepage,
                                                    const char *szAbout,
                                                    unsigned long nZipcode)
  : CPacketUdp(ICQ_CMDxSND_UPDATExDETAIL)
{

  m_nCountry = nCountry;
  m_cTimezone = ICQUser::SystemTimezone();
  m_nAge = nAge;
  m_cSex = cSex;
  m_nZipcode = nZipcode;

  m_nSize += strlen(szCity) + strlen(szState) +
             strlen(szPhone) + strlen(szHomepage) + strlen(szAbout) + 27;
  InitBuffer();

#if ICQ_VERSION == 2
  buffer->PackUnsignedShort(m_nSubSequence);
#endif

  gTranslator.ClientToServer((char *) szCity);
  gTranslator.ClientToServer((char *) szHomepage);
  gTranslator.ClientToServer((char *) szPhone);
  gTranslator.ClientToServer((char *) szState);
//  gTranslator.ClientToServer((char *) szAbout);

  m_szCity = buffer->PackString(szCity);
  buffer->PackUnsignedShort(m_nCountry);
  buffer->PackChar(m_cTimezone);
  m_szState = buffer->PackString(szState, 5);
  buffer->PackUnsignedShort(m_nAge);
  buffer->PackChar(m_cSex);
  m_szPhone = buffer->PackString(szPhone);
  m_szHomepage = buffer->PackString(szHomepage);
  m_szAbout = buffer->PackString(szAbout);
  buffer->PackUnsignedLong(m_nZipcode);

}



//-----Ping---------------------------------------------------------------------
CPU_Ping::CPU_Ping()
  : CSrvPacketTcp(ICQ_CHNxPING)
{
  InitBuffer();
}


CPU_ReverseTCPRequest::CPU_ReverseTCPRequest(unsigned long nDestinationUin,
                                             unsigned long nIp,  unsigned short nPort,
                                             unsigned short nPort2)
  : CPacketUdp(ICQ_CMDxRCV_REVERSExTCP)
{
  m_nDestinationUin = nDestinationUin;
  m_nSize += 21;

  InitBuffer();

  buffer->PackUnsignedLong( nDestinationUin );
  buffer->PackUnsignedLong( nIp ); // IP
  buffer->PackUnsignedShort( nPort ); // port
  buffer->PackUnsignedShort( nPort ); // junk
  buffer->PackChar( 6 );
  buffer->PackUnsignedShort( nPort2 );  //nport2
  buffer->PackUnsignedShort( nPort2 ); // junk  nport2
  // junk twice
  buffer->PackUnsignedLong( nPort );
}
#if 0
      unsigned long nUin, nIp;
      unsigned short nPort, nJunk, nPort2, nVersion;
      char cJunk;
      packet >> nUin >> nIp
             >> nPort >> nJunk
             >> cJunk
             >> nPort2 >> nJunk // port which they tried to connect to
             >> nJunk >> nJunk // nPort again
             >> nVersion;
      nIp = PacketIpToNetworkIp(nIp);
#endif



//-----Authorize----------------------------------------------------------------
CPU_Authorize::CPU_Authorize(unsigned long nAuthorizeUin) : CPacketUdp(ICQ_CMDxRCV_REVERSExTCP)
{
}

//------SetPassword---------------------------------------------------------
CPU_SetPassword::CPU_SetPassword(const char *szPassword)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, 0x0002)
{
  m_nMetaCommand = ICQ_CMDxMETA_PASSWORDxSET;

  unsigned short nDataLen = strlen(szPassword) + 19;
  m_nSize += nDataLen;
  InitBuffer();

  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackUnsignedShortBE(nDataLen - 4);
  buffer->PackUnsignedShort(nDataLen - 6);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xd007);
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(ICQ_CMDxMETA_PASSWORDxSET);

  // LNTS, but we want the password in this class
  //buffer->PackUnsignedShort(nDataLen - 19);
  m_szPassword = buffer->PackLNTS(szPassword);//buffer->PackString(szPassword);
  //buffer->PackChar(0x00);
}

//-----RequestSysMsg------------------------------------------------------------
CPU_RequestSysMsg::CPU_RequestSysMsg()
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxOFF_SYSMSG)
{
  m_nSize += 14;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x0001000a); // TLV
  buffer->PackUnsignedShort(8);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedLongBE(0x3c000200);
}

//-----SysMsgDoneAck------------------------------------------------------------
CPU_SysMsgDoneAck::CPU_SysMsgDoneAck(unsigned short nId)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxOFF_SYSMSGxACK)
{
  m_nSize += 14;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x0001000a); // TLV
  buffer->PackUnsignedShort(0x0008); // len again
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0x3e00);
  buffer->PackUnsignedShortBE(nId);
}

//-----CPU_SetRandomChatGroup------------------------------------------------
CPU_SetRandomChatGroup::CPU_SetRandomChatGroup(unsigned long nGroup)
 : CPacketUdp(ICQ_CMDxSND_SETxRANDOMxCHAT)
{
  m_nGroup = nGroup;

  m_nSize += 4;
  InitBuffer();

  buffer->PackUnsignedLong(m_nGroup);
}


//-----CPU_RandomChatSearch--------------------------------------------------
CPU_RandomChatSearch::CPU_RandomChatSearch(unsigned long nGroup)
 : CPacketUdp(ICQ_CMDxSND_RANDOMxSEARCH)
{
  m_nGroup = nGroup;

  m_nSize += 4;
  InitBuffer();

  buffer->PackUnsignedLong(m_nGroup);
}


//-----Meta_SetGeneralInfo---------------------------------------------------
CPU_Meta_SetGeneralInfo::CPU_Meta_SetGeneralInfo(const char *szAlias,
                          const char *szFirstName, const char *szLastName,
                          const char *szEmailPrimary, const char *szEmailSecondary,
                          const char *szEmailOld,
                          const char *szCity, const char *szState,
                          const char *szPhoneNumber, const char *szFaxNumber,
                          const char *szAddress, const char *szCellularNumber,
                          const char *szZipCode,
                          unsigned short nCountryCode,
                          bool bHideEmail)
  : CPacketUdp(ICQ_CMDxSND_META)
{
  m_nMetaCommand = ICQ_CMDxMETA_GENERALxINFOxSET;

  m_nCountryCode = nCountryCode;
  m_nTimezone = ICQUser::SystemTimezone();
  m_nHideEmail = bHideEmail ? 1 : 0;

  m_nSize += strlen_safe(szAlias) + strlen_safe(szFirstName) + strlen_safe(szLastName) +
             strlen_safe(szEmailPrimary) * 2 + strlen_safe(szEmailSecondary) + strlen_safe(szCity) +
             strlen_safe(szState) + strlen_safe(szPhoneNumber) + strlen_safe(szEmailOld) +
             strlen_safe(szFaxNumber) + strlen_safe(szAddress) + strlen_safe(szZipCode) +
             strlen_safe(szCellularNumber) + 36 + 12;
  InitBuffer();

  buffer->PackUnsignedShort(m_nMetaCommand);

  gTranslator.ClientToServer((char *) szAlias);
  gTranslator.ClientToServer((char *) szFirstName);
  gTranslator.ClientToServer((char *) szLastName);
  gTranslator.ClientToServer((char *) szEmailPrimary);
  gTranslator.ClientToServer((char *) szEmailSecondary);
  gTranslator.ClientToServer((char *) szEmailOld);
  gTranslator.ClientToServer((char *) szCity);
  gTranslator.ClientToServer((char *) szState);
  gTranslator.ClientToServer((char *) szPhoneNumber);
  gTranslator.ClientToServer((char *) szFaxNumber);
  gTranslator.ClientToServer((char *) szAddress);
  gTranslator.ClientToServer((char *) szCellularNumber);

  m_szAlias = buffer->PackString(szAlias);
  m_szFirstName = buffer->PackString(szFirstName);
  m_szLastName = buffer->PackString(szLastName);
  m_szEmailPrimary = buffer->PackString(szEmailPrimary);
  m_szEmailSecondary = buffer->PackString(szEmailSecondary);
  m_szEmailOld = buffer->PackString(szEmailOld);
  m_szCity = buffer->PackString(szCity);
  m_szState = buffer->PackString(szState);
  m_szPhoneNumber = buffer->PackString(szPhoneNumber);
  m_szFaxNumber = buffer->PackString(szFaxNumber);
  m_szAddress = buffer->PackString(szAddress);
  m_szCellularNumber = buffer->PackString(szCellularNumber);
  m_szZipCode = buffer->PackString(szZipCode);
  buffer->PackUnsignedShort(m_nCountryCode);
  buffer->PackChar(m_nTimezone);
  buffer->PackChar(m_nHideEmail);

  // Check for possible problems
  char *sz = m_szAlias;
  while (*sz != '\0' && strncasecmp(sz, "icq", 3) != 0) sz++;
  if (*sz != '\0')
  {
    gLog.Warn("%sAlias may not contain \"icq\".\n", L_WARNxSTR);
    *sz = '-';
  }
}


//-----Meta_SetMoreInfo------------------------------------------------------
CPU_Meta_SetMoreInfo::CPU_Meta_SetMoreInfo( unsigned short nAge,
                       char nGender,
                       const char *szHomepage,
                       unsigned short nBirthYear,
                       char nBirthMonth,
                       char nBirthDay,
                       char nLanguage1,
                       char nLanguage2,
                       char nLanguage3)
  : CPacketUdp(ICQ_CMDxSND_META)
{
  m_nMetaCommand = ICQ_CMDxMETA_MORExINFOxSET;
  m_nAge = nAge;
  m_nGender = nGender;
  m_nBirthYear = nBirthYear;
  m_nBirthMonth = nBirthMonth;
  m_nBirthDay = nBirthDay;
  m_nLanguage1 = nLanguage1;
  m_nLanguage2 = nLanguage2;
  m_nLanguage3 = nLanguage3;

  m_nSize += strlen_safe(szHomepage) + 14;
  InitBuffer();

  gTranslator.ClientToServer((char *) szHomepage);

  buffer->PackUnsignedShort(m_nMetaCommand);
  buffer->PackUnsignedShort(m_nAge);
  buffer->PackChar(nGender);
  m_szHomepage = buffer->PackString(szHomepage);
  buffer->PackUnsignedShort(m_nBirthYear);
  buffer->PackChar(m_nBirthMonth);
  buffer->PackChar(m_nBirthDay);
  buffer->PackChar(m_nLanguage1);
  buffer->PackChar(m_nLanguage2);
  buffer->PackChar(m_nLanguage3);

}


//-----Meta_SetWorkInfo------------------------------------------------------
CPU_Meta_SetWorkInfo::CPU_Meta_SetWorkInfo(
    const char *szCity,
    const char *szState,
    const char *szPhoneNumber,
    const char *szFaxNumber,
    const char *szAddress,
    const char *szName,
    const char *szDepartment,
    const char *szPosition,
    const char *szHomepage) : CPacketUdp(ICQ_CMDxSND_META)
{
  m_nMetaCommand = ICQ_CMDxMETA_WORKxINFOxSET;

  char szStatebuf[6];

  szStatebuf[5] = '\0';
  snprintf(szStatebuf, 6, szState);

  m_nSize += strlen_safe(szCity) + strlen_safe(szStatebuf) + strlen_safe(szPhoneNumber) +
             strlen_safe(szFaxNumber) + strlen_safe(szAddress) + strlen_safe(szName) +
             strlen_safe(szDepartment) + strlen_safe(szPosition) +
             strlen_safe(szHomepage) + 8 + 26;
  InitBuffer();

  gTranslator.ClientToServer((char *) szCity);
  gTranslator.ClientToServer((char *) szState);
  gTranslator.ClientToServer((char *) szPhoneNumber);
  gTranslator.ClientToServer((char *) szFaxNumber);
  gTranslator.ClientToServer((char *) szAddress);
  gTranslator.ClientToServer((char *) szName);
  gTranslator.ClientToServer((char *) szDepartment);
  gTranslator.ClientToServer((char *) szPosition);
  gTranslator.ClientToServer((char *) szHomepage);

  buffer->PackUnsignedShort(m_nMetaCommand);
  m_szCity = buffer->PackString(szCity);
  m_szState = buffer->PackString(szStatebuf);
  m_szPhoneNumber = buffer->PackString(szPhoneNumber);
  m_szFaxNumber = buffer->PackString(szFaxNumber);
  m_szAddress = buffer->PackString(szAddress);
  buffer->PackUnsignedLong(0x0100);
  buffer->PackUnsignedShort(0xFFFF);
  m_szName = buffer->PackString(szName);
  m_szDepartment = buffer->PackString(szDepartment);
  m_szPosition = buffer->PackString(szPosition);
  buffer->PackUnsignedShort(0x04);
  m_szHomepage = buffer->PackString(szHomepage);

}


//-----Meta_SetAbout---------------------------------------------------------
CPU_Meta_SetAbout::CPU_Meta_SetAbout(const char *szAbout)
  : CPacketUdp(ICQ_CMDxSND_META)
{
  m_nMetaCommand = ICQ_CMDxMETA_ABOUTxSET;

  m_nSize += strlen_safe(szAbout) + 5;
  InitBuffer();

  m_szAbout = szAbout == NULL ? strdup("") : strdup(szAbout);
  buffer->PackUnsignedShort(m_nMetaCommand);
  char *sz = gTranslator.NToRN(szAbout);
  gTranslator.ClientToServer(sz);
  if (strlen(sz) > MAX_MESSAGE_SIZE)
    sz[MAX_MESSAGE_SIZE] = '\0';
  buffer->PackString(sz);
  if (sz != NULL) free(sz);
}

CPU_Meta_SetAbout::~CPU_Meta_SetAbout()
{
  free(m_szAbout);
}

//-----Meta_SetSecurityInfo--------------------------------------------------
CPU_Meta_SetSecurityInfo::CPU_Meta_SetSecurityInfo(
    bool bAuthorization,
    bool bHideIp,
    bool bWebAware)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA_INFO)
{
  m_nMetaCommand = 0x2404;
  m_nAuthorization = bAuthorization ? 0 : 1;
  m_nHideIp =  bHideIp ? 1 : 0;
  m_nWebAware = bWebAware ? 1 : 0;


  int packetSize = 2+2+2+4+2+2+2+4;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); // TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); // bytes remaining
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xd007); // type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShortBE(0x2404); // subtype
  buffer->PackChar(m_nAuthorization);
  buffer->PackChar(m_nWebAware);
  buffer->PackChar(m_nHideIp);
  buffer->PackChar(0);
}


//-----Meta_RequestInfo------------------------------------------------------
CPU_Meta_RequestAllInfo::CPU_Meta_RequestAllInfo(unsigned long nUin)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA_INFO)
{
  m_nMetaCommand = 0xb204;
  m_nUin = nUin;

  int packetSize = 2+2+2+4+2+2+2+4;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); // TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); // bytes remaining
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xd007); // type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShortBE(0xb204); // subtype
  buffer->PackUnsignedLong(m_nUin);
}


//-----Meta_RequestInfo------------------------------------------------------
CPU_Meta_RequestBasicInfo::CPU_Meta_RequestBasicInfo(unsigned long nUin)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA_INFO)
{
  m_nMetaCommand = ICQ_CMDxMETA_REQUESTxBASICxINFO;
  m_nUin = nUin;

  m_nSize += 20;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x0001000e); // TLV

  buffer->PackUnsignedShort(0x000c); // Bytes remaining
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShort(ICQ_CMDxMETA_REQUESTxBASICxINFO);
  buffer->PackUnsignedShort(m_nSubSequence);
  buffer->PackUnsignedLong(nUin);
}

CPacketTcp_Handshake_v2::CPacketTcp_Handshake_v2(unsigned long nLocalPort)
{
  m_nLocalPort = nLocalPort;

  m_nSize = 26;
  buffer = new CBuffer(m_nSize);

  buffer->PackChar(ICQ_CMDxTCP_HANDSHAKE);
  buffer->PackUnsignedLong(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(m_nLocalPort);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(s_nMode);
  buffer->PackUnsignedLong(m_nLocalPort);
}


CPacketTcp_Handshake_v4::CPacketTcp_Handshake_v4(unsigned long nLocalPort)
{
  m_nLocalPort = nLocalPort;

  m_nSize = 26;
  buffer = new CBuffer(m_nSize);

  buffer->PackChar(ICQ_CMDxTCP_HANDSHAKE);
  buffer->PackUnsignedLong(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(0x00000000);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedLong(s_nLocalIp); // maybe should be 0
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(s_nMode);
  buffer->PackUnsignedLong(m_nLocalPort);
}


//=====PacketTcp_Handshake======================================================
CPacketTcp_Handshake_v6::CPacketTcp_Handshake_v6(unsigned long nDestinationUin,
   unsigned long nSessionId, unsigned short nLocalPort)
{
  m_nDestinationUin = nDestinationUin;
  m_nSessionId = nSessionId;
  if (m_nSessionId == 0) m_nSessionId = rand();

  m_nSize = 44;
  buffer = new CBuffer(m_nSize);

  buffer->PackChar(ICQ_CMDxTCP_HANDSHAKE);
  //buffer->PackUnsignedLong(0x00270006);
  buffer->PackUnsignedLong(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(m_nDestinationUin);
  buffer->PackUnsignedShort(0);
  buffer->PackUnsignedLong(nLocalPort);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(s_nMode);
  buffer->PackUnsignedLong(nLocalPort == 0 ? s_nLocalPort : nLocalPort);
  buffer->PackUnsignedLong(m_nSessionId);
  buffer->PackUnsignedLong(0x00000050); // constant
  buffer->PackUnsignedLong(0x00000003); // constant
}


CPacketTcp_Handshake_v6::CPacketTcp_Handshake_v6(CBuffer *inbuf)
{
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
   unsigned long nSessionId, unsigned short nLocalPort)
{
  m_nDestinationUin = nDestinationUin;
  m_nSessionId = nSessionId;
  if (m_nSessionId == 0) m_nSessionId = rand();

  m_nSize = 48;
  buffer = new CBuffer(m_nSize);

  buffer->PackChar(ICQ_CMDxTCP_HANDSHAKE);
  buffer->PackUnsignedLong(0x002b0007);
  //buffer->PackUnsignedLong(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(m_nDestinationUin);
  buffer->PackUnsignedShort(0);
  buffer->PackUnsignedLong(nLocalPort);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackChar(s_nMode);
  buffer->PackUnsignedLong(nLocalPort == 0 ? s_nLocalPort : nLocalPort);
  buffer->PackUnsignedLong(m_nSessionId);
  buffer->PackUnsignedLong(0x00000050); // constant
  buffer->PackUnsignedLong(0x00000003); // constant
  buffer->PackUnsignedLong(0x00000000); // ???

}


CPacketTcp_Handshake_v7::CPacketTcp_Handshake_v7(CBuffer *inbuf)
{
  m_nHandshake = inbuf->UnpackChar();
  m_nVersionMajor = inbuf->UnpackUnsignedShort();
  m_nVersionMinor = inbuf->UnpackUnsignedShort();
  m_nDestinationUin = inbuf->UnpackUnsignedLong();
  inbuf->UnpackUnsignedShort();
  inbuf->UnpackUnsignedLong();
  m_nSourceUin = inbuf->UnpackUnsignedLong();
  m_nRealIp = inbuf->UnpackUnsignedLong();
  m_nLocalIp = inbuf->UnpackUnsignedLong();
  m_nMode = inbuf->UnpackChar();
  inbuf->UnpackUnsignedLong(); // port of some kind...?
  m_nSessionId = inbuf->UnpackUnsignedLong();
  //inbuf->UnpackUnsignedLong(); // constant
  //inbuf->UnpackUnsignedLong(); // constant
}


CPacketTcp_Handshake_Ack::CPacketTcp_Handshake_Ack()
{
  m_nSize = 4;
  buffer = new CBuffer(4);
  buffer->PackUnsignedLong(1);
}


//=====PacketTcp================================================================
CBuffer *CPacketTcp::Finalize(INetSocket *s)
{
  // Set the local port in the tcp packet now
  if (s != NULL && LocalPortOffset() != NULL)
  {
    LocalPortOffset()[0] = s->LocalPort() & 0xFF;
    LocalPortOffset()[1] = (s->LocalPort() >> 8) & 0xFF;
  }

  Encrypt_Client(buffer, m_nVersion);
  return buffer;
}

CPacketTcp::CPacketTcp(unsigned long _nCommand, unsigned short _nSubCommand,
   const char *szMessage, bool _bAccept, unsigned short nLevel, ICQUser *user)
{
  // Setup the message type and status fields using our online status
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short s = o->Status();
  if (user->StatusToUser() != ICQ_STATUS_OFFLINE) s = user->StatusToUser();
  m_nLevel = nLevel;

  switch(_nCommand)
  {
    case ICQ_CMDxTCP_CANCEL:
    case ICQ_CMDxTCP_START:
    {
      m_nStatus = 0;
      m_nMsgType = nLevel;
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
      if (o->StatusInvisible())
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
            m_nStatus = (*user->CustomAutoResponse() && _nSubCommand == ICQ_CMDxTCP_READxDNDxMSG)
              ? ICQ_TCPxACK_DNDxCAR : ICQ_TCPxACK_DND;
            break;
          case ICQ_STATUS_OCCUPIED:
            m_nStatus = (*user->CustomAutoResponse() && _nSubCommand == ICQ_CMDxTCP_READxOCCUPIEDxMSG)
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
  gUserManager.DropOwner();

  m_nSourceUin = gUserManager.OwnerUin();
  m_nCommand = _nCommand;
  m_nSubCommand = _nSubCommand;
  m_szMessage = (szMessage == NULL ? strdup("") : strdup(szMessage));
  m_nLocalPort = user->LocalPort();

  // don't increment the sequence if this is an ack and cancel packet
  if (m_nCommand == ICQ_CMDxTCP_START) m_nSequence = user->Sequence(true);

  m_nVersion = user->ConnectionVersion();

  // v4,6 packets are smaller then v2 so we just set the size based on a v2 packet
  m_nSize = 18 + strlen(m_szMessage) + 25;
  buffer = NULL;
}

CPacketTcp::~CPacketTcp()
{
  free (m_szMessage);
  delete buffer;
}


void CPacketTcp::InitBuffer()
{
  switch (m_nVersion)
  {
    case 6:
    case 7:
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
  }
}

void CPacketTcp::PostBuffer()
{
  switch (m_nVersion)
  {
    case 6:
    case 7:
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
}


void CPacketTcp::InitBuffer_v2()
{
  buffer = new CBuffer(m_nSize + 4);

  buffer->PackUnsignedLong(m_nSourceUin);
  buffer->PackUnsignedShort(m_nVersion == 2 ? 2 : ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(m_nCommand);
  buffer->PackUnsignedLong(m_nSourceUin);
  buffer->PackUnsignedShort(m_nSubCommand);
  buffer->PackString(m_szMessage);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  m_szLocalPortOffset = buffer->getDataPosWrite();
  buffer->PackUnsignedLong(m_nLocalPort);
  buffer->PackChar(s_nMode);
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
    buffer->PackUnsignedShort(INT_VERSION);
  }
}


void CPacketTcp::InitBuffer_v4()
{
  buffer = new CBuffer(m_nSize + 8);

  buffer->PackUnsignedLong(m_nSourceUin);
  buffer->PackUnsignedShort(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(0x00000000);  // Checksum
  buffer->PackUnsignedLong(m_nCommand);
  buffer->PackUnsignedLong(m_nSourceUin);
  buffer->PackUnsignedShort(m_nSubCommand);
  buffer->PackString(m_szMessage);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  m_szLocalPortOffset = buffer->getDataPosWrite();
  buffer->PackUnsignedLong(m_nLocalPort);
  buffer->PackChar(s_nMode);
  buffer->PackUnsignedShort(m_nStatus);
  buffer->PackUnsignedShort(m_nMsgType);
}

void CPacketTcp::PostBuffer_v4()
{
  buffer->PackUnsignedLong(m_nSequence);
  buffer->PackChar('L');
  buffer->PackUnsignedShort(INT_VERSION);
}


void CPacketTcp::InitBuffer_v6()
{
  buffer = new CBuffer(m_nSize + 4);

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
  buffer->PackString(m_szMessage);

  m_szLocalPortOffset = NULL;
}

void CPacketTcp::PostBuffer_v6()
{
// don't break ICQ2000
//   buffer->PackChar('L');
//   buffer->PackUnsignedShort(INT_VERSION);
}


//-----Message------------------------------------------------------------------
CPT_Message::CPT_Message(char *_sMessage, unsigned short nLevel, bool bMR,
 CICQColor *pColor, ICQUser *pUser)
  : CPacketTcp(ICQ_CMDxTCP_START,
       ICQ_CMDxSUB_MSG | (bMR ? ICQ_CMDxSUB_FxMULTIREC : 0),
       _sMessage, true, nLevel, pUser)
{
  InitBuffer();
  if (m_nVersion == 6)
  {
    if (pColor == NULL)
    {
      buffer->PackUnsignedLong(0x00000000);
      buffer->PackUnsignedLong(0x00FFFFFF);
    }
    else
    {
      buffer->PackUnsignedLong(pColor->Foreground());
      buffer->PackUnsignedLong(pColor->Background());
    }
  }
  PostBuffer();
}

//-----Url----------------------------------------------------------------------
CPT_Url::CPT_Url(char *szMessage, unsigned short nLevel, bool bMR,
 CICQColor *pColor, ICQUser *pUser)
  : CPacketTcp(ICQ_CMDxTCP_START,
       ICQ_CMDxSUB_URL | (bMR ? ICQ_CMDxSUB_FxMULTIREC : 0),
       szMessage, true, nLevel, pUser)
{
  InitBuffer();
  if (m_nVersion == 6)
  {
    if (pColor == NULL)
    {
      buffer->PackUnsignedLong(0x00000000);
      buffer->PackUnsignedLong(0x00FFFFFF);
    }
    else
    {
      buffer->PackUnsignedLong(pColor->Foreground());
      buffer->PackUnsignedLong(pColor->Background());
    }
  }
  PostBuffer();
}


//-----ContactList-----------------------------------------------------------
CPT_ContactList::CPT_ContactList(char *sz, unsigned short nLevel, bool bMR,
   CICQColor *pColor, ICQUser *pUser)
  : CPacketTcp(ICQ_CMDxTCP_START,
       ICQ_CMDxSUB_CONTACTxLIST | (bMR ? ICQ_CMDxSUB_FxMULTIREC : 0),
       sz, true, nLevel, pUser)
{
  InitBuffer();
  if (m_nVersion == 6)
  {
    if (pColor == NULL)
    {
      buffer->PackUnsignedLong(0x00000000);
      buffer->PackUnsignedLong(0x00FFFFFF);
    }
    else
    {
      buffer->PackUnsignedLong(pColor->Foreground());
      buffer->PackUnsignedLong(pColor->Background());
    }
  }
  PostBuffer();
}


//-----ReadAwayMessage----------------------------------------------------------
CPT_ReadAwayMessage::CPT_ReadAwayMessage(ICQUser *_cUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxTCP_READxAWAYxMSG, "", true, false, _cUser)
{
  // Properly set the subcommand to get the correct away message
  switch(_cUser->Status())
  {
    case ICQ_STATUS_AWAY: m_nSubCommand = ICQ_CMDxTCP_READxAWAYxMSG; break;
    case ICQ_STATUS_NA: m_nSubCommand = ICQ_CMDxTCP_READxNAxMSG; break;
    case ICQ_STATUS_DND: m_nSubCommand = ICQ_CMDxTCP_READxDNDxMSG; break;
    case ICQ_STATUS_OCCUPIED: m_nSubCommand = ICQ_CMDxTCP_READxOCCUPIEDxMSG; break;
    case ICQ_STATUS_FREEFORCHAT: m_nSubCommand = ICQ_CMDxTCP_READxFFCxMSG; break;
    default: m_nSubCommand = ICQ_CMDxTCP_READxAWAYxMSG; break;
  }

  InitBuffer();
  if (m_nVersion == 6)
  {
    buffer->PackUnsignedLong(0xFFFFFFFF);
    buffer->PackUnsignedLong(0xFFFFFFFF);
  }
  PostBuffer();
}

//-----ChatRequest--------------------------------------------------------------
CPT_ChatRequest::CPT_ChatRequest(char *_sMessage, const char *szChatUsers,
   unsigned short nPort, unsigned short nLevel, ICQUser *pUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_CHAT, _sMessage, true,
    nLevel, pUser)
{
  m_nSize += 2 + strlen_safe(szChatUsers) + 1 + 8;
  InitBuffer();

  buffer->PackString(szChatUsers);
  buffer->PackUnsignedLong(ReversePort(nPort));
  buffer->PackUnsignedLong(nPort);

  PostBuffer();
}


//-----FileTransfer--------------------------------------------------------------
CPT_FileTransfer::CPT_FileTransfer(const char *_szFilename,
   const char *_szDescription, unsigned short nLevel, ICQUser *_cUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_FILE, _szDescription,
               true, nLevel, _cUser)
{
  m_bValid = true;

  // Check file exists and get size
  struct stat buf;
  if (_szFilename == NULL || stat(_szFilename, &buf) < 0)
  {
     m_bValid = false;
     return;
  }
  m_nFileSize = buf.st_size;

  // Remove path from filename (if it exists)
  char *pcEndOfPath = strrchr(_szFilename, '/');
  if (pcEndOfPath != NULL)
     m_szFilename = strdup(pcEndOfPath + 1);
  else
     m_szFilename = strdup(_szFilename);

  m_nSize += 15 + strlen(m_szFilename);
  InitBuffer();

  buffer->PackUnsignedLong(0);
  buffer->PackString(m_szFilename);
  buffer->PackUnsignedLong(m_nFileSize);
  buffer->PackUnsignedLong(0);

  PostBuffer();
}


//-----Key------------------------------------------------------------------
CPT_OpenSecureChannel::CPT_OpenSecureChannel(ICQUser *_cUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_SECURExOPEN,
       "", true, ICQ_TCPxMSG_NORMAL, _cUser)
{
  InitBuffer();
  PostBuffer();
}


CPT_CloseSecureChannel::CPT_CloseSecureChannel(ICQUser *_cUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_SECURExCLOSE,
       "", true, ICQ_TCPxMSG_NORMAL, _cUser)
{
  InitBuffer();
  PostBuffer();
}


//+++++Ack++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
char *PipeInput(char *m_szMessage)
{
  // Check for pipes
  char *pipe = NULL;
  if (m_szMessage[0] == '|')
    pipe = m_szMessage;
  else
  {
    pipe = strstr(m_szMessage, "\n|");
    if (pipe != NULL) pipe++;
  }

  while (pipe != NULL)
  {
    char *sz = pipe;
    char szCmd[81];
    char szCmdOutput[4097];
    unsigned short i;
    CUtilityInternalWindow win;

    // Move over the '|'
    sz++;
    // Find the end of the command
    for (i = 0; *sz != '\r' && *sz != '\0'; i++)
    {
      if (i < 80) szCmd[i] = *sz;
      sz++;
    }
    szCmd[i] = '\0';
    // Ensure sz points to after the command and \r\n
    if (*sz == '\r') sz += 2;

    //gLog.Info("-> \"%s\"\n", szCmd);

    if (!win.POpen(szCmd))
    {
      gLog.Warn("%sCould not execute \"%s\" for auto-response.\n", L_WARNxSTR, szCmd);
      szCmdOutput[0] = '\0';
    }
    else
    {
      int c;
      i = 0;
      while (((c = fgetc(win.StdOut())) != EOF) && (i < 4096))
      {
        szCmdOutput[i++] = c;
      }
      szCmdOutput[i] = '\0';

      if ((i = win.PClose()) != 0)
      {
        gLog.Warn("%s%s returned abnormally: exit code %d\n", L_WARNxSTR, szCmd, i);
        // do anything to szCmdOutput ???
      }
    }

    // Create the new response
    char *szNewMsg = (char *)malloc(
     (pipe - m_szMessage) + (strlen(szCmdOutput) << 1) + strlen(sz) );
    *pipe = '\0';
    strcpy(szNewMsg, m_szMessage);
    // Make pipe point to the same place in the new message
    pipe = &szNewMsg[pipe - m_szMessage];
    // Copy the command output, converting N to RN and making pipe point to
    // after the output
    for (char *p = szCmdOutput; *p != '\0'; p++)
    {
      if (*p == '\n') *pipe++ = '\r';
      *pipe++ = *p;
    }
    // Copy the post-command stuff
    strcpy(pipe, sz);
    free(m_szMessage);
    m_szMessage = szNewMsg;

    // Try again now
    if (pipe[0] != '|')
    {
      pipe = strstr(pipe, "\n|");
      if (pipe != NULL) pipe++;
    }
  }

  return m_szMessage;
}



CPT_Ack::CPT_Ack(unsigned short _nSubCommand, unsigned long _nSequence,
                bool _bAccept, bool l, ICQUser *pUser)
  : CPacketTcp(ICQ_CMDxTCP_ACK, _nSubCommand, "", _bAccept,
               l ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL, pUser)
{
  m_nSequence = _nSequence;
  free(m_szMessage);
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);

  // don't sent out AutoResponse if we're online
  // it could contain stuff the other site shouldn't be able to read
  // also some clients always pop up the auto response
  // window when they receive one, annoying for them..
  if(((pUser->StatusToUser() != ICQ_STATUS_OFFLINE &&
       pUser->StatusToUser() != ICQ_STATUS_ONLINE)  ?
      pUser->StatusToUser() : o->Status()) != ICQ_STATUS_ONLINE)
  {
    if (*pUser->CustomAutoResponse())
    {
      //m_szMessage = (char *)malloc(strlen(pUser->CustomAutoResponse()) + 512);
      //pUser->usprintf(m_szMessage, pUser->CustomAutoResponse(), USPRINTF_NTORN);
      char *cus = (char *)malloc(strlen(pUser->CustomAutoResponse()) + 512);
      char *def = (char *)malloc(strlen(o->AutoResponse()) + 512);
      pUser->usprintf(def, o->AutoResponse(), USPRINTF_NTORN | USPRINTF_PIPEISCMD);
      pUser->usprintf(cus, pUser->CustomAutoResponse(), USPRINTF_NTORN | USPRINTF_PIPEISCMD);
      m_szMessage = (char *)malloc(strlen(cus) + strlen(def) + 60);
      sprintf(m_szMessage, "%s\r\n--------------------\r\n%s", def, cus);
      free(cus);
      free(def);
    }
    else
    {
      m_szMessage = (char *)malloc(strlen(o->AutoResponse()) + 512);
      pUser->usprintf(m_szMessage, o->AutoResponse(), USPRINTF_NTORN | USPRINTF_PIPEISCMD);
    }

  }
  else
    m_szMessage = strdup("");

  gUserManager.DropOwner();

  // Check for pipes, should possibly go after the ClientToServer call
  m_szMessage = PipeInput(m_szMessage);

  gTranslator.ClientToServer(m_szMessage);

  m_nSize += strlen(m_szMessage);
}


//-----AckGeneral---------------------------------------------------------------
CPT_AckGeneral::CPT_AckGeneral(unsigned short nCmd, unsigned long nSequence,
   bool bAccept, bool nLevel, ICQUser *pUser)
  : CPT_Ack(nCmd, nSequence, bAccept, nLevel, pUser)
{
  InitBuffer();
  if (m_nVersion == 6)
  {
    buffer->PackUnsignedLong(0x00000000);
    buffer->PackUnsignedLong(0x00000000);
  }
  PostBuffer();
}


//-----AckKey---------------------------------------------------------------
CPT_AckOpenSecureChannel::CPT_AckOpenSecureChannel(unsigned long nSequence,
   bool ok, ICQUser *pUser)
  : CPT_Ack(ICQ_CMDxSUB_SECURExOPEN, nSequence, true, true, pUser)
{
  m_nSize -= strlen(m_szMessage);
  free(m_szMessage);
  m_szMessage = strdup(ok ? "1" : "");
  m_nSize += strlen(m_szMessage);

  InitBuffer();
  PostBuffer();
}


CPT_AckOldSecureChannel::CPT_AckOldSecureChannel(unsigned long nSequence,
   ICQUser *pUser)
  : CPT_Ack(ICQ_CMDxSUB_SECURExOPEN, nSequence, true, true, pUser)
{
  m_nSize -= strlen(m_szMessage);
  free(m_szMessage);
  m_szMessage = strdup("");
  m_nSize += strlen(m_szMessage);

  InitBuffer();
  if (m_nVersion == 6)
  {
    buffer->PackUnsignedLong(0x00000000);
    buffer->PackUnsignedLong(0x00000000);
  }
  PostBuffer();
}


CPT_AckCloseSecureChannel::CPT_AckCloseSecureChannel(unsigned long nSequence,
   ICQUser *pUser)
  : CPT_Ack(ICQ_CMDxSUB_SECURExCLOSE, nSequence, true, true, pUser)
{
  m_nSize -= strlen(m_szMessage);
  free(m_szMessage);
  m_szMessage = strdup("");
  m_nSize += strlen(m_szMessage);

  InitBuffer();
  PostBuffer();
}



#if 0
//-----AckMessage---------------------------------------------------------------
CPT_AckMessage::CPT_AckMessage(unsigned long _nSequence, bool _bAccept,
                               bool nLevel, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_MSG, _nSequence, _bAccept, nLevel, _cUser)
{
  InitBuffer();
  PostBuffer();
}



//-----AckReadAwayMessage-------------------------------------------------------
CPT_AckReadAwayMessage::CPT_AckReadAwayMessage(unsigned short _nSubCommand,
                                               unsigned long _nSequence,
                                               bool _bAccept, ICQUser *_cUser)
  : CPT_Ack(_nSubCommand, _nSequence, _bAccept, false, _cUser)
{
  InitBuffer();
  PostBuffer();
}


//-----AckUrl-------------------------------------------------------------------
CPT_AckUrl::CPT_AckUrl(unsigned long _nSequence, bool _bAccept, bool nLevel,
                      ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_URL, _nSequence, _bAccept, nLevel, _cUser)
{
  InitBuffer();
  PostBuffer();
}


//-----AckContactList--------------------------------------------------------
CPT_AckContactList::CPT_AckContactList(unsigned long _nSequence, bool _bAccept,
                                       bool nLevel, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_CONTACTxLIST, _nSequence, _bAccept, nLevel, _cUser)
{
  InitBuffer();
  PostBuffer();
}
#endif

//-----AckChatRefuse------------------------------------------------------------
CPT_AckChatRefuse::CPT_AckChatRefuse(const char *szReason,
   unsigned long _nSequence, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_CHAT, _nSequence, false, false, _cUser)
{
  free (m_szMessage);
  m_szMessage = szReason == NULL ? strdup("") : strdup(szReason);
  char temp_1[11] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  m_nSize += 11 + strlen(m_szMessage);
  InitBuffer();
  buffer->Pack(temp_1, 11);
  PostBuffer();
}


//-----AckChatAccept------------------------------------------------------------
CPT_AckChatAccept::CPT_AckChatAccept(unsigned short _nPort,
                                    unsigned long _nSequence, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_CHAT, _nSequence, true, true, _cUser)
{
  m_nPort = _nPort;
  m_nStatus = ICQ_TCPxACK_ONLINE;

  m_nSize += 11;
  InitBuffer();

  buffer->PackString("");
  buffer->PackUnsignedLong(ReversePort(m_nPort));
  buffer->PackUnsignedLong(m_nPort);

  PostBuffer();
}


//-----AckFileRefuse------------------------------------------------------------
CPT_AckFileRefuse::CPT_AckFileRefuse(const char *szReason,
                                    unsigned long _nSequence, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_FILE, _nSequence, false, false, _cUser)
{
  free(m_szMessage);
  m_szMessage = (szReason == NULL ? strdup("") : strdup(szReason));

  m_nSize += 15 + strlen(m_szMessage);
  InitBuffer();

  buffer->PackUnsignedLong(0);
  buffer->PackString("");
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);

  PostBuffer();
}


//-----AckFileAccept------------------------------------------------------------
CPT_AckFileAccept::CPT_AckFileAccept(unsigned short _nPort,
                                    unsigned long _nSequence, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_FILE, _nSequence, true, true, _cUser)
{
  m_nFileSize = 0;
  m_nPort = _nPort;
  m_nStatus = ICQ_TCPxACK_ONLINE;

  m_nSize += 15;
  InitBuffer();

  buffer->PackUnsignedLong(ReversePort(m_nPort));
  buffer->PackString("");
  buffer->PackUnsignedLong(m_nFileSize);
  buffer->PackUnsignedLong(m_nPort);

  PostBuffer();
}


//+++++Cancel+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CPT_Cancel::CPT_Cancel(unsigned short _nSubCommand, unsigned long _nSequence,
                      ICQUser *_cUser)
  : CPacketTcp(ICQ_CMDxTCP_CANCEL, _nSubCommand, "", true, false, _cUser)
{
  m_nSequence = _nSequence;
}



//-----CancelChat---------------------------------------------------------------
CPT_CancelChat::CPT_CancelChat(unsigned long _nSequence, ICQUser *_cUser)
  : CPT_Cancel(ICQ_CMDxSUB_CHAT, _nSequence, _cUser)
{
  char temp_1[11] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  m_nSize += 11;
  InitBuffer();
  buffer->Pack(temp_1, 11);
  PostBuffer();
}


//-----CancelFile---------------------------------------------------------------
CPT_CancelFile::CPT_CancelFile(unsigned long _nSequence, ICQUser *_cUser)
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
