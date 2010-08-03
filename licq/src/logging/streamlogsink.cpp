/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007, 2010 Licq developers
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

#include "streamlogsink.h"

#include <licq/logging/logutils.h>

#include <iomanip>
#include <sstream>
#include <time.h>

using Licq::Log;
using namespace Licq::LogUtils;
using namespace LicqDaemon;

// Ascii escape char
const char Esc = 0x1B;

// ANSI color codes
// http://en.wikipedia.org/wiki/ANSI_escape_code
enum ForgroundColors
{
  ForgroundBlack = 30,
  ForgroundRed,
  ForgroundGreen,
  ForgroundYellow,
  ForgroundBlue,
  ForgroundMagenta,
  ForgroundCyan,
  ForgroundWhite,
  // 38 is reserved
  ForgroundDefault = 39
};

static std::string getPrefixColor()
{
  std::ostringstream color;
  color << Esc << '[' << ForgroundGreen << 'm';
  return color.str();
}

static std::string getColor(Log::Level level)
{
  std::ostringstream color;
  color << Esc << '[';

  switch (level)
  {
    case Log::Unknown:
      color << ForgroundMagenta;
      break;
    case Log::Info:
      color << ForgroundDefault;
      break;
    case Log::Warning:
      color << ForgroundYellow;
      break;
    case Log::Error:
      color << ForgroundRed;
      break;
    case Log::Debug:
      color << ForgroundGreen;
      break;
  }

  color << "m";
  return color.str();
}

static inline std::string getResetColor()
{
  static const char reset[] = { Esc, '[', '0', 'm', '\0' };
  return reset;
}

/**
 * Print @a tv to @a os as hh:mm:ss.xxx.
 */
static inline std::ostream& operator<<(
    std::ostream& os, const Licq::LogSink::Message::Time& msgTime)
{
  return os << timeToString(msgTime);
}

StreamLogSink::StreamLogSink(std::ostream& stream) :
  myStream(stream),
  myUseColors(true)
{
  // Empty
}

void StreamLogSink::log(Message::Ptr message)
{
  std::string color;

  if (myUseColors)
  {
    myStream << getPrefixColor();
    color = getColor(message->level);
  }

  std::ostringstream ss;
  ss << message->time << color << " [";
  ss << levelToShortString(message->level);
  ss << "] " << message->sender << ": ";

  const std::string header = ss.str();
  myStream << header;

  size_t end = message->text.find_first_of('\n');
  myStream << message->text.substr(0, end) << '\n';

  const std::string prefix = std::string(header.size() - color.size(), ' ');

  while (end != std::string::npos && end != (message->text.size() - 1))
  {
    size_t start = end + 1;
    size_t count;

    end = message->text.find_first_of('\n', start);
    if (end != std::string::npos)
      count = end - start;
    else
      count = message->text.size() - start;

    myStream << prefix << message->text.substr(start, count) << '\n';
  }

  if (isLoggingPackets() && !message->packet.empty())
    myStream << packetToString(message) << '\n';

  if (myUseColors)
    myStream << getResetColor();
}
