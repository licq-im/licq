// -*- c-basic-offset: 2 -*-
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

// written by Graham Roff <graham@licq.org>
// contributions by Dirk A. Mueller <dirk@licq.org>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qglobal.h>
#if QT_VERSION < 300

#include <qfont.h>
#include <qpainter.h>
#include <qaccel.h>

#include "mlview2.h"

MLView::MLView (QWidget* parent, const char *name)
  : QMultiLineEdit(parent, name)
{
  setWordWrap(WidgetWidth);
  setWrapPolicy(AtWhiteSpace);
  setReadOnly(true);
}


void MLView::appendNoNewLine(const QString& s)
{
  if (!atEnd()) GotoEnd();
  QMultiLineEdit::insert(s);
}

void MLView::append(const QString& s)
{
#if QT_VERSION < 300
  appendNoNewLine(s + "\n");
#endif
}

void MLView::GotoEnd()
{
#if QT_VERSION < 300
  setCursorPosition(numLines() - 1, lineLength(numLines() - 1) - 1);
#endif
}


void MLView::setBackground(const QColor& c)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QColorGroup::Base, c);
  pal.setColor(QPalette::Inactive, QColorGroup::Base, c);

  setPalette(pal);
}


// -----------------------------------------------------------------------------


void MLView::setForeground(const QColor& c)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QColorGroup::Text, c);
  pal.setColor(QPalette::Inactive, QColorGroup::Text, c);

  setPalette(pal);
}

// -----------------------------------------------------------------------------

void MLView::setFormatQuoted(bool enable)
{
  m_bFormatQuoted = enable;
}

// -----------------------------------------------------------------------------

void MLView::paintCell(QPainter* p, int row, int col)
{

#if QT_VERSION >= 210
  if (m_bFormatQuoted)
  {
    QString s = stringShown(row);
    int i = (s[0] == ' ');
    bool italic = (s[i] == '>' && (s[i+1] == ' ' || s[i+1] == '>'));

    if (italic ^ p->font().italic())
    {
      QFont f(p->font());
      f.setItalic(italic);
      p->setFont(f);
    }
  }
#endif

  QMultiLineEdit::paintCell(p, row, col);
}

void MLView::setCellWidth ( int cellW )
{
#if QT_VERSION == 210
    if ( cellWidth() == cellW )
        return;

    QTableView::setCellWidth(cellW);

    if ( autoUpdate() && isVisible() )
        repaint();
#else
    QMultiLineEdit::setCellWidth( cellW );
#endif
}

#include "mlview2.moc"

#endif
