#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif
#ifdef HAVE_PATHS_H
#include <paths.h>
#else
#define _PATH_BSHELL "/bin/sh"
#endif


#include "autoreply.h"
#include "licq_log.h"
#include "licq_icqd.h"
#include "licq_file.h"
#include "licq_user.h"
#include "licq_socket.h"
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
  conf.ReadStr("Arguments", m_szArguments, "");
  conf.ReadBool("PassMessage", m_bPassMessage, false);
  conf.CloseFile();

  // Log on if necessary
  if (m_szStatus != NULL)
  {
    unsigned long s = StringToStatus(m_szStatus);
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
      if (s->SubSignal() == USER_EVENTS && s->Uin() != gUserManager.OwnerUin() && s->Argument() > 0)
        ProcessUserEvent(s->Uin(), s->Argument());
      break;
    // We should never get any other signal
    default:
      break;
  }
  delete s;
}


/*---------------------------------------------------------------------------
 * CLicqAutoReply::ProcessEvent
 *-------------------------------------------------------------------------*/
void CLicqAutoReply::ProcessEvent(ICQEvent *e)
{
  CUserEvent *user_event;

  if (e->Result() != EVENT_ACKED)
  {
    if (e->Command() == ICQ_CMDxTCP_START &&
        (e->SubCommand() != ICQ_CMDxSUB_CHAT &&
         e->SubCommand() != ICQ_CMDxSUB_FILE))
    {
	    user_event = e->UserEvent();
      CICQEventTag *tag = licqDaemon->icqSendMessage(e->Uin(), user_event->Text(), false,
        ICQ_TCPxMSG_URGENT); //urgent, because, hey, he asked us, right?
	    delete tag;
    }
  }

  delete e;
}


void CLicqAutoReply::ProcessUserEvent(unsigned long nUin, unsigned long nId)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL)
  {
    gLog.Warn("%sInvalid uin received from daemon (%ld).\n", L_AUTOREPxSTR, nUin);
    return;
  }

  CUserEvent *e = u->EventPeekId(nId);
  gUserManager.DropUser(u);

  if (e == NULL)
  {
    gLog.Warn("%sInvalid message id (%ld).\n", L_AUTOREPxSTR, nId);
  }
  else
  {
    bool r = AutoReplyEvent(nUin, e);
    if (m_bDelete && r)
    {
      u = gUserManager.FetchUser(nUin, LOCK_W);
      u->EventClearId(nId);
      gUserManager.DropUser(u);
    }
  }
}


bool CLicqAutoReply::AutoReplyEvent(unsigned long nUin, CUserEvent *event)
{
  char m_szMessage[4096], szCommand[4096];
  char c;
  int pos = 0;

  for (int i = 0; i < 4096; i++)
  {
    m_szMessage[i] = '\0';
  }

  char *buf = szCommand;
  buf += sprintf(buf, "%s ", m_szProgram);
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  u->usprintf(buf, m_szArguments);
  gUserManager.DropUser(u);

  if (!POpen(szCommand))
  {
    gLog.Warn("%sCould not execute %s\n", L_AUTOREPxSTR, szCommand);
    return false;
  }
  if (m_bPassMessage)
  {
    fprintf(fStdIn, "%s\n", event->Text());
    fclose(fStdIn);
    fStdIn = NULL;
  }

  while (((c = fgetc(fStdOut)) != EOF) && (pos < 4096))
  {
    m_szMessage[pos++] = c;
  }
  PClose();

  char *szText = new char[4096 + 256];
  sprintf(szText, "%s", m_szMessage);
  CICQEventTag *tag = licqDaemon->icqSendMessage(nUin, szText, true,
     ICQ_TCPxMSG_URGENT);
  delete []szText;

  u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return false;

  if (tag == NULL)
  {
    gLog.Warn("%sSending message to %s (%ld) failed.\n", L_AUTOREPxSTR,
     u->GetAlias(), nUin);
  }
  else
  {
    gLog.Info("%sSent autoreply to %s (%ld).\n", L_AUTOREPxSTR, u->GetAlias(),
     nUin);
  }

  gUserManager.DropUser(u);
  delete tag;
  return tag != NULL;
}



bool CLicqAutoReply::POpen(const char *cmd)
{
  int pdes_out[2], pdes_in[2];

  if (pipe(pdes_out) < 0) return false;
  if (pipe(pdes_in) < 0) return false;

  switch (pid = fork())
  {
    case -1:                        /* Error. */
    {
      close(pdes_out[0]);
      close(pdes_out[1]);
      close(pdes_in[0]);
      close(pdes_in[1]);
      return false;
      /* NOTREACHED */
    }
    case 0:                         /* Child. */
    {
      if (pdes_out[1] != STDOUT_FILENO)
      {
        dup2(pdes_out[1], STDOUT_FILENO);
        close(pdes_out[1]);
      }
      close(pdes_out[0]);
      if (pdes_in[0] != STDIN_FILENO)
      {
        dup2(pdes_in[0], STDIN_FILENO);
        close(pdes_in[0]);
      }
      close(pdes_in[1]);
      execl(_PATH_BSHELL, "sh", "-c", cmd, NULL);
      _exit(127);
      /* NOTREACHED */
    }
  }

  /* Parent; assume fdopen can't fail. */
  fStdOut = fdopen(pdes_out[0], "r");
  close(pdes_out[1]);
  fStdIn = fdopen(pdes_in[1], "w");
  close(pdes_in[0]);

  // Set both streams to line buffered
  setvbuf(fStdOut, (char*)NULL, _IOLBF, 0);
  setvbuf(fStdIn, (char*)NULL, _IOLBF, 0);

  return true;
}


void CLicqAutoReply::PClose()
{
   int r, pstat;

   // Close the file descriptors
   if (fStdOut != NULL) fclose(fStdOut);
   if (fStdIn != NULL) fclose(fStdIn);
   fStdOut = fStdIn = NULL;

   // See if the child is still there
   r = waitpid(pid, &pstat, WNOHANG);
   // Return if child has exited or there was an inor
   if (r == pid || r == -1) return;

   // Give the process another .2 seconds to die
   struct timeval tv = { 0, 200000 };
   select(0, NULL, NULL, NULL, &tv);

   // Still there?
   r = waitpid(pid, &pstat, WNOHANG);
   if (r == pid || r == -1) return;

   // Try and kill the process
   if (kill(pid, SIGTERM) == -1) return;

   // Give it 1 more second to die
   tv.tv_sec = 1;
   tv.tv_usec = 0;
   select(0, NULL, NULL, NULL, &tv);

   // See if the child is still there
   r = waitpid(pid, &pstat, WNOHANG);
   if (r == pid || r == -1) return;

   // Kill the bastard
   kill(pid, SIGKILL);
   // Now he will die for sure
   waitpid(pid, &pstat, 0);
}


