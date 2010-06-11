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

#include <cstdio>
#include <cstdlib>

#include <licq/daemon.h>

using namespace std;
using Licq::gDaemon;

class ReadWriteMutex::Private
{
public:
  Private(ReadWriteMutex* parent) :
    myParent(parent),
    myName("no name")
  {
    // Empty
  }

  ~Private()
  {
    assert(myParent->myNumReaders == 0);
    assert(myParent->myHasWriter == false);
  }

  void setName(const std::string& name) { myName = name; }

  void waitRead() { debugWait(false); }
  void waitWrite() { debugWait(true); }

  void printUsers(FILE* file, bool writing) const;
  void debugWait(bool writing);

  void setWriter();
  void unsetWriter();

  void setReader();
  void unsetReader();

private:
  static const int RW_MUTEX_MAX_READERS = 20;
  static const unsigned int WAIT_TIMEOUT = 30 * 1000;

  ReadWriteMutex* myParent;
  pthread_t myWriterThread;
  pthread_t myReaderThreads[RW_MUTEX_MAX_READERS];
  std::string myName;
};

void ReadWriteMutex::Private::printUsers(FILE* file, bool writing) const
{
  ::fprintf(file, "Possible deadlock for thread %p trying to get a %s lock "
            "on '%s'\n", (void*)::pthread_self(),
            (writing ? "write" : "read"), myName.c_str());

  if (myParent->myHasWriter)
    ::fprintf(file, "Thread %p holds the write lock\n", (void*)myWriterThread);
  else
    ::fprintf(file, "No thread holds the write lock\n");

  if (myParent->myNumReaders > 0)
  {
    ::fprintf(file, "These threads hold the read lock:");
    for (int i = 0; i < myParent->myNumReaders; ++i)
      ::fprintf(file, " %p", (void*)myReaderThreads[i]);
    ::fprintf(file, "\n");
  }
  else
    fprintf(file, "No thread holds the read lock\n");
}

void ReadWriteMutex::Private::debugWait(bool writing)
{
  if (!myParent->myLockFree.wait(myParent->myMutex, WAIT_TIMEOUT))
  {
    // Print to stderr
    printUsers(stderr, writing);

    // Also save it in <basedir>/licq.debug_rw_mutex
    string filename = gDaemon.baseDir() + "licq.debug_rw_mutex";
    FILE* file = ::fopen(filename.c_str(), "w");
    if (file != NULL)
    {
      ::fprintf(file, "time: %lu\n", (unsigned long)time(NULL));
      printUsers(file, writing);
      ::fclose(file);
    }

    ::abort();
  }
}

void ReadWriteMutex::Private::setWriter()
{
  myWriterThread = ::pthread_self();
}

void ReadWriteMutex::Private::unsetWriter()
{
  assert(::pthread_equal(::pthread_self(), myWriterThread));
}

void ReadWriteMutex::Private::setReader()
{
  int i;

  // Find this thread
  for (i = 0; i < myParent->myNumReaders; ++i)
  {
    if (::pthread_equal(::pthread_self(), myReaderThreads[i]))
      break;
  }

  // Make sure that this thread doesn't have a read lock already.
  // Comment the assert if a thread should be allowed to have multiple
  // read locks.
  assert(i == myParent->myNumReaders);

  assert(myParent->myNumReaders < RW_MUTEX_MAX_READERS);
  myReaderThreads[myParent->myNumReaders] = ::pthread_self();
}

void ReadWriteMutex::Private::unsetReader()
{
  int i;

  // Find this thread
  for (i = 0; i < myParent->myNumReaders; ++i)
  {
    if (::pthread_equal(::pthread_self(), myReaderThreads[i]))
      break;
  }

  // Make sure that this thread has a read lock
  assert(i < myParent->myNumReaders);

  // Remove the reference to this thread
  for (; i < myParent->myNumReaders - 1; ++i)
    myReaderThreads[i] = myReaderThreads[i + 1];
}
