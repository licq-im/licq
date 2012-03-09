/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQJABBER_CLIENT_H
#define LICQJABBER_CLIENT_H

#include <boost/noncopyable.hpp>
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
class ConnectionTCPClient;
class RosterManager;
}

namespace LicqJabber
{

class Config;
class Handler;
class SessionManager;
class UserToVCard;

class JClient : private boost::noncopyable,
                public gloox::Client
{
public:
  JClient(const gloox::JID& jid, const std::string& password, int port=-1);
  virtual ~JClient();

protected:
  virtual bool checkStreamVersion(const std::string& version);
};

class Client : private boost::noncopyable,
               public gloox::ConnectionListener,
               public gloox::RosterListener,
               public gloox::LogHandler,
               public gloox::VCardHandler
{
public:
  Client(const Config& config, Handler& handler, const std::string& user,
      const std::string& password, const std::string& host, int port);
  virtual ~Client();

  int getSocket();
  void recv();
  void ping();

  SessionManager* getSessionManager() { return mySessionManager; }

  void setPassword(const std::string& password);
  bool connect(unsigned status);
  bool isConnected();
  void changeStatus(unsigned status, bool notifyHandler = true);
  void getVCard(const std::string& user);
  void setOwnerVCard(const UserToVCard& wrapper);
  void addUser(const std::string& user, const gloox::StringList& groupNames, bool notify);
  void changeUserGroups(const std::string& user,
                        const gloox::StringList& groups);
  void removeUser(const std::string& user);
  void renameUser(const std::string& user, const std::string& newName);
  void grantAuthorization(const std::string& user);
  void refuseAuthorization(const std::string& user);
  void requestAuthorization(const std::string& user, const std::string& msg);

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
                            gloox::Presence::PresenceType presence,
                            const std::string& msg);
  void handleSelfPresence(const gloox::RosterItem& item,
                          const std::string& resource,
                          gloox::Presence::PresenceType presence,
                          const std::string& msg);
  bool handleSubscriptionRequest(const gloox::JID& jid,
                                 const std::string& msg);
  bool handleUnsubscriptionRequest(const gloox::JID& jid,
                                   const std::string& msg);
  void handleNonrosterPresence(const gloox::Presence& presence);
  void handleRosterError(const gloox::IQ& iq);

  // gloox::LogHandler
  void handleLog(gloox::LogLevel level, gloox::LogArea area,
                 const std::string& message);

  // gloox::VCardHandler
  void handleVCard(const gloox::JID& jid, const gloox::VCard* vcard);
  void handleVCardResult(gloox::VCardHandler::VCardContext context,
                         const gloox::JID& jid,
                         gloox::StanzaError error);

private:
  Handler& myHandler;
  SessionManager* mySessionManager;
  gloox::JID myJid;
  JClient myClient;
  gloox::ConnectionTCPClient* myTcpClient;
  gloox::RosterManager* myRosterManager;
  gloox::VCardManager myVCardManager;

  bool addRosterItem(const gloox::RosterItem& item);

  unsigned presenceToStatus(gloox::Presence::PresenceType presence);
  gloox::Presence::PresenceType statusToPresence(unsigned status);
};

} // namespace LicqJabber

#endif
