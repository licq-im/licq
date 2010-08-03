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

#ifndef LICQDAEMON_LOG_H
#define LICQDAEMON_LOG_H

#include <licq/logging/log.h>
#include <licq/logging/logsink.h>

namespace LicqDaemon
{

class Log : public Licq::Log
{
public:
  /**
   * Creates a log that logs to the given @a sink with @a owner as sender.
   *
   * @param owner The sender of log messages.
   * @param sink Sink to send log messages to.
   */
  Log(const std::string& owner, Licq::LogSink& sink);

  // From Licq::Log
  void log(Level level, const std::string& msg);
  void packet(Level level, const uint8_t* data, size_t size,
              const std::string& msg);

  // Bring in the other variants
  using Licq::Log::log;
  using Licq::Log::packet;

private:
  void fill(Licq::LogSink::Message& message, Level level,
            const std::string& text);

  const std::string myOwner;
  Licq::LogSink& mySink;
};

} // namespace LicqDaemon

#endif
