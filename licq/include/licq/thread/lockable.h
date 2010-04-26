#ifndef LICQ_LOCKABLE_H
#define LICQ_LOCKABLE_H

#include "readwritemutex.h"

#include <pthread.h>
#include <string>

// Thread lock constants
const unsigned short LOCK_N = 0;
const unsigned short LOCK_R = 1;
const unsigned short LOCK_W = 2;

namespace Licq
{

/**
 * Interface class for objects that need a public available read write mutex for data access
 */
class Lockable
{
public:
  Lockable() : myLockType(LOCK_R) {};

  /**
   * Lock object for access
   * Convenience function for easier migration from old mutex handling in LicqUser
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   */
  void Lock(unsigned short lockType = LOCK_R) const
  { if (lockType == LOCK_W) lockWrite(); else lockRead(); }

  /**
   * Release current lock for object
   * Convenience function for easier migration from old mutex handling in LicqUser
   */
  void Unlock() const
  { if (myLockType == LOCK_W) unlockWrite(); else unlockRead(); }

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
  void lockWrite() const { myMutex.lockWrite(); myLockType=LOCK_W; }

  /**
   * Release the write lock
   */
  void unlockWrite() const { myLockType=LOCK_R; myMutex.unlockWrite(); }

protected:
  mutable Licq::ReadWriteMutex myMutex;
  mutable unsigned short myLockType;
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
  ReadMutexGuard(T* object, bool locked = true)
    : myObject(object)
  {
    if (!locked)
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
  ReadMutexGuard<T> operator=(const ReadMutexGuard<T>&) {}

  T* myObject;
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
  WriteMutexGuard(T* object, bool locked = true)
    : myObject(object)
  {
    if (!locked)
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
  WriteMutexGuard<T> operator=(const WriteMutexGuard<T>&) {}

  T* myObject;
};

} // namespace Licq

#endif
