/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2012 Licq developers <licq-dev@googlegroups.com>
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

#include "calendar.h"

#include "config.h"

#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
#include <QLocale>
#elif defined(__GLIBC__)
#include <langinfo.h>
#endif

#include <QPainter>
#include <QTextCharFormat>


using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Calendar */

Calendar::Calendar(QWidget* parent)
    : QCalendarWidget(parent)
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
  setFirstDayOfWeek(QLocale::system().firstDayOfWeek());
#else

#ifdef __GLIBC__
  // Non-standard locale parameter available in gnu libc only
  int firstday = *nl_langinfo(_NL_TIME_FIRST_WEEKDAY);
  if (firstday > 0)
  {
    // locale data uses: 1=Sunday, 2=Monday..., 7=Saturday
    // Qt data uses: 1=Monday, 2=Tuesday..., 7=Sunday
    firstday -= 1;
    if (firstday == 0)
      firstday = Qt::Sunday;
    setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(firstday));
  }
  else
#endif
    setFirstDayOfWeek(Qt::Monday);

#endif
}

void Calendar::markDate(const QDate& date)
{
  QTextCharFormat textFormat = dateTextFormat(date);
  // Mark dates with bold
  textFormat.setFontWeight(QFont::Bold);
  // Background must be transparent to not overwrite the elipse
  textFormat.setBackground(Qt::transparent);
  setDateTextFormat(date, textFormat);
}

void Calendar::addMatch(const QDate& date)
{
  if (myMatches.contains(date))
    return;

  myMatches.append(date);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
  updateCell(date);
#else
  update();
#endif
}

void Calendar::clearMatches()
{
  myMatches.clear();
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
  updateCells();
#else
  update();
#endif
}

void Calendar::paintCell(QPainter* painter, const QRect& rect, const QDate& date) const
{
  QTextCharFormat format = dateTextFormat(date);
  if (format.fontWeight() == QFont::Bold)
  {
    painter->save();
    const int adjust = 1;
    QRect center = rect.adjusted(adjust, adjust, -adjust, -adjust);
    painter->setPen(Qt::NoPen);
    painter->setRenderHints(painter->renderHints() | QPainter::Antialiasing);
    painter->setBrush(myMatches.contains(date) ? Qt::green : Qt::yellow);
    painter->drawEllipse(center);
    painter->restore();
  }

  QCalendarWidget::paintCell(painter, rect, date);
}
