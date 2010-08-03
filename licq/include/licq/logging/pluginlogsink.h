/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#ifndef LICQ_PLUGINLOGSINK_H
#define LICQ_PLUGINLOGSINK_H

#include "logsink.h"
#include "../macro.h"

#include <boost/shared_ptr.hpp>

namespace Licq
{

class PluginLogSink : public AdjustableLogSink
{
public:
  typedef boost::shared_ptr<PluginLogSink> Ptr;

  PluginLogSink();
  ~PluginLogSink();

  /**
   * Get the file descriptor that will be readable when there is a new log
   * message to be poped (one byte per message). The returned fd can be used in
   * e.g. select().
   */
  int getReadPipe();

  /**
   * Removes the first message from the queue and returns it, optionally
   * reading a byte from the pipe.
   *
   * @param readPipe If true one byte will be read from the pipe. If the pipe
   * is read outside of this method, pass false here.
   * @return A log message or a null pointer on error.
   */
  Message::Ptr popMessage(bool readPipe = true);

  // LogSink
  bool isLogging(Log::Level level) const;
  bool isLoggingPackets() const;
  void log(Message::Ptr message);

  // AdjustableLogSink
  void setLogLevel(Log::Level level, bool enable);
  void setLogPackets(bool enable);
  void setAllLogLevels(bool enable);
  void setLogLevelsFromBitmask(unsigned int levels);
  unsigned int getLogLevelsBitmask() const;
 
private:
  LICQ_DECLARE_PRIVATE();
};

} // namespace Licq

#endif
