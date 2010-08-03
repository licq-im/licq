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

#ifndef LICQ_THREADSPECIFICDATA_H
#define LICQ_THREADSPECIFICDATA_H

#include <pthread.h>

namespace Licq
{

/**
 * ThreadSpecificData stores data specific to a thread.
 * @ingroup thread
 */
template<typename T>
class ThreadSpecificData
{
public:
  /**
   * Creates a new thread specific data object.
   */
  ThreadSpecificData() { ::pthread_key_create(&myKey, &deleter); }

  /**
   * Destroys the thread specific data object.
   */
  ~ThreadSpecificData() { ::pthread_key_delete(myKey); }

  /**
   * Sets new data. ThreadSpecificData takes ownership of the data, which must
   * have been allocated on the heap with new. The data will be deleted when
   * the thread terminates or when set is called the next time.
   */
  inline void set(const T* data);

  /**
   * Returns the data previously set(); or 0 if no data is set.
   */
  inline T* get() const;

private:
  static inline void deleter(void* data);
  pthread_key_t myKey;
};

template<typename T>
void ThreadSpecificData<T>::set(const T* data)
{
  delete get();
  ::pthread_setspecific(myKey, data);
}

template<typename T>
T* ThreadSpecificData<T>::get() const
{
  return static_cast<T*>(::pthread_getspecific(myKey));
}

template<typename T>
void ThreadSpecificData<T>::deleter(void* data)
{
  delete static_cast<T*>(data);
}

} // namespace Licq

#endif
