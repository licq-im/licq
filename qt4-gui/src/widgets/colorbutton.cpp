/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2006-2009 Licq developers
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

#include "colorbutton.h"

#ifndef USE_KDE
#include <QColorDialog>
#endif

using namespace LicqQtGui;

ColorButton::ColorButton(QWidget* parent)
  : COLORBUTTON_BASE(parent)
{
#ifndef USE_KDE
  setFixedSize(40, 20);
  connect(this, SIGNAL(clicked()), SLOT(selectColor()));
#endif
}

QString ColorButton::colorName() const
{
  return color().name();
}

#ifndef USE_KDE
QColor ColorButton::color() const
{
  return palette().color(backgroundRole());
}

void ColorButton::selectColor()
{
  QColor color = QColorDialog::getColor(palette().color(backgroundRole()), this);
  if (color.isValid())
    setColor(color);
}

void ColorButton::setColor(const QColor& color)
{
  QPalette pal(palette());
  pal.setColor(backgroundRole(), color);
  setPalette(pal);
  emit changed(color);
}
#endif
