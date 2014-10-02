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

#include "dockiconwidget.h"

#include "config.h"

#include <QCloseEvent>
#include <QBitmap>
#include <QMenu>
#include <QPainter>

#include "helpers/support.h"

#include "dockicon.h"

using namespace LicqQtGui;

DockIconWidget::DockIconWidget(QMenu* menu)
  : QWidget(),
    myMenu(menu),
    myFace(NULL)
{
  resize(64, 64);
  setAttribute(Qt::WA_NoSystemBackground, true);
  setAttribute(Qt::WA_AlwaysShowToolTips, true);
  Support::setWidgetProps(this, "DockIconWidget");
  myHandler = Support::dockWindow(winId());
  show();
}

DockIconWidget::~DockIconWidget()
{
  Support::undockWindow(winId(), myHandler);
  delete myFace;
}

void DockIconWidget::setFace(QPixmap* newFace, bool updateMask)
{
  if (newFace == NULL || newFace->isNull())
    return;

  delete myFace;
  myFace = new QPixmap(*newFace);
  resize(myFace->size());
  if (updateMask)
    setMask(myFace->mask());
  update();
}

QPixmap* DockIconWidget::face()
{
  if (myFace == NULL)
    return NULL;

  return new QPixmap(*myFace);
}

void DockIconWidget::closeEvent(QCloseEvent* event)
{
  event->ignore();
}

void DockIconWidget::mousePressEvent(QMouseEvent* event)
{
  switch (event->button())
  {
    case Qt::LeftButton:
      emit clicked();
      break;

    case Qt::MidButton:
      emit middleClicked();
      break;
 
    case Qt::RightButton:
      myMenu->popup(event->globalPos());
      break;

    default:
      break;
  }
}

void DockIconWidget::paintEvent(QPaintEvent* event)
{
  if (myFace == NULL)
    return;

  event->ignore();
  QPainter painter(this);
  painter.drawPixmap(0, 0, *myFace);
}
