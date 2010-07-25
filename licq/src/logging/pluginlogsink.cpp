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

#include <licq/pipe.h>
#include <licq/logging/pluginlogsink.h>
#include <licq/thread/mutex.h>
#include <licq/thread/mutexlocker.h>

#include <deque>

using namespace Licq;

class PluginLogSink::Private : public LicqDaemon::AdjustableLogSink
{
public:
  void log(Message::Ptr message)
  {
    MutexLocker locker(myMutex);
    myMessages.push_back(message);
    myPipe.putChar('M');
  }

  LogSink::Message::Ptr popMessage(bool readPipe)
  {
    MutexLocker locker(myMutex);
    if (myMessages.empty())
      return Message::Ptr();

    if (readPipe)
      myPipe.getChar();

    Message::Ptr message = myMessages.front();
    myMessages.pop_front();
    return message;
  }

  Pipe myPipe;
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
  return d->popMessage(readPipe);
}

bool PluginLogSink::isLogging(Log::Level level) const
{
  LICQ_D();
  return d->isLogging(level);
}

bool PluginLogSink::isLoggingPackets() const
{
  LICQ_D();
  return d->isLoggingPackets();
}

void PluginLogSink::log(Message::Ptr message)
{
  LICQ_D();
  d->log(message);
}

void PluginLogSink::setLogLevel(Log::Level level, bool enable)
{
  LICQ_D();
  d->setLogLevel(level, enable);
}

void PluginLogSink::setLogPackets(bool enable)
{
  LICQ_D();
  d->setLogPackets(enable);
}

void PluginLogSink::setAllLogLevels(bool enable)
{
  LICQ_D();
  d->setAllLogLevels(enable);
}

void PluginLogSink::setLogLevelsFromBitmask(unsigned int levels)
{
  LICQ_D();
  d->setLogLevelsFromBitmask(levels);
}

unsigned int PluginLogSink::getLogLevelsBitmask() const
{
  LICQ_D();
  return d->getLogLevelsBitmask();
}
