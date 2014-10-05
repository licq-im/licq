/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2014 Licq developers <licq-dev@googlegroups.com>
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

#include <boost/foreach.hpp>
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <list>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

#include "autoreply.h"

#include <licq/logging/log.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/event.h>
#include <licq/exec.h>
#include <licq/inifile.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/protocolsignal.h>
#include <licq/translator.h>
#include <licq/userevents.h>

using Licq::UserId;
using Licq::gLog;
using Licq::gProtocolManager;
using Licq::gUserManager;

const unsigned short SUBJ_CHARS = 20;

/*---------------------------------------------------------------------------
 * CLicqAutoReply::Constructor
 *-------------------------------------------------------------------------*/
CLicqAutoReply::CLicqAutoReply()
  : myIsEnabled(false),
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

bool CLicqAutoReply::init(int argc, char** argv)
{
  //char *LocaleVal = new char;
  //LocaleVal = setlocale (LC_ALL, "");
  //bindtextdomain (PACKAGE, LOCALEDIR);
  //textdomain (PACKAGE);

  // parse command line for arguments
  int i = 0;
  while ( (i = getopt(argc, argv, "del:")) > 0)
  {
    switch (i)
    {
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
    {
      // Get a list of owners first since we can't call changeStatus with list locked
      std::list<Licq::UserId> owners;
      {
        Licq::OwnerListGuard ownerList;
        BOOST_FOREACH(const Licq::Owner* o, **ownerList)
          owners.push_back(o->id());
      }

      BOOST_FOREACH(const Licq::UserId& ownerId, owners)
        gProtocolManager.setStatus(ownerId, s);
    }
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

bool CLicqAutoReply::isEnabled() const
{
  return myIsEnabled;
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
    case PipeSignal:
      if (myIsEnabled)
        ProcessSignal(popSignal().get());
      else
        popSignal();
      break;

    case PipeEvent:
      // An event is pending (should never happen)
      if (myIsEnabled)
        ProcessEvent(popEvent().get());
      else
        popEvent();
      break;

    case PipeShutdown:
    {
      gLog.info("Exiting");
      m_bExit = true;
      break;
    }

    case PipeDisable:
    {
      gLog.info("Disabling");
      myIsEnabled = false;
      break;
    }

    case PipeEnable:
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
void CLicqAutoReply::ProcessSignal(const Licq::PluginSignal* s)
{
  switch (s->signal())
  {
    case Licq::PluginSignal::SignalUser:
      if (s->subSignal() == Licq::PluginSignal::UserEvents && !s->userId().isOwner() && s->argument() > 0)
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
void CLicqAutoReply::ProcessEvent(const Licq::Event* e)
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
  std::string command = myProgram + " ";
  {
    Licq::UserReadGuard u(userId);
    command += u->usprintf(myArguments);
  }

  std::string message;
  try
  {
    Licq::Exec cmd(command.c_str());
    message = cmd.process(event->textLoc() + "\n", true);
    if (m_bFailOnExitCode)
    {
      int r = cmd.kill(200*1000);
      if (r != 0)
      {
        gLog.warning("%s returned abnormally: exit code %d", command.c_str(), r);
        return !m_bAbortDeleteOnExitCode;
      }
    }
  }
  catch (...)
  {
    gLog.warning("Could not execute %s", command.c_str());
    return false;
  }

  unsigned flags = Licq::ProtocolSignal::SendUrgent;
  if (!m_bSendThroughServer)
    flags |= Licq::ProtocolSignal::SendDirect;

  unsigned long tag = gProtocolManager.sendMessage(userId,
      Licq::gTranslator.toUtf8(message), flags);

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
