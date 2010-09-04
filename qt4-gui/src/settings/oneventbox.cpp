// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#include "oneventbox.h"

#include "config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

#include <licq/oneventmanager.h>

#include "core/mainwin.h"
#include "widgets/filenameedit.h"

using Licq::OnEventData;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::OnEventBox */

OnEventBox::OnEventBox(bool isGlobal, QWidget* parent) :
  QGroupBox(tr("Sound Options"), parent),
  myIsGlobal(isGlobal)
{
  QGridLayout* mainLayout = new QGridLayout(this);
  mainLayout->setColumnStretch(3, 1);

  QLabel* enabledLabel = new QLabel(tr("Sounds enabled:"));
  enabledLabel->setToolTip(tr("Select for which statuses \"Command\" should be run."));
  mainLayout->addWidget(enabledLabel, 0, 1);
  myEnabledCombo = new QComboBox();
  myEnabledCombo->addItem(tr("Never"));
  myEnabledCombo->addItem(tr("Only when online"));
  myEnabledCombo->addItem(tr("When online or away"));
  myEnabledCombo->addItem(tr("When online, away or N/A"));
  myEnabledCombo->addItem(tr("Always except DND"));
  myEnabledCombo->addItem(tr("Always"));
  myEnabledCombo->setToolTip(enabledLabel->toolTip());
  enabledLabel->setBuddy(myEnabledCombo);
  mainLayout->addWidget(myEnabledCombo, 0, 2);

  QLabel* commandLabel = new QLabel(tr("Command:"));
  commandLabel->setToolTip("<p>" + tr("Command to execute when an event is received.<br>"
      "It will be passed the relevant parameters from below.<br>"
      "Parameters can contain the following expressions <br> "
      "which will be replaced with the relevant information:") + "</p>" +
      gMainWindow->usprintfHelp);
  mainLayout->addWidget(commandLabel, 1, 1);
  myCommandEdit = new FileNameEdit();
  myCommandEdit->setToolTip(commandLabel->toolTip());
  commandLabel->setBuddy(myCommandEdit);
  mainLayout->addWidget(myCommandEdit, 1, 2, 1, 2);

  // Spacing between general properties and parameters
  mainLayout->setRowMinimumHeight(2, 20);

  QLabel* paramLabel[OnEventData::NumOnEventTypes];
  paramLabel[OnEventData::OnEventMessage] = new QLabel(tr("Message:"));
  paramLabel[OnEventData::OnEventMessage]->setToolTip(tr("Parameter for received messages"));
  paramLabel[OnEventData::OnEventUrl] = new QLabel(tr("URL:"));
  paramLabel[OnEventData::OnEventUrl]->setToolTip(tr("Parameter for received URLs"));
  paramLabel[OnEventData::OnEventChat] = new QLabel(tr("Chat request:"));
  paramLabel[OnEventData::OnEventChat]->setToolTip(tr("Parameter for received chat requests"));
  paramLabel[OnEventData::OnEventFile] = new QLabel(tr("File transfer:"));
  paramLabel[OnEventData::OnEventFile]->setToolTip(tr("Parameter for received file transfers"));
  paramLabel[OnEventData::OnEventSms] = new QLabel(tr("SMS:"));
  paramLabel[OnEventData::OnEventSms]->setToolTip(tr("Parameter for received SMSs"));
  paramLabel[OnEventData::OnEventOnline] = new QLabel(tr("Online notify:"));
  paramLabel[OnEventData::OnEventOnline]->setToolTip(tr("Parameter for online notification"));
  paramLabel[OnEventData::OnEventSysMsg] = new QLabel(tr("System msg:"));
  paramLabel[OnEventData::OnEventSysMsg]->setToolTip(tr("Parameter for received system messages"));
  paramLabel[OnEventData::OnEventMsgSent] = new QLabel(tr("Message sent:"));
  paramLabel[OnEventData::OnEventMsgSent]->setToolTip(tr("Parameter for sent messages"));

  for (int i = 0; i < OnEventData::NumOnEventTypes; ++i)
  {
    myParameterEdit[i] = new FileNameEdit();
    myParameterEdit[i]->setToolTip(paramLabel[i]->toolTip());
    paramLabel[i]->setBuddy(myParameterEdit[i]);
    mainLayout->addWidget(paramLabel[i], i+3, 1);
    mainLayout->addWidget(myParameterEdit[i], i+3, 2, 1, 2);
  }

  myAlwaysNotifyCheck = new QCheckBox(tr("Online notify when logging on"));
  myAlwaysNotifyCheck->setToolTip(tr("Perform the online notify OnEvent "
     "when logging on (this is different from how the Mirabilis client works)"));
  mainLayout->addWidget(myAlwaysNotifyCheck, OnEventData::NumOnEventTypes+3, 1, 1, 3);

  if (!myIsGlobal)
  {
    myOverEnabledCheck = new QCheckBox();
    myOverEnabledCheck->setToolTip(tr("Override default setting for enabling sounds"));
    mainLayout->addWidget(myOverEnabledCheck, 0, 0);
    myEnabledCombo->setEnabled(false);
    connect(myOverEnabledCheck, SIGNAL(toggled(bool)), myEnabledCombo, SLOT(setEnabled(bool)));

    myOverCommandCheck = new QCheckBox();
    myOverCommandCheck->setToolTip(tr("Override default command"));
    mainLayout->addWidget(myOverCommandCheck, 1, 0);
    myCommandEdit->setEnabled(false);
    connect(myOverCommandCheck, SIGNAL(toggled(bool)), myCommandEdit, SLOT(setEnabled(bool)));

    for (int i = 0; i < OnEventData::NumOnEventTypes; ++i)
    {
      myOverParameterCheck[i] = new QCheckBox();
      myOverParameterCheck[i]->setToolTip(tr("Override parameter"));
      mainLayout->addWidget(myOverParameterCheck[i], i+3, 0);
      myParameterEdit[i]->setEnabled(false);
      connect(myOverParameterCheck[i], SIGNAL(toggled(bool)), myParameterEdit[i], SLOT(setEnabled(bool)));
    }

    myOverAlwaysNotifyCheck = new QCheckBox();
    myOverAlwaysNotifyCheck->setToolTip(tr("Override always online notify setting"));
    mainLayout->addWidget(myOverAlwaysNotifyCheck, OnEventData::NumOnEventTypes+3, 0);
    myAlwaysNotifyCheck->setEnabled(false);
    connect(myOverAlwaysNotifyCheck, SIGNAL(toggled(bool)), myAlwaysNotifyCheck, SLOT(setEnabled(bool)));
  }
}

void OnEventBox::load(const OnEventData* effectiveData, const OnEventData* realData)
{
  myEnabledCombo->setCurrentIndex(effectiveData->enabled());
  myCommandEdit->setFileName(effectiveData->command().c_str());
  for (int i = 0; i < OnEventData::NumOnEventTypes; ++i)
    myParameterEdit[i]->setFileName(effectiveData->parameter(i).c_str());
  myAlwaysNotifyCheck->setChecked(effectiveData->alwaysOnlineNotify() != 0);

  if (!myIsGlobal)
  {
    myOverEnabledCheck->setChecked(realData != NULL && realData->enabled() != OnEventData::EnabledDefault);
    myOverCommandCheck->setChecked(realData != NULL && realData->command() != OnEventData::Default);
    for (int i = 0; i < OnEventData::NumOnEventTypes; ++i)
      myOverParameterCheck[i]->setChecked(realData != NULL && realData->parameter(i) != OnEventData::Default);
    myOverAlwaysNotifyCheck->setChecked(realData != NULL && realData->alwaysOnlineNotify() != -1);
  }
}

void OnEventBox::apply(OnEventData* eventData)
{
  if (myIsGlobal || myOverEnabledCheck->isChecked())
    eventData->setEnabled(myEnabledCombo->currentIndex());
  else
    eventData->setEnabled(OnEventData::EnabledDefault);

  if (myIsGlobal || myOverCommandCheck->isChecked())
    eventData->setCommand(myCommandEdit->fileName().toLatin1().data());
  else
    eventData->setCommand(OnEventData::Default);

  for (int i = 0; i < OnEventData::NumOnEventTypes; ++i)
  {
    if (myIsGlobal || myOverParameterCheck[i]->isChecked())
      eventData->setParameter(i, myParameterEdit[i]->fileName().toLatin1().data());
    else
      eventData->setParameter(i, OnEventData::Default);
  }

  if (myIsGlobal || myOverAlwaysNotifyCheck->isChecked())
    eventData->setAlwaysOnlineNotify(myAlwaysNotifyCheck->isChecked() ? 1 : 0);
  else
    eventData->setAlwaysOnlineNotify(-1);
}
