#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cctype>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#ifdef HAVE_ERRNO_H
#include <cerrno>
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
#include "licq_constants.h"

extern "C" { const char *LP_Version(); }

const char L_AUTOREPxSTR[]  = "[RPL] ";
const unsigned short SUBJ_CHARS = 20;

/*---------------------------------------------------------------------------
 * CLicqAutoReply::Constructor
 *-------------------------------------------------------------------------*/
CLicqAutoReply::CLicqAutoReply(bool _bEnable, bool _bDelete, char *_szStatus)
{
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

  char filename[256];
  sprintf (filename, "%s/licq_autoreply.conf", BASE_DIR);
  CIniFile conf;
  conf.LoadFile(filename);
  conf.SetSection("Reply");
  conf.ReadStr("Program", m_szProgram, "cat");
  conf.ReadStr("Arguments", m_szArguments, "");
  conf.ReadBool("PassMessage", m_bPassMessage, false);
  conf.ReadBool("FailOnExitCode", m_bFailOnExitCode, false);
  conf.ReadBool("AbortDeleteOnExitCode", m_bAbortDeleteOnExitCode, false);
  conf.ReadBool("SendThroughServer", m_bSendThroughServer, true);
  conf.ReadBool("StartEnabled", m_bEnabled, false);
  conf.ReadBool("DeleteMessage", m_bDelete, false);
  conf.CloseFile();

  // Log on if necessary
  if (m_szStatus != NULL)
  {
    unsigned long s = StringToStatus(m_szStatus);
    ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
    bool b = o->StatusOffline();
    gUserManager.DropOwner(o);
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
      if (s->SubSignal() == USER_EVENTS && gUserManager.FindOwner(s->Id(), s->PPID()) == NULL && s->Argument() > 0)
        ProcessUserEvent(s->Id(), s->PPID(), s->Argument());
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
  const CUserEvent* user_event;

  if (e->Result() != EVENT_ACKED)
  {
    if (e->Command() == ICQ_CMDxTCP_START &&
        (e->SubCommand() != ICQ_CMDxSUB_CHAT &&
         e->SubCommand() != ICQ_CMDxSUB_FILE))
    {
	    user_event = e->UserEvent();
      licqDaemon->icqSendMessage(e->Id(), user_event->Text(), !m_bSendThroughServer,
        ICQ_TCPxMSG_URGENT); //urgent, because, hey, he asked us, right?
    }
  }

  delete e;
}


void CLicqAutoReply::ProcessUserEvent(const char *szId, unsigned long nPPID, unsigned long nId)
{
  ICQUser *u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
  if (u == NULL)
  {
    gLog.Warn("%sInvalid user id received from daemon (%s).\n", L_AUTOREPxSTR, szId);
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
    bool r = AutoReplyEvent(szId, nPPID, e);
    if (m_bDelete && r)
    {
      u = gUserManager.FetchUser(szId, nPPID, LOCK_W);
      u->EventClearId(nId);
      gUserManager.DropUser(u);
    }
  }
}


bool CLicqAutoReply::AutoReplyEvent(const char *szId, unsigned long nPPID, CUserEvent *event)
{
  char *szCommand;
  char buf[4096];
  char *tmp;
  sprintf(buf, "%s ", m_szProgram);
  ICQUser *u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
  tmp = u->usprintf(m_szArguments);
  gUserManager.DropUser(u);
  szCommand = new char[strlen(buf) + strlen(tmp) + 1];
  strcpy(szCommand, buf);
  strcat(szCommand, tmp);
  free(tmp);

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

  int pos = 0;
  int c;
  char m_szMessage[4097];
  while (((c = fgetc(fStdOut)) != EOF) && (pos < 4096))
  {
    m_szMessage[pos++] = c;
  }
  m_szMessage[pos] = '\0';

  int r = 0;
  if ((r = PClose()) != 0 && m_bFailOnExitCode)
  {
    gLog.Warn("%s%s returned abnormally: exit code %d\n", L_AUTOREPxSTR,
     szCommand, r);
    delete [] szCommand;
    return !m_bAbortDeleteOnExitCode;
  }

  char *szText = new char[4096 + 256];
  sprintf(szText, "%s", m_szMessage);
  unsigned long tag = licqDaemon->ProtoSendMessage(szId, nPPID, szText, !m_bSendThroughServer,
     ICQ_TCPxMSG_URGENT);
  delete []szText;
  delete [] szCommand;

  u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
  if (u == NULL) return false;

  if (tag == 0)
  {
    gLog.Warn("%sSending message to %s (%s) failed.\n", L_AUTOREPxSTR,
     u->GetAlias(), szId);
  }
  else
  {
    gLog.Info("%sSent autoreply to %s (%s).\n", L_AUTOREPxSTR, u->GetAlias(),
     szId);
  }

  gUserManager.DropUser(u);
  return tag != 0;
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


int CLicqAutoReply::PClose()
{
   int r, pstat;
   struct timeval tv = { 0, 200000 };

   // Close the file descriptors
   if (fStdOut != NULL) fclose(fStdOut);
   if (fStdIn != NULL) fclose(fStdIn);
   fStdOut = fStdIn = NULL;

   // See if the child is still there
   r = waitpid(pid, &pstat, WNOHANG);
   // Return if child has exited or there was an inor
   if (r == pid || r == -1) goto pclose_done;

   // Give the process another .2 seconds to die
   select(0, NULL, NULL, NULL, &tv);

   // Still there?
   r = waitpid(pid, &pstat, WNOHANG);
   if (r == pid || r == -1) goto pclose_done;

   // Try and kill the process
   if (kill(pid, SIGTERM) == -1) return -1;

   // Give it 1 more second to die
   tv.tv_sec = 1;
   tv.tv_usec = 0;
   select(0, NULL, NULL, NULL, &tv);

   // See if the child is still there
   r = waitpid(pid, &pstat, WNOHANG);
   if (r == pid || r == -1) goto pclose_done;

   // Kill the bastard
   kill(pid, SIGKILL);
   // Now he will die for sure
   waitpid(pid, &pstat, 0);

pclose_done:

   if (WIFEXITED(pstat)) return WEXITSTATUS(pstat);
   return -1;

}


