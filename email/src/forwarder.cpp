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

#include "forwarder.h"
#include "log.h"
#include "icqd.h"
#include "file.h"
#include "user.h"
#include "eventdesc.h"
#include "constants.h"

extern "C" { const char *LP_Version(); }

const char L_FORWARDxSTR[]  = "[FOR] ";
const unsigned short SUBJ_CHARS = 20;

/*---------------------------------------------------------------------------
 * CLicqForwarder::Constructor
 *-------------------------------------------------------------------------*/
CLicqForwarder::CLicqForwarder(bool _bEnable, bool _bDelete, char *_szStatus)
{
  tcp = new TCPSocket;
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
  licqDaemon->UnregisterPlugin();
}


/*---------------------------------------------------------------------------
 * CLicqForwarder::Run
 *-------------------------------------------------------------------------*/
int CLicqForwarder::Run(CICQDaemon *_licqDaemon)
{
  // Register with the daemon, we only want the update user signal
  m_nPipe = _licqDaemon->RegisterPlugin(SIGNAL_UPDATExUSER);
  licqDaemon = _licqDaemon;

  // Create our snmp information
  m_nSMTPPort = 25; //getservicebyname("snmp");
  char filename[256];
  sprintf (filename, "%s/licq_forwarder.conf", BASE_DIR);
  CIniFile conf(INI_FxFATAL | INI_FxERROR);
  conf.LoadFile(filename);
  conf.SetSection("Forward");
  conf.ReadNum("Type", m_nForwardType, FORWARD_EMAIL);

  switch (m_nForwardType)
  {
    case FORWARD_EMAIL:
      conf.SetSection("SMTP");
      conf.ReadStr("Host", m_szSMTPHost);
      conf.ReadStr("To", m_szSMTPTo);
      conf.ReadStr("From", m_szSMTPFrom);
      conf.ReadStr("Domain", m_szSMTPDomain);
      break;
    case FORWARD_ICQ:
      conf.SetSection("ICQ");
      conf.ReadNum("Uin", m_nUINTo, 0);
      if (m_nUINTo == 0)
      {
        gLog.Error("%sInvalid ICQ forward UIN: %ld\n", L_FORWARDxSTR, m_nUINTo);
        return 1;
      }
      break;
    default:
      gLog.Error("%sInvalid forward type: %d\n", L_FORWARDxSTR, m_nForwardType);
      return 1;
      break;
  }
  conf.CloseFile();

  // Log on if necessary
  if (m_szStatus != NULL)
  {
    unsigned long s = licqDaemon->StringToStatus(m_szStatus);
    ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
    bool b = o->StatusOffline();
    gUserManager.DropOwner();
    if (s == INT_MAX)
      gLog.Warn("%sInvalid startup status.\n", L_FORWARDxSTR);
    else
    {
      if (b)
        licqDaemon->icqLogon(s);
      else
        licqDaemon->icqSetStatus(s);
    }
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
 * CLicqForwarder::ProcessPipe
 *-------------------------------------------------------------------------*/
void CLicqForwarder::ProcessPipe()
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
    gLog.Info("%sExiting.\n", L_FORWARDxSTR);
    m_bExit = true;
    break;
  }

  case '0':  // disable
  {
    gLog.Info("%sDisabling.\n", L_FORWARDxSTR);
    m_bEnabled = false;
    break;
  }

  case '1':  // enable
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
void CLicqForwarder::ProcessSignal(CICQSignal *s)
{
  switch (s->Signal())
  {
  case SIGNAL_UPDATExUSER:
    if (s->SubSignal() == USER_EVENTS)
      ProcessUserEvent(s->Uin());
    break;
  // We should never get any other signal
  case SIGNAL_UPDATExLIST:
    break;
  case SIGNAL_LOGON:
    break;
  default:
    gLog.Warn("%sInternal error: CLicqForwarder::ProcessSignal(): Unknown signal command received from daemon: %d.\n", 
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


void CLicqForwarder::ProcessUserEvent(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u == NULL)
  {
    gLog.Warn("Invalid uin received from daemon (%ld).\n", nUin);
    return;
  }

  CUserEvent *e = NULL;

  if (m_bDelete)
  {
    while (u->NewMessages() > 0)
    {
      // Fetch the event
      e = u->EventPop();
      // Forward it
      if (!ForwardEvent(u, e))
      {
        delete e;
        break;
      }
      // Erase the event
      delete e;
    }
  }
  else
  {
    e = u->EventPeekLast();
    // Forward it
    ForwardEvent(u, e);
  }

  gUserManager.DropUser(u);
}


bool CLicqForwarder::ForwardEvent(ICQUser *u, CUserEvent *e)
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


bool CLicqForwarder::ForwardEvent_ICQ(ICQUser *u, CUserEvent *e)
{
  char *szText = new char[strlen(e->Text()) + 256];
  char szTime[64];
  time_t t = e->Time();
  strftime(szTime, 64, "%a %b %d, %R", localtime(&t));
  sprintf(szText, "[ %s from %s (ICQ#%ld) sent %s ]\n\n%s\n", EventDescription(e),
          u->GetAlias(), u->Uin(), szTime, e->Text());
  CICQEventTag *tag = licqDaemon->icqSendMessage(m_nUINTo, szText, false, false);
  delete []szText;
  if (tag == NULL)
  {
    gLog.Warn("%sSending message to %ld failed.\n", L_FORWARDxSTR, m_nUINTo);
    return false;
  }
  gLog.Info("%sForwarded message from %s (%ld) to %ld.\n", L_FORWARDxSTR, u->GetAlias(), u->Uin(), m_nUINTo);
  delete tag;
  return true;
}


bool CLicqForwarder::ForwardEvent_Email(ICQUser *u, CUserEvent *e)
{
  char szTo[256],
       szFrom[256],
       szSubject[256],
       szDate[256],
       szReplyTo[256];
  time_t t = e->Time();

  // Fill in the strings
  if (!u->User())
  {
    sprintf(szTo, "To: %s <%s>", u->GetAlias(), m_szSMTPTo);
    sprintf (szFrom, "From: ICQ System Message <support@mirabilis.com>");
    sprintf (szReplyTo, "Reply-To: Mirabilis <support@mirabilis.com>");
  }
  else
  {
    ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
    sprintf(szTo, "To: %s <%s>", o->GetAlias(), m_szSMTPTo);
    gUserManager.DropOwner();
    sprintf (szFrom, "From: \"%s\" <%ld@pager.mirabilis.com>", u->GetAlias(), u->Uin());
    sprintf (szReplyTo, "Reply-To: \"%s %s\" <%s>", u->GetFirstName(), u->GetLastName(), u->GetEmail1());
  }
  sprintf (szDate, "Date: %s", ctime(&t));

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
    sprintf (szSubject, "Subject: %s [%s%s]", EventDescription(e),
             s, strlen(e->Text()) > SUBJ_CHARS ? "..." : "");
    delete []s;
    break;
  }
  case ICQ_CMDxSUB_URL:
    sprintf (szSubject, "Subject: %s [%s]", EventDescription(e),
             ((CEventUrl *)e)->Url());
    break;
  case ICQ_CMDxSUB_FILE:
    sprintf (szSubject, "Subject: %s [%s]", EventDescription(e),
             ((CEventFile *)e)->Filename());
    break;
  default:
    sprintf (szSubject, "Subject: %s", EventDescription(e));
  }


  // Connect to the SMTP server
  if (!tcp->DestinationSet() && !tcp->SetRemoteAddr(m_szSMTPHost, m_nSMTPPort))
  {
    char buf[128];
    gLog.Warn("%sUnable to determine SMTP host ip:\n%s%s.\n", L_WARNxSTR, L_BLANKxSTR,
              tcp->ErrorStr(buf, 128));
    return false;
  }
  if (!tcp->OpenConnection())
  {
    char buf[128];
    gLog.Warn("%sUnable to connect to %s:%d:\n%s%s.\n", L_ERRORxSTR,
              tcp->RemoteIpStr(buf), tcp->RemotePort(), L_BLANKxSTR,
              tcp->ErrorStr(buf, 128));
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

  fprintf(fs, "HELO %s\n", m_szSMTPDomain);
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 250)
  {
    gLog.Warn("%sInvalid response to HELO:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR, fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf (fs, "MAIL From: %s\n", m_szSMTPFrom);
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 250)
  {
    gLog.Warn("%sInvalid response to MAIL:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR, fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "RCPT TO: %s\n", m_szSMTPTo);
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 250)
  {
    gLog.Warn("%sInvalid response to RCPT TO:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR, fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "DATA\n");
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 354)
  {
    gLog.Warn("%sInvalid response to DATA:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR, fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "%s"
              "%s\n"
              "%s\n"
              "%s\n"
              "%s\n"
              "\n"
              "%s\n.\n",
              szDate, szFrom, szTo, szReplyTo, szSubject, e->Text());

  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 250)
  {
    gLog.Warn("%sInvalid response to DATA done:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR, fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "quit\n");

  // Close our connection
  tcp->CloseConnection();

  gLog.Info("%sForwarded message from %s (%ld) to %s.\n", L_FORWARDxSTR, u->GetAlias(), u->Uin(), m_szSMTPTo);
  return true;
}




