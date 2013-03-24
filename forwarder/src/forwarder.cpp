/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "forwarder.h"

#include <boost/foreach.hpp>
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstring>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

#include <licq/logging/log.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/event.h>
#include <licq/inifile.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/socket.h>
#include <licq/translator.h>
#include <licq/userevents.h>

using std::string;
using Licq::gLog;
using Licq::gProtocolManager;
using Licq::gUserManager;
using Licq::UserId;

extern const char* const FORWARDER_CONF;

const unsigned short SUBJ_CHARS = 20;

/*---------------------------------------------------------------------------
 * CLicqForwarder::Constructor
 *-------------------------------------------------------------------------*/
CLicqForwarder::CLicqForwarder()
  : myIsEnabled(false),
    myMarkAsRead(false)
{
  tcp = new Licq::TCPSocket;
  m_bExit = false;
}


/*---------------------------------------------------------------------------
 * CLicqForwarder::Destructor
 *-------------------------------------------------------------------------*/
CLicqForwarder::~CLicqForwarder()
{
  delete tcp;
}

bool CLicqForwarder::init(int argc, char** argv)
{
  //char *LocaleVal = new char;
  //LocaleVal = setlocale (LC_ALL, "");
  //bindtextdomain (PACKAGE, LOCALEDIR);
  //textdomain (PACKAGE);

  // parse command line for arguments
  int i = 0;
  while ( (i = getopt(argc, argv, "el:d")) > 0)
  {
    switch (i)
    {
      case 'e': // enable
        myIsEnabled = true;
        break;
      case 'l': //log on
        myStartupStatus = optarg;
        break;
      case 'd':
        myMarkAsRead = true;
        break;
    }
  }
  return true;
}

/*---------------------------------------------------------------------------
 * CLicqForwarder::Run
 *-------------------------------------------------------------------------*/
int CLicqForwarder::run()
{
  // Register with the daemon, we only want the update user signal
  m_nPipe = getReadPipe();
  setSignalMask(Licq::PluginSignal::SignalUser);

  // Create our smtp information
  m_nSMTPPort = 25; //getservicebyname("smtp");
  string filename = "licq_forwarder.conf";
  Licq::IniFile conf(filename);
  if (!conf.loadFile())
  {
    conf.loadRawConfiguration(FORWARDER_CONF);
    if (!conf.writeFile())
    {
      gLog.error("Could not create default configuration file: %s", filename.c_str());
      return 1;
    }
    gLog.info("A default configuration file has been created: %s", filename.c_str());
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
    case FORWARD_LICQ:
    {
      conf.setSection("Licq");
      string accountId;
      conf.get("UserId", accountId);
      string ownerAccountId;
      conf.get("OwnerId", ownerAccountId);
      string protocol;
      conf.get("Protocol", protocol);
      unsigned long protocolId = Licq::protocolId_fromString(protocol);

      if (protocolId == 0 || ownerAccountId.empty() || accountId.empty())
      {
        gLog.error("Missing or invalid configuration in Licq section");
        return 1;
      }

      myUserId = Licq::UserId(Licq::UserId(protocolId, ownerAccountId), accountId);
      break;
    }
    default:
      gLog.error("Invalid forward type: %d", m_nForwardType);
      return 1;
      break;
  }

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
  return 0;
}

bool CLicqForwarder::isEnabled() const
{
  return myIsEnabled;
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
      gLog.info("Exiting forwarder");
      m_bExit = true;
      break;

    case PipeDisable:
      gLog.info("Disabling forwarder");
      myIsEnabled = false;
      break;

    case PipeEnable:
      gLog.info("Enabling forwarder");
      myIsEnabled = true;
      break;

  default:
    gLog.warning("Unknown notification type from daemon: %c", buf[0]);
  }
}


/*---------------------------------------------------------------------------
 * CLicqForwarder::ProcessSignal
 *-------------------------------------------------------------------------*/
void CLicqForwarder::ProcessSignal(const Licq::PluginSignal* s)
{
  switch (s->signal())
  {
    case Licq::PluginSignal::SignalUser:
      if (s->subSignal() == Licq::PluginSignal::UserEvents && s->argument() > 0)
        ProcessUserEvent(s->userId(), s->argument());
    break;
  // We should never get any other signal
    default:
      gLog.warning("Internal error: CLicqForwarder::ProcessSignal(): "
                   "Unknown signal command received from daemon: %d",
                   s->signal());
      break;
  }
}


/*---------------------------------------------------------------------------
 * CLicqForwarder::ProcessEvent
 *-------------------------------------------------------------------------*/
void CLicqForwarder::ProcessEvent(const Licq::Event* /* e */)
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
  }*/
}


void CLicqForwarder::ProcessUserEvent(const UserId& userId, unsigned long nId)
{
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
  {
    gLog.warning("Invalid user received from daemon (%s)",
                 userId.toString().c_str());
    return;
  }

  const Licq::UserEvent* e = u->EventPeekId(nId);

  if (e == NULL)
  {
    gLog.warning("Invalid message id (%ld)", nId);
  }
  else
  {
    bool r = ForwardEvent(*u, e);
    if (myMarkAsRead && r)
      u->EventClearId(nId);
  }
}


bool CLicqForwarder::ForwardEvent(const Licq::User* u, const Licq::UserEvent* e)
{
  if (e == NULL) return false;

  bool s = false;
  switch (m_nForwardType)
  {
    case FORWARD_EMAIL:
      s = ForwardEvent_Email(u, e);
      break;
    case FORWARD_LICQ:
      s = ForwardEvent_Licq(u, e);
      break;
  }
  return s;
}


bool CLicqForwarder::ForwardEvent_Licq(const Licq::User* u, const Licq::UserEvent* e)
{
  char szTime[64];
  time_t t = e->Time();
  strftime(szTime, 64, "%a %b %d, %R", localtime(&t));
  string text = "[ " + Licq::gTranslator.toUtf8(e->description()) + " from " +
      u->getAlias() + " (" + u->accountId() + ") sent " + szTime + " ]\n\n" +
      e->text() + "\n";
  unsigned long tag = gProtocolManager.sendMessage(myUserId, text);
  if (tag == 0)
  {
    gLog.warning("Sending message to %s failed", myUserId.toString().c_str());
    return false;
  }
  gLog.info("Forwarded message from %s (%s) to %s",
      u->getAlias().c_str(), u->accountId().c_str(), myUserId.toString().c_str());
  return true;
}


bool CLicqForwarder::ForwardEvent_Email(const Licq::User* u, const Licq::UserEvent* e)
{
  string headTo, headFrom, headDate, headReplyTo;
  time_t t = e->Time();
  string subject;

  // Fill in the strings
  if (!u->isUser())
  {
    headTo = "To: " + u->getAlias() + " <" + mySmtpTo + ">";
    if (u->protocolId() == ICQ_PPID)
    {
      headFrom = "From: ICQ System Message <support@icq.com>";
      headReplyTo = "Reply-To: Mirabilis <support@icq.com>";
    }
    else
    {
      headFrom = "From: System Message <" + u->getEmail() + ">";
    }
  }
  else
  {
    {
      Licq::OwnerReadGuard o(u->id().ownerId());
      headTo = "To: " + o->getAlias() + " <" + mySmtpTo + ">";
    }
    if (u->protocolId() == ICQ_PPID)
      headFrom = "From: \"" + u->getAlias() + "\" <" + u->accountId() + "@pager.icq.com>";
    else
      headFrom = "From: \"" + u->getAlias() + "\" <" + u->getEmail() + ">";
    headReplyTo = "Reply-To: \"" + u->getFullName() + "\" <" + u->getEmail() + ">";
  }
  char ctimeBuf[32]; // Minimum 26 char according to man page
  headDate = string("Date: ") + ctime_r(&t, ctimeBuf);
  // ctime returns a string ending with \n, drop it
  headDate.erase(headDate.size()-1);

  string eventText = e->textLoc();

  switch (e->eventType())
  {
    case Licq::UserEvent::TypeMessage:
    case Licq::UserEvent::TypeChat:
    {
      string s = eventText.substr(0, SUBJ_CHARS);
      size_t pos = s.find('\n');
      if (pos != string::npos)
        s.erase(pos);
      subject = "Subject: " + e->description() + " [" + s +
          (eventText.size() > SUBJ_CHARS ? "..." : "") + "]";
      break;
    }
    case Licq::UserEvent::TypeUrl:
      subject = "Subject: " + e->description() + " [" +
          Licq::gTranslator.fromUtf8(dynamic_cast<const Licq::EventUrl*>(e)->url()) + "]";
      break;
    case Licq::UserEvent::TypeFile:
      subject = "Subject: " + e->description() + " [" +
          dynamic_cast<const Licq::EventFile*>(e)->filename() + "]";
      break;
    default:
      subject = "Subject: " + e->description();
  }


  // Connect to the SMTP server
  if (!tcp->DestinationSet() && !tcp->connectTo(mySmtpHost, m_nSMTPPort))
  {
    gLog.warning("Unable to connect to %s:%d: %s",
        tcp->getRemoteIpString().c_str(), tcp->getRemotePort(),
        tcp->errorStr().c_str());
    return false;
  }
  char fin[256];
  int code;
  FILE *fs = fdopen(tcp->Descriptor(), "r+");
  if (fs == NULL)
  {
    gLog.warning("Unable to open socket descriptor in file stream mode: %s",
        strerror(errno));
    return false;
  }

  // Talk to the server
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 220)
  {
    gLog.warning("Invalid SMTP welcome: %s", fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "HELO %s\r\n", mySmtpDomain.c_str());
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 250)
  {
    gLog.warning("Invalid response to HELO: %s", fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "MAIL From: %s\r\n", mySmtpFrom.c_str());
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 250)
  {
    gLog.warning("Invalid response to MAIL: %s", fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "RCPT TO: %s\r\n", mySmtpTo.c_str());
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 250)
  {
    gLog.warning("Invalid response to RCPT TO: %s", fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "DATA\r\n");
  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 354)
  {
    gLog.warning("Invalid response to DATA: %s", fin);
    tcp->CloseConnection();
    return false;
  }

  string textDos = Licq::gTranslator.returnToDos(eventText);
  string msg = headDate + "\r\n" + headFrom + "\r\n" + headTo + "\r\n" +
      headReplyTo + "\r\n" + subject + "\r\n\r\n" + textDos + "\r\n.\r\n";
  fprintf(fs, "%s", msg.c_str());

  fgets(fin, 256, fs);
  code = atoi(fin);
  if (code != 250)
  {
    gLog.warning("Invalid response to DATA done: %s", fin);
    tcp->CloseConnection();
    return false;
  }

  fprintf(fs, "quit\r\n");

  // Close our connection
  tcp->CloseConnection();

  gLog.info("Forwarded message from %s (%s) to %s",
      u->getAlias().c_str(), u->accountId().c_str(), mySmtpTo.c_str());
  return true;
}




