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


//=====UDP======================================================================
unsigned short CPacketUdp::s_nSequence = 0;
unsigned short CPacketUdp::s_nSubSequence = 0;
unsigned long  CPacketUdp::s_nSessionId = 0;


void CPacketUdp::Encrypt(void)
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
  m_nCheckSum = chk1 ^ chk2;
  unsigned long key = l * 0x66756b65 + m_nCheckSum;
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
  buf[16] = m_nCheckSum & 0xFF;
  buf[17] = (m_nCheckSum >> 8) & 0xFF;
  buf[18] = (m_nCheckSum >> 16) & 0xFF;
  buf[19] = (m_nCheckSum >> 24) & 0xFF;

#elif ICQ_VERSION == 5
  unsigned long l = buffer->getDataSize();
  unsigned char *buf = (unsigned char *)buffer->getDataStart();

  // Make sure packet is long enough
  if (l == 24)
  {
    buffer->add((unsigned long)rand());
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
  unsigned short r1 = 24 + rand() % (l - 24);
  unsigned short r2 = rand() & 0xFF;
  unsigned long chk2 = ( r1 << 24 ) |
                       ( buf[r1] << 16 ) |
                       ( r2 << 8 ) |
                       ( icq_check_data[r2] );
  chk2 ^= 0x00FF00FF;
  m_nCheckSum = chk1 ^ chk2;
  unsigned long key = l * 0x68656C6C + m_nCheckSum;

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
  unsigned long a1 = m_nCheckSum & 0x0000001F;
  unsigned long a2 = m_nCheckSum & 0x03E003E0;
  unsigned long a3 = m_nCheckSum & 0xF8000400;
  unsigned long a4 = m_nCheckSum & 0x0000F800;
  unsigned long a5 = m_nCheckSum & 0x041F0000;
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
}


CPacketUdp::~CPacketUdp(void)
{
  if (buffer != NULL) delete buffer;
}


void CPacketUdp::InitBuffer(void)
{
  buffer = new CBuffer(getSize() + 8);
#if ICQ_VERSION == 2
  buffer->add(m_nVersion);
  buffer->add(m_nCommand);
  buffer->add(m_nSequence);
  buffer->add(m_nSourceUin);
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
  buffer->add(m_nVersion);
  buffer->add(m_nZero);
  buffer->add(m_nSourceUin);
  buffer->add(m_nSessionId);
  buffer->add(m_nCommand);
  buffer->add(m_nSequence);
  buffer->add(m_nSubSequence);
  buffer->add(m_nCheckSum);
#endif;
}

//-----Register--------------------------------------------------------------
CPU_Register::CPU_Register(const char *_szPasswd)
  : CPacketUdp(ICQ_CMDxSND_REGISTERxUSER)
{
  m_nUnknown1 = 0x0002;
  m_nPasswdLen = strlen(_szPasswd) + 1;
  m_szPasswd = strdup(_szPasswd);
  m_nUnknown2 = 0x000000A0;
  m_nUnknown3 = 0x00002461;
  m_nUnknown4 = 0x00A00000;
  m_nUnknown5 = 0x00000000;

  InitBuffer();

  buffer = new CBuffer(getSize());
  buffer->add(m_nUnknown1);
  buffer->add(m_nPasswdLen);
  buffer->add(m_szPasswd, m_nPasswdLen);
  buffer->add(m_nUnknown2);
  buffer->add(m_nUnknown3);
  buffer->add(m_nUnknown4);
  buffer->add(m_nUnknown5);

  Encrypt();
}


CPU_Register::~CPU_Register(void)
{
  free (m_szPasswd);
}

unsigned long CPU_Register::getSize(void)
{
  return (strlen (m_szPasswd) + 1 + 20);
}



//-----Logon--------------------------------------------------------------------
unsigned long CPU_Logon::getSize(void)
{
#if ICQ_VERSION == 2
  return (CPacketUdp::getSize() + 6 + m_nPasswordLength + 27);
#elif ICQ_VERSION == 4
  return (CPacketUdp::getSize() + 6 + m_nPasswordLength + 31);
#elif ICQ_VERSION == 5
  return (CPacketUdp::getSize() + 6 + m_nPasswordLength + 41);
#endif
}

CPU_Logon::CPU_Logon(INetSocket *_s, const char *_szPassword, unsigned short _nLogonStatus)
  : CPacketUdp(ICQ_CMDxSND_LOGON)
{
#if ICQ_VERSION == 2
  m_nUnknown_1 = 0x00040072;
  char temp[10] = { 2, 0, 0, 0, 0, 0, 4, 0, 0x72, 0 };
  memcpy(m_aUnknown_2, temp_3, 10);
#elif ICQ_VERSION == 4
  m_nUnknown_1 = 0x98;
  char temp[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0x98, 0 };
  memcpy(m_aUnknown_2, temp_3, 10);
  m_nTime = time(NULL);
#elif ICQ_VERSION == 5
  m_nUnknown_1 = 0xD5;
  char temp[20] = { 0x00, 0x00, 0x00, 0x00,
                    0xEC, 0x01, 0x2C, 0x82,
                    0x50, 0x00, 0x00, 0x00,
                    0x03, 0x00, 0x00, 0x00,
                    0x00, 0x16, 0xD6, 0x36 };
  memcpy(m_aUnknown_2, temp, 20);
  m_nTime = time(NULL);
  s_nSessionId = rand() & 0x3FFFFFFF;
  s_nSequence = rand() & 0x7FFF;
  s_nSubSequence = 1;
  m_nSessionId = s_nSessionId;
  m_nSequence = s_nSequence++;
  m_nSubSequence = s_nSubSequence++;
#endif
  m_nLocalPort = _s->LocalPort();
  m_sPassword = strdup(_szPassword);
  m_nPasswordLength = strlen(m_sPassword) + 1;
  m_nLocalIP = NetworkIpToPacketIp(_s->LocalIp());
  m_nMode = MODE_DIRECT;
  m_nLogonStatus = _nLogonStatus;
  m_nTcpVersion = 0x06;//ICQ_VERSION_TCP;

  InitBuffer();

#if ICQ_VERSION == 4 || ICQ_VERSION == 5
  buffer->add(m_nTime);
#endif
  buffer->add(m_nLocalPort);
  buffer->add(m_nPasswordLength);
  buffer->add(m_sPassword, m_nPasswordLength);
  buffer->add(m_nUnknown_1);
  buffer->add(m_nLocalIP);
  buffer->add(m_nMode);
  buffer->add(m_nLogonStatus);
  buffer->add(m_nTcpVersion);
#if ICQ_VERSION == 2 || ICQ_VERSION == 4
  buffer->add(m_aUnknown_2, 10);
#else
  buffer->add(m_aUnknown_2, 20);
#endif

  Encrypt();
}

CPU_Logon::~CPU_Logon(void)
{
  free (m_sPassword);
}


//-----Ack----------------------------------------------------------------------
unsigned long CPU_Ack::getSize(void)
{
  return (CPacketUdp::getSize());
}

#if ICQ_VERSION == 2
CPU_Ack::CPU_Ack(unsigned short _nSequence) : CPacketUdp(ICQ_CMDxSND_ACK)
{
  m_nSequence = _nSequence;
  InitBuffer();
  Encrypt();
}
#elif ICQ_VERSION == 4 || ICQ_VERSION == 5
CPU_Ack::CPU_Ack(unsigned short _nSequence, unsigned short _nSubSequence)
  : CPacketUdp(ICQ_CMDxSND_ACK)
{
  m_nSequence = _nSequence;
  m_nSubSequence = _nSubSequence;
  InitBuffer();
  Encrypt();
}
#endif

//-----GetUserBasicInfo---------------------------------------------------------
unsigned long CPU_GetUserBasicInfo::getSize(void)
{
  return (CPacketUdp::getSize() + 6);
}

CPU_GetUserBasicInfo::CPU_GetUserBasicInfo(unsigned long _nUserUin)
  : CPacketUdp(ICQ_CMDxSND_USERxGETINFO)
{
  m_nUserUin = _nUserUin;

  InitBuffer();
#if ICQ_VERSION == 2
  buffer->add(m_nSubSequence);
#endif
  buffer->add(m_nUserUin);

  Encrypt();
}


//-----GetUserExtInfo-----------------------------------------------------------
unsigned long CPU_GetUserExtInfo::getSize(void)
{
  return (CPacketUdp::getSize() + 6);
}

CPU_GetUserExtInfo::CPU_GetUserExtInfo(unsigned long _nUserUin)
  : CPacketUdp(ICQ_CMDxSND_USERxGETDETAILS)
{
  m_nUserUin = _nUserUin;

  InitBuffer();

#if ICQ_VERSION == 2
  buffer->add(m_nSubSequence);
#endif
  buffer->add(m_nUserUin);
  Encrypt();
}


//-----AddUser------------------------------------------------------------------
unsigned long CPU_AddUser::getSize(void)
{
  return (CPacketUdp::getSize() + 4);
}

CPU_AddUser::CPU_AddUser(unsigned long _nAddedUin)
  : CPacketUdp(ICQ_CMDxSND_USERxADD)
{
  m_nAddedUin = _nAddedUin;

  InitBuffer();

  buffer->add(m_nAddedUin);
  Encrypt();
}


//-----Logoff-------------------------------------------------------------------
unsigned long CPU_Logoff::getSize(void)
{
  return (CPacketUdp::getSize() + 24);
}

CPU_Logoff::CPU_Logoff(void) : CPacketUdp(ICQ_CMDxSND_LOGOFF)
{
  char temp_1[2] = { 0x14, 0x00 };
  memcpy(m_aUnknown_1, temp_1, sizeof(m_aUnknown_1));
  strcpy(m_aDisconnect, "B_USER_DISCONNECTED");
  char temp_2[2] = { 0x05, 0x00 };
  memcpy(m_aUnknown_2, temp_2, sizeof(m_aUnknown_2));

  InitBuffer();

  buffer->add(m_aUnknown_1, 2);
  buffer->add(m_aDisconnect, 20);
  buffer->add(m_aUnknown_2, 2);
  Encrypt();
}


//-----ContactList--------------------------------------------------------------

unsigned long CPU_ContactList::getSize(void)
{
  return(CPacketUdp::getSize() + 1 + (m_vnUins.size() * sizeof(unsigned long)));
}

CPU_ContactList::CPU_ContactList(UinList &uins)
  : CPacketUdp(ICQ_CMDxSND_USERxLIST), m_vnUins(uins)
{
  m_nNumUsers = m_vnUins.size();

  InitBuffer();

  buffer->add(m_nNumUsers);
  for (unsigned short i  = 0; i < m_vnUins.size(); i++)
    buffer->add(m_vnUins[i]);
  Encrypt();
}

CPU_ContactList::CPU_ContactList(unsigned long _nUin)
  : CPacketUdp(ICQ_CMDxSND_USERxLIST)
{
  m_vnUins.push_back(_nUin);
  m_nNumUsers = m_vnUins.size();

  InitBuffer();

  buffer->add(m_nNumUsers);
  for (unsigned short i  = 0; i < m_vnUins.size(); i++)
     buffer->add(m_vnUins[i]);
  Encrypt();
}


//-----VisibleList--------------------------------------------------------------
unsigned long CPU_VisibleList::getSize(void)
{
  return(CPacketUdp::getSize() + 1 + (m_vnUins.size() * sizeof(unsigned long)));
}

CPU_VisibleList::CPU_VisibleList(UinList &uins)
  : CPacketUdp(ICQ_CMDxSND_VISIBLExLIST), m_vnUins(uins)
{
  m_nNumUsers = m_vnUins.size();

  InitBuffer();

  buffer->add(m_nNumUsers);
  for (unsigned short i  = 0; i < m_vnUins.size(); i++)
    buffer->add(m_vnUins[i]);

  Encrypt();
}


//-----InvisibleList--------------------------------------------------------------
unsigned long CPU_InvisibleList::getSize(void)
{
  return(CPacketUdp::getSize() + 1 + (m_vnUins.size() * sizeof(unsigned long)));
}

CPU_InvisibleList::CPU_InvisibleList(UinList &uins)
  : CPacketUdp(ICQ_CMDxSND_INVISIBLExLIST), m_vnUins(uins)
{
  m_nNumUsers = m_vnUins.size();

  InitBuffer();

  buffer->add(m_nNumUsers);
  for (unsigned short i  = 0; i < m_vnUins.size(); i++)
    buffer->add(m_vnUins[i]);

  Encrypt();
}



//-----StartSearch--------------------------------------------------------------
unsigned long CPU_StartSearch::getSize(void)
{
  return (CPacketUdp::getSize() + 10 + m_nAliasLength + m_nFirstNameLength +
          m_nLastNameLength + m_nEmailLength);
}

CPU_StartSearch::CPU_StartSearch(const char *_sAlias, const char *_sFirstName,
                                const char *_sLastName, const char *_sEmail)
  : CPacketUdp(ICQ_CMDxSND_SEARCHxSTART)
{
  m_nAliasLength = strlen(_sAlias) + 1;
  m_sAlias = strdup(_sAlias);
  m_nFirstNameLength = strlen(_sFirstName) + 1;
  m_sFirstName = strdup(_sFirstName);
  m_nLastNameLength = strlen(_sLastName) + 1;
  m_sLastName = strdup(_sLastName);
  m_nEmailLength = strlen(_sEmail) + 1;
  m_sEmail = strdup(_sEmail);

  InitBuffer();

#if ICQ_VERSION == 2
  buffer->add(m_nSubSequence);
#endif
  buffer->add(m_nAliasLength);
  buffer->add(m_sAlias, m_nAliasLength);
  buffer->add(m_nFirstNameLength);
  buffer->add(m_sFirstName, m_nFirstNameLength);
  buffer->add(m_nLastNameLength);
  buffer->add(m_sLastName, m_nLastNameLength);
  buffer->add(m_nEmailLength);
  buffer->add(m_sEmail, m_nEmailLength);

  Encrypt();
}

CPU_StartSearch::~CPU_StartSearch(void)
{
  free (m_sAlias);
  free (m_sFirstName);
  free (m_sLastName);
  free (m_sEmail);
}


//-----UpdatePersonalInfo-------------------------------------------------------
unsigned long CPU_UpdatePersonalBasicInfo::getSize(void)
{
  return (CPacketUdp::getSize() + 11 + m_nAliasLength + m_nFirstNameLength +
          m_nLastNameLength + m_nEmailLength);
}

CPU_UpdatePersonalBasicInfo::CPU_UpdatePersonalBasicInfo(const char *_sAlias,
                                                        const char *_sFirstName,
                                                        const char *_sLastName,
                                                        const char *_sEmail,
                                                        bool _bAuthorization)
  : CPacketUdp(ICQ_CMDxSND_UPDATExBASIC)
{
  m_nAliasLength = strlen(_sAlias) + 1;
  m_sAlias = strdup(_sAlias);
  m_nFirstNameLength = strlen(_sFirstName) + 1;
  m_sFirstName = strdup(_sFirstName);
  m_nLastNameLength = strlen(_sLastName) + 1;
  m_sLastName = strdup(_sLastName);
  m_nEmailLength = strlen(_sEmail) + 1;
  m_sEmail = strdup(_sEmail);
  m_nAuthorization = (_bAuthorization ? 0 : 1);  // 0 for require authorization

  InitBuffer();

#if ICQ_VERSION == 2
  buffer->add(m_nSubSequence);
#endif
  buffer->add(m_nAliasLength);
  buffer->add(m_sAlias, m_nAliasLength);
  buffer->add(m_nFirstNameLength);
  buffer->add(m_sFirstName, m_nFirstNameLength);
  buffer->add(m_nLastNameLength);
  buffer->add(m_sLastName, m_nLastNameLength);
  buffer->add(m_nEmailLength);
  buffer->add(m_sEmail, m_nEmailLength);
  buffer->add(m_nAuthorization);

  Encrypt();
}


CPU_UpdatePersonalBasicInfo::~CPU_UpdatePersonalBasicInfo(void)
{
  free (m_sAlias);
  free (m_sFirstName);
  free (m_sLastName);
  free (m_sEmail);
}


//-----UpdatePersonalExtInfo-------------------------------------------------------
unsigned long CPU_UpdatePersonalExtInfo::getSize(void)
{
  return (CPacketUdp::getSize() + m_nCityLength + m_nStateLength +
          m_nPhoneLength + m_nHomepageLength + m_nAboutLength +
#if ICQ_VERSION == 2
          22
#elif ICQ_VERSION == 4 || ICQ_VERSION == 5
          20
#endif
          );
}

CPU_UpdatePersonalExtInfo::CPU_UpdatePersonalExtInfo(const char *_sCity,
                                                    unsigned short _nCountry,
                                                    const char *_sState,
                                                    unsigned short _nAge,
                                                    char _cSex,
                                                    const char *_sPhone,
                                                    const char *_sHomepage,
                                                    const char *_sAbout,
                                                    unsigned long _nZipcode)
  : CPacketUdp(ICQ_CMDxSND_UPDATExDETAIL)
{

  m_sCity = strdup(_sCity == NULL ? "" : _sCity);
  m_nCityLength = strlen(m_sCity) + 1;
  m_nCountry = _nCountry;
  struct timezone tz;
  gettimeofday(NULL, &tz);
  m_cTimezone = tz.tz_minuteswest / 30;
  //if (m_cTimezone > 23) m_cTimezone = 23 - m_cTimezone;
  m_sState = strdup(_sState == NULL ? "" : _sState);
  m_nStateLength = strlen(m_sState) + 1;
  if (m_nStateLength > 6)  // State is max 5 characters + NULL
  {
     m_sState[5] = '\0';
     m_nStateLength = 6;
  }
  m_nAge = _nAge;
  m_cSex = _cSex;
  m_sPhone = strdup(_sPhone == NULL ? "" : _sPhone);
  m_nPhoneLength = strlen(m_sPhone) + 1;
  m_sHomepage = strdup(_sHomepage == NULL ? "" : _sHomepage);
  m_nHomepageLength = strlen(m_sHomepage) + 1;
  m_sAbout = strdup(_sAbout == NULL ? "" : _sAbout);
  m_nAboutLength = strlen(m_sAbout) + 1;
  m_nZipcode = _nZipcode;

  InitBuffer();

#if ICQ_VERSION == 2
  buffer->add(m_nSubSequence);
#endif
  buffer->add(m_nCityLength);
  buffer->add(m_sCity, m_nCityLength);
  buffer->add(m_nCountry);
  buffer->add(m_cTimezone);
  buffer->add(m_nStateLength);
  buffer->add(m_sState, m_nStateLength);
  buffer->add(m_nAge);
  buffer->add(m_cSex);
  buffer->add(m_nPhoneLength);
  buffer->add(m_sPhone, m_nPhoneLength);
  buffer->add(m_nHomepageLength);
  buffer->add(m_sHomepage, m_nHomepageLength);
  buffer->add(m_nAboutLength);
  buffer->add(m_sAbout, m_nAboutLength);
  buffer->add(m_nZipcode);

  Encrypt();
}


CPU_UpdatePersonalExtInfo::~CPU_UpdatePersonalExtInfo(void)
{
  free (m_sCity);
  free (m_sState);
  free (m_sPhone);
  free (m_sHomepage);
  free (m_sAbout);
}


//-----Ping---------------------------------------------------------------------
unsigned long CPU_Ping::getSize(void)
{
  return (CPacketUdp::getSize());
}

CPU_Ping::CPU_Ping(void) : CPacketUdp(ICQ_CMDxSND_PING)
{
  InitBuffer();
  Encrypt();
}


//-----ThroughServer------------------------------------------------------------
unsigned long CPU_ThroughServer::getSize(void)
{
  return (CPacketUdp::getSize() + 8 + strlen(m_sMessage) + 1);
}

CPU_ThroughServer::CPU_ThroughServer(unsigned long _nSourceUin,
                                    unsigned long _nDestinationUin,
                                    unsigned short _nSubCommand,
                                    char *_sMessage)
  : CPacketUdp(ICQ_CMDxSND_THRUxSERVER)
{
  m_nSourceUin = (_nSourceUin == 0 ? gUserManager.OwnerUin() : _nSourceUin);
  m_nDestinationUin = _nDestinationUin;
  m_nSubCommand = _nSubCommand;
  if (_sMessage != NULL)
  {
    m_nMsgLength = strlen(_sMessage) + 1;
    m_sMessage = strdup(_sMessage);
  }
  else
  {
    m_nMsgLength = 1;
    m_sMessage = strdup("");
  }

  InitBuffer();

  buffer->add(m_nDestinationUin);
  buffer->add(m_nSubCommand);
  buffer->add(m_nMsgLength);
  buffer->add(m_sMessage, m_nMsgLength);

  Encrypt();
}

CPU_ThroughServer::~CPU_ThroughServer(void)
{
  free (m_sMessage);
}

//-----SetStatus----------------------------------------------------------------
unsigned long CPU_SetStatus::getSize(void)
{
  return (CPacketUdp::getSize() + 4);
}

CPU_SetStatus::CPU_SetStatus(unsigned long _nNewStatus) : CPacketUdp(ICQ_CMDxSND_SETxSTATUS)
{
  m_nNewStatus = _nNewStatus;

  InitBuffer();

  buffer->add(m_nNewStatus);
  Encrypt();
}

//-----Authorize----------------------------------------------------------------
unsigned long CPU_Authorize::getSize(void)
{
  return (CPacketUdp::getSize() + 9);
}

CPU_Authorize::CPU_Authorize(unsigned long _nAuthorizeUin) : CPacketUdp(ICQ_CMDxSND_AUTHORIZE)
{
  m_nAuthorizeUin = _nAuthorizeUin;
  char temp_1[5] = { 0x08, 0x00, 0x01, 0x00, 0x00 };
  memcpy(m_aUnknown_1, temp_1, sizeof(m_aUnknown_1));

  InitBuffer();

  buffer->add(m_nAuthorizeUin);
  buffer->add(m_aUnknown_1, 5);
  Encrypt();
}


//-----RequestSysMsg------------------------------------------------------------
unsigned long CPU_RequestSysMsg::getSize(void)
{
  return(CPacketUdp::getSize());
}

CPU_RequestSysMsg::CPU_RequestSysMsg(void) : CPacketUdp(ICQ_CMDxSND_SYSxMSGxREQ)
{
  InitBuffer();
  Encrypt();
}


//-----SysMsgDoneAck------------------------------------------------------------
unsigned long CPU_SysMsgDoneAck::getSize(void)
{
  return(CPacketUdp::getSize());
}

#if ICQ_VERSION == 2
CPU_SysMsgDoneAck::CPU_SysMsgDoneAck(unsigned short _nSequence)
  : CPacketUdp(ICQ_CMDxSND_SYSxMSGxDONExACK)
{
  m_nSequence = _nSequence;
  InitBuffer();
  Encrypt();
}
#elif ICQ_VERSION == 4
CPU_SysMsgDoneAck::CPU_SysMsgDoneAck(unsigned short _nSequence, unsigned short _nSubSequence)
  : CPacketUdp(ICQ_CMDxSND_SYSxMSGxDONExACK)
{
  m_nSequence = _nSequence;
  m_nSubSequence = _nSubSequence;
  InitBuffer();
  Encrypt();
}
#elif ICQ_VERSION == 5
CPU_SysMsgDoneAck::CPU_SysMsgDoneAck(void)
  : CPacketUdp(ICQ_CMDxSND_SYSxMSGxDONExACK)
{
  InitBuffer();
  Encrypt();
}
#endif

//-----Meta_SetWorkInfo------------------------------------------------------
CPU_Meta_SetWorkInfo::CPU_Meta_SetWorkInfo(
    const char *_szCity,
    const char *_szState,
    const char *_szFax,
    const char *_szAddress,
    const char *_szName,
    const char *_szDepartment,
    const char *_szPosition,
    const char *_szHomepage) : CPacketUdp(ICQ_CMDxSND_META)
{
  m_nMetaCommand = 0;//ICQ_CMDxMETA_WORKxINFOxSET;
  m_nCityLength = strlen(_szCity) + 1;
  m_szCity = strdup(_szCity);
  m_nStateLength = strlen(_szState) + 1;
  m_szState = strdup(_szState);
  m_nFaxLength = strlen(_szFax) + 1;
  m_szFax = strdup(_szFax);
  m_nAddressLength = strlen(_szAddress) + 1;
  m_szAddress = strdup(_szAddress);
  m_nUnknown1 = 0x0100;
  m_nUnknown2 = 0xffff;
  m_nNameLength = strlen(_szName) + 1;
  m_szName = strdup(_szName);
  m_nDepartmentLength = strlen(_szDepartment) + 1;
  m_szDepartment = strdup(_szDepartment);
  m_nPositionLength = strlen(_szPosition) + 1;
  m_szPosition = strdup(_szPosition);
  m_nUnknown3 = 0x04;
  m_nHomepageLength = strlen(_szHomepage) + 1;
  m_szHomepage = strdup(_szHomepage);

  InitBuffer();

  buffer->add(m_nMetaCommand);
  buffer->add(m_nCityLength);
  buffer->add(m_szCity, m_nCityLength);
  buffer->add(m_nStateLength);
  buffer->add(m_szState, m_nStateLength);
  buffer->add(m_nFaxLength);
  buffer->add(m_szFax, m_nFaxLength);
  buffer->add(m_nAddressLength);
  buffer->add(m_szAddress, m_nAddressLength);
  buffer->add(m_nUnknown1);
  buffer->add(m_nUnknown2);
  buffer->add(m_nNameLength);
  buffer->add(m_szName, m_nNameLength);
  buffer->add(m_nDepartmentLength);
  buffer->add(m_szDepartment, m_nDepartmentLength);
  buffer->add(m_nPositionLength);
  buffer->add(m_szPosition, m_nPositionLength);
  buffer->add(m_nUnknown3);
  buffer->add(m_nHomepageLength);
  buffer->add(m_szHomepage, m_nHomepageLength);

  Encrypt();
}


CPU_Meta_SetWorkInfo::~CPU_Meta_SetWorkInfo(void)
{
  free(m_szCity);
  free(m_szState);
  free(m_szFax);
  free(m_szAddress);
  free(m_szName);
  free(m_szDepartment);
  free(m_szPosition);
  free(m_szHomepage);
}

unsigned long CPU_Meta_SetWorkInfo::getSize(void)
{
  return CPacketUdp::getSize() + m_nCityLength + m_nStateLength +
         m_nFaxLength + m_nAddressLength + m_nNameLength +
         m_nDepartmentLength + m_nPositionLength + m_nHomepageLength + 26;
}


//-----Meta_SetSecurityInfo--------------------------------------------------
CPU_Meta_SetSecurityInfo::CPU_Meta_SetSecurityInfo(
    bool _bAuthorization,
    bool _bHideIp,
    bool _bWebAware)
  : CPacketUdp(ICQ_CMDxSND_META)
{
  m_nMetaCommand = ICQ_CMDxMETA_SECURITYxSET;
  m_bAuthorization = _bAuthorization ? 0 : 1;
  m_bHideIp =  _bHideIp ? 1 : 0;
  m_bWebAware = _bWebAware ? 1 : 0;

  InitBuffer();

  buffer->add(m_nMetaCommand);
  buffer->add(m_bAuthorization);
  buffer->add(m_bHideIp);
  buffer->add(m_bWebAware);

  Encrypt();
}


unsigned long CPU_Meta_SetSecurityInfo::getSize(void)
{
  return CPacketUdp::getSize() + 5;
}


//-----Meta_RequestInfo------------------------------------------------------
CPU_Meta_RequestInfo::CPU_Meta_RequestInfo(unsigned long _nUin)
  : CPacketUdp(ICQ_CMDxSND_META)
{
  m_nMetaCommand = ICQ_CMDxMETA_REQUESTxINFO;
  m_nUin = _nUin;

  InitBuffer();

  buffer->add(m_nMetaCommand);
  buffer->add(m_nUin);

  Encrypt();
}

unsigned long CPU_Meta_RequestInfo::getSize(void)
{
  return CPacketUdp::getSize() + 6;
}



//=====PacketTcp_Handshake======================================================
CPacketTcp_Handshake::~CPacketTcp_Handshake(void)
{
  if (buffer != NULL) delete buffer;
}


CPacketTcp_Handshake::CPacketTcp_Handshake(unsigned long _nLocalPort)
{
  m_cHandshakeCommand = ICQ_CMDxTCP_HANDSHAKE;
  m_nTcpVersion = ICQ_VERSION_TCP;
  m_nLocalPort = _nLocalPort;
  m_nSourceUin = gUserManager.OwnerUin();
  m_nLocalHost = LOCALHOST;
  m_aUnknown_2 = 0x04;

  InitBuffer();
}


void CPacketTcp_Handshake::InitBuffer(void)
{
  buffer = new CBuffer(getSize());

  buffer->add(m_cHandshakeCommand);
  buffer->add(m_nTcpVersion);
  buffer->add(m_nLocalPort);
  buffer->add(m_nSourceUin);
  buffer->add(m_nLocalHost);
  buffer->add(m_nLocalHost);
  buffer->add(m_aUnknown_2);
  buffer->add(m_nLocalPort);
}


//=====PacketTcp================================================================
CPacketTcp::CPacketTcp(unsigned long _nSourceUin, unsigned long _nCommand,
                      unsigned short _nSubCommand, const char *_sMessage,
                      bool _bAccept, bool _bUrgent, ICQUser *_cUser)
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
  m_nTcpVersion = ICQ_VERSION_TCP;
  m_nCommand = _nCommand;
  m_nSubCommand = _nSubCommand;
  if (_sMessage != NULL)
  {
   m_nMsgLength = strlen(_sMessage) + 1;
   m_sMessage = strdup(_sMessage);
  }
  else
  {
   m_nMsgLength = 1;
   m_sMessage = strdup("");
  }
  m_nLocalIP = 0;
  m_nLocalHost = LOCALHOST;
  m_nLocalPort = 0;
  m_aUnknown_1 = 0x04;

  m_sLicqTag = 'L';
  m_nLicqVersion = INT_VERSION;

  // don't increment the sequence if this is an ack and cancel packet
  if (m_nCommand == ICQ_CMDxTCP_START) m_nSequence = _cUser->Sequence(true);

  m_cUser = _cUser;
  buffer = NULL;
}

CPacketTcp::~CPacketTcp(void)
{
  free (m_sMessage);
  if (buffer != NULL) delete buffer;
}


void CPacketTcp::Create(void)
{
  // This is bad because the user might have been deleted between the
  // constructor and this call...
  INetSocket *s = gSocketManager.FetchSocket(m_cUser->SocketDesc());
  if (s == NULL)
  {
    gLog.Error("%sNo socket found for %s (%ld) while creating packet.\n", L_ERRORxSTR,
              m_cUser->GetAlias(), m_cUser->Uin());
    m_nLocalIP = LOCALHOST;
    m_nLocalPort = 0;
  }
  else
  {
    m_nLocalIP = NetworkIpToPacketIp(s->LocalIp());
    m_nLocalPort = s->LocalPort();
    gSocketManager.DropSocket(s);
  }
}


void CPacketTcp::InitBuffer(void)
{
  buffer = new CBuffer(getSize());

  buffer->add(m_nSourceUin);
  buffer->add(m_nTcpVersion);
  buffer->add(m_nCommand);
  buffer->add(m_nSourceUin);
  buffer->add(m_nSubCommand);
  buffer->add(m_nMsgLength);
  buffer->add(m_sMessage, m_nMsgLength);
  buffer->add(m_nLocalIP);
  buffer->add(m_nLocalHost);
  buffer->add(m_nLocalPort);
  buffer->add(m_aUnknown_1);
  buffer->add(m_nStatus);
  buffer->add(m_nMsgType);
}

void CPacketTcp::postBuffer(void)
{
  buffer->add(m_nSequence);
  buffer->add(m_sLicqTag);
  buffer->add(m_nLicqVersion);
}

unsigned long CPacketTcp::getSize(void)
{
  return (18 + m_nMsgLength + 24);
}

//-----Message------------------------------------------------------------------
unsigned long CPT_Message::getSize(void)
{
  return (CPacketTcp::getSize());
}

CPT_Message::CPT_Message(unsigned long _nSourceUin, char *_sMessage, bool _bUrgent,
                        ICQUser *_cUser)
  : CPacketTcp(_nSourceUin, ICQ_CMDxTCP_START, ICQ_CMDxSUB_MSG, _sMessage,
               true, _bUrgent, _cUser)
{
  //InitBuffer();
  //postBuffer();
}

void CPT_Message::Create(void)
{
  CPacketTcp::Create();
  InitBuffer();
  postBuffer();
}

//-----Url----------------------------------------------------------------------
unsigned long CPT_Url::getSize(void)
{
  return (CPacketTcp::getSize());
}

CPT_Url::CPT_Url(unsigned long _nSourceUin, char *_sMessage, bool _bUrgent,
                ICQUser *_cUser)
  : CPacketTcp(_nSourceUin, ICQ_CMDxTCP_START, ICQ_CMDxSUB_URL, _sMessage,
               true, _bUrgent, _cUser)
{
  //InitBuffer();
  //postBuffer();
}

void CPT_Url::Create(void)
{
  CPacketTcp::Create();
  InitBuffer();
  postBuffer();
}


//-----ReadAwayMessage----------------------------------------------------------
unsigned long CPT_ReadAwayMessage::getSize(void)
{
  return (CPacketTcp::getSize());
}

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
  //InitBuffer();
  //postBuffer();
}


void CPT_ReadAwayMessage::Create(void)
{
  CPacketTcp::Create();
  InitBuffer();
  postBuffer();
}

//-----ChatRequest--------------------------------------------------------------
unsigned long CPT_ChatRequest::getSize(void)
{
  return (CPacketTcp::getSize() + 11);
}

CPT_ChatRequest::CPT_ChatRequest(unsigned long _nSourceUin, char *_sMessage,
                                bool _bUrgent, ICQUser *_cUser)
  : CPacketTcp(_nSourceUin, ICQ_CMDxTCP_START, ICQ_CMDxSUB_CHAT, _sMessage,
               true, _bUrgent, _cUser)
{
  char temp_1[11] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  memcpy(m_aUnknown_1, temp_1, 11);

  /*InitBuffer();

  buffer->add(m_aUnknown_1, 11);

  postBuffer();*/
}


void CPT_ChatRequest::Create(void)
{
  CPacketTcp::Create();
  InitBuffer();
  buffer->add(m_aUnknown_1, 11);
  postBuffer();
}


//-----FileTransfer--------------------------------------------------------------
unsigned long CPT_FileTransfer::getSize(void)
{
  return (CPacketTcp::getSize() + 14 + m_nFilenameLength);
}

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

  m_nUnknown_1 = 0;
  m_nFilenameLength = strlen(m_szFilename) + 1;
  // m_szFilename set above
  // m_nFileSize set above
  m_nUnknown_2 = 0;

  /*InitBuffer();

  buffer->add(m_nUnknown_1);
  buffer->add(m_nFilenameLength);
  buffer->add(m_szFilename, m_nFilenameLength);
  buffer->add(m_nFileSize);
  buffer->add(m_nUnknown_2);

  postBuffer();*/
}


void CPT_FileTransfer::Create(void)
{
  CPacketTcp::Create();
  InitBuffer();

  buffer->add(m_nUnknown_1);
  buffer->add(m_nFilenameLength);
  buffer->add(m_szFilename, m_nFilenameLength);
  buffer->add(m_nFileSize);
  buffer->add(m_nUnknown_2);

  postBuffer();
}


//+++++Ack++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
unsigned long CPT_Ack::getSize(void)
{
  return (CPacketTcp::getSize());
}

CPT_Ack::CPT_Ack(unsigned short _nSubCommand, unsigned long _nSequence,
                bool _bAccept, bool _bUrgent, ICQUser *_cUser)
  : CPacketTcp(0, ICQ_CMDxTCP_ACK, _nSubCommand, "", _bAccept, _bUrgent,
               _cUser)
{
  m_nSequence = _nSequence;
  free(m_sMessage);
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  m_sMessage = gTranslator.NToRN(o->AutoResponse());
  gUserManager.DropOwner();
  m_nMsgLength = strlen(m_sMessage) + 1;
  gTranslator.ClientToServer(m_sMessage);
}


//-----AckMessage---------------------------------------------------------------
unsigned long CPT_AckMessage::getSize(void)
{
  return (CPT_Ack::getSize());
}

CPT_AckMessage::CPT_AckMessage(unsigned long _nSequence, bool _bAccept,
                              bool _bUrgent, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_MSG, _nSequence, _bAccept, _bUrgent, _cUser)
{
  CPacketTcp::Create();
  InitBuffer();
  postBuffer();
}



//-----AckReadAwayMessage-------------------------------------------------------
unsigned long CPT_AckReadAwayMessage::getSize(void)
{
  return (CPT_Ack::getSize());
}

CPT_AckReadAwayMessage::CPT_AckReadAwayMessage(unsigned short _nSubCommand,
                                              unsigned long _nSequence,
                                              bool _bAccept, ICQUser *_cUser)
  : CPT_Ack(_nSubCommand, _nSequence, _bAccept, false, _cUser)
{
  CPacketTcp::Create();
  InitBuffer();
  postBuffer();
}


//-----AckUrl-------------------------------------------------------------------
unsigned long CPT_AckUrl::getSize(void)
{
  return (CPT_Ack::getSize());
}

CPT_AckUrl::CPT_AckUrl(unsigned long _nSequence, bool _bAccept, bool _bUrgent,
                      ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_URL, _nSequence, _bAccept, _bUrgent, _cUser)
{
  CPacketTcp::Create();
  InitBuffer();
  postBuffer();
}


//-----AckUrl-------------------------------------------------------------------
unsigned long CPT_AckContactList::getSize(void)
{
  return (CPT_Ack::getSize());
}

CPT_AckContactList::CPT_AckContactList(unsigned long _nSequence, bool _bAccept,
                                      bool _bUrgent, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_CONTACTxLIST, _nSequence, _bAccept, _bUrgent, _cUser)
{
  CPacketTcp::Create();
  InitBuffer();
  postBuffer();
}


//-----AckChatRefuse------------------------------------------------------------
unsigned long CPT_AckChatRefuse::getSize(void)
{
  return (CPT_Ack::getSize() + 11);
}

CPT_AckChatRefuse::CPT_AckChatRefuse(const char *_sReason,
                                    unsigned long _nSequence, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_CHAT, _nSequence, false, false, _cUser)
{
  free (m_sMessage);
  if (_sReason == NULL)
  {
    m_nMsgLength = 1;
    m_sMessage = strdup("");
  }
  else
  {
    m_nMsgLength = strlen(_sReason) + 1;
    m_sMessage = strdup(_sReason);
  }
  char temp_1[11] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  memcpy(m_aUnknown_1, temp_1, 11);

  CPacketTcp::Create();
  InitBuffer();

  buffer->add(m_aUnknown_1, 11);

  postBuffer();
}


//-----AckChatAccept------------------------------------------------------------
unsigned long CPT_AckChatAccept::getSize(void)
{
  return (CPT_Ack::getSize() + 11);
}

CPT_AckChatAccept::CPT_AckChatAccept(unsigned short _nPort,
                                    unsigned long _nSequence, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_CHAT, _nSequence, true, true, _cUser)
{
  char temp_1[3] = { 1, 0, 0 };
  memcpy(m_aUnknown_1, temp_1, sizeof(m_aUnknown_1));
  m_nPortReversed = ((_nPort & 0xFF) << 8) + ((_nPort >> 8) & 0xFF);
  m_nPort = _nPort;
  m_nStatus = ICQ_TCPxACK_ONLINE;

  CPacketTcp::Create();
  InitBuffer();

  buffer->add(m_aUnknown_1, sizeof(m_aUnknown_1));
  buffer->add(m_nPortReversed);
  buffer->add(m_nPort);

  postBuffer();
}


//-----AckFileRefuse------------------------------------------------------------
unsigned long CPT_AckFileRefuse::getSize(void)
{
  return (CPT_Ack::getSize() + 15);
}

CPT_AckFileRefuse::CPT_AckFileRefuse(const char *_sReason,
                                    unsigned long _nSequence, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_FILE, _nSequence, false, false, _cUser)
{
  free (m_sMessage);
  if (_sReason == NULL)
  {
    m_nMsgLength = 1;
    m_sMessage = strdup("");
  }
  else
  {
    m_nMsgLength = strlen(_sReason) + 1;
    m_sMessage = strdup(_sReason);
  }
  m_nUnknown_1 = 0;
  m_nStrLen = 1;
  m_cEmptyStr = '\0';
  m_nUnknown_2 = 0;
  m_nUnknown_3 = 0;

  CPacketTcp::Create();
  InitBuffer();

  buffer->add(m_nUnknown_1);
  buffer->add(m_nStrLen);
  buffer->add(m_cEmptyStr);
  buffer->add(m_nUnknown_2);
  buffer->add(m_nUnknown_3);

  postBuffer();
}


//-----AckFileAccept------------------------------------------------------------
unsigned long CPT_AckFileAccept::getSize(void)
{
  return (CPT_Ack::getSize() + 15);
}

CPT_AckFileAccept::CPT_AckFileAccept(unsigned short _nPort,
                                    unsigned long _nSequence, ICQUser *_cUser)
  : CPT_Ack(ICQ_CMDxSUB_FILE, _nSequence, true, true, _cUser)
{
  m_nPortReversed = ((_nPort & 0xFF) << 8) + ((_nPort >> 8) & 0xFF);
  m_nStrLength = 1;
  m_cEmptyStr = '\0';
  m_nFileSize = 0;  // not used in the ack
  m_nPort = _nPort;
  m_nStatus = ICQ_TCPxACK_ONLINE;

  CPacketTcp::Create();
  InitBuffer();

  buffer->add(m_nPortReversed);
  buffer->add(m_nStrLength);
  buffer->add(m_cEmptyStr);
  buffer->add(m_nFileSize);
  buffer->add(m_nPort);

  postBuffer();
}


//+++++Cancel+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
unsigned long CPT_Cancel::getSize(void)
{
  return (CPacketTcp::getSize());
}

CPT_Cancel::CPT_Cancel(unsigned short _nSubCommand, unsigned long _nSequence,
                      ICQUser *_cUser)
  : CPacketTcp(0, ICQ_CMDxTCP_CANCEL, _nSubCommand, "", true, false, _cUser)
{
  m_nSequence = _nSequence;
}



//-----CancelChat---------------------------------------------------------------
unsigned long CPT_CancelChat::getSize(void)
{
  return (CPT_Cancel::getSize() + 11);
}

CPT_CancelChat::CPT_CancelChat(unsigned long _nSequence, ICQUser *_cUser)
  : CPT_Cancel(ICQ_CMDxSUB_CHAT, _nSequence, _cUser)
{
  char temp_1[11] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  memcpy(m_aUnknown_1, temp_1, 11);

  CPacketTcp::Create();
  InitBuffer();

  buffer->add(m_aUnknown_1, 11);

  postBuffer();
}


//-----CancelFile---------------------------------------------------------------
unsigned long CPT_CancelFile::getSize(void)
{
  return (CPT_Cancel::getSize() + 15);
}

CPT_CancelFile::CPT_CancelFile(unsigned long _nSequence, ICQUser *_cUser)
  : CPT_Cancel(ICQ_CMDxSUB_FILE, _nSequence, _cUser)
{
  m_nUnknown_1 = 0;
  m_nStrLen = 1;
  m_cEmptyStr = '\0';
  m_nUnknown_2 = 0;
  m_nUnknown_3 = 0;

  CPacketTcp::Create();
  InitBuffer();

  buffer->add(m_nUnknown_1);
  buffer->add(m_nStrLen);
  buffer->add(m_cEmptyStr);
  buffer->add(m_nUnknown_2);
  buffer->add(m_nUnknown_3);

  postBuffer();
}


//=====Chat=====================================================================
void CPacketChat::InitBuffer(void)
{
  buffer = new CBuffer(getSize());
}

//-----ChatColor----------------------------------------------------------------
CPChat_Color::CPChat_Color(char *_sLocalName, unsigned short _nLocalPort,
                          unsigned long _nColorForeground,
                          unsigned long _nColorBackground)
{
  m_nCommand = 0x64;
  char temp_1[4] = { 0xFD, 0xFF, 0xFF, 0xFF };
  memcpy(m_aUnknown_1, temp_1, sizeof(m_aUnknown_1));
  m_nSourceUin = gUserManager.OwnerUin();
  m_nLocalNameLength = strlen(_sLocalName) + 1;
  m_sLocalName = strdup(_sLocalName);
  m_aLocalPortReversed[0] = ((char *)(&_nLocalPort))[1];
  m_aLocalPortReversed[1] = ((char *)(&_nLocalPort))[0];
  m_nColorForeground = _nColorForeground;
  m_nColorBackground = _nColorBackground;
  m_aUnknown_2 = '\0';

  InitBuffer();

  buffer->add(m_nCommand);
  buffer->add(m_aUnknown_1, sizeof(m_aUnknown_1));
  buffer->add(m_nSourceUin);
  buffer->add(m_nLocalNameLength);
  buffer->add(m_sLocalName, m_nLocalNameLength);
  buffer->add(m_aLocalPortReversed, sizeof(m_aLocalPortReversed));
  buffer->add(m_nColorForeground);
  buffer->add(m_nColorBackground);
  buffer->add(m_aUnknown_2);

}


//-----ChatColorFont----------------------------------------------------------------
CPChat_ColorFont::CPChat_ColorFont(char *_sLocalName, unsigned short _nLocalPort,
                                  unsigned long _nColorForeground,
                                  unsigned long _nColorBackground,
                                  unsigned long _nFontSize,
                                  unsigned long _nFontFace, char *_sFontName)
{
  m_nCommand = 0x64;
  m_nSourceUin = gUserManager.OwnerUin();
  m_nLocalNameLength = strlen(_sLocalName) + 1;
  m_sLocalName = strdup(_sLocalName);
  m_nColorForeground = _nColorForeground;
  m_nColorBackground = _nColorBackground; 
  m_nUnknown_1 = 0x00000003;
  m_nLocalPort = _nLocalPort;
  m_nLocalHost = LOCALHOST;
  m_aUnknown_2 = 0x04;
  m_nUnknown_Port = 0x5A75;
  m_nFontSize = _nFontSize;
  m_nFontFace = _nFontFace;
  m_nFontNameLength = strlen(_sFontName) + 1;
  m_sFontName = strdup(_sFontName);
  char temp_3[3] = { 0, 0, 0 };
  memcpy(m_aUnknown_3, temp_3, sizeof(m_aUnknown_3));
  
  InitBuffer();

  buffer->add(m_nCommand);
  buffer->add(m_nSourceUin);
  buffer->add(m_nLocalNameLength);
  buffer->add(m_sLocalName, m_nLocalNameLength);
  buffer->add(m_nColorForeground);
  buffer->add(m_nColorBackground);
  buffer->add(m_nUnknown_1);
  buffer->add(m_nLocalPort);
  buffer->add(m_nLocalHost);
  buffer->add(m_nLocalHost);
  buffer->add(m_aUnknown_2);
  buffer->add(m_nUnknown_Port);
  buffer->add(m_nFontSize);
  buffer->add(m_nFontFace);
  buffer->add(m_nFontNameLength);
  buffer->add(m_sFontName, m_nFontNameLength);
  buffer->add(m_aUnknown_3, sizeof(m_aUnknown_3));
}



//-----ChatFont---------------------------------------------------------------------
CPChat_Font::CPChat_Font(unsigned short _nLocalPort, unsigned long _nFontSize, 
                        unsigned long _nFontFace, char *_sFontName)
{
  m_nCommand = 0x03;
  m_nLocalPort = _nLocalPort;
  m_nLocalHost = LOCALHOST;
  m_aUnknown_2 = 0x04;
  m_nUnknown_Port = 0x5A75;
  m_nFontSize = _nFontSize;
  m_nFontFace = _nFontFace;
  m_nFontNameLength = strlen(_sFontName) + 1;
  m_sFontName = strdup(_sFontName);
  char temp_3[3] = { 0, 0, 0 };
  memcpy(m_aUnknown_3, temp_3, sizeof(m_aUnknown_3));
  
  InitBuffer();

  buffer->add(m_nCommand);
  buffer->add(m_nLocalPort);
  buffer->add(m_nLocalHost);
  buffer->add(m_nLocalHost);
  buffer->add(m_aUnknown_2);
  buffer->add(m_nUnknown_Port);
  buffer->add(m_nFontSize);
  buffer->add(m_nFontFace);
  buffer->add(m_nFontNameLength);
  buffer->add(m_sFontName, m_nFontNameLength);
  buffer->add(m_aUnknown_3, sizeof(m_aUnknown_3));
}


//-----FileInitClient-----------------------------------------------------------
CPFile_InitClient::CPFile_InitClient(char *_szLocalName,
                                    unsigned long _nNumFiles,
                                    unsigned long _nTotalSize)
{
  m_cUnknown1 = 0;
  m_nUnknown2 = 0;
  m_nNumFiles = _nNumFiles;
  m_nTotalSize = _nTotalSize;
  m_nUnknown3 = 0x64;
  m_nLocalNameLength = strlen(_szLocalName) + 1;
  m_szLocalName = strdup(_szLocalName);

  InitBuffer();

  buffer->add(m_cUnknown1);
  buffer->add(m_nUnknown2);
  buffer->add(m_nNumFiles);
  buffer->add(m_nTotalSize);
  buffer->add(m_nUnknown3);
  buffer->add(m_nLocalNameLength);
  buffer->add(m_szLocalName, m_nLocalNameLength);
}


//-----FileInitServer-----------------------------------------------------------
CPFile_InitServer::CPFile_InitServer(char *_szLocalName)
{
  m_cUnknown1 = 1;
  m_nUnknown2 = 0x64;
  m_nLocalNameLength = strlen(_szLocalName) + 1;
  m_szLocalName = strdup(_szLocalName);

  InitBuffer();

  buffer->add(m_cUnknown1);
  buffer->add(m_nUnknown2);
  buffer->add(m_nLocalNameLength);
  buffer->add(m_szLocalName, m_nLocalNameLength);
}


//-----FileBatch----------------------------------------------------------------
CPFile_Info::CPFile_Info(const char *_szFileName)
{
  m_bValid = true;
  m_nError = 0;
  m_nPacketId = 0x02;

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
  m_nUnknown1 = 0x0001;
  m_cUnknown2 = 0x00;
  m_nFileSize = buf.st_size;
  m_nUnknown3 = 0x00;
  m_nUnknown4 = 0x64;

  InitBuffer();

  buffer->add(m_nPacketId);

  // Add all the file names
  unsigned short nFilenameLen = strlen(m_szFileName) + 1;
  buffer->add(nFilenameLen);
  buffer->add(m_szFileName, nFilenameLen);
  // Add the empty file name
  buffer->add(m_nUnknown1);
  buffer->add(m_cUnknown2);
  //Add the file length
  buffer->add(m_nFileSize);
  buffer->add(m_nUnknown3);
  buffer->add(m_nUnknown4);
}


unsigned long CPFile_Info::getSize(void)
{
  // Add the length of each file name + 1 for the terminating nulll + 2 for
  // the short representing the length of the file name + 4 for the file size
  unsigned short n = strlen(m_szFileName) + 1 + 2 + 4;
  return (CPacketFile::getSize() + 13 + n);
}


CPFile_Info::~CPFile_Info(void)
{
  free (m_szFileName);
}


//-----FileStart----------------------------------------------------------------
CPFile_Start::CPFile_Start(unsigned long _nFilePos)
{
  m_cUnknown1 = 3;
  m_nFilePos = _nFilePos;
  m_nUnknown2 = 0;
  m_nUnknown3 = 0x64;

  InitBuffer();

  buffer->add(m_cUnknown1);
  buffer->add(m_nFilePos);
  buffer->add(m_nUnknown2);
  buffer->add(m_nUnknown3);
}

