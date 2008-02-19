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
  layPageOnEvent = new QVBoxLayout(w);
  layPageOnEvent->setContentsMargins(0, 0, 0, 0);

  boxNewMsgActions = new QGroupBox(tr("Actions On Incoming Messages"));
  myMsgActionsLayout = new QGridLayout(boxNewMsgActions);

  chkBoldOnMsg = new QCheckBox(tr("Bold message label"));
  chkBoldOnMsg->setToolTip(tr("Show the message info label in bold font if there are incoming messages"));
  myMsgActionsLayout->addWidget(chkBoldOnMsg, 0, 0);

  chkAutoPopup = new QCheckBox(tr("Auto-popup message"));
  chkAutoPopup->setToolTip(tr("Open all incoming messages automatically when received if we are online (or free for chat)"));
  myMsgActionsLayout->addWidget(chkAutoPopup, 1, 0);

  chkAutoFocus = new QCheckBox(tr("Auto-focus message"));
  chkAutoFocus->setToolTip(tr("Automatically focus opened message windows."));
  myMsgActionsLayout->addWidget(chkAutoFocus, 2, 0);

  chkAutoRaise = new QCheckBox(tr("Auto-raise main window"));
  chkAutoRaise->setToolTip(tr("Raise the main window on incoming messages"));
  myMsgActionsLayout->addWidget(chkAutoRaise, 3, 0);

  chkFlashTaskbar = new QCheckBox(tr("Flash taskbar"));
  chkFlashTaskbar->setToolTip(tr("Flash the taskbar on incoming messages"));
  myMsgActionsLayout->addWidget(chkFlashTaskbar, 0, 1);

  chkFlashAll = new QCheckBox(tr("Blink all events"));
  chkFlashAll->setToolTip(tr("All incoming events will blink"));
  myMsgActionsLayout->addWidget(chkFlashAll, 1, 1);

  chkFlashUrgent = new QCheckBox(tr("Blink urgent events"));
  chkFlashUrgent->setToolTip(tr("Only urgent events will blink"));
  myMsgActionsLayout->addWidget(chkFlashUrgent, 2, 1);

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

  boxParanoia = new QGroupBox(tr("Paranoia"));
  layParanoia = new QVBoxLayout(boxParanoia);

  chkIgnoreNewUsers = new QCheckBox(tr("Ignore new users"));
  chkIgnoreNewUsers->setToolTip(tr("Determines if new users are automatically added to your list or must first request authorization."));
  layParanoia->addWidget(chkIgnoreNewUsers);

  chkIgnoreMassMsg = new QCheckBox(tr("Ignore mass messages"));
  chkIgnoreMassMsg->setToolTip(tr("Determines if mass messages are ignored or not."));
  layParanoia->addWidget(chkIgnoreMassMsg);

  chkIgnoreWebPanel = new QCheckBox(tr("Ignore web panel"));
  chkIgnoreWebPanel->setToolTip(tr("Determines if web panel messages are ignored or not."));
  layParanoia->addWidget(chkIgnoreWebPanel);

  chkIgnoreEmailPager = new QCheckBox(tr("Ignore email pager"));
  chkIgnoreEmailPager->setToolTip(tr("Determines if email pager messages are ignored or not."));
  layParanoia->addWidget(chkIgnoreEmailPager);


  layPageOnEvent->addWidget(boxNewMsgActions);
  layPageOnEvent->addWidget(boxParanoia);
  layPageOnEvent->addStretch(1);

  return w;
}

QWidget* Settings::Events::createPageSounds(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  layPageSounds = new QVBoxLayout(w);
  layPageSounds->setContentsMargins(0, 0, 0, 0);

  QHBoxLayout* laySndTopRow = new QHBoxLayout();

  chkOnEvents = new QCheckBox(tr("Sounds enabled"));
  chkOnEvents->setToolTip(tr("Enable running of \"Command\" when the relevant event occurs."));
  connect(chkOnEvents, SIGNAL(toggled(bool)), SLOT(slot_chkOnEventsToggled(bool)));
  laySndTopRow->addWidget(chkOnEvents);

  QWidget* dummy = new QWidget();
  dummy->setFixedSize(50, 1);
  laySndTopRow->addWidget(dummy);

  lblSndPlayer = new QLabel(tr("Command:"));
  lblSndPlayer->setToolTip("<p>" + tr("Command to execute when an event is received.<br>"
      "It will be passed the relevant parameters from below.<br>"
      "Parameters can contain the following expressions <br> "
      "which will be replaced with the relevant information:") + "</p>" +
      gMainWindow->usprintfHelp);
  laySndTopRow->addWidget(lblSndPlayer);

  edtSndPlayer = new FileNameEdit();
  edtSndPlayer->setToolTip(lblSndPlayer->toolTip());
  lblSndPlayer->setBuddy(edtSndPlayer);
  laySndTopRow->addWidget(edtSndPlayer);


  boxEventParams = new QGroupBox(tr("Parameters"));
  layEventParams = new QGridLayout(boxEventParams);

  lblSndMsg = new QLabel(tr("Message:"));
  lblSndMsg->setToolTip(tr("Parameter for received messages"));
  layEventParams->addWidget(lblSndMsg, 0, 0);
  edtSndMsg = new FileNameEdit();
  edtSndMsg->setToolTip(lblSndMsg->toolTip());
  lblSndMsg->setBuddy(edtSndMsg);
  layEventParams->addWidget(edtSndMsg, 0, 1);

  lblSndUrl = new QLabel(tr("URL:"));
  lblSndUrl->setToolTip(tr("Parameter for received URLs"));
  layEventParams->addWidget(lblSndUrl, 1, 0);
  edtSndUrl = new FileNameEdit();
  edtSndUrl->setToolTip(lblSndUrl->toolTip());
  lblSndUrl->setBuddy(edtSndUrl);
  layEventParams->addWidget(edtSndUrl, 1, 1);

  lblSndChat = new QLabel(tr("Chat request:"));
  lblSndChat->setToolTip(tr("Parameter for received chat requests"));
  layEventParams->addWidget(lblSndChat, 2, 0);
  edtSndChat = new FileNameEdit();
  edtSndChat->setToolTip(lblSndChat->toolTip());
  lblSndChat->setBuddy(edtSndChat);
  layEventParams->addWidget(edtSndChat, 2, 1);

  lblSndFile = new QLabel(tr("File transfer:"));
  lblSndFile->setToolTip(tr("Parameter for received file transfers"));
  layEventParams->addWidget(lblSndFile, 3, 0);
  edtSndFile = new FileNameEdit();
  edtSndFile->setToolTip(lblSndFile->toolTip());
  lblSndFile->setBuddy(edtSndFile);
  layEventParams->addWidget(edtSndFile, 3, 1);

  lblSndNotify = new QLabel(tr("Online notify:"));
  lblSndNotify->setToolTip(tr("Parameter for online notification"));
  layEventParams->addWidget(lblSndNotify, 4, 0);
  edtSndNotify = new FileNameEdit();
  edtSndNotify->setToolTip(lblSndNotify->toolTip());
  lblSndNotify->setBuddy(edtSndNotify);
  layEventParams->addWidget(edtSndNotify, 4, 1);

  lblSndSysMsg = new QLabel(tr("System msg:"));
  lblSndSysMsg->setToolTip(tr("Parameter for received system messages"));
  layEventParams->addWidget(lblSndSysMsg, 5, 0);
  edtSndSysMsg = new FileNameEdit();
  edtSndSysMsg->setToolTip(lblSndSysMsg->toolTip());
  lblSndSysMsg->setBuddy(edtSndSysMsg);
  layEventParams->addWidget(edtSndSysMsg, 5, 1);

  lblSndMsgSent = new QLabel(tr("Message sent:"));
  lblSndMsgSent->setToolTip(tr("Parameter for sent messages"));
  layEventParams->addWidget(lblSndMsgSent, 6, 0);
  edtSndMsgSent = new FileNameEdit();
  edtSndMsgSent->setToolTip(lblSndMsgSent->toolTip());
  lblSndMsgSent->setBuddy(edtSndMsgSent);
  layEventParams->addWidget(edtSndMsgSent, 6, 1);


  boxAcceptEvents = new QGroupBox(tr("Enable Events"));
  layAcceptEvents = new QGridLayout(boxAcceptEvents);

  chkOEAway = new QCheckBox(tr("Sounds when Away"));
  chkOEAway->setToolTip(tr("Perform OnEvent command in away mode"));
  layAcceptEvents->addWidget(chkOEAway, 0, 0);

  chkOENA = new QCheckBox(tr("Sounds when N/A"), boxAcceptEvents);
  chkOENA->setToolTip(tr("Perform OnEvent command in not available mode"));
  layAcceptEvents->addWidget(chkOENA, 1, 0);

  chkOEOccupied = new QCheckBox(tr("Sounds when Occupied"), boxAcceptEvents);
  chkOEOccupied->setToolTip(tr("Perform OnEvent command in occupied mode"));
  layAcceptEvents->addWidget(chkOEOccupied, 2, 0);

  chkOEDND = new QCheckBox(tr("Sounds when DND"), boxAcceptEvents);
  chkOEDND->setToolTip(tr("Perform OnEvent command in do not disturb mode"));
  layAcceptEvents->addWidget(chkOEDND, 3, 0);

  chkAlwaysOnlineNotify = new QCheckBox(tr("Online notify when logging on"), boxAcceptEvents);
  chkAlwaysOnlineNotify->setToolTip(tr("Perform the online notify OnEvent "
     "when logging on (this is different from how the Mirabilis client works)"));
  layAcceptEvents->addWidget(chkAlwaysOnlineNotify, 0, 1);


  layPageSounds->addLayout(laySndTopRow);
  layPageSounds->addWidget(boxEventParams);
  layPageSounds->addWidget(boxAcceptEvents);
  layPageSounds->addStretch(1);

  slot_chkOnEventsToggled(chkOnEvents->isChecked());

  return w;
}

void Settings::Events::slot_chkOnEventsToggled(bool b_checked)
{
  edtSndPlayer->setEnabled(b_checked);
  edtSndMsg->setEnabled(b_checked);
  edtSndUrl->setEnabled(b_checked);
  edtSndChat->setEnabled(b_checked);
  edtSndFile->setEnabled(b_checked);
  edtSndNotify->setEnabled(b_checked);
  edtSndSysMsg->setEnabled(b_checked);
  edtSndMsgSent->setEnabled(b_checked);
  chkOEAway->setEnabled(b_checked);
  chkOENA->setEnabled(b_checked);
  chkOEOccupied->setEnabled(b_checked);
  chkOEDND->setEnabled(b_checked);
  chkAlwaysOnlineNotify->setEnabled(b_checked);
}

void Settings::Events::load()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::ContactList* contactListConfig = Config::ContactList::instance();
  Config::General* generalConfig = Config::General::instance();

  chkAutoRaise->setChecked(generalConfig->autoRaiseMainwin());
  chkBoldOnMsg->setChecked(generalConfig->boldOnMsg());
  myHotKeyField->setText(generalConfig->msgPopupKey().isEmpty() ? "none" : generalConfig->msgPopupKey());

  Config::ContactList::FlashMode flash = contactListConfig->flash();
  chkFlashUrgent->setChecked(flash == Config::ContactList::FlashUrgent || flash == Config::ContactList::FlashAll);
  chkFlashAll->setChecked(flash == Config::ContactList::FlashAll);

  chkAutoPopup->setChecked(chatConfig->autoPopup());
  chkAutoFocus->setChecked(chatConfig->autoFocus());
  chkFlashTaskbar->setChecked(chatConfig->flashTaskbar());

  chkIgnoreNewUsers->setChecked(gLicqDaemon->Ignore(IGNORE_NEWUSERS));
  chkIgnoreMassMsg->setChecked(gLicqDaemon->Ignore(IGNORE_MASSMSG));
  chkIgnoreWebPanel->setChecked(gLicqDaemon->Ignore(IGNORE_WEBPANEL));
  chkIgnoreEmailPager->setChecked(gLicqDaemon->Ignore(IGNORE_EMAILPAGER));

  COnEventManager* oem = gLicqDaemon->OnEventManager();
  chkOnEvents->setChecked(oem->CommandType() != ON_EVENT_IGNORE);
  oem->Lock();
  edtSndPlayer->setFileName(oem->Command());
  edtSndMsg->setFileName(oem->Parameter(ON_EVENT_MSG));
  edtSndUrl->setFileName(oem->Parameter(ON_EVENT_URL));
  edtSndChat->setFileName(oem->Parameter(ON_EVENT_CHAT));
  edtSndFile->setFileName(oem->Parameter(ON_EVENT_FILE));
  edtSndNotify->setFileName(oem->Parameter(ON_EVENT_NOTIFY));
  edtSndSysMsg->setFileName(oem->Parameter(ON_EVENT_SYSMSG));
  edtSndMsgSent->setFileName(oem->Parameter(ON_EVENT_MSGSENT));
  oem->Unlock();
  //TODO make general for all plugins
  ICQOwner* o = gUserManager.FetchOwner(LOCK_R);
  if (o != NULL)
  {
    chkOEAway->setChecked(o->AcceptInAway());
    chkOENA->setChecked(o->AcceptInNA());
    chkOEOccupied->setChecked(o->AcceptInOccupied());
    chkOEDND->setChecked(o->AcceptInDND());
    gUserManager.DropOwner();
  }
  chkAlwaysOnlineNotify->setChecked(gLicqDaemon->AlwaysOnlineNotify());
}

void Settings::Events::apply()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::ContactList* contactListConfig = Config::ContactList::instance();
  Config::General* generalConfig = Config::General::instance();
  chatConfig->blockUpdates(true);
  contactListConfig->blockUpdates(true);
  generalConfig->blockUpdates(true);

  generalConfig->setAutoRaiseMainwin(chkAutoRaise->isChecked());
  generalConfig->setBoldOnMsg(chkBoldOnMsg->isChecked());
  generalConfig->setMsgPopupKey(myHotKeyField->text() != "none" ? myHotKeyField->text() : QString());

  if (chkFlashAll->isChecked())
    contactListConfig->setFlash(Config::ContactList::FlashAll);
  else if(chkFlashUrgent->isChecked())
    contactListConfig->setFlash(Config::ContactList::FlashUrgent);
  else
    contactListConfig->setFlash(Config::ContactList::FlashNone);

  chatConfig->setAutoPopup(chkAutoPopup->isChecked());
  chatConfig->setAutoFocus(chkAutoFocus->isChecked());
  chatConfig->setFlashTaskbar(chkFlashTaskbar->isChecked());

  gLicqDaemon->SetIgnore(IGNORE_NEWUSERS, chkIgnoreNewUsers->isChecked());
  gLicqDaemon->SetIgnore(IGNORE_MASSMSG, chkIgnoreMassMsg->isChecked());
  gLicqDaemon->SetIgnore(IGNORE_WEBPANEL, chkIgnoreWebPanel->isChecked());
  gLicqDaemon->SetIgnore(IGNORE_EMAILPAGER, chkIgnoreEmailPager->isChecked());

  COnEventManager* oem = gLicqDaemon->OnEventManager();
  oem->SetCommandType(chkOnEvents->isChecked() ? ON_EVENT_RUN : ON_EVENT_IGNORE);

  QString txtSndPlayer = edtSndPlayer->fileName();
  QString txtSndMsg = edtSndMsg->fileName();
  QString txtSndUrl = edtSndUrl->fileName();
  QString txtSndChat = edtSndChat->fileName();
  QString txtSndFile = edtSndFile->fileName();
  QString txtSndNotify = edtSndNotify->fileName();
  QString txtSndSysMsg = edtSndSysMsg->fileName();
  QString txtSndMsgSent = edtSndMsgSent->fileName();

  const char* oemparams[8] = {
      txtSndMsg.toLatin1(),
      txtSndUrl.toLatin1(),
      txtSndChat.toLatin1(),
      txtSndFile.toLatin1(),
      txtSndNotify.toLatin1(),
      txtSndSysMsg.toLatin1(),
      txtSndMsgSent.toLatin1(),
      0
  };

  oem->SetParameters(txtSndPlayer.toLatin1(), oemparams);
  //TODO Make general for all plugins
  ICQOwner* o = gUserManager.FetchOwner(LOCK_W);
  if (o)
  {
    o->SetEnableSave(false);
    o->SetAcceptInAway(chkOEAway->isChecked());
    o->SetAcceptInNA(chkOENA->isChecked());
    o->SetAcceptInOccupied(chkOEOccupied->isChecked());
    o->SetAcceptInDND(chkOEDND->isChecked());
    o->SetEnableSave(true);
    o->SaveLicqInfo();
    gUserManager.DropOwner();
  }
  gLicqDaemon->SetAlwaysOnlineNotify(chkAlwaysOnlineNotify->isChecked());

  chatConfig->blockUpdates(false);
  contactListConfig->blockUpdates(true);
  generalConfig->blockUpdates(false);
}
