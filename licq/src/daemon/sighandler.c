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

void licq_segv_handler(void (*f)(int))
{
  signal(SIGSEGV, f);
  /*struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = f;
  sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
  sigaction(SIGSEGV, &sa, NULL);*/
}


void signal_handler_managerThread(int s /*, siginfo_t *si, void *context */)
{
  if (s == SIGSEGV)
    licq_handle_sigsegv("Manager Thread" /*, si, context */);
}



void signal_handler_chatThread(int s /*, siginfo_t *si, void *context */)
{
  if (s == SIGSEGV)
    licq_handle_sigsegv("Chat Thread" /*, si, context */);
}


void signal_handler_ftThread(int s /*, siginfo_t *si, void *context */)
{
  if (s == SIGSEGV)
    licq_handle_sigsegv("File Transfer Thread" /*, si, context */);
}



void signal_handler_eventThread(int s /*, siginfo_t *si, void *context */)
{
  if (s == SIGSEGV)
    licq_handle_sigsegv("Event Thread" /*, si, context */);
}


void signal_handler_pingThread(int s /*, siginfo_t *si, void *context */)
{
  if (s == SIGSEGV)
    licq_handle_sigsegv("Ping Thread" /*, si, context */);
}


void signal_handler_monitorThread(int s /*, siginfo_t *si, void *context */)
{
  if (s == SIGSEGV)
    licq_handle_sigsegv("Monitor Thread" /*, si, context */);
}




void licq_handle_sigsegv(const char *s /*, siginfo_t *si, void *context */)
{
  fprintf(stderr, "Licq Segmentation Violation Detected [%s].\n", s);
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

  /* stupid line to stop useless warning */
  /*if ((unsigned long)context == 1) printf("c");*/

  abort();
}


