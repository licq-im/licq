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

#include "generalplugin.h"

#include "licq_constants.h"

#include <cstring>

using namespace LicqDaemon;

GeneralPlugin::GeneralPlugin(DynamicLibrary::Ptr lib)
  : Plugin(lib, "LP"),
    myArgc(0),
    myArgv(NULL)
{
  loadSymbol("LP_Init", myInit);
  loadSymbol("LP_Status", myStatus);
  loadSymbol("LP_Description", myDescription);
  loadSymbol("LP_Usage", myUsage);
  loadSymbol("LP_BuildDate", myBuildDate);
  loadSymbol("LP_BuildTime", myBuildTime);

  try
  {
    // LP_ConfigFile is not required
    loadSymbol("LP_ConfigFile", myConfigFile);
  }
  catch (DynamicLibrary::Exception&)
  {
    myConfigFile = NULL;
  }

  ::pthread_mutex_init(&myEventsMutex, NULL);
}

GeneralPlugin::~GeneralPlugin()
{
  ::pthread_mutex_destroy(&myEventsMutex);

  for (int i = 0; i < myArgc - 1; ++i)
    ::free(myArgv[i]);
  delete[] myArgv;
}

bool GeneralPlugin::init(int argc, char** argv)
{
  myArgc = argc + 2;
  myArgv = new char*[myArgc];

  myArgv[0] = ::strdup(myLib->getName().c_str());
  for (int i = 0; i < argc; ++i)
    myArgv[i + 1] = ::strdup(argv[i]);
  myArgv[argc + 1] = NULL;

  // Set optind to 0 so plugins can use getopt
  optind = 0;

  return (*myInit)(myArgc, myArgv);
}

void GeneralPlugin::enable()
{
  myPipe.putChar('1');
}

void GeneralPlugin::disable()
{
  myPipe.putChar('0');
}

void GeneralPlugin::pushEvent(LicqEvent* event)
{
  ::pthread_mutex_lock(&myEventsMutex);
  myEvents.push_back(event);
  ::pthread_mutex_unlock(&myEventsMutex);
  myPipe.putChar(PLUGIN_EVENT);
}

LicqEvent* GeneralPlugin::popEvent()
{
  LicqEvent* event = NULL;
  ::pthread_mutex_lock(&myEventsMutex);
  if (!myEvents.empty())
  {
    event = myEvents.front();
    myEvents.pop_front();
  }
  ::pthread_mutex_unlock(&myEventsMutex);
  return event;
}
