#ifndef LICQEMAIL_H
#define LICQEMAIL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

class CICQDaemon;
class TCPSocket;
class ICQUser;
class CUserEvent;
class CICQSignal;
class ICQEvent;

class CLicqEmail
{
public:
  CLicqEmail(bool, char *);
  ~CLicqEmail();
  int Run(CICQDaemon *);
  void Shutdown();
  bool Enabled() { return m_bEnabled; }

protected:
  int m_nPipe;
  bool m_bExit, m_bEnabled;
  char *m_szStatus;

  unsigned short m_nSMTPPort;
  char m_szSMTPHost[256], m_szSMTPTo[256], m_szSMTPFrom[256],
       m_szSMTPDomain[256];

  CICQDaemon *licqDaemon;
  TCPSocket *tcp;

public:
  void ProcessPipe();
  void ProcessSignal(CICQSignal *);
  void ProcessEvent(ICQEvent *);

  void ProcessUserEvent(unsigned long);
  bool ForwardEvent(ICQUser *, CUserEvent *);

};


#endif
