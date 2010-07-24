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

#include "adjustablelogsink.h"

#include <licq/thread/mutexlocker.h>

void Licq::adjustLogSinkOldFormat(Licq::AdjustableLogSink::Ptr sink,
                                  int levels)
{
  sink->setAllLogLevels(false);

  sink->setLogLevel(Log::Info, levels & 0x1);
  sink->setLogLevel(Log::Unknown, levels & 0x2);
  sink->setLogLevel(Log::Error, levels & 0x4);
  sink->setLogLevel(Log::Warning, levels & 0x8);

  sink->setLogLevel(Log::Debug, levels & 0x10);
  sink->setLogPackets(levels & 0x10);
}

using namespace LicqDaemon;
using Licq::MutexLocker;

AdjustableLogSink::AdjustableLogSink()
  : myLogLevels(0),
    myLogPackets(false)
{
  // Empty
}

bool AdjustableLogSink::isLogging(Licq::Log::Level level)
{
  MutexLocker locker(myMutex);
  return myLogLevels & (1 << level);
}

bool AdjustableLogSink::isLoggingPackets()
{
  MutexLocker locker(myMutex);
  return myLogPackets;
}

void AdjustableLogSink::setLogLevel(Licq::Log::Level level, bool enable)
{
  MutexLocker locker(myMutex);
  if (enable)
    myLogLevels |= (1 << level);
  else
    myLogLevels &= ~(1 << level);
}

void AdjustableLogSink::setLogPackets(bool enable)
{
  MutexLocker locker(myMutex);
  myLogPackets = enable;
}

void AdjustableLogSink::setAllLogLevels(bool enable)
{
  MutexLocker locker(myMutex);
  myLogLevels = enable ? 0x3f : 0;
}
