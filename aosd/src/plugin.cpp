/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2010 Licq developers
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

#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/icq.h>
#include <licq/plugin.h>
#include <licq/pluginbase.h>
#include <licq/pluginmanager.h>
#include <licq/pluginsignal.h>

#include "iface.h"
#include "pluginversion.h"

using Licq::gPluginManager;

bool blocked = false;

const char* LP_Name()
{
  static const char name[] = "OSD";
  return name;
}

const char* LP_Description()
{
  static const char desc[] = "OSD plugin based on libaosd";
  return desc;
}

const char* LP_Version()
{
  static const char version[] = PLUGIN_VERSION_STRING;
  return version;
}

const char* LP_Usage()
{
  static const char usage[] = "No CLI options exist.";
  return usage;
}

const char* LP_ConfigFile()
{
  static const char file[] = "licq_aosd.ini";
  return file;
}

const char* LP_Status()
{
  static const char enabled[] = "enabled";
  static const char disabled[] = "disabled";
  return blocked ? disabled : enabled;
}

bool LP_Init(int argc, char** argv)
{
  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-h") == 0)
    {
      printf("%s\n", LP_Usage());
      return false;
    }
  }

  return true;
}

int LP_Main()
{
  int pipe = gPluginManager.registerGeneralPlugin(
      Licq::PluginSignal::SignalUser |
      Licq::PluginSignal::SignalLogon |
      Licq::PluginSignal::SignalLogoff);
  bool finita = false;
  char msg[3];

  Iface* iface = new Iface();

  while (!finita)
  {
    read(pipe, msg, 1);

    switch (msg[0])
    {
      case Licq::GeneralPlugin::PipeSignal:
        {
          Licq::PluginSignal* sig = Licq::gDaemon.popPluginSignal();
          if (sig != NULL)
          {
            if (!blocked)
              iface->processSignal(sig);
            delete sig;
            sig = NULL;
          }
        }
        break;

      case Licq::GeneralPlugin::PipeEvent:
        {
          Licq::Event* ev = Licq::gDaemon.PopPluginEvent();
          if (ev != NULL)
            delete ev;
        }
        break;

      case Licq::GeneralPlugin::PipeShutdown:
        finita = true;
        break;

      case Licq::GeneralPlugin::PipeDisable:
        blocked = true;
        break;

      case Licq::GeneralPlugin::PipeEnable:
        iface->updateTextRenderData();
        blocked = false;
        break;

      default:
        break;
    }
  }

  gPluginManager.unregisterGeneralPlugin();

  delete iface;

  return 0;
}

/* vim: set ts=2 sw=2 et : */
