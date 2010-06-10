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

#include <licq/pipe.h>
#include "licq/pluginlogsink.h"
#include "licq/thread/mutex.h"
#include "licq/thread/mutexlocker.h"

#include <deque>

using namespace Licq;

class PluginLogSink::Private
{
public:
  Private() :
    myLogLevels(0),
    myLogPackets(false)
  {
    // Empty
  }

  Pipe myPipe;

  Mutex myMutex;
  int myLogLevels;
  bool myLogPackets;

  std::deque<Message::Ptr> myMessages;
};

PluginLogSink::PluginLogSink() :
  myPrivate(new Private())
{
  // Empty
}

PluginLogSink::~PluginLogSink()
{
  delete myPrivate;
}

int PluginLogSink::getReadPipe()
{
  LICQ_D();
  return d->myPipe.getReadFd();
}

LogSink::Message::Ptr PluginLogSink::popMessage(bool readPipe)
{
  LICQ_D();
  MutexLocker locker(d->myMutex);
  if (d->myMessages.empty())
    return Message::Ptr();

  if (readPipe)
    d->myPipe.getChar();

  Message::Ptr message = d->myMessages.front();
  d->myMessages.pop_front();
  return message;
}

void PluginLogSink::setLogLevel(Log::Level level, bool enable)
{
  LICQ_D();
  MutexLocker locker(d->myMutex);
  if (enable)
    d->myLogLevels |= (1 << level);
  else
    d->myLogLevels &= ~(1 << level);
}

void PluginLogSink::setLogPackets(bool enable)
{
  LICQ_D();
  MutexLocker locker(d->myMutex);
  d->myLogPackets = enable;
}

void PluginLogSink::setAllLogLevels(bool enable)
{
  LICQ_D();
  MutexLocker locker(d->myMutex);
  if (enable)
    d->myLogLevels = 0x3f;
  else
    d->myLogLevels = 0;
}

bool PluginLogSink::isLogging(Log::Level level)
{
  LICQ_D();
  MutexLocker locker(d->myMutex);
  return d->myLogLevels & (1 << level);
}

bool PluginLogSink::isLoggingPackets()
{
  LICQ_D();
  MutexLocker locker(d->myMutex);
  return d->myLogPackets;
}

void PluginLogSink::log(Message::Ptr message)
{
  LICQ_D();
  MutexLocker locker(d->myMutex);
  d->myMessages.push_back(message);
  d->myPipe.putChar('M');
}
