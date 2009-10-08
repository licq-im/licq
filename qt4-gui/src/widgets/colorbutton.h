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

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include "config.h"

#ifdef USE_KDE
#include <KDE/KColorButton>
#define COLORBUTTON_BASE KColorButton
#else
#include <QPushButton>
#define COLORBUTTON_BASE QPushButton
#endif

namespace LicqQtGui
{
class ColorButton : public COLORBUTTON_BASE
{
  Q_OBJECT

public:
  ColorButton(QWidget* parent = NULL);
  QString colorName() const;
#ifndef USE_KDE
  QColor color() const;

public slots:
  void selectColor();
  void setColor(const QColor& color);

signals:
   void changed(const QColor& color);
#endif
};

} // namespace LicqQtGui

#endif
