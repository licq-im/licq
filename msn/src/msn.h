#ifndef __MSN_H
#define __MSN_H

#include "licq_events.h"
#include "licq_icqd.h"
#include "licq_socket.h"

#include "msnbuffer.h"

#define MSN_PPID 0x4D534E5F
#define L_MSNxSTR "[MSN] "

class CMSNPacket;

class CMSN
{
public:
  CMSN(CICQDaemon *, int);
  ~CMSN();
  
  void Run();
  
private:
  void ProcessSignal(CSignal *);
  void ProcessPipe();
  void ProcessServerPacket(CMSNBuffer &);
  void ProcessSSLServerPacket(CMSNBuffer &);
  
  // Network functions
  void SendPacket(CMSNPacket *);
  void MSNLogon(const char *, int);
  void MSNAuthenticate(char *);

  // Variables
  CICQDaemon *m_pDaemon;
  int m_nPipe;
  int m_nServerSocket;
  int m_nSSLSocket;
  CMSNBuffer *m_pPacketBuf;
  
  char *m_szUserName,
       *m_szPassword;
};

#endif // __MSN_H
