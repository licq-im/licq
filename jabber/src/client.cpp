/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Erik Johansson <erijo@licq.org>
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

#include "client.h"
#include "jabber.h"

#include <gloox/connectiontcpclient.h>
#include <gloox/disco.h>

#include <licq_icq.h>
#include <licq_log.h>
#include <licq/licqversion.h>

Client::Client(const std::string& username, const std::string& password) :
  myJid(username + "/Licq"),
  myClient(myJid, password)
{
  myClient.registerConnectionListener(this);
  myClient.registerPresenceHandler(this);
  myClient.logInstance().registerLogHandler(
      gloox::LogLevelDebug, gloox::LogAreaAll, this);

  myClient.disco()->setVersion("Licq", LICQ_VERSION_STRING);
}

Client::~Client()
{
  myClient.disconnect();
}

int Client::getSocket()
{
  return static_cast<gloox::ConnectionTCPClient*>(
      myClient.connectionImpl())->socket();
}

void Client::recv()
{
  myClient.recv();
}

bool Client::connect(unsigned long status)
{
  changeStatus(status);
  return myClient.connect(false);
}

void Client::changeStatus(unsigned long status)
{
  gloox::Presence presence;
  switch (status & 0xFF)
  {
    case ICQ_STATUS_ONLINE:
      presence = gloox::PresenceAvailable;
      break;
    case ICQ_STATUS_AWAY:
      presence = gloox::PresenceAway;
      break;
    case ICQ_STATUS_DND:
    case ICQ_STATUS_OCCUPIED:
      presence = gloox::PresenceDnd;
      break;
    case ICQ_STATUS_NA:
      presence = gloox::PresenceXa;
      break;
    case ICQ_STATUS_FREEFORCHAT:
      presence = gloox::PresenceChat;
      break;
    default:
      return;
  }

  myClient.setPresence(presence);
}

void Client::onConnect()
{
}

bool Client::onTLSConnect(const gloox::CertInfo& /*info*/)
{
  return true;
}

void Client::onDisconnect(gloox::ConnectionError /*error*/)
{
}

void Client::handlePresence(gloox::Stanza* /*stanza*/)
{
}

void Client::handleLog(gloox::LogLevel level, gloox::LogArea area,
                       const std::string& message)
{
  const char* areaStr = "Area ???";
  switch (area)
  {
    case gloox::LogAreaClassParser:
      areaStr = "Parser";
      break;
    case gloox::LogAreaClassConnectionTCPBase:
      areaStr = "TCP base";
      break;
    case gloox::LogAreaClassClient:
      areaStr = "Client";
      break;
    case gloox::LogAreaClassClientbase:
      areaStr = "Client base";
      break;
    case gloox::LogAreaClassComponent:
      areaStr = "Component";
      break;
    case gloox::LogAreaClassDns:
      areaStr = "DNS";
      break;
    case gloox::LogAreaClassConnectionHTTPProxy:
      areaStr = "HTTP proxy";
      break;
    case gloox::LogAreaClassConnectionSOCKS5Proxy:
      areaStr = "SOCKS5 proxy";
      break;
    case gloox::LogAreaClassConnectionTCPClient:
      areaStr = "TCP client";
      break;
    case gloox::LogAreaClassConnectionTCPServer:
      areaStr = "TCP server";
      break;
    case gloox::LogAreaClassS5BManager:
      areaStr = "SOCKS5";
      break;
    case gloox::LogAreaXmlIncoming:
      areaStr = "XML in";
      break;
    case gloox::LogAreaXmlOutgoing:
      areaStr = "XML out";
      break;
    case gloox::LogAreaUser:
      areaStr = "User";
      break;
    case gloox::LogAreaAllClasses:
    case gloox::LogAreaAll:
      areaStr = "All";
      break;
  }

  switch (level)
  {
    default:
    case gloox::LogLevelDebug:
      gLog.Info("%s[%s] %s\n", L_JABBERxSTR, areaStr, message.c_str());
      break;
    case gloox::LogLevelWarning:
      gLog.Warn("%s[%s] %s\n", L_JABBERxSTR, areaStr, message.c_str());
      break;
    case gloox::LogLevelError:
      gLog.Error("%s[%s] %s\n", L_JABBERxSTR, areaStr, message.c_str());
      break;
  }
}
