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

#ifndef LICQDAEMON_LOGDISTRIBUTOR_H
#define LICQDAEMON_LOGDISTRIBUTOR_H

#include <licq/logging/logsink.h>
#include <licq/thread/mutex.h>

#include <list>

namespace LicqDaemon
{

/**
 * Distributes log messages to all registered sinks.
 */
class LogDistributor : public Licq::LogSink
{
public:
  /**
   * Registers a sink that will receive a copy of future log messages.
   *
   * @param sink Sink to register.
   */
  void registerSink(LogSink::Ptr sink);

  /**
   * Unregisters a previous registered log sink.
   *
   * @param sink Sink to unregister.
   */
  void unregisterSink(LogSink::Ptr sink);

  // From Licq::LogSink
  bool isLogging(Licq::Log::Level level) const;
  bool isLoggingPackets() const;
  void log(Message::Ptr message);

private:
  mutable Licq::Mutex myMutex;

  typedef std::list<LogSink::Ptr> LogSinkList;
  LogSinkList mySinks;
};

} // namespace LicqDaemon

#endif
