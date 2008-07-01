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

#define FORWARD_EMAIL 0
#define FORWARD_ICQ 1

class CLicqForwarder
{
public:
  CLicqForwarder(bool, bool, char *);
  ~CLicqForwarder();
  int Run(CICQDaemon *);
  void Shutdown();
  bool Enabled() { return m_bEnabled; }

protected:
  int m_nPipe;
  bool m_bExit, m_bEnabled, m_bDelete;
  char *m_szStatus;

  unsigned short m_nSMTPPort;
  char m_szSMTPHost[256], m_szSMTPTo[256], m_szSMTPFrom[256],
       m_szSMTPDomain[256];
  char myUserId[16];
  unsigned short m_nForwardType;

  CICQDaemon *licqDaemon;
  TCPSocket *tcp;

public:
  void ProcessPipe();
  void ProcessSignal(CICQSignal *);
  void ProcessEvent(ICQEvent *);

  void ProcessUserEvent(const char *, unsigned long, unsigned long);
  bool ForwardEvent(ICQUser *, CUserEvent *);
  bool ForwardEvent_ICQ(ICQUser *, CUserEvent *);
  bool ForwardEvent_Email(ICQUser *, CUserEvent *);

private:
  bool CreateDefaultConfig();

};


#endif
