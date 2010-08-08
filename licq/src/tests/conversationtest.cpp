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

#include "../conversation.h"

#include <gtest/gtest.h>

using namespace std;
using Licq::UserId;
using LicqDaemon::Conversation;
using LicqDaemon::gConvoManager;

// Dummy normalizer so Licq::UserId becomes usable even though it isn't currently tested
string Licq::UserId::normalizeId(const string& accountId, unsigned long /* ppid */)
{
  return accountId;
}

TEST(Conversation, constructor)
{
  UserId ownerId("owner", 0x54657374);
  Conversation convo(5, ownerId, 8);

  EXPECT_EQ(5, convo.id());
  EXPECT_TRUE(ownerId == convo.ownerId());
  EXPECT_EQ(8, convo.socketId());
  EXPECT_EQ(0, convo.numUsers());
  EXPECT_TRUE(convo.isEmpty());
}

TEST(Conversation, userList)
{
  UserId ownerId("owner", 0x54657374);
  Conversation convo(5, ownerId, 8);
  Licq::ConversationUsers users;

  // Convo should have no users when just constructed
  convo.getUsers(users);
  EXPECT_TRUE(users.empty());
  users.clear();

  // Try adding users
  EXPECT_TRUE(convo.addUser(UserId("uno", 0x54657374)));
  EXPECT_TRUE(convo.addUser(UserId("second", 0x54657374)));
  EXPECT_TRUE(convo.addUser(UserId("user3", 0x54657374)));
  convo.getUsers(users);
  EXPECT_EQ(3u, users.size());
  users.clear();

  // Adding duplicate should fail
  EXPECT_FALSE(convo.addUser(UserId("second", 0x54657374)));
  convo.getUsers(users);
  EXPECT_EQ(3u, users.size());
  users.clear();

  // Try removing
  EXPECT_TRUE(convo.removeUser(UserId("uno", 0x54657374)));
  convo.getUsers(users);
  EXPECT_EQ(2u, users.size());
  users.clear();

  // Removing non-existent should fail
  EXPECT_FALSE(convo.removeUser(UserId("missing", 0x54657374)));
  convo.getUsers(users);
  EXPECT_EQ(2u, users.size());
  users.clear();
}

TEST(ConvoManager, convoList)
{
  UserId ownerId("owner", 0x54657374);

  // Creating two conversations, they should get unique IDs
  Licq::Conversation* convo1 = gConvoManager.add(ownerId, 5);
  int convoId1 = convo1->id();
  Licq::Conversation* convo2 = gConvoManager.add(ownerId, 8);
  int convoId2 = convo2->id();
  EXPECT_NE(convoId1, convoId2);

  const Licq::Conversation* const nullConvo = NULL;

  // Verify get functions
  EXPECT_EQ(convo1, gConvoManager.get(convoId1));
  EXPECT_EQ(nullConvo, gConvoManager.get(12345));
  EXPECT_EQ(convo2, gConvoManager.getFromSocket(8));
  EXPECT_EQ(nullConvo, gConvoManager.getFromSocket(1));

  // Verify remove
  EXPECT_FALSE(gConvoManager.remove(12345));
  EXPECT_TRUE(gConvoManager.remove(convoId1));
  EXPECT_EQ(nullConvo, gConvoManager.get(convoId1));
  EXPECT_EQ(convo2, gConvoManager.get(convoId2));

  // Cleanup
  gConvoManager.remove(convoId2);
}
