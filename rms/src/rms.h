#ifndef LICQRMS_H
#define LICQRMS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licq_socket.h"


class CICQDaemon;
class TCPSocket;
class ICQUser;
class CUserEvent;
class CICQSignal;
class ICQEvent;
class CLogService_Plugin;

const unsigned short MAX_LINE_LENGTH = 1024 * 1;
const unsigned short MAX_TEXT_LENGTH = 1024 * 8;

typedef list<class CRMSClient *> ClientList;
typedef list<unsigned long> TagList;

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
  void ProcessSignal(CICQSignal *);
  void ProcessEvent(ICQEvent *);
  void ProcessServer();
  void ProcessLog();

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
  int Process_INFO();
  int Process_STATUS();
  int Process_HELP();
  int Process_GROUPS();
  int Process_LIST();
  int Process_MESSAGE();
  int Process_URL();
  int Process_LOG();
  int Process_VIEW();
  int Process_AR();

protected:
  TCPSocket sock;
  FILE *fs;
  TagList tags;
  unsigned short m_nState;
  char data_line[MAX_LINE_LENGTH];
  char *data_arg;
  unsigned short data_line_pos;
  unsigned long m_nCheckUin;
  unsigned long m_nLogTypes;

  unsigned long m_nUin;
  char m_szText[MAX_TEXT_LENGTH];
  char m_szLine[MAX_LINE_LENGTH];
  unsigned short m_nTextPos;

  int StateMachine();
  int ProcessCommand();
  bool ProcessEvent(ICQEvent *);
  bool AddLineToText();

  int Process_MESSAGE_text();
  int Process_URL_url();
  int Process_URL_text();
  int Process_AR_text();


  static char buf[128];

friend class CLicqRMS;
};

extern CLicqRMS *licqRMS;


#endif
