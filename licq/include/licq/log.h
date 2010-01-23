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
#include <stdint.h>
#include <string>

namespace Licq
{

class Log
{
public:
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

    /// Fatal errors after which Licq is unable to continue running.
    Fatal,

    /// Debugging aid.
    Debug
  };

  virtual void log(Level level, const std::string& msg) = 0;
  void log(Level level, const boost::format& msg) { log(level, msg.str()); }

  void unknown(const std::string& msg) { log(Unknown, msg); }
  void unknown(const boost::format& msg) { log(Unknown, msg); }

  void info(const std::string& msg) { log(Info, msg); }
  void info(const boost::format& msg) { log(Info, msg); }

  void warning(const std::string& msg) { log(Warning, msg); }
  void warning(const boost::format& msg) { log(Warning, msg); }

  void error(const std::string& msg) { log(Error, msg); }
  void error(const boost::format& msg) { log(Error, msg); }

  void fatal(const std::string& msg) { log(Fatal, msg); }
  void fatal(const boost::format& msg) { log(Fatal, msg); }

  void debug(const std::string& msg) { log(Debug, msg); }
  void debug(const boost::format& msg) { log(Debug, msg); }

  virtual void packet(const std::string& msg, const uint8_t* data,
                      size_t size) = 0;
  void packet(const boost::format& msg, const uint8_t* data, size_t size)
  { packet(msg.str(), data, size); }

protected:
  virtual ~Log() { /* Empty */ }
};

} // namespace Licq

#endif
