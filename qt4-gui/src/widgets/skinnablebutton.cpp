// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Licq developers
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

#include "skinnablebutton.h"

#include <QMouseEvent>
#include <QPainter>

#include "config/skin.h"

using namespace LicqQtGui;

SkinnableButton::SkinnableButton(const Config::ButtonSkin& skin, QString defaultText, QWidget* parent)
  : QPushButton(parent),
    myDefaultText(defaultText),
    myPressedModifiers(Qt::NoModifier)
{
  applySkin(skin);
}

SkinnableButton::SkinnableButton(QString defaultText, QWidget* parent)
  : QPushButton(defaultText, parent),
    myDefaultText(defaultText),
    myPressedModifiers(Qt::NoModifier)
{
}

void SkinnableButton::applySkin(const Config::ButtonSkin& skin)
{
  // Load new images, set to null pixmaps if any fails to load
  myNormalImage = skin.pixmapUpNoFocus;
  myHighlightedImage = skin.pixmapUpFocus;
  myPressedImage = skin.pixmapDown;

  // Set button text (only used if images are not used or not loadable)
  setText(skin.caption.isNull() ? myDefaultText : skin.caption);

  // Set colors
  QPalette pal;
  if (skin.background.isValid())
    pal.setColor(QPalette::Window, skin.background);
  if (skin.foreground.isValid())
    pal.setColor(QPalette::Text, skin.foreground);
  setPalette(pal);
}

Qt::KeyboardModifiers SkinnableButton::modifiersWhenPressed()
{
  Qt::KeyboardModifiers b = myPressedModifiers;
  myPressedModifiers = Qt::NoModifier;
  return b;
}

void SkinnableButton::mousePressEvent(QMouseEvent* e)
{
  myPressedModifiers = e->modifiers();
  QPushButton::mousePressEvent(e);
}

void SkinnableButton::enterEvent(QEvent* e)
{
  // Trigger a paintEvent to redraw button in new state
  update();
  QPushButton::enterEvent(e);
}

void SkinnableButton::leaveEvent(QEvent* e)
{
  // Trigger a paintEvent to redraw button in new state
  update();
  QPushButton::leaveEvent(e);
}

void SkinnableButton::paintEvent(QPaintEvent* e)
{
  QPixmap* image;

  // Check button state to select which image to use
  if (isDown())
    image = &myPressedImage;
  else if (underMouse())
    image = &myHighlightedImage;
  else
    image = &myNormalImage;

  // Use default paint function if no image is defined for this state
  if (image->isNull())
  {
    QPushButton::paintEvent(e);
    return;
  }

  QPainter p(this);
  p.drawPixmap(0, 0, *image);
}
