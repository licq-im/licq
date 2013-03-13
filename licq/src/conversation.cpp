/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "conversation.h"

#include <boost/foreach.hpp>

#include <licq/thread/mutexlocker.h>

using namespace LicqDaemon;
using Licq::MutexLocker;
using Licq::UserId;

// Declare global ConversationManager (internal for daemon)
LicqDaemon::ConversationManager LicqDaemon::gConvoManager;

// Initialize global Licq::ConversationManager to refer to the internal ConversationManager
Licq::ConversationManager& Licq::gConvoManager(LicqDaemon::gConvoManager);


Conversation::Conversation(int id, const UserId& ownerId, int socketId)
  : myId(id),
    myOwnerId(ownerId),
    mySocketId(socketId)
{
  // Empty
}

Conversation::~Conversation()
{
  // Empty
}

int Conversation::id() const
{
  return myId;
}

UserId Conversation::ownerId() const
{
  MutexLocker lock(myMutex);
  return myOwnerId;
}

bool Conversation::hasUser(const UserId& userId) const
{
  MutexLocker lock(myMutex);
  BOOST_FOREACH(const UserId& i, myUsers)
  {
    if (i == userId)
      return true;
  }
  return false;
}

int Conversation::numUsers() const
{
  MutexLocker lock(myMutex);
  return myUsers.size();
}

void Conversation::getUsers(Licq::ConversationUsers& ret) const
{
  MutexLocker lock(myMutex);
  BOOST_FOREACH(const UserId& i, myUsers)
    ret.push_back(i);
}

int Conversation::socketId() const
{
  return mySocketId;
}

bool Conversation::addUser(const UserId& userId)
{
  MutexLocker lock(myMutex);

  BOOST_FOREACH(const UserId& i, myUsers)
  {
    if (i == userId)
      return false;
  }

  myUsers.push_back(userId);
  return true;
}

bool Conversation::removeUser(const UserId& userId)
{
  MutexLocker lock(myMutex);
  std::list<Licq::UserId>::iterator i;
  for (i = myUsers.begin(); i != myUsers.end(); ++i)
    if (*i == userId)
    {
      myUsers.erase(i);
      return true;
    }
  return false;
}

ConversationManager::ConversationManager()
{
  myLastConvoId = 0;
}

ConversationManager::~ConversationManager()
{
  // Empty
}

Licq::Conversation* ConversationManager::add(const UserId& ownerId, int socketId)
{
  MutexLocker lock(myMutex);

  // Find next free conversation id
  do
  {
    ++myLastConvoId;
    if (myLastConvoId <= 0)
      myLastConvoId = 1;
  }
  while (myConversations.count(myLastConvoId) > 0);

  Conversation* convo = new Conversation(myLastConvoId, ownerId, socketId);
  myConversations[myLastConvoId] = convo;
  return convo;
}

bool ConversationManager::remove(int convoId)
{
  MutexLocker lock(myMutex);
  std::map<int, Conversation*>::iterator i = myConversations.find(convoId);
  if (i == myConversations.end())
    return false;
  delete i->second;
  myConversations.erase(i);
  return true;
}

Licq::Conversation* ConversationManager::get(int convoId)
{
  MutexLocker lock(myMutex);
  std::map<int, Conversation*>::iterator i = myConversations.find(convoId);
  if (i != myConversations.end())
    return i->second;
  return NULL;
}

Licq::Conversation* ConversationManager::getFromSocket(int socketId)
{
  MutexLocker lock(myMutex);
  std::map<int, Conversation*>::iterator i;
  for (i = myConversations.begin(); i != myConversations.end(); ++i)
    if (i->second->socketId() == socketId)
      return i->second;
  return NULL;
}

Licq::Conversation* ConversationManager::getFromUser(const UserId& userId)
{
  MutexLocker lock(myMutex);
  std::map<int, Conversation*>::iterator i;
  for (i = myConversations.begin(); i != myConversations.end(); ++i)
    if (i->second->hasUser(userId))
      return i->second;
  return NULL;
}
