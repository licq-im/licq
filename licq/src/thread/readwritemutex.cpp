/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#include <licq/thread/condition.h>
#include <licq/thread/mutex.h>
#include <licq/thread/mutexlocker.h>
#include <licq/thread/readwritemutex.h>

#include <cassert>

using Licq::MutexLocker;
using Licq::ReadWriteMutex;

#ifdef LICQDAEMON_DEBUG_RW_MUTEX
#include "readwritemutex_debug.cpp"
#else

class ReadWriteMutex::Private
{
public:
  Private() :
    myNumReaders(0),
    myHasWriter(false)
  {
    // Empty
  }

  void setName(const std::string& /*name*/) { /* Empty */ }

  void waitRead() { myLockFree.wait(myMutex); }
  void waitWrite() { myLockFree.wait(myMutex); }

  void setReader() { /* Empty */ }
  void unsetReader() { /* Empty */ }

  void setWriter() { /* Empty */ }
  void unsetWriter() { /* Empty */ }

  int myNumReaders;
  bool myHasWriter;

  Mutex myMutex;
  Condition myLockFree;
};

#endif

ReadWriteMutex::ReadWriteMutex() :
  myPrivate(new Private)
{
  // Empty
}

ReadWriteMutex::~ReadWriteMutex()
{
  delete myPrivate;
}

void ReadWriteMutex::lockRead()
{
  LICQ_D();
  MutexLocker locker(d->myMutex);

  while (d->myHasWriter)
    d->waitRead();

  d->setReader();
  d->myNumReaders += 1;
}

void ReadWriteMutex::unlockRead()
{
  LICQ_D();
  MutexLocker locker(d->myMutex);

  assert(d->myNumReaders > 0);
  if (d->myNumReaders > 0)
  {
    d->unsetReader();
    d->myNumReaders -= 1;
    if (d->myNumReaders == 0)
      d->myLockFree.signal();
  }
}

void ReadWriteMutex::lockWrite()
{
  LICQ_D();
  MutexLocker locker(d->myMutex);

  while (d->myHasWriter || d->myNumReaders > 0)
    d->waitWrite();

  d->setWriter();
  d->myHasWriter = true;
}

void ReadWriteMutex::unlockWrite()
{
  LICQ_D();
  MutexLocker locker(d->myMutex);

  assert(d->myHasWriter);
  if (d->myHasWriter)
  {
    d->unsetWriter();
    d->myHasWriter = false;
    d->myLockFree.broadcast();
  }
}

void ReadWriteMutex::setName(const std::string& name)
{
  LICQ_D();
  MutexLocker locker(d->myMutex);

  d->setName(name);
}
