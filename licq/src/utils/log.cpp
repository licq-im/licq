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

#include <sys/time.h>

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

  LogSink::Message message;
  fill(message, level, msg);

  mySink.log(message);
}

void Log::packet(const std::string& msg, const uint8_t* data, size_t size)
{
  if (!mySink.isLoggingPackets())
    return;

  LogSink::Packet packet;
  fill(packet.message, Debug, msg);

  packet.data.reserve(size);
  packet.data.assign(data, data + size);

  mySink.logPacket(packet);
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
