/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998-2011 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#include "config.h"

#include <boost/foreach.hpp>
#include <ctime>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include <cerrno>

#include <boost/scoped_array.hpp>

#include <licq/byteorder.h>
#include <licq/color.h>
#include <licq/contactlist/owner.h>
#include <licq/md5.h>
#include <licq/socket.h>
#include <licq/translator.h>
#include <licq/utility.h>
#include <licq/logging/log.h>
#include <licq/version.h>

#include "../contactlist/group.h"
#include "../contactlist/usermanager.h"
#include "../gettext.h"
#include "defines.h"
#include "icq.h"
#include "packet.h"

using namespace std;
using Licq::Group;
using Licq::StringList;
using Licq::UserCategoryMap;
using Licq::UserGroupList;
using Licq::UserId;
using Licq::gLog;
using Licq::gTranslator;
using LicqDaemon::GroupMap;
using LicqDaemon::gUserManager;

// TODO: Remove when no longer needed
typedef Licq::Owner ICQOwner;
typedef Licq::User ICQUser;


unsigned short ReversePort(unsigned short p)
{
  return ((p >> 8) & 0xFF) + ((p & 0xFF) << 8);
}

size_t lengthField(const string& field)
{
  // By SC 27434326
  // Auxiliary function for whitepage search
  //+ 7 byte for command (2),length field (4)
  // end string (1)

  size_t len = field.size();
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

static unsigned short login_fix [] = {
  5695, 23595, 23620, 23049, 0x2886, 0x2493, 23620, 23049, 2853, 17372, 1255, 1796, 1657, 13606, 1930, 23918, 31234, 30120, 0x1BEA, 0x5342, 0x30CC, 0x2294, 0x5697, 0x25FA, 0x3303, 0x078A, 0x0FC5, 0x25D6, 0x26EE,0x7570, 0x7F33, 0x4E94, 0x07C9, 0x7339, 0x42A8
};

void Encrypt_Server(CBuffer* /* buffer */)
{
#if ICQ_VERSION == 2
// No encryption in V2
#elif ICQ_VERSION == 4
  buffer->log(Log::Debug, tr("Unencrypted Packet"));

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

  buffer->log(Log::Debug, tr("Unencrypted Packet (%lu bytes)"), l);

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

const struct PluginList info_plugins[] =
{
  { "Picture"   , PLUGIN_PICTURE   , "Picture"                          },
  { "Phone Book", PLUGIN_PHONExBOOK, "Phone Book / Phone \"Follow Me\"" }
};

const struct PluginList status_plugins[] =
{
  {"Phone \"Follow Me\"", PLUGIN_FOLLOWxME, "Phone Book / Phone \"Follow Me\""},
  { "Shared Files Directory", PLUGIN_FILExSERVER, "Shared Files Directory" },
  { "ICQphone Status"       , PLUGIN_ICQxPHONE  , "ICQphone Status"        }
};

//======Server TCP============================================================
bool CSrvPacketTcp::s_bRegistered = false;
unsigned short CSrvPacketTcp::s_nSequence[32] = { 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
                                                  0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff };
unsigned short CSrvPacketTcp::s_nSubSequence = 0;
pthread_mutex_t CSrvPacketTcp::s_xMutex = PTHREAD_MUTEX_INITIALIZER;

CSrvPacketTcp::CSrvPacketTcp(unsigned char icqChannel)
  : myIcqChannel(icqChannel)
{
  pthread_mutex_lock(&s_xMutex);
  // will set m_nSequence later, in InitBuffer;
  m_nSubSequence = s_nSubSequence++;
  pthread_mutex_unlock(&s_xMutex);
  m_nFamily = m_nSubType = m_nSubCommand = m_nExtraInfo = 0;

  buffer = NULL;
  m_nSize = 0;
  m_nService = 0;
  m_szSequenceOffset = NULL;
}

CSrvPacketTcp::~CSrvPacketTcp()
{
  // Empty
}

CBuffer *CSrvPacketTcp::Finalize(Licq::INetSocket*)
{
  //  m_szSequenceOffset
  if (!getBuffer()) return new CBuffer;
  return new CBuffer(*getBuffer());
}

void CSrvPacketTcp::InitBuffer()
{
  pthread_mutex_lock(&s_xMutex);
  if (s_nSequence[m_nService] == 0xffff)
    s_nSequence[m_nService] = login_fix[ rand() % (sizeof(login_fix)/sizeof(login_fix[0])-1) ];
  m_nSequence = s_nSequence[m_nService]++;
  s_nSequence[m_nService] &= 0x7fff;
  pthread_mutex_unlock(&s_xMutex);

  buffer = new CBuffer(m_nSize+6);
  buffer->PackChar(0x2a);
  buffer->PackChar(myIcqChannel);
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

  pkt->log(Licq::Log::Debug, tr("Decrypted (ICQ) TCP Packet (%lu bytes)"), size);

  return true;
}


//=====UDP======================================================================
unsigned short CPacketUdp::s_nSequence = 0;
unsigned short CPacketUdp::s_nSubSequence = 0;
unsigned long  CPacketUdp::s_nSessionId = 0;
bool CPacketUdp::s_bRegistered = false;

CBuffer *CPacketUdp::Finalize(Licq::INetSocket*)
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
  // Empty
}

void CPacketUdp::InitBuffer()
{
  buffer = new CBuffer(m_nSize + 8);
#if ICQ_VERSION == 2
  buffer->PackUnsignedShort(m_nVersion);
  buffer->PackUnsignedShort(m_nCommand);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
#elif ICQ_VERSION == 4
  buffer->add(m_nVersion);
  buffer->add(m_nRandom);
  buffer->add(m_nZero);
  buffer->add(m_nCommand);
  buffer->add(m_nSequence);
  buffer->add(m_nSubSequence);
  buffer->add(gUserManager.icqOwnerUin());
  buffer->add(m_nCheckSum);
#elif ICQ_VERSION == 5
  buffer->PackUnsignedShort(m_nVersion);
  buffer->PackUnsignedLong(m_nZero);
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedLong(m_nSessionId);
  buffer->PackUnsignedShort(m_nCommand);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedShort(m_nSubSequence);
  buffer->PackUnsignedLong(m_nCheckSum);
#endif
}

//-----Register----------------------------------------------------------------
#if ICQ_VERSION == 2 || ICQ_VERSION == 6
CPU_Register::CPU_Register(const string& password)
{
  m_nVersion = ICQ_VERSION;
  m_nCommand = ICQ_CMDxSND_REGISTERxUSER;
  m_nSequence = 0x001;
  m_nPasswdLen = password.size() + 1;
  myPassword = password;

  m_nSize = myPassword.size() + 1 + 18;
  buffer = new CBuffer(m_nSize);

  buffer->PackUnsignedShort(m_nVersion);
  buffer->PackUnsignedShort(m_nCommand);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedShort(0x02);
  buffer->packString(myPassword);
  buffer->PackUnsignedShort(0x72);
  buffer->PackUnsignedShort(0x00);
}

CPU_Register::~CPU_Register()
{
  // Empty
}

#elif ICQ_VERSION == 4 || ICQ_VERSION == 5

CPU_Register::CPU_Register(const string& password)
  : CPacketUdp(ICQ_CMDxSND_REGISTERxUSER)
{
  m_nSize += password.size() + 1 + 20;

#if ICQ_VERSION == 5
  m_nSessionId = s_nSessionId = rand() & 0x3FFFFFFF;
#endif
  s_nSequence = rand() & 0x7FFF;
  m_nSequence = s_nSequence++;
  m_nSubSequence = s_nSubSequence = 1;
  s_bRegistered = true;

  InitBuffer();

  buffer->packString(password);
  buffer->PackUnsignedLong(0x000000A0);
  buffer->PackUnsignedLong(0x00002461);
  buffer->PackUnsignedLong(0x00A00000);
  buffer->PackUnsignedLong(0x00000000);
}

CPU_Register::~CPU_Register()
{
  // Empty
}

#elif ICQ_VERSION >= 7

CPU_RegisterFirst::CPU_RegisterFirst()
	: CSrvPacketTcp(ICQ_CHNxNEW)
{
  m_nSize = 4;

  pthread_mutex_lock(&s_xMutex);
  s_nSequence[m_nService] = 0xffff;
  s_bRegistered = true;
  pthread_mutex_unlock(&s_xMutex);

  InitBuffer();

  buffer->PackUnsignedLongBE(1);
}

CPU_RegisterFirst::~CPU_RegisterFirst()
{
  // Empty
}

CPU_Register::CPU_Register(const string& password)
	: CPU_CommonFamily(ICQ_SNACxFAM_AUTH, ICQ_SNACxREGISTER_USER)
{
  int nPassLen = password.size();
  m_nSize += 55 + nPassLen;

  InitBuffer();

  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackUnsignedShortBE(nPassLen+51);
  buffer->PackUnsignedLongBE(0x00000000);
  buffer->PackUnsignedLongBE(0x28000000);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);//x82270000);
  buffer->PackUnsignedLongBE(0);//x82270000);
  for (int x = 0; x < 4; x++) buffer->PackUnsignedLongBE(0);
  buffer->PackLNTS(password.c_str());
  buffer->PackUnsignedLongBE(0);//x82270000);
  buffer->PackUnsignedLongBE(0xf2070000);
}

CPU_Register::~CPU_Register()
{
  // Empty
}

#endif

CPU_VerifyRegistration::CPU_VerifyRegistration()
  : CPU_CommonFamily(ICQ_SNACxFAM_AUTH, ICQ_SNACxREQUEST_IMAGE)
{
  // Yes, it's empty
  
  InitBuffer();
}

CPU_VerifyRegistration::~CPU_VerifyRegistration()
{
  // Empty
}

CPU_SendVerification::CPU_SendVerification(const string& password, const string& verification)
  : CPU_CommonFamily(ICQ_SNACxFAM_AUTH, ICQ_SNACxREGISTER_USER)
{
  m_nSize += 55 + password.size() + verification.size() + 4;

  InitBuffer();

  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackUnsignedShortBE(password.size()+51);
  buffer->PackUnsignedLongBE(0x00000000);
  buffer->PackUnsignedLongBE(0x28000000);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);//x82270000);
  buffer->PackUnsignedLongBE(0);//x82270000);
  for (int x = 0; x < 4; x++) buffer->PackUnsignedLongBE(0);
  buffer->PackLNTS(password.c_str());
  buffer->PackUnsignedLongBE(0);//x82270000);
  buffer->PackUnsignedLongBE(0xf2070000);
  // Verification TLV
  buffer->PackUnsignedShortBE(0x0009);
  buffer->PackUnsignedShortBE(verification.size());
  buffer->pack(verification);
}

CPU_SendVerification::~CPU_SendVerification()
{
  // Empty
}

CPU_ConnectStart::CPU_ConnectStart()
  : CSrvPacketTcp(ICQ_CHNxNEW)
{
  pthread_mutex_lock(&s_xMutex);
  if (!s_bRegistered) {
    s_nSequence[m_nService] = 0xffff;
    s_bRegistered = true;
  }
  pthread_mutex_unlock(&s_xMutex);

  m_nSize = 12;
  InitBuffer();

  buffer->PackUnsignedLongBE(0x00000001);
  buffer->PackUnsignedLongBE(0x80030004);
  buffer->PackUnsignedLongBE(0x00100000);
}

CPU_RequestLogonSalt::CPU_RequestLogonSalt(const std::string &id)
  : CPU_CommonFamily(ICQ_SNACxFAM_AUTH, ICQ_SNACxAUTHxREQUEST_SALT)
{
  m_nSize += id.size() + 4;
  InitBuffer();

  buffer->PackTLV(0x0001, id.size(), id.c_str());
}

//-----NewLogon-----------------------------------------------------------------
CPU_NewLogon::CPU_NewLogon(const string& password, const string& accountId, const string& md5Salt)
  : CPU_CommonFamily(ICQ_SNACxFAM_AUTH, ICQ_SNACxAUTHxLOGON)
{
  // truncate password to MAX 8 characters
  string pass(password);
  if (pass.size() > 8)
  {
    gLog.warning(tr("Password too long, truncated to 8 Characters!"));
    pass.erase(8);
  }

  string toHash(md5Salt);
  toHash += pass;
  toHash += "AOL Instant Messenger (SM)";
  unsigned char szDigest[MD5_DIGEST_LENGTH];
  Licq::md5((const unsigned char*)toHash.c_str(), toHash.size(), szDigest);

  m_nSize += accountId.size() + MD5_DIGEST_LENGTH + 70;
  InitBuffer();

  buffer->PackTLV(0x0001, accountId.size(), accountId.c_str());
  buffer->PackTLV(0x0025, MD5_DIGEST_LENGTH, reinterpret_cast<char *>(szDigest));

  buffer->PackTLV(0x0003, 0x0008, "ICQBasic");

  // Static versioning
  buffer->PackUnsignedLongBE(0x00160002);
  buffer->PackUnsignedShortBE(0x010B);
  // Client version major (4 == ICQ2000, 5 == ICQ2001)
  buffer->PackUnsignedLongBE(0x00170002);
  buffer->PackUnsignedShortBE(0x0014);
  // Client version minor
  buffer->PackUnsignedLongBE(0x00180002);
  buffer->PackUnsignedShortBE(0x0022);
  buffer->PackUnsignedLongBE(0x00190002);
  buffer->PackUnsignedShortBE(0x0000);
  // Client version build
  buffer->PackUnsignedLongBE(0x001a0002);
  buffer->PackUnsignedShortBE(0x0BB8);
  buffer->PackUnsignedLongBE(0x00140004);
  buffer->PackUnsignedLongBE(0x0000043D);

  // locale info, just use english, usa for now, i don't know what else they use
  buffer->PackTLV(0x000f, 0x0002, "en");
  buffer->PackTLV(0x000e, 0x0002, "us");
}

//-----Logon--------------------------------------------------------------------
CPU_Logon::CPU_Logon(const string& password, const string& accountId, unsigned short _nLogonStatus)
  : CSrvPacketTcp(ICQ_CHNxNEW)
{
  // truncate password to MAX 8 characters
  string pass(password);
  if (pass.size() > 8)
  {
    gLog.warning(tr("Password too long, truncated to 8 Characters!"));
    pass.erase(8);
  }

  char szEncPass[16];
  unsigned int j;

  pthread_mutex_lock(&s_xMutex);
  if (!s_bRegistered) {
    s_nSequence[m_nService] = 0xffff;
    s_bRegistered = true;
  }
  pthread_mutex_unlock(&s_xMutex);

  m_nLogonStatus = _nLogonStatus;
  m_nTcpVersion = ICQ_VERSION_TCP;

  unsigned int pwlen = pass.size();

  m_nSize = accountId.size() + pwlen + 74;
  InitBuffer();

  // Encrypt our password here
  unsigned char xor_table[] = { 0xf3, 0x26, 0x81, 0xc4, 0x39, 0x86, 0xdb, 0x92,
                           0x71, 0xa3, 0xb9, 0xe6, 0x53, 0x7a, 0x95, 0x7c};
  for (j = 0; j < pwlen; j++)
    szEncPass[j] = (pass[j] ^ xor_table[j]);
  szEncPass[j] = 0;

  buffer->PackUnsignedLongBE(0x00000001);
  buffer->PackTLV(0x0001, accountId.size(), accountId.c_str());
  buffer->PackTLV(0x0002, pwlen, szEncPass);
  buffer->PackTLV(0x0003,  0x0008, "ICQBasic");

  // Static versioning
  buffer->PackUnsignedLongBE(0x00160002);
  buffer->PackUnsignedShortBE(0x010B);
  // Client version major (4 == ICQ2000, 5 == ICQ2001)
  buffer->PackUnsignedLongBE(0x00170002);
  buffer->PackUnsignedShortBE(0x0014);
  // Client version minor
  buffer->PackUnsignedLongBE(0x00180002);
  buffer->PackUnsignedShortBE(0x0022);
  buffer->PackUnsignedLongBE(0x00190002);
  buffer->PackUnsignedShortBE(0x0000);
  // Client version build
  buffer->PackUnsignedLongBE(0x001a0002);
  buffer->PackUnsignedShortBE(0x0BB8);
  buffer->PackUnsignedLongBE(0x00140004);
  buffer->PackUnsignedLongBE(0x0000043D);

  // locale info, just use english, usa for now, i don't know what else they use
  buffer->PackTLV(0x000f, 0x0002, "en");
  buffer->PackTLV(0x000e, 0x0002, "us");
}

CPU_Logon::~CPU_Logon()
{
  // Empty
}

//-----SendCookie------------------------------------------------------------
CPU_SendCookie::CPU_SendCookie(const string& cookie, unsigned short nService)
  : CSrvPacketTcp(ICQ_CHNxNEW)
{
  m_nService = nService;
  m_nSize = cookie.size() + 8;
  pthread_mutex_lock(&s_xMutex);
  s_nSequence[m_nService] = 0xffff;
  pthread_mutex_unlock(&s_xMutex);
  InitBuffer();

  buffer->PackUnsignedLongBE(0x00000001);
  buffer->PackTLV(0x0006, cookie.size(), cookie.c_str());
}

CPU_SendCookie::~CPU_SendCookie()
{
  // Empty
}

//-----ListRequestRights-----------------------------------------------------
CPU_ListRequestRights::CPU_ListRequestRights()
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_REQUESTxRIGHTS)
{
  m_nSize += 6;
  InitBuffer();

  buffer->PackUnsignedLongBE(0x000b0002);
  buffer->PackUnsignedShortBE(0x000f);
}

//-----ImICQ-----------------------------------------------------------------
CPU_ImICQ::CPU_ImICQ()
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_IMxICQ)
{
  m_nSize += 40;
  InitBuffer();

  // setting communication parameters (key / value pair) ?
  // seems to say which channels should be enabled
  buffer->PackUnsignedLongBE(0x00010004);
  buffer->PackUnsignedLongBE(0x00130004);
  buffer->PackUnsignedLongBE(0x00020001);
  buffer->PackUnsignedLongBE(0x00030001);
  buffer->PackUnsignedLongBE(0x00150001);
  buffer->PackUnsignedLongBE(0x00040001);
  buffer->PackUnsignedLongBE(0x00060001);
  buffer->PackUnsignedLongBE(0x00090001);
  buffer->PackUnsignedLongBE(0x000a0001);
  buffer->PackUnsignedLongBE(0x000b0001);
}

CPU_ImICQ::CPU_ImICQ(unsigned short VerArray[][2], unsigned short NumVer,
                     unsigned short nService)
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_IMxICQ)
{
  m_nService = nService;
  m_nSize += NumVer * 4;
  InitBuffer();
  
  for (int i = 0; i < NumVer; i++)
  {
    buffer->PackUnsignedShortBE(VerArray[i][0]);
    buffer->PackUnsignedShortBE(VerArray[i][1]);
  }
}

//-----ImICQMode-------------------------------------------------------------
CPU_ICQMode::CPU_ICQMode(unsigned short channel, unsigned long flags)
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SETxICQxMODE)
{
  m_nSize += 16;

  InitBuffer();

  /* channel 0 sets default options for channels we did not specifically
     initialize */
  buffer->PackUnsignedShortBE(channel);
  buffer->PackUnsignedLongBE(flags);   // the meanings of the flags are unknown
  buffer->PackUnsignedShortBE(8000);      // max message size
  buffer->PackUnsignedShortBE(999);       // max sender warning level
  buffer->PackUnsignedShortBE(999);       // max receiver warning level
  buffer->PackUnsignedShortBE(0);         // min message interval
  buffer->PackUnsignedShortBE(0);         // unknown
}

//-----RateAck-----------------------------------------------------------------
CPU_RateAck::CPU_RateAck(unsigned short nService)
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSND_RATE_ACK)
{
  m_nService = nService;
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
  char data[8][CAP_LENGTH];
  m_nSize += 4 + sizeof(data);
  InitBuffer();

  memcpy(data[0], ICQ_CAPABILITY_DIRECT, CAP_LENGTH);
  memcpy(data[1], ICQ_CAPABILITY_SRVxRELAY, CAP_LENGTH);
  memcpy(data[2], ICQ_CAPABILITY_TYPING, CAP_LENGTH);
  memcpy(data[3], ICQ_CAPABILITY_LICQxVER, CAP_LENGTH);
  memcpy(data[4], ICQ_CAPABILITY_AIMxINTER, CAP_LENGTH);
  memcpy(data[5], ICQ_CAPABILITY_RTFxMSGS, CAP_LENGTH);
  memcpy(data[6], ICQ_CAPABILITY_ICHAT, CAP_LENGTH);
  memcpy(data[7], ICQ_CAPABILITY_BART, CAP_LENGTH);

  // Send our licq version
  data[3][12] = Licq::extractMajorVersion(LICQ_VERSION);
  data[3][13] = Licq::extractMinorVersion(LICQ_VERSION);
  data[3][14] = Licq::extractReleaseVersion(LICQ_VERSION);
#ifdef USE_OPENSSL
  // If we support SSL
  data[3][15] = 1;
#endif
  buffer->PackTLV(0x05, sizeof(data), (char *)data);  
}

//-----RequestBuddyIcon---------------------------------------------------------
CPU_RequestBuddyIcon::CPU_RequestBuddyIcon(const string& accountId,
                      unsigned short _nBuddyIconType, char _nBuddyIconHashType,
      const string& buddyIconHash, unsigned short nService)
  : CPU_CommonFamily(ICQ_SNACxFAM_BART, ICQ_SNACxBART_DOWNLOADxREQUEST)
{
  m_nService = nService;
  m_nSize += 6 + accountId.size() + buddyIconHash.size();

  InitBuffer();

  buffer->PackChar(accountId.size());
  buffer->pack(accountId);
  buffer->PackChar(0x01);	// number of hashes being requested in this packet
  buffer->PackUnsignedShortBE(_nBuddyIconType);
  buffer->PackChar(_nBuddyIconHashType);
  buffer->PackChar(buddyIconHash.size());
  buffer->pack(buddyIconHash);
}

//-----RequestService-----------------------------------------------------------
CPU_RequestService::CPU_RequestService(unsigned short nFam)
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSUB_NEW_SERVICE)
{
  m_nSize += 2;

  InitBuffer();

  buffer->PackUnsignedShortBE(nFam);
}

//-----SetPrivacy---------------------------------------------------------------
CPU_SetPrivacy::CPU_SetPrivacy(unsigned char _cPrivacy)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxUPD_GROUP)
{
  m_nSize += 15;

  InitBuffer();

  unsigned short nPDINFO;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    nPDINFO = o->GetPDINFO();
  }

  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedShortBE(nPDINFO);
  buffer->PackUnsignedLongBE(0x00040005);
  buffer->PackUnsignedLongBE(0x00CA0001); // Privacy TLV
  buffer->PackChar(_cPrivacy);
}

//-----SetStatus----------------------------------------------------------------
CPU_SetStatus::CPU_SetStatus(unsigned long _nNewStatus)
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS)
{
  m_nNewStatus = _nNewStatus;

  bool bInvis = _nNewStatus & ICQ_STATUS_FxPRIVATE;

  m_nSize += 8;
  if (!bInvis)
    m_nSize += 47;

  InitBuffer();

  //ICQ 4.0 doesn't like this
  m_nNewStatus &= ~(ICQ_STATUS_FxPFM | ICQ_STATUS_FxPFMxAVAILABLE);

  buffer->PackUnsignedLongBE(0x00060004);     // TLV
  buffer->PackUnsignedLongBE(m_nNewStatus);  // ICQ status
  if (!bInvis)
  {
    buffer->PackUnsignedLongBE(0x000C0025); // TLV
    buffer->PackUnsignedLong(s_nLocalIp);    // direct connection info
    buffer->PackUnsignedLongBE(s_nLocalPort);
    buffer->PackChar(gIcqProtocol.directMode() ? MODE_DIRECT : MODE_INDIRECT);
    buffer->PackUnsignedShortBE(ICQ_VERSION_TCP);
    buffer->PackUnsignedLongBE(0x00000000);    // local direction conn cookie
    buffer->PackUnsignedLongBE(0x00000050);
    buffer->PackUnsignedLongBE(0x00000003);
    buffer->PackUnsignedLongBE(0);
    // some kind of timestamp ?
    buffer->PackUnsignedLongBE(0);
    buffer->PackUnsignedLongBE(0);
    buffer->PackUnsignedShort(0);

    buffer->PackUnsignedLongBE(0x00080002); // TLV
    buffer->PackUnsignedShort(0); // Error code
  }
}

CPU_SetStatusFamily::CPU_SetStatusFamily()
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS)
{
  m_nSize += 55;
}

void CPU_SetStatusFamily::InitBuffer()
{
  CPU_CommonFamily::InitBuffer();

  //ICQ 4.0 doesn't like this
  m_nNewStatus &= ~(ICQ_STATUS_FxPFM | ICQ_STATUS_FxPFMxAVAILABLE);

  buffer->PackUnsignedLongBE(0x00060004);     // TLV
  buffer->PackUnsignedLongBE(m_nNewStatus);  // ICQ status
  buffer->PackUnsignedLongBE(0x00080002);    // TLV
  buffer->PackUnsignedShortBE(0);            // error code ?
  buffer->PackUnsignedLongBE(0x000c0025);    // TLV
  buffer->PackUnsignedLong(s_nLocalIp);      // direct connection info
  buffer->PackUnsignedLongBE(s_nLocalPort);
  buffer->PackChar(gIcqProtocol.directMode() ? MODE_DIRECT : MODE_INDIRECT);
  buffer->PackUnsignedShortBE(ICQ_VERSION_TCP);
  buffer->PackUnsignedLongBE(0x00000000);    // local direction conn cookie
  buffer->PackUnsignedLongBE(0x00000050);
  buffer->PackUnsignedLongBE(0x00000003);

  // build date of the core DLL of Mirabilis ICQ
  // we use that to tell other users that we're Licq
  // ICQ99b:  0x385BFAAC;
  
  /* should eventually switch to some other way to identify licq, probably
     use capabilities */
  Licq::OwnerReadGuard o(LICQ_PPID);
#ifdef USE_OPENSSL
   buffer->PackUnsignedLongBE(LICQ_WITHSSL | LICQ_VERSION);
#else
   buffer->PackUnsignedLongBE(LICQ_WITHOUTSSL | LICQ_VERSION);
#endif
   // some kind of timestamp ?
  buffer->PackUnsignedLongBE(o->ClientStatusTimestamp());
  buffer->PackUnsignedLongBE(o->ClientInfoTimestamp());
  buffer->PackUnsignedShortBE(0x0000);
}

CPU_SetLogonStatus::CPU_SetLogonStatus(unsigned long _nNewStatus)
  : CPU_SetStatusFamily()
{
  m_nNewStatus = _nNewStatus;

  InitBuffer();
}

CPU_UpdateInfoTimestamp::CPU_UpdateInfoTimestamp(const char *GUID)
  : CPU_SetStatusFamily()
{
  unsigned long timestamp;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    m_nNewStatus = IcqProtocol::addStatusFlags(IcqProtocol::icqStatusFromStatus(o->status()), *o);
    timestamp = o->ClientInfoTimestamp();
  }

  m_nSize += 4 + 1 + 4 + 6 + GUID_LENGTH + 4 + 1;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x00110022); // TLV
  buffer->PackChar(2);                    // info update
  buffer->PackUnsignedLong(timestamp);
  buffer->PackUnsignedShort(2);           //Unknown
  buffer->PackUnsignedShort(1);           //Unknown
  buffer->PackUnsignedShort(1);           //Unknown
  buffer->Pack(GUID, GUID_LENGTH);
  buffer->PackUnsignedLong(timestamp);
  buffer->PackChar(0);                    //No info follows ??
}

CPU_UpdateStatusTimestamp::CPU_UpdateStatusTimestamp(const char *GUID,
                                                     unsigned long nState,
                                                     unsigned long nStatus)
  : CPU_SetStatusFamily()
{
  unsigned long clientTime;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    m_nNewStatus = nStatus != ICQ_STATUS_OFFLINE ? nStatus :
        IcqProtocol::addStatusFlags(IcqProtocol::icqStatusFromStatus(o->status()), *o);
    clientTime = o->ClientStatusTimestamp();
  }

  m_nSize += 4 + 1 + 4 + 6 + GUID_LENGTH + 1 + 4 + 4 + 6;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x0011002C); // TLV
  buffer->PackChar(3);                    // info update
  buffer->PackUnsignedLong(clientTime);
  buffer->PackUnsignedShort(0);           //Unknown
  buffer->PackUnsignedShort(1);           //Unknown
  buffer->PackUnsignedShort(1);           //Unknown
  buffer->Pack(GUID, GUID_LENGTH);
  buffer->PackChar(1);                    //Unknown
  buffer->PackUnsignedLong(nState);
  buffer->PackUnsignedLong(clientTime);
  buffer->PackUnsignedShort(0);           //Unknown
  buffer->PackUnsignedShort(0);           //Unknown
  buffer->PackUnsignedShort(1);           //Unknown
}

CPU_UpdateTimestamp::CPU_UpdateTimestamp()
  : CPU_SetStatusFamily()
{
  Licq::OwnerReadGuard o(LICQ_PPID);
  m_nNewStatus = IcqProtocol::addStatusFlags(IcqProtocol::icqStatusFromStatus(o->status()), *o);

  m_nSize += 4 + 1 + 4;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x00110005);         // TLV
  buffer->PackChar(0);                            // server info update
  buffer->PackUnsignedLong(o->ClientTimestamp());
}

//-----GenericUinList-----------------------------------------------------------
CPU_GenericUinList::CPU_GenericUinList(const StringList& users, unsigned short family, unsigned short Subtype)
  : CPU_CommonFamily(family, Subtype)
{
  char len[2];
  len[1] = '\0';
  int nLen = 0;
  StringList::const_iterator it;
  for (it = users.begin(); it != users.end(); ++it)
    nLen += it->size() + 1;

  string contacts;

  for (it = users.begin(); it != users.end(); ++it) {
    len[0] = it->size();
    contacts += len;
    contacts += *it;
  }

  m_nSize += contacts.size();
  InitBuffer();

  buffer->pack(contacts);
}

CPU_GenericUinList::CPU_GenericUinList(const string& accountId, unsigned short family, unsigned short Subtype)
  : CPU_CommonFamily(family, Subtype)
{
  m_nSize += accountId.size()+1;
  InitBuffer();

  buffer->PackChar(accountId.size());
  buffer->pack(accountId);
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

CPU_GenericFamily::CPU_GenericFamily(unsigned short Family, unsigned short SubType,
                                     unsigned short nService)
  : CPU_CommonFamily(Family, SubType)
{
  m_nService = nService;
  m_nSize += 0;
  InitBuffer();
}

CPU_GenericFamily::~CPU_GenericFamily()
{
  // Empty
}

CPU_CommonFamily::CPU_CommonFamily(unsigned short Family, unsigned short SubType)
  : CSrvPacketTcp(ICQ_CHNxDATA)
{
  m_nSize += 10;

  m_nFamily = Family;
  m_nSubType = SubType;
}

CPU_CommonFamily::~CPU_CommonFamily()
{
  // Empty
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
  m_nSize += 80;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x00010004);
  buffer->PackUnsignedLongBE(0x011008e4);
  buffer->PackUnsignedLongBE(0x00020001);
  buffer->PackUnsignedLongBE(0x011008e4);
  buffer->PackUnsignedLongBE(0x00030001);
  buffer->PackUnsignedLongBE(0x011008e4);
  buffer->PackUnsignedLongBE(0x00150001);
  buffer->PackUnsignedLongBE(0x011008e4);
  buffer->PackUnsignedLongBE(0x00040001);
  buffer->PackUnsignedLongBE(0x011008e4);
  buffer->PackUnsignedLongBE(0x00060001);
  buffer->PackUnsignedLongBE(0x011008e4);
  buffer->PackUnsignedLongBE(0x00090001);
  buffer->PackUnsignedLongBE(0x011008e4);
  buffer->PackUnsignedLongBE(0x000a0001);
  buffer->PackUnsignedLongBE(0x011008e4);
  buffer->PackUnsignedLongBE(0x00130004);
  buffer->PackUnsignedLongBE(0x011008e4);
  buffer->PackUnsignedLongBE(0x000b0004);
  buffer->PackUnsignedLongBE(0x011008e4);
#endif
}

CPU_ClientReady::CPU_ClientReady(unsigned short VerArray[][4], unsigned short NumVer,
                                 unsigned short nService)
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSUB_READYxCLIENT)
{
  m_nService = nService;
  m_nSize += NumVer * 8;
  InitBuffer();
  
  for (int i = 0; i < NumVer; i++)
  {
    buffer->PackUnsignedShortBE(VerArray[i][0]);
    buffer->PackUnsignedShortBE(VerArray[i][1]);
    buffer->PackUnsignedShortBE(VerArray[i][2]);
    buffer->PackUnsignedShortBE(VerArray[i][3]);
  }
}

CPU_AckNameInfo::CPU_AckNameInfo()
  : CPU_CommonFamily(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSND_NAMExINFOxACK)
{
  m_nSize += 4;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x00000000);
}

//-----TypingNotification--------------------------------------------------
CPU_TypingNotification::CPU_TypingNotification(const string& accountId, bool bActive)
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_TYPING)
{
  unsigned short nTyping = bActive ? ICQ_TYPING_ACTIVE : ICQ_TYPING_INACTIVEx0;
  m_nSize += 13 + accountId.size();

  InitBuffer();

  buffer->PackUnsignedLongBE(0x00000000);
  buffer->PackUnsignedLongBE(0x00000000);
  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackChar(accountId.size());
  buffer->pack(accountId);
  buffer->PackUnsignedShortBE(nTyping);
}

//-----CheckInvisible------------------------------------------------------
CPU_CheckInvisible::CPU_CheckInvisible(const string& accountId)
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER)
{
  m_nSize += 15 + accountId.size();

  InitBuffer();
  
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE((unsigned long)m_nSequence);
  buffer->PackUnsignedShortBE(0x0002);
  buffer->PackChar(accountId.size());
  buffer->pack(accountId);
  buffer->PackUnsignedLongBE(0x00060000);
}

//-----ThroughServer-------------------------------------------------------
CPU_ThroughServer::CPU_ThroughServer(const string& accountId,
    unsigned char msgType, const string& message, unsigned short nCharset, bool bOffline)
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER)
{
	m_nSubCommand = msgType;

  int msgLen = message.size();
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
      nTypeLen = msgLen = 0;
      gLog.warning(tr("Command not implemented yet (%04X)."), msgType);
      return;
  }

  m_nSize += 11 + nTypeLen + accountId.size() + 4; // 11 all bytes pre-tlv
	//  8 fom tlv type, tlv len
  if (bOffline)
    m_nSize += 4;

	InitBuffer();

	buffer->PackUnsignedLongBE(0); // upper 4 bytes of message id
	buffer->PackUnsignedLongBE(0); // lower 4 bytes of message id
	buffer->PackUnsignedShortBE(nFormat); // message format
  buffer->PackChar(accountId.size());
  buffer->pack(accountId);

	tlvData.Create(nTypeLen);

	switch (nFormat)
	{
	case 1:
 		nTLVType = 0x02;

 		tlvData.PackUnsignedLongBE(0x05010001);
		tlvData.PackUnsignedShortBE(0x0101);
 		tlvData.PackChar(0x01);
		tlvData.PackUnsignedShortBE(msgLen + 4);
                tlvData.PackUnsignedShortBE(nCharset);
 		tlvData.PackUnsignedShortBE(0);
      tlvData.Pack(message.c_str(), msgLen);
      break;

	case 4:
 		nTLVType = 0x05;

      tlvData.PackUnsignedLong(gUserManager.icqOwnerUin());
		tlvData.PackChar(msgType);
		tlvData.PackChar(0); // message flags
      tlvData.PackLNTS(message.c_str());
      break;
  }

	buffer->PackTLV(nTLVType, nTypeLen, &tlvData);
        if (bOffline)
	  buffer->PackUnsignedLongBE(0x00060000); // tlv type: 6, tlv len: 0
}

//-----Type2Message-------------------------------------------------------------
CPU_Type2Message::CPU_Type2Message(const ICQUser* u, bool _bAck, bool _bDirectInfo,
                                   const char *cap,
                                   unsigned long nMsgID1,
                                   unsigned long nMsgID2)
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER)
{
  int nUinLen = u->accountId().size();
        unsigned short nDirectInfo = _bDirectInfo ? 14 : 0; //size of di

	m_nSize += 55 + nUinLen + nDirectInfo;

	m_pUser = u;
        m_bAck = _bAck;
        m_bDirectInfo = _bDirectInfo;
	m_nMsgID[0] = nMsgID1;
	m_nMsgID[1] = nMsgID2;

  memcpy(m_cap, cap, CAP_LENGTH);
  m_nExtraLen = 0;
}

void CPU_Type2Message::InitBuffer()
{
	CPU_CommonFamily::InitBuffer();

  Licq::OwnerReadGuard o(LICQ_PPID);

  int nUinLen = m_pUser->accountId().size();

	unsigned long nID1, nID2;
  unsigned short nDirectInfo = m_bDirectInfo ? 14 : 0; // size of direct info

	if (m_bAck)
	{
		nID1 = m_nMsgID[0];
		nID2 = m_nMsgID[1];
	}
	else
	{
		nID1 = 0;
		nID2 = m_nSubSequence;
	}

	buffer->PackUnsignedLongBE(nID1); // upper 4 bytes of message id
	buffer->PackUnsignedLongBE(nID2); // lower 4 bytes of message id
	buffer->PackUnsignedShortBE(0x02); // message format
	buffer->PackChar(nUinLen);
  buffer->pack(m_pUser->accountId());

	buffer->PackUnsignedShortBE(0x0005);	// tlv - message info
	buffer->PackUnsignedShortBE(m_nSize - 25 - nUinLen - m_nExtraLen);
	buffer->PackUnsignedShortBE(0);
	buffer->PackUnsignedLongBE(nID1);	// upper 4 bytes of message id again
	buffer->PackUnsignedLongBE(nID2); // lower 4 bytes of message id again
	buffer->Pack(m_cap, CAP_LENGTH);
        buffer->PackUnsignedLongBE(0x000A0002); // tlv - ack or not
	buffer->PackUnsignedShortBE((m_bAck ? 2 : 1));

  buffer->PackUnsignedLongBE(0x000F0000); // tlv - empty

  if (nDirectInfo)
  {
    buffer->PackUnsignedLongBE(0x00030004); // tlv - internal ip
    buffer->PackUnsignedLong(o->IntIp());
    buffer->PackUnsignedLongBE(0x00050002); // tlv - listening port
    buffer->PackUnsignedShort(o->Port());
  }
	buffer->PackUnsignedShortBE(0x2711); // tlv - more message info
	buffer->PackUnsignedShortBE(m_nSize - 29 - nUinLen - 36 - nDirectInfo -
          m_nExtraLen);
}

//-----ReverseConnect-----------------------------------------------------------
CPU_ReverseConnect::CPU_ReverseConnect(const ICQUser* u, unsigned long nLocalIP,
                                       unsigned short nLocalPort,
                                       unsigned short nRemotePort)
  : CPU_Type2Message(u, false, false, ICQ_CAPABILITY_DIRECT)
{
  m_nSize += 4 + 4 + 4 + 1 + 4 + 4 + 2 + 4;
//  m_nExtraLen += 4; m_nSize += 4;

  CPU_Type2Message::InitBuffer();

  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedLong(nLocalIP);
  buffer->PackUnsignedLong(nLocalPort);
  buffer->PackChar(MODE_DIRECT); /* why would you set it to anything else?
                                    if you do icq just sends you back the
                                    same ip and port number to connect to */
  buffer->PackUnsignedLong(nRemotePort);
  buffer->PackUnsignedLong(nLocalPort);
  buffer->PackUnsignedShort(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(m_nSubSequence);
//  buffer->PackUnsignedLongBE(0x00030000);
}

CPU_ReverseConnectFailed::CPU_ReverseConnectFailed(const string& accountId,
                                                   unsigned long nMsgID1,
                                                   unsigned long nMsgID2,
                                                   unsigned short nFailedPort,
                                                   unsigned short nOurPort,
                                                   unsigned long nConnectID)
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SERVERxREPLYxMSG)
{
  m_nSize += 8 + 2 + 1 + accountId.size() + 2 + 4 + 4 + 4 + 2 + 4;

  InitBuffer();

  buffer->PackUnsignedLongBE(nMsgID1);
  buffer->PackUnsignedLongBE(nMsgID2);
  buffer->PackUnsignedShortBE(0x0002);
  buffer->PackChar(accountId.size());
  buffer->pack(accountId);
  buffer->PackUnsignedShortBE(0x0003);
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedLong(nFailedPort);
  buffer->PackUnsignedLong(nOurPort);
  buffer->PackUnsignedShort(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(nConnectID);
}

//-----PluginMessage-----------------------------------------------------------
CPU_PluginMessage::CPU_PluginMessage(const ICQUser* u, bool bAck,
                                     const char *PluginGUID,
                                     unsigned long nMsgID1,
                                     unsigned long nMsgID2)
  : CPU_Type2Message(u, bAck, false, ICQ_CAPABILITY_SRVxRELAY, nMsgID1, nMsgID2)
{
  m_nSize += 49;

  memcpy(m_PluginGUID, PluginGUID, GUID_LENGTH);
}

void CPU_PluginMessage::InitBuffer()
{
  CPU_Type2Message::InitBuffer();

	buffer->PackUnsignedShort(0x001B); // len
	buffer->PackUnsignedShort(m_pUser->ConnectionVersion());
  buffer->Pack(m_PluginGUID, GUID_LENGTH);
  buffer->PackUnsignedLong(0x00000003); //unknown
  buffer->PackChar(0);  //unknown
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedShort(0x0012);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedShort(m_bAck ? 0 : 1);
  buffer->PackUnsignedShort(0);
}

//-----Send error reply------------------------------------------------------
CPU_PluginError::CPU_PluginError(const ICQUser* u, unsigned long nMsgID1,
                                 unsigned long nMsgID2,
                                 unsigned short nSequence, const char *cap)
  : CPU_AckThroughServer(u, nMsgID1, nMsgID2, nSequence, 0, true, 0, cap)
{
  // this is a bit of a hack
  myMessage[0] = ICQ_PLUGIN_ERROR;
  InitBuffer();
}

//-----Send info plugin request------------------------------------------------
CPU_InfoPluginReq::CPU_InfoPluginReq(const ICQUser* u, const char *GUID,
                                     unsigned long nTime)
  : CPU_PluginMessage(u, false, PLUGIN_INFOxMANAGER)
{
  m_nSize += 27;
  memcpy(m_ReqGUID, GUID, GUID_LENGTH);

  InitBuffer();
  buffer->PackUnsignedShort(2);
  buffer->PackUnsignedShort(1);
  buffer->PackChar(0);
  buffer->Pack(GUID, GUID_LENGTH);
  buffer->PackUnsignedLong(nTime);
}

//-----Send info plugin list---------------------------------------------------
CPU_InfoPluginListResp::CPU_InfoPluginListResp(const ICQUser *u,
                                               unsigned long nMsgID1,
                                               unsigned long nMsgID2,
                                               unsigned short nSequence)
  : CPU_AckThroughServer(u, nMsgID1, nMsgID2, nSequence, 0, true,
                         ICQ_TCPxMSG_URGENT2, PLUGIN_INFOxMANAGER)
{
  unsigned long num_plugins = sizeof(info_plugins)/sizeof(struct PluginList);

  unsigned long nLen;
  if (num_plugins == 0)
    nLen = 0;
  else
  {
    nLen = 4 + 4;
    for (unsigned long i = 0; i < num_plugins; i ++)
    {
      nLen += GUID_LENGTH + 2 + 2 + 4 + strlen(info_plugins[i].name)
              + 4 + strlen(info_plugins[i].description) + 4;
    }
  }

  m_nSize += 2 + 2 + 4 + 4 + nLen;

  InitBuffer();

  buffer->PackUnsignedShort(0);
  buffer->PackUnsignedShort(1);

  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    buffer->PackUnsignedLong(o->ClientInfoTimestamp());
  }
  buffer->PackUnsignedLong(nLen);
  if (nLen != 0)
  {
    buffer->PackUnsignedLong(ICQ_PLUGIN_RESP_INFOxLIST);
    buffer->PackUnsignedLong(num_plugins);
    for (unsigned long i = 0; i < num_plugins; i++)
    {
      buffer->Pack(info_plugins[i].guid, GUID_LENGTH);

      buffer->PackUnsignedShort(0); //Unknown
      buffer->PackUnsignedShort(1); //Unknown

      unsigned int l = strlen(info_plugins[i].name);
      buffer->PackUnsignedLong(l);
      buffer->Pack(info_plugins[i].name, l);

      l = strlen(info_plugins[i].description);
      buffer->PackUnsignedLong(l);
      buffer->Pack(info_plugins[i].description, l);

      buffer->PackUnsignedLong(0);  //Unknown
    }
  }
}

//-----Send phone book response-----------------------------------------------
CPU_InfoPhoneBookResp::CPU_InfoPhoneBookResp(const ICQUser* u, unsigned long nMsgID1,
                                             unsigned long nMsgID2,
                                             unsigned short nSequence)
  : CPU_AckThroughServer(u, nMsgID1, nMsgID2, nSequence, 0, true,
                         ICQ_TCPxMSG_URGENT2, PLUGIN_INFOxMANAGER)
{
  Licq::OwnerReadGuard o(LICQ_PPID);
  const Licq::ICQUserPhoneBook* book = o->GetPhoneBook();

  unsigned long num_entries;
  unsigned long nLen = 4 + 4;
  const struct Licq::PhoneBookEntry* entry;
  for (num_entries = 0; book->Get(num_entries, &entry); num_entries++)
  {
    nLen += 4 + entry->description.size() + 4 + entry->areaCode.size()
        + 4 + entry->phoneNumber.size() + 4 + entry->extension.size()
        + 4 + entry->country.size() + 4 + 4 + 4
        + 4 + entry->gateway.size() + 4 + 4 + 4 + 4;
  }

  m_nSize += 2 + 2 + 4 + 4 + nLen;
  InitBuffer();

  buffer->PackUnsignedShort(0);   //Unknown
  buffer->PackUnsignedShort(1);   //Unknown
  buffer->PackUnsignedLong(o->ClientInfoTimestamp());
  buffer->PackUnsignedLong(nLen);  //Bytes remaining in packet

  buffer->PackUnsignedLong(ICQ_PLUGIN_RESP_PHONExBOOK); //Response ID

  buffer->PackUnsignedLong(num_entries);

  for (unsigned long i = 0; book->Get(i, &entry); i++)
  {
    buffer->PackUnsignedLong(entry->description.size());
    buffer->pack(entry->description);

    buffer->PackUnsignedLong(entry->areaCode.size());
    buffer->pack(entry->areaCode);

    buffer->PackUnsignedLong(entry->phoneNumber.size());
    buffer->pack(entry->phoneNumber);

    buffer->PackUnsignedLong(entry->extension.size());
    buffer->pack(entry->extension);

    buffer->PackUnsignedLong(entry->country.size());
    buffer->pack(entry->country);

    buffer->PackUnsignedLong(entry->nActive);
  }

  for (unsigned long i = 0; book->Get(i, &entry); i++)
  {
    unsigned long sLen = entry->gateway.size();
    buffer->PackUnsignedLong(4 + 4 + sLen + 4 + 4 + 4 + 4);

    buffer->PackUnsignedLong(entry->nType);

    buffer->PackUnsignedLong(sLen);
    buffer->pack(entry->gateway);

    buffer->PackUnsignedLong(entry->nGatewayType);
    buffer->PackUnsignedLong(entry->nSmsAvailable);
    buffer->PackUnsignedLong(entry->nRemoveLeading0s);
    buffer->PackUnsignedLong(entry->nPublish);
  }
}

//-----Send picture response-------------------------------------------------
CPU_InfoPictureResp::CPU_InfoPictureResp(const ICQUser* u, unsigned long nMsgID1,
                                         unsigned long nMsgID2,
                                         unsigned short nSequence)
 : CPU_AckThroughServer(u, nMsgID1, nMsgID2, nSequence, 0, true,
                         ICQ_TCPxMSG_URGENT2, PLUGIN_INFOxMANAGER)
{
  Licq::OwnerReadGuard o(LICQ_PPID);
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
        gLog.error(tr("Unable to stat picture file (%s): %s."),
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

//-----Send status plugin request---------------------------------------------
CPU_StatusPluginReq::CPU_StatusPluginReq(const ICQUser* u, const char *GUID,
                                         unsigned long nTime)
  : CPU_PluginMessage(u, false, PLUGIN_STATUSxMANAGER)
{
  m_nSize += 27;
  memcpy(m_ReqGUID, GUID, GUID_LENGTH);

  InitBuffer();
  buffer->PackUnsignedShort(0);
  buffer->PackUnsignedShort(1);
  buffer->PackChar(0);
  buffer->Pack(GUID, GUID_LENGTH);
  buffer->PackUnsignedLong(nTime);
}

//-----Send status plugin list------------------------------------------------
CPU_StatusPluginListResp::CPU_StatusPluginListResp(const ICQUser* u,
                                                   unsigned long nMsgID1,
                                                   unsigned long nMsgID2,
                                                   unsigned short nSequence)
  : CPU_AckThroughServer(u, nMsgID1, nMsgID2, nSequence, 0, true,
                         0, PLUGIN_STATUSxMANAGER)
{
  unsigned long num_plugins = sizeof(status_plugins)/sizeof(struct PluginList);

  unsigned long nLen;
  if (num_plugins == 0)
    nLen = 0;
  else
  {
    nLen = 4 + 4;
    for (unsigned long i = 0; i < num_plugins; i ++)
    {
      nLen += GUID_LENGTH + 2 + 2 + 4 + strlen(status_plugins[i].name) 
              + 4 + strlen(status_plugins[i].description) + 4;
    }
  }

  m_nSize += 13 + 4 + 4 + nLen;
  InitBuffer();

  buffer->PackUnsignedShort(0);   //Unknown
  buffer->PackUnsignedShort(1);   //Unknown
  buffer->PackUnsignedLong(0);    //Unknown
  buffer->PackUnsignedLong(0);    //Unknown
  buffer->PackChar(1);            //Unknown
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    buffer->PackUnsignedLong(o->ClientStatusTimestamp());
  }
  buffer->PackUnsignedLong(nLen);  //Bytes remaining in packet

  if (nLen != 0)
  {
    buffer->PackUnsignedLong(ICQ_PLUGIN_RESP_STATUSxLIST); //Response ID
    buffer->PackUnsignedLong(num_plugins);
    for (unsigned long i = 0; i < num_plugins; i++)
    {
      buffer->Pack(status_plugins[i].guid, GUID_LENGTH);

      buffer->PackUnsignedShort(0); //Unknown
      buffer->PackUnsignedShort(1); //Unknown

      unsigned int l = strlen(status_plugins[i].name);
      buffer->PackUnsignedLong(l);
      buffer->Pack(status_plugins[i].name, l);

      l = strlen(status_plugins[i].description);
      buffer->PackUnsignedLong(l);
      buffer->Pack(status_plugins[i].description, l);

      buffer->PackUnsignedLong(0);  //Unknown
    }
  }
}

//-----Send the plugin status------------------------------------------------
CPU_StatusPluginResp::CPU_StatusPluginResp(const ICQUser* u, unsigned long nMsgID1,
                                           unsigned long nMsgID2,
                                           unsigned short nSequence,
                                           unsigned long nStatus)
  : CPU_AckThroughServer(u, nMsgID1, nMsgID2, nSequence, 0, true,
                         0, PLUGIN_STATUSxMANAGER)
{
  // this is a bit of a hack
  myMessage[0] = ICQ_PLUGIN_STATUSxREPLY;
  m_nSize += 2 + 2 + 4 + 4 + 1;
  InitBuffer();

  buffer->PackUnsignedShort(0);   //Unknown
  buffer->PackUnsignedShort(1);   //Unknown

  buffer->PackUnsignedLong(nStatus);
  Licq::OwnerReadGuard o(LICQ_PPID);
  buffer->PackUnsignedLong(o->ClientStatusTimestamp());
  buffer->PackChar(1);            //Unknown
}

//-----AdvancedMessage---------------------------------------------------------
CPU_AdvancedMessage::CPU_AdvancedMessage(const ICQUser* u, unsigned short _nMsgType,
  unsigned short _nMsgFlags, bool _bAck, unsigned short _nSequence,
  unsigned long nMsgID1, unsigned long nMsgID2)
  : CPU_Type2Message(u, _bAck,
                     (_nMsgType == ICQ_CMDxSUB_ICBM) || (!_bAck && _nMsgType == ICQ_CMDxTCP_READxAWAYxMSG),
                     ICQ_CAPABILITY_SRVxRELAY, nMsgID1, nMsgID2)
{
  m_nSize += 54;

  m_nMsgFlags = _nMsgFlags;
  m_nSequence = _nSequence;

  if (!_bAck && _nMsgType == ICQ_CMDxTCP_READxAWAYxMSG)
  {
    // Get the correct message
    unsigned status = m_pUser->status();
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

    InitBuffer();
  }
  else
  {
    m_nSubCommand = _nMsgType;
  }
}

void CPU_AdvancedMessage::InitBuffer()
{
  CPU_Type2Message::InitBuffer();

  unsigned short nStatus;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    if (m_pUser->statusToUser() != Licq::User::OfflineStatus)
      nStatus = IcqProtocol::icqStatusFromStatus(m_pUser->statusToUser());
    else
      nStatus = IcqProtocol::icqStatusFromStatus(o->status());
  }

  // XXX Is this really a status? XXX
  // If this is non-zero, ICQ5 ignores our accept file ack.
  nStatus = 0;

  buffer->PackUnsignedShort(0x001B); // len
  buffer->PackUnsignedShort(m_pUser->ConnectionVersion());
  buffer->Pack(PLUGIN_NORMAL, GUID_LENGTH);
  buffer->PackUnsignedLong(0x00000003); // len
  buffer->PackChar(0x04); // accept connections or firewalled
  buffer->PackUnsignedShort(m_nSequence); // sequence
  buffer->PackUnsignedShort(0x000E); // len
  buffer->PackUnsignedShort(m_nSequence); // sequence
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedShort(m_nSubCommand);
  buffer->PackUnsignedShort(nStatus);
  buffer->PackUnsignedShort(m_nMsgFlags);
  buffer->PackUnsignedShort(0x0001); // message len
  buffer->PackChar(0); // message
}

//-----ChatRequest-------------------------------------------------------------
CPU_ChatRequest::CPU_ChatRequest(const string& message, const string& chatUsers,
                                 unsigned short nPort, unsigned short nLevel,
    const ICQUser* _pUser, bool bICBM)
  : CPU_AdvancedMessage(_pUser, bICBM ? ICQ_CMDxSUB_ICBM : ICQ_CMDxSUB_CHAT,
                        nLevel, false, 0)
{
  m_nSize += 14 + chatUsers.size() + message.size();
  if (bICBM)
    m_nSize += 44 + 21; // 21 = strlen(pluginname)

//  m_nExtraLen += 4; m_nSize += 4; //ack request
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
    buffer->PackUnsignedLong(message.size() + chatUsers.size() + 15);
    buffer->PackUnsignedLong(message.size());
    if (!message.empty())
      buffer->pack(message);
  }
  else
    buffer->packString(message);

  buffer->packString(chatUsers);
	buffer->PackUnsignedShortBE(nPort);
	buffer->PackUnsignedShort(0);
	buffer->PackUnsignedShort(nPort);
	buffer->PackUnsignedShort(0);
//	buffer->PackUnsignedLongBE(0x00030000); // ack request
}

//-----FileTransfer------------------------------------------------------------
CPU_FileTransfer::CPU_FileTransfer(const Licq::User* u, const list<string>& lFileList,
    const string& file, const string& desc, unsigned short nLevel, bool bICBM)
  : CPU_AdvancedMessage(u, bICBM ? ICQ_CMDxSUB_ICBM : ICQ_CMDxSUB_FILE, nLevel,
                        false, 0),
    CPX_FileTransfer(lFileList, file)
{
	if (!m_bValid)  return;

  myDesc = desc;

  m_nSize += 18 + myFilename.size() + desc.size();
  if (bICBM)
    m_nSize += 44 + 4; // 4 = strlen("File")
//  m_nExtraLen += 4; m_nSize += 4; //the ack request

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
    buffer->PackUnsignedLong(desc.size() + myFilename.size() + 19); //remaining  - is 4 bytes
																											//dont count last 4 bytes
    buffer->PackUnsignedLong(desc.size()); // file desc - is 4 bytes
    buffer->pack(desc);
  }
  else
    buffer->packString(desc);

	buffer->PackUnsignedLongBE(0x2D384444); // ???
  buffer->packString(myFilename);
	buffer->PackUnsignedLong(m_nFileSize);
	buffer->PackUnsignedLongBE(0);
//	buffer->PackUnsignedLongBE(0x00030000); // ack request
}

//-----NoManager------------------------------------------------------------
CPU_NoManager::CPU_NoManager(const ICQUser* u, unsigned long nMsgID1,
                             unsigned long nMsgID2)
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SERVERxREPLYxMSG)
{
  unsigned long nUinLen = u->accountId().size();

  m_nSize += 17 + nUinLen;

  InitBuffer();

  buffer->PackUnsignedLongBE(nMsgID1);
  buffer->PackUnsignedLongBE(nMsgID2);
  buffer->PackUnsignedShortBE(2);
  buffer->PackChar(nUinLen);
  buffer->pack(u->accountId());
  buffer->PackUnsignedShortBE(0x03);  /* tlv3?? who knows, doesn't fit with the
                                         ack below */
  buffer->PackUnsignedShortBE(0x02);
  buffer->PackUnsignedShort(0);
}
 
//-----AckThroughServer--------------------------------------------------------
CPU_AckThroughServer::CPU_AckThroughServer(const ICQUser* u,
                                           unsigned long nMsgID1,
                                           unsigned long nMsgID2,
                                           unsigned short nSequence,
                                           unsigned short nMsgType,
                                           bool bAccept, unsigned short nLevel,
                                           const char *GUID)
  : CPU_CommonFamily(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SERVERxREPLYxMSG)
{
  myAccountId = u->accountId();

  m_nSize += 66 + myAccountId.size();

  m_nMsgID[0] = nMsgID1;
  m_nMsgID[1] = nMsgID2;
  m_nSequence = nSequence;
  m_nSubCommand = nMsgType;
  m_nSequence = nSequence;
  m_nLevel = nLevel;
  memcpy(m_GUID, GUID, GUID_LENGTH);
  
  if (memcmp(GUID, PLUGIN_NORMAL, GUID_LENGTH) != 0)
  {
    myMessage.assign(1, (bAccept ? ICQ_PLUGIN_SUCCESS : ICQ_PLUGIN_REJECTED));
    m_nStatus = ICQ_TCPxACK_ONLINE;
    m_nSize ++;
  }
  else
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    unsigned short s;
    if (u->statusToUser() != Licq::User::OfflineStatus)
      s = IcqProtocol::icqStatusFromStatus(u->statusToUser());
    else
      s = IcqProtocol::icqStatusFromStatus(o->status());

    if (!bAccept)
      m_nStatus = ICQ_TCPxACK_REFUSE;
    else
    {
      if (s & Licq::User::DoNotDisturbStatus)
        m_nStatus = ICQ_TCPxACK_DNDxCAR;
      else if (s & Licq::User::OccupiedStatus)
        m_nStatus = ICQ_TCPxACK_OCCUPIEDxCAR;
      else if (s & Licq::User::NotAvailableStatus)
        m_nStatus = ICQ_TCPxACK_NA;
      else if (s & Licq::User::AwayStatus)
        m_nStatus = ICQ_TCPxACK_AWAY;
      else
        m_nStatus = ICQ_TCPxACK_ONLINE;
    }
  
    // don't send out AutoResponse if we're online
    // it could contain stuff the other site shouldn't be able to read
    // also some clients always pop up the auto response
    // window when they receive one, annoying for them..
    if(((u->statusToUser() != Licq::User::OfflineStatus &&
        u->statusToUser() != Licq::User::OnlineStatus)  ?
        u->statusToUser() : o->status()) != Licq::User::OfflineStatus)
    {
      myMessage = u->usprintf(o->autoResponse(), Licq::User::usprintf_quotepipe, true);

      if (!u->customAutoResponse().empty())
      {
        myMessage += "\r\n--------------------\r\n";
        myMessage += u->usprintf(u->customAutoResponse(), Licq::User::usprintf_quotepipe, true);
      }
    }
    else
      myMessage.clear();

    // Check for pipes, should possibly go after the ClientToServer call
    myMessage = pipeInput(myMessage);

    myMessage = gTranslator.clientToServer(myMessage);

    // If message is 8099 characters or longer the server will disconnect us so better to truncate
    if (myMessage.size() >= 8099)
      myMessage.resize(8098);

    m_nSize += myMessage.size() + 1;
  }
}

void CPU_AckThroughServer::InitBuffer()
{
  CPU_CommonFamily::InitBuffer();

  bool bPlugin = memcmp(m_GUID, PLUGIN_NORMAL, GUID_LENGTH) != 0;
  
  buffer->PackUnsignedLongBE(m_nMsgID[0]);
  buffer->PackUnsignedLongBE(m_nMsgID[1]);
  buffer->PackUnsignedShortBE(2);
  buffer->PackChar(myAccountId.size());
  buffer->pack(myAccountId);
  buffer->PackUnsignedShortBE(0x03);
  buffer->PackUnsignedShort(0x1b);
  buffer->PackUnsignedShort(ICQ_VERSION_TCP);
  buffer->Pack(m_GUID, GUID_LENGTH);
  buffer->PackUnsignedLong(3);
  buffer->PackChar(0);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedShort(bPlugin ? 0x12 : 0x0E);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedShort(m_nSubCommand);
  buffer->PackUnsignedShort(m_nStatus);
  buffer->PackUnsignedShort(m_nLevel);
  if (bPlugin)
  {
    buffer->PackUnsignedShort(1);
    buffer->PackChar(myMessage[0]);
  }
  else
    buffer->PackString(myMessage.c_str());
}

//-----AckGeneral--------------------------------------------------------------
CPU_AckGeneral::CPU_AckGeneral(const ICQUser* u, unsigned long nMsgID1,
                               unsigned long nMsgID2, unsigned short nSequence,
                               unsigned short nMsgType, bool bAccept,
                               unsigned short nLevel)
  : CPU_AckThroughServer(u, nMsgID1, nMsgID2, nSequence, nMsgType, bAccept,
                         nLevel, PLUGIN_NORMAL)
{
  m_nSize += 8;
  InitBuffer();

  buffer->PackUnsignedLongBE(0);
  buffer->PackUnsignedLongBE(0xFFFFFF00);
}


//-----AckFileAccept-----------------------------------------------------------
CPU_AckFileAccept::CPU_AckFileAccept(const ICQUser* u,//unsigned long nUin,
    const unsigned long nMsgID[2], unsigned short nSequence, unsigned short nPort,
    const string& desc, const string& file, unsigned long nFileSize)
   : CPU_AdvancedMessage(u, ICQ_CMDxSUB_ICBM, 0, true, nSequence, nMsgID[0],
       nMsgID[1])
{
  m_nSize += 66 + file.size() + desc.size();
	InitBuffer();

	buffer->PackUnsignedShort(0x29);  // len of following plugin info
	buffer->PackUnsignedLongBE(0xF02D12D9);
	buffer->PackUnsignedLongBE(0x3091D311);
	buffer->PackUnsignedLongBE(0x8DD70010);
	buffer->PackUnsignedLongBE(0x4B06462E);
	buffer->PackUnsignedShortBE(0x0000);
	buffer->PackUnsignedLong(4); // strlen - is 13 bytes though
	buffer->Pack("File", 4);
	buffer->PackUnsignedLongBE(0x00000100);
	buffer->PackUnsignedLongBE(0x00010000);
	buffer->PackUnsignedLongBE(0);
	buffer->PackUnsignedShortBE(0);
	buffer->PackChar(0);
  buffer->PackUnsignedLong(19 + desc.size() + file.size());
  buffer->PackUnsignedLong(desc.size()); // file desc - is 4 bytes
  buffer->pack(desc); // file desc
	buffer->PackUnsignedLong(ReversePort(nPort)); // port reversed
  buffer->packString(file); // filename
	buffer->PackUnsignedLong(nFileSize); // filesize
	buffer->PackUnsignedLong(nPort); // port
}


//-----AckFileRefuse-----------------------------------------------------------
CPU_AckFileRefuse::CPU_AckFileRefuse(const ICQUser* u, const unsigned long nMsgID[2],
    unsigned short nSequence, const string& message)
  : CPU_AckThroughServer(u, nMsgID[0], nMsgID[1], nSequence,
      ICQ_CMDxSUB_FILE, false, 0, PLUGIN_NORMAL)
{
	// XXX This is not the ICBM way yet!
  m_nSize += message.size() + 18;
	InitBuffer();

  buffer->packString(message);
	buffer->PackUnsignedLong(0); // port reversed
	buffer->PackString("");
	buffer->PackUnsignedLong(0);
	buffer->PackUnsignedLong(0);
}

//-----AckChatAccept-----------------------------------------------------------
CPU_AckChatAccept::CPU_AckChatAccept(const ICQUser* u, const string& clients,
    const unsigned long nMsgID[2], unsigned short nSequence,
                                          unsigned short nPort)
	: CPU_AdvancedMessage(u, ICQ_CMDxSUB_CHAT, 0, true, nSequence,
                             nMsgID[0], nMsgID[1])
{
	// XXX This is not the ICBM way yet!
  m_nSize += 11 + clients.size();
	InitBuffer();

  buffer->packString(clients);
	buffer->PackUnsignedLong(ReversePort(nPort)); // port reversed
	buffer->PackUnsignedLong(nPort);
}

//-----AckChatRefuse-----------------------------------------------------------
CPU_AckChatRefuse::CPU_AckChatRefuse(const ICQUser* u, const unsigned long nMsgID[2],
    unsigned short nSequence, const string& message)
  : CPU_AckThroughServer(u, nMsgID[0], nMsgID[1], nSequence,
      ICQ_CMDxSUB_CHAT, false, 0, PLUGIN_NORMAL)
{
	// XXX This is not the ICBM way yet!
  m_nSize += message.size() + 14;
	InitBuffer();

  buffer->packString(message);
	buffer->PackString("");
	buffer->PackUnsignedLong(0);
	buffer->PackUnsignedLong(0);
}

//-----SendSms-----------------------------------------------------------------
CPU_SendSms::CPU_SendSms(const string& number, const string& message)
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

  string parsedNumber = IcqProtocol::parseDigits(number);

  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    snprintf(szXmlStr, 460, "<icq_sms_message><destination>%s</destination><text>%.160s</text><codepage>1252</codepage><encoding>utf8</encoding><senders_UIN>%s</senders_UIN><senders_name>%s</senders_name><delivery_receipt>Yes</delivery_receipt><time>%s</time></icq_sms_message>",
        parsedNumber.c_str(), message.c_str(), o->accountId().c_str(), o->getAlias().c_str(), szTime);
    szXmlStr[459] = '\0';
  }

  int nLenXmlStr = strlen(szXmlStr) + 1;
  int packetSize = 2+2+2+4+2+2+2 + 22 + 2 + nLenXmlStr;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
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
CPU_RequestAuth::CPU_RequestAuth(const string& accountId, const string& message)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_AUTHxREQ)
{
  m_nSize += accountId.size() + message.size() + 5;
  InitBuffer();

  buffer->PackChar(accountId.size());
  buffer->pack(accountId);
  buffer->PackUnsignedShortBE(message.size());
  buffer->pack(message);
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

CPU_Logoff::~CPU_Logoff()
{
  // Empty
}

//-----RequestList--------------------------------------------------------------
CPU_RequestList::CPU_RequestList()
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_REQUESTxROST)
{
  m_nSize += 6;
  InitBuffer();

  Licq::OwnerReadGuard o(LICQ_PPID);
  buffer->PackUnsignedLongBE(o->GetSSTime());
  buffer->PackUnsignedShortBE(o->GetSSCount());
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
CPU_ExportToServerList::CPU_ExportToServerList(const list<UserId>& users,
                                               unsigned short _nType)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxADD)
{
  unsigned short m_nSID = 0;
  unsigned short m_nGSID = 0;
  int nSize = 0;

  list<UserId>::const_iterator i;
  for (i = users.begin(); i != users.end(); ++i)
  {
    Licq::UserReadGuard pUser(*i);
    if (pUser.isLocked())
    {
      nSize += pUser->accountId().size();
      nSize += 10;

      string::size_type nAliasLen = pUser->getAlias().size();
      if (nAliasLen && _nType == ICQ_ROSTxNORMAL)
          nSize += 4 + nAliasLen;
    }
  }

  m_nSize += nSize;
  InitBuffer();

  for (i = users.begin(); i != users.end(); i++)
  {
    int nLen;
    string unicodeName;

    m_nSID = gUserManager.GenerateSID();

    // Save the SID
    Licq::UserWriteGuard u(*i);
    if (!u.isLocked())
    {
      gLog.warning(tr("Trying to export invalid user %s to server"),
          i->toString().c_str());
      continue;
    }

    if (u->protocolId() != LICQ_PPID)
    {
      gLog.warning(tr("Trying to export non ICQ user %s to ICQ server."),
          i->toString().c_str());
      continue;
    }

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
      const UserGroupList& userGroups = u->GetGroups();

      for (UserGroupList::const_iterator j = userGroups.begin(); j != userGroups.end(); ++j)
      {
        Licq::GroupReadGuard group(*j);
        m_nGSID = group->serverId(LICQ_PPID);
        if (m_nGSID != 0)
          break;
      }

      // No group yet? Use ID of the first one in the list
      if (m_nGSID == 0)
      {
        // First group if none was specified
        {
          Licq::GroupListGuard groups(false);
          if (groups->size() > 0)
          {
            Group* g = *groups->begin();
            g->lockRead();
            m_nGSID = g->serverId(LICQ_PPID);
            g->unlockRead();
          }
        }

        if (m_nGSID == 0)
          m_nGSID = 1; // Must never actually reach this point

        u->addToGroup(gUserManager.GetGroupFromID(m_nGSID));
      }

      u->SetGSID(m_nGSID);
      unicodeName = u->getAlias();
    }

    string accountId = u->accountId();

    SetExtraInfo(m_nGSID);

    nLen = accountId.size();
    buffer->PackUnsignedShortBE(nLen);
    buffer->Pack(accountId.c_str(), nLen);
    buffer->PackUnsignedShortBE(m_nGSID);
    buffer->PackUnsignedShortBE(m_nSID);
    buffer->PackUnsignedShortBE(_nType);

    if (!unicodeName.empty())
    {
      buffer->PackUnsignedShortBE(unicodeName.size() + 4);
      buffer->PackUnsignedShortBE(0x0131);
      buffer->PackUnsignedShortBE(unicodeName.size());
      buffer->pack(unicodeName);
    }
    else
      buffer->PackUnsignedShortBE(0);
  }
}

//-----ExportGroupsToServerList-------------------------------------------------
CPU_ExportGroupsToServerList::CPU_ExportGroupsToServerList(const GroupNameMap& groups)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxADD)
{
  int nSize = 0;
  int nGSID = 0;

  GroupNameMap::const_iterator g;
  for (g = groups.begin(); g != groups.end(); ++g)
  {
    string unicode = gTranslator.toUnicode(g->second);
    nSize += unicode.size();
    nSize += 10;
  }

  m_nSize += nSize;
  InitBuffer();

  // Not necessary, but just to make it explicit that it occurs
  SetExtraInfo(0); // top level group (contains all the groups)

  for (g = groups.begin(); g != groups.end(); g++)
  {
    nGSID = gUserManager.GenerateSID();

    gUserManager.ModifyGroupID(g->first, nGSID);

    string unicodeName = gTranslator.toUnicode(g->second);

    buffer->PackUnsignedShortBE(unicodeName.size());
    buffer->pack(unicodeName);
    buffer->PackUnsignedShortBE(nGSID);
    buffer->PackUnsignedShortBE(0);
    buffer->PackUnsignedShortBE(ICQ_ROSTxGROUP);
    buffer->PackUnsignedShortBE(0);
  }
}

//-----AddPrivacyInfo-----------------------------------------------------------
CPU_AddPDINFOToServerList::CPU_AddPDINFOToServerList()
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxADD), m_nSID(0),
    m_nGSID(0)
{
  m_nSID = gUserManager.GenerateSID();
  m_nSize += 15;
  InitBuffer();

  Licq::OwnerWriteGuard o(LICQ_PPID);
  o->SetPDINFO(m_nSID);

  buffer->PackUnsignedShortBE(0);
  buffer->PackUnsignedShortBE(0);
  buffer->PackUnsignedShortBE(m_nSID);
  buffer->PackUnsignedShortBE(ICQ_ROSTxPDINFO);
  buffer->PackUnsignedShortBE(5);
  buffer->PackUnsignedLongBE(0x00CA0001);
  buffer->PackChar(ICQ_PRIVACY_ALLOW_ALL); // Default so users can see us
}

//-----AddToServerList----------------------------------------------------------
CPU_AddToServerList::CPU_AddToServerList(const string& name,
                                         unsigned short _nType,
                                         unsigned short _nGroup, bool _bAuthReq,
                                         bool _bTopLevel)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxADD),
    m_nSID(0),
    m_nGSID(0)
{
  UserId userId(name, LICQ_PPID);
  unsigned short nStrLen = name.size();
  unsigned short nExportSize = 0;
  string unicodeName;
  Licq::TlvList tlvs;
  CBuffer tlvBuffer;

  m_nSID = gUserManager.GenerateSID();

  switch (_nType)
  {
    case ICQ_ROSTxNORMAL:
    {
      // Save the SID
      Licq::UserWriteGuard u(userId);
      u->SetSID(m_nSID);
      u->SetAwaitingAuth(_bAuthReq);

      // Check for a group id

      {
        // Passed in id
        m_nGSID = 0;
        Licq::GroupReadGuard group(_nGroup);
        if (group.isLocked())
          m_nGSID = group->serverId(LICQ_PPID);
      }
      if (m_nGSID == 0)
      {
        // Preset id
        m_nGSID = u->GetGSID();
      }
      if (m_nGSID == 0)
      {
        // Use the first group the user is in as the server stored group
        const UserGroupList& userGroups = u->GetGroups();
        for (UserGroupList::iterator iter = userGroups.begin(); iter != userGroups.end(); ++iter)
        {
          Licq::GroupReadGuard group(*iter);
          m_nGSID = group->serverId(LICQ_PPID);
          if (m_nGSID != 0)
            break;
        }
      }
      if (m_nGSID == 0)
      {
        // Use the first group from the list
        Licq::GroupListGuard groups;
        if (groups->size() > 0)
        {
          Group* g = *groups->begin();
          g->lockRead();
          m_nGSID = g->serverId(LICQ_PPID);
          g->unlockRead();
        }
      }
      if (m_nGSID == 0)
        m_nGSID = 1;

      // Get all the TLV's attached to this user, otherwise the server will delete
      // all of the ones that we don't send
      tlvs = u->GetTLVList();

      // We need to iterate two times since we don't have a dynamic CBuffer
      unsigned short extraTlvSize = 0;
      Licq::TlvList::iterator tlv_iter;
      for (tlv_iter = tlvs.begin(); tlv_iter != tlvs.end(); ++tlv_iter)
        extraTlvSize += tlv_iter->second->getLength() + 4;

      // Make the proper sized buffer for all the TLVs the user has
      tlvBuffer.Create(extraTlvSize);

      // Now copy them to the new buffer
      for (tlv_iter = tlvs.begin(); tlv_iter != tlvs.end(); ++tlv_iter)
        tlvBuffer.PackTLV(tlv_iter->second);

      SetExtraInfo(m_nGSID);
      u->SetGSID(m_nGSID);
      u->addToGroup(gUserManager.GetGroupFromID(m_nGSID));

      break;
    }

    case ICQ_ROSTxGROUP:
    {
      // the way it works
      m_nGSID = _bTopLevel ? 0 : m_nSID;
      m_nSID = 0;
      SetExtraInfo(0);

      unicodeName = gTranslator.toUnicode(name);
      nStrLen = unicodeName.size();

      if (_bTopLevel)
        nExportSize += 4 + (2 * gUserManager.NumGroups());
      else
      {
        int groupId = Licq::gUserManager.GetGroupFromName(name);
        if (groupId != 0)
          Licq::gUserManager.ModifyGroupID(groupId, m_nGSID);
      }
      break;
    }

    case ICQ_ROSTxINVISIBLE:
    case ICQ_ROSTxVISIBLE:
    case ICQ_ROSTxIGNORE:
    {
      m_nGSID = 0;

      Licq::UserWriteGuard u(userId);
      if (_nType == ICQ_ROSTxIGNORE)
      {
        u->SetSID(m_nSID);
        u->SetGSID(0); // m_nGSID
      }
      else if (_nType == ICQ_ROSTxVISIBLE)
        u->SetVisibleSID(m_nSID);
      else
        u->SetInvisibleSID(m_nSID);

      SetExtraInfo(0); // not necessary except by design
      break;
    }
  }

  // Do we need extras since we handle the tlvs as a complete block?
  m_nSize += 10+nStrLen+nExportSize+(_bAuthReq ? 4 : 0)+tlvBuffer.getDataSize();
  InitBuffer();

  buffer->PackUnsignedShortBE(nStrLen);
  buffer->pack(!unicodeName.empty() ? unicodeName : name);
  buffer->PackUnsignedShortBE(m_nGSID);
  buffer->PackUnsignedShortBE(m_nSID);
  buffer->PackUnsignedShortBE(_nType);
  // TODO should only be tlvBuffer.getDataSize()
  buffer->PackUnsignedShortBE(nExportSize+(_bAuthReq ? 4 : 0)+tlvBuffer.getDataSize());
  if (nExportSize)
  {
    if (_bTopLevel)
    {
      // We are creating our top level group, so attach all the group ids now
      buffer->PackUnsignedShortBE(0x00C8);
      buffer->PackUnsignedShortBE(gUserManager.NumGroups() * 2);

      Licq::GroupListGuard groupList(false);
      BOOST_FOREACH(const Licq::Group* group, **groupList)
      {
        Licq::GroupReadGuard pGroup(group);
        buffer->PackUnsignedShortBE(pGroup->serverId(LICQ_PPID));
      }
    }
    else
    {
      // Addming a normal user, so save their alias
      // TODO Have this be a part of tlvBuffer
      buffer->PackUnsignedShortBE(0x0131);
      buffer->PackUnsignedShortBE(nExportSize-4);
      if (!unicodeName.empty())
        buffer->Pack(unicodeName.c_str(), nExportSize-4);
    }
  }

  // TODO Have this be a part of tlvBuffer so we don't need this check against tlvBuffer
  if (_bAuthReq && !tlvBuffer.hasTLV(0x0066))
    buffer->PackUnsignedLongBE(0x00660000);

  if (!tlvBuffer.Empty())
    buffer->Pack(&tlvBuffer);
}

//-----RemoveFromServerList-----------------------------------------------------
CPU_RemoveFromServerList::CPU_RemoveFromServerList(const string& name,
	unsigned short _nGSID, unsigned short _nSID, unsigned short _nType)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxREM)
{
  UserId userId(name, LICQ_PPID);
  int nNameLen = name.size();
  string unicodeName;
  CBuffer tlvBuffer;

  if (_nType == ICQ_ROSTxNORMAL)
  {
    Licq::UserReadGuard u(userId);
    if (u.isLocked())
    {
      Licq::TlvList tlvs = u->GetTLVList();

      unsigned short extraTlvSize = 0;
      Licq::TlvList::iterator tlv_iter;
      for (tlv_iter = tlvs.begin(); tlv_iter != tlvs.end(); ++tlv_iter)
        extraTlvSize += tlv_iter->second->getLength() + 4;

      // Make the proper sized buffer for all the TLVs the user has
      tlvBuffer.Create(extraTlvSize);

      // Now copy them to the new buffer
      for (tlv_iter = tlvs.begin(); tlv_iter != tlvs.end(); ++tlv_iter)
        tlvBuffer.PackTLV(tlv_iter->second);
    }
  }
  else if (_nType == ICQ_ROSTxGROUP)
  {
    unicodeName = gTranslator.toUnicode(name);
    nNameLen = unicodeName.size();
  }

  m_nSize += 10+nNameLen+tlvBuffer.getDataSize();
  InitBuffer();

  buffer->PackUnsignedShortBE(nNameLen);
  buffer->pack(!unicodeName.empty() ? unicodeName : name);
  buffer->PackUnsignedShortBE(_nGSID);
  buffer->PackUnsignedShortBE(_nSID);
  buffer->PackUnsignedShortBE(_nType);
  buffer->PackUnsignedShortBE(static_cast<unsigned short>(tlvBuffer.getDataSize()));
  if (!tlvBuffer.Empty())
    buffer->Pack(&tlvBuffer);

  if (_nType == ICQ_ROSTxGROUP)
    SetExtraInfo(0);
  else
    SetExtraInfo(_nGSID);
}

//-----ClearServerList------------------------------------------------------
CPU_ClearServerList::CPU_ClearServerList(const StringList& uins,
                                         unsigned short _nType)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxREM)
{
  int nSize = 0;
  StringList::const_iterator i;

  if (_nType == ICQ_ROSTxGROUP)
    return;

  for (i = uins.begin(); i != uins.end(); ++i)
  {
    UserId userId(*i, LICQ_PPID);
    Licq::UserReadGuard u(userId);
    if (u.isLocked())
    {
      nSize += i->size() + 2;
      nSize += 8;
      if (u->GetAwaitingAuth())
        nSize += 4;
    }
  }

  m_nSize += nSize;
  InitBuffer();

  for (i = uins.begin(); i != uins.end(); i++)
  {
    UserId userId(*i, LICQ_PPID);
    Licq::UserWriteGuard pUser(userId);
    if (pUser.isLocked())
    {
      bool bAuthReq = pUser->GetAwaitingAuth();
      unsigned short nGSID = 0;
      
      if (_nType == ICQ_ROSTxNORMAL)
        nGSID = pUser->GetGSID();

      buffer->PackUnsignedShortBE(i->size());
      buffer->Pack(i->c_str(), i->size());
      buffer->PackUnsignedShortBE(nGSID);
      buffer->PackUnsignedShortBE(pUser->GetSID());
      buffer->PackUnsignedShortBE(_nType);
      buffer->PackUnsignedShortBE(bAuthReq ? 4 : 0);
      if (bAuthReq)
        buffer->PackUnsignedLongBE(0x00660000);
        
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

      pUser->save(Licq::User::SaveLicqInfo);
    }
  }
}

//-----UpdateToServerList---------------------------------------------------
CPU_UpdateToServerList::CPU_UpdateToServerList(const string& name,
                                               unsigned short _nType,
                                               unsigned short _nGSID, // only for groups
                                               bool _bAuthReq)
  : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_ROSTxUPD_GROUP)
{
  UserId userId(name, LICQ_PPID);
  unsigned short nGSID = 0;
  unsigned short nSID = 0;
  unsigned short nExtraLen = 0;
  unsigned short nNameLen = name.size();
  string unicodeName;
  CBuffer tlvBuffer;

  list<unsigned long> groupIds;

  switch (_nType)
  {
    case ICQ_ROSTxNORMAL:
    {
      Licq::UserReadGuard u(userId);
      if (u.isLocked())
      {
        if (u->GetAwaitingAuth())
          _bAuthReq = true;

        // Get all the TLV's attached to this user, otherwise the server will delete
        // all of the ones that we don't send
        Licq::TlvList tlvs = u->GetTLVList();

        // We need to iterate two times since we don't have a dynamic CBuffer
        unsigned short extraTlvSize = 0;
        Licq::TlvList::iterator tlv_iter;
        for (tlv_iter = tlvs.begin(); tlv_iter != tlvs.end(); ++tlv_iter)
          extraTlvSize += tlv_iter->second->getLength() + 4;

        // Make the proper sized buffer for all the TLVs the user has
        tlvBuffer.Create(extraTlvSize);

        // Now copy them to the new buffer
        for (tlv_iter = tlvs.begin(); tlv_iter != tlvs.end(); ++tlv_iter)
          tlvBuffer.PackTLV(tlv_iter->second);

        nGSID = u->GetGSID();
        nSID = u->GetSID();
        nExtraLen = tlvBuffer.getDataSize();
      }

      break;
    }

    case ICQ_ROSTxGROUP:
    {
      nGSID = _nGSID;
      nSID = 0;

      if (nGSID == 0)
      {
        // Get group ids from list, we'll need them later
        Licq::GroupListGuard groups;
        BOOST_FOREACH(const Group* i, **groups)
        {
          i->lockRead();
          groupIds.push_back(i->serverId(LICQ_PPID));
          i->unlockRead();
        }
        nExtraLen += (groups->size() * 2);
      }
      else
      {
        unicodeName = gTranslator.toUnicode(name);
        nNameLen = unicodeName.size();

        Licq::UserListGuard userList;
        BOOST_FOREACH(const Licq::User* user, **userList)
        {
          Licq::UserReadGuard pUser(user);
          if (pUser->GetGSID() == nGSID)
            nExtraLen += 2;
        }
      }

      if (nExtraLen)
        nExtraLen += 4;
      break;
    }
  }

  m_nSize += 10 + nNameLen + nExtraLen + (_bAuthReq ? 4 : 0);
  InitBuffer();

  buffer->PackUnsignedShortBE(nNameLen);
  if (!unicodeName.empty() && _nType == ICQ_ROSTxGROUP)
    buffer->pack(unicodeName);
  else
    buffer->pack(name);
  buffer->PackUnsignedShortBE(nGSID);
  buffer->PackUnsignedShortBE(nSID);
  buffer->PackUnsignedShortBE(_nType);
  buffer->PackUnsignedShortBE(nExtraLen + (_bAuthReq ? 4 : 0));
  if (nExtraLen)
  {
    if (_nType == ICQ_ROSTxNORMAL)
    {
      buffer->Pack(&tlvBuffer);
    }
    else if (_nType == ICQ_ROSTxGROUP)
    {
      buffer->PackUnsignedShortBE(0x00C8);
      buffer->PackUnsignedShortBE(nExtraLen-4);

      if (nGSID == 0)
      {
        BOOST_FOREACH(unsigned long id, groupIds)
          buffer->PackUnsignedShortBE(id);
      }
      else
      {
        Licq::UserListGuard userList;
        BOOST_FOREACH(const Licq::User* user, **userList)
        {
          Licq::UserReadGuard pUser(user);
          if (pUser->GetGSID() == nGSID)
            buffer->PackUnsignedShortBE(pUser->GetSID());
        }
      }
    }
  }

  if (_bAuthReq)
    buffer->PackUnsignedLongBE(0x00660000);
}

//-----SearchWhitePages---------------------------------------------------------
CPU_SearchWhitePages::CPU_SearchWhitePages(const string& firstName, const string& lastName,
    const string& alias, const string& email, unsigned short nMinAge, unsigned short nMaxAge,
    char nGender, char nLanguage, const string& city, const string& state,
    unsigned short nCountryCode, const string& coName, const string& coDept,
    const string& coPos, const string& keyword, bool bOnlineOnly)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  unsigned short nDataLen;	// length of data info to add packet size

  nDataLen = lengthField(firstName) + lengthField(lastName) +
             lengthField(alias) + lengthField(city) +
             lengthField(email) + lengthField(state) +
             lengthField(coName) + lengthField(coDept) +
             lengthField(coPos) + lengthField(keyword);

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
      case 60: nMaxAge = 10000; break;
    }
  }

  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackUnsignedShortBE(16 + nDataLen - 4);
  buffer->PackUnsignedShort(16 + nDataLen - 6);
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShortBE(0xd007);
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand);

  packSearch(ICQ_CMDxWPxFNAME, gTranslator.clientToServer(firstName));
  packSearch(ICQ_CMDxWPxLNAME, gTranslator.clientToServer(lastName));
  packSearch(ICQ_CMDxWPxALIAS, gTranslator.clientToServer(alias));
  packSearch(ICQ_CMDxWPxEMAIL, gTranslator.clientToServer(email));
  packSearch(ICQ_CMDxWPxCITY, gTranslator.clientToServer(city));
  packSearch(ICQ_CMDxWPxSTATE, gTranslator.clientToServer(state));
  packSearch(ICQ_CMDxWPxCOMPANY, gTranslator.clientToServer(coName));
  packSearch(ICQ_CMDxWPxCODEPT, gTranslator.clientToServer(coDept));
  packSearch(ICQ_CMDxWPxCOPOS, gTranslator.clientToServer(coPos));
  packSearch(ICQ_CMDxWPxKEYWORD, gTranslator.clientToServer(keyword));

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

void CPU_SearchWhitePages::packSearch(unsigned short nCmd, const string& field)
{
  // By SC
  // Pack string field info if exist

  int nLenField = field.size() + 1;

  if (nLenField > 1)
  {
    buffer->PackUnsignedShortBE(nCmd);
    buffer->PackUnsignedShort(nLenField + 2);
    buffer->PackLNTS(field.c_str());
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
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShortBE(0xD007); // type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); // sub type

  buffer->PackUnsignedLongBE(0x36010400);
  buffer->PackUnsignedLong(nUin);
}


//-----UpdatePersonalInfo-------------------------------------------------------
CPU_UpdatePersonalBasicInfo::CPU_UpdatePersonalBasicInfo(const string& alias,
    const string& firstName, const string& lastName, const string& email, bool bAuthorization)
  : CPacketUdp(ICQ_CMDxSND_UPDATExBASIC)
{

  m_nAuthorization = bAuthorization ? 0 : 1;

  m_nSize += 15 + alias.size() + firstName.size() + lastName.size() + email.size();
  InitBuffer();

#if ICQ_VERSION == 2
  buffer->PackUnsignedShort(m_nSubSequence);
#endif

  myAlias = buffer->packString(gTranslator.clientToServer(alias));
  myFirstName = buffer->packString(gTranslator.clientToServer(firstName));
  myLastName = buffer->packString(gTranslator.clientToServer(lastName));
  myEmail = buffer->packString(gTranslator.clientToServer(email));
  buffer->PackChar(m_nAuthorization);

}


//-----UpdatePersonalExtInfo-------------------------------------------------------
CPU_UpdatePersonalExtInfo::CPU_UpdatePersonalExtInfo(const string& city,
    unsigned short nCountry, const string& state, unsigned short nAge, char cSex,
    const string& phone, const string& homepage, const string& about, unsigned long nZipcode)
  : CPacketUdp(ICQ_CMDxSND_UPDATExDETAIL)
{

  m_nCountry = nCountry;
  m_cTimezone = ICQUser::SystemTimezone();
  m_nAge = nAge;
  m_cSex = cSex;
  m_nZipcode = nZipcode;

  m_nSize += city.size() + state.size() + phone.size() + homepage.size() + about.size() + 27;
  InitBuffer();

#if ICQ_VERSION == 2
  buffer->PackUnsignedShort(m_nSubSequence);
#endif

  myCity = buffer->packString(gTranslator.clientToServer(city));
  buffer->PackUnsignedShort(m_nCountry);
  buffer->PackChar(m_cTimezone);
  myState = buffer->packString(gTranslator.clientToServer(state), 5);
  buffer->PackUnsignedShort(m_nAge);
  buffer->PackChar(m_cSex);
  myPhone = buffer->packString(gTranslator.clientToServer(phone));
  myHomepage = buffer->packString(gTranslator.clientToServer(homepage));
  myAbout = buffer->packString(about);
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
      nIp = LE_32(nIp);
#endif



//-----Authorize----------------------------------------------------------------
CPU_Authorize::CPU_Authorize(const string& accountId)
 : CPU_CommonFamily(ICQ_SNACxFAM_LIST, ICQ_SNACxLIST_AUTHxGRANT)
{
  m_nSize += 1 + accountId.size() + 5;
  InitBuffer();

  buffer->PackChar(accountId.size());
  buffer->pack(accountId);
  buffer->PackChar(0x01);
  buffer->PackUnsignedLong(0);
}

//------SetPassword---------------------------------------------------------
CPU_SetPassword::CPU_SetPassword(const string& password)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nSubCommand = ICQ_CMDxMETA_PASSWORDxSET;

  unsigned short nDataLen = password.size() + 19;
  m_nSize += nDataLen;
  InitBuffer();

  buffer->PackUnsignedShortBE(0x0001);
  buffer->PackUnsignedShortBE(nDataLen - 4);
  buffer->PackUnsignedShort(nDataLen - 6);
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShortBE(0xd007);
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(ICQ_CMDxMETA_PASSWORDxSET);

  // LNTS, but we want the password in this class
  //buffer->PackUnsignedShort(nDataLen - 19);
  myPassword = buffer->PackLNTS(password.c_str());//buffer->PackString(szPassword);
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
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
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
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
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
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
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
    buffer->PackChar(gIcqProtocol.directMode() ? MODE_DIRECT : MODE_INDIRECT);
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
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShortBE(0xD007); // type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); // sub type
  buffer->PackUnsignedShort(nGroup);
}


CPU_Meta_SetGeneralInfo::CPU_Meta_SetGeneralInfo(const string& alias,
    const string& firstName, const string& lastName, const string& emailPrimary,
    const string& city, const string& state, const string& phoneNumber,
    const string& faxNumber, const string& address, const string& cellularNumber,
    const string& zipCode, unsigned short nCountryCode, bool bHideEmail)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_WPxINFOxSET;

  m_nCountryCode = nCountryCode;
  m_nTimezone = ICQUser::SystemTimezone();
  m_nHideEmail = bHideEmail ? 1 : 0;

  int packetSize = 2+2+2+4+2+2+2 + alias.size() + firstName.size() + lastName.size() +
    emailPrimary.size() + city.size() + state.size() + phoneNumber.size() +
    faxNumber.size() + address.size() + zipCode.size() + cellularNumber.size() + 89;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1
  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShortBE(0xD007); 			// type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); // subtype

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxTIMEZONE);
  buffer->PackUnsignedShort(0x0001);
  buffer->PackChar(m_nTimezone);
  
  buffer->PackUnsignedShortBE(ICQ_CMDxWPxALIAS);
  buffer->PackUnsignedShort(alias.size()+3);
  myAlias = buffer->packString(gTranslator.clientToServer(alias));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxFNAME);
  buffer->PackUnsignedShort(firstName.size()+3);
  myFirstName = buffer->packString(gTranslator.clientToServer(firstName));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxLNAME);
  buffer->PackUnsignedShort(lastName.size()+3);
  myLastName = buffer->packString(gTranslator.clientToServer(lastName));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxEMAIL);
  buffer->PackUnsignedShort(emailPrimary.size()+3);
  myEmailPrimary = buffer->packString(gTranslator.clientToServer(emailPrimary));
  buffer->PackChar(m_nHideEmail);
  
  buffer->PackUnsignedShortBE(ICQ_CMDxWPxCITY);
  buffer->PackUnsignedShort(city.size()+3);
  myCity = buffer->packString(gTranslator.clientToServer(city));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxSTATE);
  buffer->PackUnsignedShort(state.size()+3);
  myState = buffer->packString(gTranslator.clientToServer(state));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxPHONExHOME);
  buffer->PackUnsignedShort(phoneNumber.size()+3);
  myPhoneNumber = buffer->packString(gTranslator.clientToServer(phoneNumber));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxHOME_FAX);
  buffer->PackUnsignedShort(faxNumber.size()+3);
  myFaxNumber = buffer->packString(gTranslator.clientToServer(faxNumber));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxHOME_ADDR);
  buffer->PackUnsignedShort(address.size()+3);
  myAddress = buffer->packString(gTranslator.clientToServer(address));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxPHONExCELL);
  buffer->PackUnsignedShort(cellularNumber.size()+3);
  myCellularNumber = buffer->packString(gTranslator.clientToServer(cellularNumber));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxHOME_ZIP);
  buffer->PackUnsignedShort(zipCode.size()+3);
  myZipCode = buffer->packString(gTranslator.clientToServer(zipCode));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxCOUNTRY);
  buffer->PackUnsignedShort(0x0002);
  buffer->PackUnsignedShort(m_nCountryCode);
  
  // Check for possible problems
  //ar *sz = m_szAlias;
  //ile (*sz != '\0' && strncasecmp(sz, "icq", 3) != 0) sz++;
  // (*sz != '\0')
  //
  //gLog.warning(tr("Alias may not contain \"icq\"."));
  //*sz = '-';
  //
}

//-----Meta_SetEmailInfo------------------------------------------------------
CPU_Meta_SetEmailInfo::CPU_Meta_SetEmailInfo(const string& emailSecondary, const string& emailOld)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_EMAILxINFOxSET;

  int packetSize = 2+2+2+4+2+2+2 + emailSecondary.size() + 3
      + emailOld.size() + 3 + 3;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShortBE(0xD007); 			// type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); // subtype

  buffer->PackChar(2);
  buffer->PackChar(0);
  myEmailSecondary = buffer->packString(gTranslator.clientToServer(emailSecondary));
  buffer->PackChar(0);
  myEmailOld = buffer->packString(gTranslator.clientToServer(emailOld));
}

//-----Meta_SetMoreInfo------------------------------------------------------
CPU_Meta_SetMoreInfo::CPU_Meta_SetMoreInfo( unsigned short nAge, char nGender,
    const string& homepage, unsigned short nBirthYear, char nBirthMonth,
    char nBirthDay, char nLanguage1, char nLanguage2, char nLanguage3)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_WPxINFOxSET;

  m_nAge = nAge;
  m_nGender = nGender;
  m_nBirthYear = nBirthYear;
  m_nBirthMonth = nBirthMonth;
  m_nBirthDay = nBirthDay;
  m_nLanguage1 = nLanguage1;
  m_nLanguage2 = nLanguage2;
  m_nLanguage3 = nLanguage3;

  int packetSize = 2+2+2+4+2+2+2 + homepage.size()+3 + 43;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShortBE(0xD007); 			// type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); 			// subtype

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxAGE);
  buffer->PackUnsignedShort(0x0002);
  buffer->PackUnsignedShort(m_nAge);

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxGENDER);
  buffer->PackUnsignedShort(0x0001);
  buffer->PackChar(nGender);

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxHOMEPAGE);
  buffer->PackUnsignedShort(homepage.size()+3);
  myHomepage = buffer->packString(gTranslator.clientToServer(homepage));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxBIRTHDAY);
  buffer->PackUnsignedShort(0x0006);
  buffer->PackUnsignedShort(m_nBirthYear);
  buffer->PackUnsignedShort(m_nBirthMonth);
  buffer->PackUnsignedShort(m_nBirthDay);

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxLANGUAGE);
  buffer->PackUnsignedShort(0x0002);
  buffer->PackUnsignedShort(m_nLanguage1);

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxLANGUAGE);
  buffer->PackUnsignedShort(0x0002);
  buffer->PackUnsignedShort(m_nLanguage2);

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxLANGUAGE);
  buffer->PackUnsignedShort(0x0002);
  buffer->PackUnsignedShort(m_nLanguage3);
}

//-----Meta_SetInterestsInfo----------------------------------------------------
CPU_Meta_SetInterestsInfo::CPU_Meta_SetInterestsInfo(const UserCategoryMap& interests)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_INTERESTSxINFOxSET;

  int packetSize = 2 + 2 + 2 + 4 + 2 + 2 + 2 + 1;
  UserCategoryMap::const_iterator i;
  for (i = interests.begin(); i != interests.end(); ++i)
  {
    char* tmp = strdup(i->second.c_str());
    gTranslator.ClientToServer(tmp);
    myInterests[i->first] = tmp;
    packetSize += 2 + 2 + strlen(tmp) + 1;
    free(tmp);
  }

  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShortBE(0xD007); 			// type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); 			// subtype

  buffer->PackChar(myInterests.size());
  for (i = myInterests.begin(); i != myInterests.end(); ++i)
  {
    buffer->PackUnsignedShort(i->first);
    buffer->PackString(i->second.c_str());
  }
}

//-----Meta_SetOrgBackInfo------------------------------------------------------
CPU_Meta_SetOrgBackInfo::CPU_Meta_SetOrgBackInfo(const UserCategoryMap& orgs,
    const UserCategoryMap& background)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_ORGBACKxINFOxSET;

  int packetSize = 2 + 2 + 2 + 4 + 2 + 2 + 2 + 2;
  UserCategoryMap::const_iterator i;
  for (i = orgs.begin(); i != orgs.end(); ++i)
  {
    char* tmp = strdup(i->second.c_str());
    gTranslator.ClientToServer(tmp);
    myOrganizations[i->first] = tmp;
    packetSize += 2 + 2 + strlen(tmp) + 1;
    free(tmp);
  }
  for (i = background.begin(); i != background.end(); ++i)
  {
    char* tmp = strdup(i->second.c_str());
    gTranslator.ClientToServer(tmp);
    myBackgrounds[i->first] = tmp;
    packetSize += 2 + 2 + strlen(tmp) + 1;
    free(tmp);
  }

  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShortBE(0xD007); 			// type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); 			// subtype

  buffer->PackChar(myBackgrounds.size());
  for (i = myBackgrounds.begin(); i != myBackgrounds.end(); ++i)
  {
    buffer->PackUnsignedShort(i->first);
    buffer->PackString(i->second.c_str());
  }
  buffer->PackChar(myOrganizations.size());
  for (i = myOrganizations.begin(); i != myOrganizations.end(); ++i)
  {
    buffer->PackUnsignedShort(i->first);
    buffer->PackString(i->second.c_str());
  }
}

//-----Meta_SetWorkInfo------------------------------------------------------
CPU_Meta_SetWorkInfo::CPU_Meta_SetWorkInfo(const string& city, const string& state,
    const string& phoneNumber, const string& faxNumber, const string& address,
    const string& zip, unsigned short nCompanyCountry, const string& name,
    const string& department, const string& position, unsigned short nCompanyOccupation,
    const string& homepage)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_WPxINFOxSET;
  
  m_nCompanyCountry = nCompanyCountry;  
  m_nCompanyOccupation = nCompanyOccupation;

  string statebuf(state, 0, 5);

  int packetSize = 2+2+2+4+2+2+2 + city.size() + statebuf.size() + phoneNumber.size() +
      faxNumber.size() + address.size() + zip.size() + name.size() + department.size() +
      position.size() + homepage.size() + 82;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShortBE(0xD007); 			// type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); 			// subtype

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxWORK_CITY);
  buffer->PackUnsignedShort(city.size()+3);
  myCity = buffer->packString(gTranslator.clientToServer(city));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxWORK_STATE);
  buffer->PackUnsignedShort(statebuf.size()+3);
  myState = buffer->packString(gTranslator.clientToServer(statebuf));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxWORK_PHONE);
  buffer->PackUnsignedShort(phoneNumber.size()+3);
  myPhoneNumber = buffer->packString(gTranslator.clientToServer(phoneNumber));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxWORK_FAX);
  buffer->PackUnsignedShort(faxNumber.size()+3);
  myFaxNumber = buffer->packString(gTranslator.clientToServer(faxNumber));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxWORK_ADDR);
  buffer->PackUnsignedShort(address.size()+3);
  myAddress = buffer->packString(gTranslator.clientToServer(address));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxWORK_ZIP);
  buffer->PackUnsignedShort(zip.size()+3);
  myZip = buffer->packString(gTranslator.clientToServer(zip));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxWORK_COUNTRY);
  buffer->PackUnsignedShort(0x0002);
  buffer->PackUnsignedShort(nCompanyCountry);

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxCOMPANY);
  buffer->PackUnsignedShort(name.size()+3);
  myName = buffer->packString(gTranslator.clientToServer(name));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxCODEPT);
  buffer->PackUnsignedShort(department.size()+3);
  myDepartment = buffer->packString(gTranslator.clientToServer(department));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxCOPOS);
  buffer->PackUnsignedShort(position.size()+3);
  myPosition = buffer->packString(gTranslator.clientToServer(position));

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxOCCUP);
  buffer->PackUnsignedShort(0x0002);
  buffer->PackUnsignedShort(m_nCompanyOccupation);

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxWORK_URL);
  buffer->PackUnsignedShort(homepage.size()+3);
  myHomepage = buffer->packString(gTranslator.clientToServer(homepage));
}

//-----Meta_SetAbout---------------------------------------------------------
CPU_Meta_SetAbout::CPU_Meta_SetAbout(const string& about)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA),
    myAbout(about)
{
  m_nMetaCommand = ICQ_CMDxMETA_WPxINFOxSET;

  if (myAbout.size() > size_t(IcqProtocol::MaxMessageSize))
    myAbout.resize(IcqProtocol::MaxMessageSize);

  int packetSize = 2+2+2+4+2+2+2 + myAbout.size() + 7;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); 		// TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); 		// bytes remaining
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShortBE(0xD007); 			// type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); 			// subtype

  buffer->PackUnsignedShortBE(ICQ_CMDxWPxABOUT);
  buffer->PackUnsignedShort(myAbout.size()+3);
  buffer->packString(myAbout);
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


  int packetSize = 2+2+2+4+2+2+2+4+6;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); // TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); // bytes remaining
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShortBE(0xd007); // type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); // subtype
  buffer->PackUnsignedShortBE(0x0C03);
  buffer->PackUnsignedShortBE(0x0100);
  buffer->PackChar(m_nWebAware);
  buffer->PackChar(0xF8);
  buffer->PackChar(2);
  buffer->PackChar(1);
  buffer->PackChar(0);
  buffer->PackChar(m_nAuthorization);
}


//-----Meta_RequestInfo------------------------------------------------------
CPU_Meta_RequestAllInfo::CPU_Meta_RequestAllInfo(const string& accountId)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  if (accountId == gUserManager.ownerUserId(LICQ_PPID).accountId())
    m_nMetaCommand = ICQ_CMDxMETA_REQUESTxALLxINFO;
  else
    m_nMetaCommand = ICQ_CMDxMETA_REQUESTxALLxINFOxOWNER;
  myAccountId = accountId;

  int packetSize = 2+2+2+4+2+2+2+4;
  m_nSize += packetSize;
  InitBuffer();

  buffer->PackUnsignedShortBE(1);
  buffer->PackUnsignedShortBE(packetSize-2-2); // TLV 1

  buffer->PackUnsignedShort(packetSize-2-2-2); // bytes remaining
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShortBE(0xd007); // type
  buffer->PackUnsignedShortBE(m_nSubSequence);
  buffer->PackUnsignedShort(m_nMetaCommand); // subtype
  buffer->PackUnsignedLong(strtoul(myAccountId.c_str(), (char **)NULL, 10));
}

//-----Meta_RequestInfo------------------------------------------------------
CPU_Meta_RequestBasicInfo::CPU_Meta_RequestBasicInfo(const string& accountId)
  : CPU_CommonFamily(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA)
{
  m_nMetaCommand = ICQ_CMDxMETA_REQUESTxBASICxINFO;
  myAccountId = accountId;

  m_nSize += 20;

  InitBuffer();

  buffer->PackUnsignedLongBE(0x0001000e); // TLV

  buffer->PackUnsignedShort(0x000c); // Bytes remaining
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedShort(m_nMetaCommand);
  buffer->PackUnsignedShort(m_nSubSequence);
  buffer->PackUnsignedLong(strtoul(myAccountId.c_str(), (char **)NULL, 10));
}

//-----RequestInfo-------------------------------------------------------------
CPU_RequestInfo::CPU_RequestInfo(const string& accountId)
  : CPU_CommonFamily(ICQ_SNACxFAM_LOCATION, ICQ_SNACxREQUESTxUSERxINFO)
{
  m_nSize += 5 + accountId.size();

  InitBuffer();

  buffer->PackUnsignedLongBE(0x00000003);
  buffer->PackChar(accountId.size());
  buffer->pack(accountId);
}

//-----AIMFetchAwayMessage-----------------------------------------------------
CPU_AIMFetchAwayMessage::CPU_AIMFetchAwayMessage(const string& accountId)
  : CPU_CommonFamily(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_INFOxREQ)
{
  m_nSize += 3 + accountId.size();

  InitBuffer();

  buffer->PackUnsignedShortBE(0x0003); // away message type
  buffer->PackChar(accountId.size());
  buffer->pack(accountId);
}

CPacketTcp_Handshake_v2::CPacketTcp_Handshake_v2(unsigned long nLocalPort)
{
  m_nLocalPort = nLocalPort;

  m_nSize = 26;
  buffer = new CBuffer(m_nSize);

  buffer->PackChar(ICQ_CMDxTCP_HANDSHAKE);
  buffer->PackUnsignedLong(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(m_nLocalPort);
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(gIcqProtocol.directMode() ? MODE_DIRECT : MODE_INDIRECT);
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
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
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

  m_nSize = 44;
  buffer = new CBuffer(m_nSize);

  buffer->PackChar(ICQ_CMDxTCP_HANDSHAKE);
  buffer->PackUnsignedShort(ICQ_VERSION_TCP);
  buffer->PackUnsignedShort(0x0027); //size
  buffer->PackUnsignedLong(m_nDestinationUin);
  buffer->PackUnsignedShort(0);
  buffer->PackUnsignedLong(nLocalPort);
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(gIcqProtocol.directMode() ? MODE_DIRECT : MODE_INDIRECT);
  buffer->PackUnsignedLong(nLocalPort == 0 ? s_nLocalPort : nLocalPort);

  char id[16];
  snprintf(id, 16, "%lu", nDestinationUin);
  UserId userId(id, LICQ_PPID);
  Licq::UserReadGuard u(userId);
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

  m_nSize = 48;
  buffer = new CBuffer(m_nSize);

  buffer->PackChar(ICQ_CMDxTCP_HANDSHAKE);
  buffer->PackUnsignedShort(ICQ_VERSION_TCP);
  buffer->PackUnsignedShort(0x002b); // size
  buffer->PackUnsignedLong(m_nDestinationUin);
  buffer->PackUnsignedShort(0);
  buffer->PackUnsignedLong(nLocalPort == 0 ? s_nLocalPort : nLocalPort);
  buffer->PackUnsignedLong(gUserManager.icqOwnerUin());
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackChar(gIcqProtocol.directMode() ? MODE_DIRECT : MODE_INDIRECT);
  buffer->PackUnsignedLong(nLocalPort == 0 ? s_nLocalPort : nLocalPort);

  char id[16];
  snprintf(id, 16, "%lu", nDestinationUin);
  UserId userId(id, LICQ_PPID);
  Licq::UserReadGuard u(userId);
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
  m_nSize = 4;
  buffer = new CBuffer(4);
  buffer->PackUnsignedLong(1);
}

CPacketTcp_Handshake_Confirm::CPacketTcp_Handshake_Confirm(int channel,
  unsigned short nSequence)
  : myChannel(channel)
{
  m_nSize = 33;
  buffer = new CBuffer(m_nSize);

  const char *GUID;
  unsigned long nOurId;
  switch (channel)
  {
    case Licq::TCPSocket::ChannelNormal:
    nOurId = 0x00000001;
    GUID = PLUGIN_NORMAL;
      break;
    case Licq::TCPSocket::ChannelInfo:
    nOurId = 0x000003EB;
    GUID = PLUGIN_INFOxMANAGER;
      break;
    case Licq::TCPSocket::ChannelStatus:
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
    myChannel = Licq::TCPSocket::ChannelNormal;
  else if (memcmp(GUID, PLUGIN_INFOxMANAGER, 16) == 0)
    myChannel = Licq::TCPSocket::ChannelInfo;
  else if (memcmp(GUID, PLUGIN_STATUSxMANAGER, 16) == 0)
    myChannel = Licq::TCPSocket::ChannelStatus;
  else
  {
    gLog.warning(tr("Unknown channel GUID."));
    myChannel = Licq::TCPSocket::ChannelUnknown;
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
    const string& message, bool _bAccept, unsigned short nLevel, Licq::User* user)
  : myChannel(channel)
{
  // Setup the message type and status fields using our online status
  Licq::OwnerReadGuard o(LICQ_PPID);
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

  m_nSourceUin = gUserManager.icqOwnerUin();
  m_nCommand = _nCommand;
  m_nSubCommand = _nSubCommand;
  myMessage = message;
  m_nLocalPort = user->LocalPort();

  // don't increment the sequence if this is an ack and cancel packet
  if (m_nCommand == ICQ_CMDxTCP_START) m_nSequence = user->Sequence(true);

  // v4,6 packets are smaller then v2 so we just set the size based on a v2 packet
  m_nSize = 18 + myMessage.size()+1 + 25;
  buffer = NULL;
}

CPacketTcp::~CPacketTcp()
{
  // Empty
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
}


void CPacketTcp::InitBuffer_v2()
{
  buffer = new CBuffer(m_nSize + 4);

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
  buffer = new CBuffer(m_nSize + 8);

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
  buffer = new CBuffer(m_nSize + 4);

  buffer->PackChar(0x02);
  buffer->PackUnsignedLong(0); // Checksum
  buffer->PackUnsignedShort(m_nCommand);
  buffer->PackUnsignedShort((channel() == Licq::TCPSocket::ChannelNormal) ? 0x000E : 0x0012);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedShort(m_nSubCommand);
  buffer->PackUnsignedShort(m_nStatus);
  buffer->PackUnsignedShort((channel() == Licq::TCPSocket::ChannelNormal) ? m_nMsgType : m_nLevel);

  if (channel() == Licq::TCPSocket::ChannelNormal)
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
    const Licq::Color* pColor, ICQUser *pUser, bool isUtf8)
  : CPacketTcp(ICQ_CMDxTCP_START,
       ICQ_CMDxSUB_MSG | (bMR ? ICQ_CMDxSUB_FxMULTIREC : 0),
        Licq::TCPSocket::ChannelNormal,
        message, true, nLevel, pUser)
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
    const Licq::Color* pColor, ICQUser *pUser)
  : CPacketTcp(ICQ_CMDxTCP_START,
       ICQ_CMDxSUB_URL | (bMR ? ICQ_CMDxSUB_FxMULTIREC : 0),
        Licq::TCPSocket::ChannelNormal,
        message, true, nLevel, pUser)
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
      buffer->PackUnsignedLong(pColor->foreground());
      buffer->PackUnsignedLong(pColor->background());
    }
  }
  PostBuffer();
}


//-----ContactList-----------------------------------------------------------
CPT_ContactList::CPT_ContactList(const string& message, unsigned short nLevel, bool bMR,
    const Licq::Color* pColor, ICQUser *pUser)
  : CPacketTcp(ICQ_CMDxTCP_START,
       ICQ_CMDxSUB_CONTACTxLIST | (bMR ? ICQ_CMDxSUB_FxMULTIREC : 0),
        Licq::TCPSocket::ChannelNormal,
        message, true, nLevel, pUser)
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
      buffer->PackUnsignedLong(pColor->foreground());
      buffer->PackUnsignedLong(pColor->background());
    }
  }
  PostBuffer();
}


//-----ReadAwayMessage----------------------------------------------------------
CPT_ReadAwayMessage::CPT_ReadAwayMessage(ICQUser *_cUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxTCP_READxAWAYxMSG,
        Licq::TCPSocket::ChannelNormal, "", true, ICQ_TCPxMSG_AUTOxREPLY, _cUser)
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
   unsigned short nPort, unsigned short nLevel, ICQUser *pUser, bool bICBM)
  : CPacketTcp(ICQ_CMDxTCP_START, bICBM ? ICQ_CMDxSUB_ICBM : ICQ_CMDxSUB_CHAT,
        Licq::TCPSocket::ChannelNormal,
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
    buffer->PackUnsignedLong(ReversePort(nPort));
    buffer->PackUnsignedLong(nPort);
  }

  PostBuffer();
}


//-----FileTransfer--------------------------------------------------------------
CPT_FileTransfer::CPT_FileTransfer(const list<string>& lFileList, const string& filename,
    const string& description, unsigned short nLevel, ICQUser *_cUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_FILE, Licq::TCPSocket::ChannelNormal,
        description, true, nLevel, _cUser),
    CPX_FileTransfer(lFileList, filename)
{
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
CPT_OpenSecureChannel::CPT_OpenSecureChannel(ICQUser *_cUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_SECURExOPEN,
        Licq::TCPSocket::ChannelNormal,
       "", true, ICQ_TCPxMSG_NORMAL, _cUser)
{
  InitBuffer();
  PostBuffer();
}


CPT_CloseSecureChannel::CPT_CloseSecureChannel(ICQUser *_cUser)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_SECURExCLOSE,
        Licq::TCPSocket::ChannelNormal,
       "", true, ICQ_TCPxMSG_NORMAL, _cUser)
{
  InitBuffer();
  PostBuffer();
}


//+++++Ack++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
string pipeInput(const string& message)
{
  string m(message);
  size_t posPipe = 0;

  while (true)
  {
    posPipe = m.find('|', posPipe);
    if (posPipe == string::npos)
      break;

    if (posPipe != 0 && m[posPipe-1] != '\n')
    {
      // Pipe char isn't at begining of a line, ignore it
      ++posPipe;
      continue;
    }

    // Find end of command
    size_t posEnd = m.find('\r', posPipe+1);
    if (posEnd == string::npos)
      posEnd = m.size();
    size_t cmdLen = posEnd - posPipe - 2;

    string cmd(m, posPipe+1, cmdLen);
    string cmdOutput;
    Licq::UtilityInternalWindow win;
    if (!win.POpen(cmd))
    {
      gLog.warning(tr("Could not execute \"%s\" for auto-response."), cmd.c_str());
    }
    else
    {
      int c;
      while ((c = fgetc(win.StdOut())) != EOF)
      {
        if (c == '\n')
          cmdOutput += '\r';
        cmdOutput += c;
      }

      int i;
      if ((i = win.PClose()) != 0)
      {
        gLog.warning(tr("%s returned abnormally: exit code %d."), cmd.c_str(), i);
        // do anything to cmdOutput ???
      }
    }

    m.replace(posPipe, cmdLen + 1, cmdOutput);
    posPipe += cmdOutput.size() + 1;
  }

  return m;
}



CPT_Ack::CPT_Ack(unsigned short _nSubCommand, unsigned short _nSequence,
                bool _bAccept, bool l, ICQUser *pUser)
  : CPacketTcp(ICQ_CMDxTCP_ACK, _nSubCommand, Licq::TCPSocket::ChannelNormal,
      "", _bAccept, l ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL, pUser)
{
  m_nSequence = _nSequence;
  Licq::OwnerReadGuard o(LICQ_PPID);

  m_nSize -= myMessage.size();

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
      myMessage += pUser->usprintf(pUser->customAutoResponse().c_str(), Licq::User::usprintf_quotepipe, true);
    }
  }
  else
    myMessage.clear();

  // Check for pipes, should possibly go after the ClientToServer call
  myMessage = pipeInput(myMessage);

  myMessage = gTranslator.clientToServer(myMessage);

  m_nSize += myMessage.size();
}

CPT_Ack::~CPT_Ack()
{
  // Empty
}

//-----AckGeneral---------------------------------------------------------------
CPT_AckGeneral::CPT_AckGeneral(unsigned short nCmd, unsigned short nSequence,
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
CPT_AckOpenSecureChannel::CPT_AckOpenSecureChannel(unsigned short nSequence,
   bool ok, ICQUser *pUser)
  : CPT_Ack(ICQ_CMDxSUB_SECURExOPEN, nSequence, true, true, pUser)
{
  m_nSize -= myMessage.size();
  myMessage = (ok ? "1" : "");
  m_nSize += myMessage.size()+1;

  InitBuffer();
  PostBuffer();
}


CPT_AckOldSecureChannel::CPT_AckOldSecureChannel(unsigned short nSequence,
   ICQUser *pUser)
  : CPT_Ack(ICQ_CMDxSUB_SECURExOPEN, nSequence, true, true, pUser)
{
  m_nSize -= myMessage.size();
  myMessage.clear();
  m_nSize += myMessage.size() + 1;

  InitBuffer();
  if (m_nVersion == 6)
  {
    buffer->PackUnsignedLong(0x00000000);
    buffer->PackUnsignedLong(0x00000000);
  }
  PostBuffer();
}


CPT_AckCloseSecureChannel::CPT_AckCloseSecureChannel(unsigned short nSequence,
   ICQUser *pUser)
  : CPT_Ack(ICQ_CMDxSUB_SECURExCLOSE, nSequence, true, true, pUser)
{
  m_nSize -= myMessage.size();
  myMessage.clear();
  m_nSize += myMessage.size() + 1;

  InitBuffer();
  PostBuffer();
}



#if 0
//-----AckMessage---------------------------------------------------------------
CPT_AckMessage::CPT_AckMessage(unsigned short _nSequence, bool _bAccept,
                               bool nLevel, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_MSG, _nSequence, _bAccept, nLevel, _cUser)
{
  InitBuffer();
  PostBuffer();
}



//-----AckReadAwayMessage-------------------------------------------------------
CPT_AckReadAwayMessage::CPT_AckReadAwayMessage(unsigned short _nSubCommand,
                                               unsigned short _nSequence,
                                               bool _bAccept, ICQUser *_cUser)
  : CPT_Ack(_nSubCommand, _nSequence, _bAccept, false, _cUser)
{
  InitBuffer();
  PostBuffer();
}


//-----AckUrl-------------------------------------------------------------------
CPT_AckUrl::CPT_AckUrl(unsigned short _nSequence, bool _bAccept, bool nLevel,
                      ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_URL, _nSequence, _bAccept, nLevel, _cUser)
{
  InitBuffer();
  PostBuffer();
}


//-----AckContactList--------------------------------------------------------
CPT_AckContactList::CPT_AckContactList(unsigned short _nSequence, bool _bAccept,
                                       bool nLevel, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_CONTACTxLIST, _nSequence, _bAccept, nLevel, _cUser)
{
  InitBuffer();
  PostBuffer();
}
#endif

//-----AckChatRefuse------------------------------------------------------------
CPT_AckChatRefuse::CPT_AckChatRefuse(const string& reason,
   unsigned short _nSequence, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_CHAT, _nSequence, false, false, _cUser)
{
  myMessage = reason;
  char temp_1[11] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  m_nSize += 11 + myMessage.size();
  InitBuffer();
  buffer->Pack(temp_1, 11);
  PostBuffer();
}


//-----AckChatAccept------------------------------------------------------------
CPT_AckChatAccept::CPT_AckChatAccept(unsigned short _nPort, const string& clients,
                                    unsigned short _nSequence, ICQUser *_cUser,
                                    bool bICBM)
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
    buffer->PackUnsignedLong(ReversePort(m_nPort));
    buffer->PackUnsignedLong(m_nPort);
  }

  PostBuffer();
}


//-----AckFileRefuse------------------------------------------------------------
CPT_AckFileRefuse::CPT_AckFileRefuse(const string& reason,
                                    unsigned short _nSequence, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_FILE, _nSequence, false, false, _cUser)
{
  myMessage = reason;

  m_nSize += 15 + myMessage.size();
  InitBuffer();

  buffer->PackUnsignedLong(0);
  buffer->PackString("");
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(0);

  PostBuffer();
}


//-----AckFileAccept------------------------------------------------------------
CPT_AckFileAccept::CPT_AckFileAccept(unsigned short _nPort,
                                    unsigned short _nSequence, ICQUser *_cUser)
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
CPT_Cancel::CPT_Cancel(unsigned short _nSubCommand, unsigned short _nSequence,
                      ICQUser *_cUser)
  : CPacketTcp(ICQ_CMDxTCP_CANCEL, _nSubCommand, Licq::TCPSocket::ChannelNormal, "", true, 0, _cUser)
{
  m_nSequence = _nSequence;
}



//-----CancelChat---------------------------------------------------------------
CPT_CancelChat::CPT_CancelChat(unsigned short _nSequence, ICQUser *_cUser)
  : CPT_Cancel(ICQ_CMDxSUB_CHAT, _nSequence, _cUser)
{
  char temp_1[11] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  m_nSize += 11;
  InitBuffer();
  buffer->Pack(temp_1, 11);
  PostBuffer();
}


//-----CancelFile---------------------------------------------------------------
CPT_CancelFile::CPT_CancelFile(unsigned short _nSequence, ICQUser *_cUser)
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
CPT_PluginError::CPT_PluginError(ICQUser *_cUser, unsigned short nSequence,
    int channel)
  : CPacketTcp(ICQ_CMDxTCP_ACK, 0, channel, "\x03", true, 0, _cUser)
{
  m_nSequence = nSequence;

  InitBuffer();
  PostBuffer();
}

//-----Send info plugin request------------------------------------------------
CPT_InfoPluginReq::CPT_InfoPluginReq(ICQUser *_cUser, const char *GUID,
  unsigned long nTime)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_MSG, Licq::TCPSocket::ChannelInfo, "", true, 0, _cUser)
{
  m_nSize += 22;
  memcpy(m_ReqGUID, GUID, GUID_LENGTH);

  InitBuffer();

  buffer->Pack(GUID, GUID_LENGTH);

  buffer->PackUnsignedLong(nTime);

  PostBuffer();
}

//----Reply to phone book request-----------------------------------------------
CPT_InfoPhoneBookResp::CPT_InfoPhoneBookResp(ICQUser *_cUser,
  unsigned short nSequence)
  : CPacketTcp(ICQ_CMDxTCP_ACK, 0, Licq::TCPSocket::ChannelInfo, "\x01", true, ICQ_TCPxMSG_URGENT2, _cUser)
{
  Licq::OwnerReadGuard o(LICQ_PPID);
  const Licq::ICQUserPhoneBook* book = o->GetPhoneBook();

  unsigned long num_entries;
  unsigned long nLen = 4 + 4;
  const struct Licq::PhoneBookEntry* entry;
  for (num_entries = 0; book->Get(num_entries, &entry); num_entries++)
  {
    nLen += 4 + entry->description.size() + 4 + entry->areaCode.size()
        + 4 + entry->phoneNumber.size() + 4 + entry->extension.size()
        + 4 + entry->country.size() + 4 + 4 + 4
        + 4 + entry->gateway.size() + 4 + 4 + 4 + 4;
  }

  m_nSize += 2 + 2 + 4 + 4 + nLen;
  m_nSequence = nSequence;
  InitBuffer();

  buffer->PackUnsignedShort(0);   //Unknown
  buffer->PackUnsignedShort(1);   //Unknown
  buffer->PackUnsignedLong(o->ClientInfoTimestamp());
  buffer->PackUnsignedLong(nLen);  //Bytes remaining in packet

  buffer->PackUnsignedLong(ICQ_PLUGIN_RESP_PHONExBOOK); //Response ID

  buffer->PackUnsignedLong(num_entries);

  for (unsigned long i = 0; book->Get(i, &entry); i++)
  {
    buffer->PackUnsignedLong(entry->description.size());
    buffer->pack(entry->description);

    buffer->PackUnsignedLong(entry->areaCode.size());
    buffer->pack(entry->areaCode);

    buffer->PackUnsignedLong(entry->phoneNumber.size());
    buffer->pack(entry->phoneNumber);

    buffer->PackUnsignedLong(entry->extension.size());
    buffer->pack(entry->extension);

    buffer->PackUnsignedLong(entry->country.size());
    buffer->pack(entry->country);

    buffer->PackUnsignedLong(entry->nActive);
  }

  for (unsigned long i = 0; book->Get(i, &entry); i++)
  {
    unsigned long sLen = entry->gateway.size();
    buffer->PackUnsignedLong(4 + 4 + sLen + 4 + 4 + 4 + 4);

    buffer->PackUnsignedLong(entry->nType);

    buffer->PackUnsignedLong(sLen);
    buffer->pack(entry->gateway);

    buffer->PackUnsignedLong(entry->nGatewayType);
    buffer->PackUnsignedLong(entry->nSmsAvailable);
    buffer->PackUnsignedLong(entry->nRemoveLeading0s);
    buffer->PackUnsignedLong(entry->nPublish);
  }

  PostBuffer();
}

//----Reply to picture request--------------------------------------------------
CPT_InfoPictureResp::CPT_InfoPictureResp(ICQUser *_cUser,
  unsigned short nSequence)
  : CPacketTcp(ICQ_CMDxTCP_ACK, 0, Licq::TCPSocket::ChannelInfo, "\x01", true, ICQ_TCPxMSG_URGENT2, _cUser)
{
  Licq::OwnerReadGuard o(LICQ_PPID);
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
CPT_InfoPluginListResp::CPT_InfoPluginListResp(ICQUser *_cUser,
  unsigned short nSequence)
  : CPacketTcp(ICQ_CMDxTCP_ACK, 0, Licq::TCPSocket::ChannelInfo, "\x01", true, ICQ_TCPxMSG_URGENT2, _cUser)
{
  unsigned long num_plugins = sizeof(info_plugins)/sizeof(struct PluginList);

  unsigned long nLen;
  if (num_plugins == 0)
    nLen = 0;
  else
  {
    nLen = 4 + 4;
    for (unsigned long i = 0; i < num_plugins; i ++)
    {
      nLen += GUID_LENGTH + 2 + 2 + 4 + strlen(info_plugins[i].name) 
              + 4 + strlen(info_plugins[i].description) + 4;
    }
  }

  m_nSize += 2 + 2 + 4 + 4 + nLen;
  m_nSequence = nSequence;
  InitBuffer();

  buffer->PackUnsignedShort(0);   //Unknown
  buffer->PackUnsignedShort(1);   //Unknown
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    buffer->PackUnsignedLong(o->ClientInfoTimestamp());
  }
  buffer->PackUnsignedLong(nLen);  //Bytes remaining in packet
  if (nLen != 0)
  {
    buffer->PackUnsignedLong(ICQ_PLUGIN_RESP_INFOxLIST); //Response ID
    buffer->PackUnsignedLong(num_plugins);
    for (unsigned long i = 0; i < num_plugins; i++)
    {
      buffer->Pack(info_plugins[i].guid, GUID_LENGTH);

      buffer->PackUnsignedShort(0); //Unknown
      buffer->PackUnsignedShort(1); //Unknown

      unsigned int l = strlen(info_plugins[i].name);
      buffer->PackUnsignedLong(l);
      buffer->Pack(info_plugins[i].name, l);

      l = strlen(info_plugins[i].description);
      buffer->PackUnsignedLong(l);
      buffer->Pack(info_plugins[i].description, l);

      buffer->PackUnsignedLong(0);  //Unknown
    }
  }
  PostBuffer();
}

//-----Send status plugin request----------------------------------------------
CPT_StatusPluginReq::CPT_StatusPluginReq(ICQUser *_cUser, const char *GUID,
  unsigned long nTime)
  : CPacketTcp(ICQ_CMDxTCP_START, ICQ_CMDxSUB_MSG, Licq::TCPSocket::ChannelStatus, "", true, 0, _cUser)
{
  m_nSize += 22;
  memcpy(m_ReqGUID, GUID, GUID_LENGTH);

  InitBuffer();

  buffer->Pack(GUID, GUID_LENGTH);

  buffer->PackUnsignedLong(nTime);

  PostBuffer();
}

//----Reply to plugin list request----------------------------------------------
CPT_StatusPluginListResp::CPT_StatusPluginListResp(ICQUser *_cUser,
  unsigned short nSequence)
  : CPacketTcp(ICQ_CMDxTCP_ACK, 0, Licq::TCPSocket::ChannelStatus, "\x01", true, 0, _cUser)
{
  unsigned long num_plugins = sizeof(status_plugins)/sizeof(struct PluginList);

  unsigned long nLen;
  if (num_plugins == 0)
    nLen = 0;
  else
  {
    nLen = 4 + 4;
    for (unsigned long i = 0; i < num_plugins; i ++)
    {
      nLen += GUID_LENGTH + 2 + 2 + 4 + strlen(status_plugins[i].name) 
              + 4 + strlen(status_plugins[i].description) + 4;
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
    Licq::OwnerReadGuard o(LICQ_PPID);
    buffer->PackUnsignedLong(o->ClientStatusTimestamp());
  }
  buffer->PackUnsignedLong(nLen);  //Bytes remaining in packet
  if (nLen != 0)
  {
    buffer->PackUnsignedLong(ICQ_PLUGIN_RESP_STATUSxLIST); //Response ID
    buffer->PackUnsignedLong(num_plugins);
    for (unsigned long i = 0; i < num_plugins; i++)
    {
      buffer->Pack(status_plugins[i].guid, GUID_LENGTH);

      buffer->PackUnsignedShort(0); //Unknown
      buffer->PackUnsignedShort(1); //Unknown

      unsigned int l = strlen(status_plugins[i].name);
      buffer->PackUnsignedLong(l);
      buffer->Pack(status_plugins[i].name, l);

      l = strlen(status_plugins[i].description);
      buffer->PackUnsignedLong(l);
      buffer->Pack(status_plugins[i].description, l);

      buffer->PackUnsignedLong(0);  //Unknown
    }
  }
  PostBuffer();
}

//----Reply to status request--------------------------------------------------
CPT_StatusPluginResp::CPT_StatusPluginResp(ICQUser *_cUser,
  unsigned short nSequence,
  unsigned long nStatus)
  : CPacketTcp(ICQ_CMDxTCP_ACK, 0, Licq::TCPSocket::ChannelStatus, "\x02", true, 0, _cUser)
{
  m_nSize += 2 + 2 + 4 + 4 + 1;
  m_nSequence = nSequence;
  InitBuffer();

  buffer->PackUnsignedShort(0);   //Unknown
  buffer->PackUnsignedShort(1);   //Unknown
  buffer->PackUnsignedLong(nStatus);
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    buffer->PackUnsignedLong(o->ClientStatusTimestamp());
  }
  buffer->PackChar(1);            //Unknown

  PostBuffer();
}

// Connection independent base classes

//-----FileTransfer------------------------------------------------------------
CPX_FileTransfer::CPX_FileTransfer(const list<string>& lFileList, const string& filename)
  : m_lFileList(lFileList.begin(), lFileList.end())
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
}

CPX_FileTransfer::~CPX_FileTransfer()
{
  // Empty
}
