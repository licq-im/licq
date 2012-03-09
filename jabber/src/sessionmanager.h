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

#ifndef LICQJABBER_SESSIONMANAGER_H
#define LICQJABBER_SESSIONMANAGER_H

#include <boost/noncopyable.hpp>
#include <gloox/chatstatehandler.h>
#include <gloox/messagehandler.h>
#include <gloox/messagesessionhandler.h>
#include <map>

namespace gloox
{
class ChatStateFilter;
class Client;
}

namespace LicqJabber
{

class Handler;

class SessionManager : private boost::noncopyable,
                       public gloox::MessageSessionHandler,
                       public gloox::MessageHandler,
                       public gloox::ChatStateHandler
{
public:
  SessionManager(gloox::Client& client, Handler& handler);
  ~SessionManager();

  void sendMessage(const std::string& user, const std::string& message,
      bool urgent);
  void notifyTyping(const std::string& user, bool active);

  // gloox::MessageSessionHandler
  void handleMessageSession(gloox::MessageSession* session);

  // gloox::MessageHandler
  void handleMessage(const gloox::Message& message,
                     gloox::MessageSession* session);

  // gloox::ChatStateHandler
  void handleChatState(const gloox::JID& from, gloox::ChatStateType state);

private:
  gloox::Client& myClient;
  Handler& myHandler;

  struct Session
  {
    gloox::MessageSession* mySession;
    gloox::ChatStateFilter* myChatStateFilter;
  };

  typedef std::map<std::string, Session> Sessions;
  Sessions mySessions;

  Session& findSession(const std::string& user);
};

} // namespace LicqJabber

#endif
