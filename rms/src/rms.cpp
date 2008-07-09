#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rms.h"

#include <cctype>
#include <climits>
#include <cstdio>
#include <cstring>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef HAVE_ERRNO_H
#include <cerrno>
#else
extern int errno;
#endif

#include <licq_constants.h>
#include <licq_icqd.h>
#include <licq_file.h>
#include <licq_log.h>
#include <licq_socket.h>
#include <licq_user.h>

using namespace std;

extern "C" { const char *LP_Version(); }

CLicqRMS *licqRMS = NULL;
static CICQDaemon *licqDaemon = NULL;

const char L_RMSxSTR[]  = "[RMS] ";

// 100 - information
const unsigned short CODE_QUIT = 100;
const unsigned short CODE_HELP = 101;
const unsigned short CODE_COMMANDxSTART = 102;
const unsigned short CODE_LOG = 103;
// 200 - fulfilled
const unsigned short CODE_HELLO = 200;
const unsigned short CODE_USERxINFO = 201;
const unsigned short CODE_STATUS = 202;
const unsigned short CODE_RESULTxSUCCESS = 203;
const unsigned short CODE_LISTxUSER = 204;
const unsigned short CODE_LISTxGROUP = 205;
const unsigned short CODE_LISTxDONE = 206;
const unsigned short CODE_LOGxTYPE = 207;
const unsigned short CODE_VIEWxMSG = 208;
const unsigned short CODE_VIEWxURL = 209;
const unsigned short CODE_VIEWxCHAT= 210;
const unsigned short CODE_VIEWxFILE = 211;
const unsigned short CODE_STATUSxDONE = 212;
const unsigned short CODE_VIEWxTIME = 220;
const unsigned short CODE_VIEWxFLAGS = 221;
const unsigned short CODE_VIEWxTEXTxSTART = 222;
const unsigned short CODE_VIEWxTEXTxEND = 223;
const unsigned short CODE_ADDUSERxDONE = 224;
const unsigned short CODE_REMUSERxDONE = 225;
const unsigned short CODE_SECURExOPEN = 226;
const unsigned short CODE_SECURExCLOSE = 227;
const unsigned short CODE_SECURExSTAT = 228;
const unsigned short CODE_NOTIFYxON = 229;
const unsigned short CODE_NOTIFYxOFF = 230;
const unsigned short CODE_VIEWxUNKNOWN = 299;
// 300 - further action required
const unsigned short CODE_ENTERxUIN = 300;
const unsigned short CODE_ENTERxPASSWORD = 301;
const unsigned short CODE_ENTERxTEXT = 302;
const unsigned short CODE_ENTERxLINE = 303;
// 400 - client error
const unsigned short CODE_INVALID = 400;
const unsigned short CODE_INVALIDxCOMMAND = 401;
const unsigned short CODE_INVALIDxUSER = 402;
const unsigned short CODE_INVALIDxSTATUS = 403;
const unsigned short CODE_EVENTxCANCELLED = 404;
const unsigned short CODE_VIEWxNONE = 405;
// 500 - server error
const unsigned short CODE_EVENTxTIMEDOUT = 500;
const unsigned short CODE_EVENTxFAILED = 501;
const unsigned short CODE_EVENTxERROR = 502;
const unsigned short CODE_ADDUSERxERROR = 503;
const unsigned short CODE_SECURExNOTCOMPILED = 504;

const unsigned short CODE_NOTIFYxSTATUS = 600;
const unsigned short CODE_NOTIFYxMESSAGE = 601;

const unsigned short STATE_UIN = 1;
const unsigned short STATE_PASSWORD = 2;
const unsigned short STATE_COMMAND = 3;
const unsigned short STATE_ENTERxMESSAGE = 4;
const unsigned short STATE_ENTERxURLxDESCRIPTION = 5;
const unsigned short STATE_ENTERxURL = 6;
const unsigned short STATE_ENTERxAUTOxRESPONSE = 7;
const unsigned short STATE_ENTERxSMSxMESSAGE = 8;
const unsigned short STATE_ENTERxSMSxNUMBER = 9;

#define NEXT_WORD(s) while (*s != '\0' && *s == ' ') s++;

struct Command
{
  const char *name;
  int (CRMSClient::*fcn)();
  const char *help;
};

static struct Command commands[] =
{
  { "ADDUSER", &CRMSClient::Process_ADDUSER,
    "Add user to contact list { <id>[.<protocol>] }." },
  { "AR", &CRMSClient::Process_AR,
    "Set your (or a user custom) auto response { [ <id>[.<protocol>] ] }." },
  { "GROUPS", &CRMSClient::Process_GROUPS,
    "Show list of groups." },
  { "HELP", &CRMSClient::Process_HELP,
    "Print out help on commands." },
  { "INFO", &CRMSClient::Process_INFO,
    "Print out user information.  Argument is the id and protocol, or none for personal." },
  { "LIST", &CRMSClient::Process_LIST,
    "List users { [ <group #> ] [ <online|offline|all> ] [ <format> ] }." },
  { "LOG", &CRMSClient::Process_LOG,
    "Dump log messages { <log types> }." },
  { "MESSAGE", &CRMSClient::Process_MESSAGE,
    "Send a message { <id>[.<protocol>] }." },
  { "QUIT", &CRMSClient::Process_QUIT,
    "Close the connection.  With an argument of 1 causes the plugin to unload." },
  { "REMUSER", &CRMSClient::Process_REMUSER,
    "Remove user from contact list { <id>[.<protocol>] }." },
  { "SECURE", &CRMSClient::Process_SECURE,
    "Open/close/check secure channel { <uin> [ <open|close> ] } ." },
  { "STATUS", &CRMSClient::Process_STATUS,
    "Set or show status.  Argument is new status and protocol, or blank to display current. { [ <status>[.<protocol>] ] }." },
  { "TERM", &CRMSClient::Process_TERM,
    "Terminate the licq daemon." },
  { "VIEW", &CRMSClient::Process_VIEW,
    "View event (next or specific user) { [ <id>[.<protocol>] ] }." },
  { "URL", &CRMSClient::Process_URL,
    "Send a url { <id>[.<protocol>] }." },
  { "SMS", &CRMSClient::Process_SMS,
    "Send an sms { <uin> }." },
  { "NOTIFY", &CRMSClient::Process_NOTIFY,
    "Notify events" },
};

static const unsigned short NUM_COMMANDS = sizeof(commands)/sizeof(*commands);

/*---------------------------------------------------------------------------
 * CLicqRMS::Constructor
 *-------------------------------------------------------------------------*/
CLicqRMS::CLicqRMS(bool bEnable, unsigned short nPort)
{
  server = NULL;
  log = NULL;
  m_bExit = false;
  m_bEnabled = bEnable;
  m_nPort = nPort;
}


/*---------------------------------------------------------------------------
 * CLicqRMS::Destructor
 *-------------------------------------------------------------------------*/
CLicqRMS::~CLicqRMS()
{
  delete server;
  ClientList::iterator iter;
  for (iter = clients.begin(); iter != clients.end(); iter++)
    delete *iter;
}

/*---------------------------------------------------------------------------
 * CLicqRMS::Shutdown
 *-------------------------------------------------------------------------*/
void CLicqRMS::Shutdown()
{
  gLog.Info("%sShutting down remote manager server.\n", L_RMSxSTR);
  licqDaemon->UnregisterPlugin();
}


/*---------------------------------------------------------------------------
 * CLicqRMS::Run
 *-------------------------------------------------------------------------*/
int CLicqRMS::Run(CICQDaemon *_licqDaemon)
{
  unsigned short nPort;

  // Register with the daemon, we only want the update user signal
  m_nPipe = _licqDaemon->RegisterPlugin(SIGNAL_ALL);
  licqDaemon = _licqDaemon;
  
  char filename[256];
  sprintf (filename, "%s/licq_rms.conf", BASE_DIR);
  CIniFile conf;
  if (conf.LoadFile(filename))
  {
    conf.SetSection("RMS");
    conf.ReadNum("Port", nPort, 0);
    conf.CloseFile();
  }

  server = new TCPSocket();

  if (licqDaemon->TCPPortsLow() != 0 && nPort == 0)
  {
    if (!licqDaemon->StartTCPServer(server)) return 1;
  }
  else
  {
    if (!server->StartServer(nPort))
    {
      gLog.Error("Could not start server on port %u, "
                 "maybe this port is already in use?\n", nPort);
      return 1;
    };
  }

  gLog.Info("%sRMS server started on port %d.\n", L_RMSxSTR, server->LocalPort());
  CRMSClient::sockman.AddSocket(server);
  CRMSClient::sockman.DropSocket(server);

  fd_set f;
  int l;
  int nResult;

  while (!m_bExit)
  {
    f = CRMSClient::sockman.SocketSet();
    l = CRMSClient::sockman.LargestSocket() + 1;

    // Add the new socket pipe descriptor
    FD_SET(m_nPipe, &f);
    if (m_nPipe >= l) l = m_nPipe + 1;
    if (log != NULL)
    {
      FD_SET(log->LogWindow()->Pipe(), &f);
      if (log->LogWindow()->Pipe() >= l) l = log->LogWindow()->Pipe() + 1;
    }

    nResult = select(l, &f, NULL, NULL, NULL);
    if (nResult == -1)
    {
      gLog.Error("%sError in select(): %s\n", L_ERRORxSTR, strerror(errno));
      m_bExit = true;
    }
    else
    {
      while (nResult > 0)
      {
        if (FD_ISSET(m_nPipe, &f))
          ProcessPipe();
        else if (FD_ISSET(server->Descriptor(), &f))
          ProcessServer();
        else if (log != NULL && FD_ISSET(log->LogWindow()->Pipe(), &f))
          ProcessLog();
        else
        {
          ClientList::iterator iter;
          for (iter = clients.begin(); iter != clients.end(); iter++)
          {
            if (FD_ISSET((*iter)->sock.Descriptor(), &f))
            {
              if ((*iter)->Activity() == -1)
              {
                clients.erase(iter);
                delete *iter;
                if (clients.size() == 0 && log != NULL)
                  log->SetLogTypes(0);
              }
              break;
            }
          }
        }
        nResult--;
      }
    }
  }
  return 0;
}


/*---------------------------------------------------------------------------
 * CLicqRMS::ProcessPipe
 *-------------------------------------------------------------------------*/
void CLicqRMS::ProcessPipe()
{
  char buf[16];
  read(m_nPipe, buf, 1);
  switch (buf[0])
  {
  case 'S':  // A signal is pending
  {
    CICQSignal *s = licqDaemon->PopPluginSignal();
    if (m_bEnabled) ProcessSignal(s);
    break;
  }

  case 'E':  // An event is pending (should never happen)
  {
    ICQEvent *e = licqDaemon->PopPluginEvent();
    if (m_bEnabled) ProcessEvent(e);
    break;
  }

  case 'X':  // Shutdown
  {
    gLog.Info("%sExiting.\n", L_RMSxSTR);
    m_bExit = true;
    break;
  }

  case '0':  // disable
  {
    gLog.Info("%sDisabling.\n", L_RMSxSTR);
    m_bEnabled = false;
    break;
  }

  case '1':  // enable
  {
    gLog.Info("%sEnabling.\n", L_RMSxSTR);
    m_bEnabled = true;
    break;
  }

  default:
    gLog.Warn("%sUnknown notification type from daemon: %c.\n", L_WARNxSTR, buf[0]);
  }
}


/*---------------------------------------------------------------------------
 * CLicqRMS::ProcessLog
 *-------------------------------------------------------------------------*/
void CLicqRMS::ProcessLog()
{
  static char buf[2];
  read(log->LogWindow()->Pipe(), buf, 1);

  ClientList::iterator iter;
  for (iter = clients.begin(); iter != clients.end(); iter++)
  {
    if ((*iter)->m_nLogTypes & log->LogWindow()->NextLogType())
    {
      fprintf((*iter)->fs, "%d %s", CODE_LOG, log->LogWindow()->NextLogMsg());
      fflush((*iter)->fs);
    }
  }

  log->LogWindow()->ClearLog();
}


/*---------------------------------------------------------------------------
 * CLicqRMS::ProcessSignal
 *-------------------------------------------------------------------------*/
void CLicqRMS::ProcessSignal(CICQSignal *s)
{
  switch (s->Signal())
  {
  case SIGNAL_UPDATExUSER:
    if (s->SubSignal() == USER_STATUS)
    {
      ICQUser *u = gUserManager.FetchUser(s->Id(), s->PPID(), LOCK_R);
      if (u)
      {
        ClientList::iterator iter;
        for (iter = clients.begin(); iter != clients.end(); iter++)
        {
          if ((*iter)->m_bNotify)
          {
            char format[128], *ubuf;
			strcpy(format, "%u %P %-20a %3m %s");
			ubuf = u->usprintf(format);
            fprintf((*iter)->fs, "%d %s\n", CODE_NOTIFYxSTATUS, ubuf);
			free(ubuf);
            fflush((*iter)->fs);
          }
        }
        gUserManager.DropUser(u);
      }
      break;
    }
    else if (s->SubSignal() == USER_EVENTS)
    {
      ICQUser *u = gUserManager.FetchUser(s->Id(), s->PPID(), LOCK_R);
      if (u)
      {
        ClientList::iterator iter;
        for (iter = clients.begin(); iter != clients.end(); iter++)
        {
          if ((*iter)->m_bNotify)
          {
            char format[128], *ubuf;
			strcpy(format, "%u %P %3m");
			ubuf = u->usprintf(format);
            fprintf((*iter)->fs, "%d %s\n", CODE_NOTIFYxMESSAGE, ubuf);
			free(ubuf);
            fflush((*iter)->fs);
          }
        }
        gUserManager.DropUser(u);
      }
    }
  case SIGNAL_UPDATExLIST:
    break;
  case SIGNAL_LOGON:
    break;
  case SIGNAL_EVENTxID:
	AddEventTag(s->Id(), s->PPID(), s->Argument());
    break;
  default:
    break;
    
  }
  delete s;
}

/*---------------------------------------------------------------------------
 * CLicqRMS::AddEventTag
 *-------------------------------------------------------------------------*/
void CLicqRMS::AddEventTag(const char *_szId, unsigned long _nPPID, unsigned long _nEventTag)
{
  ClientList ::iterator iter;
  for (iter = clients.begin(); iter != clients.end(); iter++)
  {
    (*iter)->AddEventTag(_szId, _nPPID, _nEventTag);
  }
}

/*---------------------------------------------------------------------------
 * CLicqRMS::ProcessEvent
 *-------------------------------------------------------------------------*/
void CLicqRMS::ProcessEvent(ICQEvent *e)
{
  ClientList ::iterator iter;
  for (iter = clients.begin(); iter != clients.end(); iter++)
  {
    if ((*iter)->ProcessEvent(e)) break;
  }

  delete e;
}



/*---------------------------------------------------------------------------
 * CLicqRMS::ProcessServer
 *-------------------------------------------------------------------------*/
void CLicqRMS::ProcessServer()
{
  server->Lock();
  clients.push_back(new CRMSClient(server));
  server->Unlock();
}



CSocketManager CRMSClient::sockman;
char CRMSClient::buf[128];

/*---------------------------------------------------------------------------
 * CRMSClient::constructor
 *-------------------------------------------------------------------------*/
CRMSClient::CRMSClient(TCPSocket* sin)
{
  sin->RecvConnection(sock);
  sockman.AddSocket(&sock);
  sockman.DropSocket(&sock);

  gLog.Info("%sClient connected from %s.\n", L_RMSxSTR, sock.RemoteIpStr(buf));
  fs = fdopen(sock.Descriptor(), "r+");
  fprintf(fs, "Licq Remote Management Server v%s\n"
     "%d Enter your UIN:\n", LP_Version(), CODE_ENTERxUIN);
  fflush(fs);

  m_szCheckId = 0;
  m_szId = 0;
  m_nState = STATE_UIN;
  m_nLogTypes = 0;
  data_line_pos = 0;
  m_bNotify = false;
  m_szEventId = 0;
  m_nEventPPID = 0;
}


/*---------------------------------------------------------------------------
 * CRMSClient::destructor
 *-------------------------------------------------------------------------*/
CRMSClient::~CRMSClient()
{
  sockman.CloseSocket(sock.Descriptor(), false, false);
  
  if (m_szCheckId)
    free(m_szCheckId);
}

/*---------------------------------------------------------------------------
 * CRMSClient::AddEventTag
 *-------------------------------------------------------------------------*/
void CRMSClient::AddEventTag(const char *_szId, unsigned long _nPPID, unsigned long _nEventTag)
{
  if (m_szEventId && m_nEventPPID &&
       !strcmp(m_szEventId, _szId) && m_nEventPPID == _nPPID)
  {
    fprintf(fs, "%d [%ld] Sending message to %s.\n", CODE_COMMANDxSTART,
       _nEventTag, _szId);
    tags.push_back(_nEventTag);
	free(m_szEventId);
	m_szEventId = 0;
	m_nEventPPID = 0;
  }
}

/*---------------------------------------------------------------------------
 * CRMSClient::GetProtocol
 *-------------------------------------------------------------------------*/
unsigned long CRMSClient::GetProtocol(const char *szData)
{
  unsigned long nPPID = 0;
  ProtoPluginsList pl;
  ProtoPluginsListIter it;
  licqDaemon->ProtoPluginList(pl);
  for (it = pl.begin(); it != pl.end(); it++)
  {
    if (strcasecmp((*it)->Name(), szData) == 0)
    {
      nPPID = (*it)->PPID();
      break;
    }
  }
  
  return nPPID;
}

/*---------------------------------------------------------------------------
 * CRMSClient::ParseUser
 *-------------------------------------------------------------------------*/
void CRMSClient::ParseUser(const char *szData)
{
  if (m_szId)
    free(m_szId);
    
  string strData(szData);
  string::size_type nPos= strData.find_last_of(".");
  if (nPos == string::npos)
  {
    m_szId = strdup(data_arg);
    m_nPPID = 0;
    ProtoPluginsList pl;
    ProtoPluginsListIter it;
    licqDaemon->ProtoPluginList(pl);
    for (it = pl.begin(); it != pl.end(); it++)
    {
      ICQUser *u = gUserManager.FetchUser(m_szId, (*it)->PPID(), LOCK_R);
      if (u)
      {
        gUserManager.DropUser(u);
        m_nPPID = (*it)->PPID();
        break;
      }
    }
  }
  else
  {
    string strId(strData, 0, strData.find_last_of("."));
    string strProtocol(strData, strData.find_last_of(".")+1, strData.size());
    m_szId = strdup(strId.c_str());
    m_nPPID = GetProtocol(strProtocol.c_str());
  }
}

/*---------------------------------------------------------------------------
 * CRMSClient::ProcessEvent
 *-------------------------------------------------------------------------*/
bool CRMSClient::ProcessEvent(ICQEvent *e)
{
  TagList::iterator iter;
  for (iter = tags.begin(); iter != tags.end(); iter++)
  {
    if ( e->Equals(*iter) ) break;
  }
  if (iter == tags.end()) return false;

  unsigned long tag = *iter;
  tags.erase(iter);

  unsigned short nCode = 0;
  const char *szr = NULL;
  switch(e->Result())
  {
    case EVENT_ACKED:
    case EVENT_SUCCESS:
      nCode = CODE_RESULTxSUCCESS;
      szr = "done";
      break;
    case EVENT_TIMEDOUT:
      nCode = CODE_EVENTxTIMEDOUT;
      szr = "timed out";
      break;
    case EVENT_FAILED:
      nCode = CODE_EVENTxFAILED;
      szr = "failed";
      break;
    case EVENT_ERROR:
      nCode = CODE_EVENTxERROR;
      szr = "error";
      break;
    case EVENT_CANCELLED:
      nCode = CODE_EVENTxCANCELLED;
      szr = "cancelled";
      break;
  }
  fprintf(fs, "%d [%ld] Event %s.\n", nCode, tag, szr);
  fflush(fs);

  return true;
}


/*---------------------------------------------------------------------------
 * CRMSClient::Activity
 *-------------------------------------------------------------------------*/
int CRMSClient::Activity()
{
  if (!sock.RecvRaw())
  {
    gLog.Info("%sClient %s disconnected.\n", L_RMSxSTR, sock.RemoteIpStr(buf));
    return -1;
  }

  char *in = sock.RecvBuffer().getDataStart();
  char *last = sock.RecvBuffer().getDataPosWrite();

  do
  {
    while (in != last && *in != '\n')
    {
      if (!iscntrl(*in) && data_line_pos < MAX_LINE_LENGTH)
        data_line[data_line_pos++] = *in;
      in++;
    }

    if (in != last && *in == '\n')
    {
      data_line[data_line_pos] = '\0';
      in++;
      if (StateMachine() == -1) return -1;

      data_line_pos = 0;
    }

  } while (in != last);

  data_line[data_line_pos] = '\0';

  sock.ClearRecvBuffer();

  return 0;
}


/*---------------------------------------------------------------------------
 * CRMSClient::StateMachine
 *-------------------------------------------------------------------------*/
int CRMSClient::StateMachine()
{
  switch(m_nState)
  {
    case STATE_UIN:
    {
      m_szCheckId = data_line ? strdup(data_line) : 0;
      fprintf(fs, "%d Enter your password:\n", CODE_ENTERxPASSWORD);
      fflush(fs);
      m_nState = STATE_PASSWORD;
      break;
    }
    case STATE_PASSWORD:
    {
      ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
      if (o == NULL)
        return -1;

      bool ok = (strcmp(m_szCheckId, o->IdString()) == 0 &&
         (strcmp(o->Password(), data_line) == 0));
      free(m_szCheckId);
      m_szCheckId = 0;
      if (!ok)
      {
        gUserManager.DropOwner(o);
        gLog.Info("%sClient failed validation from %s.\n", L_RMSxSTR,
           sock.RemoteIpStr(buf));
        fprintf(fs, "%d Invalid ID/Password.\n", CODE_INVALID);
        fflush(fs);
        return -1;
      }
      gLog.Info("%sClient validated from %s.\n", L_RMSxSTR,
         sock.RemoteIpStr(buf));
      fprintf(fs, "%d Hello %s.  Type HELP for assistance.\n", CODE_HELLO,
         o->GetAlias());
      fflush(fs);
      gUserManager.DropOwner(o);
      m_nState = STATE_COMMAND;
      break;
    }
    case STATE_COMMAND:
    {
      if (ProcessCommand() == -1) return -1;
      break;
    }
    case STATE_ENTERxMESSAGE:
    {
      if (AddLineToText())
         return Process_MESSAGE_text();
      break;
    }
    case STATE_ENTERxURLxDESCRIPTION:
    {
      if (AddLineToText())
         return Process_URL_text();
      break;
    }
    case STATE_ENTERxURL:
    {
      return Process_URL_url();
    }
    case STATE_ENTERxSMSxMESSAGE:
    {
      if (AddLineToText())
         return Process_SMS_message();
      break;
    }
    case STATE_ENTERxSMSxNUMBER:
    {
      return Process_SMS_number();
    }
    case STATE_ENTERxAUTOxRESPONSE:
    {
      if (AddLineToText())
         return Process_AR_text();
      break;
    }
  }
  return 0;
}


/*---------------------------------------------------------------------------
 * CRMSClient::AddLineToText
 *-------------------------------------------------------------------------*/
bool CRMSClient::AddLineToText()
{
  if (data_line[0] == '.' && data_line[1] == '\0') return true;

  m_nTextPos += sprintf(&m_szText[m_nTextPos], "%s\n", data_line);

  return false;
}


/*---------------------------------------------------------------------------
 * CRMSClient::ProcessCommand
 *-------------------------------------------------------------------------*/
int CRMSClient::ProcessCommand()
{
  data_arg = data_line;
  while (*data_arg != '\0' && *data_arg != ' ') data_arg++;
  if (*data_arg == ' ')
  {
    *data_arg++ = '\0';
    NEXT_WORD(data_arg);
  }

  for (unsigned short i = 0; i < NUM_COMMANDS; i++)
  {
    if (strcasecmp(commands[i].name, data_line) == 0)
      return  (this->*(commands[i].fcn))();
  }

  fprintf(fs, "%d Invalid command.  Type HELP for assistance.\n",
     CODE_INVALIDxCOMMAND);
  return fflush(fs);
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_INFO
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_INFO()
{
  char *szId = strdup(data_arg);
  NEXT_WORD(data_arg);
  unsigned long nPPID = GetProtocol(data_arg);

  //XXX Handle the case when we have the owner
  if (szId == 0)
    m_nUin = strtoul(gUserManager.OwnerId(LICQ_PPID).c_str(), (char**)NULL, 10);

  // Print the user info
  ICQUser *u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
  if (u == NULL)
  {
    fprintf(fs, "%d No such user.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }

  fprintf(fs, "%d %s Alias: %s\n", CODE_USERxINFO, u->IdString(),
    u->GetAlias());
  fprintf(fs, "%d %s Status: %s\n", CODE_USERxINFO, u->IdString(),
    u->StatusStr());
  fprintf(fs, "%d %s First Name: %s\n", CODE_USERxINFO, u->IdString(),
    u->GetFirstName());
  fprintf(fs, "%d %s Last Name: %s\n", CODE_USERxINFO, u->IdString(),
    u->GetLastName());
  fprintf(fs, "%d %s Email 1: %s\n", CODE_USERxINFO, u->IdString(),
    u->GetEmailPrimary());
  fprintf(fs, "%d %s Email 2: %s\n", CODE_USERxINFO, u->IdString(),
    u->GetEmailSecondary());

  gUserManager.DropUser(u);

  if (szId)
    free(szId);
    
  return fflush(fs);
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_STATUS
 *
 * Command:
 *   STATUS [ status | protocol ]
 *
 * Response:
 *
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_STATUS()
{
  // Show status
  if (data_arg[0] == '\0')
  {
    ProtoPluginsList l;
    ProtoPluginsListIter it;
    licqDaemon->ProtoPluginList(l);
    for (it = l.begin(); it != l.end(); it++)
    {
      ICQOwner *o = gUserManager.FetchOwner((*it)->PPID(), LOCK_R);
      if (o)
      {
        fprintf(fs, "%d %s %s %s\n", CODE_STATUS, o->IdString(), (*it)->Name(), o->StatusStr());
        gUserManager.DropOwner(o);
      }
    }
    fprintf(fs, "%d\n", CODE_STATUSxDONE);
    return fflush(fs);
  }

  // Set status
  string strData(data_arg);
  string::size_type nPos = strData.find_last_of(".");
  if (nPos == string::npos)
  {
    unsigned long nStatus = StringToStatus(data_arg);
    ProtoPluginsList l;
    ProtoPluginsListIter it;
    licqDaemon->ProtoPluginList(l);
    for (it = l.begin(); it != l.end(); it++)
    {
      ChangeStatus((*it)->PPID(), nStatus, data_arg);
    }
  }
  else
  {
    string strStatus(strData, 0, strData.find_last_of("."));
    string strProtocol(strData, strData.find_last_of(".")+1, strData.size());
    unsigned long nPPID = GetProtocol(strProtocol.c_str());
    char *szStatus = strdup(strStatus.c_str());
    unsigned long nStatus = StringToStatus(szStatus);
    ChangeStatus(nPPID, nStatus, szStatus);
    free(szStatus);
  }
  fprintf(fs, "%d Done setting status\n", CODE_STATUSxDONE);
  return fflush(fs);
}

int CRMSClient::ChangeStatus(unsigned long nPPID, unsigned long nStatus, const char *szStatus)
{
  if (nStatus == INT_MAX)
  {
    fprintf(fs, "%d Invalid status.\n", CODE_INVALIDxSTATUS);
    return -1;
  }
  if (nStatus == ICQ_STATUS_OFFLINE)
  {
    fprintf(fs, "%d [0] Logging off %s.\n", CODE_COMMANDxSTART, szStatus);
    fflush(fs);
    licqDaemon->ProtoLogoff(nPPID);
    fprintf(fs, "%d [0] Event done.\n", CODE_STATUSxDONE);
    return 0;
  }
  else
  {
    ICQOwner *o = gUserManager.FetchOwner(nPPID, LOCK_R);
    bool b = o->StatusOffline();
    gUserManager.DropOwner(o);
    unsigned long tag = 0;
    if (b)
    {
      tag = licqDaemon->ProtoLogon(nPPID, nStatus);
      fprintf(fs, "%d [%ld] Logging on to %s.\n", CODE_COMMANDxSTART, tag, szStatus);
    }
    else
    {
      tag = licqDaemon->ProtoSetStatus(nPPID, nStatus);
      fprintf(fs, "%d [%ld] Setting status for %s.\n", CODE_COMMANDxSTART, tag, szStatus);
    }
    tags.push_back(tag);
  }
  return 0;
}

/*---------------------------------------------------------------------------
 * CRMSClient::Process_QUIT
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_QUIT()
{
  fprintf(fs, "%d Sayonara.\n", CODE_QUIT);
  fflush(fs);
  if (strtoul(data_arg, (char**)NULL, 10) > 0) licqRMS->m_bExit = true;
  return -1;
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_TERM
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_TERM()
{
  licqDaemon->Shutdown();
  return -1;
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_HELP
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_HELP()
{
  for (unsigned short i = 0; i < NUM_COMMANDS; i++)
  {
    fprintf(fs, "%d %s: %s\n", CODE_HELP, commands[i].name, commands[i].help);
  }
  return fflush(fs);
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_GROUPS
 *
 * Command:
 *  GROUPS
 *    Prints out the list of groups as follows.
 *
 * Response:
 *  CODE_LISTxGROUP 000 All Users
 *  CODE_LISTxGROUP 001 First Group
 *  ...
 *  CODE_LISTxDONE
 *
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_GROUPS()
{
  fprintf(fs, "%d 000 All Users\n", CODE_LISTxGROUP);
  int i = 1;
  FOR_EACH_GROUP_START_SORTED(LOCK_R)
  {
    fprintf(fs, "%d %03d %s\n", CODE_LISTxGROUP, i, pGroup->name().c_str());
    ++i;
  }
  FOR_EACH_GROUP_END
  fprintf(fs, "%d\n", CODE_LISTxDONE);

  return fflush(fs);
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_LIST
 *
 * Command:
 *   LIST [ group ] [ online|offline|all ] [ format ]
 *     All options are optional and can be left out arbitrarily, ie
 *     "LIST all" is a valid call and will print all online and offline users.
 *     <format> is a printf style string using the user % symbols as
 *     documented in UTILITIES.HOWTO.  The default is "%u %P %-20a %3m %s"
 *     and prints out users as follows.
 *
 * Response:
 *   CODE_LISTxUSER   5550000 Licq            AnAlias   2 Online
 *     The default line contains the uin, protocol, alias, number of new 
 *     messages and status all column and white space deliminated.  Note that
 *     the alias may contain white space.
 *   CODE_LISTxUSER ...
 *   ...
 *   CODE_LISTxDONE
 *
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_LIST()
{
  unsigned short nGroup = 0;
  if (isdigit(*data_arg))
  {
    nGroup = strtoul(data_arg, (char**)NULL, 10);
    while (*data_arg != '\0' && *data_arg != ' ') data_arg++;
    NEXT_WORD(data_arg);
  }

  unsigned short n = 3;
  if (strncasecmp(data_arg, "online", 6) == 0)
  {
    n = 1;
    data_arg += 6;
  }
  else if (strncasecmp(data_arg, "offline", 7) == 0)
  {
    n = 2;
    data_arg += 7;
  }
  else if (strncasecmp(data_arg, "all", 3) == 0)
  {
    n = 3;
    data_arg += 3;
  }
  NEXT_WORD(data_arg);

  char format[128], *ubuf;
  if (*data_arg == '\0')
  {
    strcpy(format, "%u %P %-20a %3m %s");
  }
  else
  {
    strcpy(format, data_arg);
  }

  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->GetInGroup(GROUPS_USER, nGroup) &&
        ((pUser->StatusOffline() && n&2) || (!pUser->StatusOffline() && n&1)))
    {
      ubuf = pUser->usprintf(format);
      fprintf(fs, "%d %s\n", CODE_LISTxUSER, ubuf);
      free(ubuf);
    }
  }
  FOR_EACH_USER_END
  fprintf(fs, "%d\n", CODE_LISTxDONE);

  return fflush(fs);
}



/*---------------------------------------------------------------------------
 * CRMSClient::Process_MESSAGE
 *
 * Command:
 *     MESSAGE <id>[.<protocol>]
 *
 * Response:
 *   CODE_ENTERxTEXT | CODE_INVALIDxUSER
 *     At which point the message should be entered line by line and
 *     terminated by entering a "." on a line by itself.  Invalid user
 *     means the uin was invalid (< 10000) and the message was aborted.
 *   CODE_COMMANDxSTART
 *     < ...time... >
 *   CODE_RESULTxSUCCESS | CODE_EVENTxTIMEDOUT | CODE_EVENTxERROR
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_MESSAGE()
{
  if (m_nEventPPID || m_szEventId)
  {
    //client is trying to send another message before we've received
    //the event tag for a previous one
    fprintf(fs, "%d Error, cannot send concurrent messages\n", CODE_INVALIDxCOMMAND);
    return -1;
  }
  fprintf(fs, "%d Enter message, terminate with a . on a line by itself:\n",
     CODE_ENTERxTEXT);

  ParseUser(data_arg);

  m_szText[0] = '\0';
  m_nTextPos = 0;

  m_nState = STATE_ENTERxMESSAGE;
  return fflush(fs);
}

int CRMSClient::Process_MESSAGE_text()
{
  //XXX Give a tag...
  m_szText[strlen(m_szText) - 1] = '\0';
  unsigned long tag = licqDaemon->ProtoSendMessage(m_szId, m_nPPID, m_szText,
    false, ICQ_TCPxMSG_NORMAL);

  m_nState = STATE_COMMAND;

  if (m_nPPID == LICQ_PPID)
  {
    fprintf(fs, "%d [%ld] Sending message to %s.\n", CODE_COMMANDxSTART,
       tag, m_szId);
    tags.push_back(tag);
  }
  else
  {
    m_nEventPPID = m_nPPID;
    m_szEventId = strdup(m_szId);
  }
  return fflush(fs);
}



/*---------------------------------------------------------------------------
 * CRMSClient::Process_URL
 *
 * Command:
 *   URL <id>[.<protocol>]
 *
 * Response:
 *   CODE_ENTERxLINE | CODE_INVALIDxUSER
 *     At which point the url should be entered on a line by itself.
 *     Invalid user means the uin was invalid (< 10000) and the url
 *     was aborted.
 *   CODE_ENTERxTEXT
 *     Now the description should be entered and terminated by a "." on
 *     a line by itself.
 *   CODE_COMMANDxSTART
 *     < ...time... >
 *   CODE_RESULTxSUCCESS | CODE_EVENTxTIMEDOUT | CODE_EVENTxERROR
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_URL()
{
  ParseUser(data_arg);
  
  m_nTextPos = 0;

  m_nState = STATE_ENTERxURL;
  return fflush(fs);
}


int CRMSClient::Process_URL_url()
{
  strcpy(m_szLine, data_line);

  fprintf(fs, "%d Enter description, terminate with a . on a line by itself:\n",
     CODE_ENTERxTEXT);

  m_szText[0] = '\0';
  m_nTextPos = 0;

  m_nState = STATE_ENTERxURLxDESCRIPTION;
  return fflush(fs);
}


int CRMSClient::Process_URL_text()
{
  unsigned long tag = licqDaemon->ProtoSendUrl(m_szId, m_nPPID, m_szLine,
    m_szText, false, ICQ_TCPxMSG_NORMAL);

  fprintf(fs, "%d [%ld] Sending URL to %s.\n", CODE_COMMANDxSTART,
     tag, m_szId);

  if (m_nPPID == LICQ_PPID)
    tags.push_back(tag);
    
  m_nState = STATE_COMMAND;

  return fflush(fs);
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_SMS
 *
 * Command:
 *   SMS <uin>
 *
 * Response:
 *   CODE_ENTERxLINE | CODE_INVALIDxUSER
 *     At which point the phone number should be entered on a line by itself
 *     without the "+", but including country code.  Invalid user means the
 *     uin was invalid (< 10000) and the url was aborted.
 *   CODE_ENTERxTEXT
 *     Now the message should be entered and terminated by a "." on a line
 *     by itself.
 *   CODE_COMMANDxSTART
 *     < ...time... >
 *   CODE_RESULTxSUCCESS | CODE_EVENTxTIMEDOUT | CODE_EVENTxERROR
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_SMS()
{
  unsigned long nUin = strtoul(data_arg, (char**)NULL, 10);

  if (nUin < 10000)
  {
    fprintf(fs, "%d Invalid UIN.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }
  fprintf(fs, "%d Enter NUMBER:\n", CODE_ENTERxLINE);

  m_nUin = nUin;
  m_nTextPos = 0;

  m_nState = STATE_ENTERxSMSxNUMBER;
  return fflush(fs);
}


int CRMSClient::Process_SMS_number()
{
  strcpy(m_szLine, data_line);

  fprintf(fs, "%d Enter message, terminate with a . on a line by itself:\n",
     CODE_ENTERxTEXT);

  m_szText[0] = '\0';
  m_nTextPos = 0;

  m_nState = STATE_ENTERxSMSxMESSAGE;
  return fflush(fs);
}


int CRMSClient::Process_SMS_message()
{
  char id[16];
  snprintf(id, 16, "%lu", m_nUin);
  unsigned long tag = licqDaemon->icqSendSms(id, LICQ_PPID, m_szLine, m_szText);

  fprintf(fs, "%d [%lu] Sending SMS to %lu (%s).\n", CODE_COMMANDxSTART,
     tag, m_nUin, m_szLine);

  tags.push_back(tag);
  m_nState = STATE_COMMAND;

  return fflush(fs);
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_AR
 *
 * Command:
 *     AR [ <id>[.<protocol>] ]
 *
 * Response:
 *   CODE_ENTERxTEXT | CODE_INVALIDxUIN
 *     At which point the auto response should be entered line by line and
 *     terminated by entering a "." on a line by itself.
 *   CODE_RESULTxSUCCESS
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_AR()
{
  ParseUser(data_arg);

  if (m_szId && !gUserManager.IsOnList(m_szId, m_nPPID))
  {
    fprintf(fs, "%d Invalid User.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }

  fprintf(fs, "%d Enter %sauto response, terminate with a . on a line by itself:\n",
     CODE_ENTERxTEXT, m_szId == 0 ? "" : "custom " );

  m_szText[0] = '\0';
  m_nTextPos = 0;

  m_nState = STATE_ENTERxAUTOxRESPONSE;
  return fflush(fs);
}

int CRMSClient::Process_AR_text()
{
  if (m_szId == 0)
  {
    ICQOwner *o = gUserManager.FetchOwner(m_nPPID, LOCK_W);
    o->SetAutoResponse(m_szText);
    gUserManager.DropOwner(o);
  }
  else
  {
    ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
    u->SetCustomAutoResponse(m_szText);
    gUserManager.DropUser(u);
  }

  fprintf(fs, "%d Auto response saved.\n", CODE_RESULTxSUCCESS);
  m_nState = STATE_COMMAND;
  return fflush(fs);
}




/*---------------------------------------------------------------------------
 * CRMSClient::Process_LOG
 *
 * Command:
 *   LOG <log types>
 *
 * Response:
 *   CODE_LOG 12:04:34 [TCP] Message from ...
 *   ...
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_LOG()
{
  unsigned short lt = strtoul(data_arg, (char**)NULL, 10);

  if (licqRMS->log == NULL)
  {
    licqRMS->log = new CLogService_Plugin(new CPluginLog, 0);
    gLog.AddService(licqRMS->log);
  }

  licqRMS->log->SetLogTypes(lt);
  m_nLogTypes = lt;

  fprintf(fs, "%d Log type set to %d.\n", CODE_LOGxTYPE, lt);

  return fflush(fs);
}

/*---------------------------------------------------------------------------
 * CRMSClient::Process_NOTIFY
 *
 * Command:
 *   NOTIFY
 *
 * Response:
 *   CODE_NOTIFYxON|CODE_NOTIFYxOFF
 *   ...
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_NOTIFY()
{
  m_bNotify = !m_bNotify;

  if (m_bNotify)
    fprintf(fs, "%d Notify set ON.\n", CODE_NOTIFYxON);
  else
    fprintf(fs, "%d Notify set OFF.\n", CODE_NOTIFYxOFF);

  return fflush(fs);
}

/*---------------------------------------------------------------------------
 * CRMSClient::Process_VIEW
 *
 * Command:
 *   VIEW [ <id>[.<protocol> ]
 *
 * Response:
 *
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_VIEW()
{
  if (*data_arg != '\0')
  {
    ParseUser(data_arg);
  }
  else
  {
    // XXX Check system messages first

    // Check user messages now
    FOR_EACH_USER_START(LOCK_R)
    {
      if(pUser->NewMessages() > 0)
      {
        if (m_szId)
          free(m_szId);
        m_szId = strdup(pUser->IdString());
        m_nPPID = pUser->PPID();
        FOR_EACH_USER_BREAK
      }
    }
    FOR_EACH_USER_END
  
    if (m_szId == 0)
    {
      fprintf(fs, "%d No new messages.\n", CODE_VIEWxNONE);
      return fflush(fs);
    }
  }

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  if (u == NULL)
  {
    fprintf(fs, "%d No such user.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }

  CUserEvent *e = u->EventPop();
  if (e)
  {
    char szEventHeader[75]; // Allows 50 chars for a nick
    switch (e->SubCommand())
    {
      case ICQ_CMDxSUB_MSG:
        sprintf(szEventHeader, "%d Message ", CODE_VIEWxMSG);
        break;

      case ICQ_CMDxSUB_URL:
        sprintf(szEventHeader, "%d URL ", CODE_VIEWxURL);
        break;

      case ICQ_CMDxSUB_CHAT:
        sprintf(szEventHeader, "%d Chat Request ", CODE_VIEWxCHAT);
        break;

      case ICQ_CMDxSUB_FILE:
        sprintf(szEventHeader, "%d File Request ", CODE_VIEWxFILE);
        break;

      default:
        sprintf(szEventHeader, "%d Unknown Event ", CODE_VIEWxUNKNOWN);
    }

    strcat(szEventHeader, "from ");
    strncat(szEventHeader, u->GetAlias(), 50);
    strcat(szEventHeader, "\n\0");

    // Write out the event header
    fprintf(fs, szEventHeader);

    // Timestamp
    char szTimestamp[39];
    char szTime[25];
    time_t nMessageTime = e->Time();
    struct tm *pTM = localtime(&nMessageTime);
    strftime(szTime, 25, "%H:%M:%S", pTM);
    sprintf(szTimestamp, "%d Sent At ", CODE_VIEWxTIME);
    strncat(szTimestamp, szTime, 25);
    strcat(szTimestamp, "\n\0");
    fprintf(fs, szTimestamp);

    // Message
    fprintf(fs, "%d Message Start\n", CODE_VIEWxTEXTxSTART);
    fprintf(fs, "%s", e->Text());
    fprintf(fs, "\n");
    fprintf(fs, "%d Message Complete\n", CODE_VIEWxTEXTxEND);
  }
  else
  {
    fprintf(fs, "%d Invalid event\n", CODE_EVENTxERROR);
  }

  gUserManager.DropUser(u);
    
  return fflush(fs);
}

/*---------------------------------------------------------------------------
 * CRMSClient::Process_ADDUSER
 *
 * Command:
 *   ADDUSER <id> <protocol>
 *
 * Response:
 *
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_ADDUSER()
{
  char *szId = strdup(data_arg);
  NEXT_WORD(data_arg);
  unsigned long nPPID = GetProtocol(data_arg);

  if (licqDaemon->AddUserToList(szId, nPPID))
  {
    fprintf(fs, "%d User added\n", CODE_ADDUSERxDONE);
  }
  else
  {
    fprintf(fs, "%d User not added\n", CODE_ADDUSERxERROR);
  }

  free(szId);
  return fflush(fs);
}

/*---------------------------------------------------------------------------
 * CRMSClient::Process_REMUSER
 *
 * Command:
 *   REMUSER <uin>
 *
 * Response:
 *
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_REMUSER()
{
  unsigned long nUin = strtoul(data_arg, (char**)NULL, 10);

  if (nUin >= 10000)
  {
    licqDaemon->RemoveUserFromList(data_arg, LICQ_PPID);
    fprintf(fs, "%d User removed\n", CODE_REMUSERxDONE);
  }
  else
  {
    fprintf(fs, "%d Invalid UIN.\n", CODE_INVALIDxUSER);
  }

  return fflush(fs);
}

/*---------------------------------------------------------------------------
 * CRMSClient::Process_SECURE
 *
 * Command:
 *   SECURE <uin> <what>
 *
 * Response:
 *
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_SECURE()
{
  unsigned long nUin = 0;

  if(!licqDaemon->CryptoEnabled())
  {
    fprintf(fs, "%d Licq secure channel not compiled. Please recompile with OpenSSL.\n", CODE_SECURExNOTCOMPILED);
    return fflush(fs);
  }

  if (!isdigit(*data_arg))
  {
    fprintf(fs, "%d Invalid UIN.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }
  char* id = strdup(data_arg);
  nUin = strtoul(data_arg, (char**)NULL, 10);
  while (*data_arg != '\0' && *data_arg != ' ') data_arg++;
  NEXT_WORD(data_arg);

  if (nUin < 10000)
  {
    fprintf(fs, "%d Invalid UIN.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }

  if (strncasecmp(data_arg, "open", 4) == 0)
  {
    fprintf(fs, "%d Opening secure connection.\n", CODE_SECURExOPEN);
    licqDaemon->icqOpenSecureChannel(id);
  }
  else
  if (strncasecmp(data_arg, "close", 5) == 0)
  {
    fprintf(fs, "%d Closing secure connection.\n", CODE_SECURExCLOSE);
    licqDaemon->icqCloseSecureChannel(id);
  }
  else
  {
    ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_R);
   if (u->Secure() == 0)
   {
    fprintf(fs, "%d Status: secure connection is closed.\n", CODE_SECURExSTAT);
   }
   if (u->Secure() == 1)
   {
    fprintf(fs, "%d Status: secure connection is open.\n", CODE_SECURExSTAT);
   }
   gUserManager.DropUser(u);
  }

  free(id);
  return fflush(fs);
}
