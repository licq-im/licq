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

#include "config.h"

#include "tabwidget.h"

#ifdef USE_KDE
# include <QTabBar>
#else
# include <QWheelEvent>
#endif

using namespace LicqQtGui;

#ifndef USE_KDE
TabBar::TabBar(QWidget* parent)
  : QTabBar(parent)
{
  // Empty
}

void TabBar::setPreviousTab()
{
  int index = currentIndex() - 1;
  if (index < 0)
    index = count() - 1;

  setCurrentIndex(index);
}

void TabBar::setNextTab()
{
  int index = currentIndex() + 1;
  if (index >= count())
    index = 0;

  setCurrentIndex(index);
}

void TabBar::wheelEvent(QWheelEvent* e)
{
  if (count() <= 1 || !underMouse())
  {
    e->ignore();
    return;
  }

  if (e->delta() > 0)
    setPreviousTab();
  else
    setNextTab();

  e->accept();
}

void TabBar::mousePressEvent(QMouseEvent* e)
{
  if ((e->button() & Qt::MouseButtonMask) == Qt::MidButton)
    myClickedTab = tabAt(e->pos());

  QTabBar::mousePressEvent(e);
}

void TabBar::mouseReleaseEvent(QMouseEvent* e)
{
  if ((e->button() & Qt::MouseButtonMask) == Qt::MidButton)
  {
    int t = tabAt(e->pos());
    if (t > -1 && t == myClickedTab)
      emit mouseMiddleClick(t);
  }

  myClickedTab = -1;
  QTabBar::mouseReleaseEvent(e);
}
#endif


TabWidget::TabWidget(QWidget* parent)
  : TABWIDGET_BASE(parent)
{
#ifndef USE_KDE
  TabBar* tb = new TabBar(this);
  setTabBar(tb);
  connect(tb, SIGNAL(mouseMiddleClick(int)), SLOT(slot_middleClick(int)));
#endif
}

void TabWidget::setTabColor(QWidget* tab, const QColor& color)
{
  int index = indexOf(tab);
  if (index != -1)
#ifdef USE_KDE
    setTabTextColor(index, color);
#else
    tabBar()->setTabTextColor(index, color);
#endif
}

void TabWidget::setPreviousPage()
{
  int index = tabBar()->currentIndex() - 1;
  if (index < 0)
    index = tabBar()->count() - 1;

  tabBar()->setCurrentIndex(index);
}

void TabWidget::setNextPage()
{
  int index = tabBar()->currentIndex() + 1;
  if (index >= tabBar()->count())
    index = 0;

  tabBar()->setCurrentIndex(index);
}

#ifndef USE_KDE
void TabWidget::wheelEvent(QWheelEvent* e)
{
  if (count() <= 1)
  {
    e->ignore();
    return;
  }

  const QTabBar* tabs = tabBar();
  const bool cursorAboveTabBar = (e->y() < tabs->y());
  const bool cursorBelowTabBar = (e->y() > (tabs->y() + tabs->height()));
  if (cursorAboveTabBar || cursorBelowTabBar)
  {
    e->ignore();
    return;
  }

  if (e->delta() > 0)
    setPreviousPage();
  else
    setNextPage();

  e->accept();
}

void TabWidget::slot_middleClick(int t)
{
  QWidget* p = widget(t);
  if (p)
    emit mouseMiddleClick(p);
}
#endif
