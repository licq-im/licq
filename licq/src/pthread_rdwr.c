// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
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
#include <assert.h>
#include <pthread.h>
#include "pthread_rdwr.h"

#ifdef DEBUG_RW_MUTEX
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

extern char BASE_DIR[];
#endif

int pthread_rdwr_init_np(pthread_rdwr_t *rdwrp, pthread_rdwrattr_t *attrp)
{
  // no warning
  attrp = attrp;

  rdwrp->readers_reading = 0;
  rdwrp->writer_writing = 0;
  pthread_mutex_init(&(rdwrp->mutex), NULL);
  pthread_cond_init(&(rdwrp->lock_free), NULL);

#ifdef DEBUG_RW_MUTEX
  rdwrp->name = strdup("no name");
#endif

  return 0;
}

int pthread_rdwr_destroy_np(pthread_rdwr_t *rdwrp)
{
#ifdef DEBUG_RW_MUTEX
  assert(rdwrp->readers_reading == 0);
  assert(rdwrp->writer_writing == 0);

  free(rdwrp->name);
  rdwrp->name = NULL;
#else
  (void)rdwrp;
#endif

  return 0;
}

#ifdef DEBUG_RW_MUTEX

static void print_users(FILE* file, pthread_rdwr_t *rdwrp, int reading)
{
  int i;

  fprintf(file, "Possible deadlock for thread %p trying to get a %s lock on '%s'\n",
      (void*)pthread_self(), (reading ? "read" : "write"), rdwrp->name);

  if (rdwrp->writer_writing > 0)
  {
    fprintf(file, "Thread %p holds the write lock\n", (void*)rdwrp->writer);
  }
  else
    fprintf(file, "No thread holds the write lock\n");

  if (rdwrp->readers_reading > 0)
  {
    fprintf(file, "These threads hold the read lock:");
    for (i = 0; i < rdwrp->readers_reading; ++i)
    {
      fprintf(file, " %p", (void*)rdwrp->readers[i]);
    }
    fprintf(file, "\n");
  }
  else
    fprintf(file, "No thread holds the read lock\n");
}

static void debug_cond_wait(pthread_rdwr_t *rdwrp, int reading)
{
  struct timeval now;
  gettimeofday(&now, NULL);

  struct timespec timeout;
  timeout.tv_sec = now.tv_sec + 30;
  timeout.tv_nsec = now.tv_usec * 1000;

  int ret = pthread_cond_timedwait(&(rdwrp->lock_free), &(rdwrp->mutex), &timeout);
  if (ret == ETIMEDOUT)
  {
    // Print to stderr
    print_users(stderr, rdwrp, reading);

    // Also save it in <basedir>/licq.debug_rw_mutex
    char filename[255];
    snprintf(filename, 255, "%s/licq.debug_rw_mutex", BASE_DIR);
    FILE* file = fopen(filename, "w");
    if (file != NULL)
    {
      fprintf(file, "time: %lu\n", (unsigned long)time(NULL));
      print_users(file, rdwrp, reading);
      fclose(file);
    }

    abort();
  }
}

static void set_writer(pthread_rdwr_t *rdwrp)
{
  rdwrp->writer = pthread_self();
}

static void unset_writer(pthread_rdwr_t *rdwrp)
{
  assert(pthread_equal(pthread_self(), rdwrp->writer));
}

static void set_reader(pthread_rdwr_t *rdwrp)
{
  int i;

  // Find this thread
  for (i = 0; i < rdwrp->readers_reading; ++i)
  {
    if (pthread_equal(pthread_self(), rdwrp->readers[i]))
      break;
  }

  // Make sure that this thread doesn't have a read lock already.
  // Comment the assert if a thread should be allowed to have multiple
  // read locks.
  assert(i == rdwrp->readers_reading);

  assert(rdwrp->readers_reading < RW_MUTEX_MAX_READERS);
  rdwrp->readers[rdwrp->readers_reading] = pthread_self();
}

static void unset_reader(pthread_rdwr_t *rdwrp)
{
  int i;

  // Find this thread
  for (i = 0; i < rdwrp->readers_reading; ++i)
  {
    if (pthread_equal(pthread_self(), rdwrp->readers[i]))
      break;
  }

  // Make sure that this thread has a read lock
  assert(i < rdwrp->readers_reading);

  // Remove the reference to this thread
  for (; i < rdwrp->readers_reading - 1; ++i)
    rdwrp->readers[i] = rdwrp->readers[i + 1];
}

#endif /* DEBUG_RW_MUTEX */

int pthread_rdwr_rlock_np(pthread_rdwr_t *rdwrp)
{
  pthread_mutex_lock(&(rdwrp->mutex));
  while (rdwrp->writer_writing)
  {
#ifdef DEBUG_RW_MUTEX
    debug_cond_wait(rdwrp, 1);
#else
    pthread_cond_wait(&(rdwrp->lock_free), &(rdwrp->mutex));
#endif
  }
#ifdef DEBUG_RW_MUTEX
  set_reader(rdwrp);
#endif
  rdwrp->readers_reading++;
  pthread_mutex_unlock(&(rdwrp->mutex));
  return 0;
}

int pthread_rdwr_runlock_np(pthread_rdwr_t *rdwrp)
{
  pthread_mutex_lock(&(rdwrp->mutex));
#ifdef DEBUG_RW_MUTEX
  assert(rdwrp->readers_reading > 0);
#endif
  if (rdwrp->readers_reading == 0)
  {
    pthread_mutex_unlock(&(rdwrp->mutex));
    return -1;
  }
  else
  {
#ifdef DEBUG_RW_MUTEX
    unset_reader(rdwrp);
#endif
    rdwrp->readers_reading--;
    if (rdwrp->readers_reading == 0)
      pthread_cond_signal(&(rdwrp->lock_free));
    pthread_mutex_unlock(&(rdwrp->mutex));
    return 0;
  }
}

int pthread_rdwr_wlock_np(pthread_rdwr_t *rdwrp)
{
  pthread_mutex_lock(&(rdwrp->mutex));
  while (rdwrp->writer_writing || rdwrp->readers_reading)
  {
#ifdef DEBUG_RW_MUTEX
    debug_cond_wait(rdwrp, 0);
#else
    pthread_cond_wait(&(rdwrp->lock_free), &(rdwrp->mutex));
#endif
  }
#ifdef DEBUG_RW_MUTEX
  set_writer(rdwrp);
#endif
  rdwrp->writer_writing++;
  pthread_mutex_unlock(&(rdwrp->mutex));
  return 0;
}

int pthread_rdwr_wunlock_np(pthread_rdwr_t *rdwrp)
{
  pthread_mutex_lock(&(rdwrp->mutex));
#ifdef DEBUG_RW_MUTEX
  assert(rdwrp->writer_writing > 0);
#endif
  if (rdwrp->writer_writing == 0)
  {
    pthread_mutex_unlock(&(rdwrp->mutex));
    return -1;
  }
  else
  {
#ifdef DEBUG_RW_MUTEX
    unset_writer(rdwrp);
#endif
    rdwrp->writer_writing = 0;
    pthread_cond_broadcast(&(rdwrp->lock_free));
    pthread_mutex_unlock(&(rdwrp->mutex));
    return 0;
  }
}
