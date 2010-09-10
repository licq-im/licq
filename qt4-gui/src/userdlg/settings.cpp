// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2008-2010 Licq developers
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

#include "settings.h"

#include "config.h"

#include <boost/foreach.hpp>

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QRadioButton>
#include <QStringList>
#include <QTableWidget>
#include <QVBoxLayout>

#include <licq/contactlist/group.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/oneventmanager.h>
#include <licq/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>

#include "contactlist/contactlist.h"
#include "dialogs/awaymsgdlg.h"
#include "settings/oneventbox.h"
#include "widgets/mledit.h"

#include "userdlg.h"

using Licq::gProtocolManager;
using Licq::User;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserPages::Settings */

UserPages::Settings::Settings(bool isOwner, UserDlg* parent)
  : QObject(parent),
    myIsOwner(isOwner)
{
  // No settings here for owner so don't create anything
  if (myIsOwner)
    return;

  parent->addPage(UserDlg::SettingsPage, createPageSettings(parent),
      tr("Settings"));
  parent->addPage(UserDlg::StatusPage, createPageStatus(parent),
      tr("Status"), UserDlg::SettingsPage);
  parent->addPage(UserDlg::OnEventPage, createPageOnEvent(parent),
      tr("Sounds"), UserDlg::SettingsPage);
  parent->addPage(UserDlg::GroupsPage, createPageGroups(parent),
      tr("Groups"));
}

QWidget* UserPages::Settings::createPageSettings(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageModesLayout = new QGridLayout(w);
  myPageModesLayout->setContentsMargins(0, 0, 0, 0);

  mySettingsBox = new QGroupBox(tr("Misc Modes"));
  mySettingsLayout = new QGridLayout(mySettingsBox);

  myAcceptInAwayCheck = new QCheckBox(tr("Accept in away"));
  myAcceptInAwayCheck->setToolTip(tr("Play sounds for this contact when my status is away."));
  mySettingsLayout->addWidget(myAcceptInAwayCheck, 0, 0);

  myAcceptInNaCheck = new QCheckBox(tr("Accept in not available"));
  myAcceptInNaCheck->setToolTip(tr("Play sounds for this contact when my status is not available."));
  mySettingsLayout->addWidget(myAcceptInNaCheck, 1, 0);

  myAcceptInOccupiedCheck = new QCheckBox(tr("Accept in occupied"));
  myAcceptInOccupiedCheck->setToolTip(tr("Play sounds for this contact when my status is occupied."));
  mySettingsLayout->addWidget(myAcceptInOccupiedCheck, 2, 0);

  myAcceptInDndCheck = new QCheckBox(tr("Accept in do not disturb"));
  myAcceptInDndCheck->setToolTip(tr("Play sounds for this contact when my status is do not disturb."));
  mySettingsLayout->addWidget(myAcceptInDndCheck, 3, 0);

  myAutoAcceptFileCheck = new QCheckBox(tr("Auto accept files"));
  myAutoAcceptFileCheck->setToolTip(tr("Automatically accept file transfers from this contact."));
  mySettingsLayout->addWidget(myAutoAcceptFileCheck, 0, 1);

  myAutoAcceptChatCheck = new QCheckBox(tr("Auto accept chats"));
  myAutoAcceptChatCheck->setToolTip(tr("Automatically accept chat requests from this contact."));
  mySettingsLayout->addWidget(myAutoAcceptChatCheck, 1, 1);

  myAutoSecureCheck = new QCheckBox(tr("Auto request secure"));
  myAutoSecureCheck->setToolTip(tr("Automatically request secure channel to this contact."));
  mySettingsLayout->addWidget(myAutoSecureCheck, 2, 1);

  myUseGpgCheck = new QCheckBox(tr("Use GPG encryption"));
  myUseGpgCheck->setToolTip(tr("Use GPG encryption for messages with this contact."));
  mySettingsLayout->addWidget(myUseGpgCheck, 3, 1);
  if (!Licq::gDaemon.haveGpgSupport())
    myUseGpgCheck->setVisible(false);

  myUseRealIpCheck = new QCheckBox(tr("Use real ip (LAN)"));
  myUseRealIpCheck->setToolTip(tr("Use real IP for when sending to this contact."));
  mySettingsLayout->addWidget(myUseRealIpCheck, 4, 0);

  myPageModesLayout->addWidget(mySettingsBox, 0, 0, 1, 2);
  myPageModesLayout->setRowStretch(1, 1);

  return w;
}

QWidget* UserPages::Settings::createPageStatus(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageStatusLayout = new QGridLayout(w);
  myPageStatusLayout->setContentsMargins(0, 0, 0, 0);

  myStatusBox = new QGroupBox(tr("Status to User"));
  myStatusLayout = new QVBoxLayout(myStatusBox);

  myStatusNoneRadio = new QRadioButton(tr("Not overridden"));
  myStatusNoneRadio->setToolTip(tr("Contact will see your normal status."));
  myStatusLayout->addWidget(myStatusNoneRadio);

  myStatusOnlineRadio = new QRadioButton(tr("Online"));
  myStatusOnlineRadio->setToolTip(tr("Contact will always see you as online."));
  myStatusLayout->addWidget(myStatusOnlineRadio);

  myStatusAwayRadio = new QRadioButton(tr("Away"));
  myStatusAwayRadio->setToolTip(tr("Contact will always see your status as away."));
  myStatusLayout->addWidget(myStatusAwayRadio);

  myStatusNaRadio = new QRadioButton(tr("Not available"));
  myStatusNaRadio->setToolTip(tr("Contact will always see your status as not available."));
  myStatusLayout->addWidget(myStatusNaRadio);

  myStatusOccupiedRadio = new QRadioButton(tr("Occupied"));
  myStatusOccupiedRadio->setToolTip(tr("Contact will always see your status as occupied."));
  myStatusLayout->addWidget(myStatusOccupiedRadio);

  myStatusDndRadio = new QRadioButton(tr("Do not disturb"));
  myStatusDndRadio->setToolTip(tr("Contact will always see your status as do not disturb."));
  myStatusLayout->addWidget(myStatusDndRadio);

  myStatusLayout->addStretch(1);


  mySysGroupBox = new QGroupBox(tr("System Groups"));
  mySysGroupLayout = new QVBoxLayout(mySysGroupBox);

  myOnlineNotifyCheck = new QCheckBox(ContactListModel::systemGroupName(ContactListModel::OnlineNotifyGroupId));
  myOnlineNotifyCheck->setToolTip(tr("Notify when this contact comes online."));
  mySysGroupLayout->addWidget(myOnlineNotifyCheck);

  myVisibleListCheck = new QCheckBox(ContactListModel::systemGroupName(ContactListModel::VisibleListGroupId));
  myVisibleListCheck->setToolTip(tr("Contact will see you online even if you're invisible."));
  mySysGroupLayout->addWidget(myVisibleListCheck);

  myInvisibleListCheck = new QCheckBox(ContactListModel::systemGroupName(ContactListModel::InvisibleListGroupId));
  myInvisibleListCheck->setToolTip(tr("Contact will always see you as offline."));
  mySysGroupLayout->addWidget(myInvisibleListCheck);

  myIgnoreListCheck = new QCheckBox(ContactListModel::systemGroupName(ContactListModel::IgnoreListGroupId));
  myIgnoreListCheck->setToolTip(tr("Ignore any events from this contact."));
  mySysGroupLayout->addWidget(myIgnoreListCheck);

  myNewUserCheck = new QCheckBox(ContactListModel::systemGroupName(ContactListModel::NewUsersGroupId));
  myNewUserCheck->setToolTip(tr("Contact was recently added to the list."));
  mySysGroupLayout->addWidget(myNewUserCheck);

  mySysGroupLayout->addStretch(1);

  myAutoRespBox = new QGroupBox(tr("Custom Auto Response"));
  myAutoRespLayout = new QHBoxLayout(myAutoRespBox);

  myAutoRespEdit = new MLEdit(true);
  myAutoRespEdit->setSizeHintLines(5);
  myAutoRespLayout->addWidget(myAutoRespEdit);

  QVBoxLayout* autoRespButtons = new QVBoxLayout();

  myAutoRespHintsButton = new QPushButton(tr("Hints"));
  connect(myAutoRespHintsButton, SIGNAL(clicked()), SLOT(showAutoRespHints()));
  autoRespButtons->addWidget(myAutoRespHintsButton);

  myAutoRespClearButton = new QPushButton(tr("Clear"));
  connect(myAutoRespClearButton, SIGNAL(clicked()), myAutoRespEdit, SLOT(clear()));
  autoRespButtons->addWidget(myAutoRespClearButton);

  autoRespButtons->addStretch(1);
  myAutoRespLayout->addLayout(autoRespButtons);


  myPageStatusLayout->addWidget(myStatusBox, 0, 0);
  myPageStatusLayout->addWidget(mySysGroupBox, 0, 1);
  myPageStatusLayout->addWidget(myAutoRespBox, 1, 0, 1, 2);
  myPageStatusLayout->setRowStretch(2, 1);

  return w;
}

QWidget* UserPages::Settings::createPageOnEvent(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageOnEventLayout = new QVBoxLayout(w);
  myPageOnEventLayout->setContentsMargins(0, 0, 0, 0);

  myOnEventBox = new OnEventBox(false);

  myPageOnEventLayout->addWidget(myOnEventBox);
  myPageOnEventLayout->addStretch(1);

  return w;
}

QWidget* UserPages::Settings::createPageGroups(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageGroupsLayout = new QVBoxLayout(w);
  myPageGroupsLayout->setContentsMargins(0, 0, 0, 0);

  myGroupsBox = new QGroupBox(tr("Groups"));
  myGroupsLayout = new QVBoxLayout(myGroupsBox);

  myGroupsTable = new QTableWidget(0, 3);
  myGroupsTable->setShowGrid(false);
  myGroupsTable->setSelectionMode(QTableWidget::NoSelection);
  myGroupsTable->setEditTriggers(QTableWidget::NoEditTriggers);
  myGroupsLayout->addWidget(myGroupsTable);

  QStringList headerLabels;
  headerLabels << tr("Group") << tr("Local") << tr("Server");
  myGroupsTable->setHorizontalHeaderLabels(headerLabels);
  myGroupsTable->verticalHeader()->hide();

  myPageGroupsLayout->addWidget(myGroupsBox);

  return w;
}

void UserPages::Settings::load(const Licq::User* user)
{
  if (myIsOwner)
    return;

  myAcceptInAwayCheck->setChecked(user->AcceptInAway());
  myAcceptInNaCheck->setChecked(user->AcceptInNA());
  myAcceptInOccupiedCheck->setChecked(user->AcceptInOccupied());
  myAcceptInDndCheck->setChecked(user->AcceptInDND());
  myAutoAcceptFileCheck->setChecked(user->AutoFileAccept());
  myAutoAcceptChatCheck->setChecked(user->AutoChatAccept());
  myAutoSecureCheck->setChecked(user->AutoSecure());
  myUseGpgCheck->setChecked(user->UseGPG());
  myUseRealIpCheck->setChecked(user->SendRealIp());

  unsigned statusToUser = user->statusToUser();
  myStatusNoneRadio->setChecked(statusToUser == User::OfflineStatus);
  myStatusOnlineRadio->setChecked(statusToUser == User::OnlineStatus);
  myStatusAwayRadio->setChecked(statusToUser & User::AwayStatus);
  myStatusNaRadio->setChecked(statusToUser & User::NotAvailableStatus);
  myStatusOccupiedRadio->setChecked(statusToUser & User::OccupiedStatus);
  myStatusDndRadio->setChecked(statusToUser & User::DoNotDisturbStatus);

  myOnlineNotifyCheck->setChecked(user->OnlineNotify());
  myVisibleListCheck->setChecked(user->VisibleList());
  myInvisibleListCheck->setChecked(user->InvisibleList());
  myIgnoreListCheck->setChecked(user->IgnoreList());
  myNewUserCheck->setChecked(user->NewUser());

  unsigned int ppid = user->protocolId();
  bool isIcq = (ppid == LICQ_PPID);
  myUseRealIpCheck->setEnabled(isIcq);
  myStatusNoneRadio->setEnabled(isIcq);
  myStatusOnlineRadio->setEnabled(isIcq);
  myStatusAwayRadio->setEnabled(isIcq);
  myStatusNaRadio->setEnabled(isIcq);
  myStatusOccupiedRadio->setEnabled(isIcq);
  myStatusDndRadio->setEnabled(isIcq);

  unsigned long sendFuncs = 0;
  Licq::ProtocolPlugin::Ptr protocol = Licq::gPluginManager.getProtocolPlugin(ppid);
  if (protocol.get() != NULL)
    sendFuncs = protocol->getSendFunctions();

  myAutoAcceptFileCheck->setEnabled(sendFuncs & Licq::ProtocolPlugin::CanSendFile);
  myAutoAcceptChatCheck->setEnabled(sendFuncs & Licq::ProtocolPlugin::CanSendChat);
  myAutoSecureCheck->setEnabled(Licq::gDaemon.haveCryptoSupport() && (sendFuncs & Licq::ProtocolPlugin::CanSendSecure));

  myGroupsTable->clearContents();
  myGroupsTable->setRowCount(0);
  int serverGroup = (user->GetSID() ? Licq::gUserManager.GetGroupFromID(user->GetGSID()) : 0);
  int i = 0;
  Licq::GroupListGuard groups;
  BOOST_FOREACH(const Licq::Group* group, **groups)
  {
    Licq::GroupReadGuard pGroup(group);
    QString name = QString::fromLocal8Bit(pGroup->name().c_str());
    int gid = pGroup->id();

    myGroupsTable->setRowCount(i+1);

    QTableWidgetItem* nameItem = new QTableWidgetItem(name);
    nameItem->setData(Qt::UserRole, gid);
    myGroupsTable->setItem(i, 0, nameItem);

    QCheckBox* localCheck = new QCheckBox("");
    myGroupsTable->setCellWidget(i, 1, localCheck);

    QRadioButton* serverRadio = new QRadioButton("");
    myGroupsTable->setCellWidget(i, 2, serverRadio);

    // User must be member of group locally if member of the serve group
    // Disable the local checkbox for the current server group and make sure
    // the local group is checked when selecting a server group. This works
    // since clicked() isn't called when radio button looses checked state
    connect(serverRadio, SIGNAL(toggled(bool)), localCheck, SLOT(setDisabled(bool)));
    connect(serverRadio, SIGNAL(clicked(bool)), localCheck, SLOT(setChecked(bool)));

    localCheck->setChecked(user->isInGroup(gid));
    serverRadio->setChecked(gid == serverGroup);

    ++i;
  }

  myGroupsTable->resizeRowsToContents();
  myGroupsTable->resizeColumnsToContents();

  // Get onevents data for user
  Licq::OnEventData* effectiveData = Licq::gOnEventManager.getEffectiveUser(user);
  const Licq::OnEventData* userData = Licq::gOnEventManager.lockUser(user->id());
  myOnEventBox->load(effectiveData, userData);
  Licq::gOnEventManager.unlock(userData);
  Licq::gOnEventManager.dropEffective(effectiveData);
}

void UserPages::Settings::apply(Licq::User* user)
{
  if (myIsOwner)
    return;

  // Set misc modes
  user->SetAcceptInAway(myAcceptInAwayCheck->isChecked());
  user->SetAcceptInNA(myAcceptInNaCheck->isChecked());
  user->SetAcceptInOccupied(myAcceptInOccupiedCheck->isChecked());
  user->SetAcceptInDND(myAcceptInDndCheck->isChecked());
  user->SetAutoFileAccept(myAutoAcceptFileCheck->isChecked());
  user->SetAutoChatAccept(myAutoAcceptChatCheck->isChecked());
  user->SetAutoSecure(myAutoSecureCheck->isChecked());
  user->SetUseGPG(myUseGpgCheck->isChecked());
  user->SetSendRealIp(myUseRealIpCheck->isChecked());

  // System groups wich doesn't require server update
  user->SetOnlineNotify(myOnlineNotifyCheck->isChecked());
  user->SetNewUser(myNewUserCheck->isChecked());

  // Set status to user
  unsigned statusToUser = User::OfflineStatus;
  if (myStatusOnlineRadio->isChecked())
    statusToUser = User::OnlineStatus;
  if (myStatusAwayRadio->isChecked())
    statusToUser = User::AwayStatus | User::OnlineStatus;
  if (myStatusNaRadio->isChecked())
    statusToUser = User::NotAvailableStatus | User::OnlineStatus;
  if (myStatusOccupiedRadio->isChecked())
    statusToUser = User::OccupiedStatus | User::OnlineStatus;
  if (myStatusDndRadio->isChecked())
    statusToUser = User::DoNotDisturbStatus | User::OnlineStatus;
  user->setStatusToUser(statusToUser);

  // Set auto response (empty string will disable custom auto response)
  user->setCustomAutoResponse(myAutoRespEdit->toPlainText().trimmed().toLocal8Bit().data());

  // Save onevent settings
  Licq::OnEventData* userData = Licq::gOnEventManager.lockUser(user->id(), true);
  myOnEventBox->apply(userData);
  Licq::gOnEventManager.unlock(userData, true);
}

void UserPages::Settings::apply2(const Licq::UserId& userId)
{
  if (myIsOwner)
    return;

  int serverGroup = 0;
  Licq::UserGroupList userGroups;
  bool visibleList;
  bool invisibleList;
  bool ignoreList;
  {
    Licq::UserReadGuard u(userId);
    if (!u.isLocked())
      return;

   // Get current group memberships so we only set those that have actually changed
    if (u->GetSID() != 0)
      serverGroup = Licq::gUserManager.GetGroupFromID(u->GetGSID());
    userGroups = u->GetGroups();
    visibleList = u->VisibleList();
    invisibleList = u->InvisibleList();
    ignoreList = u->IgnoreList();
  }

  // First set server group
  for (int i = 0; i < myGroupsTable->rowCount(); ++i)
  {
    int gid = myGroupsTable->item(i, 0)->data(Qt::UserRole).toInt();

    if (dynamic_cast<QRadioButton*>(myGroupsTable->cellWidget(i, 2))->isChecked())
    {
      if (gid != serverGroup)
        Licq::gUserManager.setUserInGroup(userId, gid, true, true);
    }
  }

  // Set local user groups
  for (int i = 0; i < myGroupsTable->rowCount(); ++i)
  {
    int gid = myGroupsTable->item(i, 0)->data(Qt::UserRole).toInt();

    bool inLocal = dynamic_cast<QCheckBox*>(myGroupsTable->cellWidget(i, 1))->isChecked();
    if ((userGroups.count(gid) > 0) != inLocal)
      Licq::gUserManager.setUserInGroup(userId, gid, inLocal, false);
  }

  // Set system groups
  if (myVisibleListCheck->isChecked() != visibleList)
    gProtocolManager.visibleListSet(userId, myVisibleListCheck->isChecked());
  if (myInvisibleListCheck->isChecked() != invisibleList)
    gProtocolManager.invisibleListSet(userId, myInvisibleListCheck->isChecked());
  if (myIgnoreListCheck->isChecked() != ignoreList)
    gProtocolManager.ignoreListSet(userId, myIgnoreListCheck->isChecked());
}

void UserPages::Settings::userUpdated(const Licq::User* user, unsigned long subSignal)
{
  switch (subSignal)
  {
    case Licq::PluginSignal::UserGroups:
    case Licq::PluginSignal::UserSettings:
      load(user);
      break;
  }
}

void UserPages::Settings::showAutoRespHints()
{
  AwayMsgDlg::showAutoResponseHints(dynamic_cast<UserDlg*>(parent()));
}
