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

#include "tabwidget.h"

#include <QWheelEvent>

using namespace LicqQtGui;

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
      emit middleClick(t);
  }

  myClickedTab = -1;
  QTabBar::mouseReleaseEvent(e);
}


TabWidget::TabWidget(QWidget* parent)
  : QTabWidget(parent)
{
  TabBar* tb = new TabBar(this);
  setTabBar(tb);
  connect(tb, SIGNAL(middleClick(int)), SLOT(slot_middleClick(int)));
}

void TabWidget::setTabColor(QWidget* tab, const QColor& color)
{
  int index = indexOf(tab);
  if (index != -1)
    tabBar()->setTabTextColor(index, color);
}

void TabWidget::setPreviousPage()
{
  dynamic_cast<TabBar*>(tabBar())->setPreviousTab();
}

void TabWidget::setNextPage()
{
  dynamic_cast<TabBar*>(tabBar())->setNextTab();
}

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
    emit middleClick(p);
}
