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

#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include <QSystemTrayIcon>

#include "dockicon.h"

namespace LicqQtGui
{
/**
 * Dock icon that uses system tray present in many window managers
 */
class SystemTrayIcon : public DockIcon
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param menu System menu to popup from tray icon
   */
  SystemTrayIcon(QMenu* menu);

  /**
   * Update number of unread events
   *
   * @param newMsg Number of unread user events
   * @param sysMsg Number of unread system events
   */
  virtual void updateIconMessages(int newMsg, int sysMsg);

  /**
   * Popup message from system tray
   *
   * @param title Popup title text
   * @param message Message text
   * @param icon Icon to display in the popup
   * @param timeout Time before hiding popup
   */
  virtual void popupMessage(QString title, QString message, const QPixmap& icon, int timeout);

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

private slots:
  /**
   * Tray has been activated by user
   *
   * @param reason Type of action that occured
   */
  virtual void trayActivated(QSystemTrayIcon::ActivationReason reason);

  /**
   * Called when new tooltip is available
   *
   * @param toolTip The tooltip text
   */
  void updateToolTip(const QString& toolTip);

private:
  /**
   * Timer has expired, used to blink unread events
   *
   * @param event Event object
   */
  virtual void timerEvent(QTimerEvent* event);

  /**
   * Update tray icon
   */
  void updateIcon();

  QSystemTrayIcon* myTrayIcon;
  bool myTimerToggle;
  bool myBlink;
};

} // namespace LicqQtGui

#endif
