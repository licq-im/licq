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

#include "sessionmanager.h"
#include "handler.h"

#include <gloox/attention.h>
#include <gloox/client.h>
#include <gloox/message.h>
#include <licq/logging/log.h>

using Licq::gLog;

SessionManager::SessionManager(gloox::Client& client, Handler& handler) :
  myClient(client),
  myHandler(handler)
{
  // Empty
}

SessionManager::~SessionManager()
{
  // Sessions are owned by client so no need to delete any
  mySessions.clear();
}

void SessionManager::sendMessage(
    const std::string& user, const std::string& message, bool urgent)
{
  gloox::StanzaExtensionList extensions;
  if (urgent)
  {
    // TODO: Only add extension if receiver supports it
    extensions.push_back(new gloox::Attention);
  }

  Sessions::iterator it = mySessions.find(user);
  if (it == mySessions.end())
  {
    handleMessageSession(new gloox::MessageSession(&myClient, user));
    it = mySessions.find(user);
    assert(it != mySessions.end());
  }

  it->second->send(message, gloox::EmptyString, extensions);
}

void SessionManager::handleMessageSession(gloox::MessageSession* session)
{
  const gloox::JID& jid = session->target();
  gLog.debug("Creating new message session for %s", jid.full().c_str());

  Sessions::iterator it = mySessions.find(jid.bare());
  if (it != mySessions.end())
  {
    gLog.debug("Disposing old message session for %s",
               it->second->target().full().c_str());
    myClient.disposeMessageSession(it->second);
    mySessions.erase(it);
  }

  mySessions[jid.bare()] = session;
  session->registerMessageHandler(this);
}

void SessionManager::handleMessage(
    const gloox::Message& message, gloox::MessageSession* /*session*/)
{
  const bool urgent = message.findExtension(gloox::ExtAttention) != NULL;
  if (!message.body().empty())
    myHandler.onMessage(message.from().bare(), message.body(), urgent);
  else if (urgent)
    myHandler.onMessage(message.from().bare(), "buzz", urgent);
}
