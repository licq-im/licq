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
#include <cstring>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

#include "forwarder.h"
#include "forwarder.conf.h"
#include <licq/logging/log.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/icqdefines.h>
#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/inifile.h>
#include <licq/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/socket.h>
#include <licq/translator.h>
#include <licq/userevents.h>

extern "C" { const char *LP_Version(); }

using std::string;
using Licq::gLog;
using Licq::gPluginManager;
using Licq::gProtocolManager;
using Licq::gUserManager;
using Licq::UserId;

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
  gLog.info("Shutting down forwarder");
  gPluginManager.unregisterGeneralPlugin();
}


/*---------------------------------------------------------------------------
 * CLicqForwarder::Run
 *-------------------------------------------------------------------------*/
int CLicqForwarder::Run()
{
  // Register with the daemon, we only want the update user signal
  m_nPipe = gPluginManager.registerGeneralPlugin(Licq::PluginSignal::SignalUser);

  // Create our smtp information
  m_nSMTPPort = 25; //getservicebyname("smtp");
  string filename = "licq_forwarder.conf";
  Licq::IniFile conf(filename);
  if (!conf.loadFile())
  {
    if(!CreateDefaultConfig())
    {
      gLog.error("Could not create default configuration file: %s", filename.c_str());
      return 1;
    }
    gLog.info("A default configuration file has been created: %s", filename.c_str());
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
        gLog.error("Invalid ICQ forward UIN: %s", accountId.c_str());
        return 1;
      }
      myUserId = UserId(accountId, LICQ_PPID);
      break;
    }
    default:
      gLog.error("Invalid forward type: %d",  m_nForwardType);
      return 1;
      break;
  }

  // Log on if necessary
  if (m_szStatus != NULL)
  {
    unsigned s;
    if (!Licq::User::stringToStatus(m_szStatus, s))
      gLog.warning("Invalid startup status");
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

/*---------------------------------------------------------------------------
 * CLicqForwarder::CreateDefaultConfig
 *-------------------------------------------------------------------------*/
bool CLicqForwarder::CreateDefaultConfig()
{
  // Create licq_forwarder.conf
  string filename = Licq::gDaemon.baseDir() + "licq_forwarder.conf";
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
      Licq::PluginSignal* s = Licq::gDaemon.popPluginSignal();
      if (m_bEnabled)
        ProcessSignal(s);
      delete s;
      break;
    }

    case Licq::GeneralPlugin::PipeEvent:
    {
      // An event is pending (should never happen)
      Licq::Event* e = Licq::gDaemon.PopPluginEvent();
      if (m_bEnabled)
        ProcessEvent(e);
      delete e;
      break;
    }

    case Licq::GeneralPlugin::PipeShutdown:
    {
    gLog.info("Exiting forwarder");
    m_bExit = true;
    break;
  }

    case Licq::GeneralPlugin::PipeDisable:
    {
    gLog.info("Disabling forwarder");
    m_bEnabled = false;
    break;
  }

    case Licq::GeneralPlugin::PipeEnable:
    {
    gLog.info("Enabling forwarder");
    m_bEnabled = true;
    break;
  }

  default:
    gLog.warning("Unknown notification type from daemon: %c", buf[0]);
  }
}


/*---------------------------------------------------------------------------
 * CLicqForwarder::ProcessSignal
 *-------------------------------------------------------------------------*/
void CLicqForwarder::ProcessSignal(Licq::PluginSignal* s)
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
void CLicqForwarder::ProcessEvent(Licq::Event* /* e */)
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
    if (m_bDelete && r)
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
    case FORWARD_ICQ:
      s = ForwardEvent_ICQ(u, e);
      break;
  }
  return s;
}


bool CLicqForwarder::ForwardEvent_ICQ(const Licq::User* u, const Licq::UserEvent* e)
{
  char *szText = new char[e->text().size() + 256];
  char szTime[64];
  time_t t = e->Time();
  strftime(szTime, 64, "%a %b %d, %R", localtime(&t));
  sprintf(szText, "[ %s from %s (%s) sent %s ]\n\n%s\n", e->description().c_str(),
      u->getAlias().c_str(), u->accountId().c_str(), szTime, e->text().c_str());
  unsigned long tag = gProtocolManager.sendMessage(myUserId, szText, true, ICQ_TCPxMSG_NORMAL);
  delete []szText;
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
  char szTo[256],
       szFrom[256],
       szDate[256],
       szReplyTo[256];
  time_t t = e->Time();
  string subject;

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
      string s = e->text().substr(0, SUBJ_CHARS);
      size_t pos = s.find('\n');
      if (pos != string::npos)
        s.erase(pos);
      subject = "Subject: " + e->description() + " [" + s +
          (e->text().size() > SUBJ_CHARS ? "..." : "") + "]";
      break;
    }
  case ICQ_CMDxSUB_URL:
      subject = "Subject: " + e->description() + " [" +
          dynamic_cast<const Licq::EventUrl*>(e)->url() + "]";
      break;
  case ICQ_CMDxSUB_FILE:
      subject = "Subject: " + e->description() + " [" +
          dynamic_cast<const Licq::EventFile*>(e)->filename() + "]";
      break;
    default:
      subject = "Subject: " + e->description();
  }


  // Connect to the SMTP server
  if (!tcp->DestinationSet() && !tcp->connectTo(mySmtpHost.c_str(), m_nSMTPPort))
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

  string textDos = Licq::gTranslator.returnToDos(e->text());
  fprintf(fs, "%s"
              "%s\r\n"
              "%s\r\n"
              "%s\r\n"
              "%s\r\n"
              "\r\n"
              "%s\r\n.\r\n",
      szDate, szFrom, szTo, szReplyTo, subject.c_str(), textDos.c_str());

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




