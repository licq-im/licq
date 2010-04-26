#ifndef LICQEMAIL_H
#define LICQEMAIL_H

class TCPSocket;
class CUserEvent;
class LicqSignal;
class LicqEvent;

namespace Licq
{
class User;
class UserId;
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
  char m_szSMTPHost[256], m_szSMTPTo[256], m_szSMTPFrom[256],
       m_szSMTPDomain[256];
  char myUserId[16];
  unsigned short m_nForwardType;

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
