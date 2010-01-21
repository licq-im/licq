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

#ifndef CLIENT_H
#define CLIENT_H

#include <gloox/client.h>
#include <gloox/connectionlistener.h>
#include <gloox/loghandler.h>
#include <gloox/messagehandler.h>
#include <gloox/rosterlistener.h>
#include <gloox/vcardmanager.h>
#include <gloox/vcardhandler.h>

namespace gloox
{
class Client;
class RosterManager;
}

class Handler;

class Client : public gloox::ConnectionListener,
               public gloox::RosterListener,
               public gloox::MessageHandler,
               public gloox::LogHandler,
               public gloox::VCardHandler
{
public:
  Client(Handler& handler, const std::string& user,
         const std::string& password);
  virtual ~Client();

  int getSocket();
  void recv();

  bool connect(unsigned long status);
  void changeStatus(unsigned long status);
  void sendMessage(const std::string& user, const std::string& message);
  void getVCard(const std::string& user);
  void addUser(const std::string& user);
  void changeUserGroups(const std::string& user, const gloox::StringList& groups);

  // gloox::ConnectionListener
  void onConnect();
  bool onTLSConnect(const gloox::CertInfo& info);
  void onDisconnect(gloox::ConnectionError error);

  // gloox::RosterListener
  void handleItemAdded(const gloox::JID& jid);
  void handleItemSubscribed(const gloox::JID& jid);
  void handleItemRemoved(const gloox::JID& jid);
  void handleItemUpdated(const gloox::JID& jid);
  void handleItemUnsubscribed(const gloox::JID& jid);
  void handleRoster(const gloox::Roster& roster);
  void handleRosterPresence(const gloox::RosterItem& item,
                            const std::string& resource,
                            gloox::Presence presence,
                            const std::string& msg);
  void handleSelfPresence(const gloox::RosterItem& item,
                          const std::string& resource,
                          gloox::Presence presence,
                          const std::string& msg);
  bool handleSubscriptionRequest(const gloox::JID& jid,
                                 const std::string& msg);
  bool handleUnsubscriptionRequest(const gloox::JID& jid,
                                   const std::string& msg);
  void handleNonrosterPresence(gloox::Stanza* stanza);
  void handleRosterError(gloox::Stanza* stanza);

  // gloox::MessageHandler
  void handleMessage(gloox::Stanza* stanza, gloox::MessageSession* session);

  // gloox::LogHandler
  void handleLog(gloox::LogLevel level, gloox::LogArea area,
                 const std::string& message);

  // gloox::VCardHandler
  void handleVCard(const gloox::JID& jid, gloox::VCard* vcard);
  void handleVCardResult(gloox::VCardHandler::VCardContext context,
                         const gloox::JID& jid,
                         gloox::StanzaError error);

private:
  Handler& myHandler;
  gloox::JID myJid;
  gloox::Client myClient;
  gloox::RosterManager* myRosterManager;
  gloox::VCardManager myVCardManager;

  unsigned long presenceToStatus(gloox::Presence presence);
  gloox::Presence statusToPresence(unsigned long status);
};

#endif
