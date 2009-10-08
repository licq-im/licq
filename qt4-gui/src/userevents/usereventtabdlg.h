// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2009 Licq developers
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

#ifndef USEREVENTTABDLG_H
#define USEREVENTTABDLG_H

#include "config.h"

#include <QWidget>

class LicqUser;

namespace LicqQtGui
{
class TabWidget;
class UserEventCommon;

class UserEventTabDlg : public QWidget
{
  Q_OBJECT

public:
  UserEventTabDlg(QWidget* parent = 0, const char* name = 0);
  ~UserEventTabDlg();

  void addTab(UserEventCommon* tab, int index = -1);
  void selectTab(QWidget* tab);
  void replaceTab(QWidget* oldTab, UserEventCommon* newTab);
  bool tabIsSelected(QWidget* tab);
  bool tabExists(QWidget* tab);
  void updateConvoLabel(UserEventCommon* tab);
  void updateTabLabel(const LicqUser* u);
  void updateTabLabel(UserEventCommon* tab, const LicqUser* u = NULL);
  void setTyping(const LicqUser* u, int convoId);

#ifdef USE_KDE
  virtual void setIcon(const QPixmap& icon);
#endif

signals:
  void signal_done();

public slots:
  /**
   * Switch to tab index in action data
   *
   * @param action Action to get tab index from
   */
  void switchTab(QAction* action);

  void currentChanged(int index);
  void moveLeft();
  void moveRight();
  void removeTab(QWidget* tab);
  void setMsgWinSticky(bool sticky = true);

private slots:
  /**
   * Update keyboard shortcuts from configuration
   */
  void updateShortcuts();

private:
  TabWidget* myTabs;
  QAction* myTabSwitch01Action;
  QAction* myTabSwitch02Action;
  QAction* myTabSwitch03Action;
  QAction* myTabSwitch04Action;
  QAction* myTabSwitch05Action;
  QAction* myTabSwitch06Action;
  QAction* myTabSwitch07Action;
  QAction* myTabSwitch08Action;
  QAction* myTabSwitch09Action;
  QAction* myTabSwitch10Action;

  void updateTitle(QWidget* tab);
  void clearEvents(QWidget* tab);
  void saveGeometry();

  virtual void moveEvent(QMoveEvent* e);
  virtual void resizeEvent(QResizeEvent* e);
};

} // namespace LicqQtGui

#endif
