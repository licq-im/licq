/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Erik Johansson <erijo@licq.org>
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

#include "plugin.h"

#include "licq_constants.h"

#include <pthread.h>

using namespace LicqDaemon;

Plugin::Plugin(boost::shared_ptr<DynamicLibrary> lib,
               const std::string& prefix)
  : myLib(lib)
{
  loadSymbol(prefix + "_Main", myMain);
  loadSymbol(prefix + "_Main_tep", myMainThreadEntryPoint);
  loadSymbol(prefix + "_Exit", myExit);
  loadSymbol(prefix + "_Name", myName);
  loadSymbol(prefix + "_Version", myVersion);

  loadSymbol("LP_Id", myId);
  *myId = -1;

  ::pthread_mutex_init(&mySignalsMutex, NULL);
}

Plugin::~Plugin()
{
  ::pthread_mutex_destroy(&mySignalsMutex);
}

void Plugin::startThread(CICQDaemon* daemon)
{
  ::pthread_create(&myThread, NULL, myMainThreadEntryPoint, daemon);
}

int Plugin::joinThread()
{
  void* result;
  if (::pthread_join(myThread, &result) == 0)
  {
    int* retval = reinterpret_cast<int*>(result);
    int value = *retval;
    ::free(retval);
    return value;
  }

  return -1;
}

void Plugin::shutdown()
{
  myPipe.putChar(PLUGIN_SHUTDOWN);
}

void Plugin::pushSignal(LicqSignal* signal)
{
  ::pthread_mutex_lock(&mySignalsMutex);
  mySignals.push_back(signal);
  ::pthread_mutex_unlock(&mySignalsMutex);
  myPipe.putChar(PLUGIN_SIGNAL);
}

LicqSignal* Plugin::popSignal()
{
  LicqSignal* signal = NULL;
  ::pthread_mutex_lock(&mySignalsMutex);
  if (!mySignals.empty())
  {
    signal = mySignals.front();
    mySignals.pop_front();
  }
  ::pthread_mutex_unlock(&mySignalsMutex);
  return signal;
}
