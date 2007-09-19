// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licq_sighandler.h"
#include "licq_constants.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#if __GLIBC__ == 2 && __GLIBC_MINOR__ >= 1
#define HAVE_BACKTRACE
#endif

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

// Localization
#include "gettext.h"

void licq_handle_sigsegv(int);
void licq_handle_sigabrt(int);
void licq_handle_sigchld(int);

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

void licq_signal_handler()
{
  signal(SIGSEGV, &licq_handle_sigsegv);
  signal(SIGABRT, &licq_handle_sigabrt);
  /* writing to socket after remote disconnected causes this, ignore it, the
     disconnection will be handled, eventually */
  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, &licq_handle_sigchld);
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
    fprintf(stderr, tr("Unknown signal.\n"));
    return;
  }

  fprintf(stderr, tr("Licq Segmentation Violation Detected.\n"));
  /*fprintf(stderr, "Fault Address: [0x%08lX]\n", (unsigned long)si->si_addr); */

  // SIGABRT handler will print backtrace
  abort();
}



void licq_handle_sigabrt(int s)
{
  if (s != SIGABRT)
  {
    fprintf(stderr, tr("Unknown signal.\n"));
    return;
  }

  /*
   * Use gdb to try to generate a backtrace for all threads and save
   * it in BASE_DIR/licq.backtrace.gdb.
   */
  char cmd[MAX_FILENAME_LEN];
  snprintf(cmd, MAX_FILENAME_LEN, "%s/licqcmd.gdb", BASE_DIR);
  FILE* cmdfile = fopen(cmd, "w");
  if (cmdfile != NULL)
  {
    fprintf(cmdfile, "set logging file %s/licq.backtrace.gdb\n", BASE_DIR);
    fprintf(cmdfile, "set pagination off\n");
    fprintf(cmdfile, "set logging overwrite\n");
    fprintf(cmdfile, "set logging redirect on\n");
    fprintf(cmdfile, "set logging on\n");
    fprintf(cmdfile, "thread apply all bt\n");
    fprintf(cmdfile, "thread apply all bt full\n");
    fprintf(cmdfile, "detach\n");
    fclose(cmdfile);

    char command[64 + MAX_FILENAME_LEN];
    snprintf(command, 64 + MAX_FILENAME_LEN,
             "gdb --batch-silent -x %s --pid %u", cmd, getpid());

    fprintf(stderr, "\nUsing gdb to save backtrace to %s/licq.backtrace.gdb\n"
            "Running: %s\n", BASE_DIR, command);
    int ret = system(command);
    fprintf(stderr, "%s (exit code %d)\n\n", (ret == 0) ? "done" : "failed", ret);

    unlink(cmd);
  }

#ifdef HAVE_BACKTRACE
  char filename[MAX_FILENAME_LEN];
  snprintf(filename, MAX_FILENAME_LEN, "%s/licq.backtrace", BASE_DIR);
  FILE* file = fopen(filename, "w");

  fprintf(stderr, tr("Backtrace (saved in %s):\n"), filename);
  {
    const int size = 100;
    int i;
    void *array[size];
    int n = backtrace(array, size);
    char **res = backtrace_symbols(array, n);
    for (i = 0; i < n; i++)
    {
      fprintf(stderr, "%s\n", res[i]);
      if (file != NULL)
        fprintf(file, "%s\n", res[i]);
    }

    free(res);
    /*array[0] = si->si_addr;
    res = backtrace_symbols(array, 1);
    fprintf(stderr, "%s\n", res[0]);*/
  }
  fprintf(stderr, tr("Attempting to generate core file.\n"));

  if (file != NULL)
    fclose(file);

  #ifdef HAVE_PTHREAD_KILL_OTHER_THREADS_NP
  // only available in LinuxThreads, not in NPTL
  pthread_kill_other_threads_np();
  #endif
#endif
}



void licq_handle_sigchld(int s)
{
  if (s != SIGCHLD)
  {
    fprintf(stderr, tr("Unknown signal.\n"));
    return;
  }

  wait(NULL);
}



