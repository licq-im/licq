/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998-2011 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#include "config.h"

#include <signal.h>
#include <cstdlib>
#include <cstring>

#include <locale>

#include "licq.h"
#include "plugin/pluginmanager.h"
#include "plugin/pluginthread.h"

#ifdef USE_SOCKS5
#define SOCKS
#define INCLUDE_PROTOTYPES
extern "C" {
#include <socks.h>
}
#endif

// Localization
#include "gettext.h"

// sighandler.cpp
void licq_install_signal_handlers();

using LicqDaemon::PluginThread;

static int threadArgc;
static char** threadArgv;
static int threadMain(PluginThread::Ptr mainThread)
{
  using LicqDaemon::gPluginManager;
  gPluginManager.setGuiThread(mainThread);

  int ret = 1;
  CLicq licq;
  if (licq.Init(threadArgc, threadArgv))
    ret = licq.Main();

  gPluginManager.setGuiThread(PluginThread::Ptr());
  return ret;
}

int main(int argc, char **argv)
{
#if ENABLE_NLS
  // prepare daemon localization
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
#endif
  
  // Make sure argv[0] is defined otherwise licq will crash if it is NULL
  if (argv[0] == NULL)
    argv[0] = strdup("licq");
#ifdef USE_SOCKS5
   SOCKSinit(argv[0]);
#endif

  licq_install_signal_handlers();

  threadArgc = argc;
  threadArgv = argv;

  // On some systems (e.g. Mac OS X) the GUI plugin must run in the main thread
  // and the daemon will run in a new thread.
#if __APPLE__
  return PluginThread::createWithCurrentThread(&threadMain);
#else
  return threadMain(PluginThread::Ptr());
#endif
}
