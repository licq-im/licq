#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#include "rms.h"
#include "licq_log.h"
#include "licq_icqd.h"
#include "licq_file.h"
#include "licq_user.h"
#include "licq_constants.h"

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
const unsigned short CODE_VIEWxTIME = 220;
const unsigned short CODE_VIEWxFLAGS = 221;
const unsigned short CODE_VIEWxTEXTxSTART = 222;
const unsigned short CODE_VIEWxTEXTxEND = 223;
const unsigned short CODE_ADDUSERxDONE = 224;
const unsigned short CODE_REMUSERxDONE = 225;
const unsigned short CODE_SECURExOPEN = 226;
const unsigned short CODE_SECURExCLOSE = 227;
const unsigned short CODE_SECURExSTAT = 228;
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

const unsigned short STATE_UIN = 1;
const unsigned short STATE_PASSWORD = 2;
const unsigned short STATE_COMMAND = 3;
const unsigned short STATE_ENTERxMESSAGE = 4;
const unsigned short STATE_ENTERxURLxDESCRIPTION = 5;
const unsigned short STATE_ENTERxURL = 6;
const unsigned short STATE_ENTERxAUTOxRESPONSE = 7;

#define NEXT_WORD(s) while (*s != '\0' && *s == ' ') s++;

struct Command
{
  char *name;
  int (CRMSClient::*fcn)();
  char *help;
};

static const unsigned short NUM_COMMANDS = 15;
static struct Command commands[NUM_COMMANDS] =
{
  { "ADDUSER", &CRMSClient::Process_ADDUSER,
    "Add user to contact list { <uin> }." },
  { "AR", &CRMSClient::Process_AR,
    "Set your (or a user custom) auto response { [ <uin> ] }." },
  { "GROUPS", &CRMSClient::Process_GROUPS,
    "Show list of groups." },
  { "HELP", &CRMSClient::Process_HELP,
    "Print out help on commands." },
  { "INFO", &CRMSClient::Process_INFO,
    "Print out user information.  Argument is the uin, or none for personal." },
  { "LIST", &CRMSClient::Process_LIST,
    "List users { [ <group #> ] [ <online|offline|all> ] [ <format> ] }." },
  { "LOG", &CRMSClient::Process_LOG,
    "Dump log messages { <log types> }." },
  { "MESSAGE", &CRMSClient::Process_MESSAGE,
    "Send a message { <uin> }." },
  { "QUIT", &CRMSClient::Process_QUIT,
    "Close the connection.  With an argument of 1 causes the plugin to unload." },
  { "REMUSER", &CRMSClient::Process_REMUSER,
    "Remove user from contact list { <uin> }." },
  { "SECURE", &CRMSClient::Process_SECURE,
    "Open/close/check secure channel { <uin> [ <open|close> ] } ." },
  { "STATUS", &CRMSClient::Process_STATUS,
    "Set or show status.  Argument is new status, or blank to display current." },
  { "TERM", &CRMSClient::Process_TERM,
    "Terminate the licq daemon." },
  { "VIEW", &CRMSClient::Process_VIEW,
    "View event (next or specific user) { [ <uin> ] }." },
  { "URL", &CRMSClient::Process_URL,
    "Send a url { <uin> }." }
};


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
  m_nPipe = _licqDaemon->RegisterPlugin(SIGNAL_UPDATExUSER);
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

  server = new TCPSocket(0);

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
      printf("Could not start server on port %u, "
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
                delete *iter;
                clients.erase(iter);
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
  case SIGNAL_UPDATExLIST:
  case SIGNAL_LOGON:
  default:
    break;
  }
  delete s;
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
CRMSClient::CRMSClient(TCPSocket *sin) : sock(0)
{
  sin->RecvConnection(sock);
  sockman.AddSocket(&sock);
  sockman.DropSocket(&sock);

  gLog.Info("%sClient connected from %s.\n", L_RMSxSTR, sock.RemoteIpStr(buf));

  fs = fdopen(sock.Descriptor(), "r+");
  fprintf(fs, "Licq Remote Management Server v%s\n"
     "%d Enter your UIN:\n", LP_Version(), CODE_ENTERxUIN);
  fflush(fs);

  m_nState = STATE_UIN;
  m_nLogTypes = 0;
  data_line_pos = 0;
}


/*---------------------------------------------------------------------------
 * CRMSClient::destructor
 *-------------------------------------------------------------------------*/
CRMSClient::~CRMSClient()
{
  sockman.CloseSocket(sock.Descriptor(), false, false);
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
  char *szr = NULL;
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
      m_nCheckUin = strtoul(data_line, (char**)NULL, 10);
      fprintf(fs, "%d Enter your password:\n", CODE_ENTERxPASSWORD);
      fflush(fs);
      m_nState = STATE_PASSWORD;
      break;
    }
    case STATE_PASSWORD:
    {
      ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
      bool ok = (m_nCheckUin == o->Uin() &&
         strcmp(o->Password(), data_line) == 0);
      if (!ok)
      {
        gUserManager.DropOwner();
        gLog.Info("%sClient failed validation from %s.\n", L_RMSxSTR,
           sock.RemoteIpStr(buf));
        fprintf(fs, "%d Invalid UIN/Password.\n", CODE_INVALID);
        fflush(fs);
        return -1;
      }
      gLog.Info("%sClient validated from %s.\n", L_RMSxSTR,
         sock.RemoteIpStr(buf));
      fprintf(fs, "%d Hello %s.  Type HELP for assistance.\n", CODE_HELLO,
         o->GetAlias());
      fflush(fs);
      gUserManager.DropOwner();
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
  unsigned long nUin = strtoul(data_arg, (char**)NULL, 10);

  if (nUin == 0) nUin = gUserManager.OwnerUin();

  // Print the user info
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL)
  {
    fprintf(fs, "%d No such user.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }

  fprintf(fs, "%d %ld Alias: %s\n", CODE_USERxINFO, u->Uin(), u->GetAlias());
  fprintf(fs, "%d %ld Status: %s\n", CODE_USERxINFO, u->Uin(), u->StatusStr());
  fprintf(fs, "%d %ld First Name: %s\n", CODE_USERxINFO, u->Uin(), u->GetFirstName());
  fprintf(fs, "%d %ld Last Name: %s\n", CODE_USERxINFO, u->Uin(), u->GetLastName());
  fprintf(fs, "%d %ld Email 1: %s\n", CODE_USERxINFO, u->Uin(), u->GetEmailPrimary());
  fprintf(fs, "%d %ld Email 2: %s\n", CODE_USERxINFO, u->Uin(), u->GetEmailSecondary());

  gUserManager.DropUser(u);

  return fflush(fs);
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_STATUS
 *
 * Command:
 *   STATUS [ status ]
 *
 * Response:
 *
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_STATUS()
{
  // Show status
  if (data_arg[0] == '\0')
  {
    ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
    fprintf(fs, "%d %s\n", CODE_STATUS, o->StatusStr());
    gUserManager.DropOwner();
    return fflush(fs);
  }

  // Set status
  unsigned long nStatus = StringToStatus(data_arg);
  if (nStatus == INT_MAX)
  {
    fprintf(fs, "%d Invalid status.\n", CODE_INVALIDxSTATUS);
    return fflush(fs);
  }

  if (nStatus == ICQ_STATUS_OFFLINE)
  {
    fprintf(fs, "%d [0] Logging off.\n", CODE_COMMANDxSTART);
    fflush(fs);
    licqDaemon->icqLogoff();
    fprintf(fs, "%d [0] Event done.\n", CODE_RESULTxSUCCESS);
    return fflush(fs);
  }

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  bool b = o->StatusOffline();
  gUserManager.DropOwner();
  unsigned long tag = 0;
  if (b)
  {
    tag = licqDaemon->icqLogon(nStatus);
    fprintf(fs, "%d [%ld] Logging on.\n", CODE_COMMANDxSTART, tag);
  }
  else
  {
    tag = licqDaemon->icqSetStatus(nStatus);
    fprintf(fs, "%d [%ld] Setting status.\n", CODE_COMMANDxSTART, tag);
  }
  tags.push_back(tag);

  return fflush(fs);
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_QUIT
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_QUIT()
{
  fprintf(fs, "%d Aurevoir.\n", CODE_QUIT);
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
  GroupList *g = gUserManager.LockGroupList(LOCK_R);
  for (unsigned short i = 0; i < g->size(); i++)
  {
    fprintf(fs, "%d %03d %s\n", CODE_LISTxGROUP, i + 1, (*g)[i]);
  }
  gUserManager.UnlockGroupList();
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
 *     documented in UTILITIES.HOWTO.  The default is "%9u %-20a %3m %s"
 *     and prints out users as follows.
 *
 * Response:
 *   CODE_LISTxUSER   5550000              AnAlias   2 Online
 *     The default line contains the uin, alias, number of new messages
 *     and status all column and white space deliminated.  Note that the
 *     alias may contain white space.
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
    strcpy(format, "%9u %-20a %3m %s");
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
 *     MESSAGE <uin>
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
  unsigned long nUin = strtoul(data_arg, (char**)NULL, 10);

  if (nUin < 10000)
  {
    fprintf(fs, "%d Invalid UIN.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }

  fprintf(fs, "%d Enter message, terminate with a . on a line by itself:\n",
     CODE_ENTERxTEXT);

  m_nUin = nUin;
  m_szText[0] = '\0';
  m_nTextPos = 0;

  m_nState = STATE_ENTERxMESSAGE;
  return fflush(fs);
}

int CRMSClient::Process_MESSAGE_text()
{
  unsigned long tag = licqDaemon->icqSendMessage(m_nUin, m_szText, false, ICQ_TCPxMSG_NORMAL);

  fprintf(fs, "%d [%ld] Sending message to %ld.\n", CODE_COMMANDxSTART,
     tag, m_nUin);

  tags.push_back(tag);
  m_nState = STATE_COMMAND;

  return fflush(fs);
}



/*---------------------------------------------------------------------------
 * CRMSClient::Process_URL
 *
 * Command:
 *   URL <uin>
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
  unsigned long nUin = strtoul(data_arg, (char**)NULL, 10);

  if (nUin < 10000)
  {
    fprintf(fs, "%d Invalid UIN.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }
  fprintf(fs, "%d Enter URL:\n", CODE_ENTERxLINE);

  m_nUin = nUin;
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
  unsigned long tag = licqDaemon->icqSendUrl(m_nUin, m_szLine, m_szText, false, ICQ_TCPxMSG_NORMAL);

  fprintf(fs, "%d [%ld] Sending URL to %ld.\n", CODE_COMMANDxSTART,
     tag, m_nUin);

  tags.push_back(tag);
  m_nState = STATE_COMMAND;

  return fflush(fs);
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_AR
 *
 * Command:
 *     AR [ <uin> ]
 *
 * Response:
 *   CODE_ENTERxTEXT | CODE_INVALIDxUIN
 *     At which point the auto response should be entered line by line and
 *     terminated by entering a "." on a line by itself.
 *   CODE_RESULTxSUCCESS
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_AR()
{
/*
  if (data_arg[0] == '\0')
  {
    ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
    // print...
    gUserManager.DropOwner();
    return fflush(fs);
  }
*/
  unsigned long nUin = strtoul(data_arg, (char**)NULL, 10);

  if (nUin != 0 && !gUserManager.IsOnList(nUin))
  {
    fprintf(fs, "%d Invalid UIN.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }

  fprintf(fs, "%d Enter %sauto response, terminate with a . on a line by itself:\n",
     CODE_ENTERxTEXT, nUin == 0 ? "" : "custom " );

  m_nUin = nUin;
  m_szText[0] = '\0';
  m_nTextPos = 0;

  m_nState = STATE_ENTERxAUTOxRESPONSE;
  return fflush(fs);
}

int CRMSClient::Process_AR_text()
{
  if (m_nUin == 0)
  {
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    o->SetAutoResponse(m_szText);
    gUserManager.DropOwner();
  }
  else
  {
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
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
 * CRMSClient::Process_VIEW
 *
 * Command:
 *   VIEW <uin>
 *
 * Response:
 *
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_VIEW()
{
  unsigned long nUin = 0;

  if (*data_arg != '\0')
  {
    nUin = strtoul(data_arg, (char**)NULL, 10);
  }
  else
  {
    // XXX Check system messages first

    // Check user messages now
    FOR_EACH_USER_START(LOCK_R)
    {
      if(pUser->NewMessages() > 0)
        nUin = pUser->Uin();
    }
    FOR_EACH_USER_END
  
    if (nUin == 0)
    {
      fprintf(fs, "%d No new messages.\n", CODE_VIEWxNONE);
      return fflush(fs);
    }
  }

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
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
    fprintf(fs, e->Text());
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
 *   ADDUSER <uin>
 *
 * Response:
 *
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_ADDUSER()
{
  unsigned long nUin = strtoul(data_arg, (char**)NULL, 10);

  if (nUin >= 10000)
  {
    if (licqDaemon->AddUserToList(nUin))
    {
      fprintf(fs, "%d User added\n", CODE_ADDUSERxDONE);
    }
    else
    {
      fprintf(fs, "%d User not added\n", CODE_ADDUSERxERROR);
    }
  }
  else
  {
    fprintf(fs, "%d Invalid UIN.\n", CODE_INVALIDxUSER);
  }

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
    licqDaemon->RemoveUserFromList(nUin);
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
  

  if (isdigit(*data_arg))
  {
    nUin = strtoul(data_arg, (char**)NULL, 10);
    while (*data_arg != '\0' && *data_arg != ' ') data_arg++;
    NEXT_WORD(data_arg);
  }
   else
  {
    fprintf(fs, "%d Invalid UIN.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }

  if (nUin < 10000)
  {
    fprintf(fs, "%d Invalid UIN.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }

  if (strncasecmp(data_arg, "open", 4) == 0)
  {
    fprintf(fs, "%d Opening secure connection.\n", CODE_SECURExOPEN);
    licqDaemon->icqOpenSecureChannel(nUin);
  }
  else
  if (strncasecmp(data_arg, "close", 5) == 0)
  {
    fprintf(fs, "%d Closing secure connection.\n", CODE_SECURExCLOSE);
    licqDaemon->icqCloseSecureChannel(nUin);
  }
  else
  {
   ICQUser *u = gUserManager.FetchUser(nUin,LOCK_R);
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
  
  return fflush(fs);
}
