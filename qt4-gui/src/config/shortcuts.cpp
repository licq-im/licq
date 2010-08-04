// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2009-2010 Licq developers
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

#include "shortcuts.h"

#include <licq/inifile.h>

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Config::Shortcuts */

Config::Shortcuts* Config::Shortcuts::myInstance = NULL;

void Config::Shortcuts::createInstance(QObject* parent)
{
  myInstance = new Config::Shortcuts(parent);
}

Config::Shortcuts::Shortcuts(QObject* parent)
  : QObject(parent),
    myBlockUpdates(false),
    myShortcutsHasChanged(false)
{
  // Define all shortcuts here by populating config file keys and default values in the maps

#define ADD_SHORTCUT(function, inikey, def) \
  myDefaultShortcutsMap.insert(function, def); \
  myConfigKeysMap.insert(function, inikey);

  // Shortcuts for message dialog
  ADD_SHORTCUT(ChatColorBack, "Chat.ColorBack", Qt::ALT + Qt::Key_B)
  ADD_SHORTCUT(ChatColorFore, "Chat.ColorFore", Qt::ALT + Qt::Key_T)
  ADD_SHORTCUT(ChatEmoticonMenu, "Chat.EmoticonMenu", Qt::ALT + Qt::Key_L)
  ADD_SHORTCUT(ChatEncodingMenu, "Chat.EncodingMenu", Qt::ALT + Qt::Key_O)
  ADD_SHORTCUT(ChatEventMenu, "Chat.EventMenu", Qt::ALT + Qt::Key_P)
  ADD_SHORTCUT(ChatHistory, "Chat.History", Qt::ALT + Qt::Key_H)
  ADD_SHORTCUT(ChatPopupNextMessage, "Chat.PopupNextMessage", 0);
  ADD_SHORTCUT(ChatTab01, "Chat.Tab01", Qt::ALT + Qt::Key_1)
  ADD_SHORTCUT(ChatTab02, "Chat.Tab02", Qt::ALT + Qt::Key_2)
  ADD_SHORTCUT(ChatTab03, "Chat.Tab03", Qt::ALT + Qt::Key_3)
  ADD_SHORTCUT(ChatTab04, "Chat.Tab04", Qt::ALT + Qt::Key_4)
  ADD_SHORTCUT(ChatTab05, "Chat.Tab05", Qt::ALT + Qt::Key_5)
  ADD_SHORTCUT(ChatTab06, "Chat.Tab06", Qt::ALT + Qt::Key_6)
  ADD_SHORTCUT(ChatTab07, "Chat.Tab07", Qt::ALT + Qt::Key_7)
  ADD_SHORTCUT(ChatTab08, "Chat.Tab08", Qt::ALT + Qt::Key_8)
  ADD_SHORTCUT(ChatTab09, "Chat.Tab09", Qt::ALT + Qt::Key_9)
  ADD_SHORTCUT(ChatTab10, "Chat.Tab10", Qt::ALT + Qt::Key_0)
  ADD_SHORTCUT(ChatToggleMassMessage, "Chat.ToggleMassMessage", Qt::ALT + Qt::Key_U)
  ADD_SHORTCUT(ChatToggleSecure, "Chat.ToggleSecure", Qt::ALT + Qt::Key_E)
  ADD_SHORTCUT(ChatToggleSendServer, "Chat.ToggleSendServer", Qt::ALT + Qt::Key_N)
  ADD_SHORTCUT(ChatToggleUrgent, "Chat.ToggleUrgent", Qt::ALT + Qt::Key_R)
  ADD_SHORTCUT(ChatUserInfo, "Chat.UserInfo", Qt::ALT + Qt::Key_I)
  ADD_SHORTCUT(ChatUserMenu, "Chat.UserMenu", Qt::ALT + Qt::Key_M)

  // Shortcuts for MLEdit used for text input in several dialogs including message dialog
  ADD_SHORTCUT(InputClear, "Input.Clear", Qt::CTRL + Qt::Key_L)
  ADD_SHORTCUT(InputDeleteLine, "Input.DeleteLine", Qt::CTRL + Qt::Key_U)
  ADD_SHORTCUT(InputDeleteLineBack, "Input.DeleteLineBack", Qt::CTRL + Qt::Key_N)
  ADD_SHORTCUT(InputDeleteWordBack, "Input.DeleteWordBack", Qt::CTRL + Qt::Key_W)

  // Shortcuts for main window (contact list)
  ADD_SHORTCUT(MainwinAccountManager, "Mainwin.AccountManager", 0)
  ADD_SHORTCUT(MainwinAddGroup, "Mainwin.AddGroup", 0)
  ADD_SHORTCUT(MainwinEditGroups, "Mainwin.EditGroups", Qt::CTRL + Qt::Key_G)
  ADD_SHORTCUT(MainwinExit, "Mainwin.Exit", Qt::CTRL + Qt::Key_Q)
  ADD_SHORTCUT(MainwinHide, "Mainwin.Hide", Qt::CTRL + Qt::Key_H)
  ADD_SHORTCUT(MainwinNetworkLog, "Mainwin.NetworkLog", 0)
  ADD_SHORTCUT(MainwinPopupAllMessages, "Mainwin.PopupAllMessages", Qt::CTRL + Qt::Key_P)
  ADD_SHORTCUT(MainwinPopupMessage, "Mainwin.PopupMessage", Qt::CTRL + Qt::Key_I)
  ADD_SHORTCUT(MainwinRedrawContactList, "Mainwin.RedrawContactList", Qt::CTRL + Qt::Key_L)
  ADD_SHORTCUT(MainwinSetAutoResponse, "Mainwin.SetAutoResponse", 0)
  ADD_SHORTCUT(MainwinSettings, "Mainwin.Settings", 0)
  ADD_SHORTCUT(MainwinStatusAway, "Mainwin.StatusAway", Qt::ALT + Qt::Key_A)
  ADD_SHORTCUT(MainwinStatusDoNotDisturb, "Mainwin.StatusDoNotDisturb", Qt::ALT + Qt::Key_D)
  ADD_SHORTCUT(MainwinStatusFreeForChat, "Mainwin.StatusFreeForChat", Qt::ALT + Qt::Key_H)
  ADD_SHORTCUT(MainwinStatusInvisible, "Mainwin.StatusInvisible", Qt::ALT + Qt::Key_I)
  ADD_SHORTCUT(MainwinStatusNotAvailable, "Mainwin.StatusNotAvailable", Qt::ALT + Qt::Key_N)
  ADD_SHORTCUT(MainwinStatusOccupied, "Mainwin.StatusOccupied", Qt::ALT + Qt::Key_C)
  ADD_SHORTCUT(MainwinStatusOffline, "Mainwin.StatusOffline", Qt::ALT + Qt::Key_F)
  ADD_SHORTCUT(MainwinStatusOnline, "Mainwin.StatusOnline", Qt::ALT + Qt::Key_O)
  ADD_SHORTCUT(MainwinToggleEmptyGroups, "Mainwin.MainwinToggleEmptyGroups", 0)
  ADD_SHORTCUT(MainwinToggleMiniMode, "Mainwin.MainwinToggleMiniMode", Qt::CTRL + Qt::Key_M)
  ADD_SHORTCUT(MainwinToggleShowHeader, "Mainwin.MainwinToggleShowHeader", 0)
  ADD_SHORTCUT(MainwinToggleShowOffline, "Mainwin.MainwinToggleShowOffline", Qt::CTRL + Qt::Key_O)
  ADD_SHORTCUT(MainwinUserCheckAutoresponse, "Mainwin.UserCheckAutoresponse", Qt::CTRL + Qt::Key_C)
  ADD_SHORTCUT(MainwinUserSendChatRequest, "Mainwin.UserSendChatRequest", Qt::CTRL + Qt::Key_C)
  ADD_SHORTCUT(MainwinUserSendMessage, "Mainwin.UserSendMessage", Qt::CTRL + Qt::Key_S)
  ADD_SHORTCUT(MainwinUserSendFile, "Mainwin.UserSendFile", Qt::CTRL + Qt::Key_F)
  ADD_SHORTCUT(MainwinUserSendUrl, "Mainwin.UserSendUrl", Qt::CTRL + Qt::Key_U)
  ADD_SHORTCUT(MainwinUserViewHistory, "Mainwin.UserViewHistory", 0);
  ADD_SHORTCUT(MainwinUserViewMessage, "Mainwin.UserViewMessage", Qt::CTRL + Qt::Key_V)

#undef ADD_SHORTCUT
}

void Config::Shortcuts::loadConfiguration(Licq::IniFile& iniFile)
{
  iniFile.setSection("shortcuts");

  QMap<ShortcutType, QString>::iterator i;
  for (i = myConfigKeysMap.begin(); i != myConfigKeysMap.end(); ++i)
  {
    std::string s;
    iniFile.get(i.value().toAscii().data(), s);
    if (s.empty())
      myShortcutsMap[i.key()] = QKeySequence(myDefaultShortcutsMap[i.key()]);
    else if(s == "None")
      myShortcutsMap[i.key()] = QKeySequence();
    else
      myShortcutsMap[i.key()] = QKeySequence(s.c_str());
  }

  emit shortcutsChanged();
}

void Config::Shortcuts::saveConfiguration(Licq::IniFile& iniFile) const
{
  iniFile.setSection("shortcuts");

  QMap<ShortcutType, QString>::const_iterator i;
  for (i = myConfigKeysMap.begin(); i != myConfigKeysMap.end(); ++i)
    iniFile.set(i.value().toAscii().data(),
        myShortcutsMap[i.key()].isEmpty() ? "None" :
        myShortcutsMap[i.key()].toString(QKeySequence::PortableText).toLatin1().data());
}

void Config::Shortcuts::blockUpdates(bool block)
{
  myBlockUpdates = block;

  if (block)
    return;

  if (myShortcutsHasChanged)
  {
    myShortcutsHasChanged = false;
    emit shortcutsChanged();
  }
}

void Config::Shortcuts::setShortcut(ShortcutType function, const QKeySequence& shortcut)
{
  QKeySequence& var = myShortcutsMap[function];
  if (var == shortcut)
    return;

  var = shortcut;

  if (myBlockUpdates)
    myShortcutsHasChanged = true;
  else
    emit shortcutsChanged();
}
