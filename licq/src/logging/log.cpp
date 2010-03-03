/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007, 2010 Erik Johansson <erijo@licq.org>
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

#include "log.h"

#include <cstdio>
#include <sys/time.h>

static inline std::string vaToString(const char* format, va_list args)
{
  const size_t size = 2046;
  char msg[size];

  ::vsnprintf(msg, size, format, args);
  msg[size - 1] = '\0';

  return std::string(msg);
}

void Licq::Log::log(Licq::Log::Level level, const char* format, va_list args)
{
  log(level, vaToString(format, args));
}

void Licq::Log::packet(Licq::Log::Level level, const uint8_t* data,
                       size_t size, const char* format, va_list args)
{
  packet(level, data, size, vaToString(format, args));
}

using Licq::LogSink;
using namespace LicqDaemon;

Log::Log(const std::string& owner, LogSink& sink) :
  myOwner(owner),
  mySink(sink)
{
  // Empty
}

void Log::log(Level level, const std::string& msg)
{
  if (!mySink.isLogging(level))
    return;

  LogSink::Message* message = new LogSink::Message();
  fill(*message, level, msg);

  mySink.log(LogSink::Message::Ptr(message));
}

void Log::packet(Level level, const uint8_t* data, size_t size,
                 const std::string& msg)
{
  if (!mySink.isLogging(level))
    return;

  LogSink::Message* message = new LogSink::Message();
  fill(*message, level, msg);

  if (mySink.isLoggingPackets())
  {
    message->packet.reserve(size);
    message->packet.assign(data, data + size);
  }

  mySink.log(LogSink::Message::Ptr(message));
}

void Log::fill(LogSink::Message& message, Level level, const std::string& text)
{
  timeval tv;
  ::gettimeofday(&tv, NULL);
  message.time.sec = tv.tv_sec;
  message.time.msec = tv.tv_usec / 1000;

  message.level = level;
  message.sender = myOwner;
  message.text = text;
}
