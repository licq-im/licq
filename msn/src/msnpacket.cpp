/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// written by Jon Keating <jon@licq.org>

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
  if (s_nSequence > 9999)
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
    m_nSize += snprintf(buf, 32, "%s %lu ", m_szCommand, m_nSequence) + 2; //don't forget \r\n
  
  m_pBuffer = new CMSNBuffer(m_nSize);
  m_pBuffer->Pack(buf, strlen(buf));
}

CMSNPayloadPacket::CMSNPayloadPacket(bool _bAck) : CMSNPacket(), m_bAck(_bAck)
{
  m_nPayloadSize = 0;
}

void CMSNPayloadPacket::InitBuffer()
{
  if (strlen(m_szCommand) == 0)
    return;
  char buf[32];
  
  m_nSize = snprintf(buf, 32, "%s %lu %c %lu\r\n", m_szCommand, m_nSequence,
    m_bAck ? 'A' : 'N', m_nPayloadSize);
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
  
  m_szUserName = strdup(szUserName);
  
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack(m_szUserName, strlen(m_szUserName));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNUser::CPS_MSNUser(char *szUserName) : CMSNPacket()
{
  m_szCommand = strdup("USR");
  char szParams[] = "TWN I ";
  m_nSize += strlen(szParams) + strlen(szUserName);
  InitBuffer();
  
  m_szUserName = strdup(szUserName);
  
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack(m_szUserName, strlen(m_szUserName));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNGetServer::CPS_MSNGetServer() : CMSNPacket()
{
  char szParams[] = "GET /rdr/pprdr.asp HTTP/1.0\r\n\r\n";
  m_nSize += strlen(szParams);
  
  m_pBuffer = new CMSNBuffer(m_nSize);
  m_pBuffer->Pack(szParams, strlen(szParams));
};

CPS_MSNAuthenticate::CPS_MSNAuthenticate(char *_szUserName, char *_szPassword, const char *szCookie)
  : CMSNPacket()
{
  //TODO make a real url encoder
  char *szPassword = new char[strlen(_szPassword) * 3 + 1];
  char *szUserName = new char[strlen(_szUserName) * 3 + 1];
  memset(szPassword, 0, (strlen(_szPassword) * 3) + 1);
  memset(szUserName, 0, (strlen(_szUserName) * 3) + 1);
  char *szPasswd = szPassword;
  char *szUser = szUserName;
  unsigned int i;
  for (i = 0; i < strlen(_szPassword); i++)
  {
    if (isalnum(_szPassword[i]))
    {
      *szPasswd = _szPassword[i];
      szPasswd += 1;
    }
    else
    {
      sprintf(szPasswd, "%%%02X", _szPassword[i]); 
      szPasswd += 3;
    }
  }
  szPasswd = '\0';
  for (i = 0; i < strlen(_szUserName); i++)
  {
    if (isalnum(_szUserName[i]))
    {
      *szUser = _szUserName[i];
      szUser += 1;
    }
    else
    {
      sprintf(szUser, "%%%02X", _szUserName[i]);
      szUser += 3;
    }
  }
  szUser = '\0';
  
  char szParams1[] = "GET /login2.srf HTTP/1.1\r\n"
                    "Authorization: Passport1.4 OrgVerb=GET,OrgURL=http%3A%2F%2Fmessenger%2Emsn%2Ecom,"
                    "sign-in=";
  char szParams2[] = ",pwd=";
  char szParams3[] = "User-Agent: MSMSGS\r\nHost: loginnet.passport.com\r\n"
                     "Connection: Keep-Alive\r\nCache-Control: no-cache\r\n";
  m_nSize = strlen(szParams1) + strlen(szPassword) + strlen(szUserName)
            + strlen(szParams2) + strlen(szParams3) + strlen(szCookie) + 1 + 4;
  
  m_szCookie = strdup(szCookie);
  
  m_pBuffer = new CMSNBuffer(m_nSize);
  m_pBuffer->Pack(szParams1, strlen(szParams1));
  m_pBuffer->Pack(szUserName, strlen(szUserName));
  m_pBuffer->Pack(szParams2, strlen(szParams2));
  m_pBuffer->Pack(szPassword, strlen(szPassword));
  m_pBuffer->Pack(",", 1);
  m_pBuffer->Pack(m_szCookie, strlen(m_szCookie));
  m_pBuffer->Pack("\r\n", 2);
  m_pBuffer->Pack(szParams3, strlen(szParams3));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNSendTicket::CPS_MSNSendTicket(const char *szTicket) : CMSNPacket()
{
  m_szCommand = strdup("USR");
  char szParams[] = "TWN S ";
  m_nSize += strlen(szParams) + strlen(szTicket);
  InitBuffer();
  
  m_szTicket = strdup(szTicket);
  
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack(m_szTicket, strlen(m_szTicket));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNChangeStatus::CPS_MSNChangeStatus(unsigned long _nStatus) : CMSNPacket()
{
  m_szCommand = strdup("CHG");
  char szParams[] = " 268435500";
  m_nSize += strlen(szParams) + 3;
  InitBuffer();
  
  char szStatus[4];
  
  if (_nStatus & ICQ_STATUS_FxPRIVATE)
    strcpy(szStatus, "HDN");
  else
  {
    switch (_nStatus & 0x0000FFFF)
    {
      case ICQ_STATUS_ONLINE:
      case ICQ_STATUS_FREEFORCHAT:
        strcpy(szStatus, "NLN");
        break;
      
      case ICQ_STATUS_OCCUPIED:
      case ICQ_STATUS_DND:
        strcpy(szStatus, "BSY");
        break;
      
      default:
        strcpy(szStatus, "AWY");
        break;
    } 
  }
  szStatus[3] = '\0';
  
  m_pBuffer->Pack(szStatus, strlen(szStatus));
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNLogoff::CPS_MSNLogoff() : CMSNPacket(true)
{
  m_szCommand = strdup("OUT");
  m_nSize += 0;
  InitBuffer();
  
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNSync::CPS_MSNSync(unsigned long nVersion) : CMSNPacket()
{
  m_szCommand = strdup("SYN");
  char szParams[15];
  int nSize = sprintf(szParams, "%lu", nVersion);
  m_nSize += nSize;
  InitBuffer();
  
  m_pBuffer->Pack(szParams, nSize);
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

CPS_MSNAddUser::CPS_MSNAddUser(const char *szUser, const char *szList)
  : CMSNPacket()
{
  m_szCommand = strdup("ADD");
  m_nSize += strlen(szList) + (strlen(szUser) * 2) + 2;
  InitBuffer();
  
  m_szUser = strdup(szUser);
  m_szList = strdup(szList);
  
  m_pBuffer->Pack(m_szList, strlen(m_szList));
  m_pBuffer->Pack(" ", 1);
  m_pBuffer->Pack(m_szUser, strlen(m_szUser));
  m_pBuffer->Pack(" ", 1);
  m_pBuffer->Pack(m_szUser, strlen(m_szUser));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNRemoveUser::CPS_MSNRemoveUser(const char *szUser, const char *szList)
  : CMSNPacket()
{
  m_szCommand = strdup("REM");
  m_nSize += strlen(szList) + strlen(szUser) + 1;
  InitBuffer();
  
  m_szUser = strdup(szUser);
  m_szList = strdup(szList);
  
  m_pBuffer->Pack(m_szList, strlen(m_szList));
  m_pBuffer->Pack(" ", 1);
  m_pBuffer->Pack(m_szUser, strlen(m_szUser));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNRenameUser::CPS_MSNRenameUser(const char *szUser, const char *szNewNick)
  : CMSNPacket()
{
  m_szCommand = strdup("REA");
  m_szUser = strdup(szUser);
  m_szNewNick = strdup(szNewNick);
  m_nSize += strlen(m_szUser) + strlen(m_szNewNick) + 1;
  InitBuffer();

  m_pBuffer->Pack(m_szUser, strlen(m_szUser));
  m_pBuffer->Pack(" ", 1);
  m_pBuffer->Pack(m_szNewNick, strlen(m_szNewNick));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSN_SBStart::CPS_MSN_SBStart(const char *szCookie, const char *szUser)
  : CMSNPacket()
{
  m_szCommand = strdup("USR");
  m_nSize += strlen(szCookie) + strlen(szUser) + 1;
  InitBuffer();
  
  m_szUser = strdup(szUser);
  m_szCookie = strdup(szCookie);
  
  m_pBuffer->Pack(m_szUser, strlen(m_szUser));
  m_pBuffer->Pack(" ", 1);
  m_pBuffer->Pack(m_szCookie, strlen(m_szCookie));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSN_SBAnswer::CPS_MSN_SBAnswer(const char *szSession, const char *szCookie,
  const char *szUser)
  : CMSNPacket()
{
  m_szCommand = strdup("ANS");
  m_nSize += strlen(szSession) + strlen(szCookie) + strlen(szUser) + 2;
  InitBuffer();
  
  m_szUser = strdup(szUser);
  m_szSession = strdup(szSession);
  m_szCookie = strdup(szCookie);
  
  m_pBuffer->Pack(m_szUser, strlen(m_szUser));
  m_pBuffer->Pack(" ", 1);
  m_pBuffer->Pack(m_szCookie, strlen(m_szCookie));
  m_pBuffer->Pack(" ", 1);
  m_pBuffer->Pack(m_szSession, strlen(m_szSession));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNMessage::CPS_MSNMessage(const char *szMsg) : CMSNPayloadPacket(true)
{
  m_szCommand = strdup("MSG");
  char szParams[] = "MIME-Version: 1.0\r\n"
    "Content-Type: text/plain; charset=UTF-8\r\n"
    "X-MMS-IM-Format: FN=MS%20Sans%20Serif; EF=; CO=0; CS=0; PF=0\r\n"
    "\r\n";
  m_nPayloadSize = strlen(szMsg) + strlen(szParams);
  CMSNPayloadPacket::InitBuffer();
  
  m_szMsg = strdup(szMsg);
  
  m_pBuffer->Pack(szParams, strlen(szParams));
  m_pBuffer->Pack(m_szMsg, strlen(m_szMsg));
}

CPS_MSNPing::CPS_MSNPing() : CMSNPacket(true)
{
  m_szCommand = strdup("PNG");
  m_nSize += 0;
  InitBuffer();
  
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNXfr::CPS_MSNXfr() : CMSNPacket()
{
  m_szCommand = strdup("XFR");
  m_nSize += 2;
  InitBuffer();
  
  m_pBuffer->Pack("SB\r\n", 4);
}

CPS_MSNCall::CPS_MSNCall(char *szUser) : CMSNPacket()
{
  m_szCommand = strdup("CAL");
  m_nSize += strlen(szUser);
  InitBuffer();
  
  m_szUser = strdup(szUser);
  
  m_pBuffer->Pack(m_szUser, strlen(m_szUser));
  m_pBuffer->Pack("\r\n", 2);
}

CPS_MSNTypingNotification::CPS_MSNTypingNotification(const char *szEmail)
  : CMSNPayloadPacket(false)
{
  m_szCommand = strdup("MSG");
  char szParams1[] = "MIME-Version: 1.0\r\n"
    "Content-Type: text/x-msmsgscontrol\r\n"
    "TypingUser: ";
  char szParams2[] = "\r\n\r\n\r\n";    
  m_nPayloadSize = strlen(szParams1) + strlen(szParams2) +
    strlen(szEmail);
  CMSNPayloadPacket::InitBuffer();  

  m_pBuffer->Pack(szParams1, strlen(szParams1));
  m_pBuffer->Pack(szEmail, strlen(szEmail));
  m_pBuffer->Pack(szParams2, strlen(szParams2));
}
