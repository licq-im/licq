/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2009-2011 Licq developers
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

#ifndef CONFIG_SHORTCUTS_H
#define CONFIG_SHORTCUTS_H

#include <QKeySequence>
#include <QObject>
#include <QMap>

namespace Licq
{
class IniFile;
}

namespace LicqQtGui
{
namespace Config
{
/**
 * Keybort shortcuts configuration
 */
class Shortcuts : public QObject
{
  Q_OBJECT

public:
  enum ShortcutType
  {
    ChatColorBack,
    ChatColorFore,
    ChatEmoticonMenu,
    ChatEncodingMenu,
    ChatEventMenu,
    ChatHistory,
    ChatPopupNextMessage,
    ChatTab01,
    ChatTab02,
    ChatTab03,
    ChatTab04,
    ChatTab05,
    ChatTab06,
    ChatTab07,
    ChatTab08,
    ChatTab09,
    ChatTab10,
    ChatToggleMassMessage,
    ChatToggleSecure,
    ChatToggleSendServer,
    ChatToggleUrgent,
    ChatUserInfo,
    ChatUserMenu,
#ifdef Q_WS_X11
    GlobalPopupMessage,
    GlobalShowMainwin,
#endif
    InputClear,
    InputDeleteLine,
    InputDeleteLineBack,
    InputDeleteWordBack,
    MainwinAccountManager,
    MainwinAddGroup,
    MainwinEditGroups,
    MainwinExit,
    MainwinHide,
    MainwinNetworkLog,
    MainwinPopupAllMessages,
    MainwinPopupMessage,
    MainwinRedrawContactList,
    MainwinSetAutoResponse,
    MainwinSettings,
    MainwinStatusAway,
    MainwinStatusDoNotDisturb,
    MainwinStatusFreeForChat,
    MainwinStatusInvisible,
    MainwinStatusNotAvailable,
    MainwinStatusOccupied,
    MainwinStatusOffline,
    MainwinStatusOnline,
    MainwinToggleEmptyGroups,
    MainwinToggleMiniMode,
    MainwinToggleShowHeader,
    MainwinToggleShowOffline,
    MainwinUserCheckAutoresponse,
    MainwinUserSendChatRequest,
    MainwinUserSendMessage,
    MainwinUserSendFile,
    MainwinUserSendUrl,
    MainwinUserViewHistory,
    MainwinUserViewMessage,
  };

  /**
   * Create the singleton instance
   *
   * @param parent Parent object
   */
  static void createInstance(QObject* parent = NULL);

  /**
   * Get the singleton instance
   *
   * @return The instance
   */
  static Shortcuts* instance()
  { return myInstance; }

  /**
   * Constuctor
   */
  Shortcuts(QObject* parent = 0);

  /**
   * Destructor
   */
  ~Shortcuts() {}

  /**
   * Allow or disallow signals for configuration changes
   *
   * @param block True to block signals or false to unblock and send any pending signals
   */
  void blockUpdates(bool block);

  /**
   * Get a keyboard shortcut
   *
   * @param function Function to get shortcut for
   * @return The currently assigned shortcut
   */
  QKeySequence getShortcut(ShortcutType function) const
  { return myShortcutsMap.value(function); }

public slots:
  /**
   * Load configuration from file
   */
  void loadConfiguration(Licq::IniFile& iniFile);

  /**
   * Save configuration to file
   */
  void saveConfiguration(Licq::IniFile& iniFile) const;

  /**
   * Set a keyboard shortcut
   *
   * @param function Function to set shortcut for
   * @param shortcut New shortcut to assign
   */
  void setShortcut(ShortcutType function, const QKeySequence& shortcut);

signals:
  /**
   * Shortcut has changed
   */
  void shortcutsChanged();

private:
  static Shortcuts* myInstance;

  // Changes has been made that should trigger changed() signal
  bool myBlockUpdates;
  bool myShortcutsHasChanged;

  QMap<ShortcutType, QKeySequence> myShortcutsMap;
  QMap<ShortcutType, int> myDefaultShortcutsMap;
  QMap<ShortcutType, QString> myConfigKeysMap;
};

} // namespace Config
} // namespace LicqQtGui

#endif
