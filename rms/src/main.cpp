/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#include <stdio.h>
#include <unistd.h>

#include <licq/pluginbase.h>

#include "pluginversion.h"
#include "rms.h"

const char *LP_Usage()
{
  static const char usage[] =
    "Usage:  Licq [options] -p rms -- [ -h ] [ -d ]\n"
    "         -h          : help\n"
    "         -d          : start disabled\n";
  return usage;
}

const char *LP_Name()
{
  static const char name[] = "RMS";
  return name;
}


const char *LP_Description()
{
  static const char desc[] = "ICQ remote management server for telnet ICQ access";
  return desc;
}


const char *LP_Version()
{
  static const char version[] = PLUGIN_VERSION_STRING;
  return version;
}


const char *LP_ConfigFile()
{
  return "licq_rms.conf";
}


const char *LP_Status()
{
  static const char enabled[] = "RMS enabled";
  static const char disabled[] = "RMS disabled";
  return licqRMS->Enabled() ? enabled : disabled;
}


bool LP_Init(int argc, char **argv)
{
  //char *LocaleVal = new char;
  //LocaleVal = setlocale (LC_ALL, "");
  //bindtextdomain (PACKAGE, LOCALEDIR);
  //textdomain (PACKAGE);

  // parse command line for arguments
  bool bEnable = true;
  unsigned short nPort = 0;
  int i = 0;
  while( (i = getopt(argc, argv, "hdp:")) > 0)
  {
    switch (i)
    {
    case 'h':  // help
      puts(LP_Usage());
      return false;
    case 'd': // enable
      bEnable = false;
      break;
    case 'p':
      nPort = atol(optarg);
      break;
    }
  }
  licqRMS = new CLicqRMS(bEnable, nPort);
  return true;
}


int LP_Main()
{
  int nResult = licqRMS->Run();
  licqRMS->Shutdown();
  delete licqRMS;
  return nResult;
}


