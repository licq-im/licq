#ifndef RDWR_H
#define RDWR_H

#include <pthread.h>
#include <string>

/*
 * Define DEBUG_RW_MUTEX and recompile the daemon and all plugins to
 * debug deadlocks. If a (potential) deadlock is discovered, the
 * daemon will print a message to stderr (and to the file
 * <basedir>/licq.debug_rw_mutex) and then abort.
 */
#define DEBUG_RW_MUTEX


#ifdef DEBUG_RW_MUTEX
#include <stdio.h>
#include <string.h>
#endif


/**
 * A read/write mutex
 */
class ReadWriteMutex
{
public:
  /**
   * Constructor
   */
  ReadWriteMutex();

  /**
   * Destructor
   */
  ~ReadWriteMutex();

  /**
   * Get a read lock
   */
  void lockRead();

  /**
   * Release a read lock
   */
  void unlockRead();

  /**
   * Get a write lock
   */
  void lockWrite();

  /**
   * Release a write lock
   */
  void unlockWrite();

  /**
   * Set mutex name, used for debugging
   *
   * @param new_name New name for mutex
   */
  void setName(const std::string& name)
#ifdef DEBUG_RW_MUTEX
  { myName = name; }
#else
  { }
#endif

private:
#ifdef DEBUG_RW_MUTEX
  void printUsers(FILE* file, bool writing) const;
  void debugCondWait(bool writing);
  void setWriter();
  void unsetWriter();
  void setReader();
  void unsetReader();
#endif

  int myNumReaders;
  bool myHasWriter;
  pthread_mutex_t myMutex;
  pthread_cond_t myLockFree;
#ifdef DEBUG_RW_MUTEX
#define RW_MUTEX_MAX_READERS 20
  pthread_t myWriterThread;
  pthread_t myReaderThreads[RW_MUTEX_MAX_READERS];
  std::string myName;
#endif
};


#endif
