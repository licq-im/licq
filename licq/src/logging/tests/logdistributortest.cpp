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

#include "../logdistributor.h"
#include "mocklogsink.h"

#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

using Licq::Log;
using Licq::LogSink;
using namespace LicqDaemon;

class LogDistributorFixture : public ::testing::Test
{
public:
  StrictMock<MockLogSink> myMockSink1;
  StrictMock<MockLogSink> myMockSink2;

  LogSink::Ptr mySink1;
  LogSink::Ptr mySink2;

  LogDistributor distributor;

  LogDistributorFixture() :
    mySink1(&myMockSink1, &nullDeleter),
    mySink2(&myMockSink2, &nullDeleter)
  {
    // Empty
  }

  static void nullDeleter(void*)
  {
    // Empty
  }
};

TEST(LogDistributor, shouldNotLogWhenEmpty)
{
  LogDistributor distributor;
  EXPECT_FALSE(distributor.isLogging(Log::Unknown));
  EXPECT_FALSE(distributor.isLogging(Log::Info));
  EXPECT_FALSE(distributor.isLogging(Log::Warning));
  EXPECT_FALSE(distributor.isLogging(Log::Error));
  EXPECT_FALSE(distributor.isLogging(Log::Debug));
  EXPECT_FALSE(distributor.isLoggingPackets());
}

TEST_F(LogDistributorFixture, registerSinkWorks)
{
  EXPECT_CALL(myMockSink1, isLogging(_))
      .WillOnce(Return(false));

  distributor.registerSink(mySink1);

  LogSink::Message::Ptr msg(new LogSink::Message());
  distributor.log(msg);
}

TEST_F(LogDistributorFixture, sameSinkIsOnlyRegisteredOnce)
{
  EXPECT_CALL(myMockSink1, isLogging(_))
      .WillOnce(Return(false));

  distributor.registerSink(mySink1);
  distributor.registerSink(mySink1);

  LogSink::Message::Ptr msg(new LogSink::Message());
  distributor.log(msg);
}

TEST_F(LogDistributorFixture, unregisterSinkWorks)
{
  EXPECT_CALL(myMockSink2, isLogging(_))
      .WillOnce(Return(false));

  distributor.registerSink(mySink1);
  distributor.registerSink(mySink2);
  distributor.unregisterSink(mySink1);

  LogSink::Message::Ptr msg(new LogSink::Message());
  distributor.log(msg);
}

TEST_F(LogDistributorFixture, logsAreSentToAllActiveSinks)
{
  EXPECT_CALL(myMockSink1, isLogging(Log::Info))
      .WillOnce(Return(true));
  EXPECT_CALL(myMockSink1, isLogging(Log::Debug))
      .WillOnce(Return(true));
  EXPECT_CALL(myMockSink1, log(_))
      .Times(2);

  EXPECT_CALL(myMockSink2, isLogging(Log::Info))
      .WillOnce(Return(true));
  EXPECT_CALL(myMockSink2, isLogging(Log::Debug))
      .WillOnce(Return(false));
  EXPECT_CALL(myMockSink2, log(_));

  distributor.registerSink(mySink1);
  distributor.registerSink(mySink2);

  LogSink::Message* msg1 = new LogSink::Message();
  msg1->level = Log::Info;
  distributor.log(LogSink::Message::Ptr(msg1));
  msg1 = NULL;

  LogSink::Message* msg2 = new LogSink::Message();
  msg2->level = Log::Debug;
  distributor.log(LogSink::Message::Ptr(msg2));
  msg2 = NULL;
}

TEST_F(LogDistributorFixture, logPacketWorks)
{
  EXPECT_CALL(myMockSink1, isLoggingPackets())
      .WillRepeatedly(Return(true));

  EXPECT_CALL(myMockSink2, isLoggingPackets())
      .WillRepeatedly(Return(false));

  distributor.registerSink(mySink2);
  distributor.registerSink(mySink1);

  EXPECT_TRUE(distributor.isLoggingPackets());
}
