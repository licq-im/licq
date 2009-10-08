// -*- c-basic-offset: 2 -*-
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

#include "infofield.h"

#include <QDateTime>
#include <QKeyEvent>

#include "config/shortcuts.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::InfoField */

InfoField::InfoField(bool ro, QWidget* parent)
  : QLineEdit(parent)
{
  myBaseRo = palette().color(QPalette::Disabled, QPalette::Base);
  myBaseRw = palette().color(QPalette::Active, QPalette::Base);

  setReadOnly(ro);
}

void InfoField::setReadOnly(bool ro)
{
  QPalette pal(palette());
  pal.setColor(QPalette::Base, ro ? myBaseRo : myBaseRw);
  setPalette(pal);

  QLineEdit::setReadOnly(ro);
}

void InfoField::setText(const char* data)
{
  setText(QString::fromLocal8Bit(data));
}

void InfoField::setText(unsigned long data)
{
  setText(QString::number(data));
}

void InfoField::setDateTime(uint timestamp)
{
  if (timestamp == 0)
    setText(tr("Unknown"));
  else
    setText(QDateTime::fromTime_t(timestamp).toString());
}

void InfoField::keyPressEvent(QKeyEvent* event)
{
  Config::Shortcuts* shortcuts = Config::Shortcuts::instance();
  QKeySequence ks = QKeySequence(event->key() | event->modifiers());

  if (ks ==  shortcuts->getShortcut(Config::Shortcuts::InputClear))
    clear();

  QLineEdit::keyPressEvent(event);
}
