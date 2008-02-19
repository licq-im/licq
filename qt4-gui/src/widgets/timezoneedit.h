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

#ifndef TIMEZONEEDIT_H
#define TIMEZONEEDIT_H

#include <QSpinBox>
#include <QValidator>

namespace LicqQtGui
{
/**
 * Input field for timezones allowing both manual entering and stepping using
 * buttons.
 */
class TimeZoneEdit : public QSpinBox
{
  Q_OBJECT

public:
  /**
   * Constructor, creates a time zone input control
   *
   * @param parent Parent widget
   */
  TimeZoneEdit(QWidget* parent = NULL);

  /**
   * Set time zone
   *
   * @param data A time zone in format as used by licq daemon
   */
  void setData(char data);

  /**
   * Get current time zone
   *
   * @return Time zone currently selected in format as used by licq daemon
   */
  char data() const;

private:
  /**
   * Test if user input is a valid time zone or at least is the beginning of one
   *
   * @param input Entered text to test
   * @param pos Postition in string
   * @return Acceptable if input is a valid time zone, Intermediate if it is
   *         the beginning of one, otherwise Invalid.
   */
  QValidator::State validate(QString& input, int& pos) const;

  /**
   * Convert internal counter to a string representation of the zone
   *
   * @param v Numerical time zone value
   * @return Time zone as text
   */
  QString textFromValue(int v) const;

  /**
   * Convert time zone in text form to internal counter value
   *
   * @param text Time zone as text string
   * @return Numerical time zone value
   */
  int valueFromText(const QString& text) const;

  /// Value used to internally represent time zone "Unknown", must be lowest valid zone minus one
  static const int undefinedValue = -24;
};

} // namespace LicqQtGui

#endif
