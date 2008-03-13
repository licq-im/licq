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

#include "config.h"

#ifdef USE_KDE
# include <KDE/KTabWidget>
# define TABWIDGET_BASE KTabWidget
#else
# include <QTabBar>
# include <QTabWidget>
# define TABWIDGET_BASE QTabWidget
#endif

namespace LicqQtGui
{

#ifndef USE_KDE
class TabBar : public QTabBar
{
  Q_OBJECT

public:
  TabBar(QWidget* parent = NULL);

  void setPreviousTab();
  void setNextTab();

signals:
  void mouseMiddleClick(int t);

private:
  virtual void wheelEvent(QWheelEvent* e);
  virtual void mousePressEvent(QMouseEvent* e);
  virtual void mouseReleaseEvent(QMouseEvent* e);

  int myClickedTab;
};
#endif


class TabWidget : public TABWIDGET_BASE
{
  Q_OBJECT

public:
  TabWidget(QWidget* parent = NULL);
  void setTabColor(QWidget* tab, const QColor& color);

  void setPreviousPage();
  void setNextPage();

#ifdef USE_KDE
  /**
   * Overloaded to fix tab shortcuts
   */
  void setTabText(int index, const QString& label);

protected:
  /**
   * Overloaded to fix tab shortcuts
   */
  virtual void resizeEvent(QResizeEvent* event);

private:
  /**
   * Restore tab shortcuts after KTabWidget has broken them
   */
  void restoreShortcuts();
#else
signals:
  void mouseMiddleClick(QWidget* p);

private slots:
  void slot_middleClick(int t);

private:
  virtual void wheelEvent(QWheelEvent* e);
#endif
};

} // namespace LicqQtGui

#endif
