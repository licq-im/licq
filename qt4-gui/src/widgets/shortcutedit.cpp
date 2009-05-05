// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2009 Licq developers
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

#include "config.h"

#include "shortcutedit.h"

#ifndef USE_KDE
#include <QHBoxLayout>
#include <QKeyEvent>
#endif

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ShortcutEdit */
/* TRANSLATOR LicqQtGui::ShortcutButton */

ShortcutEdit::ShortcutEdit(QWidget* parent)
  : SHORTCUTEDIT_BASE(parent)
{
#ifndef USE_KDE
  QHBoxLayout* lay = new QHBoxLayout(this);
  lay->setContentsMargins(0, 0, 0, 0);

  myKeyButton = new ShortcutButton;
  lay->addWidget(myKeyButton);

  myClearButton = new QToolButton();
  myClearButton->setText(tr("X"));
  connect(myClearButton, SIGNAL(clicked()), myKeyButton, SLOT(clearShortcut()));
  lay->addWidget(myClearButton);

  connect(myKeyButton, SIGNAL(shortcutChanged(const QKeySequence&)),
      SIGNAL(keySequenceChanged(const QKeySequence&)));
#endif
}

#ifndef USE_KDE
ShortcutButton::ShortcutButton(QWidget* parent)
  : QToolButton(parent),
    myCapturing(false)
{
  // Set focus policy, otherwise we won't get focusOutEvent
  setFocusPolicy(Qt::StrongFocus);

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  clearShortcut();
  connect(this, SIGNAL(clicked()), SLOT(startCapture()));
}

QKeySequence ShortcutButton::shortcut() const
{
  return myShortcut;
}

void ShortcutButton::setShortcut(const QKeySequence& shortcut)
{
  myShortcut = shortcut;
  updateText();
  emit shortcutChanged(myShortcut);
}

void ShortcutButton::updateText()
{
  QString text;

  if (myCapturing)
  {
    // Capture is waiting for key to be pressed, show what we got so far
    if (myModifiers == Qt::NoModifier)
      text = tr("Input");
    if ((myModifiers & Qt::ShiftModifier) != 0)
      text += tr("Shift+");
    if ((myModifiers & Qt::ControlModifier) != 0)
      text += tr("Ctrl+");
    if ((myModifiers & Qt::AltModifier) != 0)
      text += tr("Alt+");
    if ((myModifiers & Qt::MetaModifier) != 0)
      text += tr("Meta+");
    text += " ...";
  }
  else
  {
    // No capture ongoing, show current shortcut
    text = (myCapturing ? myNewShortcut : myShortcut).toString(QKeySequence::NativeText);
    text.replace('&', "&&");
  }

  if (text.isEmpty())
    text = tr("None");

  setText(' ' + text + ' ');
}

void ShortcutButton::startCapture()
{
  myNewShortcut = QKeySequence();
  myModifiers = Qt::NoModifier;
  myCapturing = true;
  grabKeyboard();
  setDown(true);
  updateText();
}

void ShortcutButton::stopCapture(bool change)
{
  if (change)
    myShortcut = myNewShortcut;
  myCapturing = false;
  releaseKeyboard();
  setDown(false);
  updateText();
  if (change)
    emit shortcutChanged(myShortcut);
}

bool ShortcutButton::event(QEvent* event)
{
  // If we are in capture, we want to keep any keypresses to ourselves
  // Without this, dialog shortcuts are still recognised

  if (myCapturing && event->type() == QEvent::KeyPress)
  {
    keyPressEvent(dynamic_cast<QKeyEvent*>(event));
    return true;
  }
  if (myCapturing && event->type() == QEvent::ShortcutOverride)
  {
    event->accept();
    return true;
  }

  return QToolButton::event(event);
}

void ShortcutButton::keyPressEvent(QKeyEvent* event)
{
  if (!myCapturing)
    return QToolButton::keyPressEvent(event);

  event->accept();
  int key = event->key();
  myModifiers = event->modifiers();

  // AltGr is more like a shift key so best to ignore it as a modifier
  if (key == Qt::Key_AltGr)
    return;

  // If it was just a modifier key, update text and wait for the rest
  if (key == Qt::Key_Shift ||
      key == Qt::Key_Control ||
      key == Qt::Key_Meta ||
      key == Qt::Key_Alt)
  {
      updateText();
      return;
  }

  // Make sure we actually got a key
  if (key == 0 || key == -1)
    return;

  // Abort capture if we got Escape without any modifier
  if (key == Qt::Key_Escape && myModifiers == Qt::NoModifier)
  {
    stopCapture(false);
    return;
  }

  // If key doesn't have any modifier we might not want to have it as a shortcut
  // (Shift is ignored as it may be present for some non-acceptable characters.)
  if ((myModifiers & ~Qt::ShiftModifier) == 0 && keyMustHaveModifier(key))
    return;

  // We got a valid key, save it and end capture
  myNewShortcut = QKeySequence(key | myModifiers);
  stopCapture();
}

void ShortcutButton::keyReleaseEvent(QKeyEvent* event)
{
  if (!myCapturing)
    return QToolButton::keyReleaseEvent(event);

  event->accept();

  Qt::KeyboardModifiers newModifiers = event->modifiers();
  if ((myModifiers & newModifiers) != myModifiers)
  {
    // One of our known modifiers was released, update the button text
    myModifiers = newModifiers;
    updateText();
  }
}

bool ShortcutButton::keyMustHaveModifier(int key)
{
  // Keys represented with a single character is not accepted
  if (QKeySequence(key).toString(QKeySequence::NativeText).length() == 1)
    return true;

  // List of keys that should also not be allowed without any modifier
  switch (key)
  {
    case Qt::Key_Return:
    case Qt::Key_Space:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
    case Qt::Key_Escape:
      return true;
  }

  return false;
}

void ShortcutButton::focusOutEvent(QFocusEvent* event)
{
  if (myCapturing)
    stopCapture(false);
  QToolButton::focusOutEvent(event);
}

#endif
