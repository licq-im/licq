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

#ifndef LICQDAEMON_STREAMLOGSINK_H
#define LICQDAEMON_STREAMLOGSINK_H

#include "licq/logsink.h"

#include <iosfwd>

namespace LicqDaemon
{

/**
 * A log sink that prints all log messages to a given std::ostream. The log
 * messages are formatted as:
 * "HH:MM:SS.mmm [XXX] sender: a text message"
 * where XXX is three letters indicating log level.
 */
class StreamLogSink : public Licq::LogSink
{
public:
  StreamLogSink(std::ostream& stream);

  void setUseColors(bool useColors) { myUseColors = useColors; }
  void setLogLevel(Licq::Log::Level level, bool enable);
  void setLogPackets(bool enable) { myLogPackets = enable; }

  void setLogLevels(int levels);

  // From Licq::LogSink
  bool isLogging(Licq::Log::Level level);
  bool isLoggingPackets();
  void log(const Message& message);
  void logPacket(const Packet& packet);

private:
  std::ostream& myStream;
  bool myUseColors;
  int myLogLevels;
  bool myLogPackets;
};

} // namespace LicqDaemon

#endif
