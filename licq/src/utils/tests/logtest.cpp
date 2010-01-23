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

#include "../log.h"
#include "mocklogsink.h"

#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AllOf;
using ::testing::ElementsAreArray;
using ::testing::Field;
using ::testing::Return;
using ::testing::StrictMock;

using namespace LicqDaemon;

class LogFixture : public ::testing::TestWithParam<Licq::Log::Level>
{
public:
  MockLogSink myLogSink;
  Log myLog;

  LogFixture() :
    myLog("test", myLogSink)
  {
    EXPECT_CALL(myLogSink,
                log(AllOf(Field(&Licq::LogSink::Message::level,
                                GetParam()),
                          Field(&Licq::LogSink::Message::sender,
                                "test"))));
    EXPECT_CALL(myLogSink, isLogging(GetParam()))
        .WillOnce(Return(true));
  }

  template<typename T>
  void log(const T& msg)
  {
    switch (GetParam())
    {
      case Licq::Log::Unknown:
        myLog.unknown(msg);
        break;
      case Licq::Log::Info:
        myLog.info(msg);
        break;
      case Licq::Log::Warning:
        myLog.warning(msg);
        break;
      case Licq::Log::Error:
        myLog.error(msg);
        break;
      case Licq::Log::Fatal:
        myLog.fatal(msg);
        break;
      case Licq::Log::Debug:
        myLog.debug(msg);
        break;
    }
  }
};

TEST_P(LogFixture, correctLevelWithStdStringLog)
{
  std::string msg("message");
  log(msg);
}

TEST_P(LogFixture, correctLevelWithBoostFormatLog)
{
  boost::format msg("message level %1%");
  log(msg % GetParam());
}

INSTANTIATE_TEST_CASE_P(logLevels, LogFixture,
                        ::testing::Values(Licq::Log::Unknown,
                                          Licq::Log::Info,
                                          Licq::Log::Warning,
                                          Licq::Log::Error,
                                          Licq::Log::Fatal,
                                          Licq::Log::Debug));

TEST(Log, shouldNotLogIfIsLoggingReturnsFalse)
{
  StrictMock<MockLogSink> logSink;
  EXPECT_CALL(logSink, isLogging(Licq::Log::Info))
      .WillOnce(Return(false));

  Log log("test", logSink);
  log.info("foobar");
}

TEST(Log, shouldNotLogPacketIfIsLoggingPacketsReturnsFalse)
{
  StrictMock<MockLogSink> logSink;
  EXPECT_CALL(logSink, isLoggingPackets())
      .WillOnce(Return(false));

  Log log("test", logSink);
  log.packet("foobar", NULL, 0);
}

TEST(Log, packet)
{
  const uint8_t packet[] = { 1, 2, 3, 4 };

  StrictMock<MockLogSink> logSink;
  EXPECT_CALL(logSink, isLoggingPackets())
      .Times(2)
      .WillRepeatedly(Return(true));
  EXPECT_CALL(logSink, logPacket(Field(&Licq::LogSink::Packet::data,
                                       ElementsAreArray(packet))))
      .Times(2);

  Log log("test", logSink);
  log.packet(std::string("message"), packet, sizeof(packet));
  log.packet(boost::format("message %1%") % 1, packet, sizeof(packet));
}
