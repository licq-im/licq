/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq Developers <licq-dev@googlegroups.com>
 *
 * Please refer to the COPYRIGHT file distributed with this source
 * distribution for the names of the individual contributors.
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
#include <gloox/message.h>
#include <gloox/rostermanager.h>

#include <licq/contactlist/user.h>
#include <licq/log.h>
#include <licq/licqversion.h>

using Licq::User;
using Licq::gLog;

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

  myClient.disco()->setIdentity("client", "pc");
  myClient.disco()->setVersion("Licq", LICQ_VERSION_STRING);

  // TODO: Fix in a more generic way
  if (myClient.server() == "chat.facebook.com")
    myClient.setTls(gloox::TLSDisabled);
}

Client::~Client()
{
  myVCardManager.cancelVCardOperations(this);
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

void Client::setPassword(const std::string& password)
{
  myClient.setPassword(password);
}

bool Client::connect(unsigned status)
{
  changeStatus(status);
  return myClient.connect(false);
}

bool Client::isConnected()
{
  return myClient.authed();
}

void Client::changeStatus(unsigned status)
{
  std::string msg = myHandler.getStatusMessage(status);
  myClient.setPresence(statusToPresence(status), 0, msg);
}

void Client::sendMessage(const std::string& user, const std::string& message)
{
  gloox::Message msg(gloox::Message::Chat, user, message);
  myClient.send(msg);
}

void Client::getVCard(const std::string& user)
{
  myVCardManager.fetchVCard(gloox::JID(user), this);
}

void Client::addUser(const std::string& user)
{
  myRosterManager->add(gloox::JID(user), user, gloox::StringList());
}

void Client::changeUserGroups(const std::string& user,
                              const gloox::StringList& groups)
{
  gloox::RosterItem* item = myRosterManager->getRosterItem(gloox::JID(user));
  if (item != NULL)
  {
    item->setGroups(groups);
    myRosterManager->synchronize();
  }
}

void Client::removeUser(const std::string& user)
{
  myRosterManager->remove(gloox::JID(user));
}

void Client::renameUser(const std::string& user, const std::string& newName)
{
  gloox::RosterItem* item = myRosterManager->getRosterItem(gloox::JID(user));
  if (item != NULL)
  {
    item->setName(newName);
    myRosterManager->synchronize();
  }
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
  gloox::RosterItem* ri = myRosterManager->getRosterItem(jid);

  myHandler.onUserAdded(jid.bare(), ri->name(), ri->groups());
}

void Client::handleItemSubscribed(const gloox::JID& /*jid*/)
{
}

void Client::handleItemRemoved(const gloox::JID& jid)
{
  myHandler.onUserRemoved(jid.bare());
}

void Client::handleItemUpdated(const gloox::JID& jid)
{
  gloox::RosterItem* ri = myRosterManager->getRosterItem(jid);

  myHandler.onUserAdded(jid.bare(), ri->name(), ri->groups());
}

void Client::handleItemUnsubscribed(const gloox::JID& /*jid*/)
{
}

void Client::handleRoster(const gloox::Roster& roster)
{
  std::set<std::string> jidlist;
  gloox::Roster::const_iterator it;

  for (it = roster.begin(); it != roster.end(); ++it)
  {
    myHandler.onUserAdded(it->first, it->second->name(), it->second->groups());
    jidlist.insert(it->first);
  }

  myHandler.onRosterReceived(jidlist);
}

void Client::handleRosterPresence(const gloox::RosterItem& item,
                                  const std::string& /*resource*/,
                                  gloox::Presence::PresenceType presence,
                                  const std::string& /*msg*/)
{
  myHandler.onUserStatusChange(gloox::JID(item.jid()).bare(),
      presenceToStatus(presence));
}

void Client::handleSelfPresence(const gloox::RosterItem& /*item*/,
                                const std::string& resource,
                                gloox::Presence::PresenceType presence,
                                const std::string& /*msg*/)
{
  if (resource == myClient.resource())
    myHandler.onChangeStatus(presenceToStatus(presence));
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

void Client::handleNonrosterPresence(const gloox::Presence& /*presence*/)
{
}

void Client::handleRosterError(const gloox::IQ& /*iq*/)
{
}

void Client::handleMessage(const gloox::Message& msg,
                           gloox::MessageSession* /*session*/)
{
  if (!msg.body().empty())
    myHandler.onMessage(msg.from().bare(), msg.body());
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
    case gloox::LogAreaClassSOCKS5Bytestream:
      areaStr = "SOCKS5 bytestream";
      break;
    case gloox::LogAreaClassConnectionBOSH:
      areaStr = "BOSH";
      break;
    case gloox::LogAreaClassConnectionTLS:
      areaStr = "TLS";
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
      gLog.info("[%s] %s", areaStr, message.c_str());
      break;
    case gloox::LogLevelWarning:
      gLog.warning("[%s] %s", areaStr, message.c_str());
      break;
    case gloox::LogLevelError:
      gLog.error("[%s] %s", areaStr, message.c_str());
      break;
  }
}

void Client::handleVCard(const gloox::JID& jid, const gloox::VCard* vcard)
{
  (void)jid;
  delete vcard;
}

void Client::handleVCardResult(gloox::VCardHandler::VCardContext context,
                               const gloox::JID& jid, gloox::StanzaError error)
{
  if (error != gloox::StanzaErrorUndefined)
  {
    gLog.warning("%s VCard for user %s failed with error %u",
        context == gloox::VCardHandler::StoreVCard ? "Storing" : "Fetching",
        jid.bare().c_str(), error);
  }
}

unsigned Client::presenceToStatus(gloox::Presence::PresenceType presence)
{
  switch (presence)
  {
    case gloox::Presence::Invalid:
    case gloox::Presence::Probe:
    case gloox::Presence::Error:
    case gloox::Presence::Unavailable:
      return User::OfflineStatus;

    case gloox::Presence::Chat:
      return User::OnlineStatus | User::FreeForChatStatus;

    case gloox::Presence::Away:
      return User::OnlineStatus | User::AwayStatus;

    case gloox::Presence::DND:
      return User::OnlineStatus | User::DoNotDisturbStatus;

    case gloox::Presence::XA:
      return User::OnlineStatus | User::NotAvailableStatus;

    case gloox::Presence::Available:
    default:
      return User::OnlineStatus;
  }
}

gloox::Presence::PresenceType Client::statusToPresence(unsigned status)
{
  if (status == User::OfflineStatus)
    return gloox::Presence::Unavailable;

  if (status & User::AwayStatus)
    return gloox::Presence::Away;

  if (status & User::NotAvailableStatus)
    return gloox::Presence::XA;

  if (status & (User::OccupiedStatus | User::DoNotDisturbStatus))
    return gloox::Presence::DND;

  if (status & User::FreeForChatStatus)
    return gloox::Presence::Chat;

  return gloox::Presence::Available;
}
