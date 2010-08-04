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

#include "status.h"

#include "config.h"

#include <boost/foreach.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/pluginmanager.h>
#include <licq/sarmanager.h>

#include "config/general.h"

#include "core/mainwin.h"

#include "widgets/mledit.h"

#include "settingsdlg.h"

using Licq::SarManager;
using Licq::User;
using Licq::gSarManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Settings::Status */

Settings::Status::Status(SettingsDlg* parent)
  : QObject(parent)
{
  parent->addPage(SettingsDlg::StatusPage, createPageStatus(parent),
      tr("Status"));
  parent->addPage(SettingsDlg::RespMsgPage, createPageRespMsg(parent),
      tr("Auto Response"), SettingsDlg::StatusPage);

  load();
}

QWidget* Settings::Status::createPageStatus(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageStatusLayout = new QVBoxLayout(w);
  myPageStatusLayout->setContentsMargins(0, 0, 0, 0);

  myAutoLogonBox = new QGroupBox(tr("Startup"));
  myAutoLogonLayout = new QGridLayout(myAutoLogonBox);

  {
    Licq::OwnerListGuard ownerList;
    int line = 0;
    BOOST_FOREACH(const Licq::Owner* owner, **ownerList)
    {
      unsigned long protocolId = owner->protocolId();
      Licq::ProtocolPlugin::Ptr protocol = Licq::gPluginManager.getProtocolPlugin(protocolId);
      if (protocol.get() == NULL)
        continue;

      QLabel* autoLogonLabel = new QLabel(QString(protocol->getName()) + ": ");
      myAutoLogonLayout->addWidget(autoLogonLabel, line, 0);

#define ADD_STATUS(status, cond) \
      if (cond) \
        myAutoLogonCombo[protocolId]->addItem(User::statusToString(status).c_str(), status);

      myAutoLogonCombo[protocolId] = new QComboBox();
      ADD_STATUS(User::OfflineStatus, true);
      ADD_STATUS(User::OnlineStatus, true);
      ADD_STATUS(User::OnlineStatus | User::AwayStatus, true);
      ADD_STATUS(User::OnlineStatus | User::NotAvailableStatus, protocolId != MSN_PPID);
      ADD_STATUS(User::OnlineStatus | User::OccupiedStatus, protocolId != JABBER_PPID);
      ADD_STATUS(User::OnlineStatus | User::DoNotDisturbStatus, protocolId != MSN_PPID);
      ADD_STATUS(User::OnlineStatus | User::FreeForChatStatus, protocolId != MSN_PPID);
#undef ADD_STATUS
      myAutoLogonCombo[protocolId]->setToolTip(tr("Automatically log on when first starting up."));
      myAutoLogonLayout->addWidget(myAutoLogonCombo[protocolId], line, 1);

      myAutoLogonInvisibleCheck[protocolId] = new QCheckBox(tr("Invisible"));
      if (protocolId == JABBER_PPID)
        myAutoLogonInvisibleCheck[protocolId]->setEnabled(false);
      myAutoLogonLayout->addWidget(myAutoLogonInvisibleCheck[protocolId], line, 2);

      ++line;
    }
  }

  myAutoAwayBox = new QGroupBox(tr("Auto Change Status"));
  myAutoAwayLayout = new QGridLayout(myAutoAwayBox);
  myAutoAwayLayout->setColumnMinimumWidth(2, 20);

  myAutoAwayLabel = new QLabel(tr("Auto Away:"));
  myAutoAwayLabel->setToolTip(tr("Number of minutes of inactivity after which to "
        "automatically be marked \"away\".  Set to \"0\" to disable."));
  myAutoAwayLayout->addWidget(myAutoAwayLabel, 0, 0);

  myAutoAwaySpin = new QSpinBox();
  myAutoAwaySpin->setSpecialValueText(tr("Never"));
  myAutoAwayLayout->addWidget(myAutoAwaySpin, 0, 1);

  myAutoAwayMessCombo = new QComboBox();
  myAutoAwayLayout->addWidget(myAutoAwayMessCombo, 0, 3);

  myAutoNaLabel = new QLabel(tr("Auto N/A:"));
  myAutoNaLabel->setToolTip(tr("Number of minutes of inactivity after which to "
        "automatically be marked \"not available\".  Set to \"0\" to disable."));
  myAutoAwayLayout->addWidget(myAutoNaLabel, 1, 0);

  myAutoNaSpin = new QSpinBox();
  myAutoNaSpin->setSpecialValueText(tr("Never"));
  myAutoAwayLayout->addWidget(myAutoNaSpin, 1, 1);

  myAutoNaMessCombo = new QComboBox();
  myAutoAwayLayout->addWidget(myAutoNaMessCombo, 1, 3);

  myAutoOfflineLabel = new QLabel(tr("Auto Offline:"));
  myAutoOfflineLabel->setToolTip(tr("Number of minutes of inactivity after which to "
        "automatically go offline.  Set to \"0\" to disable."));
  myAutoAwayLayout->addWidget(myAutoOfflineLabel, 2, 0);

  myAutoOfflineSpin = new QSpinBox();
  myAutoOfflineSpin->setSpecialValueText(tr("Never"));
  myAutoAwayLayout->addWidget(myAutoOfflineSpin, 2, 1);

  buildAutoStatusCombos(1);

  myPageStatusLayout->addWidget(myAutoLogonBox);
  myPageStatusLayout->addWidget(myAutoAwayBox);
  myPageStatusLayout->addStretch(1);

  return w;
}

QWidget* Settings::Status::createPageRespMsg(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageRespMsgLayout = new QVBoxLayout(w);
  myPageRespMsgLayout->setContentsMargins(0, 0, 0, 0);

  myDefRespMsgBox = new QGroupBox(tr("Default Auto Response Messages"));
  myPageRespMsgLayout->addWidget(myDefRespMsgBox);

  myDefRespMsgLayout = new QGridLayout(myDefRespMsgBox);

  mySarGroupLabel = new QLabel(tr("Status:"));
  myDefRespMsgLayout->addWidget(mySarGroupLabel, 0, 0);

  mySarGroupCombo = new QComboBox();
  mySarGroupCombo->addItem(User::statusToString(User::AwayStatus).c_str(), SarManager::AwayList);
  mySarGroupCombo->addItem(User::statusToString(User::NotAvailableStatus).c_str(), SarManager::NotAvailableList);
  mySarGroupCombo->addItem(User::statusToString(User::OccupiedStatus).c_str(), SarManager::OccupiedList);
  mySarGroupCombo->addItem(User::statusToString(User::DoNotDisturbStatus).c_str(), SarManager::DoNotDisturbList);
  mySarGroupCombo->addItem(User::statusToString(User::FreeForChatStatus).c_str(), SarManager::FreeForChatList);
  connect(mySarGroupCombo, SIGNAL(activated(int)), SLOT(sarGroupChanged(int)));
  myDefRespMsgLayout->addWidget(mySarGroupCombo, 0, 1);

  mySarMsgLabel = new QLabel(tr("Preset slot:"));
  myDefRespMsgLayout->addWidget(mySarMsgLabel, 1, 0);

  mySarMsgCombo = new QComboBox();
  mySarMsgCombo->setEditable(true);
  mySarMsgCombo->setInsertPolicy(QComboBox::InsertAtCurrent);
  connect(mySarMsgCombo, SIGNAL(activated(int)), SLOT(sarMsgChanged(int)));
  myDefRespMsgLayout->addWidget(mySarMsgCombo, 1, 1);

  mySartextEdit = new MLEdit(true);
  myDefRespMsgLayout->addWidget(mySartextEdit, 2, 0, 1, 3);

  QHBoxLayout* buttons = new QHBoxLayout();

  mySarhintsButton = new QPushButton(tr("Hints"));
  buttons->addWidget(mySarhintsButton);
  connect(mySarhintsButton, SIGNAL(clicked()), SLOT(showSarHints()));

  buttons->addStretch();

  mySarsaveButton = new QPushButton(tr("Save"));
  buttons->addWidget(mySarsaveButton);
  connect(mySarsaveButton, SIGNAL(clicked()), SLOT(saveSar()));

  myDefRespMsgLayout->addLayout(buttons, 3, 0, 1, 3);

  myDefRespMsgLayout->setColumnStretch(2, 1);

  sarGroupChanged(SarManager::AwayList);

  return w;
}

void Settings::Status::buildAutoStatusCombos(bool firstTime)
{
  int selectedNA, selectedAway;

  //Save selection (or get first selection)
  if (firstTime)
  {
    selectedAway = Config::General::instance()->autoAwayMess();
    selectedNA   = Config::General::instance()->autoNaMess();
  }
  else
  {
    selectedAway = myAutoAwayMessCombo->currentIndex();
    selectedNA   = myAutoNaMessCombo->currentIndex();
  }

  myAutoAwayMessCombo->clear();
  myAutoAwayMessCombo->addItem(tr("Previous Message"),0);
  const Licq::SarList& sarsAway(gSarManager.getList(SarManager::AwayList));
  int count = 0;
  for (Licq::SarList::const_iterator i = sarsAway.begin(); i != sarsAway.end(); ++i)
    myAutoAwayMessCombo->addItem(QString::fromLocal8Bit(i->name.c_str()), ++count);
  gSarManager.releaseList();

  myAutoNaMessCombo->clear();
  myAutoNaMessCombo->addItem(tr("Previous Message"),0);
  const Licq::SarList& sarsNa(gSarManager.getList(SarManager::NotAvailableList));
  count = 0;
  for (Licq::SarList::const_iterator i = sarsNa.begin(); i != sarsNa.end(); ++i)
    myAutoNaMessCombo->addItem(QString::fromLocal8Bit(i->name.c_str()), ++count);
  gSarManager.releaseList();

  myAutoAwayMessCombo->setCurrentIndex(selectedAway);
  myAutoNaMessCombo->setCurrentIndex(selectedNA);
}

void Settings::Status::sarMsgChanged(int msg)
{
  if (msg < 0)
    return;

  const Licq::SarList& sars(gSarManager.getList(static_cast<SarManager::List>(mySarGroupCombo->currentIndex())));
  mySartextEdit->setText(QString::fromLocal8Bit(sars[msg].text.c_str()));
  gSarManager.releaseList();
}

void Settings::Status::sarGroupChanged(int group)
{
  if (group < 0)
    return;

  mySarMsgCombo->clear();
  const Licq::SarList& sars(gSarManager.getList(static_cast<SarManager::List>(group)));
  for (Licq::SarList::const_iterator i = sars.begin(); i != sars.end(); ++i)
    mySarMsgCombo->addItem(QString::fromLocal8Bit(i->name.c_str()));
  gSarManager.releaseList();

  sarMsgChanged(0);
}

void Settings::Status::saveSar()
{
  Licq::SarList& sars(gSarManager.getList(static_cast<SarManager::List>(mySarGroupCombo->currentIndex())));
  Licq::SavedAutoResponse& sar(sars[mySarMsgCombo->currentIndex()]);
  sar.name = mySarMsgCombo->currentText().toLocal8Bit().data();
  sar.text = mySartextEdit->toPlainText().toLocal8Bit().data();
  gSarManager.releaseList(true);

  buildAutoStatusCombos(0);
}

void Settings::Status::showSarHints()
{
  gMainWindow->showAutoResponseHints(dynamic_cast<QWidget*>(parent()));
}

void Settings::Status::load()
{
  Config::General* generalConfig = Config::General::instance();

  myAutoAwaySpin->setValue(generalConfig->autoAwayTime());
  myAutoNaSpin->setValue(generalConfig->autoNaTime());
  myAutoOfflineSpin->setValue(generalConfig->autoOfflineTime());
  myAutoAwayMessCombo->setCurrentIndex(generalConfig->autoAwayMess());
  myAutoNaMessCombo->setCurrentIndex(generalConfig->autoNaMess());

  Licq::OwnerListGuard ownerList;
  BOOST_FOREACH(const Licq::Owner* owner, **ownerList)
  {
    Licq::OwnerReadGuard o(owner);
    unsigned long protocolId = o->protocolId();

    // In case someone added an owner since we created the page
    if (myAutoLogonCombo.find(protocolId) == myAutoLogonCombo.end())
      continue;

    int item = myAutoLogonCombo[protocolId]->findData(o->startupStatus() & ~User::InvisibleStatus);
    myAutoLogonCombo[protocolId]->setCurrentIndex(item);
    myAutoLogonInvisibleCheck[protocolId]->setChecked(o->startupStatus() & User::InvisibleStatus);
  }
}

void Settings::Status::apply()
{
  Config::General* generalConfig = Config::General::instance();
  generalConfig->blockUpdates(true);

  generalConfig->setAutoAwayTime(myAutoAwaySpin->value());
  generalConfig->setAutoNaTime(myAutoNaSpin->value());
  generalConfig->setAutoOfflineTime(myAutoOfflineSpin->value());

  generalConfig->setAutoAwayMess(myAutoAwayMessCombo->currentIndex());
  generalConfig->setAutoNaMess(myAutoNaMessCombo->currentIndex());

  generalConfig->blockUpdates(false);

  Licq::OwnerListGuard ownerList;
  BOOST_FOREACH(Licq::Owner* owner, **ownerList)
  {
    Licq::OwnerWriteGuard o(owner);
    unsigned long protocolId = o->protocolId();

    // In case someone added an owner since we created the page
    if (myAutoLogonCombo.find(protocolId) == myAutoLogonCombo.end())
      continue;

    int index = myAutoLogonCombo[protocolId]->currentIndex();
    unsigned long status = myAutoLogonCombo[protocolId]->itemData(index).toUInt();
    if (status != User::OfflineStatus && myAutoLogonInvisibleCheck[protocolId]->isChecked())
      status |= User::InvisibleStatus;

    // Don't trigger unnecessary updates
    if (status == o->startupStatus())
      continue;

    o->setStartupStatus(status);
    o->SaveLicqInfo();
  }
}
