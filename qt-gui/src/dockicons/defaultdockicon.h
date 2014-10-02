/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2009 Licq developers
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

#ifndef DEFAULTDOCKICON_H
#define DEFAULTDOCKICON_H

class QMenu;
class QPixmap;

#include "dockicon.h"

namespace LicqQtGui
{
/**
 * Standard dock icon
 * Can be displayed in two sizes: 64x64 and 64x48.
 */
class DefaultDockIcon : public DockIcon
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param menu System menu to popup from tray icon
   */
  DefaultDockIcon(QMenu* menu);

  /**
   * Update number of unread events
   *
   * @param newMsg Number of unread user events
   * @param sysMsg Number of unread system events
   */
  virtual void updateIconMessages(int newMsg, int sysMsg);

  /**
   * Popup message from system tray
   * NOT IMPLEMENTED FOR DEFAULT ICON
   *
   * @param title Popup title text
   * @param message Message text
   * @param icon Icon to display in the popup
   * @param timeout Time before hiding popup
   */
  virtual void popupMessage(QString /* title */, QString /* message */, const QPixmap& /* icon */, int /* timeout */) {}

protected slots:
  /**
   * Updated the icon for current status
   */
  virtual void updateStatusIcon();

  /**
   * Updated the icon for current event
   */
  virtual void updateEventIcon();

  /**
   * Update configuration for dock icon
   */
  virtual void updateConfig();

private:
  /**
   * Draws a given @a icon into the 64x64 dock icon area
   *
   * @param icon The icon to draw
   */
  void drawIcon64(QPixmap* icon);

  bool myFortyEight;
};

} // namespace LicqQtGui

#endif
