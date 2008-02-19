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

#include "timezoneedit.h"

#include <QRegExp>

#include <licq_user.h>

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::TimeZoneEdit */

TimeZoneEdit::TimeZoneEdit(QWidget* parent)
  : QSpinBox(parent)
{
  setMinimum(-24);
  setMaximum(24);

  // The world is round so let timezones wrap
  setWrapping(true);

  // Plus and minus seems more fitting than up and down
  setButtonSymbols(QSpinBox::PlusMinus);

  // Allow the value to be undefined as well. This will replace the lowest value (-24)
  setSpecialValueText(tr("Unknown"));
}

void TimeZoneEdit::setData(char data)
{
  // The spinbox uses the lowest value to mark the undefined state but the constant is some other value so we need to change it
  // For all defined values, the sign is inverted
  setValue(data == TIMEZONE_UNKNOWN ? undefinedValue : static_cast<int>(-data));
}

char TimeZoneEdit::data() const
{
  int v = value();
  if (v == undefinedValue)
    return TIMEZONE_UNKNOWN;
  return static_cast<char>(-v);
}

QValidator::State TimeZoneEdit::validate(QString& input, int& /* pos */) const
{
  // First check for the undefined value
  if (input == specialValueText())
    return QValidator::Acceptable;
  if (specialValueText().startsWith(input))
    return QValidator::Intermediate;

  // Check if this is a complete valid timezone
  QRegExp rxValid("^GMT[\\+\\-](1[012]|\\d)[03]0$");
  if (rxValid.indexIn(input) > -1)
    return QValidator::Acceptable;

  // Check if this is anything close to a timezone
  QRegExp rxPossible("^G?M?T?[\\+\\-]?\\d*$");
  if (rxPossible.indexIn(input) > -1)
    return QValidator::Intermediate;

  return QValidator::Invalid;
}

QString TimeZoneEdit::textFromValue(int v) const
{
  // The internal value in the spinbox is 30min intervals so convert it to something more readable
  return QString("GMT%1%2%3").arg(v < 0 ? "-" : "+").arg(abs(v) / 2).arg(v % 2 ? "30" : "00");
}

int TimeZoneEdit::valueFromText(const QString& text) const
{
  // The user entered something so now we must try and convert it back to the internal int
  QRegExp rx("^GMT(\\+|-)(\\d+)(0|3)0$");
  if (rx.indexIn(text) == -1)
    return undefinedValue;

  int ret = rx.cap(2).toInt() * 2;
  if (rx.cap(3) == "3")
    ret++;
  if (rx.cap(1) == "-")
    ret = -ret;
  return ret;
}
