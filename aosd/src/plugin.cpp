/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2011 Licq developers
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

#include <cstdio>
#include <cstring>

#include <licq/event.h>
#include <licq/plugin/generalbase.h>
#include <licq/pluginsignal.h>
#include <licq/version.h>

#include "iface.h"
#include "pluginversion.h"

using namespace std;


AosdPlugin::AosdPlugin(Licq::GeneralPlugin::Params& p)
  : Licq::GeneralPlugin(p),
    blocked(false)
{
  // Empty
}

string AosdPlugin::name() const
{
  return "OSD";
}

string AosdPlugin::description() const
{
  return "OSD plugin based on libaosd";
}

string AosdPlugin::version() const
{
  return PLUGIN_VERSION_STRING;
}

string AosdPlugin::usage() const
{
  return "No CLI options exist.";
}

string AosdPlugin::configFile() const
{
  return "licq_aosd.ini";
}

bool AosdPlugin::isEnabled() const
{
  return !blocked;
}

bool AosdPlugin::init(int argc, char** argv)
{
  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-h") == 0)
    {
      printf("%s\n", usage().c_str());
      return false;
    }
  }

  return true;
}

int AosdPlugin::run()
{
  int pipe = getReadPipe();
  setSignalMask(
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
          Licq::PluginSignal* sig = popSignal();
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
          Licq::Event* ev = popEvent();
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

  delete iface;

  return 0;
}

void AosdPlugin::destructor()
{
  delete this;
}


Licq::GeneralPlugin* AosdPluginFactory(Licq::GeneralPlugin::Params& p)
{
  return new AosdPlugin(p);
}

LICQ_GENERAL_PLUGIN_DATA(&AosdPluginFactory);
