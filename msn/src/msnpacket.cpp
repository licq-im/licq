#include "msnpacket.h"

#include <string.h>
#include <stdio.h>
#include <openssl/md5.h>

unsigned short CMSNPacket::s_nSequence = 0;
pthread_mutex_t CMSNPacket::s_xMutex = PTHREAD_MUTEX_INITIALIZER;

CMSNPacket::CMSNPacket(bool _bPing) : CPacket()
{
  m_pBuffer = 0;
  m_szCommand = 0;
  m_nSize = 0;
  m_bPing = _bPing;
  
  pthread_mutex_lock(&s_xMutex);
  if (s_nSequence > 99999)
   s_nSequence = 0;
  m_nSequence = s_nSequence++;
  pthread_mutex_unlock(&s_xMutex);
}

void CMSNPacket::InitBuffer()
{
  if (strlen(m_szCommand) == 0)
    return;
  char buf[32];
    
  if (m_bPing)
    m_nSize += snprintf(buf, 32, "%s", m_szCommand) + 2;
  else
    m_nSize += snprintf(buf, 32, "%s %u ", m_szCommand, m_nSequence) + 2; //don't forget \r\n
  
  m_pBuffer = new CMSNBuffer(m_nSize);
  m_pBuffer->Pack(buf, strlen(buf));
}

CMSNPayloadPacket::CMSNPayloadPacket() : CMSNPacket()
{
  m_nPayloadSize = 0;
}

void CMSNPayloadPacket::InitBuffer()
{
  if (strlen(m_szCommand) == 0)
    return;
  char buf[32];
  
  m_nSize = snprintf(buf, 32, "%s %u A %u\r\n", m_szCommand, m_nSequence, m_nPayloadSize);
  m_nSize += m_nPayloadSize;
  
  m_pBuffer = new CMSNBuffer(m_nSize);
  m_pBuffer->Pack(buf, strlen(buf));
}

CPS_MSNVersion::CPS_MSNVersion() : CMSNPacket()
{
  m_szCommand = strdup("VER");
  char szParams[] = "MSNP8 CVR0";
  m_nSize += strlen(szParams);
  InitBuffer();
 
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNClientVersion::CPS_MSNClientVersion(char *szUserName) : CMSNPacket()
{
  m_szCommand = strdup("CVR");
  //TODO customize locale en_US = 0x0409
  char szParams[] = "0x0409 winnt 6.0 i386 MSNMSGR 6.0.0602 MSMSGS ";
  m_nSize += strlen(szParams) + strlen(szUserName);
  InitBuffer();
  
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack(szUserName, strlen(szUserName));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNUser::CPS_MSNUser(char *szUserName) : CMSNPacket()
{
  m_szCommand = strdup("USR");
  char szParams[] = "TWN I ";
  m_nSize += strlen(szParams) + strlen(szUserName);
  InitBuffer();
  
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack(szUserName, strlen(szUserName));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNAuthenticate::CPS_MSNAuthenticate(char *_szUserName, char *_szPassword, const char *szCookie)
  : CMSNPacket()
{
  //TODO make a real url encoder
  char *szPassword = new char[strlen(_szPassword) * 3 + 1];
  char *szUserName = new char[strlen(_szUserName) * 3 + 1];
  int i;
  for (i = 0; i < strlen(_szPassword); i++)
    sprintf(&szPassword[i * 3], "%%%02X", _szPassword[i]); 
  szPassword[(i * 3) + 1] = '\0';
  for (i = 0; i < strlen(_szUserName); i++)
    sprintf(&szUserName[i * 3], "%%%02X", _szUserName[i]);
  szUserName[(i * 3) + 1] = '\0';
  
  char szParams1[] = "GET /login2.srf\r\n"
                    "Authorization: Passport1.4 OrgVerb=GET,OrgURL=http%3A%2F%2Fmessenger%2Emsn%2Ecom,"
                    "sign-in=";
  char szParams2[] = ",pwd=";
  //char szParams3[] = "Host: loginnet.passport.com\r\n";
  m_nSize = strlen(szParams1) + strlen(szPassword) + strlen(szUserName)
            + strlen(szParams2) + strlen(szCookie) + 1 + 2;
  
  m_pBuffer = new CMSNBuffer(m_nSize);
  m_pBuffer->Pack(szParams1, strlen(szParams1));
  m_pBuffer->Pack(szUserName, strlen(szUserName));
  m_pBuffer->Pack(szParams2, strlen(szParams2));
  m_pBuffer->Pack(szPassword, strlen(szPassword));
  m_pBuffer->Pack(",", 1);
  m_pBuffer->Pack(szCookie, strlen(szCookie));
  m_pBuffer->Pack("\r\n", 2);
  //m_pBuffer->Pack(szParams2, strlen(szParams2));
}

CPS_MSNSendTicket::CPS_MSNSendTicket(const char *szTicket) : CMSNPacket()
{
  m_szCommand = strdup("USR");
  char szParams[] = "TWN S ";
  m_nSize += strlen(szParams) + strlen(szTicket);
  InitBuffer();
  
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack(szTicket, strlen(szTicket));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNChangeStatus::CPS_MSNChangeStatus() : CMSNPacket()
{
  m_szCommand = strdup("CHG");
  char szParams[] = "NLN 268435500";
  m_nSize += strlen(szParams);
  InitBuffer();
  
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNSync::CPS_MSNSync() : CMSNPacket()
{
  m_szCommand = strdup("SYN");
  char szParams[] = "0";
  m_nSize += 1;
  InitBuffer();
  
  m_pBuffer->Pack(szParams, 1);
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNChallenge::CPS_MSNChallenge(const char *szHash) : CMSNPacket()
{
  m_szCommand = strdup("QRY");
  char szParams[] = "msmsgs@msnmsgr.com 32";
  m_nSize += strlen(szParams) + 32; //payload
  InitBuffer();
  
  char szSource[64];
  unsigned char szDigest[16];
  char szHexOut[32];
  snprintf(szSource, 64, "%sQ1P7W2E4J9R8U3S5", szHash);
  szSource[63] = '\0';
  MD5((const unsigned char *)szSource, strlen(szSource), szDigest);
  
  for (int i = 0; i < 16; i++)
  {
    sprintf(&szHexOut[i*2], "%02x", szDigest[i]);
  }
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack("\r\n", 2);
  m_pBuffer->Pack(szHexOut, 32);
}

CPS_MSNSetPrivacy::CPS_MSNSetPrivacy() : CMSNPacket()
{
  //TODO
  m_szCommand = strdup("GTC");
  char szParams[] = "N";
  m_nSize += strlen(szParams);
  InitBuffer();
  
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNAddUser::CPS_MSNAddUser(const char *szUser) : CMSNPacket()
{
  m_szCommand = strdup("ADD");
  char szParams[] = "FL ";
  m_nSize += strlen(szParams) + (strlen(szUser) * 2) + 1;
  InitBuffer();
  
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack(szUser, strlen(szUser));
  m_pBuffer->Pack(" ", 1);
  m_pBuffer->Pack(szUser, strlen(szUser));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSN_SBAnswer::CPS_MSN_SBAnswer(const char *szSession, const char *szCookie,
  const char *szUser)
  : CMSNPacket()
{
  m_szCommand = strdup("ANS");
  m_nSize += strlen(szSession) + strlen(szCookie) + strlen(szUser) + 2;
  InitBuffer();
  
  m_pBuffer->Pack(szUser, strlen(szUser));
  m_pBuffer->Pack(" ", 1);
  m_pBuffer->Pack(szCookie, strlen(szCookie));
  m_pBuffer->Pack(" ", 1);
  m_pBuffer->Pack(szSession, strlen(szSession));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNMessage::CPS_MSNMessage(const char *szMsg) : CMSNPayloadPacket()
{
  m_szCommand = strdup("MSG");
  char szParams[] = "MIME-Version: 1.0\r\n"
    "Content-Type: text/plain; charset=UTF-8\r\n"
    "X-MMS-IM-Format: FN=MS%20Sans%20Serif; EF=; CO=0; CS=0; PF=0\r\n"
    "\r\n";
  m_nPayloadSize = strlen(szMsg) + strlen(szParams);
  CMSNPayloadPacket::InitBuffer();
  
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack(szMsg, strlen(szMsg));
}

CPS_MSNPing::CPS_MSNPing() : CMSNPacket(true)
{
  m_szCommand = strdup("PNG");
  m_nSize = 0;
  InitBuffer();
  
  m_pBuffer->Pack("\r\n", 2);
}

