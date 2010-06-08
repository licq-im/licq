#include <cctype>
#include <climits>
#include <cstdio>
#include <cstring>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

#include "forwarder.h"
#include "forwarder.conf.h"
#include <licq_events.h>
#include "licq_log.h"
#include "licq_constants.h"
#include <licq/contactlist/usermanager.h>
#include <licq/icqdefines.h>
#include <licq/daemon.h>
#include <licq/inifile.h>
#include "licq/pluginmanager.h"
#include <licq/protocolmanager.h>
#include <licq/socket.h>
#include <licq/translator.h>

extern "C" { const char *LP_Version(); }

using std::string;
using Licq::gPluginManager;
using Licq::gProtocolManager;
using Licq::gUserManager;
using Licq::UserId;

const char L_FORWARDxSTR[]  = "[FOR] ";
const unsigned short SUBJ_CHARS = 20;

/*---------------------------------------------------------------------------
 * CLicqForwarder::Constructor
 *-------------------------------------------------------------------------*/
CLicqForwarder::CLicqForwarder(bool _bEnable, bool _bDelete, char *_szStatus)
{
  tcp = new Licq::TCPSocket;
  m_bExit = false;
  m_bEnabled = _bEnable;
  m_bDelete = _bDelete;
  m_szStatus = _szStatus == NULL ? NULL : strdup(_szStatus);
}


/*---------------------------------------------------------------------------
 * CLicqForwarder::Destructor
 *-------------------------------------------------------------------------*/
CLicqForwarder::~CLicqForwarder()
{
  delete tcp;
}

/*---------------------------------------------------------------------------
 * CLicqForwarder::Shutdown
 *-------------------------------------------------------------------------*/
void CLicqForwarder::Shutdown()
{
  gLog.Info("%sShutting down forwarder.\n", L_FORWARDxSTR);
  gPluginManager.unregisterGeneralPlugin();
}


/*---------------------------------------------------------------------------
 * CLicqForwarder::Run
 *-------------------------------------------------------------------------*/
int CLicqForwarder::Run()
{
  // Register with the daemon, we only want the update user signal
  m_nPipe = gPluginManager.registerGeneralPlugin(SIGNAL_UPDATExUSER);

  // Create our snmp information
  m_nSMTPPort = 25; //getservicebyname("snmp");
  char filename[256];
  sprintf(filename, "%slicq_forwarder.conf", BASE_DIR);
  Licq::IniFile conf(filename);
  if (!conf.loadFile())
  {
    if(!CreateDefaultConfig())
    {
      gLog.Error("%sCould not create default configuration file: %s\n", L_FORWARDxSTR, filename);
      return 1;
    }
    gLog.Info("%sA default configuration file has been created: %s\n", L_FORWARDxSTR, filename);
    conf.loadFile();
  }
  conf.setSection("Forward");
  conf.get("Type", m_nForwardType, FORWARD_EMAIL);

  switch (m_nForwardType)
  {
    case FORWARD_EMAIL:
      conf.setSection("SMTP");
      conf.get("Host", mySmtpHost);
      conf.get("To", mySmtpTo);
      conf.get("From", mySmtpFrom);
      conf.get("Domain", mySmtpDomain);
      break;
    case FORWARD_ICQ:
    {
      conf.setSection("ICQ");
      string accountId;
      conf.get("Uin", accountId, "");
      if (accountId.empty())
      {
        gLog.Error("%sInvalid ICQ forward UIN: %s\n", L_FORWARDxSTR, accountId.c_str());
        return 1;
      }
      myUserId = UserId(accountId, LICQ_PPID);
      break;
    }
    default:
      gLog.Error("%sInvalid forward type: %d\n", L_FORWARDxSTR, m_nForwardType);
      return 1;
      break;
  }

  // Log on if necessary
  if (m_szStatus != NULL)
  {
    unsigned s;
    if (!Licq::User::stringToStatus(m_szStatus, s))
      gLog.Warn("%sInvalid startup status.\n", L_FORWARDxSTR);
    else
      gProtocolManager.setStatus(gUserManager.ownerUserId(LICQ_PPID), s);
    free(m_szStatus);
    m_szStatus = NULL;
  }

  fd_set fdSet;
  int nResult;

  while (!m_bExit)
  {
    FD_ZERO(&fdSet);
    FD_SET(m_nPipe, &fdSet);

    nResult = select(m_nPipe + 1, &fdSet, NULL, NULL, NULL);
    if (nResult == -1)
    {
      gLog.Error("%sError in select(): %s\n", L_ERRORxSTR, strerror(errno));
      m_bExit = true;
    }
    else
    {
      if (FD_ISSET(m_nPipe, &fdSet))
        ProcessPipe();
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------
 * CLicqForwarder::CreateDefaultConfig
 *-------------------------------------------------------------------------*/
bool CLicqForwarder::CreateDefaultConfig()
{
  // Create licq_forwarder.conf
  string filename = BASE_DIR;
  filename += "licq_forwarder.conf";
  FILE *f = fopen(filename.c_str(), "w");
  if (f == NULL)
    return false;
  fprintf(f, "%s", FORWARDER_CONF);
  fclose(f);
  return true;
}

/*---------------------------------------------------------------------------
 * CLicqForwarder::ProcessPipe
 *-------------------------------------------------------------------------*/
void CLicqForwarder::ProcessPipe()
{
  char buf[16];
  read(m_nPipe, buf, 1);
  switch (buf[0])
  {
    case Licq::GeneralPlugin::PipeSignal:
    {
      LicqSignal* s = Licq::gDaemon.popPluginSignal();
    if (m_bEnabled) ProcessSignal(s);
    break;
  }

    case Licq::GeneralPlugin::PipeEvent:
    {
      // An event is pending (should never happen)
      LicqEvent* e = Licq::gDaemon.PopPluginEvent();
    if (m_bEnabled) ProcessEvent(e);
    break;
  }

    case Licq::GeneralPlugin::PipeShutdown:
    {
    gLog.Info("%sExiting.\n", L_FORWARDxSTR);
    m_bExit = true;
    break;
  }

    case Licq::GeneralPlugin::PipeDisable:
    {
    gLog.Info("%sDisabling.\n", L_FORWARDxSTR);
    m_bEnabled = false;
    break;
  }

    case Licq::GeneralPlugin::PipeEnable:
    {
    gLog.Info("%sEnabling.\n", L_FORWARDxSTR);
    m_bEnabled = true;
    break;
  }

  default:
    gLog.Warn("%sUnknown notification type from daemon: %c.\n", L_WARNxSTR, buf[0]);
  }
}


/*---------------------------------------------------------------------------
 * CLicqForwarder::ProcessSignal
 *-------------------------------------------------------------------------*/
void CLicqForwarder::ProcessSignal(LicqSignal* s)
{
  switch (s->Signal())
  {
  case SIGNAL_UPDATExUSER:
    if (s->SubSignal() == USER_EVENTS && s->Argument() > 0)
        ProcessUserEvent(s->userId(), s->Argument());
    break;
  // We should never get any other signal
  case SIGNAL_UPDATExLIST:
    break;
  case SIGNAL_LOGON:
    break;
  default:
    gLog.Warn("%sInternal error: CLicqForwarder::ProcessSignal(): Unknown signal command received from daemon: %ld.\n",
              L_WARNxSTR, s->Signal());
    break;
  }
  delete s;
}


/*---------------------------------------------------------------------------
 * CLicqForwarder::ProcessEvent
 *-------------------------------------------------------------------------*/
void CLicqForwarder::ProcessEvent(ICQEvent *e)
{
/*  switch (e->m_nCommand)
  {
  case ICQ_CMDxTCP_START:
  case ICQ_CMDxSND_THRUxSERVER:
  case ICQ_CMDxSND_USERxGETINFO:
  case ICQ_CMDxSND_USERxGETDETAILS:
  case ICQ_CMDxSND_UPDATExDETAIL:
  case ICQ_CMDxSND_UPDATExBASIC:
  case ICQ_CMDxSND_LOGON:
  case ICQ_CMDxSND_REGISTERxUSER:
  case ICQ_CMDxSND_SETxSTATUS:
  case ICQ_CMDxSND_AUTHORIZE:
  case ICQ_CMDxSND_USERxLIST:
  case ICQ_CMDxSND_VISIBLExLIST:
  case ICQ_CMDxSND_INVISIBLExLIST:
  case ICQ_CMDxSND_PING:
  case ICQ_CMDxSND_USERxADD:
  case ICQ_CMDxSND_SYSxMSGxREQ:
  case ICQ_CMDxSND_SYSxMSGxDONExACK:
  case ICQ_CMDxSND_SEARCHxINFO:
  case ICQ_CMDxSND_SEARCHxUIN:
    break;

  default:
    gLog.Warn("%sInternal error: CLicqForwarder::ProcessEvent(): Unknown event command received from daemon: %d.\n",
              L_WARNxSTR, e->m_nCommand);
    break;
  }*/
  delete e;
}


void CLicqForwarder::ProcessUserEvent(const UserId& userId, unsigned long nId)
{
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
  {
    gLog.Warn("%sInvalid user received from daemon (%s).\n", L_FORWARDxSTR, userId.toString().c_str());
    return;
  }

  const CUserEvent* e = u->EventPeekId(nId);

  if (e == NULL)
  {
    gLog.Warn("%sInvalid message id (%ld).\n", L_FORWARDxSTR, nId);
  }
  else
  {
    bool r = ForwardEvent(*u, e);
    if (m_bDelete && r)
      u->EventClearId(nId);
  }
}


bool CLicqForwarder::ForwardEvent(const Licq::User* u, const CUserEvent* e)
{
  if (e == NULL) return false;

  bool s = false;
  switch (m_nForwardType)
  {
    case FORWARD_EMAIL:
      s = ForwardEvent_Email(u, e);
      break;
    case FORWARD_ICQ:
      s = ForwardEvent_ICQ(u, e);
      break;
  }
  return s;
}


bool CLicqForwarder::ForwardEvent_ICQ(const Licq::User* u, const CUserEvent* e)
{
  char *szText = new char[strlen(e->Text()) + 256];
  char szTime[64];
  time_t t = e->Time();
  strftime(szTime, 64, "%a %b %d, %R", localtime(&t));
  sprintf(szText, "[ %s from %s (%s) sent %s ]\n\n%s\n", e->Description(),
      u->getAlias().c_str(), u->accountId().c_str(), szTime, e->Text());
  unsigned long tag = gProtocolManager.sendMessage(myUserId, szText, true, ICQ_TCPxMSG_NORMAL);
  delete []szText;
  if (tag == 0)
  {
    gLog.Warn("%sSending message to %s failed.\n", L_FORWARDxSTR, myUserId.toString().c_str());
    return false;
  }
  gLog.Info("%sForwarded message from %s (%s) to %s.\n", L_FORWARDxSTR,
      u->getAlias().c_str(), u->accountId().c_str(), myUserId.toString().c_str());
  return true;
}


bool CLicqForwarder::ForwardEvent_Email(const Licq::User* u, const CUserEvent* e)
{
  char szTo[256],
       szFrom[256],
       szSubject[256],
       szDate[256],
       szReplyTo[256];
  time_t t = e->Time();

  // Fill in the strings
  if (!u->isUser())
  {
    sprintf(szTo, "To: %s <%s>", u->getAlias().c_str(), mySmtpTo.c_str());
    sprintf (szFrom, "From: ICQ System Message <support@icq.com>");
    sprintf (szReplyTo, "Reply-To: Mirabilis <support@icq.com>");
  }
  else
  {
    unsigned long protocolId = u->protocolId();
    {
      Licq::OwnerReadGuard o(protocolId);
      sprintf(szTo, "To: %s <%s>", o->getAlias().c_str(), mySmtpTo.c_str());
    }
    if (protocolId == LICQ_PPID)
      sprintf (szFrom, "From: \"%s\" <%s@pager.icq.com>", u->getAlias().c_str(), u->accountId().c_str());
    else
      sprintf (szFrom, "From: \"%s\" <%s>", u->getAlias().c_str(), u->getEmail().c_str());
    sprintf (szReplyTo, "Reply-To: \"%s\" <%s>", u->getFullName().c_str(), u->getEmail().c_str());
  }
  sprintf (szDate, "Date: %s", ctime(&t));
  int l = strlen(szDate);
  szDate[l - 1] = '\r';
  szDate[l] = '\n';
  szDate[l + 1] = '\0';

  switch (e->SubCommand())
  {
  case ICQ_CMDxSUB_MSG:
  case ICQ_CMDxSUB_CHAT:
  {
    char *s = new char[SUBJ_CHARS + 1];
    strncpy(s, e->Text(), SUBJ_CHARS);
    s[40] = '\0';
    char *n = strchr(s, '\n');
    if (n != NULL) *n = '\0';
    sprintf (szSubject, "Subject: %s [%s%s]", e->Description(),
             s, strlen(e->Text()) > SUBJ_CHARS ? "..." : "");
    delete []s;
    break;
  }
  case ICQ_CMDxSUB_URL:
    sprintf (szSubject, "Subject: %s [%s]", e->Description(),
             ((CEventUrl *)e)->Url());
    break;
  case ICQ_CMDxSUB_FILE:
    sprintf (szSubject, "Subject: %s [%s]", e->Description(),
             ((CEventFile *)e)->Filename());
    break;
  default:
    sprintf (szSubject, "Subject: %s", e->Description());
  }


  // Connect to the SMTP server
  if (!tcp->DestinationSet() && !tcp->connectTo(mySmtpHost.c_str(), m_nSMTPPort))
  {
    gLog.Warn("%sUnable to connect to %s:%d:\n%s%s.\n", L_ERRORxSTR,
        tcp->getRemoteIpString().c_str(), tcp->getRemotePort(), L_BLANKxSTR,
        tcp->errorStr().c_str());
    return false;
  }
  char fin[256];
  int code;
  FILE *fs = fdopen(tcp->Descriptor(), "r+");
  if (fs == NULL)
  {
    gLog.Warn("%sUnable to open socket descriptor in file stream mode:\n%s%s.\n",
              L_ERRORxSTR, L_BLANKxSTR, strerror(errno));
    return false;
  }

  // Talk to the server
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 220)
  {
    gLog.Warn("%sInvalid SMTP welcome:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR, fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "HELO %s\r\n", mySmtpDomain.c_str());
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 250)
  {
    gLog.Warn("%sInvalid response to HELO:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR, fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "MAIL From: %s\r\n", mySmtpFrom.c_str());
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 250)
  {
    gLog.Warn("%sInvalid response to MAIL:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR, fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "RCPT TO: %s\r\n", mySmtpTo.c_str());
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 250)
  {
    gLog.Warn("%sInvalid response to RCPT TO:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR, fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "DATA\r\n");
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 354)
  {
    gLog.Warn("%sInvalid response to DATA:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR, fin);
    tcp->CloseConnection();
    return false;
  }

  char *szTextRN = Licq::gTranslator.NToRN(e->Text());
  fprintf(fs, "%s"
              "%s\r\n"
              "%s\r\n"
              "%s\r\n"
              "%s\r\n"
              "\r\n"
              "%s\r\n.\r\n",
              szDate, szFrom, szTo, szReplyTo, szSubject, szTextRN);
  delete [] szTextRN;

  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 250)
  {
    gLog.Warn("%sInvalid response to DATA done:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR, fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "quit\r\n");

  // Close our connection
  tcp->CloseConnection();

  gLog.Info("%sForwarded message from %s (%s) to %s.\n", L_FORWARDxSTR,
      u->getAlias().c_str(), u->accountId().c_str(), mySmtpTo.c_str());
  return true;
}




