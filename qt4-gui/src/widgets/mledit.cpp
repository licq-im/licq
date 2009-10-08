// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#include "mledit.h"

#ifndef USE_KDE
#include <QAction>
#endif
#include <QKeyEvent>
#include <QMenu>

#include "config/general.h"
#include "config/shortcuts.h"

#ifdef HAVE_HUNSPELL
# include "spellchecker.h"
#endif


using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::MLEdit */

MLEdit::MLEdit(bool wordWrap, QWidget* parent, bool useFixedFont, const char* name)
  : MLEDIT_BASE(parent),
#ifdef HAVE_HUNSPELL
    mySpellChecker(NULL),
#endif
    myUseFixedFont(useFixedFont),
    myFixSetTextNewlines(true),
    myLastKeyWasReturn(false),
    myLinesHint(0)
{
  setObjectName(name);
  setAcceptRichText(false);
  setTabChangesFocus(true);

  if (!wordWrap)
    setLineWrapMode(NoWrap);

  updateFont();
  connect(Config::General::instance(), SIGNAL(fontChanged()), SLOT(updateFont()));
}

MLEdit::~MLEdit()
{
  // Empty
}

#ifndef USE_KDE
void MLEdit::setCheckSpellingEnabled(bool check)
{
#ifdef HAVE_HUNSPELL
  if (check && mySpellChecker == NULL && !mySpellingDictionary.isEmpty())
    mySpellChecker = new SpellChecker(this->document(), mySpellingDictionary);
  if (!check && mySpellChecker != NULL)
    delete mySpellChecker;
#else
  Q_UNUSED(check);
#endif
}

bool MLEdit::checkSpellingEnabled() const
{
#ifdef HAVE_HUNSPELL
  return (mySpellChecker != NULL);
#else
  return false;
#endif
}
#endif

#ifdef HAVE_HUNSPELL
void MLEdit::setSpellingDictionary(const QString& dicFile)
{
  mySpellingDictionary = dicFile;
  if (mySpellChecker != NULL)
    mySpellChecker->setDictionary(dicFile);
  else
    setCheckSpellingEnabled(true);
}
#endif

void MLEdit::appendNoNewLine(const QString& s)
{
  GotoEnd();
  insertPlainText(s);
}

void MLEdit::GotoEnd()
{
  moveCursor(QTextCursor::End);
}

void MLEdit::setBackground(const QColor& color)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QPalette::Base, color);
  pal.setColor(QPalette::Inactive, QPalette::Base, color);

  setPalette(pal);
}

void MLEdit::setForeground(const QColor& color)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QPalette::Text, color);
  pal.setColor(QPalette::Inactive, QPalette::Text, color);

  setPalette(pal);
}

void MLEdit::clearKeepUndo()
{
  QTextCursor cr = textCursor();
  cr.select(QTextCursor::Document);
  cr.removeSelectedText();
}

void MLEdit::deleteLine()
{
  QTextCursor cr = textCursor();
  cr.select(QTextCursor::BlockUnderCursor);
  if (!cr.hasSelection())
    cr.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
  if (!cr.hasSelection())
    cr.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
  cr.removeSelectedText();
}

void MLEdit::deleteLineBackwards()
{
  QTextCursor cr = textCursor();
  cr.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
  if (!cr.hasSelection())
    cr.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
  cr.removeSelectedText();
}

void MLEdit::deleteWordBackwards()
{
  QTextCursor cr = textCursor();
  cr.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
  cr.removeSelectedText();
}

void MLEdit::keyPressEvent(QKeyEvent* event)
{
  // Get flag from last time and reset it before any possible returns
  bool lastKeyWasReturn = myLastKeyWasReturn;
  myLastKeyWasReturn = false;

  // Ctrl+Return will either trigger dialog or (if disabled) insert a normal line break
  if (event->modifiers() == Qt::ControlModifier &&
      (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter))
  {
    if (Config::General::instance()->useDoubleReturn())
      insertPlainText(QString("\n"));
    else
      emit ctrlEnterPressed();
    return;
  }

  if (event->modifiers() == Qt::NoModifier)
  {
    switch (event->key())
    {
      case Qt::Key_Return:
      case Qt::Key_Enter:
        if (lastKeyWasReturn && Config::General::instance()->useDoubleReturn())
        {
          // Return pressed twice, remove the previous line break and emit signal
          QTextCursor cr = textCursor();
          cr.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
          cr.removeSelectedText();
          emit ctrlEnterPressed();
          return;
        }
        else
        {
          // Return pressed once
          myLastKeyWasReturn = true;
        }
        break;
      case Qt::Key_Insert:
        if (overwriteMode())
        {
          setOverwriteMode(false);
          setCursorWidth(1);
        }
        else
        {
          setOverwriteMode(true);
          setCursorWidth(2);
        }
        break;
    }
  }

  if (event->key() == Qt::Key_Delete && event->modifiers() == Qt::ShiftModifier)
    return cut();
  if (event->key() == Qt::Key_Insert && event->modifiers() == Qt::ShiftModifier)
    return paste();
  if (event->key() == Qt::Key_Insert && event->modifiers() == Qt::ControlModifier)
    return copy();
  if (event->key() == Qt::Key_PageDown && event->modifiers() == Qt::ShiftModifier)
  {
    emit scrollDownPressed();
    return;
  }
  if (event->key() == Qt::Key_PageUp && event->modifiers() == Qt::ShiftModifier)
  {
    emit scrollUpPressed();
    return;
  }

  Config::Shortcuts* shortcuts = Config::Shortcuts::instance();
  QKeySequence ks = QKeySequence(event->key() | event->modifiers());

  if (ks == shortcuts->getShortcut(Config::Shortcuts::InputClear))
    return clearKeepUndo();
  if (ks == shortcuts->getShortcut(Config::Shortcuts::InputDeleteLine))
    return deleteLine();
  if (ks == shortcuts->getShortcut(Config::Shortcuts::InputDeleteLineBack))
    return deleteLineBackwards();
  if (ks == shortcuts->getShortcut(Config::Shortcuts::InputDeleteWordBack))
    return deleteWordBackwards();

  MLEDIT_BASE::keyPressEvent(event);
}

void MLEdit::mousePressEvent(QMouseEvent* event)
{
  emit clicked();
  MLEDIT_BASE::mousePressEvent(event);
}

#ifndef USE_KDE
void MLEdit::contextMenuEvent(QContextMenuEvent* event)
{
  QMenu* menu=createStandardContextMenu();

  if (!isReadOnly())
  {
#ifdef HAVE_HUNSPELL
    // Save position so we know which word to replace
    myMenuPos = event->pos();

    // Get word under cursor
    QTextCursor cr = cursorForPosition(myMenuPos);
    cr.select(QTextCursor::WordUnderCursor);
    QString word = cr.selectedText();
    if (!word.isEmpty())
    {
      // Get spelling suggestions
      QStringList suggestions = mySpellChecker->getSuggestions(word);
      if (!suggestions.isEmpty())
      {
        // Add spelling suggestions at the top of the menu
        QAction* firstAction = menu->actions().first();
        foreach (QString w, suggestions)
        {
          QAction* a = new QAction(w, menu);
          connect(a, SIGNAL(triggered()), SLOT(replaceWord()));
          menu->insertAction(firstAction, a);
        }
        menu->insertSeparator(firstAction);
      }
    }
#endif

    QAction* tabul = new QAction(tr("Allow Tabulations"), menu);
    tabul->setCheckable(true);
    tabul->setChecked(!tabChangesFocus());
    connect(tabul, SIGNAL(triggered()), SLOT(toggleAllowTab()));
    menu->addAction(tabul);
  }

  menu->exec(event->globalPos());
  delete menu;
}
#endif

#ifdef HAVE_HUNSPELL
void MLEdit::replaceWord()
{
  QAction* a = qobject_cast<QAction*>(sender());
  if (a == NULL)
    return;

  // Mark the word under the cursor and replace it with the text from the menu item selected
  QTextCursor cr = cursorForPosition(myMenuPos);
  cr.select(QTextCursor::WordUnderCursor);
  cr.insertText(a->text());
}
#endif

void MLEdit::updateFont()
{
  setFont(myUseFixedFont ? Config::General::instance()->fixedFont() :
      Config::General::instance()->editFont());

  // Get height of current font
  myFontHeight = fontMetrics().height();

  // Set minimum height of text area to one line of text.
  setMinimumHeight(heightForLines(1));
}

int MLEdit::heightForLines(int lines) const
{
  // We need to add frame width as we're calculating height for the widget, not just the viewport.
  // The reason for the last constant is unknown, but seems the same regardless of font size and gui style
  return lines*myFontHeight + 2*frameWidth() + 8;
}

void MLEdit::setSizeHintLines(int lines)
{
  myLinesHint = lines;
}

QSize MLEdit::sizeHint() const
{
  QSize s = MLEDIT_BASE::sizeHint();
  if (myLinesHint > 0)
    s.setHeight(heightForLines(myLinesHint));
  return s;
}

void MLEdit::toggleAllowTab()
{
  setTabChangesFocus(!tabChangesFocus());
}

#if 0
//TODO: This may or may not be needed for KTextEdit in KDE 4

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
#endif
