#ifndef RDWR_H
#define RDWR_H

#include <pthread.h>

#ifdef connect
#undef connect
#endif
#ifdef accept
#undef accept
#endif

/********************************************************
 * An example source module to accompany...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 *     by Brad nichols, Dick Buttlar, Jackie Farrell
 *     O'Reilly & Associates, Inc.
 *
 ********************************************************
 * rdwr.h --
 *
 * Include file for reader/writer locks
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Define DEBUG_RW_MUTEX and recompile the daemon and all plugins to
 * debug deadlocks. If a (potential) deadlock is discovered, the
 * daemon will print a message to stderr (and to the file
 * <basedir>/licq.debug_rw_mutex) and then abort.
 */
// #define DEBUG_RW_MUTEX

typedef struct rdwr_var {
  int readers_reading;
  int writer_writing;
  pthread_mutex_t mutex;
  pthread_cond_t lock_free;
#ifdef DEBUG_RW_MUTEX
#define RW_MUTEX_MAX_READERS 20
  pthread_t writer;
  pthread_t readers[RW_MUTEX_MAX_READERS];
  char* name;
#endif
} pthread_rdwr_t;

typedef void * pthread_rdwrattr_t;

#define pthread_rdwrattr_default NULL;

#ifdef DEBUG_RW_MUTEX
#define pthread_rdwr_set_name(rdwrp, new_name) \
  do { free((rdwrp)->name); (rdwrp)->name = strdup((new_name)); } while (0)
#else
#define pthread_rdwr_set_name(rdwrp, new_name) do {} while (0)
#endif

int pthread_rdwr_init_np(pthread_rdwr_t *rdwrp, pthread_rdwrattr_t *attrp);
int pthread_rdwr_destroy_np(pthread_rdwr_t *rdwrp);
int pthread_rdwr_rlock_np(pthread_rdwr_t *rdwrp);
int pthread_rdwr_runlock_np(pthread_rdwr_t *rdwrp);
int pthread_rdwr_wlock_np(pthread_rdwr_t *rdwrp);
int pthread_rdwr_wunlock_np(pthread_rdwr_t *rdwrp);

#ifdef __cplusplus
}
#endif

#endif
