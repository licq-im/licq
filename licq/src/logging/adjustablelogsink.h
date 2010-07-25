/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq Developers <licq-dev@googlegroups.com>
 *
 * Please refer to the COPYRIGHT file distributed with this source
 * distribution for the names of the individual contributors.
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

#ifndef LICQDAEMON_ADJUSTABLELOGSINK_H
#define LICQDAEMON_ADJUSTABLELOGSINK_H

#include <licq/logging/logsink.h>
#include <licq/thread/mutex.h>

namespace LicqDaemon
{

class AdjustableLogSink : public Licq::AdjustableLogSink
{
public:
  AdjustableLogSink();

  // Licq::LogSink (not all)
  bool isLogging(Licq::Log::Level level) const;
  bool isLoggingPackets() const;

  // Licq::AdjustableLogSink
  void setLogLevel(Licq::Log::Level level, bool enable);
  void setLogPackets(bool enable);
  void setAllLogLevels(bool enable);
  void setLogLevelsFromBitmask(unsigned int levels);
  unsigned int getLogLevelsBitmask() const;

protected:
  mutable Licq::Mutex myMutex;

private:
  unsigned int myLogLevels;
};

} // namespace LicqDaemon

#endif
