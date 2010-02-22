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

#include "licq/pluginlogsink.h"
#include "licq/thread/mutex.h"
#include "licq/thread/mutexlocker.h"
#include "utils/pipe.h"

#include <deque>

using namespace Licq;

class PluginLogSink::Private
{
public:
  Private() :
    myLogLevels(0)
  {
    // Empty
  }

  LicqDaemon::Pipe myPipe;

  Mutex myMutex;
  int myLogLevels;

  std::deque<Message::Ptr> myMessages;
  std::deque<Packet::Ptr> myPackets;
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
  return myPrivate->myPipe.getReadFd();
}

LogSink::Message::Ptr PluginLogSink::popMessage()
{
  MutexLocker locker(myPrivate->myMutex);
  if (myPrivate->myMessages.empty())
    return Message::Ptr();

  Message::Ptr message = myPrivate->myMessages.front();
  myPrivate->myMessages.pop_front();
  return message;
}

LogSink::Packet::Ptr PluginLogSink::popPacket()
{
  MutexLocker locker(myPrivate->myMutex);
  if (myPrivate->myPackets.empty())
    return Packet::Ptr();

  Packet::Ptr packet = myPrivate->myPackets.front();
  myPrivate->myPackets.pop_front();
  return packet;
}

void PluginLogSink::setLogLevel(Log::Level level, bool enable)
{
  MutexLocker locker(myPrivate->myMutex);
  if (enable)
    myPrivate->myLogLevels |= (1 << level);
  else
    myPrivate->myLogLevels &= ~(1 << level);
}

void PluginLogSink::setAllLogLevels(bool enable)
{
  MutexLocker locker(myPrivate->myMutex);
  if (enable)
    myPrivate->myLogLevels = 0x3f;
  else
    myPrivate->myLogLevels = 0;
}

bool PluginLogSink::isLogging(Log::Level level)
{
  MutexLocker locker(myPrivate->myMutex);
  return myPrivate->myLogLevels & (1 << level);
}

void PluginLogSink::log(Message::Ptr message)
{
  MutexLocker locker(myPrivate->myMutex);
  myPrivate->myMessages.push_back(message);
  myPrivate->myPipe.putChar(TYPE_MESSAGE);
}

void PluginLogSink::logPacket(Packet::Ptr packet)
{
  MutexLocker locker(myPrivate->myMutex);
  myPrivate->myPackets.push_back(packet);
  myPrivate->myPipe.putChar(TYPE_PACKET);
}
