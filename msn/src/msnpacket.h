#ifndef __MSNPACKET_H
#define __MSNPACKET_H

#include "msnbuffer.h"

#include <pthread.h>
#include <stdlib.h>

class CMSNPacket
{
public:
  CMSNPacket(bool = false);
  virtual ~CMSNPacket() { if (m_pBuffer) delete m_pBuffer; if (m_szCommand) free(m_szCommand); }
  CMSNBuffer *getBuffer() { return m_pBuffer; }
  
  const char* Command() { return m_szCommand; }
  unsigned short Sequence() { return m_nSequence; }
  
  virtual void InitBuffer();
  
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
};

class CPS_MSNUser : public CMSNPacket
{
public:
  CPS_MSNUser(char *);
};

class CPS_MSNAuthenticate : public CMSNPacket
{
public:
  CPS_MSNAuthenticate(char *, char *, const char *);
};

class CPS_MSNSendTicket : public CMSNPacket
{
public:
  CPS_MSNSendTicket(const char *);
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
};

class CPS_MSN_SBAnswer : public CMSNPacket
{
public:
  CPS_MSN_SBAnswer(const char *, const char *, const char *sz);
};

class CPS_MSNMessage : public CMSNPayloadPacket
{
public:
  CPS_MSNMessage(const char *);
};

class CPS_MSNPing : public CMSNPacket
{
public:
  CPS_MSNPing();
};

#endif // __MSNPACKET_H