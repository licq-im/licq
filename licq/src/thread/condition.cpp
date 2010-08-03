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

#include <licq/thread/condition.h>

#include <ctime>
#include <sys/time.h>

using namespace Licq;

static void conditionCleanup(void* arg)
{
  Mutex* mutex = static_cast<Mutex*>(arg);
  mutex->unlock();
}

bool Condition::wait(Mutex& mutex, unsigned int msec)
{
  if (msec == WAIT_FOREVER)
  {
    // Unlock mutex if thread is cancelled while waiting
    pthread_cleanup_push(&conditionCleanup, &mutex);
    ::pthread_cond_wait(&myCondition, &mutex.myMutex);
    pthread_cleanup_pop(0);
    return true;
  }

  timeval now;
  ::gettimeofday(&now, NULL);

  timespec abstime;
  abstime.tv_sec = now.tv_sec + (msec / 1000);
  abstime.tv_nsec = now.tv_usec * 1000 + ((msec % 1000) * 1000 * 1000);

  const long nanoInSec = 1 * 1000 * 1000 * 1000;
  if (abstime.tv_nsec >= nanoInSec)
  {
    abstime.tv_nsec -= nanoInSec;
    abstime.tv_sec += 1;
  }

  int ret;
  // Unlock mutex if thread is cancelled while waiting
  pthread_cleanup_push(&conditionCleanup, &mutex);
  ret = ::pthread_cond_timedwait(&myCondition, &mutex.myMutex, &abstime);
  pthread_cleanup_pop(0);
  return (ret == 0);
}
