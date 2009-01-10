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

#ifndef SETTINGS_SHORTCUTS_H
#define SETTINGS_SHORTCUTS_H

#include <QObject>

#include "config/shortcuts.h"

class QGroupBox;
class QKeySequence;
class QTreeWidget;
class QVBoxLayout;
class QWidget;

namespace LicqQtGui
{
class SettingsDlg;
class ShortcutEdit;

namespace Settings
{

/**
 * Settings pages for keyboard shortcuts
 */
class Shortcuts : public QObject
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param parent Settings dialog
   */
  Shortcuts(SettingsDlg* parent);

  /**
   * Destructor
   */
  virtual ~Shortcuts() {}

  /**
   * Load configuration data
   */
  void load();

  /**
   * Save configuration
   */
  void apply();

private slots:
  /**
   * A shortcut for main window was changed, make sure no other action has the same shortcut
   *
   * @param shortcut New shortcut
   */
  void mainwinShortcutChanged(const QKeySequence& shortcut);

  /**
   * A shortcut for message dialog was changed, make sure no other action has the same shortcut
   *
   * @param shortcut New shortcut
   */
  void chatShortcutChanged(const QKeySequence& shortcut);

private:
  /**
   * Setup page for contact list shortcuts
   *
   * @param parent Parent widget for page
   * @return a widget with the contact list shortcuts
   */
  QWidget* createPageMainwinShortcuts(QWidget* parent);

  /**
   * Setup page for message dialog shortcuts
   *
   * @param parent Parent widget for page
   * @return a widget with the message dialog shortcuts
   */
  QWidget* createPageChatShortcuts(QWidget* parent);

  // Widgets for main window shortcuts
  QVBoxLayout* myPageMainwinShortcutsLayout;
  QGroupBox* myMainwinBox;
  QVBoxLayout* myMainwinLayout;
  QTreeWidget* myMainwinList;
  QMap<Config::Shortcuts::ShortcutType, ShortcutEdit*> myMainwinShortcuts;

  // Widgets for message dialog shortcuts
  QVBoxLayout* myPageChatShortcutsLayout;
  QGroupBox* myChatBox;
  QVBoxLayout* myChatLayout;
  QTreeWidget* myChatList;
  QMap<Config::Shortcuts::ShortcutType, ShortcutEdit*> myChatShortcuts;
};

} // namespace Settings
} // namespace LicqQtGui

#endif
