#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#include "autoreply.h"
#include "licq_log.h"
#include "licq_icqd.h"
#include "licq_file.h"
#include "licq_user.h"
#include "licq_constants.h"

extern "C" { const char *LP_Version(); }

const char L_AUTOREPxSTR[]  = "[RPL] ";
const unsigned short SUBJ_CHARS = 20;

/*---------------------------------------------------------------------------
 * CLicqAutoReply::Constructor
 *-------------------------------------------------------------------------*/
CLicqAutoReply::CLicqAutoReply(bool _bEnable, bool _bDelete, char *_szStatus)
{
  tcp = new TCPSocket;
  m_bExit = false;
  m_bEnabled = _bEnable;
  m_bDelete = _bDelete;
  m_szStatus = _szStatus == NULL ? NULL : strdup(_szStatus);
}


/*---------------------------------------------------------------------------
 * CLicqAutoReply::Destructor
 *-------------------------------------------------------------------------*/
CLicqAutoReply::~CLicqAutoReply()
{
  delete tcp;
}

/*---------------------------------------------------------------------------
 * CLicqAutoReply::Shutdown
 *-------------------------------------------------------------------------*/
void CLicqAutoReply::Shutdown()
{
  gLog.Info("%sShutting down auto reply.\n", L_AUTOREPxSTR);
  licqDaemon->UnregisterPlugin();
}


/*---------------------------------------------------------------------------
 * CLicqAutoReply::Run
 *-------------------------------------------------------------------------*/
int CLicqAutoReply::Run(CICQDaemon *_licqDaemon)
{
  // Register with the daemon, we only want the update user signal
  m_nPipe = _licqDaemon->RegisterPlugin(SIGNAL_UPDATExUSER);
  licqDaemon = _licqDaemon;

  // Create our snmp information
  // m_nSMTPPort = 25; //getservicebyname("snmp");
  char filename[256];
  sprintf (filename, "%s/licq_autoreply.conf", BASE_DIR);
  CIniFile conf(INI_FxFATAL | INI_FxERROR);
  conf.LoadFile(filename);
  conf.SetSection("Reply");
  conf.ReadStr("Program", m_szProgram);
  conf.CloseFile();

  // Log on if necessary
  if (m_szStatus != NULL)
  {
    unsigned long s = licqDaemon->StringToStatus(m_szStatus);
    ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
    bool b = o->StatusOffline();
    gUserManager.DropOwner();
    if (s == INT_MAX)
      gLog.Warn("%sInvalid startup status.\n", L_AUTOREPxSTR);
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
 * CLicqAutoReply::ProcessPipe
 *-------------------------------------------------------------------------*/
void CLicqAutoReply::ProcessPipe()
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
    gLog.Info("%sExiting.\n", L_AUTOREPxSTR);
    m_bExit = true;
    break;
  }

  case '0':  // disable
  {
    gLog.Info("%sDisabling.\n", L_AUTOREPxSTR);
    m_bEnabled = false;
    break;
  }

  case '1':  // enable
  {
    gLog.Info("%sEnabling.\n", L_AUTOREPxSTR);
    m_bEnabled = true;
    break;
  }

  default:
    gLog.Warn("%sUnknown notification type from daemon: %c.\n", L_WARNxSTR, buf[0]);
  }
}


/*---------------------------------------------------------------------------
 * CLicqAutoReply::ProcessSignal
 *-------------------------------------------------------------------------*/
void CLicqAutoReply::ProcessSignal(CICQSignal *s)
{
  switch (s->Signal())
  {
  case SIGNAL_UPDATExUSER:
    if (s->SubSignal() == USER_EVENTS && s->Uin() != gUserManager.OwnerUin())
      ProcessUserEvent(s->Uin(), s->Argument());
    break;
  // We should never get any other signal
  case SIGNAL_UPDATExLIST:
    break;
  case SIGNAL_LOGON:
    break;
  default:
    gLog.Warn("%sInternal error: CLicqiAutoReply::ProcessSignal(): Unknown signal command received from daemon: %d.\n", 
              L_WARNxSTR, s->Signal());
    break;
  }
  delete s;
}


/*---------------------------------------------------------------------------
 * CLicqAutoReply::ProcessEvent
 *-------------------------------------------------------------------------*/
void CLicqAutoReply::ProcessEvent(ICQEvent *e)
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
    gLog.Warn("%sInternal error: CLicqAutoReply::ProcessEvent(): Unknown event command received from daemon: %d.\n",
              L_WARNxSTR, e->m_nCommand);
    break;
  }*/
  delete e;
}


void CLicqAutoReply::ProcessUserEvent(unsigned long nUin, unsigned long nId)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u == NULL)
  {
    gLog.Warn("%sInvalid uin received from daemon (%ld).\n", L_AUTOREPxSTR, nUin);
    return;
  }

  CUserEvent *e = u->EventPeekId(nId);

  if (e == NULL)
  {
    gLog.Warn("%sInvalid message id (%d).\n", L_AUTOREPxSTR, nId);
  }
  else
  {
    bool r = AutoReplyEvent(u, e);
    if (m_bDelete && r)
      u->EventClearId(nId);
  }

  gUserManager.DropUser(u);
}


bool CLicqAutoReply::AutoReplyEvent(ICQUser *u, CUserEvent *e)
{
  FILE *output;
  char m_szMessage[4096];
  char c;
  int pos = 0;

  if (e == NULL) return false;

  for (int i = 0; i < 4096; i++)
  {
    m_szMessage[i] = '\0';
  }
  output = popen (m_szProgram, "r");
  if (!output)
  {
    gLog.Warn("%sCould not execute %s\n", L_AUTOREPxSTR, m_szProgram);
    return false;
  }
  while (((c = fgetc(output)) != EOF) && (pos < 4096))
  {
    m_szMessage[pos++] = c;
  }
  pclose (output);
  char *szText = new char[4096 + 256];
  sprintf(szText, "%s", m_szMessage);
  CICQEventTag *tag = licqDaemon->icqSendMessage(u->Uin(), szText, false,
     ICQ_TCPxMSG_NORMAL);
  delete []szText;
  if (tag == NULL)
  {
    gLog.Warn("%sSending message to %s (%ld) failed.\n", L_AUTOREPxSTR,
u->GetAlias(), u->Uin());
    return false;
  }
  gLog.Info("%sSent autoreply to %s (%ld).\n", L_AUTOREPxSTR, u->GetAlias(),
u->Uin());
  delete tag;
  return true;
}

