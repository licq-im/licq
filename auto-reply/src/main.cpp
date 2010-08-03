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

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include <licq/pluginbase.h>

#include "autoreply.h"
#include "pluginversion.h"

CLicqAutoReply *licqAutoReply;

const char *LP_Usage()
{
  static const char usage[] =
    "Usage:  Licq [options] -p autoreply -- [ -h ] [ -e ] [ -l <status> ] [ -d ]\n"
    "         -h          : help\n"
    "         -e          : start enabled\n"
    "         -l <status> : log on at startup\n"
    "         -d          : delete messages after auto-replying\n";
  return usage;
}

const char *LP_Name()
{
  static const char name[] = "ICQ Auto Replyer";
  return name;
}

const char *LP_ConfigFile()
{
  return "licq_autoreply.conf";
}


const char *LP_Description()
{
  static const char desc[] = "ICQ message Auto Replyer";
  return desc;
}


const char *LP_Version()
{
  static const char version[] = PLUGIN_VERSION_STRING;
  return version;
}

const char *LP_Status()
{
  static const char enabled[] = "autoreply enabled";
  static const char disabled[] = "autoreply disabled";
  return licqAutoReply->Enabled() ? enabled : disabled;
}


bool LP_Init(int argc, char **argv)
{
  //char *LocaleVal = new char;
  //LocaleVal = setlocale (LC_ALL, "");
  //bindtextdomain (PACKAGE, LOCALEDIR);
  //textdomain (PACKAGE);

  // parse command line for arguments
  bool bEnable = false, bDelete = false;
  char *szStatus = NULL;
  int i = 0;
  while( (i = getopt(argc, argv, "dhel:")) > 0)
  {
    switch (i)
    {
      case 'h':  // help
        puts(LP_Usage());
        return false;
      case 'e': // enable
        bEnable = true;
        break;
      case 'l': //log on
        szStatus = strdup(optarg);
        break;
      case 'd': // delete new
        bDelete = true;
        break;
    }
  }
  licqAutoReply = new CLicqAutoReply(bEnable, bDelete, szStatus);
  if (szStatus != NULL) free(szStatus);
  return (licqAutoReply != NULL);
}


int LP_Main()
{
  int nResult = licqAutoReply->Run();
  licqAutoReply->Shutdown();
  delete licqAutoReply;
  return nResult;
}


