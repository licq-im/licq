#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licq_sighandler.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#if __GLIBC__ == 2 && __GLIBC_MINOR__ >= 1
#define HAVE_BACKTRACE
#endif

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif


void licq_handle_sigsegv(int);

/*
void licq_segv_handler(void (*f)(int, siginfo_t *, void *))
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = f;
  sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
  sigaction(SIGSEGV, &sa, NULL);
}
*/

void licq_segv_handler()
{
  signal(SIGSEGV, &licq_handle_sigsegv);
  /*struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = f;
  sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
  sigaction(SIGSEGV, &sa, NULL);*/
}



void licq_handle_sigsegv(int s)
{
  if (s != SIGSEGV)
  {
    fprintf(stderr, "Unknown signal.\n");
    return;
  }

  fprintf(stderr, "Licq Segmentation Violation Detected.\n");
  /*fprintf(stderr, "Fault Address: [0x%08lX]\n", (unsigned long)si->si_addr); */

#ifdef HAVE_BACKTRACE
  fprintf(stderr, "Backtrace:\n");
  {
    void *array[32];
    unsigned short i;
    int n = backtrace(array, 32);
    char **res = backtrace_symbols(array, n);
    for (i = 0; i < n; i++)
      fprintf(stderr, "%s\n", res[i]);

    /*array[0] = si->si_addr;
    res = backtrace_symbols(array, 1);
    fprintf(stderr, "%s\n", res[0]);*/
  }
  fprintf(stderr, "Attempting to generate core file.\n");
  pthread_kill_other_threads_np();
#endif

  abort();
}


