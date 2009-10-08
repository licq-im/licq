// -*- c-basic-offset: 2 -*-
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

#ifndef DOCKICONWIDGET_H
#define DOCKICONWIDGET_H

#include <QWidget>

class QMenu;

namespace LicqQtGui
{
/**
 * Icon widget used by some dock icons
 */
class DockIconWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param menu Menu to popup
   */
  DockIconWidget(QMenu* menu);

  /**
   * Destructor
   */
  virtual ~DockIconWidget();

  /**
   * Set the new face icon
   *
   * @param newFace Icon image
   * @param updateMask Should mask be updated from the @a newFace
   */
  void setFace(QPixmap* newFace, bool updateMask = true);

  /**
   * Returns a copy of the current face
   */
  QPixmap* face();

signals:
  /**
   * User has clicked on dock icon
   */
  void clicked(); 
  
  /**
   * User has middle clicked on dock icon
   */
  void middleClicked();

private:
  /**
   * Intercept close attempts
   *
   * @param event Close event
   */
  virtual void closeEvent(QCloseEvent* event);

  /**
   * User has clicked on icon
   *
   * @param event Event object
   */
  virtual void mousePressEvent(QMouseEvent* event);

  /**
   * Repaint icon widget
   *
   * @param event Event object
   */
  virtual void paintEvent(QPaintEvent* event);

  QMenu* myMenu;
  QPixmap* myFace;
  WId myHandler;
};

} // namespace LicqQtGui

#endif
