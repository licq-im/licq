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

unsigned short LengthField(const char *szField)
{
  // By SC 27434326
  // Auxiliary function for whitepage search
  //+ 7 byte for command (2),length field (4)
  // end string (1)
  
  int len = strlen(szField);
  return (len ? (len + 7 ) : 0);
}

#define DEBUG_ENCRYPTION(x)
//#define DEBUG_ENCRYPTION(x) fprintf(stderr, x)

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
  //fprintf(stderr, "key: 0x%08x\n", key);

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
pthread_mutex_t CSrvPacketTcp::s_xMutex = PTHREAD_MUTEX_INITIALIZER;

CSrvPacketTcp::CSrvPacketTcp(unsigned char nChannel)
{
  m_nChannel = nChannel;
  pthread_mutex_lock(&s_xMutex);
  m_nSequence = s_nSequence++;
  m_nSubSequence = s_nSubSequence++;
  pthread_mutex_unlock(&s_xMutex);
  m_nFamily = m_nSubType = m_nSubCommand = m_nExtraInfo = 0;

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
    case 7:
    case 8:
    case 6:
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
#if ICQ_VERSION == 2 || ICQ_VERSION == 6
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

CPU_RegisterFirst::CPU_RegisterFirst()
	: CSrvPacketTcp(ICQ_CHNxNEW)
{
  m_nSize = 4;

  s_nSequence = rand() & 0x7fff;
  s_bRegistered = true;
  m_nSequence = s_nSequence++;

  InitBuffer();

  buffer->PackUnsignedLongBE(1);
}

CPU_Register::CPU_Register(const char *szPasswd)
	: CPU_CommonFamily(ICQ_SNACxFAM_NEWUIN, ICQ_SNACxREGISTER_USER)
{
  int nPassLen = strlen(szPasswd);
  m_nSize += 55 + nPassLen;
  m_nSubSequence = 0;

  InitBuffer();

  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackUnsignedShortBE(nPassLen+51);
  buffer->PackUnsignedLongBE(0x00000000);
  buffer->PackUnsignedLongBE(0x28000300);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0x82270000);
  buffer->PackUnsignedLongBE(0x82270000);
  for (int x = 0; x < 4; x++) buffer->PackUnsignedLongBE(0);
  buffer->PackLNTS(szPasswd);
  buffer->PackUnsignedLongBE(0x82270000);
  buffer->PackUnsignedLongBE(0x00001902);
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
  m_nSize = nLen + 8;
  s_nSequence = (rand() & 0x7fff);
  m_nSequence = s_nSequence++;
  InitBuffer();

  buffer->PackUnsignedLongBE(0x00000001);
  buffer->PackTLV(0x0006, nLen, szCookie);
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
CPU_ThroughServer::CPU_ThroughServer(unsigned long nDestinationUin,
																		 unsigned char msgType, char *szMessage)
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER)
{
	m_nSubCommand = msgType;

  int msgLen = szMessage ? strlen(szMessage) : 0;
  char uin[13];
  uin[12] = '\0';
  int nUinLen = snprintf(uin, 12, "%lu", nDestinationUin);
  unsigned short nFormat = 0;
  int nTypeLen = 0, nTLVType = 0;
  CBuffer tlvData;

  switch (msgType)
  {
  case ICQ_CMDxSUB_MSG:
  	nTypeLen = 13+msgLen;
  	nFormat = 1;
  	break;

  case ICQ_CMDxSUB_URL:
  case ICQ_CMDxSUB_CONTACTxLIST:
  case ICQ_CMDxSUB_AUTHxGRANTED:
  case ICQ_CMDxSUB_AUTHxREFUSED:
  case ICQ_CMDxSUB_AUTHxREQUEST:
  case ICQ_CMDxSUB_ADDEDxTOxLIST:
  	nTypeLen = 9+msgLen;
  	nFormat = 4;
  	break;

  default:
  	nUinLen = nTypeLen = msgLen = 0;
  	gLog.Warn("%sCommand not implemented yet (%04X).\n", L_BLANKxSTR, msgType);
		return;
  }

  m_nSize += 11 + nTypeLen + nUinLen + 8; // 11 all bytes pre-tlv
	//  8 fom tlv type, tlv len, and last 4 bytes

	InitBuffer();

	buffer->PackUnsignedLongBE(0); // upper 4 bytes of message id
	buffer->PackUnsignedLongBE(0); // lower 4 bytes of message id
	buffer->PackUnsignedShortBE(nFormat); // message format
	buffer->PackChar(nUinLen);
	buffer->Pack(uin, nUinLen);

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


//-----AdvancedMessage---------------------------------------------------------
CPU_AdvancedMessage::CPU_AdvancedMessage(ICQUser *u, unsigned short _nMsgType,
																				 unsigned short _nMsgFlags, bool _bAck,
																				 unsigned short _nSequence,
																				 unsigned long nMsgID1,
																				 unsigned long nMsgID2)
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER)
{
	char szUin[13];
	int nUinLen = snprintf(szUin, 12, "%ld", u->Uin());

	m_nSize += 123 + nUinLen;

	m_bAck = _bAck;
	m_nMsgFlags = _nMsgFlags;
	m_nSubCommand = _nMsgType;
	m_pUser = u;
	m_nSequence = _nSequence;
	m_nMsgID[0] = nMsgID1;
	m_nMsgID[1] = nMsgID2;

  // nobody inheirited us
  if (m_nSubCommand == ICQ_CMDxTCP_READxAWAYxMSG)
  {
    m_nSize -= 14; // no direct connection info
    InitBuffer();
  }
}

void CPU_AdvancedMessage::InitBuffer()
{
	CPU_CommonFamily::InitBuffer();

	ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
	unsigned short nStatus = o->Status();
  if (m_pUser->StatusToUser() != ICQ_STATUS_OFFLINE)
    nStatus = m_pUser->StatusToUser();

	char szUin[13];
	int nUinLen = snprintf(szUin, 12, "%ld", m_pUser->Uin());

	unsigned short nSequence;
	unsigned long nID1, nID2;
  unsigned short nDirectInfo = 14; // size of direct info

	if (m_bAck)
	{
		nSequence = m_nSequence;
		nID1 = m_nMsgID[0];
		nID2 = m_nMsgID[1];
	}
	else
	{
		nSequence = m_pUser->Sequence(true);
		nID1 = 0;
		nID2 = m_nSubSequence;
    if (m_nSubCommand == ICQ_CMDxTCP_READxAWAYxMSG)
    {
      nDirectInfo = 0; // no direct info needed

      // Get the correct message
      switch(m_pUser->Status())
      {
        case ICQ_STATUS_AWAY: m_nSubCommand = ICQ_CMDxTCP_READxAWAYxMSG; break;
        case ICQ_STATUS_NA: m_nSubCommand = ICQ_CMDxTCP_READxNAxMSG; break;
        case ICQ_STATUS_DND: m_nSubCommand = ICQ_CMDxTCP_READxDNDxMSG; break;
        case ICQ_STATUS_OCCUPIED: m_nSubCommand = ICQ_CMDxTCP_READxOCCUPIEDxMSG; break;
        case ICQ_STATUS_FREEFORCHAT: m_nSubCommand = ICQ_CMDxTCP_READxFFCxMSG; break;
        default: m_nSubCommand = ICQ_CMDxTCP_READxAWAYxMSG; break;
      }
    }
	}

	buffer->PackUnsignedLongBE(nID1); // upper 4 bytes of message id
	buffer->PackUnsignedLongBE(nID2); // lower 4 bytes of message id
	buffer->PackUnsignedShortBE(0x02); // message format
	buffer->PackChar(nUinLen);
	buffer->Pack(szUin, nUinLen);

	buffer->PackUnsignedShortBE(0x0005);	// tlv - message info
	buffer->PackUnsignedShortBE(m_nSize - 25 - nUinLen);
	buffer->PackUnsignedShortBE((m_bAck ? 0 : 0));
	buffer->PackUnsignedLongBE(nID1);	// upper 4 bytes of message id again
	buffer->PackUnsignedLongBE(nID2); // lower 4 bytes of message id again
	buffer->PackUnsignedLongBE(0x09461349); // from icq2002a
	buffer->PackUnsignedLongBE(0x4C7F11D1); // from icq2002a
	buffer->PackUnsignedLongBE(0x82224445); // from icq2002a
	buffer->PackUnsignedLongBE(0x53540000); // from icq2002a
	buffer->PackUnsignedLongBE(0x000A0002); // tlv - ack or not
	buffer->PackUnsignedShortBE((m_bAck ? 2 : 1));

  if (nDirectInfo)
  {
	  buffer->PackUnsignedLongBE(0x00050002); // tlv - listening port
	  buffer->PackUnsignedShort(o->Port());
    buffer->PackUnsignedLongBE(0x00030004); // tlv - internal ip
    buffer->PackUnsignedLong(o->IntIp());
  }

  buffer->PackUnsignedLongBE(0x000F0000); // tlv - empty
	buffer->PackUnsignedShortBE(0x2711); // tlv - more message info
	buffer->PackUnsignedShortBE(m_nSize - 29 - nUinLen - 36 - nDirectInfo);
	buffer->PackUnsignedShort(0x001B); // len
	buffer->PackUnsignedShort(m_pUser->ConnectionVersion());
	buffer->PackUnsignedLongBE(0);
	buffer->PackUnsignedLongBE(0);
	buffer->PackUnsignedLongBE(0);
	buffer->PackUnsignedLongBE(0);
	buffer->PackUnsignedShortBE(0);
	buffer->PackUnsignedLong(0x00000003); // len
	buffer->PackChar(0x04); // accept connections or firewalled
	buffer->PackUnsignedShort(nSequence); // sequence
	buffer->PackUnsignedShort(0x000E); // len
	buffer->PackUnsignedShort(nSequence); // sequence
	buffer->PackUnsignedLongBE(0);
	buffer->PackUnsignedLongBE(0);
	buffer->PackUnsignedLongBE(0);
	buffer->PackUnsignedShort(m_nSubCommand);
	buffer->PackUnsignedShort(nStatus);
	buffer->PackUnsignedShort(m_nMsgFlags ? m_nMsgFlags : 0);
	buffer->PackUnsignedShort(0x0001); // message len
	buffer->PackChar(0); // message

  //if (!nDirectInfo)
  //  buffer->PackUnsignedLongBE(0x00030000);

	gUserManager.DropOwner();
}

//-----ChatRequest-------------------------------------------------------------
CPU_ChatRequest::CPU_ChatRequest(char *_szMessage, const char *_szChatUsers,
                                 unsigned short nPort, ICQUser *_pUser, bool bICBM)
	: CPU_AdvancedMessage(_pUser, bICBM ? ICQ_CMDxSUB_ICBM : ICQ_CMDxSUB_CHAT,
												0, false, 0)
{
	int nUsersLen = _szChatUsers ? strlen(_szChatUsers) : 0;
	int nMessageLen = _szMessage ? strlen(_szMessage) : 0;

  if (bICBM)
		m_nSize += 62 + nUsersLen + nMessageLen + 21; // 21 = strlen(pluginname)
	else
		m_nSize += 18 + nUsersLen + nMessageLen;

	InitBuffer();

	if (bICBM)
	{
		buffer->PackUnsignedShort(0x3A); // len of following pluign info
		buffer->PackUnsignedLongBE(0xBFF720B2);
		buffer->PackUnsignedLongBE(0x378ED411);
		buffer->PackUnsignedLongBE(0xBD280004);
		buffer->PackUnsignedLongBE(0xAC96D905);
		buffer->PackUnsignedShort(0);
		buffer->PackUnsignedLong(0x15); // strlen of plugin name
		buffer->Pack("Send / Start ICQ Chat", 0x15);
		buffer->PackUnsignedLongBE(0x00000100);
		buffer->PackUnsignedLongBE(0x00010000);
		buffer->PackUnsignedLongBE(0);
		buffer->PackUnsignedShortBE(0);
		buffer->PackChar(0);
		buffer->PackUnsignedLong(nMessageLen + nUsersLen + 15);
		buffer->PackUnsignedLong(nMessageLen);
		if (_szMessage)
			buffer->Pack(_szMessage, nMessageLen);
	}
	else
		buffer->PackString(_szMessage);

	buffer->PackString(_szChatUsers);
	buffer->PackUnsignedShortBE(nPort);
	buffer->PackUnsignedShort(0);
	buffer->PackUnsignedShort(nPort);
	buffer->PackUnsignedShort(0);
	buffer->PackUnsignedLongBE(0x00030000); // ack request
}

//-----FileTransfer------------------------------------------------------------
CPU_FileTransfer::CPU_FileTransfer(ICQUser *u, const char *_szFile,
	const char *_szDesc, bool bICBM)
	: CPU_AdvancedMessage(u, bICBM ? ICQ_CMDxSUB_ICBM : ICQ_CMDxSUB_FILE, 0,
												false, 0),
		CPX_FileTransfer(_szFile)
{
	if (!m_bValid)  return;

	int nFileLen = strlen(m_szFilename);
	int nDescLen = strlen(_szDesc);
	m_szDesc = strdup(_szDesc); // XXX where should this be free'd?

  if (bICBM)
		m_nSize += 66 + nFileLen + nDescLen + 4; // 4 = strlen("File")
	else
		m_nSize += 22 + nFileLen + nDescLen ;

	InitBuffer();

	if (bICBM)
	{
		buffer->PackUnsignedShort(0x29);  // len of following plugin info
		buffer->PackUnsignedLongBE(0xF02D12D9);
		buffer->PackUnsignedLongBE(0x3091D311);
		buffer->PackUnsignedLongBE(0x8DD70010);
		buffer->PackUnsignedLongBE(0x4B06462E);
		buffer->PackUnsignedShortBE(0x0000);
		buffer->PackUnsignedLong(0x04); // strlen - is 4 bytes though
		buffer->Pack("File", 4);
		buffer->PackUnsignedLongBE(0x00000100);
		buffer->PackUnsignedLongBE(0x00010000);
		buffer->PackUnsignedLongBE(0);
		buffer->PackUnsignedShortBE(0);
		buffer->PackChar(0);
		buffer->PackUnsignedLong(nDescLen + nFileLen + 19); //remaining  - is 4 bytes
																											//dont count last 4 bytes
		buffer->PackUnsignedLong(nDescLen); // file desc - is 4 bytes
		buffer->Pack(_szDesc, nDescLen);
	}
	else
		buffer->PackString(_szDesc);

	buffer->PackUnsignedLongBE(0x2D384444); // ???
	buffer->PackString(m_szFilename);
	buffer->PackUnsignedLong(m_nFileSize);
	buffer->PackUnsignedLongBE(0);
	buffer->PackUnsignedLongBE(0x00030000); // ack request

}


//-----AckThroughServer--------------------------------------------------------
CPU_AckThroughServer::CPU_AckThroughServer(ICQUser *u,
                                           unsigned long nMsgID1,
                                           unsigned long nMsgID2,
                                           unsigned short nSequence,
                                           unsigned short nMsgType,
                                           bool bAccept, unsigned short nLevel)
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SERVERxREPLYxMSG)
{
  snprintf(m_szUin, 13, "%lu", u->Uin());
  m_nUinLen = strlen(m_szUin);

  m_nSize += 66 + m_nUinLen;

  m_nUin = u->Uin();
  m_nMsgID[0] = nMsgID1;
  m_nMsgID[1] = nMsgID2;
  m_nSequence = nSequence;
  m_nSubCommand = nMsgType;
  m_nSequence = nSequence;
  m_nLevel = nLevel;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short s = o->Status();
  if (u->StatusToUser() != ICQ_STATUS_OFFLINE)
    s = u->StatusToUser();

  if (!bAccept)
    m_nStatus = ICQ_TCPxACK_REFUSE;
  else
  {
    switch (s)
    {
      case ICQ_STATUS_AWAY: m_nStatus = ICQ_TCPxACK_AWAY; break;
      case ICQ_STATUS_NA: m_nStatus = ICQ_TCPxACK_NA; break;
      case ICQ_STATUS_DND:
        m_nStatus = (*u->CustomAutoResponse() && m_nSubCommand == ICQ_CMDxTCP_READxDNDxMSG)
          ? ICQ_TCPxACK_DNDxCAR : ICQ_TCPxACK_DND;
        break;
      case ICQ_STATUS_OCCUPIED:
        m_nStatus = (*u->CustomAutoResponse() && m_nSubCommand == ICQ_CMDxTCP_READxOCCUPIEDxMSG)
          ? ICQ_TCPxACK_OCCUPIEDxCAR : ICQ_TCPxACK_OCCUPIED;
        break;
      case ICQ_STATUS_ONLINE:
      case ICQ_STATUS_FREEFORCHAT:
      default: m_nStatus = ICQ_TCPxACK_ONLINE; break;
    }
  }

  // don't send out AutoResponse if we're online
  // it could contain stuff the other site shouldn't be able to read
  // also some clients always pop up the auto response
  // window when they receive one, annoying for them..
  if(((u->StatusToUser() != ICQ_STATUS_OFFLINE &&
       u->StatusToUser() != ICQ_STATUS_ONLINE)  ?
      u->StatusToUser() : o->Status()) != ICQ_STATUS_ONLINE)
  {
    if (*u->CustomAutoResponse())
    {
      //m_szMessage = (char *)malloc(strlen(u->CustomAutoResponse()) + 512);
      //pUser->usprintf(m_szMessage, u->CustomAutoResponse(), USPRINTF_NTORN);
      char *cus = (char *)malloc(strlen(u->CustomAutoResponse()) + 512);
      char *def = (char *)malloc(strlen(o->AutoResponse()) + 512);
      u->usprintf(def, o->AutoResponse(), USPRINTF_NTORN | USPRINTF_PIPEISCMD);
      u->usprintf(cus, u->CustomAutoResponse(), USPRINTF_NTORN | USPRINTF_PIPEISCMD);
      m_szMessage = (char *)malloc(strlen(cus) + strlen(def) + 60);
      sprintf(m_szMessage, "%s\r\n--------------------\r\n%s", def, cus);
      free(cus);
      free(def);
    }
    else
    {
      m_szMessage = (char *)malloc(strlen(o->AutoResponse()) + 512);
      u->usprintf(m_szMessage, o->AutoResponse(), USPRINTF_NTORN | USPRINTF_PIPEISCMD);
    }
  }
  else
    m_szMessage = strdup("");

  gUserManager.DropOwner();

  // Check for pipes, should possibly go after the ClientToServer call
  m_szMessage = PipeInput(m_szMessage);

  gTranslator.ClientToServer(m_szMessage);

  m_nSize += strlen(m_szMessage)+1;
}

void CPU_AckThroughServer::InitBuffer()
{
  CPU_CommonFamily::InitBuffer();

  buffer->PackUnsignedLongBE(m_nMsgID[0]);
  buffer->PackUnsignedLongBE(m_nMsgID[1]);
  buffer->PackUnsignedShortBE(2);
  buffer->PackChar(m_nUinLen);
  buffer->Pack(m_szUin, m_nUinLen);
	buffer->PackUnsignedShortBE(0x03);
  buffer->PackUnsignedShort(0x1b);
  buffer->PackUnsignedShort(ICQ_VERSION_TCP);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedShortBE(0);
  buffer->PackUnsignedLong(3);
  buffer->PackChar(4);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedShort(0x0e);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedShort(m_nSubCommand);
  buffer->PackUnsignedShort(m_nStatus);
  buffer->PackUnsignedShort(m_nLevel);
  buffer->PackString(m_szMessage);
  free(m_szMessage);
}


//-----AckGeneral--------------------------------------------------------------
CPU_AckGeneral::CPU_AckGeneral(ICQUser *u, unsigned long nMsgID1,
                               unsigned long nMsgID2, unsigned short nSequence,
                               unsigned short nMsgType, bool bAccept,
                               unsigned short nLevel)
  : CPU_AckThroughServer(u, nMsgID1, nMsgID2, nSequence, nMsgType, bAccept,
                         nLevel)
{
  m_nSize += 8;
  InitBuffer();

  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0xFFFFFF00);
}


//-----AckFileAccept-----------------------------------------------------------
CPU_AckFileAccept::CPU_AckFileAccept(ICQUser *u,//unsigned long nUin,
																		 unsigned long nMsgID[2],
																		 unsigned short nSequence,
																		 unsigned short nPort)
	: CPU_AdvancedMessage(u, ICQ_CMDxSUB_FILE, 0, true, nSequence, nMsgID[0],
												nMsgID[1])
{
#if 1
	// XXX This is not the ICBM way yet!
	// XXX It doesnt' even work! Perhaps try ICBM and it'll work?
	m_nSize += 15;
	InitBuffer();

	//buffer->PackString(""); // description
	buffer->PackUnsignedLong(ReversePort(nPort)); // port reversed
	buffer->PackString(""); // filename
	buffer->PackUnsignedLong(0); // filesize
	buffer->PackUnsignedLong(nPort); // port
#else
	m_nSize += 80;
	InitBuffer();

	buffer->PackUnsignedShort(0x32);  // len of following plugin info
	buffer->PackUnsignedLongBE(0xF02D12D9);
	buffer->PackUnsignedLongBE(0x3091D311);
	buffer->PackUnsignedLongBE(0x8DD70010);
	buffer->PackUnsignedLongBE(0x4B06462E);
	buffer->PackUnsignedShortBE(0x0000);
	buffer->PackUnsignedLong(13); // strlen - is 13 bytes though
	buffer->Pack("File Transfer", 13);
	buffer->PackUnsignedLongBE(0x00000101);
	buffer->PackUnsignedLongBE(0x00000000);
	buffer->PackUnsignedLongBE(0);
	buffer->PackUnsignedShortBE(0);
	buffer->PackChar(0);
	buffer->PackUnsignedLong(20); //remaining  - is 4 bytes
                                //dont count last 4 bytes
	buffer->PackUnsignedLong(0); // file desc - is 4 bytes
	buffer->PackChar(0); // file desc
	buffer->PackUnsignedLong(ReversePort(nPort)); // port reversed
	buffer->PackString(""); // filename
	buffer->PackUnsignedLong(0); // filesize
	buffer->PackUnsignedLong(nPort); // port
	buffer->PackUnsignedLongBE(0x00030000); // ack request
#endif
}


//-----AckFileRefuse-----------------------------------------------------------
CPU_AckFileRefuse::CPU_AckFileRefuse(ICQUser *u, unsigned long nMsgID[2],
																		 unsigned short nSequence, const char *msg)
	: CPU_AckThroughServer(u, nMsgID[0], nMsgID[1], nSequence,
												 ICQ_CMDxSUB_FILE, false, 0)
{
	// XXX This is not the ICBM way yet!
	m_nSize += strlen(msg) + 18;
	InitBuffer();

	buffer->PackString(msg);
	buffer->PackUnsignedLong(0); // port reversed
	buffer->PackString("");
	buffer->PackUnsignedLong(0);
	buffer->PackUnsignedLong(0);
}

//-----AckChatAccept-----------------------------------------------------------
CPU_AckChatAccept::CPU_AckChatAccept(ICQUser *u, const char *szClients,
                                          unsigned long nMsgID[2],
                                          unsigned short nSequence,
                                          unsigned short nPort)
	: CPU_AdvancedMessage(u, ICQ_CMDxSUB_CHAT, 0, true, nSequence,
                             nMsgID[0], nMsgID[1])
{
	// XXX This is not the ICBM way yet!
	m_nSize += 11 + strlen_safe(szClients);
	InitBuffer();

	buffer->PackString(szClients);
	buffer->PackUnsignedLong(ReversePort(nPort)); // port reversed
	buffer->PackUnsignedLong(nPort);
}

//-----AckChatRefuse-----------------------------------------------------------
CPU_AckChatRefuse::CPU_AckChatRefuse(ICQUser *u, unsigned long nMsgID[2],
																		 unsigned short nSequence, const char *msg)
	: CPU_AckThroughServer(u, nMsgID[0], nMsgID[1], nSequence,
												 ICQ_CMDxSUB_CHAT, false, 0)
{
	// XXX This is not the ICBM way yet!
	m_nSize += strlen(msg) + 14;
	InitBuffer();

	buffer->PackString(msg);
	buffer->PackString("");
	buffer->PackUnsignedLong(0);
	buffer->PackUnsignedLong(0);
}

//-----SendSms-----------------------------------------------------------------
CPU_SendSms::CPU_SendSms(const char *szNumber, const char *szMessage)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_SENDxSMS;

  char szXmlStr[460];

  char szTime[30];
  time_t tTime;
  struct tm *tmTime;
  time(&tTime);
  tmTime = gmtime(&tTime);
  strftime(szTime, 30, "%a, %d %b %Y %T %Z", tmTime);
  
  char szParsedNumber[17] = "+";
  ParseDigits(&szParsedNumber[1], szNumber, 15);
  
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  char szUin[13];
  szUin[12] = '\0';
  snprintf(szUin, 12, "%lu", o->Uin());

  snprintf(szXmlStr, 460, "<icq_sms_message><destination>%s</destination><text>%.160s</text><codepage>1252</codepage><encoding>utf8</encoding><senders_UIN>%s</senders_UIN><senders_name>%s</senders_name><delivery_receipt>Yes</delivery_receipt><time>%s</time></icq_sms_message>",
	   szParsedNumber, szMessage, szUin, o->GetAlias(), szTime);
  szXmlStr[459] = '\0';
  gUserManager.DropOwner();
  
  int nLenXmlStr = strlen_safe(szXmlStr) + 1;
  int packetSize = 2+2+2+4+2+2+2 + 22 + 2 + nLenXmlStr;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xD007); 			// type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); 		// subtype
  
  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackUnsignedShortBE(0x0016);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedShort(0);
  
  buffer->PackUnsignedShortBE(nLenXmlStr);
  buffer->Pack(szXmlStr, nLenXmlStr);
}

//----RequestAuth------------------------------------------------------------
CPU_RequestAuth::CPU_RequestAuth(unsigned long _nUin, const char *_szMsg)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_AUTHxREQ)
{
  char szUin[13];
  int nUinLen = snprintf(szUin, 12, "%lu", _nUin);
  szUin[12] = '\0';
  int nMsgLen = strlen(_szMsg);

  m_nSize += nUinLen + nMsgLen + 5;
  InitBuffer();

  buffer->PackChar(nUinLen);
  buffer->Pack(szUin, nUinLen);
  buffer->PackUnsignedShortBE(nMsgLen);
  buffer->Pack(_szMsg, nMsgLen);
  buffer->PackUnsignedShortBE(0);
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

//-----RequestList--------------------------------------------------------------
CPU_RequestList::CPU_RequestList()
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_REQUESTxROST)
{
  m_nSize += 6;
  InitBuffer();

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  buffer->PackUnsignedLongBE(o->GetSSTime());
  buffer->PackUnsignedShortBE(o->GetSSCount());
  gUserManager.DropOwner();
}

//-----ExportContactStart-------------------------------------------------------
CPU_ExportContactStart::CPU_ExportContactStart()
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxEDITxSTART)
{
  m_nSize += 4;
  InitBuffer();

  buffer->PackUnsignedLongBE(0x00010000);
}

//-----ExportToServerList-------------------------------------------------------
CPU_ExportToServerList::CPU_ExportToServerList(UinList &uins,
                                               unsigned short _nType)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxADD)
{
  unsigned short m_nSID = 0;
  unsigned short m_nGSID = 0;
  int nSize = 0;

  UinList::iterator i;
  for (i = uins.begin(); i != uins.end(); i++)
  {
    ICQUser *pUser = gUserManager.FetchUser(*i, LOCK_R);
    if (pUser)
    {
      char szTmp[13];
      szTmp[12] = '\0';

      nSize += snprintf(szTmp, 12, "%lu", *i);
      nSize += 10;

      char *szUnicode = gTranslator.ToUnicode(pUser->GetAlias());
      int nAliasLen = strlen(szUnicode);
      if (nAliasLen && _nType == ICQ_ROSTxNORMAL)
          nSize += 4 + nAliasLen;
      if (szUnicode)
        delete [] szUnicode;
    }
    gUserManager.DropUser(pUser);
  }

  m_nSize += nSize;
  InitBuffer();

  for (i = uins.begin(); i != uins.end(); i++)
  {
    char szUin[13];
    szUin[12] = '\0';
    int nLen;
    int nAliasSize = 0;
    char *szUnicodeName = 0;

    m_nSID = gUserManager.GenerateSID();

    // Save the SID
    ICQUser *u = gUserManager.FetchUser(*i, LOCK_W);
    switch (_nType)
    {
      case ICQ_ROSTxIGNORE: // same as ICQ_ROSTxNORMAL
      case ICQ_ROSTxNORMAL:    u->SetSID(m_nSID);  break;
      case ICQ_ROSTxINVISIBLE: u->SetInvisibleSID(m_nSID);  break;
      case ICQ_ROSTxVISIBLE:   u->SetVisibleSID(m_nSID);  break;
    }

    if (_nType == ICQ_ROSTxNORMAL)
    {
      // Use the first group that the user is in as the server stored group
      GroupIDList *pID = gUserManager.LockGroupIDList(LOCK_R);
      for (unsigned short j = 1; j < pID->size() + 1; j++)
      {
        if (u->GetInGroup(GROUPS_USER, j))
        {
          m_nGSID = (*pID)[j-1];
          if (m_nGSID)
            break;
        }
      }

      // No group yet?  Use default.  No default? Use ID of 1 (general)
      if (m_nGSID == 0)
      {
        unsigned short nNewGroup = gUserManager.NewUserGroup();
        if (nNewGroup && nNewGroup <= pID->size())
          m_nGSID = (*pID)[nNewGroup-1];

        if (m_nGSID == 0 && pID->size())
          m_nGSID = (*pID)[0]; // first group if none was specified

        if (m_nGSID == 0)
          m_nGSID = 1; // General (unless user renamed group or wasnt created yet)
      }
      gUserManager.UnlockGroupIDList();

      u->SetGSID(m_nGSID);
      szUnicodeName = gTranslator.ToUnicode(u->GetAlias());
      nAliasSize = strlen(szUnicodeName);
    }

    gUserManager.DropUser(u);

    SetExtraInfo(m_nGSID);

    nLen = snprintf(szUin, 12, "%lu", *i);
    buffer->PackUnsignedShortBE(nLen);
    buffer->Pack(szUin, nLen);
    buffer->PackUnsignedShortBE(m_nGSID);
    buffer->PackUnsignedShortBE(m_nSID);
    buffer->PackUnsignedShortBE(_nType);

    if (nAliasSize)
    {
      buffer->PackUnsignedShortBE(nAliasSize+4);
      buffer->PackUnsignedShortBE(0x0131);
      buffer->PackUnsignedShortBE(nAliasSize);
      buffer->Pack(szUnicodeName, nAliasSize);
    }
    else
      buffer->PackUnsignedShortBE(0);

    if (szUnicodeName)
      delete [] szUnicodeName;
  }
}

//-----ExportGroupsToServerList-------------------------------------------------
CPU_ExportGroupsToServerList::CPU_ExportGroupsToServerList(GroupList &groups)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxADD)
{
  int nSize = 0;
  int nGSID = 0;

  GroupList::iterator g;
  for (g = groups.begin(); g != groups.end(); g++)
  {
    char *szUnicode = gTranslator.ToUnicode(*g);
    nSize += strlen(szUnicode);
    nSize += 10;
    delete [] szUnicode;
  }
  
  m_nSize += nSize;
  InitBuffer();

  // Not necessary, but just to make it explicit that it occurs
  SetExtraInfo(0); // top level group (contains all the groups)

  for (g = groups.begin(); g != groups.end(); g++)
  {
    nGSID = gUserManager.GenerateSID();

    gUserManager.ModifyGroupID(*g, nGSID);

    char *szUnicodeName = gTranslator.ToUnicode(*g);

    buffer->PackUnsignedShortBE(strlen(szUnicodeName));
    buffer->Pack(szUnicodeName, strlen(szUnicodeName));
    buffer->PackUnsignedShortBE(nGSID);
    buffer->PackUnsignedShortBE(0);
    buffer->PackUnsignedShortBE(ICQ_ROSTxGROUP);
    buffer->PackUnsignedShortBE(0);

    delete [] szUnicodeName;
  }
}

//-----AddToServerList----------------------------------------------------------
CPU_AddToServerList::CPU_AddToServerList(const char *_szName,
                                         unsigned short _nType,
                                         unsigned short _nGroup, bool _bAuthReq)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxADD), m_nSID(0),
    m_nGSID(0)
{
  unsigned short nStrLen = strlen(_szName);
  unsigned short nExportSize = 0;
  ICQUser *u = 0;
  char *szUnicodeName = 0;

  m_nSID = gUserManager.GenerateSID();

  switch (_nType)
  {
    case ICQ_ROSTxNORMAL:
    {
      unsigned long nUin;
      sscanf(_szName, "%lu", &nUin);

      // Save the SID
      u = gUserManager.FetchUser(nUin, LOCK_W);
      u->SetSID(m_nSID);
      u->SetAwaitingAuth(_bAuthReq);

      // Check for a group id
      GroupIDList *pID = gUserManager.LockGroupIDList(LOCK_R);
      if (_nGroup)
      {
        // Use the passed in group
        m_nGSID = (*pID)[_nGroup-1];
      }
      else if (u->GetGSID() && _bAuthReq)
      {
        m_nGSID = u->GetGSID(); // changing groups but require auth
      }
      else
      {
        // Use the first group that the user is in as the server stored group
        for (unsigned short i = 1; i < pID->size() + 1; i++)
        {
          if (u->GetInGroup(GROUPS_USER, i))
          {
            m_nGSID = (*pID)[i-1];
            if (m_nGSID)
              break;
          }
        }
      }

      if (m_nGSID == 0)
      {
        unsigned short nNewGroup = gUserManager.NewUserGroup();
        if (nNewGroup && nNewGroup <= pID->size())
          m_nGSID = (*pID)[nNewGroup-1];

        if (m_nGSID == 0 && pID->size())
          m_nGSID = (*pID)[0];
        
        if (m_nGSID == 0)
          m_nGSID = 1; // General (unless user renamed group)
      }

      szUnicodeName = gTranslator.ToUnicode(u->GetAlias());
      nExportSize = 4 + strlen(szUnicodeName);

      SetExtraInfo(m_nGSID);
      u->SetGSID(m_nGSID);
      gUserManager.UnlockGroupIDList();

      break;
    }

    case ICQ_ROSTxGROUP:
    {
      // the way it works
      m_nGSID = m_nSID;
      m_nSID = 0;
      SetExtraInfo(0);

      szUnicodeName = gTranslator.ToUnicode((char *)_szName);
      nStrLen = strlen(szUnicodeName);

      // modifygroupid needs write access, so unlock to make sure it gets what it wants.
      gUserManager.UnlockGroupIDList();

      gUserManager.ModifyGroupID(const_cast<char *>(_szName), m_nGSID);
      break;
    }

    case ICQ_ROSTxINVISIBLE:
    case ICQ_ROSTxVISIBLE:
    case ICQ_ROSTxIGNORE:
    {
      m_nGSID = 0;

      unsigned long nUin;
      sscanf(_szName, "%lu", &nUin);

      ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
      if (_nType == ICQ_ROSTxIGNORE)
      {
        u->SetSID(m_nSID);
        u->SetGSID(0); // m_nGSID
      }
      else if (_nType == ICQ_ROSTxVISIBLE)
        u->SetVisibleSID(m_nSID);
      else
        u->SetInvisibleSID(m_nSID);

      gUserManager.DropUser(u);

      SetExtraInfo(0); // not necessary except by design
      break;
    }
  }

  m_nSize += 10+nStrLen+nExportSize+(_bAuthReq ? 4 : 0);
  InitBuffer();

  buffer->PackUnsignedShortBE(nStrLen);
  if (szUnicodeName && _nType == ICQ_ROSTxGROUP)
    buffer->Pack(szUnicodeName, nStrLen);
  else
    buffer->Pack(_szName, nStrLen);
  buffer->PackUnsignedShortBE(m_nGSID);
  buffer->PackUnsignedShortBE(m_nSID);
  buffer->PackUnsignedShortBE(_nType);
  buffer->PackUnsignedShortBE(nExportSize+(_bAuthReq ? 4 : 0));
  if (nExportSize)
  {
    buffer->PackUnsignedShortBE(0x0131);
    buffer->PackUnsignedShortBE(nExportSize-4);
    buffer->Pack(szUnicodeName, nExportSize-4);
  }
  if (_bAuthReq)
    buffer->PackUnsignedLongBE(0x00660000);

  if (u)
    gUserManager.DropUser(u);

  if (szUnicodeName)
    delete [] szUnicodeName;
}

//-----RemoveFromServerList-----------------------------------------------------
CPU_RemoveFromServerList::CPU_RemoveFromServerList(const char *_szName,
	unsigned short _nGSID, unsigned short _nSID, unsigned short _nType)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxREM)
{
  int nNameLen = strlen(_szName);
  char *szUnicodeName = 0;

  if (_nType == ICQ_ROSTxGROUP)
  {
    szUnicodeName = gTranslator.ToUnicode((char *)_szName);
    nNameLen = strlen(szUnicodeName);
  }  

  m_nSize += 10+nNameLen;
  InitBuffer();

  buffer->PackUnsignedShortBE(nNameLen);
  if (szUnicodeName)
    buffer->Pack(szUnicodeName, nNameLen);
  else
    buffer->Pack(_szName, nNameLen);
  buffer->PackUnsignedShortBE(_nGSID);
  buffer->PackUnsignedShortBE(_nSID);
  buffer->PackUnsignedShortBE(_nType);
  buffer->PackUnsignedShortBE(0);

  if (_nType == ICQ_ROSTxGROUP)
    SetExtraInfo(0);
  else
    SetExtraInfo(_nGSID);

  if (szUnicodeName)
    delete [] szUnicodeName;
}

//-----ClearServerList------------------------------------------------------
CPU_ClearServerList::CPU_ClearServerList(UinList &uins,
                                         unsigned short _nType)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxREM)
{
  int nSize = 0;
  
  UinList::iterator i;
  for (i = uins.begin(); i != uins.end(); i++)
  {
    ICQUser *pUser = gUserManager.FetchUser(*i, LOCK_R);
    if (pUser)
    {
      nSize += strlen(pUser->UinString()) + 2;
      nSize += 8;
      if (pUser->GetAwaitingAuth())
        nSize += 4;
      gUserManager.DropUser(pUser);
    }
  }
  
  m_nSize += nSize;
  InitBuffer();
  
  for (i = uins.begin(); i != uins.end(); i++)
  {
    ICQUser *pUser = gUserManager.FetchUser(*i, LOCK_W);
    if (pUser)
    {
      bool bAuthReq = pUser->GetAwaitingAuth();
      unsigned short nGSID = 0;
      
      if (_nType == ICQ_ROSTxNORMAL)
        nGSID = pUser->GetGSID();
        
      buffer->PackUnsignedShortBE(strlen(pUser->UinString()));
      buffer->Pack(pUser->UinString(), strlen(pUser->UinString()));
      buffer->PackUnsignedShortBE(nGSID);
      buffer->PackUnsignedShortBE(pUser->GetSID());
      buffer->PackUnsignedShortBE(_nType);
      buffer->PackUnsignedShortBE(bAuthReq ? 4 : 0);
      if (bAuthReq)
        buffer->PackUnsignedShortBE(0x00660000);
        
      // Clear their info now
      if (_nType == ICQ_ROSTxNORMAL)
      {
        pUser->SetSID(0);
        pUser->SetGSID(0);
      }
      else if (_nType == ICQ_ROSTxVISIBLE)
        pUser->SetVisibleSID(0);
      else if (_nType == ICQ_ROSTxINVISIBLE)
        pUser->SetInvisibleSID(0);
        
      if (pUser->GetSID() == 0 && pUser->GetVisibleSID() == 0 &&
          pUser->GetInvisibleSID() == 0)
        pUser->SetAwaitingAuth(false);
    }
  }
}

//-----UpdateToServerList---------------------------------------------------
CPU_UpdateToServerList::CPU_UpdateToServerList(const char *_szName,
                                               unsigned short _nType,
                                               unsigned short _nGSID, // only for groups
                                               bool _bAuthReq)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxUPD_GROUP)
{
  unsigned long nUin = 0;
  unsigned short nGSID = 0;
  unsigned short nSID = 0;
  unsigned short nExtraLen = 0;
  unsigned short nNameLen = strlen(_szName);
  char *szUnicodeName = 0;
  GroupIDList *gID = 0;

  switch (_nType)
  {
    case ICQ_ROSTxNORMAL:
    {
      sscanf(_szName, "%lu", &nUin);
      ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
      if (u)
      {
        if (u->GetAwaitingAuth())
          _bAuthReq = true;

        nGSID = u->GetGSID();
        nSID = u->GetSID();
        szUnicodeName = gTranslator.ToUnicode(u->GetAlias());
        nExtraLen = 4 + strlen(szUnicodeName);
        gUserManager.DropUser(u);
      }

      break;
    }

    case ICQ_ROSTxGROUP:
    {
      nGSID = _nGSID;
      nSID = 0;

      if (nGSID == 0)
      {
        gID = gUserManager.LockGroupIDList(LOCK_R);
        nExtraLen += (gID->size() * 2);
        if (nExtraLen == 0)
          gUserManager.UnlockGroupIDList();
      }
      else
      {
        szUnicodeName = gTranslator.ToUnicode((char *)_szName);
        nNameLen = strlen(szUnicodeName);
        FOR_EACH_USER_START(LOCK_R)
        {
          if (pUser->GetGSID() == nGSID)
            nExtraLen += 2;
        }
        FOR_EACH_USER_END
      }

      if (nExtraLen)
        nExtraLen += 4;
      break;
    }
  }

  m_nSize += 10 + nNameLen + nExtraLen + (_bAuthReq ? 4 : 0);
  InitBuffer();

  buffer->PackUnsignedShortBE(nNameLen);
  if (szUnicodeName && _nType == ICQ_ROSTxGROUP)
    buffer->Pack(szUnicodeName, nNameLen);
  else
    buffer->Pack(_szName, nNameLen);
  buffer->PackUnsignedShortBE(nGSID);
  buffer->PackUnsignedShortBE(nSID);
  buffer->PackUnsignedShortBE(_nType);
  buffer->PackUnsignedShortBE(nExtraLen + (_bAuthReq ? 4 : 0));
  if (nExtraLen)
  {
    if (_nType == ICQ_ROSTxNORMAL)
    {
      buffer->PackUnsignedShortBE(0x0131);
      buffer->PackUnsignedShortBE(nExtraLen-4);
      buffer->Pack(szUnicodeName, nExtraLen-4);
    }
    else if (_nType == ICQ_ROSTxGROUP)
    {
      buffer->PackUnsignedShortBE(0x00C8);
      buffer->PackUnsignedShortBE(nExtraLen-4);
      
      if (nGSID == 0)
      {
        for (unsigned int i = 0; i != gID->size(); i++)
        {
          buffer->PackUnsignedShortBE((*gID)[i]);
        }
        gUserManager.UnlockGroupIDList();
      }
      else
      {
        FOR_EACH_USER_START(LOCK_R)
        {
          if (pUser->GetGSID() == nGSID)
            buffer->PackUnsignedShortBE(pUser->GetSID());
        }
        FOR_EACH_USER_END
      }
    }
  }

  if (_bAuthReq)
    buffer->PackUnsignedLongBE(0x00660000);

  if (szUnicodeName)
    delete [] szUnicodeName;
}

//-----SearchWhitePages---------------------------------------------------------
CPU_SearchWhitePages::CPU_SearchWhitePages(const char *szFirstName,
    const char *szLastName, const char *szAlias, const char *szEmail,
    unsigned short nMinAge, unsigned short nMaxAge, char nGender,
    char nLanguage, const char *szCity, const char *szState, unsigned short nCountryCode,
    const char *szCoName, const char *szCoDept, const char *szCoPos,
    const char *szKeyword, bool bOnlineOnly)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  unsigned short nDataLen;	// length of data info to add packet size
  
  nDataLen = LengthField(szFirstName) + LengthField(szLastName) +
             LengthField(szAlias) + LengthField(szCity) +
             LengthField(szEmail) + LengthField(szState) +
             LengthField(szCoName) + LengthField(szCoDept) +
             LengthField(szCoPos) + LengthField(szKeyword);

  nDataLen += 5*!(!nGender) + 6*!(!nLanguage) + 4*(!(!nMaxAge) + !(!nMinAge))
	      + 6*(!(!nCountryCode)) + 5*(bOnlineOnly);
  m_nSize += 16 + nDataLen;
  m_nMetaCommand = ICQ_CMDxMETA_SEARCHxWP;
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
  gTranslator.ClientToServer((char *) szKeyword);

  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackUnsignedShortBE(16 + nDataLen - 4);
  buffer->PackUnsignedShort(16 + nDataLen - 6);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xd007);
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand);
  
  PackSearch(ICQ_CMDxWPxFNAME, szFirstName);
  PackSearch(ICQ_CMDxWPxLNAME, szLastName);
  PackSearch(ICQ_CMDxWPxALIAS, szAlias);
  PackSearch(ICQ_CMDxWPxEMAIL, szEmail);
  PackSearch(ICQ_CMDxWPxCITY, szCity);
  PackSearch(ICQ_CMDxWPxSTATE, szState);
  PackSearch(ICQ_CMDxWPxCOMPANY, szCoName);
  PackSearch(ICQ_CMDxWPxCODEPT, szCoDept);
  PackSearch(ICQ_CMDxWPxCOPOS, szCoPos);
  PackSearch(ICQ_CMDxWPxKEYWORD, szKeyword);
  
  if (nMinAge)
  {
    buffer->PackUnsignedShortBE(ICQ_CMDxWPxAGE);
    buffer->PackUnsignedShort(0x04);	// length
    buffer->PackUnsignedShort(nMinAge);
    buffer->PackUnsignedShort(nMaxAge);
  }
  if (nGender)
  {
    buffer->PackUnsignedShortBE(ICQ_CMDxWPxGENDER);
    buffer->PackUnsignedShort(0x01);	// length
    buffer->PackChar(nGender);
  }
  if (nLanguage)
  {
    buffer->PackUnsignedShortBE(ICQ_CMDxWPxLANGUAGE);
    buffer->PackUnsignedShort(0x02);	// length
    buffer->PackUnsignedShort(nLanguage);
  }
  if (nCountryCode)
  {
    buffer->PackUnsignedShortBE(ICQ_CMDxWPxCOUNTRY);
    buffer->PackUnsignedShort(0x02);	// length
    buffer->PackUnsignedShort(nCountryCode);
  }
  if (bOnlineOnly)
  {
    buffer->PackUnsignedShortBE(ICQ_CMDxWPxONLINE);
    buffer->PackUnsignedShort(0x01);	// length
    buffer->PackChar(0x01);
  }
}

void CPU_SearchWhitePages::PackSearch(unsigned short nCmd, const char *szField)
{
  // By SC
  // Pack string field info if exist

  int nLenField = strlen(szField) + 1;

  if (nLenField > 1)
  {
    buffer->PackUnsignedShortBE(nCmd);
    buffer->PackUnsignedShort(nLenField + 2);
    buffer->PackLNTS(szField);
  }
}

//-----SearchByUin--------------------------------------------------------------
CPU_SearchByUin::CPU_SearchByUin(unsigned long nUin)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_SEARCHxUIN;
  int nPacketSize = 24; //2+2+2+4+2+2+2+4+4;
  m_nSize += nPacketSize;
  InitBuffer();

  // TLV 1
  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackUnsignedShortBE(nPacketSize - 4);
  buffer->PackUnsignedShort(nPacketSize - 6); // bytes remaining
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xD007); // type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); // sub type

  buffer->PackUnsignedLongBE(0x36010400);
  buffer->PackUnsignedLong(nUin);
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
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nSubCommand = ICQ_CMDxMETA_PASSWORDxSET;

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
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxSND_SYSxMSGxREQ;
  m_nSize += 14;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x0001000a); // TLV
  buffer->PackUnsignedShort(8);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShort(ICQ_CMDxSND_SYSxMSGxREQ);
  buffer->PackUnsignedShortBE(0x0200);
}

//-----SysMsgDoneAck------------------------------------------------------------
CPU_SysMsgDoneAck::CPU_SysMsgDoneAck(unsigned short nId)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxSND_SYSxMSGxDONExACK;
  m_nSize += 14;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x0001000a); // TLV
  buffer->PackUnsignedShort(0x0008); // len again
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShort(ICQ_CMDxSND_SYSxMSGxDONExACK);
  buffer->PackUnsignedShortBE(nId);
}

//-----CPU_SetRandomChatGroup------------------------------------------------
CPU_SetRandomChatGroup::CPU_SetRandomChatGroup(unsigned long nGroup)
 : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nGroup = nGroup;
  m_nMetaCommand = ICQ_CMDxMETA_SETxRANDOMxCHAT;
  unsigned short nPacketSize = 18;
  if (nGroup)
    nPacketSize += 33;

  m_nSize += nPacketSize;
  InitBuffer();

  // TLV 1
  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackUnsignedShortBE(nPacketSize - 4);
  buffer->PackUnsignedShort(nPacketSize - 6); // bytes remaining
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xD007); // type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); // sub type

  buffer->PackUnsignedShort(nGroup);
  if (nGroup)
  {
    buffer->PackUnsignedLongBE(0x00000300);
    buffer->PackUnsignedLong(RealIp());
    buffer->PackUnsignedLong(0); // port
    buffer->PackUnsignedLong(LocalIp());
    buffer->PackChar(Mode());
    buffer->PackUnsignedLong(ICQ_VERSION_TCP);
    buffer->PackUnsignedLong(0x00000000);
    buffer->PackUnsignedLong(0x00000050);
    buffer->PackUnsignedLong(0x00000003);
  }
}


//-----CPU_RandomChatSearch--------------------------------------------------
CPU_RandomChatSearch::CPU_RandomChatSearch(unsigned long nGroup)
 : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_RANDOMxSEARCH;
  unsigned short nPacketSize = 18;

  m_nSize += nPacketSize;
  InitBuffer();

  // TLV 1
  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackUnsignedShortBE(nPacketSize - 4);
  buffer->PackUnsignedShort(nPacketSize - 6); //bytes remaining
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xD007); // type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); // sub type
  buffer->PackUnsignedShort(nGroup);
}


//-----Meta_SetGeneralInfo---------------------------------------------------
CPU_Meta_SetGeneralInfo::CPU_Meta_SetGeneralInfo(const char *szAlias,
                          const char *szFirstName, const char *szLastName,
                          const char *szEmailPrimary,
                          const char *szCity, const char *szState,
                          const char *szPhoneNumber, const char *szFaxNumber,
                          const char *szAddress, const char *szCellularNumber,
                          const char *szZipCode,
                          unsigned short nCountryCode,
                          bool bHideEmail)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_GENERALxINFOxSET;

  m_nCountryCode = nCountryCode;
  m_nTimezone = ICQUser::SystemTimezone();
  m_nHideEmail = bHideEmail ? 1 : 0;

  int packetSize = 2+2+2+4+2+2+2 + strlen_safe(szAlias) + strlen_safe(szFirstName) + strlen_safe(szLastName) +
             strlen_safe(szEmailPrimary) + strlen_safe(szCity) +
             strlen_safe(szState) + strlen_safe(szPhoneNumber) +
             strlen_safe(szFaxNumber) + strlen_safe(szAddress) + strlen_safe(szZipCode) +
             strlen_safe(szCellularNumber) + 33 + 2 + 1 + 1;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xD007); 			// type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); // subtype

  gTranslator.ClientToServer((char *) szAlias);
  gTranslator.ClientToServer((char *) szFirstName);
  gTranslator.ClientToServer((char *) szLastName);
  gTranslator.ClientToServer((char *) szEmailPrimary);
  gTranslator.ClientToServer((char *) szCity);
  gTranslator.ClientToServer((char *) szState);
  gTranslator.ClientToServer((char *) szPhoneNumber);
  gTranslator.ClientToServer((char *) szFaxNumber);
  gTranslator.ClientToServer((char *) szAddress);
  gTranslator.ClientToServer((char *) szCellularNumber);
  gTranslator.ClientToServer((char *) szZipCode);

  m_szAlias = buffer->PackString(szAlias);
  m_szFirstName = buffer->PackString(szFirstName);
  m_szLastName = buffer->PackString(szLastName);
  m_szEmailPrimary = buffer->PackString(szEmailPrimary);
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

//-----Meta_SetEmailInfo------------------------------------------------------
CPU_Meta_SetEmailInfo::CPU_Meta_SetEmailInfo( const char *szEmailSecondary,
                       const char *szEmailOld)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_EMAILxINFOxSET;

  int packetSize = 2+2+2+4+2+2+2 + strlen_safe(szEmailSecondary) + 3
				 + strlen_safe(szEmailOld) + 3 + 3;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xD007); 			// type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); // subtype

  gTranslator.ClientToServer((char *) szEmailSecondary);
  gTranslator.ClientToServer((char *) szEmailOld);
  
  buffer->PackChar(2);
  buffer->PackChar(0);
  m_szEmailSecondary = buffer->PackString(szEmailSecondary);
  buffer->PackChar(0);
  m_szEmailOld = buffer->PackString(szEmailOld);
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
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
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

  int packetSize = 2+2+2+4+2+2+2 + strlen_safe(szHomepage)+3 + 10;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xD007); 			// type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); 			// subtype

  gTranslator.ClientToServer((char *) szHomepage);

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
    const char *szZip,
    unsigned short nCompanyCountry,
    const char *szName,
    const char *szDepartment,
    const char *szPosition,
    const char *szHomepage)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_WORKxINFOxSET;
  
  m_nCompanyCountry = nCompanyCountry;  
  
  char szStatebuf[6];

  szStatebuf[5] = '\0';
  snprintf(szStatebuf, 5, szState);

  int packetSize = 2+2+2+4+2+2+2 + strlen_safe(szCity) + strlen_safe(szStatebuf) + strlen_safe(szPhoneNumber) +
		    strlen_safe(szFaxNumber) + strlen_safe(szAddress) + strlen_safe(szZip) + 2 + strlen_safe(szName) +
		    strlen_safe(szDepartment) + strlen_safe(szPosition) + 2 +
		    strlen_safe(szHomepage) + 30;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xD007); 			// type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); 			// subtype

  gTranslator.ClientToServer((char *) szCity);
  gTranslator.ClientToServer((char *) szState);
  gTranslator.ClientToServer((char *) szPhoneNumber);
  gTranslator.ClientToServer((char *) szFaxNumber);
  gTranslator.ClientToServer((char *) szAddress);
  gTranslator.ClientToServer((char *) szZip);
  gTranslator.ClientToServer((char *) szName);
  gTranslator.ClientToServer((char *) szDepartment);
  gTranslator.ClientToServer((char *) szPosition);
  gTranslator.ClientToServer((char *) szHomepage);

  m_szCity = buffer->PackString(szCity);
  m_szState = buffer->PackString(szStatebuf);
  m_szPhoneNumber = buffer->PackString(szPhoneNumber);
  m_szFaxNumber = buffer->PackString(szFaxNumber);
  m_szAddress = buffer->PackString(szAddress);
  m_szZip = buffer->PackString(szZip);
  buffer->PackUnsignedShort(m_nCompanyCountry);
  m_szName = buffer->PackString(szName);
  m_szDepartment = buffer->PackString(szDepartment);
  m_szPosition = buffer->PackString(szPosition);
  buffer->PackUnsignedShort(0x0500);
  m_szHomepage = buffer->PackString(szHomepage);
}

//-----Meta_SetAbout---------------------------------------------------------
CPU_Meta_SetAbout::CPU_Meta_SetAbout(const char *szAbout)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_ABOUTxSET;

  int packetSize = 2+2+2+4+2+2+2 + strlen_safe(szAbout) + 3;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShortBE(0xD007); 			// type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); 			// subtype

  m_szAbout = szAbout == NULL ? strdup("") : strdup(szAbout);
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
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_SECURITYxSET;
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
  buffer->PackUnsignedShort(m_nMetaCommand); // subtype
  buffer->PackChar(m_nAuthorization);
  buffer->PackChar(m_nWebAware);
  buffer->PackChar(1);
  buffer->PackChar(0);
}


//-----Meta_RequestInfo------------------------------------------------------
CPU_Meta_RequestAllInfo::CPU_Meta_RequestAllInfo(unsigned long nUin)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_REQUESTxALLxINFO;
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
  buffer->PackUnsignedShort(m_nMetaCommand); // subtype
  buffer->PackUnsignedLong(m_nUin);
}


//-----Meta_RequestInfo------------------------------------------------------
CPU_Meta_RequestBasicInfo::CPU_Meta_RequestBasicInfo(unsigned long nUin)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_REQUESTxBASICxINFO;
  m_nUin = nUin;

  m_nSize += 20;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x0001000e); // TLV

  buffer->PackUnsignedShort(0x000c); // Bytes remaining
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShort(m_nMetaCommand);
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

  m_nSize = 48;
  buffer = new CBuffer(m_nSize);

  buffer->PackChar(ICQ_CMDxTCP_HANDSHAKE);
  buffer->PackUnsignedShort(ICQ_VERSION_TCP);
  buffer->PackUnsignedShort(0x002b); // size
  buffer->PackUnsignedLong(m_nDestinationUin);
  buffer->PackUnsignedShort(0);
  buffer->PackUnsignedLong(nLocalPort == 0 ? s_nLocalPort : nLocalPort);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackChar(s_nMode);
  buffer->PackUnsignedLong(nLocalPort == 0 ? s_nLocalPort : nLocalPort);

  ICQUser *u = gUserManager.FetchUser(nDestinationUin, LOCK_R);
  buffer->PackUnsignedLong(u->Cookie());
  m_nSessionId = u->Cookie();
  gUserManager.DropUser(u);

  buffer->PackUnsignedLong(0x00000050); // constant
  buffer->PackUnsignedLong(0x00000003); // constant
  buffer->PackUnsignedLong(0x00000000); // ???

}


CPacketTcp_Handshake_v7::CPacketTcp_Handshake_v7(CBuffer *inbuf)
{
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
}


CPacketTcp_Handshake_Ack::CPacketTcp_Handshake_Ack()
{
  m_nSize = 4;
  buffer = new CBuffer(4);
  buffer->PackUnsignedLong(1);
}

CPacketTcp_Handshake_Confirm::CPacketTcp_Handshake_Confirm(bool bIncoming)
{
  m_nSize = 33;
  buffer = new CBuffer(m_nSize);

  buffer->PackChar(0x03);
  buffer->PackUnsignedLong(0x0000000A);
  buffer->PackUnsignedLong(0x00000001);
  buffer->PackUnsignedLong(bIncoming ? 0x00000001 : 0x00000000);
  buffer->PackUnsignedLong(0x00000000);
  buffer->PackUnsignedLong(0x00000000);
  buffer->PackUnsignedLong(bIncoming ? 0x00040001 : 0x00000000);
  buffer->PackUnsignedLong(0x00000000);
  buffer->PackUnsignedLong(bIncoming ? 0x00000000 : 0x00040001);
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
  m_nVersion = user->ConnectionVersion();
  bool bHack = (m_nVersion >= 7 && 
               (user->LicqVersion() == 0 || user->LicqVersion() >= 1022));

  if (bHack)
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
      if (bHack)
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
    case 7:
    case 8:
      InitBuffer_v7();
      break;
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

void CPacketTcp::InitBuffer_v7()
{
  buffer = new CBuffer(m_nSize + 4);

  buffer->PackChar(0x02);
  buffer->PackUnsignedLong(0); // Checksum
  buffer->PackUnsignedShort(m_nCommand);
  buffer->PackUnsignedShort(0x000E); // ???
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedShort(m_nSubCommand);
  buffer->PackUnsignedShort(m_nStatus);
  buffer->PackUnsignedShort(m_nMsgType);
	//  buffer->PackUnsignedShort(0x0021);
  buffer->PackString(m_szMessage);

  m_szLocalPortOffset = NULL;
}

void CPacketTcp::PostBuffer_v7()
{
}


//-----Message------------------------------------------------------------------
CPT_Message::CPT_Message(char *_sMessage, unsigned short nLevel, bool bMR,
 CICQColor *pColor, ICQUser *pUser)
  : CPacketTcp(ICQ_CMDxTCP_START,
       ICQ_CMDxSUB_MSG | (bMR ? ICQ_CMDxSUB_FxMULTIREC : 0),
       _sMessage, true, nLevel, pUser)
{
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
   unsigned short nPort, unsigned short nLevel, ICQUser *pUser, bool bICBM)
  : CPacketTcp(ICQ_CMDxTCP_START, bICBM ? ICQ_CMDxSUB_ICBM : ICQ_CMDxSUB_CHAT, bICBM ? "" : _sMessage, true, nLevel, pUser)
{
  int nUsersLen = strlen_safe(szChatUsers);
  int nMessageLen = strlen_safe(_sMessage);

  if (bICBM)
    m_nSize += 58 + strlen_safe(szChatUsers) + strlen_safe(_sMessage) + 21;
  else
    m_nSize += 2 + strlen_safe(szChatUsers) + 1 + 8;

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

    buffer->PackUnsignedLong(nMessageLen + nUsersLen + 15);

    buffer->PackUnsignedLong(nMessageLen);

    if (nMessageLen)
      buffer->Pack(_sMessage, nMessageLen);

    buffer->PackString(szChatUsers);

    buffer->PackUnsignedShortBE(nPort);
    buffer->PackUnsignedShort(0);
    buffer->PackUnsignedShort(nPort);
    buffer->PackUnsignedShort(0);
  }
  else
  {
    buffer->PackString(szChatUsers);
    buffer->PackUnsignedLong(ReversePort(nPort));
    buffer->PackUnsignedLong(nPort);
  }

  PostBuffer();
}


//-----FileTransfer--------------------------------------------------------------
CPT_FileTransfer::CPT_FileTransfer(const char *_szFilename,
   const char *_szDescription, unsigned short nLevel, ICQUser *_cUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_FILE, _szDescription,
               true, nLevel, _cUser),
		CPX_FileTransfer(_szFilename)
{
	if (!m_bValid)  return;

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
  else if (m_nVersion >= 7)
	{
    buffer->PackUnsignedLong(0x00000000);
    buffer->PackUnsignedLong(0x00FFFFFF);
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
CPT_AckChatAccept::CPT_AckChatAccept(unsigned short _nPort, const char *szClients,
                                    unsigned long _nSequence, ICQUser *_cUser,
                                    bool bICBM)
  : CPT_Ack(bICBM ? ICQ_CMDxSUB_ICBM : ICQ_CMDxSUB_CHAT, _nSequence, true, true, _cUser)
{
  m_nPort = _nPort;
  m_nStatus = ICQ_TCPxACK_ONLINE;

  if (bICBM)
    m_nSize += 79 + strlen_safe(szClients);
  else
    m_nSize += 11 + strlen_safe(szClients);

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

    buffer->PackUnsignedLong(15 + strlen_safe(szClients));

    buffer->PackUnsignedLong(0);

    buffer->PackString(szClients);

    buffer->PackUnsignedShortBE(m_nPort);
    buffer->PackUnsignedShort(0);
    buffer->PackUnsignedShort(m_nPort);
    buffer->PackUnsignedShort(0);
  }
  else
  {
    buffer->PackString("");
    buffer->PackUnsignedLong(ReversePort(m_nPort));
    buffer->PackUnsignedLong(m_nPort);
  }

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


// Connection independent base classes

//-----FileTransfer------------------------------------------------------------
CPX_FileTransfer::CPX_FileTransfer(const char *_szFilename)
{
  m_bValid = true;
	m_szDesc = NULL;

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
}
