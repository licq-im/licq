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

#ifndef THEMEDDOCKICON_H
#define THEMEDDOCKICON_H

class QMenu;

#include "dockicon.h"

namespace LicqQtGui
{
/**
 * Dock icon that can be use custom graphics defined externaly
 */
class ThemedDockIcon : public DockIcon
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param menu System menu to popup from tray icon
   */
  ThemedDockIcon(QMenu* menu);

  /**
   * Destructor
   */
  virtual ~ThemedDockIcon();

  /**
   * Update icon to reflect current status
   */
  virtual void updateIconStatus();

  /**
   * Update number of unread events
   *
   * @param newMsg Number of unread user events
   * @param sysMsg Number of unread system events
   */
  virtual void updateIconMessages(int newMsg, int sysMsg);

  /**
   * Popup message from system tray
   * NOT IMPLEMENTED FOR THEMED ICON
   *
   * @param title Popup title text
   * @param message Message text
   * @param icon Icon to display in the popup
   * @param timeout Time before hiding popup
   */
  virtual void popupMessage(QString /* title */, QString /* message */, const QPixmap& /* icon */, int /* timeout */) {}

protected slots:
  /**
   * Update configuration for dock icon
   */
  virtual void updateConfig();

private:
  QString myTheme;
  QPixmap* pixNoMessages;
  QPixmap* pixRegularMessages;
  QPixmap* pixSystemMessages;
  QPixmap* pixBothMessages;
  QPixmap* pixOnline;
  QPixmap* pixOffline;
  QPixmap* pixAway;
  QPixmap* pixNA;
  QPixmap* pixOccupied;
  QPixmap* pixDND;
  QPixmap* pixInvisible;
  QPixmap* pixFFC;

  void cleanup(bool initial = false);
};

} // namespace LicqQtGui

#endif
