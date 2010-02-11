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

#ifndef LICQ_LOG_H
#define LICQ_LOG_H

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <cstdarg>
#include <stdint.h>
#include <string>

#ifdef __GNUC__
#define LICQ_FORMAT(x, y) __attribute__((format (printf, x, y)))
#endif

namespace Licq
{

class Log
{
public:
  typedef boost::shared_ptr<Log> Ptr;

  enum Level
  {
    /// Unknown packets or bytes.
    Unknown,

    /// Basic information about what's going on.
    Info,

    /// Warnings which are not critical but could be important.
    Warning,

    /// Critical errors which should be brought to the attention of the user.
    Error,

    /// Debugging aid.
    Debug,

    /// Packets.
    Packet
  };

  virtual void log(Level level, const std::string& msg) = 0;
  void log(Level level, const boost::format& msg) { log(level, msg.str()); }
  void log(Level level, const char* format, va_list args) LICQ_FORMAT(3, 0);

  void unknown(const std::string& msg) { log(Unknown, msg); }
  void unknown(const boost::format& msg) { log(Unknown, msg); }
  inline void unknown(const char* format, ...) LICQ_FORMAT(2, 3);

  void info(const std::string& msg) { log(Info, msg); }
  void info(const boost::format& msg) { log(Info, msg); }
  inline void info(const char* format, ...) LICQ_FORMAT(2, 3);

  void warning(const std::string& msg) { log(Warning, msg); }
  void warning(const boost::format& msg) { log(Warning, msg); }
  inline void warning(const char* format, ...) LICQ_FORMAT(2, 3);

  void error(const std::string& msg) { log(Error, msg); }
  void error(const boost::format& msg) { log(Error, msg); }
  inline void error(const char* format, ...) LICQ_FORMAT(2, 3);

  void debug(const std::string& msg) { log(Debug, msg); }
  void debug(const boost::format& msg) { log(Debug, msg); }
  inline void debug(const char* format, ...) LICQ_FORMAT(2, 3);

  virtual void packet(const std::string& msg, const uint8_t* data,
                      size_t size) = 0;
  void packet(const boost::format& msg, const uint8_t* data, size_t size)
  { packet(msg.str(), data, size); }

protected:
  virtual ~Log() { /* Empty */ }
};

inline void Log::unknown(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  log(Unknown, format, args);
  va_end(args);
}

inline void Log::info(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  log(Info, format, args);
  va_end(args);
}

inline void Log::warning(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  log(Warning, format, args);
  va_end(args);
}

inline void Log::error(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  log(Error, format, args);
  va_end(args);
}

inline void Log::debug(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  log(Debug, format, args);
  va_end(args);
}

class PluginLog
{
public:
  Log* operator->();
};

extern PluginLog pLog;

} // namespace Licq

#endif