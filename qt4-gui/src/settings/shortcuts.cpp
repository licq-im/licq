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

#include <QGroupBox>
#include <QHeaderView>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "config/shortcuts.h"

#include "widgets/shortcutedit.h"

#include "settingsdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Settings::Shortcuts */

Settings::Shortcuts::Shortcuts(SettingsDlg* parent)
  : QObject(parent)
{
  parent->addPage(SettingsDlg::MainwinShortcutsPage, createPageMainwinShortcuts(parent),
      tr("Shortcuts"), SettingsDlg::ContactListPage);
  parent->addPage(SettingsDlg::ChatShortcutsPage, createPageChatShortcuts(parent),
      tr("Shortcuts"), SettingsDlg::ChatPage);

  load();
}

QWidget* Settings::Shortcuts::createPageMainwinShortcuts(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageMainwinShortcutsLayout = new QVBoxLayout(w);
  myPageMainwinShortcutsLayout->setContentsMargins(0, 0, 0, 0);

  myMainwinBox = new QGroupBox(tr("Contact List Shortcuts"));
  myMainwinLayout = new QVBoxLayout(myMainwinBox);

  myMainwinList = new QTreeWidget();
  myMainwinList->setSelectionMode(QTreeWidget::NoSelection);
  myMainwinList->setSelectionBehavior(QTreeWidget::SelectRows);
  myMainwinList->setIndentation(0);
  QStringList headers;
  headers << tr("Action") << tr("Shortcut");
  myMainwinList->setHeaderLabels(headers);
  QTreeWidgetItem* item;
  ShortcutEdit* editor;

#define ADD_MAINWINSHORTCUT(shortcut, name) \
  item = new QTreeWidgetItem(myMainwinList); \
  item->setText(0, name); \
  editor = new ShortcutEdit(); \
  connect(editor, SIGNAL(keySequenceChanged(const QKeySequence&)), SLOT(mainwinShortcutChanged(const QKeySequence&))); \
  myMainwinShortcuts.insert(shortcut, editor); \
  myMainwinList->setItemWidget(item, 1, editor);

  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinUserViewMessage, tr("View message from user"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinUserSendMessage, tr("Send message to user"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinUserSendUrl, tr("Send URL to user"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinUserSendFile, tr("Send file to user"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinUserSendChatRequest, tr("Send chat request to user"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinUserCheckAutoresponse, tr("Check user auto response"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinUserViewHistory, tr("View user history"))

  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinAccountManager, tr("Open account manager"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinPopupAllMessages, tr("Open all unread messages"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinPopupMessage, tr("Open next unread message"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinAddGroup, tr("Add group"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinEditGroups, tr("Edit groups"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinRedrawContactList, tr("Redraw user window"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinSetAutoResponse, tr("Set auto response"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinNetworkLog, tr("Show network log"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinToggleMiniMode, tr("Toggle mini mode"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinToggleShowOffline, tr("Toggle show offline users"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinToggleEmptyGroups, tr("Toggle empty groups"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinToggleShowHeader, tr("Toggle column headers visible"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinSettings, tr("Open settings"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinHide, tr("Hide contact list"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinExit, tr("Exit Licq"))

  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinStatusOnline, tr("Switch status to online"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinStatusAway, tr("Switch status to away"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinStatusNotAvailable, tr("Switch status to N/A"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinStatusOccupied, tr("Switch status to occupied"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinStatusDoNotDisturb, tr("Switch status to DND"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinStatusFreeForChat, tr("Switch status to Free For Chat"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinStatusOffline, tr("Switch status to offline"))
  ADD_MAINWINSHORTCUT(Config::Shortcuts::MainwinStatusInvisible, tr("Toggle status Invisible"))

#undef ADD_MAINWINSHORTCUT

  myMainwinList->resizeColumnToContents(0);
  myMainwinLayout->addWidget(myMainwinList);

  myPageMainwinShortcutsLayout->addWidget(myMainwinBox);

  return w;
}

QWidget* Settings::Shortcuts::createPageChatShortcuts(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageChatShortcutsLayout = new QVBoxLayout(w);
  myPageChatShortcutsLayout->setContentsMargins(0, 0, 0, 0);

  myChatBox = new QGroupBox(tr("Contact List Shortcuts"));
  myChatLayout = new QVBoxLayout(myChatBox);

  myChatList = new QTreeWidget();
  myChatList->setSelectionMode(QTreeWidget::NoSelection);
  myChatList->setSelectionBehavior(QTreeWidget::SelectRows);
  myChatList->setIndentation(0);
  QStringList headers;
  headers << tr("Action") << tr("Shortcut");
  myChatList->setHeaderLabels(headers);
  QTreeWidgetItem* item;
  ShortcutEdit* editor;

#define ADD_CHATSHORTCUT(shortcut, name) \
  item = new QTreeWidgetItem(myChatList); \
  item->setText(0, name); \
  editor = new ShortcutEdit(); \
  connect(editor, SIGNAL(keySequenceChanged(const QKeySequence&)), SLOT(chatShortcutChanged(const QKeySequence&))); \
  myChatShortcuts.insert(shortcut, editor); \
  myChatList->setItemWidget(item, 1, editor);

  ADD_CHATSHORTCUT(Config::Shortcuts::ChatUserMenu, tr("Open user menu"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatHistory, tr("Open history"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatUserInfo, tr("Open user information"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatEncodingMenu, tr("Select encoding"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatToggleSecure, tr("Open / close secure channel"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatEventMenu, tr("Select message type"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatToggleSendServer, tr("Toggle send through server"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatToggleUrgent, tr("Toggle urgent"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatToggleMassMessage, tr("Toggle multiple recipients"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatEmoticonMenu, tr("Insert smiley"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatColorFore, tr("Change text color"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatColorBack, tr("Change background color"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatPopupNextMessage, tr("Open next unread message"))

  ADD_CHATSHORTCUT(Config::Shortcuts::ChatTab01, tr("Switch to tab 1"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatTab02, tr("Switch to tab 2"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatTab03, tr("Switch to tab 3"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatTab04, tr("Switch to tab 4"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatTab05, tr("Switch to tab 5"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatTab06, tr("Switch to tab 6"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatTab07, tr("Switch to tab 7"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatTab08, tr("Switch to tab 8"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatTab09, tr("Switch to tab 9"))
  ADD_CHATSHORTCUT(Config::Shortcuts::ChatTab10, tr("Switch to tab 10"))

#undef ADD_CHATSHORTCUT

  myChatList->resizeColumnToContents(0);
  myChatLayout->addWidget(myChatList);

  myPageChatShortcutsLayout->addWidget(myChatBox);

  return w;
}

void Settings::Shortcuts::load()
{
  Config::Shortcuts* shortcutsConfig = Config::Shortcuts::instance();
  QMap<Config::Shortcuts::ShortcutType, ShortcutEdit*>::iterator i;

  for (i = myMainwinShortcuts.begin(); i != myMainwinShortcuts.end(); ++i)
    i.value()->setKeySequence(shortcutsConfig->getShortcut(i.key()));
  myMainwinList->resizeColumnToContents(1);

  for (i = myChatShortcuts.begin(); i != myChatShortcuts.end(); ++i)
    i.value()->setKeySequence(shortcutsConfig->getShortcut(i.key()));
  myChatList->resizeColumnToContents(1);
}

void Settings::Shortcuts::apply()
{
  Config::Shortcuts* shortcutsConfig = Config::Shortcuts::instance();
  shortcutsConfig->blockUpdates(true);

  QMap<Config::Shortcuts::ShortcutType, ShortcutEdit*>::iterator i;
  for (i = myMainwinShortcuts.begin(); i != myMainwinShortcuts.end(); ++i)
    shortcutsConfig->setShortcut(i.key(), i.value()->keySequence());
  for (i = myChatShortcuts.begin(); i != myChatShortcuts.end(); ++i)
    shortcutsConfig->setShortcut(i.key(), i.value()->keySequence());

  shortcutsConfig->blockUpdates(false);
}

void Settings::Shortcuts::mainwinShortcutChanged(const QKeySequence& shortcut)
{
  // An empty shortcut is never a conflict
  if (shortcut.isEmpty())
    return;

  // Get the action that was changed so we can except it from the check
  ShortcutEdit* editor = dynamic_cast<ShortcutEdit*>(sender());
  if (editor == NULL)
    return;

  // Check if shortcut is in conflict with any of the other actions
  QMap<Config::Shortcuts::ShortcutType, ShortcutEdit*>::iterator i;
  for (i = myMainwinShortcuts.begin(); i != myMainwinShortcuts.end(); ++i)
    if (i.value() != editor && i.value()->keySequence() == shortcut)
      i.value()->setKeySequence(QKeySequence());
}

void Settings::Shortcuts::chatShortcutChanged(const QKeySequence& shortcut)
{
  // An empty shortcut is never a conflict
  if (shortcut.isEmpty())
    return;

  // Get the action that was changed so we can except it from the check
  ShortcutEdit* editor = dynamic_cast<ShortcutEdit*>(sender());
  if (editor == NULL)
    return;

  // Check if shortcut is in conflict with any of the other actions
  QMap<Config::Shortcuts::ShortcutType, ShortcutEdit*>::iterator i;
  for (i = myChatShortcuts.begin(); i != myChatShortcuts.end(); ++i)
    if (i.value() != editor && i.value()->keySequence() == shortcut)
      i.value()->setKeySequence(QKeySequence());
}
