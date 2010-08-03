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

#include "logdistributor.h"
#include <licq/thread/mutexlocker.h>

#include <boost/foreach.hpp>

using Licq::LogSink;
using Licq::MutexLocker;
using namespace LicqDaemon;

void LogDistributor::registerSink(LogSink::Ptr sink)
{
  MutexLocker locker(myMutex);
  if (std::find(mySinks.begin(), mySinks.end(), sink) != mySinks.end())
    return;

  mySinks.push_back(sink);
}

void LogDistributor::unregisterSink(LogSink::Ptr sink)
{
  MutexLocker locker(myMutex);
  mySinks.remove(sink);
}

bool LogDistributor::isLogging(Licq::Log::Level level) const
{
  MutexLocker locker(myMutex);
  BOOST_FOREACH(LogSink::Ptr sink, mySinks)
  {
    if (sink->isLogging(level))
      return true;
  }
  return false;
}

bool LogDistributor::isLoggingPackets() const
{
  MutexLocker locker(myMutex);
  BOOST_FOREACH(LogSink::Ptr sink, mySinks)
  {
    if (sink->isLoggingPackets())
      return true;
  }
  return false;
}

void LogDistributor::log(Message::Ptr message)
{
  MutexLocker locker(myMutex);
  BOOST_FOREACH(LogSink::Ptr sink, mySinks)
  {
    if (sink->isLogging(message->level))
      sink->log(message);
  }
}
