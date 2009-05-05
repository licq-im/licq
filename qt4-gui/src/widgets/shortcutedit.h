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

#ifndef SHORTCUTEDIT_H
#define SHORTCUTEDIT_H

#include <config.h>

#ifdef USE_KDE
#include <KDE/KKeySequenceWidget>
#define SHORTCUTEDIT_BASE KKeySequenceWidget
#else
#include <QToolButton>
#include <QKeySequence>
#include <QWidget>
#define SHORTCUTEDIT_BASE QWidget
#endif

namespace LicqQtGui
{

#ifndef USE_KDE
/**
 * Button used for performing the actual capture
 * Captures key when pressed
 */
class ShortcutButton : public QToolButton
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @parent Parent widget
   */
  ShortcutButton(QWidget* parent = NULL);

  /**
   * Get shortcut
   *
   * @return Current shortcut
   */
  QKeySequence shortcut() const;

public slots:
  /**
   * Set shortcut
   *
   * @param sequence New shortcut
   */
  void setShortcut(const QKeySequence& shortcut);

  /**
   * Clear current shortcut
   */
  void clearShortcut() { setShortcut(QKeySequence()); }

signals:
  /**
   * Emitted when shortcut is changed
   *
   * @param shortcut New shortcut
   */
  void shortcutChanged(const QKeySequence& shortcut);

protected:
  /**
   * Main event handler for widget
   * Overridden to stop dialog from getting shortcuts while capturing
   *
   * @param event Event
   * @return True if event was handled
   */
  bool event(QEvent* event);

  /**
   * A key was pressed
   * Overloaded to capture key presses when getting new shortcut key
   *
   * @param event Key press event
   */
  virtual void keyPressEvent(QKeyEvent* event);

  /**
   * A key was released
   * Overloaded to capture key presses when getting new shortcut key
   *
   * @param event Key release event
   */
  virtual void keyReleaseEvent(QKeyEvent* event);

  /**
   * Button lost focus
   * If a capture was ongoing, abort it
   *
   * @param event Focus out event
   */
  virtual void focusOutEvent(QFocusEvent* event);

private slots:
  /**
   * Start key capture to listen for new shortcut key
   */
  void startCapture();

  /**
   * Stop key capture
   *
   * @param change True to save changes, false to revert
   */
  void stopCapture(bool change = true);

private:
  /**
   * Update button text for current shortcut
   */
  void updateText();

  /**
   * Check if a key can be allowed as shortcut with no modifier active
   *
   * @param key Qt key code of the key to check
   * @return True if this key requires a modifier
   */
  bool keyMustHaveModifier(int key);

  QKeySequence myShortcut;
  QKeySequence myNewShortcut;
  bool myCapturing;
  Qt::KeyboardModifiers myModifiers;
};
#endif


/**
 * Control for selecting key shortcuts.
 * Uses KKeySequenceWidget when building with KDE support.
 */
class ShortcutEdit : public SHORTCUTEDIT_BASE
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @parent Parent widget
   */
  ShortcutEdit(QWidget* parent = NULL);

  /**
   * Destructor
   */
  virtual ~ShortcutEdit() {}

#ifndef USE_KDE
  /**
   * Get shortcut
   *
   * @return Current shortcut
   */
  QKeySequence keySequence() const
  { return myKeyButton->shortcut(); }

public slots:
  /**
   * Set shortcut
   *
   * @param sequence New shortcut
   */
  void setKeySequence(const QKeySequence& sequence)
  { myKeyButton->setShortcut(sequence); }

signals:
  /**
   * Emitted with shortcut is changed
   *
   * @param sequence New shortcut
   */
  void keySequenceChanged(const QKeySequence& sequence);

private:
  ShortcutButton* myKeyButton;
  QToolButton* myClearButton;
#endif
};

} // namespace LicqQtGui

#endif
