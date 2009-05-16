#ifndef LICQAUTOREPLY_H
#define LICQAUTOREPLY_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <licq_types.h>

class CICQDaemon;
class CUserEvent;
class LicqSignal;
class LicqEvent;

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

  void ProcessPipe();
  void ProcessSignal(LicqSignal* s);
  void ProcessEvent(LicqEvent* e);

  /**
   * A new event arrived for a user
   *
   * @param userId Affected user
   * @param eventId Id of event
   */
  void processUserEvent(const UserId& userId, unsigned long eventId);

  /**
   * Make auto reply for an event
   *
   * @param userId Affected user
   * @Param event Event to reply to
   * @return True if a reply was sent
   */
  bool autoReplyEvent(const UserId& userId, const CUserEvent* event);

  bool POpen(const char *cmd);
  int PClose();

protected:
  int pid;
  FILE *fStdOut;
  FILE *fStdIn;

};


#endif
