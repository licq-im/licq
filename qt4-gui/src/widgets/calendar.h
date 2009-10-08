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

#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include "config.h"

#include <QCalendarWidget>
#include <QDate>
#include <QList>

namespace LicqQtGui
{

/**
 * A calendar widget extended with function to mark dates
 */
class Calendar : public QCalendarWidget
{
  Q_OBJECT

public:
  /**
   * Contstructor
   *
   * @param parent Parent widget
   */
  Calendar(QWidget* parent = 0);

  /**
   * Destructor
   */
  virtual ~Calendar() {}

  /**
   * Mark a date in the calendar
   *
   * @param date Date to mark
   */
  void markDate(const QDate& date);

  /**
   * Mark a search match in the calendar
   * Note: Date must already be marked with markDate()
   *
   * @param date Date of the match
   */
  void addMatch(const QDate& date);

  /**
   * Clear all search matches
   */
  void clearMatches();

protected:
  /**
   * Draw contents of a date cell in the calendar
   *
   * @param painter Painter object
   * @param rect Cell area to draw in
   * @param date Date to draw
   */
  virtual void paintCell(QPainter* painter, const QRect& rect, const QDate& date) const;

private:
  QList<QDate> myMatches;
};

} // namespace LicqQtGui

#endif
