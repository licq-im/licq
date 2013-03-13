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

#include "rms.h"
#include "pluginversion.h"

#include <boost/foreach.hpp>
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

#include <licq/buffer.h>
#include <licq/contactlist/group.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/icq/icq.h>
#include <licq/inifile.h>
#include <licq/logging/log.h>
#include <licq/logging/logservice.h>
#include <licq/logging/logutils.h>
#include <licq/mainloop.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/translator.h>
#include <licq/userevents.h>

using Licq::UserId;
using Licq::gLog;
using Licq::gPluginManager;
using Licq::gProtocolManager;
using Licq::gUserManager;
using std::string;

CLicqRMS *licqRMS = NULL;

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
const unsigned short CODE_STATUSxDONE = 212;
const unsigned short CODE_VIEWxTIME = 220;
const unsigned short CODE_VIEWxFLAGS = 221;
const unsigned short CODE_VIEWxTEXTxSTART = 222;
const unsigned short CODE_VIEWxTEXTxEND = 223;
const unsigned short CODE_ADDUSERxDONE = 224;
const unsigned short CODE_REMUSERxDONE = 225;
const unsigned short CODE_SECURExOPEN = 226;
const unsigned short CODE_SECURExCLOSE = 227;
const unsigned short CODE_SECURExSTAT = 228;
const unsigned short CODE_NOTIFYxON = 229;
const unsigned short CODE_NOTIFYxOFF = 230;
const unsigned short CODE_HISTORYxEND = 231;
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

const unsigned short CODE_NOTIFYxSTATUS = 600;
const unsigned short CODE_NOTIFYxMESSAGE = 601;

const unsigned short STATE_UIN = 1;
const unsigned short STATE_PASSWORD = 2;
const unsigned short STATE_COMMAND = 3;
const unsigned short STATE_ENTERxMESSAGE = 4;
const unsigned short STATE_ENTERxURLxDESCRIPTION = 5;
const unsigned short STATE_ENTERxURL = 6;
const unsigned short STATE_ENTERxAUTOxRESPONSE = 7;
const unsigned short STATE_ENTERxSMSxMESSAGE = 8;
const unsigned short STATE_ENTERxSMSxNUMBER = 9;

#define NEXT_WORD(s) while (*s != '\0' && *s == ' ') s++;

struct Command
{
  const char *name;
  int (CRMSClient::*fcn)();
  const char *help;
};

static struct Command commands[] =
{
  { "ADDUSER", &CRMSClient::Process_ADDUSER,
    "Add user to contact list { <id>[.<protocol>] }." },
  { "AR", &CRMSClient::Process_AR,
    "Set your (or a user custom) auto response { [ <id>[.<protocol>] ] }." },
  { "GROUPS", &CRMSClient::Process_GROUPS,
    "Show list of groups." },
  { "HELP", &CRMSClient::Process_HELP,
    "Print out help on commands." },
  { "HISTORY", &CRMSClient::Process_HISTORY,
    "View history of specific user { <id>[.<protocol>] [<length>] [<offset>]}." },
  { "INFO", &CRMSClient::Process_INFO,
    "Print out user information.  Argument is the id and protocol, or none for personal." },
  { "LIST", &CRMSClient::Process_LIST,
    "List users { [ <group #> ] [ <online|offline|all> ] [ <format> ] }." },
  { "LOG", &CRMSClient::Process_LOG,
    "Dump log messages { <log types> }." },
  { "MESSAGE", &CRMSClient::Process_MESSAGE,
    "Send a message { <id>[.<protocol>] }." },
  { "QUIT", &CRMSClient::Process_QUIT,
    "Close the connection.  With an argument of 1 causes the plugin to unload." },
  { "REMUSER", &CRMSClient::Process_REMUSER,
    "Remove user from contact list { <id>[.<protocol>] }." },
  { "SECURE", &CRMSClient::Process_SECURE,
    "Open/close/check secure channel { <uin> [ <open|close> ] } ." },
  { "STATUS", &CRMSClient::Process_STATUS,
    "Set or show status.  Argument is new status and protocol, or blank to display current. { [ <status>[.<protocol>] ] }." },
  { "TERM", &CRMSClient::Process_TERM,
    "Terminate the licq daemon." },
  { "VIEW", &CRMSClient::Process_VIEW,
    "View event (next or specific user) { [ <id>[.<protocol>] ] }." },
  { "URL", &CRMSClient::Process_URL,
    "Send a url { <id>[.<protocol>] }." },
  { "SMS", &CRMSClient::Process_SMS,
    "Send an sms { <uin> }." },
  { "NOTIFY", &CRMSClient::Process_NOTIFY,
    "Notify events" },
};

static const unsigned short NUM_COMMANDS = sizeof(commands)/sizeof(*commands);

/*---------------------------------------------------------------------------
 * CLicqRMS::Constructor
 *-------------------------------------------------------------------------*/
CLicqRMS::CLicqRMS(const std::string& configFile)
  : m_bEnabled(true),
    myPort(0),
    myConfigFile(configFile)
{
  licqRMS = this;
  server = NULL;
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
  gLog.info("Shutting down remote manager server");

  if (myLogSink)
    Licq::gLogService.unregisterLogSink(myLogSink);
}

bool CLicqRMS::init(int argc, char** argv)
{
  //char *LocaleVal = new char;
  //LocaleVal = setlocale (LC_ALL, "");
  //bindtextdomain (PACKAGE, LOCALEDIR);
  //textdomain (PACKAGE);

  // parse command line for arguments
  int i = 0;
  while ( (i = getopt(argc, argv, "dp:")) > 0)
  {
    switch (i)
    {
    case 'd': // enable
      m_bEnabled = false;
      break;
    case 'p':
      myPort = atol(optarg);
      break;
    }
  }
  return true;
}

/*---------------------------------------------------------------------------
 * CLicqRMS::Run
 *-------------------------------------------------------------------------*/
int CLicqRMS::run()
{
  setSignalMask(Licq::PluginSignal::SignalAll);

  Licq::IniFile conf(myConfigFile);
  if (conf.loadFile())
  {
    conf.setSection("RMS");

    // Ignore port in config if given on command line
    if (myPort == 0)
      conf.get("Port", myPort, 0);

    string protocolStr;
    conf.get("AuthProtocol", protocolStr, "ICQ");
    conf.get("AuthUser", myAuthUser);
    if (protocolStr == "Config")
    {
      // Get password from config file
      conf.get("AuthPassword", myAuthPassword);

      // Make sure we got something instead of opening a security problem
      if (myAuthUser.empty() || myAuthPassword.empty())
      {
        gLog.warning("Missing value for AuthUser or AuthPassword in configuration, "
            "login will not be possible.");
      }
    }
    else
    {
      // Parse protocol id
      unsigned long protocolId = Licq::protocolId_fromString(protocolStr);
      if (protocolId == 0 || myAuthUser.empty())
      {
        // Invalid
        gLog.warning("Invalid value for AuthProtocol or AuthUser in configuration, "
            "login will not be possible");
      }
      else
        myAuthOwnerId = Licq::UserId(protocolId, myAuthUser);
    }
  }

  server = new Licq::TCPSocket();

  if (Licq::gDaemon.tcpPortsLow() != 0 && myPort == 0)
  {
    if (!Licq::gDaemon.StartTCPServer(server))
    {
      Shutdown();
      return 1;
    }
  }
  else
  {
    if (!server->StartServer(myPort))
    {
      gLog.error("Could not start server on port %u, "
          "maybe this port is already in use?", myPort);
      Shutdown();
      return 1;
    };
  }

  gLog.info("RMS server started on port %d", server->getLocalPort());
  myMainLoop.addSocket(server, this);
  myMainLoop.addRawFile(getReadPipe(), this);

  myMainLoop.run();

  Shutdown();
  return 0;
}

bool CLicqRMS::isEnabled() const
{
  return m_bEnabled;
}

void CLicqRMS::rawFileEvent(int fd, int /* revents */)
{
  if (fd == getReadPipe())
    ProcessPipe();
  else if (myLogSink && fd == myLogSink->getReadPipe())
    ProcessLog();
}

void CLicqRMS::setupLogSink()
{
  if (!myLogSink)
  {
    myLogSink.reset(new Licq::PluginLogSink);
    Licq::gLogService.registerLogSink(myLogSink);
    myMainLoop.addRawFile(myLogSink->getReadPipe(), this);
  }

  unsigned int mask = 0;
  BOOST_FOREACH(CRMSClient* client, clients)
    mask |= client->myLogLevelsBitmask;
  myLogSink->setLogLevelsFromBitmask(mask);
}

void CLicqRMS::deleteClient(CRMSClient* client)
{
  delete client;

  for (ClientList::iterator iter = clients.begin(); iter != clients.end(); ++iter)
    if (*iter == client)
    {
      clients.erase(iter);
      break;
    }

  if (myLogSink)
    setupLogSink();
}

/*---------------------------------------------------------------------------
 * CLicqRMS::ProcessPipe
 *-------------------------------------------------------------------------*/
void CLicqRMS::ProcessPipe()
{
  char buf;
  read(getReadPipe(), &buf, 1);
  switch (buf)
  {
    case PipeSignal:
      if (m_bEnabled)
        ProcessSignal(popSignal().get());
      else
        popSignal();
      break;

    case PipeEvent:
      // An event is pending (should never happen)
      if (m_bEnabled)
        ProcessEvent(popEvent().get());
      else
        popEvent();
      break;

    case PipeShutdown:
      gLog.info("Exiting");
      myMainLoop.quit();
      break;

    case PipeDisable:
      gLog.info("Disabling");
      m_bEnabled = false;
      break;

    case PipeEnable:
      gLog.info("Enabling");
      m_bEnabled = true;
      break;

    default:
      gLog.warning("Unknown notification type from daemon: %c", buf);
  }
}


/*---------------------------------------------------------------------------
 * CLicqRMS::ProcessLog
 *-------------------------------------------------------------------------*/
void CLicqRMS::ProcessLog()
{
  using namespace Licq::LogUtils;

  Licq::LogSink::Message::Ptr message = myLogSink->popMessage();
  const char* level = levelToShortString(message->level);
  const std::string time = timeToString(message->time);

  BOOST_FOREACH(CRMSClient* client, clients)
  {
    if (levelInBitmask(message->level, client->myLogLevelsBitmask))
    {
      if (packetInBitmask(client->myLogLevelsBitmask)
          && !message->packet.empty())
      {
        ::fprintf(client->fs, "%d %s [%s] %s: %s\n%s\n",
                  CODE_LOG, time.c_str(), level,
                  message->sender.c_str(), message->text.c_str(),
                  packetToString(message).c_str());
      }
      else
      {
        ::fprintf(client->fs, "%d %s [%s] %s: %s\n",
                  CODE_LOG, time.c_str(), level,
                  message->sender.c_str(), message->text.c_str());
      }
      ::fflush(client->fs);
    }
  }
}


/*---------------------------------------------------------------------------
 * CLicqRMS::ProcessSignal
 *-------------------------------------------------------------------------*/
void CLicqRMS::ProcessSignal(const Licq::PluginSignal* s)
{
  switch (s->signal())
  {
    case Licq::PluginSignal::SignalUser:
      if (s->subSignal() == Licq::PluginSignal::UserStatus)
      {
        Licq::UserReadGuard u(s->userId());
        if (u.isLocked())
        {
        ClientList::iterator iter;
        for (iter = clients.begin(); iter != clients.end(); iter++)
        {
          if ((*iter)->m_bNotify)
          {
            fprintf((*iter)->fs, "%d %s\n", CODE_NOTIFYxSTATUS, u->usprintf("%u %P %-20a %3m %s").c_str());
            fflush((*iter)->fs);
          }
        }
        }
        break;
      }
      else if (s->subSignal() == Licq::PluginSignal::UserEvents)
      {
        Licq::UserReadGuard u(s->userId());
        if (u.isLocked())
        {
        ClientList::iterator iter;
        for (iter = clients.begin(); iter != clients.end(); iter++)
        {
          if ((*iter)->m_bNotify)
          {
            fprintf((*iter)->fs, "%d %s\n", CODE_NOTIFYxMESSAGE, u->usprintf("%u %P %3m").c_str());
            fflush((*iter)->fs);
          }
        }
      }
    }
  default:
    break;
    
  }
}

/*---------------------------------------------------------------------------
 * CLicqRMS::ProcessEvent
 *-------------------------------------------------------------------------*/
void CLicqRMS::ProcessEvent(const Licq::Event* e)
{
  ClientList ::iterator iter;
  for (iter = clients.begin(); iter != clients.end(); iter++)
  {
    if ((*iter)->ProcessEvent(e))
      break;
  }
}

void CLicqRMS::socketEvent(Licq::INetSocket* inetSocket, int /* revents */)
{
  if (inetSocket == server)
  {
    server->Lock();
    clients.push_back(new CRMSClient(server));
    server->Unlock();
  }
}

/*---------------------------------------------------------------------------
 * CRMSClient::constructor
 *-------------------------------------------------------------------------*/
CRMSClient::CRMSClient(Licq::TCPSocket* sin)
  : myLogLevelsBitmask(0)
{
  sin->RecvConnection(sock);
  licqRMS->myMainLoop.addSocket(&sock, this);

  gLog.info("Client connected from %s", sock.getRemoteIpString().c_str());
  fs = fdopen(sock.Descriptor(), "r+");
  fprintf(fs, "Licq Remote Management Server v" PLUGIN_VERSION_STRING "\n"
      "%d Enter your UIN:\n", CODE_ENTERxUIN);
  fflush(fs);

  m_szCheckId = 0;
  m_nState = STATE_UIN;
  data_line_pos = 0;
  m_bNotify = false;
}


/*---------------------------------------------------------------------------
 * CRMSClient::destructor
 *-------------------------------------------------------------------------*/
CRMSClient::~CRMSClient()
{
  licqRMS->myMainLoop.removeSocket(&sock);
  sock.CloseConnection();

  if (m_szCheckId)
    free(m_szCheckId);
}

void CRMSClient::socketEvent(Licq::INetSocket* /*inetSocket*/, int /*revents*/)
{
  if (Activity() == -1)
  {
    licqRMS->deleteClient(this);
  }
}

/*---------------------------------------------------------------------------
 * CRMSClient::ParseUser
 *-------------------------------------------------------------------------*/
void CRMSClient::ParseUser(const string& strData)
{
  myUserId = UserId();
  unsigned long protocolId = 0;
  string accountId;

  size_t pos = strData.rfind('.');
  if (pos != string::npos)
  {
    // Protocol specified
    protocolId = Licq::protocolId_fromString(strData.substr(pos+1));
    accountId = strData.substr(0, pos-1);
  }
  else
    accountId = strData;

  // Try and find an existing user that matches
  Licq::UserListGuard userList(protocolId);
  BOOST_FOREACH(const Licq::User* user, **userList)
  {
    if (user->accountId() == accountId)
    {
      myUserId = user->id();
      return;
    }
  }

  if (protocolId != 0)
  {
    // Use first owner for protocol
    Licq::OwnerListGuard ownerList(protocolId);
    if (!ownerList->empty())
    {
      myUserId = Licq::UserId((*ownerList->begin())->id(), accountId);
      return;
    }
  }

  // Failed
  myUserId = Licq::UserId();
}

/*---------------------------------------------------------------------------
 * CRMSClient::ProcessEvent
 *-------------------------------------------------------------------------*/
bool CRMSClient::ProcessEvent(const Licq::Event* e)
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
  const char *szr = NULL;
  switch(e->Result())
  {
    case Licq::Event::ResultAcked:
    case Licq::Event::ResultSuccess:
      nCode = CODE_RESULTxSUCCESS;
      szr = "done";
      break;
    case Licq::Event::ResultTimedout:
      nCode = CODE_EVENTxTIMEDOUT;
      szr = "timed out";
      break;
    case Licq::Event::ResultFailed:
    case Licq::Event::ResultUnsupported:
      nCode = CODE_EVENTxFAILED;
      szr = "failed";
      break;
    case Licq::Event::ResultError:
      nCode = CODE_EVENTxERROR;
      szr = "error";
      break;
    case Licq::Event::ResultCancelled:
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
  Licq::Buffer buf;
  if (!sock.receive(buf))
  {
    gLog.info("Client %s disconnected", sock.getRemoteIpString().c_str());
    return -1;
  }

  char* in = buf.getDataStart();
  char* last = buf.getDataPosWrite();

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
      myLoginUser = data_line;
      fprintf(fs, "%d Enter your password:\n", CODE_ENTERxPASSWORD);
      fflush(fs);
      m_nState = STATE_PASSWORD;
      break;
    }
    case STATE_PASSWORD:
    {
      bool ok = false;
      string name;
      if (licqRMS->myAuthOwnerId.isValid())
      {
        // Check against protocol owner
        Licq::OwnerReadGuard o(licqRMS->myAuthOwnerId);
        if (!o.isLocked())
          return -1;
        ok = (myLoginUser == o->accountId() && data_line == o->password());
        name = o->getAlias();
      }
      else if (!licqRMS->myAuthUser.empty() && !licqRMS->myAuthPassword.empty())
      {
        // User and password specified in RMS config
        ok = (myLoginUser == licqRMS->myAuthUser &&
            data_line == licqRMS->myAuthPassword);
        name = myLoginUser;
      }

      if (!ok)
      {
        gLog.info("Client failed validation from %s",
            sock.getRemoteIpString().c_str());
        fprintf(fs, "%d Invalid ID/Password.\n", CODE_INVALID);
        fflush(fs);
        return -1;
      }
      gLog.info("Client validated from %s",
          sock.getRemoteIpString().c_str());
      fprintf(fs, "%d Hello %s.  Type HELP for assistance.\n", CODE_HELLO,
         name.c_str());
      fflush(fs);
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
    case STATE_ENTERxSMSxMESSAGE:
    {
      if (AddLineToText())
         return Process_SMS_message();
      break;
    }
    case STATE_ENTERxSMSxNUMBER:
    {
      return Process_SMS_number();
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

  myText += data_line;
  myText += "\n";

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
  ParseUser(data_arg);

  //XXX Handle the case when we have the owner

  // Print the user info
  Licq::UserReadGuard u(myUserId);
  if (!u.isLocked())
  {
    fprintf(fs, "%d No such user.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }

  fprintf(fs, "%d %s Alias: %s\n", CODE_USERxINFO, u->accountId().c_str(),
      u->getAlias().c_str());
  fprintf(fs, "%d %s Status: %s\n", CODE_USERxINFO, u->accountId().c_str(),
      u->statusString().c_str());
  fprintf(fs, "%d %s First Name: %s\n", CODE_USERxINFO, u->accountId().c_str(),
    u->getFirstName().c_str());
  fprintf(fs, "%d %s Last Name: %s\n", CODE_USERxINFO, u->accountId().c_str(),
    u->getLastName().c_str());
  fprintf(fs, "%d %s Email 1: %s\n", CODE_USERxINFO, u->accountId().c_str(),
    u->getUserInfoString("Email1").c_str());
  fprintf(fs, "%d %s Email 2: %s\n", CODE_USERxINFO, u->accountId().c_str(),
    u->getUserInfoString("Email2").c_str());

  return fflush(fs);
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_STATUS
 *
 * Command:
 *   STATUS [ status | protocol ]
 *
 * Response:
 *
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_STATUS()
{
  // Show status
  if (data_arg[0] == '\0')
  {
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(const Licq::Owner* owner, **ownerList)
    {
      Licq::ProtocolPlugin::Ptr protocol = Licq::gPluginManager.getProtocolPlugin(owner->protocolId());
      Licq::OwnerReadGuard o(owner);
      fprintf(fs, "%d %s %s %s\n", CODE_STATUS, o->accountId().c_str(),
          protocol->name().c_str(), o->statusString().c_str());
    }
    fprintf(fs, "%d\n", CODE_STATUSxDONE);
    return fflush(fs);
  }

  // Set status
  string strData(data_arg);
  string::size_type nPos = strData.find_last_of(".");
  string status;

  // Get a list of owners first since we can't call changeStatus with list locked
  std::list<Licq::UserId> owners;
  if (nPos == string::npos)
  {
    status = data_arg;

    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(const Licq::Owner* o, **ownerList)
      owners.push_back(o->id());
  }
  else
  {
    status = string(strData, 0, nPos);
    string param(strData, nPos+1);
    unsigned long protocolId = Licq::protocolId_fromString(param);

    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(const Licq::Owner* o, **ownerList)
      if (o->protocolId() == protocolId || o->accountId() == param)
        owners.push_back(o->id());
  }

  BOOST_FOREACH(const Licq::UserId& ownerId, owners)
    changeStatus(ownerId, status);

  fprintf(fs, "%d Done setting status\n", CODE_STATUSxDONE);
  return fflush(fs);
}

int CRMSClient::changeStatus(const Licq::UserId& ownerId, const string& strStatus)
{
  unsigned status;
  if (!Licq::User::stringToStatus(strStatus, status))
  {
    fprintf(fs, "%d Invalid status.\n", CODE_INVALIDxSTATUS);
    return -1;
  }
  if (status == Licq::User::OfflineStatus)
  {
    fprintf(fs, "%d [0] Logging off %s.\n", CODE_COMMANDxSTART, strStatus.c_str());
    fflush(fs);
    gProtocolManager.setStatus(ownerId, Licq::User::OfflineStatus);
    fprintf(fs, "%d [0] Event done.\n", CODE_STATUSxDONE);
    return 0;
  }
  else
  {
    bool b;
    {
      Licq::OwnerReadGuard o(ownerId);
      if (!o.isLocked())
      {
        fprintf(fs, "%d Invalid protocol.\n", CODE_INVALIDxUSER);
        return -1;
      }
      b = !o->isOnline();
    }
    unsigned long tag = gProtocolManager.setStatus(ownerId, status);
    if (b)
      fprintf(fs, "%d [%ld] Logging on to %s.\n", CODE_COMMANDxSTART, tag, strStatus.c_str());
    else
      fprintf(fs, "%d [%ld] Setting status for %s.\n", CODE_COMMANDxSTART, tag, strStatus.c_str());
    tags.push_back(tag);
  }
  return 0;
}

/*---------------------------------------------------------------------------
 * CRMSClient::Process_QUIT
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_QUIT()
{
  fprintf(fs, "%d Sayonara.\n", CODE_QUIT);
  fflush(fs);
  if (strtoul(data_arg, (char**)NULL, 10) > 0)
    licqRMS->myMainLoop.quit();
  return -1;
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_TERM
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_TERM()
{
  Licq::gDaemon.Shutdown();
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
  int i = 1;
  Licq::GroupListGuard groupList;
  BOOST_FOREACH(const Licq::Group* group, **groupList)
  {
    Licq::GroupReadGuard pGroup(group);
    fprintf(fs, "%d %03d %s\n", CODE_LISTxGROUP, i, pGroup->name().c_str());
    ++i;
  }
  fprintf(fs, "%d\n", CODE_LISTxDONE);

  return fflush(fs);
}

int CRMSClient::Process_HISTORY()
{
  char* s = strtok(data_arg, " ");
  if (s == NULL)
  {
    fprintf(fs, "%d Invalid User.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }
  ParseUser(s);

  int length = 10;
  s = strtok(NULL, " ");
  if (s != NULL)
    length = atoi(s);

  int offset = 0;
  s = strtok(NULL, " ");
  if (s != NULL)
    offset = atoi(s);

  Licq::HistoryList history;
  string userAlias;
  string ownerAlias = "me";

  {
    Licq::UserReadGuard u(myUserId);
    if (!u.isLocked())
    {
      fprintf(fs, "%d Invalid User (%s).\n", CODE_INVALIDxUSER, myUserId.toString().c_str());
      return fflush(fs);
    }
    if (!u->GetHistory(history))
    {
      fprintf(fs, "%d Cannot load history file.\n", CODE_EVENTxERROR);
      return fflush(fs);
    }

    if (u->isUser())
    {
      userAlias = u->getAlias();
      Licq::OwnerReadGuard o(myUserId.ownerId());
      if (o.isLocked())
        ownerAlias = o->getAlias();
    }
    else
    {
      userAlias = "system";
      ownerAlias = u->getAlias();
    }
  }

  int counter = 0;
  Licq::HistoryList::reverse_iterator it;
  for (it = history.rbegin(); it != history.rend(); ++it)
  {
    ++counter;
    if (counter < offset || counter > offset + length)
      continue;

    printUserEvent(*it, ((*it)->isReceiver() ? userAlias : ownerAlias));
  }
  fprintf(fs, "%d End.\n", CODE_HISTORYxEND);
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
 *     documented in UTILITIES.HOWTO.  The default is "%u %P %-20a %3m %s"
 *     and prints out users as follows.
 *
 * Response:
 *   CODE_LISTxUSER   5550000 Licq            AnAlias   2 Online
 *     The default line contains the uin, protocol, alias, number of new 
 *     messages and status all column and white space deliminated.  Note that
 *     the alias may contain white space.
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

  string format;
  if (*data_arg == '\0')
    format = "%u %P %-20a %3m %s";
  else
    format = data_arg;

  Licq::UserListGuard userList;
  BOOST_FOREACH(const Licq::User* user, **userList)
  {
    Licq::UserReadGuard pUser(user);
    if (pUser->isInGroup(nGroup) &&
        ((!pUser->isOnline() && n&2) || (pUser->isOnline() && n&1)))
    {
      fprintf(fs, "%d %s\n", CODE_LISTxUSER, pUser->usprintf(format).c_str());
    }
  }
  fprintf(fs, "%d\n", CODE_LISTxDONE);

  return fflush(fs);
}



/*---------------------------------------------------------------------------
 * CRMSClient::Process_MESSAGE
 *
 * Command:
 *     MESSAGE <id>[.<protocol>]
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
  fprintf(fs, "%d Enter message, terminate with a . on a line by itself:\n",
     CODE_ENTERxTEXT);

  ParseUser(data_arg);

  myText.clear();

  m_nState = STATE_ENTERxMESSAGE;
  return fflush(fs);
}

int CRMSClient::Process_MESSAGE_text()
{
  //XXX Give a tag...
  myText.erase(myText.size() - 1);
  unsigned long tag = gProtocolManager.sendMessage(myUserId,
      Licq::gTranslator.toUtf8(myText));

  fprintf(fs, "%d [%ld] Sending message to %s.\n", CODE_COMMANDxSTART,
      tag, myUserId.toString().c_str());

  tags.push_back(tag);
  m_nState = STATE_COMMAND;

  return fflush(fs);
}



/*---------------------------------------------------------------------------
 * CRMSClient::Process_URL
 *
 * Command:
 *   URL <id>[.<protocol>]
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
  ParseUser(data_arg);

  myText.clear();

  m_nState = STATE_ENTERxURL;
  return fflush(fs);
}


int CRMSClient::Process_URL_url()
{
  myLine = data_line;

  fprintf(fs, "%d Enter description, terminate with a . on a line by itself:\n",
     CODE_ENTERxTEXT);

  myText.clear();

  m_nState = STATE_ENTERxURLxDESCRIPTION;
  return fflush(fs);
}


int CRMSClient::Process_URL_text()
{
  unsigned long tag = gProtocolManager.sendUrl(myUserId, myLine,
      Licq::gTranslator.toUtf8(myText));

  fprintf(fs, "%d [%ld] Sending URL to %s.\n", CODE_COMMANDxSTART,
      tag, myUserId.toString().c_str());

  tags.push_back(tag);
  m_nState = STATE_COMMAND;

  return fflush(fs);
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_SMS
 *
 * Command:
 *   SMS <uin>
 *
 * Response:
 *   CODE_ENTERxLINE | CODE_INVALIDxUSER
 *     At which point the phone number should be entered on a line by itself
 *     without the "+", but including country code.  Invalid user means the
 *     uin was invalid (< 10000) and the url was aborted.
 *   CODE_ENTERxTEXT
 *     Now the message should be entered and terminated by a "." on a line
 *     by itself.
 *   CODE_COMMANDxSTART
 *     < ...time... >
 *   CODE_RESULTxSUCCESS | CODE_EVENTxTIMEDOUT | CODE_EVENTxERROR
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_SMS()
{
  ParseUser(data_arg);

  if (!myUserId.isValid())
  {
    fprintf(fs, "%d Invalid UIN.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }
  fprintf(fs, "%d Enter NUMBER:\n", CODE_ENTERxLINE);

  myText.clear();

  m_nState = STATE_ENTERxSMSxNUMBER;
  return fflush(fs);
}


int CRMSClient::Process_SMS_number()
{
  myLine = data_line;

  fprintf(fs, "%d Enter message, terminate with a . on a line by itself:\n",
     CODE_ENTERxTEXT);

  myText.clear();

  m_nState = STATE_ENTERxSMSxMESSAGE;
  return fflush(fs);
}


int CRMSClient::Process_SMS_message()
{
  Licq::IcqProtocol::Ptr icq = plugin_internal_cast<Licq::IcqProtocol>(
      Licq::gPluginManager.getProtocolInstance(myUserId.ownerId()));
  if (!icq)
    return fflush(fs);

  unsigned long tag = icq->icqSendSms(
      myUserId, myLine, Licq::gTranslator.toUtf8(myText));

  fprintf(fs, "%d [%lu] Sending SMS to %s (%s).\n", CODE_COMMANDxSTART,
     tag, myUserId.accountId().c_str(), myLine.c_str());

  tags.push_back(tag);
  m_nState = STATE_COMMAND;

  return fflush(fs);
}


/*---------------------------------------------------------------------------
 * CRMSClient::Process_AR
 *
 * Command:
 *     AR [ <id>[.<protocol>] ]
 *
 * Response:
 *   CODE_ENTERxTEXT | CODE_INVALIDxUIN
 *     At which point the auto response should be entered line by line and
 *     terminated by entering a "." on a line by itself.
 *   CODE_RESULTxSUCCESS
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_AR()
{
  if (data_arg[0] == '\0')
  {
    // Clear user id to set general auto response
    myUserId = Licq::UserId();
  }
  else
  {
    // Parameter is user id to set custom autoresponse for
    ParseUser(data_arg);

    if (!myUserId.isValid())
    {
      fprintf(fs, "%d Invalid User.\n", CODE_INVALIDxUSER);
      return fflush(fs);
    }
  }

  fprintf(fs, "%d Enter %sauto response, terminate with a . on a line by itself:\n",
     CODE_ENTERxTEXT, myUserId.isValid() ? "custom " : "");

  myText.clear();

  m_nState = STATE_ENTERxAUTOxRESPONSE;
  return fflush(fs);
}

int CRMSClient::Process_AR_text()
{
  string textUtf8 = Licq::gTranslator.toUtf8(myText);

  if (!myUserId.isValid())
  {
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(Licq::Owner* owner, **ownerList)
    {
      Licq::OwnerWriteGuard o(owner);
      o->setAutoResponse(textUtf8);
      o->save(Licq::Owner::SaveOwnerInfo);
    }
  }
  else
  {
    Licq::UserWriteGuard u(myUserId);
    if (u.isLocked())
      u->setCustomAutoResponse(textUtf8);
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
 *   CODE_LOG 12:04:34.042 [TCP] Message from ...
 *   ...
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_LOG()
{
  unsigned short lt = strtoul(data_arg, (char**)NULL, 10);
  myLogLevelsBitmask = Licq::LogUtils::convertOldBitmaskToNew(lt);

  licqRMS->setupLogSink();

  fprintf(fs, "%d Log type set to %d.\n", CODE_LOGxTYPE, lt);

  return fflush(fs);
}

/*---------------------------------------------------------------------------
 * CRMSClient::Process_NOTIFY
 *
 * Command:
 *   NOTIFY
 *
 * Response:
 *   CODE_NOTIFYxON|CODE_NOTIFYxOFF
 *   ...
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_NOTIFY()
{
  m_bNotify = !m_bNotify;

  if (m_bNotify)
    fprintf(fs, "%d Notify set ON.\n", CODE_NOTIFYxON);
  else
    fprintf(fs, "%d Notify set OFF.\n", CODE_NOTIFYxOFF);

  return fflush(fs);
}

/*---------------------------------------------------------------------------
 * CRMSClient::Process_VIEW
 *
 * Command:
 *   VIEW [ <id>[.<protocol> ]
 *
 * Response:
 *
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_VIEW()
{
  if (*data_arg != '\0')
  {
    ParseUser(data_arg);
  }
  else
  {
    // XXX Check system messages first

    // Check user messages now
    Licq::UserListGuard userList;
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      Licq::UserReadGuard pUser(user);
      if(pUser->NewMessages() > 0)
      {
        myUserId = pUser->id();
        break;
      }
    }

    if (!myUserId.isValid())
    {
      fprintf(fs, "%d No new messages.\n", CODE_VIEWxNONE);
      return fflush(fs);
    }
  }

  Licq::UserWriteGuard u(myUserId);
  if (!u.isLocked())
  {
    fprintf(fs, "%d No such user.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }

  Licq::UserEvent* e = u->EventPop();
  printUserEvent(e, u->getAlias());

  return fflush(fs);
}

void CRMSClient::printUserEvent(const Licq::UserEvent* e, const string& alias)
{
  if (e == NULL)
  {
    fprintf(fs, "%d Invalid event\n", CODE_EVENTxERROR);
    return;
  }

  std::ostringstream eventHeader;
  switch (e->eventType())
  {
    case Licq::UserEvent::TypeMessage:
      eventHeader << CODE_VIEWxMSG << " Message";
      break;

    case Licq::UserEvent::TypeUrl:
      eventHeader << CODE_VIEWxURL << " URL";
      break;

    case Licq::UserEvent::TypeChat:
      eventHeader << CODE_VIEWxCHAT << " Chat Request";
      break;

    case Licq::UserEvent::TypeFile:
      eventHeader << CODE_VIEWxFILE << " File Request";
      break;

    default:
      eventHeader << CODE_VIEWxUNKNOWN << " Unknown Event";
  }

  eventHeader << " from ";
  eventHeader << alias;
  eventHeader << "\n";

  // Write out the event header
  fputs(eventHeader.str().c_str(), fs);

  // Timestamp
  char szTime[25];
  time_t nMessageTime = e->Time();
  struct tm* pTM = localtime(&nMessageTime);
  strftime(szTime, 25, "%Y-%m-%d %H:%M:%S", pTM);
  fprintf(fs, "%d Sent At %s\n", CODE_VIEWxTIME, szTime);

  // Message
  fprintf(fs, "%d Message Start\n", CODE_VIEWxTEXTxSTART);
  fputs(e->textLoc().c_str(), fs);
  fprintf(fs, "\n%d Message Complete\n", CODE_VIEWxTEXTxEND);
}

/*---------------------------------------------------------------------------
 * CRMSClient::Process_ADDUSER
 *
 * Command:
 *   ADDUSER <id> <protocol>
 *
 * Response:
 *
 *-------------------------------------------------------------------------*/
int CRMSClient::Process_ADDUSER()
{
  ParseUser(data_arg);

  if (!myUserId.isValid())
  {
    fprintf(fs, "%d Invalid UIN.\n", CODE_INVALIDxUSER);
  }
  else if (gUserManager.addUser(myUserId) != 0)
  {
    fprintf(fs, "%d User added\n", CODE_ADDUSERxDONE);
  }
  else
  {
    fprintf(fs, "%d User not added\n", CODE_ADDUSERxERROR);
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
  ParseUser(data_arg);

  if (myUserId.isValid() && gUserManager.userExists(myUserId))
  {
    gUserManager.removeUser(myUserId);
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
  if (!Licq::gDaemon.haveCryptoSupport())
  {
    fprintf(fs, "%d Licq secure channel not compiled. Please recompile with OpenSSL.\n", CODE_SECURExNOTCOMPILED);
    return fflush(fs);
  }

  ParseUser(data_arg);

  if (!myUserId.isValid())
  {
    fprintf(fs, "%d Invalid UIN.\n", CODE_INVALIDxUSER);
    return fflush(fs);
  }
  while (*data_arg != '\0' && *data_arg != ' ') data_arg++;
  NEXT_WORD(data_arg);

  if (strncasecmp(data_arg, "open", 4) == 0)
  {
    fprintf(fs, "%d Opening secure connection.\n", CODE_SECURExOPEN);
    gProtocolManager.secureChannelOpen(myUserId);
  }
  else
  if (strncasecmp(data_arg, "close", 5) == 0)
  {
    fprintf(fs, "%d Closing secure connection.\n", CODE_SECURExCLOSE);
    gProtocolManager.secureChannelClose(myUserId);
  }
  else
  {
    Licq::UserReadGuard u(myUserId);
    if (u.isLocked())
    {
      if (u->Secure() == 0)
        fprintf(fs, "%d Status: secure connection is closed.\n", CODE_SECURExSTAT);
      if (u->Secure() == 1)
        fprintf(fs, "%d Status: secure connection is open.\n", CODE_SECURExSTAT);
    }
  }

  return fflush(fs);
}
