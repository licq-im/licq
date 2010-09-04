// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2010 Licq developers
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

#include "events.h"

#include "config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include <licq/contactlist/owner.h>
#include <licq/daemon.h>
#include <licq/oneventmanager.h>

#include "config/chat.h"
#include "config/contactlist.h"
#include "config/general.h"
#include "widgets/shortcutedit.h"

#include "oneventbox.h"
#include "settingsdlg.h"


using Licq::OnEventData;
using Licq::gOnEventManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Settings::Events */

Settings::Events::Events(SettingsDlg* parent)
  : QObject(parent)
{
  parent->addPage(SettingsDlg::OnEventPage, createPageOnEvent(parent),
      tr("Events"));
  parent->addPage(SettingsDlg::SoundsPage, createPageSounds(parent),
      tr("Sounds"), SettingsDlg::OnEventPage);

  load();
}

QWidget* Settings::Events::createPageOnEvent(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageOnEventLayout = new QVBoxLayout(w);
  myPageOnEventLayout->setContentsMargins(0, 0, 0, 0);

  myNewMsgActionsBox = new QGroupBox(tr("Actions On Incoming Messages"));
  myMsgActionsLayout = new QGridLayout(myNewMsgActionsBox);

  myBoldOnMsgCheck = new QCheckBox(tr("Bold message label"));
  myBoldOnMsgCheck->setToolTip(tr("Show the message info label in bold font if there are incoming messages"));
  myMsgActionsLayout->addWidget(myBoldOnMsgCheck, 0, 0);

  myAutoFocusCheck = new QCheckBox(tr("Auto-focus message"));
  myAutoFocusCheck->setToolTip(tr("Automatically focus opened message windows."));
  myMsgActionsLayout->addWidget(myAutoFocusCheck, 1, 0);

  myAutoRaiseCheck = new QCheckBox(tr("Auto-raise main window"));
  myAutoRaiseCheck->setToolTip(tr("Raise the main window on incoming messages"));
  myMsgActionsLayout->addWidget(myAutoRaiseCheck, 2, 0);

  QHBoxLayout* autoPopupLayout = new QHBoxLayout();
  QLabel* autoPopupLabel = new QLabel(tr("Auto-popup message:"));
  autoPopupLayout->addWidget(autoPopupLabel);
  myAutoPopupCombo = new QComboBox();
  myAutoPopupCombo->addItem(tr("Never"));
  myAutoPopupCombo->addItem(tr("Only when online"));
  myAutoPopupCombo->addItem(tr("When online or away"));
  myAutoPopupCombo->addItem(tr("When online, away or N/A"));
  myAutoPopupCombo->addItem(tr("Always except DND"));
  myAutoPopupCombo->addItem(tr("Always"));
  myAutoPopupCombo->setToolTip(tr("Select for which statuses incoming messages should "
      "open automatically.\nOnline also includes Free for chat."));
  autoPopupLabel->setBuddy(myAutoPopupCombo);
  autoPopupLayout->addWidget(myAutoPopupCombo);
  myMsgActionsLayout->addLayout(autoPopupLayout, 3, 0);

  myAutoUrgentCheck = new QCheckBox(tr("Auto-popup urgent only"));
  myAutoUrgentCheck->setToolTip(tr("Only auto-popup urgent messages."));
  myMsgActionsLayout->addWidget(myAutoUrgentCheck, 4, 0);

  myFlashTaskbarCheck = new QCheckBox(tr("Flash taskbar"));
  myFlashTaskbarCheck->setToolTip(tr("Flash the taskbar on incoming messages"));
  myMsgActionsLayout->addWidget(myFlashTaskbarCheck, 0, 1);

  myFlashAllCheck = new QCheckBox(tr("Blink all events"));
  myFlashAllCheck->setToolTip(tr("All incoming events will blink"));
  myMsgActionsLayout->addWidget(myFlashAllCheck, 1, 1);

  myFlashUrgentCheck = new QCheckBox(tr("Blink urgent events"));
  myFlashUrgentCheck->setToolTip(tr("Only urgent events will blink"));
  myMsgActionsLayout->addWidget(myFlashUrgentCheck, 2, 1);

  QHBoxLayout* hotKeyLayout = new QHBoxLayout();
  myHotKeyLabel = new QLabel(tr("Hot key:"));
  hotKeyLayout->addWidget(myHotKeyLabel);
  myHotKeyLabel->setToolTip(tr("Hotkey to pop up the next pending message.\n"
      "Enter the hotkey literally, like \"shift+f10\", or \"none\" for disabling."));
  myHotKeyEdit = new ShortcutEdit();
  myHotKeyEdit->setToolTip(myHotKeyLabel->toolTip());
  myHotKeyLabel->setBuddy(myHotKeyEdit);
  hotKeyLayout->addWidget(myHotKeyEdit);
  myMsgActionsLayout->addLayout(hotKeyLayout, 3, 1);

  myParanoiaBox = new QGroupBox(tr("Paranoia"));
  myParanoiaLayout = new QVBoxLayout(myParanoiaBox);

  myIgnoreNewUsersCheck = new QCheckBox(tr("Ignore new users"));
  myIgnoreNewUsersCheck->setToolTip(tr("Determines if new users are automatically added to your list or must first request authorization."));
  myParanoiaLayout->addWidget(myIgnoreNewUsersCheck);

  myIgnoreMassMsgCheck = new QCheckBox(tr("Ignore mass messages"));
  myIgnoreMassMsgCheck->setToolTip(tr("Determines if mass messages are ignored or not."));
  myParanoiaLayout->addWidget(myIgnoreMassMsgCheck);

  myIgnoreWebPanelCheck = new QCheckBox(tr("Ignore web panel"));
  myIgnoreWebPanelCheck->setToolTip(tr("Determines if web panel messages are ignored or not."));
  myParanoiaLayout->addWidget(myIgnoreWebPanelCheck);

  myIgnoreEmailPagerCheck = new QCheckBox(tr("Ignore email pager"));
  myIgnoreEmailPagerCheck->setToolTip(tr("Determines if email pager messages are ignored or not."));
  myParanoiaLayout->addWidget(myIgnoreEmailPagerCheck);


  myPageOnEventLayout->addWidget(myNewMsgActionsBox);
  myPageOnEventLayout->addWidget(myParanoiaBox);
  myPageOnEventLayout->addStretch(1);

  return w;
}

QWidget* Settings::Events::createPageSounds(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageSoundsLayout = new QVBoxLayout(w);
  myPageSoundsLayout->setContentsMargins(0, 0, 0, 0);

  myOnEventBox = new OnEventBox(true);
  myPageSoundsLayout->addWidget(myOnEventBox);

  QGridLayout* soundsLayout = dynamic_cast<QGridLayout*>(myOnEventBox->layout());
  int soundsRows = soundsLayout->rowCount();

  myNoSoundInActiveChatCheck = new QCheckBox(tr("Disable sound for active window"));
  myNoSoundInActiveChatCheck->setToolTip(tr("Don't perform OnEvent command if chat window for user is currently active."));
  soundsLayout->addWidget(myNoSoundInActiveChatCheck, soundsRows, 1, 1, 3);

  myPageSoundsLayout->addStretch(1);

  return w;
}

void Settings::Events::load()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::ContactList* contactListConfig = Config::ContactList::instance();
  Config::General* generalConfig = Config::General::instance();

  myAutoRaiseCheck->setChecked(generalConfig->autoRaiseMainwin());
  myBoldOnMsgCheck->setChecked(generalConfig->boldOnMsg());
  myHotKeyEdit->setKeySequence(QKeySequence(generalConfig->msgPopupKey()));

  Config::ContactList::FlashMode flash = contactListConfig->flash();
  myFlashUrgentCheck->setChecked(flash == Config::ContactList::FlashUrgent || flash == Config::ContactList::FlashAll);
  myFlashAllCheck->setChecked(flash == Config::ContactList::FlashAll);

  myAutoPopupCombo->setCurrentIndex(chatConfig->autoPopup());
  myAutoUrgentCheck->setChecked(chatConfig->autoPopupUrgentOnly());
  myAutoFocusCheck->setChecked(chatConfig->autoFocus());
  myFlashTaskbarCheck->setChecked(chatConfig->flashTaskbar());
  myNoSoundInActiveChatCheck->setChecked(chatConfig->noSoundInActiveChat());

  myIgnoreNewUsersCheck->setChecked(Licq::gDaemon.ignoreType(Licq::Daemon::IgnoreNewUsers));
  myIgnoreMassMsgCheck->setChecked(Licq::gDaemon.ignoreType(Licq::Daemon::IgnoreMassMsg));
  myIgnoreWebPanelCheck->setChecked(Licq::gDaemon.ignoreType(Licq::Daemon::IgnoreWebPanel));
  myIgnoreEmailPagerCheck->setChecked(Licq::gDaemon.ignoreType(Licq::Daemon::IgnoreEmailPager));

  const OnEventData* eventData = gOnEventManager.lockGlobal();
  myOnEventBox->load(eventData, NULL);
  gOnEventManager.unlock(eventData);
}

void Settings::Events::apply()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::ContactList* contactListConfig = Config::ContactList::instance();
  Config::General* generalConfig = Config::General::instance();
  chatConfig->blockUpdates(true);
  contactListConfig->blockUpdates(true);
  generalConfig->blockUpdates(true);

  generalConfig->setAutoRaiseMainwin(myAutoRaiseCheck->isChecked());
  generalConfig->setBoldOnMsg(myBoldOnMsgCheck->isChecked());
  generalConfig->setMsgPopupKey(myHotKeyEdit->keySequence().toString(QKeySequence::PortableText));

  if (myFlashAllCheck->isChecked())
    contactListConfig->setFlash(Config::ContactList::FlashAll);
  else if(myFlashUrgentCheck->isChecked())
    contactListConfig->setFlash(Config::ContactList::FlashUrgent);
  else
    contactListConfig->setFlash(Config::ContactList::FlashNone);

  chatConfig->setAutoPopup(myAutoPopupCombo->currentIndex());
  chatConfig->setAutoPopupUrgentOnly(myAutoUrgentCheck->isChecked());
  chatConfig->setAutoFocus(myAutoFocusCheck->isChecked());
  chatConfig->setFlashTaskbar(myFlashTaskbarCheck->isChecked());
  chatConfig->setNoSoundInActiveChat(myNoSoundInActiveChatCheck->isChecked());

  Licq::gDaemon.setIgnoreType(Licq::Daemon::IgnoreNewUsers, myIgnoreNewUsersCheck->isChecked());
  Licq::gDaemon.setIgnoreType(Licq::Daemon::IgnoreMassMsg, myIgnoreMassMsgCheck->isChecked());
  Licq::gDaemon.setIgnoreType(Licq::Daemon::IgnoreWebPanel, myIgnoreWebPanelCheck->isChecked());
  Licq::gDaemon.setIgnoreType(Licq::Daemon::IgnoreEmailPager, myIgnoreEmailPagerCheck->isChecked());

  OnEventData* eventData = gOnEventManager.lockGlobal();
  myOnEventBox->apply(eventData);
  gOnEventManager.unlock(eventData, true);

  chatConfig->blockUpdates(false);
  contactListConfig->blockUpdates(false);
  generalConfig->blockUpdates(false);
}
