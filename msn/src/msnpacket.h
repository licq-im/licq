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

#ifndef __MSNPACKET_H
#define __MSNPACKET_H

#include <cstdlib>
#include <pthread.h>
#include <string>

#include "licq_packets.h"

#include "msnbuffer.h"

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

class CMSNPacket : public CPacket
{
public:
  CMSNPacket(bool = false);
  virtual ~CMSNPacket() { if (m_pBuffer) delete m_pBuffer; if (m_szCommand) free(m_szCommand); }
  CMSNBuffer *getBuffer() { return m_pBuffer; }
  
  unsigned short Command() { return 0; }
  unsigned short SubSequence() { return 0; }
  unsigned short SubCommand() { return 0; }
  unsigned short Sequence() { return m_nSequence; }
  const char* MSNCommand() { return m_szCommand; }
  
  virtual void InitBuffer();

  char *CreateGUID();

protected:
  
  CMSNBuffer *m_pBuffer;
  char *m_szCommand;
  unsigned short m_nSequence;
  unsigned long m_nSize;
  bool m_bPing;
  
  // Statics
  static unsigned short s_nSequence;
  static pthread_mutex_t s_xMutex;
};

class CMSNPayloadPacket : public CMSNPacket
{
public:
  CMSNPayloadPacket(bool);
  virtual ~CMSNPayloadPacket() { }
  
  virtual void InitBuffer();
  
protected:
  unsigned long m_nPayloadSize;
  bool m_bAck;
};

class CMSNP2PPacket : public CMSNPayloadPacket
{
public:
  CMSNP2PPacket(const char *,unsigned long = 0, unsigned long = 0, unsigned long = 0,
		unsigned long = 0, unsigned long = 0, unsigned long = 0,
		unsigned long = 0, unsigned long = 0, unsigned long = 0,
		unsigned long = 0, unsigned long = 0, unsigned long = 0);
  virtual ~CMSNP2PPacket();

  void InitBuffer();

  unsigned long SessionId() { return m_nSessionId; }
  unsigned long BaseId() { return m_nBaseId; }
  char *CallGUID() { return m_szCallGUID; }
  // unsigned long DataSize() {}
  //unsigned long Offset() {}

protected:
  char *m_szToEmail,
       *m_szCallGUID;
  unsigned long m_nSessionId,
    m_nBaseId,
    m_nDataSizeLO,
    m_nDataSizeHI,
    m_nDataOffsetLO,
    m_nDataOffsetHI,
    m_nLen,
    m_nFlag,
    m_nAckId,
    m_nAckUniqueId,
    m_nAckDataLO,
    m_nAckDataHI;
};

class CPS_MSNVersion : public CMSNPacket
{
public:
  CPS_MSNVersion();
};

class CPS_MSNClientVersion : public CMSNPacket
{
public:
  CPS_MSNClientVersion(char *);
  virtual ~CPS_MSNClientVersion() { if (m_szUserName) free(m_szUserName); }

protected:
  char *m_szUserName;
};

class CPS_MSNUser : public CMSNPacket
{
public:
  CPS_MSNUser(char *);
  virtual ~CPS_MSNUser() { if(m_szUserName) free(m_szUserName); }
  
protected:
  char *m_szUserName;
};

class CPS_MSNGetServer : public CMSNPacket
{
public:
  CPS_MSNGetServer();
};

class CPS_MSNAuthenticate : public CMSNPacket
{
public:
  CPS_MSNAuthenticate(char *, char *, const char *);
  virtual ~CPS_MSNAuthenticate() { if (m_szCookie) free(m_szCookie); }
  
protected:
  char *m_szCookie;
};

class CPS_MSNSendTicket : public CMSNPacket
{
public:
  CPS_MSNSendTicket(const char *);
  virtual ~CPS_MSNSendTicket() { if (m_szTicket) free(m_szTicket); }
  
protected:
  char *m_szTicket;
};

class CPS_MSNChangeStatus : public CMSNPacket
{
public:
  CPS_MSNChangeStatus(unsigned long);
};

class CPS_MSNLogoff : public CMSNPacket
{
public:
  CPS_MSNLogoff();
};

class CPS_MSNSync : public CMSNPacket
{
public:
  CPS_MSNSync(unsigned long);
};

class CPS_MSNChallenge : public CMSNPacket
{
public:
  CPS_MSNChallenge(const char *);
};

class CPS_MSNSetPrivacy : public CMSNPacket
{
public:
  CPS_MSNSetPrivacy();
};

class CPS_MSNAddUser : public CMSNPacket
{
public:
  CPS_MSNAddUser(const char *, const char *szList);
  virtual ~CPS_MSNAddUser() { if (m_szUser) free(m_szUser); if (m_szList) free(m_szList); }
  
protected:
  char *m_szUser;
  char *m_szList;
};

class CPS_MSNRemoveUser : public CMSNPacket
{
public:
  CPS_MSNRemoveUser(const char *, const char *szList);
  virtual ~CPS_MSNRemoveUser() { if (m_szUser) free(m_szUser); if (m_szList) free (m_szList); }
  
protected:
  char *m_szUser;
  char *m_szList;
};

class CPS_MSNRenameUser : public CMSNPacket
{
public:
  CPS_MSNRenameUser(const char *, const char *);
  virtual ~CPS_MSNRenameUser() { if (m_szUser) free(m_szUser); if (m_szNewNick) free (m_szNewNick); }

protected:
  char *m_szUser;
  char *m_szNewNick;
};

class CPS_MSN_SBStart : public CMSNPacket
{
public:
  CPS_MSN_SBStart(const char *, const char *);
  virtual ~CPS_MSN_SBStart()
  { if (m_szUser) free(m_szUser); if (m_szCookie) free(m_szCookie); }
  
protected:
  char *m_szUser,
       *m_szCookie;
};

class CPS_MSN_SBAnswer : public CMSNPacket
{
public:
  CPS_MSN_SBAnswer(const char *, const char *, const char *);
  virtual ~CPS_MSN_SBAnswer()
  {
    if (m_szSession) free(m_szSession); if (m_szCookie) free(m_szCookie);
    if (m_szUser) free(m_szUser);
  }
  
protected:
  char *m_szSession,
       *m_szCookie,
       *m_szUser;
};

class CPS_MSNMessage : public CMSNPayloadPacket
{
public:
  CPS_MSNMessage(const char *);
  virtual ~CPS_MSNMessage() { if (m_szMsg) free(m_szMsg); }
  
protected:
  char *m_szMsg;
};

class CPS_MSNPing : public CMSNPacket
{
public:
  CPS_MSNPing();
};

class CPS_MSNXfr : public CMSNPacket
{
public:
  CPS_MSNXfr();
};

class CPS_MSNCall : public CMSNPacket
{
public:
  CPS_MSNCall(char *);
  virtual ~CPS_MSNCall() { if (m_szUser) free(m_szUser); }
  
protected:
  char *m_szUser;
};

class CPS_MSNTypingNotification : public CMSNPayloadPacket
{
public:
  CPS_MSNTypingNotification(const char *);
};

class CPS_MSNCancelInvite : public CMSNPayloadPacket
{
public:
  CPS_MSNCancelInvite(const std::string& cookie, const std::string& code);
};

class CPS_MSNInvitation : public CMSNP2PPacket
{
public:
  CPS_MSNInvitation(const char* szToEmail, const char* szFromEmail,
      const char* szMSNObject);
};

class CPS_MSNP2PBye : public CMSNP2PPacket
{
public:
  CPS_MSNP2PBye(const char *szId, const char *szFromId, const char *szCallId,
                unsigned long nBaseId, unsigned long nAckId,
		unsigned long nDataSizeHI, unsigned long nDataSizeLO);
};

class CPS_MSNP2PAck : public CMSNP2PPacket
{
public:
  CPS_MSNP2PAck(const char *szId, unsigned long nSessionId,
		unsigned long nBaseId, unsigned long nAckId,
		unsigned long nAckBaseId, unsigned long nDataSizeHI,
		unsigned long nDataSizeLO);
};

#endif // __MSNPACKET_H

