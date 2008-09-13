// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Licq developers
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
#include <QLineEdit>
#include <QVBoxLayout>

#include <licq_icqd.h>
#include <licq_onevent.h>

#include "config/chat.h"
#include "config/contactlist.h"
#include "config/general.h"
#include "core/mainwin.h"
#include "widgets/filenameedit.h"

#include "settingsdlg.h"


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
  myHotKeyField = new QLineEdit();
  myHotKeyField->setToolTip(myHotKeyLabel->toolTip());
  myHotKeyLabel->setBuddy(myHotKeyField);
  hotKeyLayout->addWidget(myHotKeyField);
  myMsgActionsLayout->addLayout(hotKeyLayout, 3, 1);

  // Make the columns evenly wide, otherwise the QLineEdit gets too wide
  myMsgActionsLayout->setColumnStretch(0, 1);
  myMsgActionsLayout->setColumnStretch(1, 1);

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

  QHBoxLayout* mySndTopRowLayout = new QHBoxLayout();

  myOnEventsCheck = new QCheckBox(tr("Sounds enabled"));
  myOnEventsCheck->setToolTip(tr("Enable running of \"Command\" when the relevant event occurs."));
  connect(myOnEventsCheck, SIGNAL(toggled(bool)), SLOT(setOnEventsEnabled(bool)));
  mySndTopRowLayout->addWidget(myOnEventsCheck);

  QWidget* dummy = new QWidget();
  dummy->setFixedSize(50, 1);
  mySndTopRowLayout->addWidget(dummy);

  mySndPlayerLabel = new QLabel(tr("Command:"));
  mySndPlayerLabel->setToolTip("<p>" + tr("Command to execute when an event is received.<br>"
      "It will be passed the relevant parameters from below.<br>"
      "Parameters can contain the following expressions <br> "
      "which will be replaced with the relevant information:") + "</p>" +
      gMainWindow->usprintfHelp);
  mySndTopRowLayout->addWidget(mySndPlayerLabel);

  mySndPlayerEdit = new FileNameEdit();
  mySndPlayerEdit->setToolTip(mySndPlayerLabel->toolTip());
  mySndPlayerLabel->setBuddy(mySndPlayerEdit);
  mySndTopRowLayout->addWidget(mySndPlayerEdit);


  myEventParamsBox = new QGroupBox(tr("Parameters"));
  myEventParamsLayout = new QGridLayout(myEventParamsBox);

  mySndMsgLabel = new QLabel(tr("Message:"));
  mySndMsgLabel->setToolTip(tr("Parameter for received messages"));
  myEventParamsLayout->addWidget(mySndMsgLabel, 0, 0);
  mySndMsgEdit = new FileNameEdit();
  mySndMsgEdit->setToolTip(mySndMsgLabel->toolTip());
  mySndMsgLabel->setBuddy(mySndMsgEdit);
  myEventParamsLayout->addWidget(mySndMsgEdit, 0, 1);

  mySndUrlLabel = new QLabel(tr("URL:"));
  mySndUrlLabel->setToolTip(tr("Parameter for received URLs"));
  myEventParamsLayout->addWidget(mySndUrlLabel, 1, 0);
  mySndUrlEdit = new FileNameEdit();
  mySndUrlEdit->setToolTip(mySndUrlLabel->toolTip());
  mySndUrlLabel->setBuddy(mySndUrlEdit);
  myEventParamsLayout->addWidget(mySndUrlEdit, 1, 1);

  mySndChatLabel = new QLabel(tr("Chat request:"));
  mySndChatLabel->setToolTip(tr("Parameter for received chat requests"));
  myEventParamsLayout->addWidget(mySndChatLabel, 2, 0);
  mySndChatEdit = new FileNameEdit();
  mySndChatEdit->setToolTip(mySndChatLabel->toolTip());
  mySndChatLabel->setBuddy(mySndChatEdit);
  myEventParamsLayout->addWidget(mySndChatEdit, 2, 1);

  mySndFileLabel = new QLabel(tr("File transfer:"));
  mySndFileLabel->setToolTip(tr("Parameter for received file transfers"));
  myEventParamsLayout->addWidget(mySndFileLabel, 3, 0);
  mySndFileEdit = new FileNameEdit();
  mySndFileEdit->setToolTip(mySndFileLabel->toolTip());
  mySndFileLabel->setBuddy(mySndFileEdit);
  myEventParamsLayout->addWidget(mySndFileEdit, 3, 1);

  mySndNotifyLabel = new QLabel(tr("Online notify:"));
  mySndNotifyLabel->setToolTip(tr("Parameter for online notification"));
  myEventParamsLayout->addWidget(mySndNotifyLabel, 4, 0);
  mySndNotifyEdit = new FileNameEdit();
  mySndNotifyEdit->setToolTip(mySndNotifyLabel->toolTip());
  mySndNotifyLabel->setBuddy(mySndNotifyEdit);
  myEventParamsLayout->addWidget(mySndNotifyEdit, 4, 1);

  mySndSysMsgLabel = new QLabel(tr("System msg:"));
  mySndSysMsgLabel->setToolTip(tr("Parameter for received system messages"));
  myEventParamsLayout->addWidget(mySndSysMsgLabel, 5, 0);
  mySndSysMsgEdit = new FileNameEdit();
  mySndSysMsgEdit->setToolTip(mySndSysMsgLabel->toolTip());
  mySndSysMsgLabel->setBuddy(mySndSysMsgEdit);
  myEventParamsLayout->addWidget(mySndSysMsgEdit, 5, 1);

  mySndMsgSentLabel = new QLabel(tr("Message sent:"));
  mySndMsgSentLabel->setToolTip(tr("Parameter for sent messages"));
  myEventParamsLayout->addWidget(mySndMsgSentLabel, 6, 0);
  mySndMsgSentEdit = new FileNameEdit();
  mySndMsgSentEdit->setToolTip(mySndMsgSentLabel->toolTip());
  mySndMsgSentLabel->setBuddy(mySndMsgSentEdit);
  myEventParamsLayout->addWidget(mySndMsgSentEdit, 6, 1);


  myAcceptEventsBox = new QGroupBox(tr("Enable Events"));
  myAcceptEventsLayout = new QGridLayout(myAcceptEventsBox);

  myOnEventAwayCheck = new QCheckBox(tr("Sounds when Away"));
  myOnEventAwayCheck->setToolTip(tr("Perform OnEvent command in away mode"));
  myAcceptEventsLayout->addWidget(myOnEventAwayCheck, 0, 0);

  myOnEventNaCheck = new QCheckBox(tr("Sounds when N/A"), myAcceptEventsBox);
  myOnEventNaCheck->setToolTip(tr("Perform OnEvent command in not available mode"));
  myAcceptEventsLayout->addWidget(myOnEventNaCheck, 1, 0);

  myOnEventOccupiedCheck = new QCheckBox(tr("Sounds when Occupied"), myAcceptEventsBox);
  myOnEventOccupiedCheck->setToolTip(tr("Perform OnEvent command in occupied mode"));
  myAcceptEventsLayout->addWidget(myOnEventOccupiedCheck, 2, 0);

  myOnEventDndCheck = new QCheckBox(tr("Sounds when DND"), myAcceptEventsBox);
  myOnEventDndCheck->setToolTip(tr("Perform OnEvent command in do not disturb mode"));
  myAcceptEventsLayout->addWidget(myOnEventDndCheck, 3, 0);

  myAlwaysOnlineNotifyCheck = new QCheckBox(tr("Online notify when logging on"), myAcceptEventsBox);
  myAlwaysOnlineNotifyCheck->setToolTip(tr("Perform the online notify OnEvent "
     "when logging on (this is different from how the Mirabilis client works)"));
  myAcceptEventsLayout->addWidget(myAlwaysOnlineNotifyCheck, 0, 1);


  myPageSoundsLayout->addLayout(mySndTopRowLayout);
  myPageSoundsLayout->addWidget(myEventParamsBox);
  myPageSoundsLayout->addWidget(myAcceptEventsBox);
  myPageSoundsLayout->addStretch(1);

  setOnEventsEnabled(myOnEventsCheck->isChecked());

  return w;
}

void Settings::Events::setOnEventsEnabled(bool enable)
{
  mySndPlayerEdit->setEnabled(enable);
  mySndMsgEdit->setEnabled(enable);
  mySndUrlEdit->setEnabled(enable);
  mySndChatEdit->setEnabled(enable);
  mySndFileEdit->setEnabled(enable);
  mySndNotifyEdit->setEnabled(enable);
  mySndSysMsgEdit->setEnabled(enable);
  mySndMsgSentEdit->setEnabled(enable);
  myOnEventAwayCheck->setEnabled(enable);
  myOnEventNaCheck->setEnabled(enable);
  myOnEventOccupiedCheck->setEnabled(enable);
  myOnEventDndCheck->setEnabled(enable);
  myAlwaysOnlineNotifyCheck->setEnabled(enable);
}

void Settings::Events::load()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::ContactList* contactListConfig = Config::ContactList::instance();
  Config::General* generalConfig = Config::General::instance();

  myAutoRaiseCheck->setChecked(generalConfig->autoRaiseMainwin());
  myBoldOnMsgCheck->setChecked(generalConfig->boldOnMsg());
  myHotKeyField->setText(generalConfig->msgPopupKey().isEmpty() ? "none" : generalConfig->msgPopupKey());

  Config::ContactList::FlashMode flash = contactListConfig->flash();
  myFlashUrgentCheck->setChecked(flash == Config::ContactList::FlashUrgent || flash == Config::ContactList::FlashAll);
  myFlashAllCheck->setChecked(flash == Config::ContactList::FlashAll);

  myAutoPopupCombo->setCurrentIndex(chatConfig->autoPopup());
  myAutoFocusCheck->setChecked(chatConfig->autoFocus());
  myFlashTaskbarCheck->setChecked(chatConfig->flashTaskbar());

  myIgnoreNewUsersCheck->setChecked(gLicqDaemon->Ignore(IGNORE_NEWUSERS));
  myIgnoreMassMsgCheck->setChecked(gLicqDaemon->Ignore(IGNORE_MASSMSG));
  myIgnoreWebPanelCheck->setChecked(gLicqDaemon->Ignore(IGNORE_WEBPANEL));
  myIgnoreEmailPagerCheck->setChecked(gLicqDaemon->Ignore(IGNORE_EMAILPAGER));

  COnEventManager* oem = gLicqDaemon->OnEventManager();
  myOnEventsCheck->setChecked(oem->CommandType() != ON_EVENT_IGNORE);
  oem->Lock();
  mySndPlayerEdit->setFileName(QString::fromStdString(oem->command()));
  mySndMsgEdit->setFileName(QString::fromStdString(oem->parameter(ON_EVENT_MSG)));
  mySndUrlEdit->setFileName(QString::fromStdString(oem->parameter(ON_EVENT_URL)));
  mySndChatEdit->setFileName(QString::fromStdString(oem->parameter(ON_EVENT_CHAT)));
  mySndFileEdit->setFileName(QString::fromStdString(oem->parameter(ON_EVENT_FILE)));
  mySndNotifyEdit->setFileName(QString::fromStdString(oem->parameter(ON_EVENT_NOTIFY)));
  mySndSysMsgEdit->setFileName(QString::fromStdString(oem->parameter(ON_EVENT_SYSMSG)));
  mySndMsgSentEdit->setFileName(QString::fromStdString(oem->parameter(ON_EVENT_MSGSENT)));
  oem->Unlock();

  //TODO make general for all plugins
  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o != NULL)
  {
    myOnEventAwayCheck->setChecked(o->AcceptInAway());
    myOnEventNaCheck->setChecked(o->AcceptInNA());
    myOnEventOccupiedCheck->setChecked(o->AcceptInOccupied());
    myOnEventDndCheck->setChecked(o->AcceptInDND());
    gUserManager.DropOwner(o);
  }
  myAlwaysOnlineNotifyCheck->setChecked(gLicqDaemon->AlwaysOnlineNotify());
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
  generalConfig->setMsgPopupKey(myHotKeyField->text() != "none" ? myHotKeyField->text() : QString());

  if (myFlashAllCheck->isChecked())
    contactListConfig->setFlash(Config::ContactList::FlashAll);
  else if(myFlashUrgentCheck->isChecked())
    contactListConfig->setFlash(Config::ContactList::FlashUrgent);
  else
    contactListConfig->setFlash(Config::ContactList::FlashNone);

  chatConfig->setAutoPopup(myAutoPopupCombo->currentIndex());
  chatConfig->setAutoFocus(myAutoFocusCheck->isChecked());
  chatConfig->setFlashTaskbar(myFlashTaskbarCheck->isChecked());

  gLicqDaemon->SetIgnore(IGNORE_NEWUSERS, myIgnoreNewUsersCheck->isChecked());
  gLicqDaemon->SetIgnore(IGNORE_MASSMSG, myIgnoreMassMsgCheck->isChecked());
  gLicqDaemon->SetIgnore(IGNORE_WEBPANEL, myIgnoreWebPanelCheck->isChecked());
  gLicqDaemon->SetIgnore(IGNORE_EMAILPAGER, myIgnoreEmailPagerCheck->isChecked());

  COnEventManager* oem = gLicqDaemon->OnEventManager();
  oem->SetCommandType(myOnEventsCheck->isChecked() ? ON_EVENT_RUN : ON_EVENT_IGNORE);

  oem->setCommand(mySndPlayerEdit->fileName().toStdString());
  oem->setParameter(ON_EVENT_MSG, mySndMsgEdit->fileName().toStdString());
  oem->setParameter(ON_EVENT_URL, mySndUrlEdit->fileName().toStdString());
  oem->setParameter(ON_EVENT_CHAT, mySndChatEdit->fileName().toStdString());
  oem->setParameter(ON_EVENT_FILE, mySndFileEdit->fileName().toStdString());
  oem->setParameter(ON_EVENT_NOTIFY, mySndNotifyEdit->fileName().toStdString());
  oem->setParameter(ON_EVENT_SYSMSG, mySndSysMsgEdit->fileName().toStdString());
  oem->setParameter(ON_EVENT_MSGSENT, mySndMsgSentEdit->fileName().toStdString());

  //TODO Make general for all plugins
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  if (o)
  {
    o->SetEnableSave(false);
    o->SetAcceptInAway(myOnEventAwayCheck->isChecked());
    o->SetAcceptInNA(myOnEventNaCheck->isChecked());
    o->SetAcceptInOccupied(myOnEventOccupiedCheck->isChecked());
    o->SetAcceptInDND(myOnEventDndCheck->isChecked());
    o->SetEnableSave(true);
    o->SaveLicqInfo();
    gUserManager.DropOwner(o);
  }
  gLicqDaemon->SetAlwaysOnlineNotify(myAlwaysOnlineNotifyCheck->isChecked());

  chatConfig->blockUpdates(false);
  contactListConfig->blockUpdates(false);
  generalConfig->blockUpdates(false);
}
