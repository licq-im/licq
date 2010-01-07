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
#include <gloox/presencehandler.h>
#include <gloox/vcardmanager.h>
#include <gloox/vcardhandler.h>

namespace gloox
{
class Client;
}

class Handler;

class Client : public gloox::ConnectionListener,
               public gloox::PresenceHandler,
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
  void getVCard(const std::string& user);

  // gloox::ConnectionListener
  void onConnect();
  bool onTLSConnect(const gloox::CertInfo& info);
  void onDisconnect(gloox::ConnectionError error);

  // gloox::PresenceHandler
  void handlePresence(gloox::Stanza* stanza);

  // gloox::LogHandler
  void handleLog(gloox::LogLevel level, gloox::LogArea area,
                 const std::string& message);

  // gloox::VCardHandler
  void handleVCard(const gloox::JID& jid, gloox::VCard* vcard);
  void handleVCardResult(gloox::VCardHandler::VCardContext context,
                         const gloox::JID& jid,
                         gloox::StanzaError se = gloox::StanzaErrorUndefined);

private:
  Handler& myHandler;
  gloox::JID myJid;
  gloox::Client myClient;
  gloox::VCardManager myVCardManager;
};

#endif
