/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2006 Licq developers
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

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabBar>
#include <QTabWidget>

namespace LicqQtGui
{

class TabBar : public QTabBar
{
  Q_OBJECT

public:
  TabBar(QWidget* parent = NULL);

  void setPreviousTab();
  void setNextTab();

signals:
  void middleClick(int t);

private:
  virtual void wheelEvent(QWheelEvent* e);
  virtual void mousePressEvent(QMouseEvent* e);
  virtual void mouseReleaseEvent(QMouseEvent* e);

  int myClickedTab;
};


class TabWidget : public QTabWidget
{
  Q_OBJECT

public:
  TabWidget(QWidget* parent = NULL);
  void setTabColor(QWidget* tab, const QColor& color);

  void setPreviousPage();
  void setNextPage();

signals:
  void middleClick(QWidget* p);

private slots:
  void slot_middleClick(int t);

private:
  virtual void wheelEvent(QWheelEvent* e);
};

} // namespace LicqQtGui

#endif
