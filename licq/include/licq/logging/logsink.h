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
  virtual bool isLogging(Log::Level level) const = 0;

  /**
   * @return True if the sink is interested in logging the raw packet data, in
   * which case Message::packet will contain the packet.
   */
  virtual bool isLoggingPackets() const = 0;

  /**
   * Called every time a new log messages is generated. But only if the sink
   * isLogging() the message's log level.
   */
  virtual void log(Message::Ptr message) = 0;

protected:
  virtual ~LogSink() { /* Empty */ }
};

/**
 * A LogSink that can be configured what to log in runtime.
 */
class AdjustableLogSink : public LogSink
{
public:
  typedef boost::shared_ptr<AdjustableLogSink> Ptr;

  /**
   * Enable or disable logging for a given log level.
   * @param level The log level to enable or disable.
   * @param enable True to enable logging; false to disable.
   */
  virtual void setLogLevel(Log::Level level, bool enable) = 0;

  /**
   * Enable or disable logging of packets.
   * @param enable True to enable logging of packets; false to disable.
   */
  virtual void setLogPackets(bool enable) = 0;

  /**
   * Enable or disable all log levels (not packets).
   * @param enable True to enable logging of all log levels; false to disable.
   */
  virtual void setAllLogLevels(bool enable) = 0;

  /**
   * Enable or disable all log levels (including packets).
   */
  virtual void setLogLevelsFromBitmask(unsigned int levels) = 0;

  /**
   * @return A bitmask indicating which log levels (including packets) that are
   * logged. Can be bitwise and:ed or or:ed with other bitmasks and passed to
   * setLogLevelsFromBitmask().
   */
  virtual unsigned int getLogLevelsBitmask() const = 0;

protected:
  virtual ~AdjustableLogSink() { /* Empty */ }
};

} // namespace Licq

#endif
