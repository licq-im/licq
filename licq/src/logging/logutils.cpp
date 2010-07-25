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

#include <licq/logutils.h>

#include <cstdio>
#include <sstream>

using namespace Licq;

bool LogUtils::levelInBitmask(Log::Level level, unsigned int bitmask)
{
  return bitmask & (1 << level);
}

bool LogUtils::packetInBitmask(unsigned int bitmask)
{
  return bitmask & 0x1000;
}

const char* LogUtils::levelToString(Log::Level level)
{
  switch (level)
  {
    case Log::Unknown:
      return "unknown";
    case Log::Info:
      return "info";
    case Log::Warning:
      return "warning";
    case Log::Error:
      return "error";
    case Log::Debug:
      return "debug";
  }
  return "?????";
}

const char* LogUtils::levelToShortString(Log::Level level)
{
  switch (level)
  {
    case Log::Unknown:
      return "UNK";
    case Log::Info:
      return "INF";
    case Log::Warning:
      return "WAR";
    case Log::Error:
      return "ERR";
    case Log::Debug:
      return "DBG";
  }
  return "???";
}

std::string LogUtils::timeToString(const LogSink::Message::Time& msgTime)
{
  time_t t = msgTime.sec;
  tm time;
  ::localtime_r(&t, &time);

  char buffer[8 + 4 + 1];
  ::strftime(buffer, 8 + 1, "%H:%M:%S", &time);
  ::snprintf(&buffer[8], 4 + 1, ".%03u", msgTime.msec);

  return buffer;
}

std::string LogUtils::packetToString(LogSink::Message::Ptr message)
{
  std::ostringstream ss;
  packetToString(ss, *message);
  return ss.str();
}
