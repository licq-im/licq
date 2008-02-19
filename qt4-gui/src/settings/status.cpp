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
  layPageStatus = new QVBoxLayout(w);
  layPageStatus->setContentsMargins(0, 0, 0, 0);

  boxAutoLogon = new QGroupBox(tr("Startup"));
  layAutoLogon = new QVBoxLayout(boxAutoLogon);

  cmbAutoLogon = new QComboBox();
  cmbAutoLogon->addItem(tr("Offline"));
  cmbAutoLogon->addItem(tr("Online"));
  cmbAutoLogon->addItem(tr("Away"));
  cmbAutoLogon->addItem(tr("Not Available"));
  cmbAutoLogon->addItem(tr("Occupied"));
  cmbAutoLogon->addItem(tr("Do Not Disturb"));
  cmbAutoLogon->addItem(tr("Free for Chat"));
  cmbAutoLogon->setToolTip(tr("Automatically log on when first starting up."));
  layAutoLogon->addWidget(cmbAutoLogon);

  chkAutoLogonInvisible = new QCheckBox(tr("Invisible"));
  layAutoLogon->addWidget(chkAutoLogonInvisible);


  boxAutoAway = new QGroupBox(tr("Auto Change Status"));
  layAutoAway = new QGridLayout(boxAutoAway);
  layAutoAway->setColumnMinimumWidth(2, 20);

  lblAutoAway = new QLabel(tr("Auto Away:"));
  lblAutoAway->setToolTip(tr("Number of minutes of inactivity after which to "
                                 "automatically be marked \"away\".  Set to \"0\" to disable."));
  layAutoAway->addWidget(lblAutoAway, 0, 0);

  spnAutoAway = new QSpinBox();
  spnAutoAway->setSpecialValueText(tr("Never"));
  layAutoAway->addWidget(spnAutoAway, 0, 1);

  cmbAutoAwayMess = new QComboBox();
  layAutoAway->addWidget(cmbAutoAwayMess, 0, 3);

  lblAutoNa = new QLabel(tr("Auto N/A:"));
  lblAutoNa->setToolTip(tr("Number of minutes of inactivity after which to "
                               "automatically be marked \"not available\".  Set to \"0\" to disable."));
  layAutoAway->addWidget(lblAutoNa, 1, 0);

  spnAutoNa = new QSpinBox();
  spnAutoNa->setSpecialValueText(tr("Never"));
  layAutoAway->addWidget(spnAutoNa, 1, 1);

  cmbAutoNAMess = new QComboBox();
  layAutoAway->addWidget(cmbAutoNAMess, 1, 3);

  lblAutoOffline = new QLabel(tr("Auto Offline:"));
  lblAutoOffline->setToolTip(tr("Number of minutes of inactivity after which to "
                               "automatically go offline.  Set to \"0\" to disable."));
  layAutoAway->addWidget(lblAutoOffline, 2, 0);

  spnAutoOffline = new QSpinBox();
  spnAutoOffline->setSpecialValueText(tr("Never"));
  layAutoAway->addWidget(spnAutoOffline, 2, 1);

  buildAutoStatusCombos(1);

  layPageStatus->addWidget(boxAutoLogon);
  layPageStatus->addWidget(boxAutoAway);
  layPageStatus->addStretch(1);

  return w;
}

QWidget* Settings::Status::createPageRespMsg(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  layPageRespMsg = new QVBoxLayout(w);
  layPageRespMsg->setContentsMargins(0, 0, 0, 0);

  boxDefRespMsg = new QGroupBox(tr("Default Auto Response Messages"));
  layPageRespMsg->addWidget(boxDefRespMsg);

  layDefRespMsg = new QGridLayout(boxDefRespMsg);

  lblSARgroup = new QLabel(tr("Status:"));
  layDefRespMsg->addWidget(lblSARgroup, 0, 0);

  cmbSARgroup = new QComboBox();
  cmbSARgroup->addItem(tr("Away"), SAR_AWAY);
  cmbSARgroup->addItem(tr("Not Available"), SAR_NA);
  cmbSARgroup->addItem(tr("Occupied"), SAR_OCCUPIED);
  cmbSARgroup->addItem(tr("Do Not Disturb"), SAR_DND);
  cmbSARgroup->addItem(tr("Free For Chat"), SAR_FFC);
  connect(cmbSARgroup, SIGNAL(activated(int)), SLOT(slot_SARgroup_act(int)));
  layDefRespMsg->addWidget(cmbSARgroup, 0, 1);

  lblSARmsg = new QLabel(tr("Preset slot:"));
  layDefRespMsg->addWidget(lblSARmsg, 1, 0);

  cmbSARmsg = new QComboBox();
  cmbSARmsg->setEditable(true);
  cmbSARmsg->setInsertPolicy(QComboBox::InsertAtCurrent);
  connect(cmbSARmsg, SIGNAL(activated(int)), SLOT(slot_SARmsg_act(int)));
  layDefRespMsg->addWidget(cmbSARmsg, 1, 1);

  edtSARtext = new MLEdit(true);
  layDefRespMsg->addWidget(edtSARtext, 2, 0, 1, 3);

  QHBoxLayout* buttons = new QHBoxLayout();

  btnSARhints = new QPushButton(tr("Hints"));
  buttons->addWidget(btnSARhints);
  connect(btnSARhints, SIGNAL(clicked()), SLOT(slot_SARhints()));

  buttons->addStretch();

  btnSARsave = new QPushButton(tr("Save"));
  buttons->addWidget(btnSARsave);
  connect(btnSARsave, SIGNAL(clicked()), SLOT(slot_SARsave_act()));

  layDefRespMsg->addLayout(buttons, 3, 0, 1, 3);

  layDefRespMsg->setColumnStretch(2, 1);

  slot_SARgroup_act(SAR_AWAY);

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
    selectedAway = cmbAutoAwayMess->currentIndex();
    selectedNA   = cmbAutoNAMess->currentIndex();
  }

  cmbAutoAwayMess->clear();
  cmbAutoAwayMess->addItem(tr("Previous Message"),0);
  SARList &sara = gSARManager.Fetch(SAR_AWAY);
  for (unsigned i = 0; i < sara.size(); i++)
    cmbAutoAwayMess->addItem(sara[i]->Name(),i+1);
  gSARManager.Drop();

  cmbAutoNAMess->clear();
  cmbAutoNAMess->addItem(tr("Previous Message"),0);
  SARList &sarn = gSARManager.Fetch(SAR_NA);
  for (unsigned i = 0; i < sarn.size(); i++)
    cmbAutoNAMess->addItem(sarn[i]->Name(),i+1);
  gSARManager.Drop();

  cmbAutoAwayMess->setCurrentIndex(selectedAway);
  cmbAutoNAMess->setCurrentIndex(selectedNA);
}

void Settings::Status::slot_SARmsg_act(int n)
{
  if (n < 0)
    return;

  SARList &sar = gSARManager.Fetch(cmbSARgroup->currentIndex());
  edtSARtext->setText(QString::fromLocal8Bit(sar[n]->AutoResponse()));
  gSARManager.Drop();
}

void Settings::Status::slot_SARgroup_act(int n)
{
  if (n < 0)
    return;

  cmbSARmsg->clear();
  SARList &sar = gSARManager.Fetch(n);
  for (SARListIter i = sar.begin(); i != sar.end(); i++)
    cmbSARmsg->addItem(QString::fromLocal8Bit((*i)->Name()));
  gSARManager.Drop();

  slot_SARmsg_act(0);
}

void Settings::Status::slot_SARsave_act()
{
  SARList& sar = gSARManager.Fetch(cmbSARgroup->currentIndex());
  delete sar[cmbSARmsg->currentIndex()];
  sar[cmbSARmsg->currentIndex()] =
    new CSavedAutoResponse(cmbSARmsg->currentText().toLocal8Bit().data(),
                           edtSARtext->toPlainText().toLocal8Bit().data());

  gSARManager.Drop();
  gSARManager.Save();

  buildAutoStatusCombos(0);
}

void Settings::Status::slot_SARhints()
{
  gMainWindow->showAutoResponseHints(dynamic_cast<QWidget*>(parent()));
}

void Settings::Status::load()
{
  Config::General* generalConfig = Config::General::instance();

  spnAutoAway->setValue(generalConfig->autoAwayTime());
  spnAutoNa->setValue(generalConfig->autoNaTime());
  spnAutoOffline->setValue(generalConfig->autoOfflineTime());
  cmbAutoLogon->setCurrentIndex(generalConfig->autoLogon() % 10);
  chkAutoLogonInvisible->setChecked(generalConfig->autoLogon() >= 10);
}

void Settings::Status::apply()
{
  Config::General* generalConfig = Config::General::instance();
  generalConfig->blockUpdates(true);

  generalConfig->setAutoAwayTime(spnAutoAway->value());
  generalConfig->setAutoNaTime(spnAutoNa->value());
  generalConfig->setAutoOfflineTime(spnAutoOffline->value());
  generalConfig->setAutoLogon(cmbAutoLogon->currentIndex() +
      (chkAutoLogonInvisible->isChecked() ? 10 : 0));
  generalConfig->setAutoAwayMess(cmbAutoAwayMess->currentIndex());
  generalConfig->setAutoNaMess(cmbAutoNAMess->currentIndex());

  generalConfig->blockUpdates(false);
}
