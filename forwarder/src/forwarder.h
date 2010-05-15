#ifndef LICQEMAIL_H
#define LICQEMAIL_H

#include <string>

#include <licq/userid.h>

class TCPSocket;
class CUserEvent;
class LicqSignal;
class LicqEvent;

namespace Licq
{
class User;
}

#define FORWARD_EMAIL 0
#define FORWARD_ICQ 1

class CLicqForwarder
{
public:
  CLicqForwarder(bool, bool, char *);
  ~CLicqForwarder();
  int Run();
  void Shutdown();
  bool Enabled() { return m_bEnabled; }

protected:
  int m_nPipe;
  bool m_bExit, m_bEnabled, m_bDelete;
  char *m_szStatus;

  unsigned short m_nSMTPPort;
  std::string mySmtpHost;
  std::string mySmtpTo;
  std::string mySmtpFrom;
  std::string mySmtpDomain;
  Licq::UserId myUserId;
  unsigned m_nForwardType;

  TCPSocket *tcp;

public:
  void ProcessPipe();
  void ProcessSignal(LicqSignal* s);
  void ProcessEvent(LicqEvent* e);

  void ProcessUserEvent(const Licq::UserId& userId, unsigned long nId);
  bool ForwardEvent(const Licq::User* u, const CUserEvent* e);
  bool ForwardEvent_ICQ(const Licq::User* u, const CUserEvent* e);
  bool ForwardEvent_Email(const Licq::User* u, const CUserEvent* e);

private:
  bool CreateDefaultConfig();

};


#endif
