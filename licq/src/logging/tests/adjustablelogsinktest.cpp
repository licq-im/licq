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

#include "../adjustablelogsink.h"

#include <licq/logging/logutils.h>
#include <gtest/gtest.h>

using namespace LicqDaemon;
using Licq::Log;

class AdjustableLogSinkTest : public AdjustableLogSink
{
public:
  void log(Message::Ptr /*message*/) { /* ignore */ }
};

TEST(AdjustableLogSink, notLoggingAnythingByDefault)
{
  AdjustableLogSinkTest sink;

  EXPECT_FALSE(sink.isLogging(Log::Unknown));
  EXPECT_FALSE(sink.isLogging(Log::Info));
  EXPECT_FALSE(sink.isLogging(Log::Warning));
  EXPECT_FALSE(sink.isLogging(Log::Error));
  EXPECT_FALSE(sink.isLogging(Log::Debug));
  EXPECT_FALSE(sink.isLoggingPackets());
}

TEST(AdjustableLogSink, setLogLevel)
{
  AdjustableLogSinkTest sink;

  sink.setLogLevel(Log::Info, true);
  sink.setLogLevel(Log::Warning, true);
  EXPECT_TRUE(sink.isLogging(Log::Info));
  EXPECT_TRUE(sink.isLogging(Log::Warning));

  sink.setLogLevel(Log::Info, false);
  EXPECT_FALSE(sink.isLogging(Log::Info));
  EXPECT_TRUE(sink.isLogging(Log::Warning));  
}

TEST(AdjustableLogSink, setAllLogLevels)
{
  AdjustableLogSinkTest sink;
  sink.setAllLogLevels(true);

  EXPECT_TRUE(sink.isLogging(Log::Unknown));
  EXPECT_TRUE(sink.isLogging(Log::Info));
  EXPECT_TRUE(sink.isLogging(Log::Warning));
  EXPECT_TRUE(sink.isLogging(Log::Error));
  EXPECT_TRUE(sink.isLogging(Log::Debug));

  // Packets should not be affected
  EXPECT_FALSE(sink.isLoggingPackets());
}

TEST(AdjustableLogSink, setLogPackets)
{
  AdjustableLogSinkTest sink;

  sink.setLogPackets(true);
  EXPECT_TRUE(sink.isLoggingPackets());
  sink.setLogPackets(false);
  EXPECT_FALSE(sink.isLoggingPackets());
}

TEST(AdjustableLogSink, oldFormat)
{
  Licq::AdjustableLogSink::Ptr sink(new AdjustableLogSinkTest());

  unsigned int mask = Licq::LogUtils::convertOldBitmaskToNew(0x10 | 0x2);
  sink->setLogLevelsFromBitmask(mask);

  EXPECT_TRUE(sink->isLogging(Log::Unknown));
  EXPECT_TRUE(sink->isLogging(Log::Debug));
  EXPECT_TRUE(sink->isLoggingPackets());

  EXPECT_FALSE(sink->isLogging(Log::Info));
  EXPECT_FALSE(sink->isLogging(Log::Warning));
  EXPECT_FALSE(sink->isLogging(Log::Error));
}

TEST(AdjustableLogSink, bitmaskWorksAsExpected)
{
  AdjustableLogSinkTest sink1, sink2;

  sink1.setLogLevel(Log::Warning, true);
  sink1.setLogLevel(Log::Error, true);

  sink2.setLogLevel(Log::Error, true);
  sink2.setLogPackets(true);

  AdjustableLogSinkTest sink;
  sink.setLogLevelsFromBitmask(
      sink1.getLogLevelsBitmask() | sink2.getLogLevelsBitmask());

  EXPECT_TRUE(sink.isLogging(Log::Warning));
  EXPECT_TRUE(sink.isLogging(Log::Error));
  EXPECT_TRUE(sink.isLoggingPackets());

  EXPECT_FALSE(sink.isLogging(Log::Unknown));
  EXPECT_FALSE(sink.isLogging(Log::Info));
  EXPECT_FALSE(sink.isLogging(Log::Debug));
}
