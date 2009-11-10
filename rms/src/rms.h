#ifndef LICQRMS_H
#define LICQRMS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <list>

#include <licq_socket.h>
#include <licq_types.h>

class CICQDaemon;
class TCPSocket;
class CUserEvent;
class LicqSignal;
class LicqEvent;
class CLogService_Plugin;

const unsigned short MAX_LINE_LENGTH = 1024 * 1;
const unsigned short MAX_TEXT_LENGTH = 1024 * 8;

typedef std::list<class CRMSClient*> ClientList;
typedef std::list<unsigned long> TagList;

class CLicqRMS
{
public:
  CLicqRMS(bool, unsigned short);
  ~CLicqRMS();
  int Run(CICQDaemon *);
  void Shutdown();
  bool Enabled() { return m_bEnabled; }

protected:
  int m_nPipe;
  bool m_bExit, m_bEnabled;

  unsigned short m_nPort;

  TCPSocket *server;
  ClientList clients;
  CLogService_Plugin *log;

public:
  void ProcessPipe();
  void ProcessSignal(LicqSignal* s);
  void ProcessEvent(LicqEvent* e);
  void ProcessServer();
  void ProcessLog();
  void AddEventTag(const UserId& userId, unsigned long eventTag);

friend class CRMSClient;

};


class CRMSClient
{
public:
  CRMSClient(TCPSocket *);
  ~CRMSClient();

  int Activity();

  static CSocketManager sockman;

  int Process_QUIT();
  int Process_TERM();
  int Process_INFO();
  int Process_STATUS();
  int Process_HELP();
  int Process_GROUPS();
  int Process_HISTORY();
  int Process_LIST();
  int Process_MESSAGE();
  int Process_URL();
  int Process_SMS();
  int Process_LOG();
  int Process_VIEW();
  int Process_AR();
  int Process_ADDUSER();
  int Process_REMUSER();
  int Process_SECURE();
  int Process_NOTIFY();

protected:
  TCPSocket sock;
  FILE *fs;
  TagList tags;
  unsigned short m_nState;
  char data_line[MAX_LINE_LENGTH + 1];
  char *data_arg;
  unsigned short data_line_pos;
  unsigned long m_nCheckUin;
  char *m_szCheckId;
  unsigned long m_nLogTypes;
  bool m_bNotify;

  unsigned long m_nUin;
  UserId myUserId;
  char m_szText[MAX_TEXT_LENGTH + 1];
  char m_szLine[MAX_LINE_LENGTH + 1];
  unsigned short m_nTextPos;
  UserId myEventUserId;

  int StateMachine();
  int ProcessCommand();
  bool ProcessEvent(LicqEvent* e);
  bool AddLineToText();
  unsigned long GetProtocol(const char *);
  void ParseUser(const char *);
  int ChangeStatus(unsigned long, unsigned long, const char *);
  void AddEventTag(const UserId& userId, unsigned long);

  int Process_MESSAGE_text();
  int Process_URL_url();
  int Process_URL_text();
  int Process_SMS_number();
  int Process_SMS_message();
  int Process_AR_text();

  /**
   * Output a user event
   *
   * @param e User event
   * @param alias Alias of sender
   */
  void printUserEvent(const CUserEvent* e, const std::string& alias);

friend class CLicqRMS;
};

extern CLicqRMS *licqRMS;


#endif
