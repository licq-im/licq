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
  Private() :
    myNumReaders(0),
    myHasWriter(false),
    myName("no name")
  {
    // Empty
  }

  ~Private()
  {
    assert(myNumReaders == 0);
    assert(!myHasWriter);
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

  int myNumReaders;
  bool myHasWriter;

  Mutex myMutex;
  Condition myLockFree;

private:
  static const int RW_MUTEX_MAX_READERS = 20;
  static const unsigned int WAIT_TIMEOUT = 30 * 1000;

  pthread_t myWriterThread;
  pthread_t myReaderThreads[RW_MUTEX_MAX_READERS];
  std::string myName;
};

void ReadWriteMutex::Private::printUsers(FILE* file, bool writing) const
{
  ::fprintf(file, "Possible deadlock for thread %p trying to get a %s lock "
            "on '%s'\n", (void*)::pthread_self(),
            (writing ? "write" : "read"), myName.c_str());

  if (myHasWriter)
    ::fprintf(file, "Thread %p holds the write lock\n", (void*)myWriterThread);
  else
    ::fprintf(file, "No thread holds the write lock\n");

  if (myNumReaders > 0)
  {
    ::fprintf(file, "These threads hold the read lock:");
    for (int i = 0; i < myNumReaders; ++i)
      ::fprintf(file, " %p", (void*)myReaderThreads[i]);
    ::fprintf(file, "\n");
  }
  else
    fprintf(file, "No thread holds the read lock\n");
}

void ReadWriteMutex::Private::debugWait(bool writing)
{
  if (!myLockFree.wait(myMutex, WAIT_TIMEOUT))
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
  for (i = 0; i < myNumReaders; ++i)
  {
    if (::pthread_equal(::pthread_self(), myReaderThreads[i]))
      break;
  }

  // Make sure that this thread doesn't have a read lock already.
  // Comment the assert if a thread should be allowed to have multiple
  // read locks.
  assert(i == myNumReaders);

  assert(myNumReaders < RW_MUTEX_MAX_READERS);
  myReaderThreads[myNumReaders] = ::pthread_self();
}

void ReadWriteMutex::Private::unsetReader()
{
  int i;

  // Find this thread
  for (i = 0; i < myNumReaders; ++i)
  {
    if (::pthread_equal(::pthread_self(), myReaderThreads[i]))
      break;
  }

  // Make sure that this thread has a read lock
  assert(i < myNumReaders);

  // Remove the reference to this thread
  for (; i < myNumReaders - 1; ++i)
    myReaderThreads[i] = myReaderThreads[i + 1];
}
