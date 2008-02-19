// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2006 Licq developers
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

#include "emoticonlabel.h"

#include <QKeyEvent>
#include <QMouseEvent>

using namespace LicqQtGui;

EmoticonLabel::EmoticonLabel(const QString& file, const QString& value, QWidget* parent)
  : QPushButton(parent),
    myValue(value)
{
  QPixmap icon = QPixmap(file);
  setIconSize(icon.size());
  setIcon(icon);
  setToolTip(value);
  setFixedSize(icon.size() + QSize(10, 10));
  setFlat(true);
}

void EmoticonLabel::mouseReleaseEvent(QMouseEvent* /* e */)
{
  if (underMouse())
    emit clicked(myValue);
}

void EmoticonLabel::keyPressEvent(QKeyEvent* e)
{
  if (e->modifiers() != Qt::NoModifier)
    return;

  switch (e->key())
  {
    case Qt::Key_Return: // Fall through
    case Qt::Key_Enter:
    case Qt::Key_Space:
      emit clicked(myValue);
      break;

    case Qt::Key_Up: // Fall through
    case Qt::Key_Down:
      emit move(this, e->key());
      break;

    default:
      QPushButton::keyPressEvent(e);
      break;
  }
}
