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

  QAccel *a = new QAccel( this );
  a->connectItem(a->insertItem(Key_Enter + CTRL),
                 this, SIGNAL(signal_CtrlEnterPressed()));
  a->connectItem(a->insertItem(Key_Return + CTRL),
                 this, SIGNAL(signal_CtrlEnterPressed()));

  if (editFont) QWidget::setFont(*editFont, true);
}


void MLEditWrap::appendNoNewLine(QString s)
{
#if QT_VERSION < 300
  if (!atEnd()) GotoEnd();
#endif
  QMultiLineEdit::insert(s);
}


void MLEditWrap::GotoEnd()
{
#if QT_VERSION >= 300
  scrollToBottom();
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
  if(e->state() & ControlButton) {
    switch(e->key()) {
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
