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

#include "icqpacket.h"
#include "icq-defines.h"
#include "translate.h"
#include "log.h"

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
    gLog.Packet("%sUnencrypted Packet (%d bytes):\n%s\n", L_PACKETxSTR, l,
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


// No client encryption yet
void Encrypt_Client(CBuffer *buffer)
{
}


unsigned long CPacket::s_nLocalIp = 0;
unsigned long CPacket::s_nRealIp = 0;
char CPacket::s_nMode = MODE_DIRECT;

//=====UDP======================================================================
unsigned short CPacketUdp::s_nSequence = 0;
unsigned short CPacketUdp::s_nSubSequence = 0;
unsigned long  CPacketUdp::s_nSessionId = 0;


CBuffer *CPacketUdp::Finalize(void)
{
  CBuffer *newbuf = new CBuffer(getBuffer());
  Encrypt_Server(newbuf);
  return newbuf;
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

  m_nSourceUin = gUserManager.OwnerUin();

  buffer = NULL;
#if ICQ_VERSION == 2
  m_nSize = 10;
#elif ICQ_VERSION == 4
  m_nSize = 20;
#elif ICQ_VERSION == 5
  m_nSize = 24;
#endif
}


CPacketUdp::~CPacketUdp(void)
{
  if (buffer != NULL) delete buffer;
}


void CPacketUdp::InitBuffer(void)
{
  buffer = new CBuffer(m_nSize + 8);
#if ICQ_VERSION == 2
  buffer->PackUnsignedShort(m_nVersion);
  buffer->PackUnsignedShort(m_nCommand);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedLong(m_nSourceUin);
#elif ICQ_VERSION == 4
  buffer->add(m_nVersion);
  buffer->add(m_nRandom);
  buffer->add(m_nZero);
  buffer->add(m_nCommand);
  buffer->add(m_nSequence);
  buffer->add(m_nSubSequence);
  buffer->add(m_nSourceUin);
  buffer->add(m_nCheckSum);
#elif ICQ_VERSION == 5
  buffer->PackUnsignedShort(m_nVersion);
  buffer->PackUnsignedLong(m_nZero);
  buffer->PackUnsignedLong(m_nSourceUin);
  buffer->PackUnsignedLong(m_nSessionId);
  buffer->PackUnsignedShort(m_nCommand);
  buffer->PackUnsignedShort(m_nSequence);
  buffer->PackUnsignedShort(m_nSubSequence);
  buffer->PackUnsignedLong(m_nCheckSum);
#endif;
}

//-----Register--------------------------------------------------------------
#if ICQ_VERSION == 2
CPU_Register::CPU_Register(const char *_szPasswd)
{
  m_nVersion = ICQ_VERSION;
  m_nCommand = ICQ_CMDxSND_REGISTERxUSER;
  m_nSequence = 0x001;
  m_nPasswdLen = strlen(_szPasswd) + 1;
  m_szPasswd = strdup(_szPasswd);

  m_nSize = strlen (m_szPasswd) + 1 + 18
  buffer = new CBuffer(m_nSize);

  buffer->add(m_nVersion);
  buffer->add(m_nCommand);
  buffer->add(m_nSequence);
  buffer->add(0x002);
  buffer->add(m_nPasswdLen);
  buffer->add(m_szPasswd, m_nPasswdLen);
  buffer->add(0x72);
  buffer->add(0x00);
}

CPU_Register::~CPacketRegister(void)
{
  free (m_szPasswd);
  if (buffer != NULL) delete buffer;
}

#elif ICQ_VERSION == 4 || ICQ_VERSION == 5

CPU_Register::CPU_Register(const char *szPasswd)
  : CPacketUdp(ICQ_CMDxSND_REGISTERxUSER)
{
  m_nSize += strlen (szPasswd) + 1 + 20;
  InitBuffer();

  buffer = new CBuffer(m_nSize);
  buffer->PackUnsignedShort(0x0002);
  buffer->PackString(szPasswd);
  buffer->PackUnsignedLong(0x000000A0);
  buffer->PackUnsignedLong(0x00002461);
  buffer->PackUnsignedLong(0x00A00000);
  buffer->PackUnsignedLong(0x00000000);
}

#endif

//-----Logon--------------------------------------------------------------------
CPU_Logon::CPU_Logon(INetSocket *_s, const char *szPassword, unsigned short _nLogonStatus)
  : CPacketUdp(ICQ_CMDxSND_LOGON)
{
#if ICQ_VERSION == 2
  unsigned long nUnknown = 0x00040072;
  char temp[10] = { 2, 0, 0, 0, 0, 0, 4, 0, 0x72, 0 };
#elif ICQ_VERSION == 4
  unsigned long nUnknown = 0x98;
  char temp[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0x98, 0 };
#elif ICQ_VERSION == 5
  unsigned long nUnknown = 0xD5;
  char temp[20] = { 0x00, 0x00, 0x00, 0x00,
                    0xEC, 0x01, 0x2C, 0x82,
                    0x50, 0x00, 0x00, 0x00,
                    0x03, 0x00, 0x00, 0x00,
                    0x00, 0x16, 0xD6, 0x36 };
  s_nSessionId = rand() & 0x3FFFFFFF;
  s_nSequence = rand() & 0x7FFF;
  s_nSubSequence = 1;
  m_nSessionId = s_nSessionId;
  m_nSequence = s_nSequence++;
  m_nSubSequence = s_nSubSequence++;
#endif
  s_nRealIp = NetworkIpToPacketIp(_s->LocalIp());
  s_nLocalIp = NetworkIpToPacketIp(_s->LocalIp());
  s_nMode = MODE_DIRECT;
  m_nLocalPort = _s->LocalPort();
  m_nLogonStatus = _nLogonStatus;
  m_nTcpVersion = ICQ_VERSION_TCP;

#if ICQ_VERSION == 2
  m_nSize += 6 + strlen(szPassword) + 28;
#elif ICQ_VERSION == 4
  m_nSize += 6 + strlen(szPassword) + 32;
#elif ICQ_VERSION == 5
  m_nSize += 6 + strlen(szPassword) + 42;
#endif
  InitBuffer();

#if ICQ_VERSION == 4 || ICQ_VERSION == 5
  buffer->PackUnsignedLong(time(NULL));
#endif
  buffer->PackUnsignedLong(m_nLocalPort);
  buffer->PackString(szPassword);
  buffer->PackUnsignedLong(nUnknown);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackChar(s_nMode);
  buffer->PackUnsignedLong(m_nLogonStatus);
  buffer->PackUnsignedLong(m_nTcpVersion);
#if ICQ_VERSION == 2 || ICQ_VERSION == 4
  buffer->Pack(temp, 10);
#else
  buffer->Pack(temp, 20);
#endif
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

//-----GetUserBasicInfo---------------------------------------------------------
CPU_GetUserBasicInfo::CPU_GetUserBasicInfo(unsigned long _nUserUin)
  : CPacketUdp(ICQ_CMDxSND_USERxGETINFO)
{
  m_nUserUin = _nUserUin;

  m_nSize += 6;
  InitBuffer();

#if ICQ_VERSION == 2
  buffer->PackUnsignedShort(m_nSubSequence);
#endif
  buffer->PackUnsignedLong(m_nUserUin);
}



CPU_GetUserExtInfo::CPU_GetUserExtInfo(unsigned long _nUserUin)
  : CPacketUdp(ICQ_CMDxSND_USERxGETDETAILS)
{
  m_nUserUin = _nUserUin;

  m_nSize += 6;
  InitBuffer();

#if ICQ_VERSION == 2
  buffer->PackUnsignedShort(m_nSubSequence);
#endif
  buffer->PackUnsignedLong(m_nUserUin);
}


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
CPU_Logoff::CPU_Logoff(void) : CPacketUdp(ICQ_CMDxSND_LOGOFF)
{
  m_nSize += 24;
  InitBuffer();

  buffer->PackString("B_USER_DISCONNECTED");
  buffer->PackUnsignedShort(0x0005);
}


//-----ContactList--------------------------------------------------------------

CPU_ContactList::CPU_ContactList(UinList &uins)
  : CPacketUdp(ICQ_CMDxSND_USERxLIST)
{
  m_nSize += 1 + (uins.size() * sizeof(unsigned long));
  InitBuffer();

  buffer->PackChar(uins.size());
  for (unsigned short i  = 0; i < uins.size(); i++)
    buffer->PackUnsignedLong(uins[i]);
}

CPU_ContactList::CPU_ContactList(unsigned long _nUin)
  : CPacketUdp(ICQ_CMDxSND_USERxLIST)
{
  m_nSize += 5;
  InitBuffer();

  buffer->PackChar(0x01);
  buffer->PackUnsignedLong(_nUin);

}


//-----VisibleList--------------------------------------------------------------
CPU_VisibleList::CPU_VisibleList(UinList &uins)
  : CPacketUdp(ICQ_CMDxSND_VISIBLExLIST)
{
  m_nSize += 1 + (uins.size() * 4);
  InitBuffer();

  buffer->PackChar(uins.size());
  for (unsigned short i  = 0; i < uins.size(); i++)
    buffer->PackUnsignedLong(uins[i]);

}


//-----InvisibleList--------------------------------------------------------------
CPU_InvisibleList::CPU_InvisibleList(UinList &uins)
  : CPacketUdp(ICQ_CMDxSND_INVISIBLExLIST)
{
  m_nSize += 1 + (uins.size() * 4);
  InitBuffer();

  buffer->PackChar(uins.size());
  for (unsigned short i  = 0; i < uins.size(); i++)
    buffer->PackUnsignedLong(uins[i]);

}



//-----StartSearch--------------------------------------------------------------
CPU_StartSearch::CPU_StartSearch(const char *szAlias, const char *szFirstName,
                                const char *szLastName, const char *szEmail)
  : CPacketUdp(ICQ_CMDxSND_SEARCHxSTART)
{

  m_nSize += 14 + strlen(szAlias) + strlen(szFirstName) +
             strlen(szLastName) + strlen(szEmail);
  InitBuffer();

#if ICQ_VERSION == 2
  buffer->PackUnsignedShort(m_nSubSequence);
#endif
  buffer->PackString(szAlias);
  buffer->PackString(szFirstName);
  buffer->PackString(szLastName);
  buffer->PackString(szEmail);

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
  time_t t = time(NULL);
  localtime(&t);
  m_cTimezone = timezone / 1800;
  if (m_cTimezone > 23) m_cTimezone = 23 - m_cTimezone;
  m_nAge = nAge;
  m_cSex = cSex;
  m_nZipcode = nZipcode;

  m_nSize += strlen(szCity) + strlen(szState) +
             strlen(szPhone) + strlen(szHomepage) + strlen(szAbout) + 27;
  InitBuffer();

#if ICQ_VERSION == 2
  buffer->PackUnsignedShort(m_nSubSequence);
#endif
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
CPU_Ping::CPU_Ping(void) : CPacketUdp(ICQ_CMDxSND_PING)
{
  InitBuffer();
}


//-----ThroughServer------------------------------------------------------------
CPU_ThroughServer::CPU_ThroughServer(unsigned long nSourceUin,
                                    unsigned long nDestinationUin,
                                    unsigned short nSubCommand,
                                    char *szMessage)
  : CPacketUdp(ICQ_CMDxSND_THRUxSERVER)
{
  m_nSourceUin = (nSourceUin == 0 ? gUserManager.OwnerUin() : nSourceUin);
  m_nSubCommand = nSubCommand;
  m_nDestinationUin = nDestinationUin;

  m_nSize += 8 + strlen(szMessage) + 1;
  InitBuffer();

  buffer->PackUnsignedLong(nDestinationUin);
  buffer->PackUnsignedShort(nSubCommand);
  buffer->PackString(szMessage);

}


//-----SetStatus----------------------------------------------------------------
CPU_SetStatus::CPU_SetStatus(unsigned long _nNewStatus) : CPacketUdp(ICQ_CMDxSND_SETxSTATUS)
{
  m_nNewStatus = _nNewStatus;

  m_nSize += 4;
  InitBuffer();

  buffer->PackUnsignedLong(m_nNewStatus);
}

//-----Authorize----------------------------------------------------------------
CPU_Authorize::CPU_Authorize(unsigned long nAuthorizeUin) : CPacketUdp(ICQ_CMDxSND_AUTHORIZE)
{
  char temp[5] = { 0x08, 0x00, 0x01, 0x00, 0x00 };

  m_nSize += 9;
  InitBuffer();

  buffer->PackUnsignedLong(nAuthorizeUin);
  buffer->Pack(temp, 5);
}


//-----RequestSysMsg------------------------------------------------------------
CPU_RequestSysMsg::CPU_RequestSysMsg(void) : CPacketUdp(ICQ_CMDxSND_SYSxMSGxREQ)
{
  InitBuffer();
}


//-----SysMsgDoneAck------------------------------------------------------------
#if ICQ_VERSION == 2
CPU_SysMsgDoneAck::CPU_SysMsgDoneAck(unsigned short _nSequence)
  : CPacketUdp(ICQ_CMDxSND_SYSxMSGxDONExACK)
{
  m_nSequence = _nSequence;
  InitBuffer();
}
#elif ICQ_VERSION == 4
CPU_SysMsgDoneAck::CPU_SysMsgDoneAck(unsigned short _nSequence, unsigned short _nSubSequence)
  : CPacketUdp(ICQ_CMDxSND_SYSxMSGxDONExACK)
{
  m_nSequence = _nSequence;
  m_nSubSequence = _nSubSequence;
  InitBuffer();
}
#elif ICQ_VERSION == 5
CPU_SysMsgDoneAck::CPU_SysMsgDoneAck(void)
  : CPacketUdp(ICQ_CMDxSND_SYSxMSGxDONExACK)
{
  InitBuffer();
}
#endif

//-----Meta_SetWorkInfo------------------------------------------------------
CPU_Meta_SetGeneralInfo::CPU_Meta_SetGeneralInfo(const char *szAlias,
                          const char *szFirstName, const char *szLastName,
                          const char *szEmail1, const char *szEmail2,
                          const char *szCity, const char *szState,
                          const char *szPhoneNumber, const char *szFaxNumber,
                          const char *szAddress, const char *szCellularNumber,
                          unsigned long nZipCode,
                          unsigned short nCountryCode,
                          bool bHideEmail)
  : CPacketUdp(ICQ_CMDxSND_META)
{
  m_nMetaCommand = ICQ_CMDxMETA_GENERALxINFOxSET;

  m_nZipCode = nZipCode;
  m_nCountryCode = nCountryCode;
  time_t t = time(NULL);
  localtime(&t);
  m_nTimezone = timezone / 1800;
  if (m_nTimezone > 23) m_nTimezone = 23 - m_nTimezone;
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  m_nAuthorization = o->GetAuthorization() ? 0 : 1;
  //m_nWebAware = o->GetWebAware() ? 1 : 0;
  m_nWebAware = 1;
  gUserManager.DropOwner();
  m_nHideEmail = bHideEmail ? 1 : 0;

  m_nSize += strlen(szAlias) + strlen(szFirstName) + strlen(szLastName) +
             strlen(szEmail1) + strlen(szEmail2) + strlen(szCity) +
             strlen(szState) + strlen(szPhoneNumber) +
             strlen(szFaxNumber) + strlen(szAddress) +
             strlen(szCellularNumber) + 33 + 12;
  InitBuffer();

  buffer->PackUnsignedShort(m_nMetaCommand);
  m_szAlias = buffer->PackString(szAlias);
  m_szFirstName = buffer->PackString(szFirstName);
  m_szLastName = buffer->PackString(szLastName);
  m_szEmail1 = buffer->PackString(szEmail1);
  m_szEmail2 = buffer->PackString(szEmail2);
  buffer->PackString(szEmail1);
  m_szCity = buffer->PackString(szCity);
  m_szState = buffer->PackString(szState);
  m_szPhoneNumber = buffer->PackString(szPhoneNumber);
  m_szFaxNumber = buffer->PackString(szFaxNumber);
  m_szAddress = buffer->PackString(szAddress);
  m_szCellularNumber = buffer->PackString(szCellularNumber);
  buffer->PackUnsignedLong(m_nZipCode);
  buffer->PackUnsignedShort(m_nCountryCode);
  buffer->PackChar(m_nTimezone);
  buffer->PackChar(m_nAuthorization);
  buffer->PackChar(m_nWebAware);
  buffer->PackChar(m_nHideEmail);

}


//-----Meta_SetMoreInfo------------------------------------------------------
CPU_Meta_SetMoreInfo::CPU_Meta_SetMoreInfo( unsigned short nAge,
                       char nGender,
                       const char *szHomepage,
                       char nBirthYear,
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

  m_nSize += strlen(szHomepage) + 14;
  InitBuffer();

  buffer->PackUnsignedShort(m_nMetaCommand);
  buffer->PackUnsignedShort(m_nAge);
  buffer->PackChar(nGender);
  m_szHomepage = buffer->PackString(szHomepage);
  buffer->PackChar(m_nBirthYear);
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

  m_nSize += strlen(szCity) + strlen(szState) + strlen(szPhoneNumber) +
             strlen(szFaxNumber) + strlen(szAddress) + strlen(szName) +
             strlen(szDepartment) + strlen(szPosition) +
             strlen(szHomepage) + 8 + 26;
  InitBuffer();

  buffer->PackUnsignedShort(m_nMetaCommand);
  m_szCity = buffer->PackString(szCity);
  m_szState = buffer->PackString(szState);
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

  m_nSize += strlen(szAbout) + 5;
  InitBuffer();

  buffer->PackUnsignedShort(m_nMetaCommand);
  m_szAbout = buffer->PackString(szAbout);

}


//-----Meta_SetSecurityInfo--------------------------------------------------
CPU_Meta_SetSecurityInfo::CPU_Meta_SetSecurityInfo(
    bool bAuthorization,
    bool bHideIp,
    bool bWebAware)
  : CPacketUdp(ICQ_CMDxSND_META)
{
  m_nMetaCommand = ICQ_CMDxMETA_SECURITYxSET;
  m_nAuthorization = bAuthorization ? 0 : 1;
  m_nHideIp =  bHideIp ? 1 : 0;
  m_nWebAware = bWebAware ? 1 : 0;

  m_nSize += 5;
  InitBuffer();

  buffer->PackUnsignedShort(m_nMetaCommand);
  buffer->PackChar(m_nAuthorization);
  buffer->PackChar(m_nHideIp);
  buffer->PackChar(m_nWebAware);

}


//-----Meta_RequestInfo------------------------------------------------------
CPU_Meta_RequestInfo::CPU_Meta_RequestInfo(unsigned long nUin)
  : CPacketUdp(ICQ_CMDxSND_META)
{
  m_nMetaCommand = ICQ_CMDxMETA_REQUESTxINFO;
  m_nUin = nUin;

  m_nSize += 6;
  InitBuffer();

  buffer->PackUnsignedShort(m_nMetaCommand);
  buffer->PackUnsignedLong(m_nUin);

}


//=====PacketTcp_Handshake======================================================
CPacketTcp_Handshake::~CPacketTcp_Handshake(void)
{
  if (buffer != NULL) delete buffer;
}


CPacketTcp_Handshake::CPacketTcp_Handshake(unsigned long _nLocalPort)
{
  m_nLocalPort = _nLocalPort;
  m_nSourceUin = gUserManager.OwnerUin();

  m_nSize = 26;
  InitBuffer();
}


void CPacketTcp_Handshake::InitBuffer(void)
{
  buffer = new CBuffer(m_nSize);

  buffer->PackChar(ICQ_CMDxTCP_HANDSHAKE);
  buffer->PackUnsignedLong(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(m_nLocalPort);
  buffer->PackUnsignedLong(m_nSourceUin);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(s_nMode);
  buffer->PackUnsignedLong(m_nLocalPort);
}


//=====PacketTcp================================================================
CBuffer *CPacketTcp::Finalize(void)
{
  CBuffer *newbuf = new CBuffer(getBuffer());
  Encrypt_Client(newbuf);
  return newbuf;
}

CPacketTcp::CPacketTcp(unsigned long _nSourceUin, unsigned long _nCommand,
                      unsigned short _nSubCommand, const char *szMessage,
                      bool _bAccept, bool _bUrgent, ICQUser *user)
{
  // Setup the message type and status fields using our online status
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  switch(_nCommand)
  {
  case ICQ_CMDxTCP_CANCEL:
  case ICQ_CMDxTCP_START:
   m_nStatus = 0;
   m_nMsgType = (_bUrgent ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL);
   switch (o->Status())
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

  case ICQ_CMDxTCP_ACK:
   m_nMsgType = ICQ_TCPxMSG_AUTOxREPLY;
   if (!_bAccept)
     m_nStatus = ICQ_TCPxACK_REFUSE;
   // If we are accepting a chat or file request then always say we are online
   else if (_bUrgent ||
            _nSubCommand == ICQ_CMDxSUB_CHAT ||
            _nSubCommand == ICQ_CMDxSUB_FILE)
     m_nStatus = ICQ_TCPxACK_ONLINE;
   else
   {
     switch (o->Status())
     {
     case ICQ_STATUS_AWAY: m_nStatus = ICQ_TCPxACK_AWAY; break;
     case ICQ_STATUS_NA: m_nStatus = ICQ_TCPxACK_NA; break;
     case ICQ_STATUS_DND: m_nStatus = ICQ_TCPxACK_DND; break;
     case ICQ_STATUS_OCCUPIED: m_nStatus = ICQ_TCPxACK_OCCUPIED; break;
     case ICQ_STATUS_ONLINE:
     case ICQ_STATUS_FREEFORCHAT:
     default: m_nStatus = ICQ_TCPxACK_ONLINE; break;
     }
   }
   break;
  }
  gUserManager.DropOwner();

  m_nSourceUin = (_nSourceUin == 0 ? gUserManager.OwnerUin() : _nSourceUin);
  m_nCommand = _nCommand;
  m_nSubCommand = _nSubCommand;
  m_szMessage = (szMessage == NULL ? strdup("") : strdup(szMessage));
  m_nLocalPort = 0;
  if (user->SocketDesc() != -1)
  {
    INetSocket *s = gSocketManager.FetchSocket(user->SocketDesc());
    if (s != NULL)
    {
      m_nLocalPort = s->LocalPort();
      gSocketManager.DropSocket(s);
    }
  }

  // don't increment the sequence if this is an ack and cancel packet
  if (m_nCommand == ICQ_CMDxTCP_START) m_nSequence = user->Sequence(true);

  m_nSize = 18 + strlen(m_szMessage) + 25;
  buffer = NULL;
}

CPacketTcp::~CPacketTcp(void)
{
  free (m_szMessage);
  if (buffer != NULL) delete buffer;
}


void CPacketTcp::InitBuffer(void)
{
  buffer = new CBuffer(m_nSize + 4);

  buffer->PackUnsignedLong(m_nSourceUin);
  buffer->PackUnsignedShort(ICQ_VERSION_TCP);
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

void CPacketTcp::PostBuffer(void)
{
  buffer->PackUnsignedLong(m_nSequence);
  buffer->PackChar('L');
  buffer->PackUnsignedShort(INT_VERSION);
}

//-----Message------------------------------------------------------------------
CPT_Message::CPT_Message(unsigned long _nSourceUin, char *_sMessage, bool _bUrgent,
                        ICQUser *_cUser)
  : CPacketTcp(_nSourceUin, ICQ_CMDxTCP_START, ICQ_CMDxSUB_MSG, _sMessage,
               true, _bUrgent, _cUser)
{
  InitBuffer();
  PostBuffer();
}

//-----Url----------------------------------------------------------------------
CPT_Url::CPT_Url(unsigned long _nSourceUin, char *_sMessage, bool _bUrgent,
                ICQUser *_cUser)
  : CPacketTcp(_nSourceUin, ICQ_CMDxTCP_START, ICQ_CMDxSUB_URL, _sMessage,
               true, _bUrgent, _cUser)
{
  InitBuffer();
  PostBuffer();
}


//-----ReadAwayMessage----------------------------------------------------------
CPT_ReadAwayMessage::CPT_ReadAwayMessage(unsigned long _nSourceUin, ICQUser *_cUser)
  : CPacketTcp(_nSourceUin, ICQ_CMDxTCP_START, ICQ_CMDxTCP_READxAWAYxMSG, "",
               true, false, _cUser)
{
  // Properly set the subcommand to get the correct away message
  switch(_cUser->Status())
  {
  case ICQ_STATUS_AWAY: m_nSubCommand = ICQ_CMDxTCP_READxAWAYxMSG; break;
  case ICQ_STATUS_NA: m_nSubCommand = ICQ_CMDxTCP_READxNAxMSG; break;
  case ICQ_STATUS_DND: m_nSubCommand = ICQ_CMDxTCP_READxDNDxMSG; break;
  case ICQ_STATUS_OCCUPIED: m_nSubCommand = ICQ_CMDxTCP_READxOCCUPIEDxMSG; break;
  default: m_nSubCommand = ICQ_CMDxTCP_READxAWAYxMSG; break;
  }

  InitBuffer();
  PostBuffer();
}

//-----ChatRequest--------------------------------------------------------------
CPT_ChatRequest::CPT_ChatRequest(unsigned long _nSourceUin, char *_sMessage,
                                bool _bUrgent, ICQUser *_cUser)
  : CPacketTcp(_nSourceUin, ICQ_CMDxTCP_START, ICQ_CMDxSUB_CHAT, _sMessage,
               true, _bUrgent, _cUser)
{
  char temp_1[11] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  m_nSize += 11;
  InitBuffer();
  buffer->Pack(temp_1, 11);
  PostBuffer();
}


//-----FileTransfer--------------------------------------------------------------
CPT_FileTransfer::CPT_FileTransfer(unsigned long _nSourceUin, const char *_szFilename,
                                  const char *_szDescription, bool _bUrgent,
                                  ICQUser *_cUser)
  : CPacketTcp(_nSourceUin, ICQ_CMDxTCP_START, ICQ_CMDxSUB_FILE, _szDescription,
               true, _bUrgent, _cUser)
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


//+++++Ack++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CPT_Ack::CPT_Ack(unsigned short _nSubCommand, unsigned long _nSequence,
                bool _bAccept, bool _bUrgent, ICQUser *_cUser)
  : CPacketTcp(0, ICQ_CMDxTCP_ACK, _nSubCommand, "", _bAccept, _bUrgent,
               _cUser)
{
  m_nSequence = _nSequence;
  free(m_szMessage);
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  m_szMessage = gTranslator.NToRN(o->AutoResponse());
  gUserManager.DropOwner();
  gTranslator.ClientToServer(m_szMessage);

  m_nSize += strlen(m_szMessage);
}


//-----AckMessage---------------------------------------------------------------
CPT_AckMessage::CPT_AckMessage(unsigned long _nSequence, bool _bAccept,
                              bool _bUrgent, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_MSG, _nSequence, _bAccept, _bUrgent, _cUser)
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
CPT_AckUrl::CPT_AckUrl(unsigned long _nSequence, bool _bAccept, bool _bUrgent,
                      ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_URL, _nSequence, _bAccept, _bUrgent, _cUser)
{
  InitBuffer();
  PostBuffer();
}


//-----AckUrl-------------------------------------------------------------------
CPT_AckContactList::CPT_AckContactList(unsigned long _nSequence, bool _bAccept,
                                      bool _bUrgent, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_CONTACTxLIST, _nSequence, _bAccept, _bUrgent, _cUser)
{
  InitBuffer();
  PostBuffer();
}


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
  buffer->PackUnsignedLong( ((_nPort & 0xFF) << 8) + ((_nPort >> 8) & 0xFF) );
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

  buffer->PackUnsignedLong( ((_nPort & 0xFF) << 8) + ((_nPort >> 8) & 0xFF) );
  buffer->PackString("");
  buffer->PackUnsignedLong(m_nFileSize);
  buffer->PackUnsignedLong(m_nPort);

  PostBuffer();
}


//+++++Cancel+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CPT_Cancel::CPT_Cancel(unsigned short _nSubCommand, unsigned long _nSequence,
                      ICQUser *_cUser)
  : CPacketTcp(0, ICQ_CMDxTCP_CANCEL, _nSubCommand, "", true, false, _cUser)
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


//=====Chat=====================================================================
void CPacketChat::InitBuffer(void)
{
  buffer = new CBuffer(m_nSize);
}

//-----ChatColor----------------------------------------------------------------
CPChat_Color::CPChat_Color(char *_sLocalName, unsigned short _nLocalPort,
                          unsigned long _nColorForeground,
                          unsigned long _nColorBackground)
{
  m_nSize = 10 + strlen(_sLocalName) + 16;
  InitBuffer();

  buffer->PackUnsignedLong(0x64);
  buffer->PackUnsignedLong(0xFFFFFFFD);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackUnsignedShort( ((_nLocalPort & 0xFF) << 8) + ((_nLocalPort >> 8) & 0xFF) );
  buffer->PackUnsignedLong(_nColorForeground);
  buffer->PackUnsignedLong(_nColorBackground);
  buffer->PackChar(0);

}


//-----ChatColorFont----------------------------------------------------------------
CPChat_ColorFont::CPChat_ColorFont(char *_sLocalName, unsigned short _nLocalPort,
                                  unsigned long _nColorForeground,
                                  unsigned long _nColorBackground,
                                  unsigned long _nFontSize,
                                  unsigned long _nFontFace, char *_sFontName)
{
  m_nSize = 10 + strlen(_sLocalName) + 35 + strlen(_sFontName) + 4;
  InitBuffer();

  buffer->PackUnsignedLong(0x64);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackString(_sLocalName);
  buffer->PackUnsignedLong(_nColorForeground);
  buffer->PackUnsignedLong(_nColorBackground);
  buffer->PackUnsignedLong(0x03);
  buffer->PackUnsignedLong(_nLocalPort);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(s_nMode);
  buffer->PackUnsignedShort(0x5A75);
  buffer->PackUnsignedLong(_nFontSize);
  buffer->PackUnsignedLong(_nFontFace);
  buffer->PackString(_sFontName);
  buffer->PackUnsignedShort(0x00);
  buffer->PackChar(0);
}



//-----ChatFont---------------------------------------------------------------------
CPChat_Font::CPChat_Font(unsigned short _nLocalPort, unsigned long _nFontSize,
                        unsigned long _nFontFace, char *_sFontName)
{
  char temp_3[3] = { 0, 0, 0 };

  m_nSize = 29 + strlen(_sFontName) + 4;
  InitBuffer();

  buffer->PackUnsignedLong(0x03);
  buffer->PackUnsignedLong(_nLocalPort);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(s_nMode);
  buffer->PackUnsignedShort(0x5A75);
  buffer->PackUnsignedLong(_nFontSize);
  buffer->PackUnsignedLong(_nFontFace);
  buffer->PackString(_sFontName);
  buffer->Pack(temp_3, 3);
}


//-----FileInitClient-----------------------------------------------------------
CPFile_InitClient::CPFile_InitClient(char *_szLocalName,
                                    unsigned long _nNumFiles,
                                    unsigned long _nTotalSize)
{
  m_nSize = 20 + strlen(_szLocalName);
  InitBuffer();

  buffer->PackChar(0);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(_nNumFiles);
  buffer->PackUnsignedLong(_nTotalSize);
  buffer->PackUnsignedLong(0x64);
  buffer->PackString(_szLocalName);
}


//-----FileInitServer-----------------------------------------------------------
CPFile_InitServer::CPFile_InitServer(char *_szLocalName)
{
  m_nSize = 8 + strlen(_szLocalName);
  InitBuffer();

  buffer->PackChar(1);
  buffer->PackUnsignedLong(0x64);
  buffer->PackString(_szLocalName);
}


//-----FileBatch----------------------------------------------------------------
CPFile_Info::CPFile_Info(const char *_szFileName)
{
  m_bValid = true;
  m_nError = 0;

  char *pcNoPath = NULL;
  struct stat buf;

  // Remove any path from the filename
  if ( (pcNoPath = strrchr(_szFileName, '/')) != NULL)
    m_szFileName = strdup(pcNoPath + 1);
  else
    m_szFileName = strdup(_szFileName);

  if (stat(_szFileName, &buf) < 0)
  {
    m_bValid = false;
    m_nError = errno;
    return;
  }
  m_nFileSize = buf.st_size;

  m_nSize = strlen(m_szFileName) + 21;
  InitBuffer();

  buffer->PackUnsignedShort(0x02);

  // Add all the file names
  buffer->PackString(m_szFileName);
  // Add the empty file name
  buffer->PackString("");
  //Add the file length
  buffer->PackUnsignedLong(m_nFileSize);
  buffer->PackUnsignedLong(0x00);
  buffer->PackUnsignedLong(0x64);
}


CPFile_Info::~CPFile_Info(void)
{
  free (m_szFileName);
}


//-----FileStart----------------------------------------------------------------
CPFile_Start::CPFile_Start(unsigned long _nFilePos)
{
  m_nSize = 13;
  InitBuffer();

  buffer->PackChar(0x03);
  buffer->PackUnsignedLong(_nFilePos);
  buffer->PackUnsignedLong(0x00);
  buffer->PackUnsignedLong(0x64);
}

