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

const unsigned int AllLevelsMask = (1 << (Licq::Log::Debug + 1)) - 1;
extern const unsigned int PacketBit = 0x1000;

using namespace LicqDaemon;
using Licq::MutexLocker;

AdjustableLogSink::AdjustableLogSink()
  : myLogLevels(0)
{
  // Empty
}

bool AdjustableLogSink::isLogging(Licq::Log::Level level) const
{
  MutexLocker locker(myMutex);
  return myLogLevels & (1 << level);
}

bool AdjustableLogSink::isLoggingPackets() const
{
  MutexLocker locker(myMutex);
  return myLogLevels & PacketBit;
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
  if (enable)
    myLogLevels |= PacketBit;
  else
    myLogLevels &= ~PacketBit;
}

void AdjustableLogSink::setAllLogLevels(bool enable)
{
  MutexLocker locker(myMutex);
  if (enable)
    myLogLevels |= AllLevelsMask;
  else
    myLogLevels &= ~AllLevelsMask;
}

void AdjustableLogSink::setLogLevelsFromBitmask(unsigned int levels)
{
  MutexLocker locker(myMutex);
  myLogLevels = levels & (AllLevelsMask | PacketBit);
}

unsigned int AdjustableLogSink::getLogLevelsBitmask() const
{
  MutexLocker locker(myMutex);
  return myLogLevels;
}
