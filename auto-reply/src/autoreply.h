#ifndef LICQAUTOREPLY_H
#define LICQAUTOREPLY_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

class CICQDaemon;
class TCPSocket;
class ICQUser;
class CUserEvent;
class CICQSignal;
class ICQEvent;

class CLicqAutoReply
{
public:
  CLicqAutoReply(bool, bool, char *);
  ~CLicqAutoReply();
  int Run(CICQDaemon *);
  void Shutdown();
  bool Enabled() { return m_bEnabled; }

protected:
  int m_nPipe;
  bool m_bExit, m_bEnabled, m_bDelete;
  char *m_szStatus;
  char m_szProgram[512];

  CICQDaemon *licqDaemon;
  TCPSocket *tcp;

public:
  void ProcessPipe();
  void ProcessSignal(CICQSignal *);
  void ProcessEvent(ICQEvent *);

  void ProcessUserEvent(unsigned long, unsigned long);
  bool AutoReplyEvent(ICQUser *, CUserEvent *);

};


#endif
