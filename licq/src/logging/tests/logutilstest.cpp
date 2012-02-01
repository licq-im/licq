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

#include <licq/logging/logutils.h>

#include <gtest/gtest.h>
#include <sstream>

using Licq::LogSink;
using namespace Licq::LogUtils;

namespace LicqTest {

TEST(LogUtils, emptyPacketToString)
{
  LogSink::Message::Ptr message(new LogSink::Message);
  EXPECT_EQ(std::string(), packetToString(message));
}

template<size_t N>
static void checkPacketString(const uint8_t (&data)[N],
                              const std::string& result)
{
  boost::shared_ptr<LogSink::Message> message(new LogSink::Message);
  message->packet.assign(data, data + sizeof(data));
  EXPECT_EQ(result, packetToString(message));
}

TEST(LogUtils, shortPacketToString)
{
  const uint8_t data[] = { 'a', 0, 'b', 1, 'c' };
  const std::string result =
      "     0000: 61 00 62 01 63                                     " +
      std::string("a.b.c");
  checkPacketString(data, result);
}

TEST(LogUtils, longPacketToString)
{
  uint8_t data[31];
  for (size_t i = 0; i < sizeof(data); ++i)
    data[i] = 'a' + i * 2;

  const std::string result =
      "     0000: 61 63 65 67 69 6B 6D 6F  71 73 75 77 79 7B 7D 7F   " +
      std::string("acegikmoqsuwy{}.\n") +
      "     0010: 81 83 85 87 89 8B 8D 8F  91 93 95 97 99 9B 9D      " +
      std::string(15, '.');
  checkPacketString(data, result);
}

TEST(LogUtils, tooLongPacketToString)
{
  uint8_t data[512 * 16 + 20];
  ::memset(data, 0, sizeof(data));

  std::ostringstream result;
  for (size_t line = 0; line < 512; ++line)
  {
    result << std::string(5, ' ')
           << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
           << line * 16 << ": "
           << "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00   "
           << std::string(16, '.')
           << "\n";
  }
  result << "     2000 - 2013: ...";
  checkPacketString(data, result.str());
}

} // namespace LicqTest
