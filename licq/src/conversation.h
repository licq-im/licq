/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQDAEMON_CONVERSATION_H
#define LICQDAEMON_CONVERSATION_H

#include <licq/conversation.h>

#include <list>
#include <map>

#include <licq/thread/mutex.h>
#include <licq/userid.h>

namespace LicqDaemon
{

class Conversation : public Licq::Conversation
{
public:
  Conversation(int id, const Licq::UserId& ownerId, int socketId);
  ~Conversation();

  // From Licq::Conversation
  int id() const;
  Licq::UserId ownerId() const;
  bool hasUser(const Licq::UserId& userId) const;
  int numUsers() const;
  void getUsers(Licq::ConversationUsers& ret) const;
  int socketId() const;
  bool addUser(const Licq::UserId& userId);
  bool removeUser(const Licq::UserId& userId);

private:
  const int myId;
  Licq::UserId myOwnerId;
  std::list<Licq::UserId> myUsers;
  mutable Licq::Mutex myMutex;
  int mySocketId;
};

class ConversationManager : public Licq::ConversationManager
{
public:
  ConversationManager();
  ~ConversationManager();

  // From Licq::ConversationManager
  Licq::Conversation* add(const Licq::UserId& ownerId, int socketId);
  bool remove(int convoId);
  Licq::Conversation* get(int convoId);
  Licq::Conversation* getFromSocket(int socketId);
  Licq::Conversation* getFromUser(const Licq::UserId& userId);

private:
  std::map<int, Conversation*> myConversations;
  int myLastConvoId;
  Licq::Mutex myMutex;
};

extern ConversationManager gConvoManager;

} // namespace LicqDaemon

#endif
