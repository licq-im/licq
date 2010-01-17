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

/*
 * Define DEBUG_RW_MUTEX and recompile the daemon to debug deadlocks. If a
 * (potential) deadlock is discovered, the daemon will print a message to
 * stderr (and to the file <basedir>/licq.debug_rw_mutex) and then abort.
 */
#define DEBUG_RW_MUTEX

#include "licq/thread/mutexlocker.h"
#include "licq/thread/readwritemutex.h"

#include <cassert>

using Licq::MutexLocker;
using Licq::ReadWriteMutex;

#ifdef DEBUG_RW_MUTEX
#include "readwritemutex_debug.cpp"
#else

class ReadWriteMutex::Private
{
public:
  Private(ReadWriteMutex* parent) :
    myParent(parent)
  {
    // Empty
  }

  void setName(const std::string& /*name*/) { /* Empty */ }

  void waitRead()
  {
    myParent->myLockFree.wait(myParent->myMutex);
  }

  void waitWrite()
  {
    myParent->myLockFree.wait(myParent->myMutex);    
  }

  void setReader() { /* Empty */ }
  void unsetReader() { /* Empty */ }

  void setWriter() { /* Empty */ }
  void unsetWriter() { /* Empty */ }

private:
  ReadWriteMutex* myParent;
};

#endif

ReadWriteMutex::ReadWriteMutex() :
  myPrivate(new Private(this)),
  myNumReaders(0),
  myHasWriter(false)
{
  // Empty
}

ReadWriteMutex::~ReadWriteMutex()
{
  delete myPrivate;
}

void ReadWriteMutex::lockRead()
{
  MutexLocker locker(myMutex);

  while (myHasWriter)
    myPrivate->waitRead();

  myPrivate->setReader();
  ++myNumReaders;
}

void ReadWriteMutex::unlockRead()
{
  MutexLocker locker(myMutex);

  assert(myNumReaders > 0);
  if (myNumReaders > 0)
  {
    myPrivate->unsetReader();
    if (--myNumReaders == 0)
      myLockFree.signal();
  }
}

void ReadWriteMutex::lockWrite()
{
  MutexLocker locker(myMutex);

  while (myHasWriter || myNumReaders > 0)
    myPrivate->waitWrite();

  myPrivate->setWriter();
  myHasWriter = true;
}

void ReadWriteMutex::unlockWrite()
{
  MutexLocker locker(myMutex);

  assert(myHasWriter);
  if (myHasWriter)
  {
    myPrivate->unsetWriter();
    myHasWriter = false;
    myLockFree.broadcast();
  }
}

void ReadWriteMutex::setName(const std::string& name)
{
  myPrivate->setName(name);
}
