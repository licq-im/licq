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

#include <qfont.h>
#include <qpainter.h>
#include <qaccel.h>

#include "mledit.h"


QFont *MLEditWrap::editFont = NULL;

MLEditWrap::MLEditWrap (bool wordWrap, QWidget* parent, bool doQuotes, const char *name)
  : QMultiLineEdit(parent, name)
{
  m_bDoQuotes = doQuotes;

  if (wordWrap)
  {
    setWordWrap(WidgetWidth);
    setWrapPolicy(AtWhiteSpace);
  }
  else
  {
    setWordWrap(NoWrap);
  }

  if (editFont)
    QWidget::setFont(*editFont, true);
}


void MLEditWrap::appendNoNewLine(const QString& s)
{
  if (!atEnd()) GotoEnd();
  QMultiLineEdit::insert(s);
}

void MLEditWrap::append(const QString& s)
{
#if QT_VERSION < 300
  appendNoNewLine(s + "\n");
#else
  if (strcmp(qVersion(), "3.0.0") == 0 ||
      strcmp(qVersion(), "3.0.1") == 0 ||
      strcmp(qVersion(), "3.0.2") == 0 ||
      strcmp(qVersion(), "3.0.3") == 0 || 
      strcmp(qVersion(), "3.0.4") == 0)
  {
     // Workaround --
     // In those versions, QTextEdit::append didn't add a new paragraph.
     QTextEdit::append(s);
     QTextEdit::append("\n");
  }
  else
  {
     QTextEdit::append(s);
  }
#endif
}

void MLEditWrap::GotoEnd()
{
#if QT_VERSION >= 300
  moveCursor(QTextEdit::MoveEnd, false);
#else
  setCursorPosition(numLines() - 1, lineLength(numLines() - 1) - 1);
#endif
}


void MLEditWrap::setBackground(const QColor& c)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QColorGroup::Base, c);
  pal.setColor(QPalette::Inactive, QColorGroup::Base, c);

  setPalette(pal);
}


// -----------------------------------------------------------------------------


void MLEditWrap::setForeground(const QColor& c)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QColorGroup::Text, c);
  pal.setColor(QPalette::Inactive, QColorGroup::Text, c);

  setPalette(pal);
}

bool MLEditWrap::focusNextPrevChild( bool f)
{
  return QWidget::focusNextPrevChild(f);
}


// -----------------------------------------------------------------------------

void MLEditWrap::paintCell(QPainter* p, int row, int col)
{

#if QT_VERSION >= 210 && QT_VERSION < 300
  if (m_bDoQuotes)
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

#if QT_VERSION < 300
  QMultiLineEdit::paintCell(p, row, col);
#endif
}

void MLEditWrap::keyPressEvent( QKeyEvent *e )
{
  const bool isShift   = e->state() & ShiftButton;
  const bool isControl = e->state() & ControlButton;

  if (isShift && e->key() == Key_Insert)
    return paste();

  if (isShift && e->key() == Key_Delete)
    return cut();

  if (isControl && e->key() == Key_Insert)
    return copy();

  if (isControl)
  {
    switch (e->key())
    {
    case Key_W:
      cursorWordBackward(true);
      del();
      break;
    case Key_U:
      home();
      killLine();
      break;
    case Key_L:
      clear();
      break;
    case Key_Return:
    case Key_Enter:
      emit signal_CtrlEnterPressed();
      break;
    default:
      QMultiLineEdit::keyPressEvent(e);
    }
    return;
  }

  QMultiLineEdit::keyPressEvent(e);
}

void MLEditWrap::setCellWidth ( int cellW )
{
#if QT_VERSION == 210
    if ( cellWidth() == cellW )
        return;

    QTableView::setCellWidth(cellW);

    if ( autoUpdate() && isVisible() )
        repaint();
#else
#if QT_VERSION < 300
    QMultiLineEdit::setCellWidth( cellW );
#endif
#endif
}

#include "mledit.moc"
