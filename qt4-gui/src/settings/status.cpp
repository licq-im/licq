// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2009 Licq developers
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

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <licq_sar.h>

#include "config/general.h"

#include "core/mainwin.h"

#include "widgets/mledit.h"

#include "settingsdlg.h"

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

  myGeneralBox = new QGroupBox(tr("General Status Options"));
  myGeneralLayout = new QVBoxLayout(myGeneralBox);

  myDelayStatusChangeCheck = new QCheckBox(tr("Delay status changes"));
  myDelayStatusChangeCheck->setToolTip(tr("Set status after closing the away message dialog instead of setting it directly."));
  myGeneralLayout->addWidget(myDelayStatusChangeCheck);

  myAutoLogonBox = new QGroupBox(tr("Startup"));
  myAutoLogonLayout = new QVBoxLayout(myAutoLogonBox);

  myAutoLogonCombo = new QComboBox();
  myAutoLogonCombo->addItem(tr("Offline"));
  myAutoLogonCombo->addItem(tr("Online"));
  myAutoLogonCombo->addItem(tr("Away"));
  myAutoLogonCombo->addItem(tr("Not Available"));
  myAutoLogonCombo->addItem(tr("Occupied"));
  myAutoLogonCombo->addItem(tr("Do Not Disturb"));
  myAutoLogonCombo->addItem(tr("Free for Chat"));
  myAutoLogonCombo->setToolTip(tr("Automatically log on when first starting up."));
  myAutoLogonLayout->addWidget(myAutoLogonCombo);

  myAutoLogonInvisibleCheck = new QCheckBox(tr("Invisible"));
  myAutoLogonLayout->addWidget(myAutoLogonInvisibleCheck);


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

  myPageStatusLayout->addWidget(myGeneralBox);
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
  mySarGroupCombo->addItem(tr("Away"), SAR_AWAY);
  mySarGroupCombo->addItem(tr("Not Available"), SAR_NA);
  mySarGroupCombo->addItem(tr("Occupied"), SAR_OCCUPIED);
  mySarGroupCombo->addItem(tr("Do Not Disturb"), SAR_DND);
  mySarGroupCombo->addItem(tr("Free For Chat"), SAR_FFC);
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

  sarGroupChanged(SAR_AWAY);

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
  SARList &sara = gSARManager.Fetch(SAR_AWAY);
  for (unsigned i = 0; i < sara.size(); i++)
    myAutoAwayMessCombo->addItem(sara[i]->Name(),i+1);
  gSARManager.Drop();

  myAutoNaMessCombo->clear();
  myAutoNaMessCombo->addItem(tr("Previous Message"),0);
  SARList &sarn = gSARManager.Fetch(SAR_NA);
  for (unsigned i = 0; i < sarn.size(); i++)
    myAutoNaMessCombo->addItem(sarn[i]->Name(),i+1);
  gSARManager.Drop();

  myAutoAwayMessCombo->setCurrentIndex(selectedAway);
  myAutoNaMessCombo->setCurrentIndex(selectedNA);
}

void Settings::Status::sarMsgChanged(int msg)
{
  if (msg < 0)
    return;

  SARList &sar = gSARManager.Fetch(mySarGroupCombo->currentIndex());
  mySartextEdit->setText(QString::fromLocal8Bit(sar[msg]->AutoResponse()));
  gSARManager.Drop();
}

void Settings::Status::sarGroupChanged(int group)
{
  if (group < 0)
    return;

  mySarMsgCombo->clear();
  SARList &sar = gSARManager.Fetch(group);
  for (SARListIter i = sar.begin(); i != sar.end(); i++)
    mySarMsgCombo->addItem(QString::fromLocal8Bit((*i)->Name()));
  gSARManager.Drop();

  sarMsgChanged(0);
}

void Settings::Status::saveSar()
{
  SARList& sar = gSARManager.Fetch(mySarGroupCombo->currentIndex());
  delete sar[mySarMsgCombo->currentIndex()];
  sar[mySarMsgCombo->currentIndex()] =
    new CSavedAutoResponse(mySarMsgCombo->currentText().toLocal8Bit().data(),
        mySartextEdit->toPlainText().toLocal8Bit().data());

  gSARManager.Drop();
  gSARManager.Save();

  buildAutoStatusCombos(0);
}

void Settings::Status::showSarHints()
{
  gMainWindow->showAutoResponseHints(dynamic_cast<QWidget*>(parent()));
}

void Settings::Status::load()
{
  Config::General* generalConfig = Config::General::instance();

  myDelayStatusChangeCheck->setChecked(generalConfig->delayStatusChange());
  myAutoAwaySpin->setValue(generalConfig->autoAwayTime());
  myAutoNaSpin->setValue(generalConfig->autoNaTime());
  myAutoOfflineSpin->setValue(generalConfig->autoOfflineTime());
  myAutoLogonCombo->setCurrentIndex(generalConfig->autoLogon() % 10);
  myAutoLogonInvisibleCheck->setChecked(generalConfig->autoLogon() >= 10);
  myAutoAwayMessCombo->setCurrentIndex(generalConfig->autoAwayMess());
  myAutoNaMessCombo->setCurrentIndex(generalConfig->autoNaMess());
}

void Settings::Status::apply()
{
  Config::General* generalConfig = Config::General::instance();
  generalConfig->blockUpdates(true);

  generalConfig->setDelayStatusChange(myDelayStatusChangeCheck->isChecked());
  generalConfig->setAutoAwayTime(myAutoAwaySpin->value());
  generalConfig->setAutoNaTime(myAutoNaSpin->value());
  generalConfig->setAutoOfflineTime(myAutoOfflineSpin->value());
  generalConfig->setAutoLogon(myAutoLogonCombo->currentIndex() +
      (myAutoLogonInvisibleCheck->isChecked() ? 10 : 0));
  generalConfig->setAutoAwayMess(myAutoAwayMessCombo->currentIndex());
  generalConfig->setAutoNaMess(myAutoNaMessCombo->currentIndex());

  generalConfig->blockUpdates(false);
}
