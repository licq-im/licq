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

#ifndef LICQ_LOGSINK_H
#define LICQ_LOGSINK_H

#include "log.h"

#include <boost/shared_ptr.hpp>
#include <ctime>
#include <iosfwd>
#include <string>
#include <vector>

namespace Licq
{

/**
 * A LogSink receives log messages from a Log.
 */
class LogSink
{
public:
  typedef boost::shared_ptr<LogSink> Ptr;

  struct Message
  {
    typedef boost::shared_ptr<const Message> Ptr;

    Log::Level level;
    std::string sender;
    std::string text;

    struct Time
    {
      time_t sec;
      unsigned int msec;
    } time;

    std::vector<uint8_t> packet;
  };

  /**
   * @return True if the sink is interested in log messages at @a level;
   * otherwise false.
   */
  virtual bool isLogging(Log::Level level) = 0;

  /**
   * @return True if the sink is interested in logging the raw packet data, in
   * which case Message::packet will contain the packet.
   */
  virtual bool isLoggingPackets() = 0;

  /**
   * Called every time a new log messages is generated. But only if the sink
   * isLogging() the message's log level.
   */
  virtual void log(Message::Ptr message) = 0;

protected:
  virtual ~LogSink() { /* Empty */ }
};

/**
 * Pretty-prints @a packet to the stream @a os.
 * @param os The stream to print to.
 * @param packet The packet to print.
 * @return @a os
 */
std::ostream& packetToString(
    std::ostream& os, const Licq::LogSink::Message& message);

} // namespace Licq

#endif
