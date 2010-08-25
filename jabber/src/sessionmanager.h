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

#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <gloox/messagehandler.h>
#include <gloox/messagesessionhandler.h>
#include <map>

namespace gloox
{
class Client;
}

class Handler;

class SessionManager : public gloox::MessageSessionHandler,
                       public gloox::MessageHandler
{
public:
  SessionManager(gloox::Client& client, Handler& handler);
  ~SessionManager();

  void sendMessage(const std::string& user, const std::string& message,
      bool urgent);

  // gloox::MessageSessionHandler
  void handleMessageSession(gloox::MessageSession* session);

  // gloox::MessageHandler
  void handleMessage(const gloox::Message& message,
                     gloox::MessageSession* session);

private:
  gloox::Client& myClient;
  Handler& myHandler;

  typedef std::map<std::string, gloox::MessageSession*> Sessions;
  Sessions mySessions;
};

#endif
