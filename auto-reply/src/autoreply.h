#ifndef LICQAUTOREPLY_H
#define LICQAUTOREPLY_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

class CICQDaemon;
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
  char m_szProgram[512], m_szArguments[512];
  bool m_bPassMessage, m_bFailOnExitCode, m_bAbortDeleteOnExitCode,
       m_bSendThroughServer;

  CICQDaemon *licqDaemon;

public:
  void ProcessPipe();
  void ProcessSignal(CICQSignal *);
  void ProcessEvent(ICQEvent *);

  void ProcessUserEvent(const char *, unsigned long, unsigned long);
  bool AutoReplyEvent(const char *, unsigned long, CUserEvent *);

  bool POpen(const char *cmd);
  int PClose();

protected:
  int pid;
  FILE *fStdOut;
  FILE *fStdIn;

};


#endif
