/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#include "owner.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

#include <licq/contactlist/owner.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>

#include "config/iconmanager.h"
#include "widgets/infofield.h"
#include "widgets/skinnablelabel.h"

#include "userdlg.h"

using Licq::User;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserPages::Owner */

UserPages::Owner::Owner(unsigned long protocolId, UserDlg* parent)
  : QObject(parent),
    myProtocolId(protocolId)
{
  parent->addPage(UserDlg::OwnerPage, createPageSettings(parent),
      tr("Settings"));
}

QWidget* UserPages::Owner::createPageSettings(QWidget* parent)
{
  QGroupBox* accountBox = new QGroupBox(tr("Account"));
  QGridLayout* accountLayout = new QGridLayout(accountBox);

  QLabel* idLabel = new QLabel(tr("User ID:"));
  accountLayout->addWidget(idLabel, 0, 0);
  myAccountEdit = new QLineEdit();
  myAccountEdit->setEnabled(false);
  accountLayout->addWidget(myAccountEdit, 0, 1);
  SkinnableLabel* protocolLabel = new SkinnableLabel();
  accountLayout->addWidget(protocolLabel, 0, 2);

  QLabel* passwordLabel = new QLabel(tr("Password:"));
  accountLayout->addWidget(passwordLabel, 1, 0);
  myPasswordEdit = new QLineEdit();
  myPasswordEdit->setEchoMode(QLineEdit::Password);
  accountLayout->addWidget(myPasswordEdit, 1, 1);
  mySavePwdCheck = new QCheckBox(tr("Save"));
  accountLayout->addWidget(mySavePwdCheck, 1, 2);

  QLabel* serverLabel = new QLabel(tr("Server:"));
  accountLayout->addWidget(serverLabel, 2, 0);
  myServerHostEdit = new QLineEdit();
  accountLayout->addWidget(myServerHostEdit, 2, 1);
  myServerPortSpin = new QSpinBox();
  myServerPortSpin->setRange(0, 0xFFFF);
  accountLayout->addWidget(myServerPortSpin, 2, 2);


  QLabel* statusLabel = new QLabel(tr("Startup status:"));
  accountLayout->addWidget(statusLabel, 3, 0);
  myAutoLogonCombo = new QComboBox();
  myAutoLogonCombo->setToolTip(tr("Automatically log on when first starting up."));
  accountLayout->addWidget(myAutoLogonCombo, 3, 1);

#define ADD_STATUS(status, cond) \
  if (cond) \
    myAutoLogonCombo->addItem(User::statusToString(status).c_str(), status);

  ADD_STATUS(User::OfflineStatus, true);
  ADD_STATUS(User::OnlineStatus, true);
  ADD_STATUS(User::OnlineStatus | User::AwayStatus, true);
  ADD_STATUS(User::OnlineStatus | User::NotAvailableStatus, myProtocolId != MSN_PPID);
  ADD_STATUS(User::OnlineStatus | User::OccupiedStatus, myProtocolId != JABBER_PPID);
  ADD_STATUS(User::OnlineStatus | User::DoNotDisturbStatus, myProtocolId != MSN_PPID);
  ADD_STATUS(User::OnlineStatus | User::FreeForChatStatus, myProtocolId != MSN_PPID);
#undef ADD_STATUS

  myAutoLogonInvisibleCheck = new QCheckBox(tr("Invisible"));
  if (myProtocolId == JABBER_PPID)
    myAutoLogonInvisibleCheck->setEnabled(false);
  accountLayout->addWidget(myAutoLogonInvisibleCheck, 3, 2);


  Licq::ProtocolPlugin::Ptr protocol = Licq::gPluginManager.getProtocolPlugin(myProtocolId);
  if (protocol.get() != NULL)
  {
    protocolLabel->setText(QString::fromLocal8Bit(protocol->name().c_str()));
    protocolLabel->setPrependPixmap(IconManager::instance()->iconForProtocol(myProtocolId));
  }


  QWidget* w = new QWidget(parent);
  QVBoxLayout* mainLayout = new QVBoxLayout(w);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(accountBox);
  mainLayout->addStretch(1);
  return w;
}

void UserPages::Owner::load(const Licq::User* user)
{
  const Licq::Owner* owner = dynamic_cast<const Licq::Owner*>(user);

  myAccountEdit->setText(QString::fromLocal8Bit(owner->accountId().c_str()));
  myPasswordEdit->setText(QString::fromLocal8Bit(owner->password().c_str()));
  mySavePwdCheck->setChecked(owner->SavePassword());
  myServerHostEdit->setText(QString::fromLocal8Bit(owner->serverHost().c_str()));
  myServerPortSpin->setValue(owner->serverPort());


  int item = myAutoLogonCombo->findData(owner->startupStatus() & ~User::InvisibleStatus);
  myAutoLogonCombo->setCurrentIndex(item);
  myAutoLogonInvisibleCheck->setChecked(owner->startupStatus() & User::InvisibleStatus);
}

void UserPages::Owner::apply(Licq::User* user)
{
  Licq::Owner* owner = dynamic_cast<Licq::Owner*>(user);

  owner->setPassword(myPasswordEdit->text().toLocal8Bit().constData());
  owner->SetSavePassword(mySavePwdCheck->isChecked());
  owner->setServer(myServerHostEdit->text().toLocal8Bit().constData(), myServerPortSpin->value());

  int index = myAutoLogonCombo->currentIndex();
  unsigned long status = myAutoLogonCombo->itemData(index).toUInt();
  if (status != User::OfflineStatus && myAutoLogonInvisibleCheck->isChecked())
    status |= User::InvisibleStatus;
  owner->setStartupStatus(status);
}

void UserPages::Owner::userUpdated(const Licq::User* user, unsigned long subSignal)
{
  if (subSignal == Licq::PluginSignal::UserSettings)
    load(user);
}
