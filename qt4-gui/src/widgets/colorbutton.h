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

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>

namespace LicqQtGui
{
class ColorButton : public QPushButton
{
  Q_OBJECT

public:
  ColorButton(QWidget* parent = NULL);
  QColor getColor() const;
  QString colorName() const;

public slots:
  void selectColor();
  void setColor(const QColor& color);

signals:
   void changed();
};

} // namespace LicqQtGui

#endif
