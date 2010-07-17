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

#include <licq/pluginlogsink.h>

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
  EXPECT_FALSE(sink.isLoggingPackets());
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
}

static ssize_t charsInPipe(int fd)
{
  ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

  char res[10];
  return ::read(fd, &res, 10);
}

TEST(PluginLogSink, readPipeActiveWhenNewMessages)
{
  PluginLogSink sink;
  sink.log(LogSink::Message::Ptr(new LogSink::Message()));
  sink.log(LogSink::Message::Ptr(new LogSink::Message()));

  ASSERT_EQ(2, charsInPipe(sink.getReadPipe()));
}

TEST(PluginLogSink, readPipeInPopMessage)
{
  PluginLogSink sink;
  sink.log(LogSink::Message::Ptr(new LogSink::Message()));
  sink.log(LogSink::Message::Ptr(new LogSink::Message()));
  sink.log(LogSink::Message::Ptr(new LogSink::Message()));

  sink.popMessage();
  sink.popMessage(true);
  sink.popMessage(false);

  ASSERT_EQ(1, charsInPipe(sink.getReadPipe()));
}

TEST(PluginLogSink, popWorksWhenEmpty)
{
  PluginLogSink sink;

  EXPECT_FALSE(sink.popMessage(false));
  EXPECT_FALSE(sink.popMessage(true));
}

TEST(PluginLogSink, logMessage)
{
  PluginLogSink sink;

  LogSink::Message* message = new LogSink::Message();
  message->text = "test";
  sink.log(LogSink::Message::Ptr(message));
  message = NULL;

  LogSink::Message::Ptr first = sink.popMessage();
  ASSERT_TRUE(first);
  EXPECT_EQ("test", first->text);

  EXPECT_FALSE(sink.popMessage());
}
