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

#include "skinnablelabel.h"

#include <QMenu>
#include <QMouseEvent>
#include <QPainter>

#include "config/skin.h"

using namespace LicqQtGui;

SkinnableLabel::SkinnableLabel(const Config::LabelSkin& skin, QMenu* popupMenu, QWidget* parent)
  : QLabel(parent),
    myPopupMenu(popupMenu)
{
  applySkin(skin);
}

SkinnableLabel::SkinnableLabel(QMenu* popupMenu, QWidget* parent)
  : QLabel(parent),
    myPopupMenu(popupMenu)
{
}

void SkinnableLabel::applySkin(const Config::LabelSkin& skin)
{
  setFrameStyle(skin.frameStyle);
  setIndent(skin.margin);

  // Set colors
  QPalette pal = palette();
  if (skin.background.isValid())
  {
    setAutoFillBackground(skin.background.alpha() != 0);
    pal.setColor(QPalette::Window, skin.background);
  }
  if (skin.foreground.isValid())
    pal.setColor(QPalette::WindowText, skin.foreground);
  setPalette(pal);

  // Set background image
  myBackgroundImage = skin.pixmap;

  update();
}

void SkinnableLabel::setPrependPixmap(const QPixmap& p)
{
  if (!myAddPix.isNull())
    clearPrependPixmap();

  myAddPix = p;
  myAddIndent = indent();
  setIndent(indent() + p.width() + 2);

  update();
}

void SkinnableLabel::clearPrependPixmap()
{
  if (myAddPix.isNull())
    return;

  setIndent(myAddIndent);
  myAddPix = QPixmap();

  update();
}

void SkinnableLabel::addPixmap(const QPixmap& p)
{
  myPixmaps.push_back(p);
  if (myPixmaps.size() == 1)
    myStartingIndent = indent();
  update();
}

void SkinnableLabel::clearPixmaps()
{
  if (myPixmaps.empty())
    return;

  myPixmaps.clear();
  setIndent(myStartingIndent);

  update();
}

void SkinnableLabel::setBold(bool enable)
{
  QFont newFont(font());
  newFont.setBold(enable);
  setFont(newFont);
}

void SkinnableLabel::setItalic(bool enable)
{
  QFont newFont(font());
  newFont.setItalic(enable);
  setFont(newFont);
}

void SkinnableLabel::paintEvent(QPaintEvent* e)
{
  QPainter p(this);

  if (!myBackgroundImage.isNull())
    p.drawImage(0, 0, myBackgroundImage.toImage().scaled(width(), height()));

  if (!myAddPix.isNull())
    p.drawPixmap(myAddIndent, height() / 2 - myAddPix.height() / 2, myAddPix);

  if (myPixmaps.size())
  {
    QList<QPixmap>::iterator it;
    int i = indent();
    for (it = myPixmaps.begin(); it != myPixmaps.end(); it++)
    {
      p.drawPixmap(i, height() / 2 - it->height() / 2, *it);
      i += it->width() + 2;
    }
  }

  p.end();

  QLabel::paintEvent(e);
}

void SkinnableLabel::mousePressEvent(QMouseEvent* e)
{
  if(e->button() == Qt::MidButton)
  {
    emit doubleClicked();
  }
  else if (e->button() == Qt::RightButton)
  {
    if (myPopupMenu != NULL)
    {
      QPoint clickPoint(e->x(), e->y());
      myPopupMenu->popup(mapToGlobal(clickPoint));
    }
  }
  else
  {
    QLabel::mousePressEvent(e);
  }
}

void SkinnableLabel::mouseDoubleClickEvent(QMouseEvent* /* e */)
{
  emit doubleClicked();
}

void SkinnableLabel::wheelEvent(QWheelEvent* event)
{
  // Ignore the events for horizontal wheel movements
  if (event->orientation() != Qt::Vertical)
    return QLabel::wheelEvent(event);

  if (event->delta() < 0)
    emit wheelDown();
  else
    emit wheelUp();
  event->accept();
}
