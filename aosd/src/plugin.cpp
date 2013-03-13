/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2011, 2013 Licq developers <licq-dev@googlegroups.com>
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
#include <unistd.h>

#include <licq/event.h>
#include <licq/pluginsignal.h>
#include <licq/version.h>

#include "iface.h"

AosdPlugin::AosdPlugin()
  : myBlocked(false)
{
  // Empty
}

int AosdPlugin::run()
{
  int pipe = getReadPipe();
  setSignalMask(
      Licq::PluginSignal::SignalUser |
      Licq::PluginSignal::SignalLogon |
      Licq::PluginSignal::SignalLogoff);
  bool finita = false;

  Iface* iface = new Iface();

  while (!finita)
  {
    char msg;
    read(pipe, &msg, 1);

    switch (msg)
    {
      case PipeSignal:
        if (!myBlocked)
          iface->processSignal(popSignal().get());
        else
          popSignal();
        break;

      case PipeEvent:
        popEvent();
        break;

      case PipeShutdown:
        finita = true;
        break;

      case PipeDisable:
        myBlocked = true;
        break;

      case PipeEnable:
        iface->updateTextRenderData();
        myBlocked = false;
        break;

      default:
        break;
    }
  }

  delete iface;

  return 0;
}

bool AosdPlugin::isEnabled() const
{
  return !myBlocked;
}
