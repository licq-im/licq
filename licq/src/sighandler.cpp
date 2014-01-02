/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2014 Licq developers <licq-dev@googlegroups.com>
 *
 * Please refer to the COPYRIGHT file distributed with this source
 * distribution for the names of the individual contributors.
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

#include "config.h"
#include "gettext.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

#ifdef HAVE_PRCTL
#include <sys/prctl.h>
#endif

#include <licq/daemon.h>

using std::string;
using Licq::gDaemon;

static void licq_handle_sigsegv(int);
static void licq_handle_sigabrt(int);
static void licq_handle_sigchld(int);

// licq.cpp
void displayFatalError(const string& error, int useLicqLog);
void handleExitSignal(int signal);

void licq_install_signal_handlers()
{
  signal(SIGSEGV, &licq_handle_sigsegv);
  signal(SIGABRT, &licq_handle_sigabrt);
  /* writing to socket after remote disconnected causes this, ignore it, the
     disconnection will be handled, eventually */
  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, &licq_handle_sigchld);

  struct sigaction sa;
  sa.sa_handler = &handleExitSignal;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESETHAND;
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
}


void licq_handle_sigsegv(int s)
{
  if (s != SIGSEGV)
  {
    fprintf(stderr, tr("Unknown signal.\n"));
    return;
  }

  fprintf(stderr, tr("Licq Segmentation Violation Detected.\n"));

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

  // When shutting down, calls to pthread_rdwr_destroy_np() may trigger another
  // assert causing thish andler to be called again and backtrace files
  // overwritten.
  // Restore default signal handlers now, that way we won't get called again if
  // this happens.
  signal(SIGSEGV, SIG_DFL);
  signal(SIGABRT, SIG_DFL);

  /*
   * Use gdb to try to generate a backtrace for all threads and save
   * it in BASE_DIR/licq.backtrace.gdb.
   */
  string cmd = gDaemon.baseDir() + "licqcmd.gdb";
  string btfile2 = gDaemon.baseDir() + "licq.backtrace.gdb";
  FILE* cmdfile = fopen(cmd.c_str(), "w");
  if (cmdfile != NULL)
  {
    fprintf(cmdfile, "set logging file %s\n", btfile2.c_str());
    fprintf(cmdfile, "set pagination off\n");
    fprintf(cmdfile, "set logging overwrite\n");
    fprintf(cmdfile, "set logging redirect on\n");
    fprintf(cmdfile, "set logging on\n");
    fprintf(cmdfile, "thread apply all bt\n");
    fprintf(cmdfile, "thread apply all bt full\n");
    fprintf(cmdfile, "detach\n");
    fclose(cmdfile);

    fprintf(stderr, "\nUsing gdb to save backtrace to %s\n", btfile2.c_str());

#if HAVE_PRCTL && defined(PR_SET_PTRACER)
    // Allow gdb to attach to this process on Ubuntu. For more info, see
    // https://wiki.ubuntu.com/SecurityTeam/Roadmap/KernelHardening#ptrace
    ::prctl(PR_SET_PTRACER, getpid(), 0, 0, 0);
#endif

    char parentPid[16];
    snprintf(parentPid, 16, "%u", (int)getpid());

    pid_t child = fork();
    if (child == 0)
    {
      if (setsid() < 0)
        exit(EXIT_FAILURE);

      int ret = execlp("gdb", "gdb",
#ifdef __APPLE__
                       "-batch",
#else
                       "--batch-silent",
#endif
          "-x", cmd.c_str(), "--pid", parentPid, (char *)NULL);
      fprintf(stderr, "Failed to start gdb: %s\n", strerror(errno));
      exit(ret);
    }
    else if (child > 0)
    {
      int status;
      waitpid(child, &status, 0);

      fprintf(stderr, "gdb exited with exit code %d\n\n", status);

      // Include time in file
      FILE* file = fopen(btfile2.c_str(), "a");
      if (file != NULL)
      {
        fprintf(file, "\ntime: %lu\n", (unsigned long)time(NULL));
        fclose(file);
      }
    }

    unlink(cmd.c_str());
  }

#ifdef HAVE_BACKTRACE
  string btfile1 = gDaemon.baseDir() + "licq.backtrace";
  FILE* file = fopen(btfile1.c_str(), "w");
  if (file != NULL)
    fprintf(file, "time: %lu\n", time(NULL));

  fprintf(stderr, tr("Backtrace (saved in %s):\n"), btfile1.c_str());
  {
    const int size = 100;
    int i;
    void *array[size];
    int n = backtrace(array, size);
    char **res = backtrace_symbols(array, n);
    if (res == NULL)
      fprintf(stderr, tr("Failed to retrive backtrace symbols"));
    else
    {
      for (i = 0; i < n; i++)
      {
        fprintf(stderr, "%s\n", res[i]);
        if (file != NULL)
          fprintf(file, "%s\n", res[i]);
      }

      free(res);
    }
  }
  fprintf(stderr, tr("Attempting to generate core file.\n"));

  if (file != NULL)
    fclose(file);

#endif

  char error[1024];
  snprintf(error, 1024,
           "Licq has encountered a fatal error.\n"
           "Please report this error either by creating a new ticket at"
           "https://github.com/licq-im/licq/issues or by sending an e-mail to the"
           "mailing list licq-dev@googlegroups.com (you must be registered to be"
           "able to post, see http://groups.google.com/group/licq-dev).\n"
           "\n"
           "To help us debug the error, please include a full description of "
           "what you did when the error occurred. Additionally, please include "
           "the following files (if they exist):\n"
#ifdef HAVE_BACKTRACE
      "%s\n"
#endif
      "%s\n"
#ifdef LICQDAEMON_DEBUG_RW_MUTEX
           "%slicq.debug_rw_mutex\n"
#endif
           "\n"
           "Thanks, "
           "The Licq Team",
#ifdef HAVE_BACKTRACE
      btfile1.c_str(),
#endif
      btfile2.c_str()
#ifdef LICQDAEMON_DEBUG_RW_MUTEX
      , gDaemon.baseDir().c_str()
#endif
    );
  displayFatalError(error, 0);
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
