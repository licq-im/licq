/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2013 Licq developers <licq-dev@googlegroups.com>
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
  else
    gPluginManager.cancelAllPlugins();

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
