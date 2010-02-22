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

#include "licq/pluginlogsink.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <unistd.h>

using namespace Licq;

TEST(PluginLogSink, notLoggingAnythingByDefault)
{
  PluginLogSink sink;
  EXPECT_FALSE(sink.isLogging(Log::Unknown));
  EXPECT_FALSE(sink.isLogging(Log::Info));
  EXPECT_FALSE(sink.isLogging(Log::Warning));
  EXPECT_FALSE(sink.isLogging(Log::Error));
  EXPECT_FALSE(sink.isLogging(Log::Debug));
  EXPECT_FALSE(sink.isLogging(Log::Packet));
}

TEST(PluginLogSink, setLogLevel)
{
  PluginLogSink sink;

  sink.setLogLevel(Log::Info, true);
  sink.setLogLevel(Log::Warning, true);
  EXPECT_TRUE(sink.isLogging(Log::Info));
  EXPECT_TRUE(sink.isLogging(Log::Warning));

  sink.setLogLevel(Log::Info, false);
  EXPECT_FALSE(sink.isLogging(Log::Info));
  EXPECT_TRUE(sink.isLogging(Log::Warning));  
}

TEST(PluginLogSink, setAllLogLevels)
{
  PluginLogSink sink;
  sink.setAllLogLevels(true);

  EXPECT_TRUE(sink.isLogging(Log::Unknown));
  EXPECT_TRUE(sink.isLogging(Log::Info));
  EXPECT_TRUE(sink.isLogging(Log::Warning));
  EXPECT_TRUE(sink.isLogging(Log::Error));
  EXPECT_TRUE(sink.isLogging(Log::Debug));
  EXPECT_TRUE(sink.isLogging(Log::Packet));
}

TEST(PluginLogSink, correctTypeOnFd)
{
  PluginLogSink sink;
  sink.log(LogSink::Message());
  sink.logPacket(LogSink::Packet());
  sink.log(LogSink::Message());

  int fd = sink.getReadPipe();
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

  char res[3];
  ASSERT_EQ(3, read(sink.getReadPipe(), &res, 3));

  EXPECT_EQ(PluginLogSink::TYPE_MESSAGE, res[0]);
  EXPECT_EQ(PluginLogSink::TYPE_PACKET, res[1]);
  EXPECT_EQ(PluginLogSink::TYPE_MESSAGE, res[2]);
}

TEST(PluginLogSink, getAndPopWorksWhenEmpty)
{
  PluginLogSink sink;

  EXPECT_EQ((const LogSink::Message*)NULL, sink.getFirstMessage());
  sink.popFirstMessage();

  EXPECT_EQ((const LogSink::Packet*)NULL, sink.getFirstPacket());
  sink.popFirstPacket();
}

TEST(PluginLogSink, logMessage)
{
  PluginLogSink sink;

  LogSink::Message message;
  message.text = "test";
  sink.log(message);

  const LogSink::Message* first = sink.getFirstMessage();
  ASSERT_NE((const LogSink::Message*)NULL, first);
  EXPECT_EQ("test", first->text);

  EXPECT_EQ(first, sink.getFirstMessage());
  sink.popFirstMessage();
}

TEST(PluginLogSink, logPacket)
{
  PluginLogSink sink;

  LogSink::Packet packet;
  packet.message.text = "test";
  sink.logPacket(packet);

  const LogSink::Packet* first = sink.getFirstPacket();
  ASSERT_NE((const LogSink::Packet*)NULL, first);
  EXPECT_EQ("test", first->message.text);

  EXPECT_EQ(first, sink.getFirstPacket());
  sink.popFirstPacket();
}
