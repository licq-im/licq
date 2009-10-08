// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

/********************************************************
 * An example source module to accompany...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 *     by Brad nichols, Dick Buttlar, Jackie Farrell
 *     O'Reilly & Associates, Inc.
 *
 ********************************************************
 * rdwr.c --
 *
 * Library of functions implementing reader/writer locks
 */
#include "licq_mutex.h"

#include <assert.h>
#include <pthread.h>

#ifdef DEBUG_RW_MUTEX
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

extern char BASE_DIR[];
#endif

ReadWriteMutex::ReadWriteMutex()
  : myNumReaders(0),
    myHasWriter(false)
{
  pthread_mutex_init(&myMutex, NULL);
  pthread_cond_init(&myLockFree, NULL);

#ifdef DEBUG_RW_MUTEX
  myName = "no name";
#endif
}

ReadWriteMutex::~ReadWriteMutex()
{
#ifdef DEBUG_RW_MUTEX
  assert(myNumReaders == 0);
  assert(myHasWriter == false);
#endif
}

#ifdef DEBUG_RW_MUTEX

void ReadWriteMutex::printUsers(FILE* file, bool writing) const
{
  int i;

  fprintf(file, "Possible deadlock for thread %p trying to get a %s lock on '%s'\n",
      (void*)pthread_self(), (writing ? "write" : "read"), myName.c_str());

  if (myHasWriter)
  {
    fprintf(file, "Thread %p holds the write lock\n", (void*)myWriterThread);
  }
  else
    fprintf(file, "No thread holds the write lock\n");

  if (myNumReaders > 0)
  {
    fprintf(file, "These threads hold the read lock:");
    for (i = 0; i < myNumReaders; ++i)
    {
      fprintf(file, " %p", (void*)myReaderThreads[i]);
    }
    fprintf(file, "\n");
  }
  else
    fprintf(file, "No thread holds the read lock\n");
}

void ReadWriteMutex::debugCondWait(bool writing)
{
  struct timeval now;
  gettimeofday(&now, NULL);

  struct timespec timeout;
  timeout.tv_sec = now.tv_sec + 30;
  timeout.tv_nsec = now.tv_usec * 1000;

  int ret = pthread_cond_timedwait(&myLockFree, &myMutex, &timeout);
  if (ret == ETIMEDOUT)
  {
    // Print to stderr
    printUsers(stderr, writing);

    // Also save it in <basedir>/licq.debug_rw_mutex
    char filename[255];
    snprintf(filename, 255, "%s/licq.debug_rw_mutex", BASE_DIR);
    FILE* file = fopen(filename, "w");
    if (file != NULL)
    {
      fprintf(file, "time: %lu\n", (unsigned long)time(NULL));
      printUsers(file, writing);
      fclose(file);
    }

    abort();
  }
}

void ReadWriteMutex::setWriter()
{
  myWriterThread = pthread_self();
}

void ReadWriteMutex::unsetWriter()
{
  assert(pthread_equal(pthread_self(), myWriterThread));
}

void ReadWriteMutex::setReader()
{
  int i;

  // Find this thread
  for (i = 0; i < myNumReaders; ++i)
  {
    if (pthread_equal(pthread_self(), myReaderThreads[i]))
      break;
  }

  // Make sure that this thread doesn't have a read lock already.
  // Comment the assert if a thread should be allowed to have multiple
  // read locks.
  assert(i == myNumReaders);

  assert(myNumReaders < RW_MUTEX_MAX_READERS);
  myReaderThreads[myNumReaders] = pthread_self();
}

void ReadWriteMutex::unsetReader()
{
  int i;

  // Find this thread
  for (i = 0; i < myNumReaders; ++i)
  {
    if (pthread_equal(pthread_self(), myReaderThreads[i]))
      break;
  }

  // Make sure that this thread has a read lock
  assert(i < myNumReaders);

  // Remove the reference to this thread
  for (; i < myNumReaders - 1; ++i)
    myReaderThreads[i] = myReaderThreads[i + 1];
}

#endif /* DEBUG_RW_MUTEX */


void ReadWriteMutex::lockRead()
{
  pthread_mutex_lock(&myMutex);
  while (myHasWriter)
  {
#ifdef DEBUG_RW_MUTEX
    debugCondWait(false);
#else
    pthread_cond_wait(&myLockFree, &myMutex);
#endif
  }
#ifdef DEBUG_RW_MUTEX
  setReader();
#endif
  ++myNumReaders;
  pthread_mutex_unlock(&myMutex);
}

void ReadWriteMutex::unlockRead()
{
  pthread_mutex_lock(&myMutex);
#ifdef DEBUG_RW_MUTEX
  assert(myNumReaders > 0);
#endif
  if (myNumReaders > 0)
  {
#ifdef DEBUG_RW_MUTEX
    unsetReader();
#endif
    --myNumReaders;
    if (myNumReaders == 0)
      pthread_cond_signal(&myLockFree);
  }
  pthread_mutex_unlock(&myMutex);
}

void ReadWriteMutex::lockWrite()
{
  pthread_mutex_lock(&myMutex);
  while (myHasWriter || myNumReaders > 0)
  {
#ifdef DEBUG_RW_MUTEX
    debugCondWait(true);
#else
    pthread_cond_wait(&myLockFree, &myMutex);
#endif
  }
#ifdef DEBUG_RW_MUTEX
  setWriter();
#endif
  myHasWriter = true;
  pthread_mutex_unlock(&myMutex);
}

void ReadWriteMutex::unlockWrite()
{
  pthread_mutex_lock(&myMutex);
#ifdef DEBUG_RW_MUTEX
  assert(myHasWriter != false);
#endif
  if (myHasWriter)
  {
#ifdef DEBUG_RW_MUTEX
    unsetWriter();
#endif
    myHasWriter = false;
    pthread_cond_broadcast(&myLockFree);
  }
  pthread_mutex_unlock(&myMutex);
}
