#ifndef LICQAUTOREPLY_H
#define LICQAUTOREPLY_H

#include <string>


namespace Licq
{
class Event;
class PluginSignal;
class UserEvent;
class UserId;
}

class CLicqAutoReply
{
public:
  CLicqAutoReply(bool, bool, char *);
  ~CLicqAutoReply();
  int Run();
  void Shutdown();
  bool Enabled() { return m_bEnabled; }

protected:
  int m_nPipe;
  bool m_bExit, m_bEnabled, m_bDelete;
  char *m_szStatus;
  std::string myProgram;
  std::string myArguments;
  bool m_bPassMessage, m_bFailOnExitCode, m_bAbortDeleteOnExitCode,
       m_bSendThroughServer;

  void ProcessPipe();
  void ProcessSignal(Licq::PluginSignal* s);
  void ProcessEvent(Licq::Event* e);

  /**
   * A new event arrived for a user
   *
   * @param userId Affected user
   * @param eventId Id of event
   */
  void processUserEvent(const Licq::UserId& userId, unsigned long eventId);

  /**
   * Make auto reply for an event
   *
   * @param userId Affected user
   * @Param event Event to reply to
   * @return True if a reply was sent
   */
  bool autoReplyEvent(const Licq::UserId& userId, const Licq::UserEvent* event);

  bool POpen(const char *cmd);
  int PClose();

protected:
  int pid;
  FILE *fStdOut;
  FILE *fStdIn;

};


#endif
