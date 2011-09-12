/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2011 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

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
#include <cerrno>
#ifdef __sun
# define _PATH_BSHELL "/bin/sh"
#else
# include <paths.h>
#endif

#include "autoreply.h"

#include <licq/logging/log.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/event.h>
#include <licq/inifile.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/protocolsignal.h>
#include <licq/userevents.h>

#include "pluginversion.h"

using namespace std;
using Licq::UserId;
using Licq::gLog;
using Licq::gProtocolManager;
using Licq::gUserManager;

const unsigned short SUBJ_CHARS = 20;

/*---------------------------------------------------------------------------
 * CLicqAutoReply::Constructor
 *-------------------------------------------------------------------------*/
CLicqAutoReply::CLicqAutoReply(Licq::GeneralPlugin::Params& p)
  : Licq::GeneralPlugin(p),
    myIsEnabled(false),
    myMarkAsRead(false)
{
  m_bExit = false;
}


/*---------------------------------------------------------------------------
 * CLicqAutoReply::Destructor
 *-------------------------------------------------------------------------*/
CLicqAutoReply::~CLicqAutoReply()
{
}

std::string CLicqAutoReply::name() const
{
  return "ICQ Auto Replyer";
}

std::string CLicqAutoReply::version() const
{
  return PLUGIN_VERSION_STRING;
}

std::string CLicqAutoReply::description() const
{
  return "ICQ message Auto Replyer";
}

std::string CLicqAutoReply::usage() const
{
  return
      "Usage:  Licq [options] -p autoreply -- [ -h ] [ -e ] [ -l <status> ] [ -d ]\n"
      "         -h          : help\n"
      "         -e          : start enabled\n"
      "         -l <status> : log on at startup\n"
      "         -d          : delete messages after auto-replying\n";
}

std::string CLicqAutoReply::configFile() const
{
  return "licq_autoreply.conf";
}

bool CLicqAutoReply::isEnabled() const
{
  return myIsEnabled;
}

bool CLicqAutoReply::init(int argc, char** argv)
{
  //char *LocaleVal = new char;
  //LocaleVal = setlocale (LC_ALL, "");
  //bindtextdomain (PACKAGE, LOCALEDIR);
  //textdomain (PACKAGE);

  // parse command line for arguments
  int i = 0;
  while ( (i = getopt(argc, argv, "dhel:")) > 0)
  {
    switch (i)
    {
      case 'h':  // help
        puts(usage().c_str());
        return false;
      case 'e': // enable
        myIsEnabled = true;
        break;
      case 'l': //log on
        myStartupStatus = optarg;
        break;
      case 'd': // delete new
        myMarkAsRead = true;
        break;
    }
  }

  return true;
}

/*---------------------------------------------------------------------------
 * CLicqAutoReply::Run
 *-------------------------------------------------------------------------*/
int CLicqAutoReply::run()
{
  // Register with the daemon, we only want the update user signal
  m_nPipe = getReadPipe();
  setSignalMask(Licq::PluginSignal::SignalUser);

  Licq::IniFile conf("licq_autoreply.conf");
  conf.loadFile();
  conf.setSection("Reply");
  conf.get("Program", myProgram, "cat");
  conf.get("Arguments", myArguments, "");
  conf.get("PassMessage", m_bPassMessage, false);
  conf.get("FailOnExitCode", m_bFailOnExitCode, false);
  conf.get("AbortDeleteOnExitCode", m_bAbortDeleteOnExitCode, false);
  conf.get("SendThroughServer", m_bSendThroughServer, true);
  conf.get("StartEnabled", myIsEnabled, myIsEnabled);
  conf.get("DeleteMessage", myMarkAsRead, myMarkAsRead);

  // Log on if necessary
  if (!myStartupStatus.empty())
  {
    unsigned s;
    if (!Licq::User::stringToStatus(myStartupStatus, s))
      gLog.warning("Invalid startup status");
    else
      gProtocolManager.setStatus(gUserManager.ownerUserId(LICQ_PPID), s);
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
      gLog.error("Error in select(): %s", strerror(errno));
      m_bExit = true;
    }
    else
    {
      if (FD_ISSET(m_nPipe, &fdSet))
        ProcessPipe();
    }
  }
  gLog.info("Shutting down auto reply");
  return 0;
}

void CLicqAutoReply::destructor()
{
  delete this;
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
    case Licq::GeneralPlugin::PipeSignal:
    {
      Licq::PluginSignal* s = popSignal();
      if (myIsEnabled)
        ProcessSignal(s);
      delete s;
      break;
    }

    case Licq::GeneralPlugin::PipeEvent:
    {
      // An event is pending (should never happen)
      Licq::Event* e = popEvent();
      if (myIsEnabled)
        ProcessEvent(e);
      delete e;
      break;
    }

    case Licq::GeneralPlugin::PipeShutdown:
    {
    gLog.info("Exiting");
    m_bExit = true;
    break;
  }

    case Licq::GeneralPlugin::PipeDisable:
    {
    gLog.info("Disabling");
      myIsEnabled = false;
      break;
    }

    case Licq::GeneralPlugin::PipeEnable:
    {
    gLog.info("Enabling");
      myIsEnabled = true;
      break;
    }

  default:
    gLog.warning("Unknown notification type from daemon: %c", buf[0]);
  }
}


/*---------------------------------------------------------------------------
 * CLicqAutoReply::ProcessSignal
 *-------------------------------------------------------------------------*/
void CLicqAutoReply::ProcessSignal(Licq::PluginSignal* s)
{
  switch (s->signal())
  {
    case Licq::PluginSignal::SignalUser:
      if (s->subSignal() == Licq::PluginSignal::UserEvents && !gUserManager.isOwner(s->userId()) && s->argument() > 0)
        processUserEvent(s->userId(), s->argument());
      break;
    // We should never get any other signal
    default:
      break;
  }
}


/*---------------------------------------------------------------------------
 * CLicqAutoReply::ProcessEvent
 *-------------------------------------------------------------------------*/
void CLicqAutoReply::ProcessEvent(Licq::Event* e)
{
  const Licq::UserEvent* user_event;

  if (e->Result() != Licq::Event::ResultAcked)
  {
    if ((e->flags() & Licq::Event::FlagDirect) &&
        (e->command() != Licq::Event::CommandChatInvite &&
        e->command() != Licq::Event::CommandFile))
    {
      user_event = e->userEvent();
      unsigned flags = Licq::ProtocolSignal::SendUrgent; //urgent, because, hey, he asked us, right?
      if (!m_bSendThroughServer)
        flags |= Licq::ProtocolSignal::SendDirect;
      gProtocolManager.sendMessage(e->userId(), user_event->text(), flags);
    }
  }
}


void CLicqAutoReply::processUserEvent(const UserId& userId, unsigned long nId)
{
  const Licq::UserEvent* e;

  {
    Licq::UserReadGuard u(userId);
    if (!u.isLocked())
    {
      gLog.warning("Invalid user id received from daemon (%s)",
          userId.toString().c_str());
      return;
    }

    e = u->EventPeekId(nId);
  }

  if (e == NULL)
  {
    gLog.warning("Invalid message id (%ld)", nId);
    return;
  }

  bool r = autoReplyEvent(userId, e);
  if (myMarkAsRead && r)
  {
    Licq::UserWriteGuard u(userId);
    u->EventClearId(nId);
  }
}

bool CLicqAutoReply::autoReplyEvent(const UserId& userId, const Licq::UserEvent* event)
{
  string command = myProgram + " ";
  {
    Licq::UserReadGuard u(userId);
    command += u->usprintf(myArguments);
  }

  if (!POpen(command.c_str()))
  {
    gLog.warning("Could not execute %s", command.c_str());
    return false;
  }
  if (m_bPassMessage)
  {
    fprintf(fStdIn, "%s\n", event->text().c_str());
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
    gLog.warning("%s returned abnormally: exit code %d", command.c_str(), r);
    return !m_bAbortDeleteOnExitCode;
  }

  unsigned flags = Licq::ProtocolSignal::SendUrgent;
  if (!m_bSendThroughServer)
    flags |= Licq::ProtocolSignal::SendDirect;

  unsigned long tag = gProtocolManager.sendMessage(userId, m_szMessage, flags);

  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return false;

  if (tag == 0)
  {
    gLog.warning("Sending message to %s (%s) failed",
        u->getAlias().c_str(), u->accountId().c_str());
  }
  else
  {
    gLog.info("Sent autoreply to %s (%s)",
        u->getAlias().c_str(), u->accountId().c_str());
  }

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


