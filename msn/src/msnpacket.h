#ifndef __MSNPACKET_H
#define __MSNPACKET_H

#include "licq_packets.h"

#include "msnbuffer.h"

#include <pthread.h>
#include <stdlib.h>

class CMSNPacket : public CPacket
{
public:
  CMSNPacket(bool = false);
  virtual ~CMSNPacket() { if (m_pBuffer) delete m_pBuffer; if (m_szCommand) free(m_szCommand); }
  CMSNBuffer *getBuffer() { return m_pBuffer; }
  
  const unsigned short Command() { return 0; }
  const unsigned short SubSequence() { return 0; }
  const unsigned short SubCommand() { return 0; }
  const unsigned long Sequence() { return m_nSequence; }
  const char* MSNCommand() { return m_szCommand; }
  
  virtual void InitBuffer();
  
protected:
  
  CMSNBuffer *m_pBuffer;
  char *m_szCommand;
  unsigned long m_nSequence;
  unsigned long m_nSize;
  bool m_bPing;
  
  // Statics
  static unsigned short s_nSequence;
  static pthread_mutex_t s_xMutex;
};

class CMSNPayloadPacket : public CMSNPacket
{
public:
  CMSNPayloadPacket();
  virtual ~CMSNPayloadPacket() { }
  
  void InitBuffer();
  
protected:

  unsigned long m_nPayloadSize;
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
  CPS_MSNChangeStatus();
};

class CPS_MSNSync : public CMSNPacket
{
public:
  CPS_MSNSync();
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
  CPS_MSNAddUser(const char *);
  virtual ~CPS_MSNAddUser() { if (m_szUser) free(m_szUser); }
  
protected:
  char *m_szUser;
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

#endif // __MSNPACKET_H

