/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#ifndef LICQ_CONVERSATION
#define LICQ_CONVERSATION

#include <boost/noncopyable.hpp>
#include <vector>

namespace Licq
{
class UserId;

typedef std::vector<UserId> ConversationUsers;

/**
 * Hold data for an active conversation
 */
class Conversation : private boost::noncopyable
{
public:
  /**
   * Get id of this conversation
   */
  virtual int id() const = 0;

  /**
   * Get owner that has this conversation
   */
  virtual UserId ownerId() const = 0;

  /**
   * Check if a user is member of this conversation
   *
   * @param userId User to check
   * @return True if user is in this conversation
   */
  virtual bool hasUser(const UserId& userId) const = 0;

  /**
   * Get number of users in conversation
   */
  virtual int numUsers() const = 0;

  /**
   * Convenience function to check if conversation is empty
   */
  bool isEmpty() const
  { return (numUsers() == 0); }

  /**
   * Get list of users in this conversation
   *
   * @param ret Vector to return users in
   */
  virtual void getUsers(ConversationUsers& ret) const = 0;

  /**
   * Get socket associated with this conversation
   */
  virtual int socketId() const = 0;

  /**
   * Add a user to the conversation
   * Note: This should only be called from the protocol plugin owning this conversation
   *
   * @param userId User to add
   * @return True if user was added or false if user was already in conversation
   */
  virtual bool addUser(const UserId& userId) = 0;

  /**
   * Remove a user from the conversation
   * Note: This should only be called from the protocol plugin owning this conversation
   *
   * @param userId User to remove
   * @return True if user was remove or false if user was not in conversation
   */
  virtual bool removeUser(const UserId& userId) = 0;

protected:
  virtual ~Conversation() { /* Empty */ }
};

/**
 * Manager to keep track of active conversations
 */
class ConversationManager : private boost::noncopyable
{
public:
  /**
   * Register a new conversation with the manager
   * Note: This should only be called from protocol plugins
   *
   * @param ownerId Owner for the new conversation
   * @param socketId Socket associated with this conversation
   * @return The conversation just created
   */
  virtual Conversation* add(const UserId& ownerId, int socketId) = 0;

  /**
   * Drop an existing conversation
   * Note: This should only be called from the protocol plugin owning the conversation
   *
   * @param convoId Conversation to remove
   * @return True if conversation was removed, false if conversation not found
   */
  virtual bool remove(int convoId) = 0;

  /**
   * Get a conversation
   *
   * @param convoId Conversation to fetch
   * @return Conversation if found, otherwise NULL
   */
  virtual Conversation* get(int convoId) = 0;

  /**
   * Get a conversation for a socket
   *
   * @param convoId Socket to get conversation for
   * @return Conversation if found, otherwise NULL
   */
  virtual Conversation* getFromSocket(int socketId) = 0;

protected:
  virtual ~ConversationManager() { /* Empty */ }
};

extern ConversationManager& gConvoManager;

} // namespace Licq

#endif
