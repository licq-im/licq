/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2009-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_LOCKABLE_H
#define LICQ_LOCKABLE_H

#include "readwritemutex.h"

#include <pthread.h>
#include <string>


namespace Licq
{

/**
 * Interface class for objects that need a public available read write mutex for data access
 */
class Lockable
{
public:
  Lockable()
  { /* Empty */ }

  /**
   * Acquire a read lock
   */
  void lockRead() const { myMutex.lockRead(); }

  /**
   * Release a read lock
   */
  void unlockRead() const { myMutex.unlockRead(); }

  /**
   * Acquire the write lock
   */
  void lockWrite() const
  { myMutex.lockWrite(); }

  /**
   * Release the write lock
   */
  void unlockWrite() const
  { myMutex.unlockWrite(); }

protected:
  mutable ReadWriteMutex myMutex;
};


/**
 * Read guard for mutexes
 * Class T should inherit from class Lockable
 */
template <class T> class ReadMutexGuard
{
public:
  /**
   * Constructor
   *
   * @param object Object to guard mutex for
   * @param locked True if object is already locked or false if guard should lock
   */
  ReadMutexGuard(const T* object, bool locked = false)
    : myObject(object)
  {
    if (myObject != NULL && !locked)
      myObject->lockRead();
  }

  /**
   * Copy constructor
   * Guarding ownership will be transferred to this guard
   *
   * @param guard Read guard to get object from
   */
  ReadMutexGuard(ReadMutexGuard<T>* guard)
    : myObject(guard->myObject)
  {
    guard->release();
  }

  /**
   * Destructor
   * Will unlock mutex if still held
   */
  virtual ~ReadMutexGuard()
  {
    if (myObject != NULL)
      unlock();
  }

  /**
   * Unlock the mutex
   * Note: This will make the guard forget about the guarded object
   */
  void unlock()
  {
    if (myObject == NULL)
      return;
    myObject->unlockRead();
    myObject = NULL;
  }

  /**
   * Release guard of object without unlocking the mutex
   */
  void release()
  {
    myObject = NULL;
  }

  /**
   * Check if guard is holding a mutex
   *
   * @return True if guard is active and holds a valid locked object
   */
  bool isLocked() const { return myObject != NULL; }

  // Access operators
  const T* operator*() const { return myObject; }
  const T* operator->() const { return myObject; }

private:
  // Stop assignment operator from being used by misstake
  ReadMutexGuard<T> operator=(const ReadMutexGuard<T>&);

  const T* myObject;
};


/**
 * Write guard for mutexes
 * Class T should inherit from class Lockable
 */
template <class T> class WriteMutexGuard
{
public:
  /**
   * Constructor
   *
   * @param object Object to guard mutex for
   * @param locked True if object is already locked or false if guard should lock
   */
  WriteMutexGuard(T* object, bool locked = false)
    : myObject(object)
  {
    if (myObject != NULL && !locked)
      myObject->lockWrite();
  }

  /**
   * Copy constructor
   * Guarding ownership will be transferred to this guard
   *
   * @param guard Write guard to get object from
   */
  WriteMutexGuard(WriteMutexGuard<T>* guard)
    : myObject(guard->myObject)
  {
    guard->release();
  }

  /**
   * Destructor
   * Will unlock mutex if still held
   */
  virtual ~WriteMutexGuard()
  {
    if (myObject != NULL)
      unlock();
  }

  /**
   * Unlock the mutex
   * Note: This will make the guard forget about the guarded object
   */
  void unlock()
  {
    if (myObject == NULL)
      return;
    myObject->unlockWrite();
    myObject = NULL;
  }

  /**
   * Release guard of object without unlocking the mutex
   */
  void release()
  {
    myObject = NULL;
  }

  /**
   * Check if guard is holding a mutex
   *
   * @return True if guard is active and holds a valid locked object
   */
  bool isLocked() const { return myObject != NULL; }

  // Access operators
  T* operator*() const { return myObject; }
  T* operator->() const { return myObject; }

private:
  // Stop assignment operator from being used by misstake
  WriteMutexGuard<T> operator=(const WriteMutexGuard<T>&);

  T* myObject;
};

} // namespace Licq

#endif
