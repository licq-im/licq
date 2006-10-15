/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2006-2006 Licq developers
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

#include "support.h"
#include "licq_log.h"
#ifdef USE_KDE
#include <kwin.h>
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

void CSupport::changeWinSticky(WId win, bool _bStick)
{
  // Philipp Kolmann: 2006-06-06
  // Code from
  // http://lists.trolltech.com/qt-interest/2006-01/thread00352-0.html
  // provided by Bob Shaffer II qt-interest@bobshaffer.net

  gLog.Info("Setting Sticky state of window 0x%x to %d.\n",
    (unsigned int) win, _bStick);

#ifdef USE_KDE
  KWin::setOnAllDesktops(win, _bStick);
#else
  // connect to display
  Display *display = XOpenDisplay("");

  // root window receives these events
  Window rootwinid = DefaultRootWindow(display);

  // initialize necessary atoms
  Atom StateAtom = XInternAtom(display, "_WIN_STATE", false);
  Atom LayerAtom = XInternAtom(display, "_WIN_LAYER", false);

  // construct and send (un)stick event
  XEvent xev;
  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.window = win;
  xev.xclient.message_type = StateAtom;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = (1<<0);
  xev.xclient.data.l[1] = (_bStick?(1<<0):0);
  XSendEvent(display, rootwinid, False, SubstructureRedirectMask |
    SubstructureNotifyMask, &xev);

  // construct and send layer setting event
  // fyi: layers are 0=desktop 2=below 4=normal 6=above 8=dock 10=abovedock
  xev.xclient.type = ClientMessage;
  xev.xclient.window = win;
  xev.xclient.message_type = LayerAtom;
  xev.xclient.format = 32;
  // Put it to 4=normal for now
  xev.xclient.data.l[0] = 4;
  XSendEvent(display, rootwinid, False, SubstructureRedirectMask |
      SubstructureNotifyMask, &xev);

  // close display
  XCloseDisplay(display);
#endif //USE_KDE
}
