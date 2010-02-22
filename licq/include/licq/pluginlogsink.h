/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Erik Johansson <erijo@licq.org>
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

#include <boost/shared_ptr.hpp>

namespace Licq
{

class PluginLogSink : public LogSink
{
public:
  typedef boost::shared_ptr<PluginLogSink> Ptr;

  enum { TYPE_MESSAGE = 'M' };
  enum { TYPE_PACKET = 'P' };

  PluginLogSink();
  ~PluginLogSink();

  int getReadPipe();

  const Message* getFirstMessage();
  void popFirstMessage();

  const Packet* getFirstPacket();
  void popFirstPacket();

  void setLogLevel(Log::Level level, bool enable);
  void setAllLogLevels(bool enable);
 
  // From LogSink
  bool isLogging(Log::Level level);
  void log(const Message& message);
  void logPacket(const Packet& packet);

private:
  class Private;
  Private* const myPrivate;
  friend class Private;
};

} // namespace Licq

#endif
