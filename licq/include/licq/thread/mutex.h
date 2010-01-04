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

#ifndef LICQ_MUTEX_H
#define LICQ_MUTEX_H

#include <boost/noncopyable.hpp>
#include <pthread.h>

namespace Licq
{

/**
 * A mutual exclusion device.
 *
 * It is useful for protecting shared data structures from concurrent
 * modifications, and implementing critical sections and monitors.
 *
 */
class Mutex : private boost::noncopyable
{
public:
  /**
   * Creates a new mutex.
   * @post The mutex is unlocked.
   */
  Mutex();

  /**
   * Destroys the mutex.
   * @pre The mutex is unlocked.
   */
  ~Mutex();

  /**
   * Locks the mutex. Blocks until the lock is acquired.
   * @pre The mutex is @b not locked by the calling thread.
   */
  void lock();

  /**
   * Tries to lock the mutex without blocking.
   * @returns True if the lock was acquired; otherwise false.
   */
  bool tryLock();

  /**
   * Unlocks the mutex.
   * @pre The mutex is locked the calling thread.
   */
  void unlock();

private:
  pthread_mutex_t myMytex;
};

inline Mutex::Mutex()
{
  ::pthread_mutex_init(&myMytex, NULL);
}

inline Mutex::~Mutex()
{
  :: pthread_mutex_destroy(&myMytex);
}

inline void Mutex::lock()
{
  ::pthread_mutex_lock(&myMytex);
}

inline bool Mutex::tryLock()
{
  return ::pthread_mutex_trylock(&myMytex) == 0;
}

inline void Mutex::unlock()
{
  ::pthread_mutex_unlock(&myMytex);
}

} // namespace Licq

#endif
