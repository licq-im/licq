/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010,2012 Licq Developers <licq-dev@googlegroups.com>
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
using ::testing::Pointee;
using ::testing::Return;
using ::testing::StrictMock;

using namespace LicqDaemon;

namespace LicqTest {

class LogFixture : public ::testing::TestWithParam<Licq::Log::Level>
{
public:
  MockLogSink myLogSink;
  Log myLog;

  LogFixture() :
    myLog("test", myLogSink)
  {
    EXPECT_CALL(myLogSink,
                log(Pointee(AllOf(Field(&Licq::LogSink::Message::level,
                                        GetParam()),
                                  Field(&Licq::LogSink::Message::sender,
                                        "test")))));
    EXPECT_CALL(myLogSink, isLogging(GetParam()))
        .WillOnce(Return(true));
  }

  void log(const std::string& msg)
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
      case Licq::Log::Debug:
        myLog.debug(msg);
        break;
    }
  }

  void log(const char* msg)
  {
    switch (GetParam())
    {
      case Licq::Log::Unknown:
        myLog.unknown("%s", msg);
        break;
      case Licq::Log::Info:
        myLog.info("%s", msg);
        break;
      case Licq::Log::Warning:
        myLog.warning("%s", msg);
        break;
      case Licq::Log::Error:
        myLog.error("%s", msg);
        break;
      case Licq::Log::Debug:
        myLog.debug("%s", msg);
        break;
    }
  }

  Licq::Log::Stream log()
  {
    switch (GetParam())
    {
      case Licq::Log::Unknown:
        return myLog.unknown();
        break;
      case Licq::Log::Info:
        return myLog.info();
        break;
      case Licq::Log::Warning:
        return myLog.warning();
        break;
      case Licq::Log::Error:
        return myLog.error();
        break;
      case Licq::Log::Debug:
        return myLog.debug();
        break;
    }
    return myLog.log(static_cast<Licq::Log::Level>(-1));
  }
};

TEST_P(LogFixture, correctLevelWithStdStringLog)
{
  std::string msg("message");
  log(msg);
}

TEST_P(LogFixture, correctLevelWithPrintfFormat)
{
  const char* msg = "message";
  log(msg);
}

TEST_P(LogFixture, correctLevelWithStream)
{
  log() << "message";
}

INSTANTIATE_TEST_CASE_P(logLevels, LogFixture,
                        ::testing::Values(Licq::Log::Unknown,
                                          Licq::Log::Info,
                                          Licq::Log::Warning,
                                          Licq::Log::Error,
                                          Licq::Log::Debug));

TEST(Log, shouldNotLogIfIsLoggingReturnsFalse)
{
  StrictMock<MockLogSink> logSink;
  EXPECT_CALL(logSink, isLogging(Licq::Log::Info))
      .WillRepeatedly(Return(false));

  Log log("test", logSink);
  log.info("foobar");
  log.packet(Licq::Log::Info, 0, 0, "foobar");
}

TEST(Log, packet)
{
  const uint8_t packet[] = { 1, 2, 3, 4 };

  StrictMock<MockLogSink> logSink;
  EXPECT_CALL(logSink, isLogging(Licq::Log::Info))
      .WillOnce(Return(true));
  EXPECT_CALL(logSink, isLoggingPackets())
      .WillOnce(Return(true));
  EXPECT_CALL(logSink, log(Pointee(Field(&Licq::LogSink::Message::packet,
                                         ElementsAreArray(packet)))))
      .Times(1);

  Log log("test", logSink);
  log.packet(Log::Info, packet, sizeof(packet), std::string("message"));
}

TEST(Log, streamPacket)
{
  const uint8_t packet[] = { 1, 2, 3, 4 };

  StrictMock<MockLogSink> logSink;
  EXPECT_CALL(logSink, isLogging(Licq::Log::Info))
      .WillOnce(Return(true));
  EXPECT_CALL(logSink, isLoggingPackets())
      .WillOnce(Return(true));
  EXPECT_CALL(logSink, log(Pointee(Field(&Licq::LogSink::Message::packet,
                                         ElementsAreArray(packet)))))
      .Times(1);

  Log log("test", logSink);
  log.packet(Log::Info, packet, sizeof(packet)) << std::string("message");
}

TEST(Log, streamFormatting)
{
  StrictMock<MockLogSink> logSink;
  EXPECT_CALL(logSink, isLogging(Licq::Log::Info))
      .WillOnce(Return(true));
  EXPECT_CALL(logSink, log(Pointee(Field(&Licq::LogSink::Message::text,
                                         "255 = 0xff"))));

  Log log("test", logSink);
  log(Log::Info) << 255 << " = " << std::showbase << std::hex << 255;
}

TEST(Log, streamCopyConstructor)
{
  StrictMock<MockLogSink> logSink;
  EXPECT_CALL(logSink, isLogging(Licq::Log::Debug))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(logSink, log(Pointee(Field(&Licq::LogSink::Message::text,
                                         "s1 s2"))));
  EXPECT_CALL(logSink, log(Pointee(Field(&Licq::LogSink::Message::text,
                                         "s1"))));

  Log log("test", logSink);
  {
    Licq::Log::Stream s1(log.debug());
    s1 << "s1";

    Licq::Log::Stream s2(s1);
    s2 << " s2";
  }
}

struct Custom
{
  int myValue;
};

static std::ostream& operator<<(std::ostream& os, const Custom& c)
{
  os << c.myValue;
  return os;
}

TEST(Log, streamCustomDataType)
{
  StrictMock<MockLogSink> logSink;
  EXPECT_CALL(logSink, isLogging(Licq::Log::Error))
      .WillOnce(Return(true));
  EXPECT_CALL(logSink, log(Pointee(Field(&Licq::LogSink::Message::text,
                                         "256 = 0x100"))));

  Log log("test", logSink);
  Custom c; c.myValue = 256;
  log.log(Log::Error) << c << " = " << std::showbase << std::hex << c;
}

} // namespace LicqTest
