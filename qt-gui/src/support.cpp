/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2006-2009 Licq developers
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpaintdevice.h>

#include "support.h"
#include "licq_log.h"
#ifdef USE_KDE
#include <kwin.h>
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

void CSupport::changeWinSticky(WId win, bool stick)
{
  gLog.Info("Setting Sticky state of window 0x%lx to %s.\n",
      static_cast<unsigned long>(win), stick ? "true" : "false");

#ifdef USE_KDE
  KWin::setOnAllDesktops(win, stick);
#else
  Display* dsp = QPaintDevice::x11AppDisplay();
  Window root  = DefaultRootWindow(dsp);

  unsigned long all = ~(0UL);
  unsigned long* current;

  if (!stick)
  {
    Atom retAtom;
    int retFormat;
    unsigned long retNItems;
    unsigned long retMoreBytes;

    XGetWindowProperty(dsp, root,
        XInternAtom(dsp, "_NET_CURRENT_DESKTOP", false),  // property
        0UL, sizeof(unsigned long), false,                // offset, size, del
        XInternAtom(dsp, "CARDINAL", false),              // property type
        &retAtom, &retFormat, &retNItems, &retMoreBytes,  // returned parameters
        (unsigned char**)&current);

    if (retFormat != 32 || retNItems != 1 || retMoreBytes != 0)
    {
      gLog.Info("Error reading current desktop property.");
      *current = 0UL;
    }
  }

  XEvent xev;
  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.display = dsp;
  xev.xclient.window = win;
  xev.xclient.message_type = XInternAtom(dsp, "_NET_WM_DESKTOP", false);
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = stick ? all : *current;

  XSendEvent(dsp, root, false,
      SubstructureRedirectMask | SubstructureNotifyMask, &xev);

  if (!stick)
    XFree(current);

#endif //USE_KDE
}
