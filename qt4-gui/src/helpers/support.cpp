/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2006-2011 Licq developers
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

#include "support.h"

#include "config.h"

#include <cassert>

#include <QCoreApplication>

#if defined(USE_KDE)
#include <KDE/KWindowSystem>
#endif

#if defined(Q_WS_X11)
#include <QX11Info>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif /* defined(Q_WS_X11) */

#include <licq/logging/log.h>

using namespace LicqQtGui;

void Support::changeWinSticky(WId win, bool stick)
{
#if defined(USE_KDE) || defined(Q_WS_X11)
  Licq::gLog.info("Setting Sticky state of window 0x%lx to %s",
      static_cast<unsigned long>(win), stick ? "true" : "false");
#endif

#if defined(USE_KDE)
  KWindowSystem::setOnAllDesktops(win, stick);
#elif defined(Q_WS_X11)

  Display* dsp = QX11Info::display();
  Window root  = DefaultRootWindow(dsp);

  unsigned long desktop = ~(0UL);

  if (!stick)
  {
    unsigned char* tmp = getWindowProperty(root, "_NET_CURRENT_DESKTOP");

    if (tmp == NULL)
      Licq::gLog.info("Error reading current desktop property");
    else
    {
      desktop = *(reinterpret_cast<unsigned long*>(tmp));
      XFree(tmp);
    }
  }

  XEvent xev;
  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.display = dsp;
  xev.xclient.window = win;
  xev.xclient.message_type = XInternAtom(dsp, "_NET_WM_DESKTOP", False);
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = desktop;

  XSendEvent(dsp, root, False,
      SubstructureRedirectMask | SubstructureNotifyMask, &xev);

#endif /* defined(Q_WS_X11) */
}

void Support::setWidgetProps(QWidget* widget, const QString& name)
{
  assert(widget != NULL && !name.isEmpty());
  if (widget == NULL || name.isEmpty())
    return;

  widget->setObjectName(name);

#if defined(Q_WS_X11)
  if (widget->isWindow())
  {
    Display* display = widget->x11Info().display();
    WId win = widget->winId();

    XClassHint classHint;
    if (XGetClassHint(display, win, &classHint) == 0)
      return;

    XFree(classHint.res_name);
    classHint.res_name = name.toLocal8Bit().data();

    XSetClassHint(display, win, &classHint);

    XFree(classHint.res_class);
  }
#endif
}

char* Support::netWindowManagerName()
{
  char* name = NULL;

#if defined(Q_WS_X11)
  Display* dsp = QX11Info::display();
  WId root = DefaultRootWindow(dsp);

  unsigned char* retValue1 = NULL;
  unsigned char* retValue2 = NULL;

  retValue1 = getWindowProperty(root, "_NET_SUPPORTING_WM_CHECK");
  if (retValue1 == NULL)
    return NULL;

  WId win = *(reinterpret_cast<unsigned long*>(retValue1));

  retValue2 = getWindowProperty(win, "_NET_SUPPORTING_WM_CHECK");
  if (retValue2 == NULL)
  {
    XFree(retValue1);
    return NULL;
  }

  if (win != *(reinterpret_cast<unsigned long*>(retValue2)))
  {
    XFree(retValue1);
    XFree(retValue2);
    return NULL;
  }

  XFree(retValue2);
  retValue2 = NULL;

  retValue2 = getWindowProperty(win, "_NET_WM_NAME");
  XFree(retValue1);
  if (retValue2 == NULL)
    return NULL;

  name = strdup(reinterpret_cast<const char*>(retValue2));

  XFree(retValue2);
#endif

  return name;
}

void Support::ghostWindow(WId win)
{
#if defined(Q_WS_X11)
  Display* dsp = QX11Info::display();
  Window root  = DefaultRootWindow(dsp);

  Atom win_state = XInternAtom(dsp, "_NET_WM_STATE", False);
  Atom win_state_add = XInternAtom(dsp, "_NET_WM_STATE_ADD", False);
  Atom win_state_settings[] =
  {
    XInternAtom(dsp, "_NET_WM_STATE_SKIP_TASKBAR", False),
    XInternAtom(dsp, "_NET_WM_STATE_SKIP_PAGER", False)
  };
  XChangeProperty(dsp, win, win_state, XA_ATOM, 32, PropModeReplace,
      reinterpret_cast<unsigned char*>(&win_state_settings), 2);

  XEvent xev;
  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.display = dsp;
  xev.xclient.window = win;
  xev.xclient.message_type = win_state;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = win_state_add;
  xev.xclient.data.l[1] = win_state_settings[0];
  xev.xclient.data.l[2] = win_state_settings[1];

  XSendEvent(dsp, root, false,
      SubstructureRedirectMask | SubstructureNotifyMask, &xev);

#endif
}

WId Support::dockWindow(WId win)
{
  WId handler = 0;
#if defined(Q_WS_X11)
  // Create the new top-level window
  Display* dsp = QX11Info::display();
  Window root = XDefaultRootWindow(dsp);
  Window newWin = XCreateSimpleWindow(dsp, root, 0, 0, 64, 64, 0, 0, 0);

  // Reparent our widget to newly created window
  XReparentWindow(dsp, win, newWin, 0, 0);

  // Replicate WM_CLASS attribute
  XClassHint classhint;
  XGetClassHint(dsp, win, &classhint);
  XSetClassHint(dsp, newWin, &classhint);

  // Replicate command line arguments
  QStringList args = QCoreApplication::arguments();
  QVector<char*> argv;
  while (!args.empty())
  {
    QString arg = args.takeFirst();
    argv << arg.toLocal8Bit().data();
  }
  XSetCommand(dsp, newWin, argv.data(), argv.count());

  // Make the new window be a group leader and
  // point it to the window representing its icon 
  XWMHints* hints = XAllocWMHints();
  hints->icon_window = win;
  hints->window_group = newWin;
  hints->initial_state = WithdrawnState;
  hints->flags = IconWindowHint | WindowGroupHint | StateHint;
  XSetWMHints(dsp, newWin, hints);
  XFree(hints);

  // And finally, show the window
  XMapWindow(dsp, newWin);

  handler = newWin;
#endif
  return handler;
}

void Support::undockWindow(WId win, WId handler)
{
  if (handler == 0)
    return;

#if defined(Q_WS_X11)
  Display* dsp = QX11Info::display();
  Window root = XDefaultRootWindow(dsp);

  XUnmapWindow(dsp, win);
  XUnmapWindow(dsp, handler);

  // Reparent both windows to the root and hide them
  XReparentWindow(dsp, win, root, -100, -100);
  XReparentWindow(dsp, handler, root, -100, -100);

  // And destroy the handler
  XDestroyWindow(dsp, handler);
#endif
}

#if defined(Q_WS_X11)
unsigned Support::keyToXMod(int keyCode)
{
  unsigned mod = 0;

  if (keyCode != 0)
  {
    if (keyCode & Qt::SHIFT)
      mod |= ShiftMask;
    if (keyCode & Qt::CTRL)
      mod |= ControlMask;
    if (keyCode & Qt::ALT)
      mod |= Mod1Mask;
    if (keyCode & Qt::META)
      mod |= Mod4Mask;
  }

  return mod;
}

unsigned Support::keyToXSym(int keyCode)
{
  unsigned keysym = 0;

  char* toks[4];
  char* next_tok;
  char sKey[100];
  int nb_toks = 0;
  QString s = QKeySequence(keyCode);

  if (s.isEmpty())
    return keysym;

  qstrncpy(sKey, s.toAscii(), sizeof(sKey));
  next_tok = strtok(sKey, "+");

  if (next_tok == 0L)
    return 0;

  do
  {
    toks[nb_toks] = next_tok;
    nb_toks++;
    if (nb_toks >= 4)
      return 0;
    next_tok = strtok(0L, "+");
  } while (next_tok != 0L);

  // Test for exactly one key (other tokens are accelerators)
  // Fill the keycode with infos
  bool keyFound = false;

  for (int i = 0; i < nb_toks; i++)
  {
    if (qstricmp(toks[i], "SHIFT") != 0 &&
        qstricmp(toks[i], "CTRL")  != 0 &&
        qstricmp(toks[i], "ALT")   != 0 &&
        qstricmp(toks[i], "META")  != 0)
    {
      if (keyFound)
        return 0;
      keyFound = true;
      QString l = toks[i];
      l = l.toLower();
      keysym = XStringToKeysym(l.toAscii());
      if (keysym == NoSymbol)
        keysym = XStringToKeysym(toks[i]);
      if (keysym == NoSymbol)
        return 0;
    }
  }

  return keysym;
}

void Support::grabKey(Display* dsp, Qt::HANDLE rootWin, int key, bool enable)
{
  KeyCode keycode = XKeysymToKeycode(dsp, keyToXSym(key));
  unsigned basemod = keyToXMod(key);

  // No extra modifiers
  XGrabKey(dsp, keycode, basemod, rootWin, enable, GrabModeAsync, GrabModeSync);

  // Caps Lock
  XGrabKey(dsp, keycode, basemod | LockMask, rootWin, enable, GrabModeAsync, GrabModeSync);

  // Num Lock
  XGrabKey(dsp, keycode, basemod | Mod2Mask, rootWin, enable, GrabModeAsync, GrabModeSync);

  // Caps Lock & Num Lock
  XGrabKey(dsp, keycode, basemod | LockMask | Mod2Mask, rootWin, enable, GrabModeAsync, GrabModeSync);
}

unsigned char* Support::getWindowProperty(WId win, const char* prop)
{
  Display* dsp = QX11Info::display();

  // We inhibit new Atom creation since if you request for it
  // then such Atom most probably exists already.
  // Otherwise, no surprise we return NULL here.
  Atom reqAtom = XInternAtom(dsp, prop, True);

  if (reqAtom == None)
    return NULL;

  int retCheck = None;
  Atom retType = None;
  int retFormat = 0;
  unsigned long retItems = 0UL;
  unsigned long retMoreBytes = 0UL;
  unsigned char* retValue = NULL;

  // Check if the property exists and calculate its length.
  retCheck = XGetWindowProperty(dsp, win,
      reqAtom, 0L, 0L, False, AnyPropertyType,
      &retType, &retFormat, &retItems, &retMoreBytes, &retValue);

  // The value is most probably empty, since we requested to read
  // only 0L length, thus, it's just useless...
  if (retValue != NULL)
  {
    XFree(retValue);
    retValue = NULL;
  }

  if (retCheck != Success ||
      retType == None ||
      retMoreBytes == 0)
    return NULL;

  // These are not needed for now.
  retCheck = None;
  retFormat = 0;
  retItems = 0UL;

  // Convert the byte length into 32bit multiples.
  if (retMoreBytes % 4 != 0)
    retMoreBytes += 4 - retMoreBytes % 4;
  retMoreBytes /= 4;

  // Now request the actual property value with correct length and type.
  retCheck = XGetWindowProperty(dsp, win,
      reqAtom, 0L, retMoreBytes, False, retType,
      &retType, &retFormat, &retItems, &retMoreBytes, &retValue);

  if (retCheck != Success ||
      retMoreBytes != 0)
  {
    if (retValue != NULL)
      XFree(retValue);
    return NULL;
  }

  return retValue;
}
#endif /* defined(Q_WS_X11) */
