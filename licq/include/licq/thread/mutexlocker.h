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

#ifndef LICQ_MUTEXLOCKER_H
#define LICQ_MUTEXLOCKER_H

#include "mutex.h"

#include <boost/noncopyable.hpp>

namespace Licq
{

/**
 * A class that makes mutex handling easier and exception safe.
 * 
 * Create a locker (in a limited scope) and the locker will lock the mutex when
 * constructed and unlock it when destructed.
 *
 * @ingroup thread
 */
class MutexLocker : private boost::noncopyable
{
public:
  /**
   * Create a MutexLocker and locks the mutex.
   * @param mutex An unlocked mutex.
   * @pre The mutex is unlocked.
   * @post The mutex is locked.
   */
  explicit MutexLocker(Mutex& mutex);

  /**
   * Unlocks the mutex if it is locked.
   * @post The mutex is unlocked.
   */
  ~MutexLocker();

  /**
   * Relock the mutex if it has been unlocked().
   * @pre The mutex is unlocked.
   * @post The mutex is locked.
   */
  void relock();

  /**
   * Unlock the mutex explicitly.
   * @pre The mutex is locked.
   * @post The mutex is unlocked.
   */
  void unlock();

private:
  Mutex& myMutex;
  bool myIsLocked;
};

} // namespace Licq

#endif
