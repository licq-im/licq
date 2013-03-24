/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012-2013 Licq developers <licq-dev@googlegroups.com>
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
#include <QListWidget>
#include <QVBoxLayout>

#include <licq/contactlist/owner.h>
#include <licq/icq/icq.h>
#include <licq/icq/owner.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>

#include "config/iconmanager.h"
#include "dialogs/randomchatdlg.h"
#include "widgets/infofield.h"
#include "widgets/skinnablelabel.h"
#include "widgets/specialspinbox.h"

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

  if (myProtocolId == ICQ_PPID)
  {
    parent->addPage(UserDlg::OwnerSecurityPage, createPageIcqSecurity(parent),
        tr("ICQ Security"), UserDlg::OwnerPage);
    parent->addPage(UserDlg::OwnerChatGroupPage, createPageIcqChatGroup(parent),
        tr("ICQ Random Chat Group"), UserDlg::OwnerPage);
  }
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
  myServerHostEdit->setPlaceholderText(tr("Protocol default"));
#endif
  myServerHostEdit->setToolTip(tr("Host name or IP address of server to connect to. "
      "Leave blank to use protocol default."));
  accountLayout->addWidget(myServerHostEdit, 2, 1);
  myServerPortSpin = new SpecialSpinBox(0, 0xFFFF, tr("Auto"));
  myServerPortSpin->setToolTip(tr("Port number for server. \"Auto\" will use protocol default."));
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


  QGroupBox* icqBox = NULL;
  if (myProtocolId == ICQ_PPID)
  {
    icqBox = new QGroupBox(tr("ICQ"));
    QGridLayout* icqLayout = new QGridLayout(icqBox);

    mySSListCheck = new QCheckBox(tr("Use server side contact list"));
    mySSListCheck->setToolTip(tr("Store your contacts on the server so they are accessible from different locations and/or programs"));
    icqLayout->addWidget(mySSListCheck, 0, 0);

    myReconnectAfterUinClashCheck = new QCheckBox(tr("Reconnect after Uin clash"));
    myReconnectAfterUinClashCheck->setToolTip(tr("Licq can reconnect you when you got "
        "disconnected because your Uin was used "
        "from another location. Check this if you "
        "want Licq to reconnect automatically."));
    icqLayout->addWidget(myReconnectAfterUinClashCheck, 1, 0);

    myAutoUpdateInfoCheck = new QCheckBox(tr("Auto update contact information"));
    myAutoUpdateInfoCheck->setToolTip(tr("Automatically update users' server stored information."));
    icqLayout->addWidget(myAutoUpdateInfoCheck, 0, 1);

    myAutoUpdateInfoPluginsCheck = new QCheckBox(tr("Auto update info plugins"));
    myAutoUpdateInfoPluginsCheck->setToolTip(tr("Automatically update users' Phone Book and Picture."));
    icqLayout->addWidget(myAutoUpdateInfoPluginsCheck, 1, 1);

    myAutoUpdateStatusPluginsCheck = new QCheckBox(tr("Auto update status plugins"));
    myAutoUpdateStatusPluginsCheck->setToolTip(tr("Automatically update users' Phone \"Follow Me\", File Server and ICQphone status."));
    icqLayout->addWidget(myAutoUpdateStatusPluginsCheck, 2, 1);
  }


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
  if (icqBox != NULL)
    mainLayout->addWidget(icqBox);
  mainLayout->addStretch(1);
  return w;
}

QWidget* UserPages::Owner::createPageIcqSecurity(QWidget* parent)
{
  QGroupBox* icqSecurityBox = new QGroupBox(tr("ICQ Security Settings"));
  QVBoxLayout* icqSecurityLayout = new QVBoxLayout(icqSecurityBox);

  myIcqRequireAuthCheck = new QCheckBox(tr("Authorization required"));
  myIcqRequireAuthCheck->setToolTip(tr("Determines whether regular ICQ clients "
      "require your authorization to add you to their contact list."));
  icqSecurityLayout->addWidget(myIcqRequireAuthCheck);

  myIcqWebAwareCheck = new QCheckBox(tr("Web presence"));
  myIcqWebAwareCheck->setToolTip(tr("Web Presence allows users to see if you "
      "are online through your web indicator."));
  icqSecurityLayout->addWidget(myIcqWebAwareCheck);

  QWidget* w = new QWidget(parent);
  QVBoxLayout* mainLayout = new QVBoxLayout(w);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(icqSecurityBox);
  mainLayout->addStretch(1);
  return w;
}

QWidget* UserPages::Owner::createPageIcqChatGroup(QWidget* parent)
{
  QGroupBox* icqChatGroupBox = new QGroupBox(tr("ICQ Random Chat Group"));
  QVBoxLayout* icqChatGroupLayout = new QVBoxLayout(icqChatGroupBox);

  myIcqChatGroupList = new QListWidget();
  icqChatGroupLayout->addWidget(myIcqChatGroupList);

  RandomChatDlg::fillGroupsList(myIcqChatGroupList, true, 0);

  QWidget* w = new QWidget(parent);
  QVBoxLayout* mainLayout = new QVBoxLayout(w);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(icqChatGroupBox);
  mainLayout->addStretch(1);
  return w;
}

void UserPages::Owner::load(const Licq::User* user)
{
  const Licq::Owner* owner = dynamic_cast<const Licq::Owner*>(user);
  myUserId = user->id();

  myAccountEdit->setText(QString::fromLocal8Bit(owner->accountId().c_str()));
  myPasswordEdit->setText(QString::fromLocal8Bit(owner->password().c_str()));
  mySavePwdCheck->setChecked(owner->SavePassword());
  myServerHostEdit->setText(QString::fromLocal8Bit(owner->serverHost().c_str()));
  myServerPortSpin->setValue(owner->serverPort());


  int item = myAutoLogonCombo->findData(owner->startupStatus() & ~User::InvisibleStatus);
  myAutoLogonCombo->setCurrentIndex(item);
  myAutoLogonInvisibleCheck->setChecked(owner->startupStatus() & User::InvisibleStatus);

  if (myProtocolId == ICQ_PPID)
  {
    const Licq::IcqOwner* icqowner = dynamic_cast<const Licq::IcqOwner*>(owner);
    mySSListCheck->setChecked(icqowner->useServerContactList());
    myReconnectAfterUinClashCheck->setChecked(icqowner->reconnectAfterUinClash());
    myAutoUpdateInfoCheck->setChecked(icqowner->autoUpdateInfo());
    myAutoUpdateInfoPluginsCheck->setChecked(icqowner->autoUpdateInfoPlugins());
    myAutoUpdateStatusPluginsCheck->setChecked(icqowner->autoUpdateStatusPlugins());

    myIcqRequireAuthCheck->setChecked(icqowner->GetAuthorization());
    myIcqWebAwareCheck->setChecked(icqowner->WebAware());

    unsigned chatGroup = icqowner->randomChatGroup();
    for (int i = 0; i < myIcqChatGroupList->count(); ++i)
      if (chatGroup == myIcqChatGroupList->item(i)->data(Qt::UserRole).toUInt())
      {
        myIcqChatGroupList->setCurrentRow(i);
        break;
      }
  }
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

  if (myProtocolId == ICQ_PPID)
  {
    Licq::IcqOwner* icqowner = dynamic_cast<Licq::IcqOwner*>(owner);
    icqowner->setReconnectAfterUinClash(myReconnectAfterUinClashCheck->isChecked());
    icqowner->setUseServerContactList(mySSListCheck->isChecked());
    icqowner->setAutoUpdateInfo(myAutoUpdateInfoCheck->isChecked());
    icqowner->setAutoUpdateInfoPlugins(myAutoUpdateInfoPluginsCheck->isChecked());
    icqowner->setAutoUpdateStatusPlugins(myAutoUpdateStatusPluginsCheck->isChecked());
  }
}

unsigned long UserPages::Owner::send(UserDlg::UserPage page)
{
  if (myProtocolId == ICQ_PPID)
  {
    Licq::IcqProtocol::Ptr icq = plugin_internal_cast<Licq::IcqProtocol>(
        Licq::gPluginManager.getProtocolInstance(myUserId));
    if (!icq)
      return 0;

    if (page == UserDlg::OwnerSecurityPage)
    {
      return icq->icqSetSecurityInfo(myUserId,
          myIcqRequireAuthCheck->isChecked(),
          myIcqWebAwareCheck->isChecked());
    }

    if (page == UserDlg::OwnerChatGroupPage)
    {
      unsigned chatGroup = myIcqChatGroupList->currentItem()->data(Qt::UserRole).toUInt();
      return icq->setRandomChatGroup(myUserId, chatGroup);
    }
  }

  return 0;
}

void UserPages::Owner::userUpdated(const Licq::User* user, unsigned long subSignal)
{
  if (subSignal == Licq::PluginSignal::UserSettings)
    load(user);
}
