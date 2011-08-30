/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007, 2010-2011 Licq developers
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

#include "../macro.h"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <cstdarg>
#include <iosfwd>
#include <stdint.h>
#include <string>


namespace Licq
{

class Log : private boost::noncopyable
{
public:
  typedef boost::shared_ptr<Log> Ptr;

  enum Level
  {
    Unknown     = 1, /// Unknown packets or bytes.
    Info        = 2, /// Basic information about what's going on.
    Warning     = 3, /// Warnings which are not critical but could be important.
    Error       = 4, /// Critical errors which should be brought to the attention of the user.
    Debug       = 5, /// Debugging aid.
  };

  class Stream;
  Stream operator()(Level level) { return log(level); }

  virtual void log(Level level, const std::string& msg) = 0;
  void log(Level level, const char* format, va_list args) LICQ_FORMAT(3, 0);
  Stream log(Level level) { return Stream(this, level, 0, 0); }

  void unknown(const std::string& msg) { log(Unknown, msg); }
  inline void unknown(const char* format, ...) LICQ_FORMAT(2, 3);
  Stream unknown() { return log(Unknown); }

  void info(const std::string& msg) { log(Info, msg); }
  inline void info(const char* format, ...) LICQ_FORMAT(2, 3);
  Stream info() { return log(Info); }

  void warning(const std::string& msg) { log(Warning, msg); }
  inline void warning(const char* format, ...) LICQ_FORMAT(2, 3);
  Stream warning() { return log(Warning); }

  void error(const std::string& msg) { log(Error, msg); }
  inline void error(const char* format, ...) LICQ_FORMAT(2, 3);
  Stream error() { return log(Error); }

  void debug(const std::string& msg) { log(Debug, msg); }
  inline void debug(const char* format, ...) LICQ_FORMAT(2, 3);
  Stream debug() { return log(Debug); }

  virtual void packet(Level level, const uint8_t* data, size_t size,
                      const std::string& msg) = 0;
  void packet(Level level, const uint8_t* data, size_t size,
              const char* format, va_list args) LICQ_FORMAT(5, 0);
  inline void packet(Level level, const uint8_t* data, size_t size,
                     const char* format, ...) LICQ_FORMAT(5, 6);
  Stream packet(Level level, const uint8_t* data, size_t size)
      { return Stream(this, level, data, size); }

  class Stream
  {
  public:
    friend class Log;

    Stream(const Stream& other);
    ~Stream();

    Stream& operator=(const Stream& other);

    /// Automatic conversion to std::ostream
    operator std::ostream&();

    template<typename T>
    Stream& operator<<(const T& msg) { *myStream << msg; return *this; }

  private:
    Stream(Log* log, Level level, const uint8_t* data, size_t size);

    Log* myLog;
    Level myLevel;
    const uint8_t* myData;
    size_t mySize;
    std::ostringstream* myStream;
  };

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

inline void Log::packet(Level level, const uint8_t* data, size_t size,
                        const char* format, ...)
{
  va_list args;
  va_start(args, format);
  packet(level, data, size, format, args);
  va_end(args);
}

class ThreadLog : public Log
{
private:
  Log* getLog();

public:
  // From Log
  inline void log(Level level, const std::string& msg);
  inline void packet(Level level, const uint8_t* data, size_t size,
                     const std::string& msg);

  // Bring in the other variants
  using Log::log;
  using Log::packet;
};

inline void ThreadLog::log(Level level, const std::string& msg)
{
  getLog()->log(level, msg);
}

inline void ThreadLog::packet(Level level, const uint8_t* data,
                              size_t size, const std::string& msg)
{
  getLog()->packet(level, data, size, msg);
}

/**
 * The global log.
 *
 * By default this is the global log, but threads can change it to a thread
 * specific log by calling LogService::createThreadLog().
 */
extern ThreadLog gLog;

} // namespace Licq

#endif
