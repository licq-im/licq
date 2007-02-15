// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

// written by Graham Roff <graham@licq.org>
// contributions by Dirk A. Mueller <dirk@licq.org>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qfont.h>
#include <qpopupmenu.h>

#include "mledit.h"


QFont *MLEditWrap::editFont = NULL;
bool MLEditWrap::useDoubleReturn = false;

MLEditWrap::MLEditWrap (bool wordWrap, QWidget* parent, bool /* doQuotes */, const char *name)
  : MLEditWrapBase(parent, name),
    m_fixSetTextNewlines(true),
    m_lastKeyWasReturn(false)
{
  setTextFormat(Qt::PlainText);
  setTabChangesFocus(true);

  if (wordWrap)
  {
    setWordWrap(QTextEdit::WidgetWidth);
    setWrapPolicy(QTextEdit::AtWhiteSpace);
  }
  else
  {
    setWordWrap(QTextEdit::NoWrap);
  }

  if (editFont)
    QWidget::setFont(*editFont, true);
}


void MLEditWrap::appendNoNewLine(const QString& s)
{
  GotoEnd();
  insert(s);
}

void MLEditWrap::append(const QString& s)
{
  MLEditWrapBase::append(s);
  if (strcmp(qVersion(), "3.0.0") == 0 ||
      strcmp(qVersion(), "3.0.1") == 0 ||
      strcmp(qVersion(), "3.0.2") == 0 ||
      strcmp(qVersion(), "3.0.3") == 0 ||
      strcmp(qVersion(), "3.0.4") == 0)
  {
    // Workaround --
    // In those versions, QTextEdit::append didn't add a new paragraph.
    MLEditWrapBase::append("\n");
  }
}

void MLEditWrap::GotoEnd()
{
  moveCursor(QTextEdit::MoveEnd, false);
}

void MLEditWrap::setBackground(const QColor& c)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QColorGroup::Base, c);
  pal.setColor(QPalette::Inactive, QColorGroup::Base, c);

  setPalette(pal);
}

void MLEditWrap::setForeground(const QColor& c)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QColorGroup::Text, c);
  pal.setColor(QPalette::Inactive, QColorGroup::Text, c);

  setPalette(pal);
}

void MLEditWrap::setCheckSpellingEnabled(bool check)
{
#ifdef MLEDIT_USE_KTEXTEDIT
  MLEditWrapBase::setCheckSpellingEnabled(check);
#else
  (void) check; // Remove warning about unused parameter
#endif
}

bool MLEditWrap::checkSpellingEnabled() const
{
#ifdef MLEDIT_USE_KTEXTEDIT
  return MLEditWrapBase::checkSpellingEnabled();
#else
  return false;
#endif
}

void MLEditWrap::keyPressEvent( QKeyEvent *e )
{
  const bool isShift   = e->state() & ShiftButton;
  const bool isControl = e->state() & ControlButton;

  // Get flag from last time and reset it before any possible returns
  bool lastKeyWasReturn = m_lastKeyWasReturn;
  m_lastKeyWasReturn = false;

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
      moveCursor(QTextEdit::MoveWordBackward, true);
      del();
      break;
    case Key_U:
      moveCursor(QTextEdit::MoveHome, false);
      doKeyboardAction(QTextEdit::ActionKill);
      break;
    case Key_L:
      clear();
      break;
    case Key_Return:
    case Key_Enter:
      if (useDoubleReturn)
        insert("\n");
      else
        emit signal_CtrlEnterPressed();
      break;
    default:
      MLEditWrapBase::keyPressEvent(e);
    }
    return;
  }

  if ((e->state() & Qt::KeyButtonMask) == 0)
  {
    switch (e->key())
    {
      case Key_Return:
      case Key_Enter:
        if (lastKeyWasReturn && useDoubleReturn)
        {
          // Return pressed twice, remove the previous line break and emit signal
          moveCursor(QTextEdit::MoveBackward, true);
          del();
          emit signal_CtrlEnterPressed();
          return;
        }
        else
        {
          // Return pressed once
          m_lastKeyWasReturn = true;
        }
        break;
    }
  }

  MLEditWrapBase::keyPressEvent(e);
}

QPopupMenu *MLEditWrap::createPopupMenu(const QPoint &pos)
{
  QPopupMenu *menu = MLEditWrapBase::createPopupMenu(pos);
#ifndef MLEDIT_USE_KTEXTEDIT
  menu->insertSeparator();
  int id = menu->insertItem(tr("Allow Tabulations"), this, SLOT(slotToggleAllowTab()));
  menu->setItemChecked(id, !tabChangesFocus());
#endif
  return menu;
}

void MLEditWrap::slotToggleAllowTab()
{
  setTabChangesFocus(!tabChangesFocus());
}

#ifdef MLEDIT_USE_KTEXTEDIT
/**
 * @return the number of characters @a c at the end of @a str.
 */
static unsigned int countCharRev(const QString& str, const QChar c)
{
  unsigned int count = 0;
  for (int pos = str.length() - 1; pos >= 0; pos--)
  {
    if (str.at(pos) != c)
      break;
    count += 1;
  }
  return count;
}
#endif

/*
 * KTextEdit adds a menu entry for doing spell checking. Unfortunatly KSpell
 * (which is what KTextEdit uses to do the spell check) messes with the newlines
 * at the end of the text it checks. That's why we need the hack below. It uses
 * the fact that setText(const QString&) is non-virtual and only calls the
 * virtual setText(const QString&, const QString&) with a null context.
 *
 * When KTextEdit calls setText(correctedText) after the spell check is done
 * it will call QTextEdit::setText (since it's non-virtual). QTextEdit will
 * then call setText(correctedText, QString::null) which will end up in the
 * setText below (since it's virtual). And with m_fixSetTextNewlines set to
 * true we can fix so that there is as many newlines at the end of the corrected
 * text as there is in the old.
 *
 * On the other hand, when any class that uses MLEditWrap calls
 * myMLEditWrapInstance->setText(myText) the call will end up at
 * MLEditWrap::setText(myText) which will set m_fixSetTextNewlines to false before
 * calling QTextEdit::setText(myText).
 */
void MLEditWrap::setText(const QString& text)
{
  m_fixSetTextNewlines = false;
  MLEditWrapBase::setText(text);
}

void MLEditWrap::setText(const QString& txt, const QString& context)
{
  const bool modified = isModified(); // don't let setText reset this flag
#ifdef MLEDIT_USE_KTEXTEDIT
  const QString current = text();
  if (m_fixSetTextNewlines && context.isNull())
  {
    const unsigned int currentNL = countCharRev(current, '\n');
    const unsigned int txtNL = countCharRev(txt, '\n');
    if (currentNL > txtNL)
      MLEditWrapBase::setText(txt + QString().fill('\n', currentNL - txtNL), context);
    else if (txtNL > currentNL)
      MLEditWrapBase::setText(txt.left(txt.length() - (txtNL - currentNL)), context);
    else
      MLEditWrapBase::setText(txt, context);
  }
  else
#endif
    MLEditWrapBase::setText(txt, context);

  setModified(modified);
  m_fixSetTextNewlines = true;
}

#include "mledit.moc"
