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
#include "handler.h"
#include "jabber.h"

#include <gloox/connectiontcpclient.h>
#include <gloox/disco.h>
#include <gloox/rostermanager.h>

#include <licq_icq.h>
#include <licq_log.h>
#include <licq/licqversion.h>

Client::Client(Handler& handler, const std::string& username,
               const std::string& password) :
  myHandler(handler),
  myJid(username + "/Licq"),
  myClient(myJid, password),
  myRosterManager(myClient.rosterManager()),
  myVCardManager(&myClient)
{
  myClient.registerConnectionListener(this);
  myRosterManager->registerRosterListener(this, false);
  myClient.registerMessageHandler(this);
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
  myClient.setPresence(statusToPresence(status));
}

void Client::sendMessage(const std::string& user, const std::string& message)
{
  gloox::Stanza* stanza = gloox::Stanza::createMessageStanza(user, message);
  myClient.send(stanza);
}

void Client::getVCard(const std::string& user)
{
  myVCardManager.fetchVCard(gloox::JID(user), this);
}

void Client::onConnect()
{
  myHandler.onConnect();
}

bool Client::onTLSConnect(const gloox::CertInfo& /*info*/)
{
  return true;
}

void Client::onDisconnect(gloox::ConnectionError /*error*/)
{
  myHandler.onDisconnect();
}

void Client::handleItemAdded(const gloox::JID& jid)
{
  myHandler.onUserAdded(jid.bare(), "",
      myRosterManager->getRosterItem(jid)->groups());
}

void Client::handleItemSubscribed(const gloox::JID& /*jid*/)
{
}

void Client::handleItemRemoved(const gloox::JID& jid)
{
  myHandler.onUserRemoved(jid.bare());
}

void Client::handleItemUpdated(const gloox::JID& /*jid*/)
{
}

void Client::handleItemUnsubscribed(const gloox::JID& /*jid*/)
{
}

void Client::handleRoster(const gloox::Roster& roster)
{
  for (gloox::Roster::const_iterator it = roster.begin();
       it != roster.end(); ++it)
    myHandler.onUserAdded(it->first, it->second->name(), it->second->groups());
}

void Client::handleRosterPresence(const gloox::RosterItem& item,
                                  const std::string& /*resource*/,
                                  gloox::Presence presence,
                                  const std::string& /*msg*/)
{
  myHandler.onUserStatusChange(gloox::JID(item.jid()).bare(),
      presenceToStatus(presence));
}

void Client::handleSelfPresence(const gloox::RosterItem& /*item*/,
                                const std::string& /*resource*/,
                                gloox::Presence /*presence*/,
                                const std::string& /*msg*/)
{
}

bool Client::handleSubscriptionRequest(const gloox::JID& /*jid*/,
                                       const std::string& /*msg*/)
{
  return false;
}

bool Client::handleUnsubscriptionRequest(const gloox::JID& /*jid*/,
                                         const std::string& /*msg*/)
{
  return false;
}

void Client::handleNonrosterPresence(gloox::Stanza* /*stanza*/)
{
}

void Client::handleRosterError(gloox::Stanza* /*stanza*/)
{
}

void Client::handleMessage(gloox::Stanza* stanza,
                           gloox::MessageSession* /*session*/)
{
  if (!stanza->body().empty())
    myHandler.onMessage(stanza->from().bare(), stanza->body());
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

void Client::handleVCard(const gloox::JID& jid, gloox::VCard* vcard)
{
  (void)jid;
  delete vcard;
}

void Client::handleVCardResult(gloox::VCardHandler::VCardContext context,
                               const gloox::JID& jid, gloox::StanzaError error)
{
  (void)context;
  (void)jid;
  (void)error;
}

unsigned long Client::presenceToStatus(gloox::Presence presence)
{
  unsigned long status;

  switch (presence)
  {
    case gloox::PresenceUnknown:
      status = ICQ_STATUS_OFFLINE;
      break;
    case gloox::PresenceAvailable:
      status = ICQ_STATUS_ONLINE;
      break;
    case gloox::PresenceChat:
      status = ICQ_STATUS_FREEFORCHAT;
      break;
    case gloox::PresenceAway:
      status = ICQ_STATUS_AWAY;
      break;
    case gloox::PresenceDnd:
      status = ICQ_STATUS_DND;
      break;
    case gloox::PresenceXa:
      status = ICQ_STATUS_NA;
      break;
    case gloox::PresenceUnavailable:
      status = ICQ_STATUS_OFFLINE;
      break;
  }

  return status;
}

gloox::Presence Client::statusToPresence(unsigned long status)
{
  gloox::Presence presence;

  switch (status & ~ICQ_STATUS_FxFLAGS)
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
    case ICQ_STATUS_OFFLINE:
      presence = gloox::PresenceUnavailable;
      break;
    default:
      presence = gloox::PresenceUnknown;
  }

  return presence;
}
