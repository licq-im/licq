/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2006-2007 Licq developers
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

#ifndef SUPPORT_H
#define SUPPORT_H

#include <QWidget>

namespace LicqQtGui
{
/*
 * Class holding support functions for LICQ windows
 *
 * Contains various functions needed by multiple windows and dialogs
 */
class Support
{
public:
  /**
   * Changes the stickiness state of the window
   * Makes the window appear on all desktops,
   * i.e. makes it omnipresent.
   *
   * @param win The window id to change stickiness for
   * @param stick The desired stickiness state
   */
  static void changeWinSticky(WId win, bool stick);

  /**
   * Sets widget name
   * In case of running on X11, sets WM_WINDOW_ROLE and XClassHint.res_name
   * for a window to allow window managers properly recognize it.
   *
   * @param widget The widget to set a name for
   * @param name The name to be set
   */
  static void setWidgetProps(QWidget* widget, const QString& name);

  /**
   * Gets the name of the running window manager
   */
  static char* netWindowManagerName();

  /**
   * Hides the window from the pager and taskbar
   *
   * @param win The window id to ghost
   */
  static void ghostWindow(WId win);

  /**
   * Turns the window into a docked widget on X11
   *
   * @param win The window to dock
   * @return The handler to be passed to undockWindow()
   */
  static WId dockWindow(WId win);

  /**
   * Reverts the effect of the previous method and cleans up
   *
   * @param win The window to undock
   * @param handler The handler as returned by dockWindow()
   */
  static void undockWindow(WId win, WId handler);

  /**
   * Translates @a keyCode into @return XModifier
   */
  static unsigned keyToXMod(int keyCode);

  /**
   * Translates @a keyCode into @return XKeySymbol
   */
  static unsigned keyToXSym(int keyCode);

private:
  /**
   * Convenient wrapper to XGetWindowProperty
   *
   * The returned value must be deleted with XFree().
   */
  static unsigned char* getWindowProperty(WId win, const char* prop);
};

} // namespace LicqQtGui

#endif
