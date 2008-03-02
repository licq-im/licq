// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2006 Licq developers
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

#ifndef DOCKICON_H
#define DOCKICON_H

#include <QObject>

class QPixmap;

class ICQOwner;

namespace LicqQtGui
{
class DockIconWidget;

/**
 * Base class for dock icons
 * Only holds state and common functions needed by dock icon implementations
 */
class DockIcon : public QObject
{
  Q_OBJECT

public:
  /**
   * Constructor
   */
  DockIcon();

  /**
   * Destructor
   */
  virtual ~DockIcon();

  /**
   * Update status from daemon
   * Will fetch status and update status variables
   */
  virtual void updateIconStatus();

  /**
   * Update number of unread events
   * Will update message variables
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
  virtual void popupMessage(QString title, QString message, const QPixmap& icon, int timeout) = 0;

signals:
  /**
   * User has clicked on dock icon
   */
  void clicked();

  /**
   * User has middle clicked on dock icon
   */
  void middleClicked();

  /**
   * New tooltip is available
   * Emitted when myIcon isn't set
   */
  void newToolTip(QString tooltip);

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
  virtual void updateConfig() = 0;

protected:
  /**
   * Update tool tip text for dock icon
   */
  void updateToolTip();

  /**
   * Connects signals from myIcon onto this
   */
  void relayDockIconSignals();

  DockIconWidget* myIcon;
  int myNewMsg;
  int mySysMsg;
  unsigned long myFullStatus;
  unsigned short myStatus;
  bool myInvisible;
  QPixmap* myStatusIcon;
  QPixmap* myEventIcon;

private:
  QString myId;
  unsigned long myPpid;
};

} // namespace LicqQtGui

#endif
