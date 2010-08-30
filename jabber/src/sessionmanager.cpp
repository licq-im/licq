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

#include <cstring>
#include <ctime>
#include <gloox/attention.h>
#include <gloox/chatstate.h>
#include <gloox/chatstatefilter.h>
#include <gloox/client.h>
#include <gloox/delayeddelivery.h>
#include <gloox/disco.h>
#include <gloox/message.h>
#include <licq/logging/log.h>

#define TRACE() gLog.debug("In SessionManager::%s()", __func__)

using namespace Jabber;

using Licq::gLog;

static time_t utcTmToTime(struct tm* tm)
{
#ifdef HAVE_TIMEGM
  return ::timegm(tm);
#else
  // Set timezone to UTC
  char* tz = ::getenv("TZ");
  ::setenv("TZ", "", 1);
  ::tzset();

  // Convert to time_t
  time_t time = ::mktime(tm);

  // Reset timezone
  if (tz)
    ::setenv("TZ", tz, 1);
  else
    ::unsetenv("TZ");
  ::tzset();

  return time;
#endif
}

SessionManager::SessionManager(gloox::Client& client, Handler& handler) :
  myClient(client),
  myHandler(handler)
{
  // FIXME: The feature should only be announced if the user has activated it.
  myClient.disco()->addFeature(gloox::XMLNS_ATTENTION);
  myClient.registerStanzaExtension(new gloox::Attention);

  myClient.disco()->addFeature(gloox::XMLNS_CHAT_STATES);
  myClient.registerStanzaExtension(
      new gloox::ChatState(gloox::ChatStateInvalid));

  myClient.registerStanzaExtension(new gloox::DelayedDelivery);
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

  findSession(user).mySession->send(message, gloox::EmptyString, extensions);
}

void SessionManager::notifyTyping(const std::string& user, bool active)
{
  const gloox::ChatStateType state =
      active ? gloox::ChatStateComposing : gloox::ChatStatePaused;
  findSession(user).myChatStateFilter->setChatState(state);
}

void SessionManager::handleMessageSession(gloox::MessageSession* session)
{
  const gloox::JID& jid = session->target();
  gLog.debug("Creating new message session for %s", jid.full().c_str());

  Sessions::iterator it = mySessions.find(jid.bare());
  if (it != mySessions.end())
  {
    gLog.debug("Disposing old message session for %s",
               it->second.mySession->target().full().c_str());
    myClient.disposeMessageSession(it->second.mySession);
    mySessions.erase(it);
  }

  session->registerMessageHandler(this);

  gloox::ChatStateFilter* filter = new gloox::ChatStateFilter(session);
  filter->registerChatStateHandler(this);

  Session newSession;
  newSession.mySession = session;
  newSession.myChatStateFilter = filter;
  mySessions[jid.bare()] = newSession;
}

void SessionManager::handleMessage(
    const gloox::Message& message, gloox::MessageSession* /*session*/)
{
  const bool urgent = message.findExtension(gloox::ExtAttention) != NULL;

  time_t sent = ::time(NULL);
  if (const gloox::DelayedDelivery* delayed
      = message.findExtension<gloox::DelayedDelivery>(gloox::ExtDelay))
  {
    struct tm tm;
    ::memset(&tm, 0, sizeof(tm));

    // Only support XEP-0203 date format, not obsolete XEP-0091
    if (::strptime(delayed->stamp().c_str(), "%Y-%m-%dT%H:%M:%S", &tm) != NULL)
      sent = utcTmToTime(&tm);
  }

  if (!message.body().empty())
    myHandler.onMessage(message.from().bare(), message.body(), sent, urgent);
  else if (urgent)
    myHandler.onMessage(message.from().bare(), "buzz", sent, urgent);
}

void SessionManager::handleChatState(
    const gloox::JID& from, gloox::ChatStateType state)
{
  bool active = state == gloox::ChatStateComposing;
  myHandler.onNotifyTyping(from.bare(), active);
}

SessionManager::Session& SessionManager::findSession(const std::string& user)
{
  Sessions::iterator it = mySessions.find(user);
  if (it == mySessions.end())
  {
    handleMessageSession(new gloox::MessageSession(&myClient, user));
    it = mySessions.find(user);
    assert(it != mySessions.end());
  }

  return it->second;
}
