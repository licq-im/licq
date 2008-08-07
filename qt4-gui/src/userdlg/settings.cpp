// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2008 Licq developers
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

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QRadioButton>
#include <QStringList>
#include <QTableWidget>
#include <QVBoxLayout>

#include <licq_icqd.h>
#include <licq_user.h>

#include "dialogs/awaymsgdlg.h"
#include "widgets/mledit.h"

#include "userdlg.h"


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

#ifdef HAVE_LIBGPGME
  myUseGpgCheck = new QCheckBox(tr("Use GPG encryption"));
  myUseGpgCheck->setToolTip(tr("Use GPG encryption for messages with this contact."));
  mySettingsLayout->addWidget(myUseGpgCheck, 3, 1);
#endif

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

  mySystemGroupCheck[GROUP_ONLINE_NOTIFY] = new QCheckBox(tr("Online notify"));
  mySystemGroupCheck[GROUP_ONLINE_NOTIFY]->setToolTip(tr("Notify when this contact comes online."));
  mySysGroupLayout->addWidget(mySystemGroupCheck[GROUP_ONLINE_NOTIFY]);

  mySystemGroupCheck[GROUP_VISIBLE_LIST] = new QCheckBox(tr("Visible List"));
  mySystemGroupCheck[GROUP_VISIBLE_LIST]->setToolTip(tr("Contact will see you online even if you're invisible."));
  mySysGroupLayout->addWidget(mySystemGroupCheck[GROUP_VISIBLE_LIST]);

  mySystemGroupCheck[GROUP_INVISIBLE_LIST] = new QCheckBox(tr("Invisible List"));
  mySystemGroupCheck[GROUP_INVISIBLE_LIST]->setToolTip(tr("Contact will always see you as offline."));
  mySysGroupLayout->addWidget(mySystemGroupCheck[GROUP_INVISIBLE_LIST]);

  mySystemGroupCheck[GROUP_IGNORE_LIST] = new QCheckBox(tr("Ignore List"));
  mySystemGroupCheck[GROUP_IGNORE_LIST]->setToolTip(tr("Ignore any events from this contact."));
  mySysGroupLayout->addWidget(mySystemGroupCheck[GROUP_IGNORE_LIST]);

  mySystemGroupCheck[GROUP_NEW_USERS] = new QCheckBox(tr("New Users"));
  mySystemGroupCheck[GROUP_NEW_USERS]->setToolTip(tr("Contact was recently added to the list."));
  mySysGroupLayout->addWidget(mySystemGroupCheck[GROUP_NEW_USERS]);

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

void UserPages::Settings::load(const ICQUser* user)
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
#ifdef HAVE_LIBGPGME
  myUseGpgCheck->setChecked(user->UseGPG());
#endif
  myUseRealIpCheck->setChecked(user->SendRealIp());

  unsigned short statusToUser = user->StatusToUser();
  myStatusNoneRadio->setChecked(statusToUser == ICQ_STATUS_OFFLINE);
  myStatusOnlineRadio->setChecked(statusToUser == ICQ_STATUS_ONLINE);
  myStatusAwayRadio->setChecked(statusToUser == ICQ_STATUS_AWAY);
  myStatusNaRadio->setChecked(statusToUser == ICQ_STATUS_NA);
  myStatusOccupiedRadio->setChecked(statusToUser == ICQ_STATUS_OCCUPIED);
  myStatusDndRadio->setChecked(statusToUser == ICQ_STATUS_DND);

  for (unsigned short i = 1; i < NUM_GROUPS_SYSTEM_ALL; ++i)
    mySystemGroupCheck[i]->setChecked(user->GetInGroup(GROUPS_SYSTEM, i));

  unsigned int ppid = user->PPID();
  bool isIcq = (ppid == LICQ_PPID);
  myUseRealIpCheck->setEnabled(isIcq);
  myStatusNoneRadio->setEnabled(isIcq);
  myStatusOnlineRadio->setEnabled(isIcq);
  myStatusAwayRadio->setEnabled(isIcq);
  myStatusNaRadio->setEnabled(isIcq);
  myStatusOccupiedRadio->setEnabled(isIcq);
  myStatusDndRadio->setEnabled(isIcq);

  unsigned long sendFuncs = 0xFFFFFFFF;
  if (!isIcq)
  {
    FOR_EACH_PROTO_PLUGIN_START(gLicqDaemon)
    {
      if ((*_ppit)->PPID() == ppid)
      {
        sendFuncs = (*_ppit)->SendFunctions();
        break;
      }
    }
    FOR_EACH_PROTO_PLUGIN_END
  }

  myAutoAcceptFileCheck->setEnabled(sendFuncs & PP_SEND_FILE);
  myAutoAcceptChatCheck->setEnabled(sendFuncs & PP_SEND_CHAT);
  myAutoSecureCheck->setEnabled(gLicqDaemon->CryptoEnabled() && (sendFuncs & PP_SEND_SECURE));

  myGroupsTable->clearContents();
  myGroupsTable->setRowCount(0);
  unsigned short serverGroup = (user->GetSID() ? gUserManager.GetGroupFromID(user->GetGSID()) : 0);
  int i = 0;
  FOR_EACH_GROUP_START_SORTED(LOCK_R)
  {
    QString name = QString::fromLocal8Bit(pGroup->name().c_str());
    unsigned short gid = pGroup->id();

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

    localCheck->setChecked(user->GetInGroup(GROUPS_USER, gid));
    serverRadio->setChecked(gid == serverGroup);

    ++i;
  }
  FOR_EACH_GROUP_END

  myGroupsTable->resizeRowsToContents();
  myGroupsTable->resizeColumnsToContents();
}

void UserPages::Settings::apply(ICQUser* user)
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
#ifdef HAVE_LIBGPGME
  user->SetUseGPG(myUseGpgCheck->isChecked());
#endif
  user->SetSendRealIp(myUseRealIpCheck->isChecked());

  // Set status to user
  unsigned short statusToUser = ICQ_STATUS_OFFLINE;
  if (myStatusOnlineRadio->isChecked())
    statusToUser = ICQ_STATUS_ONLINE;
  if (myStatusAwayRadio->isChecked())
    statusToUser = ICQ_STATUS_AWAY;
  if (myStatusNaRadio->isChecked())
    statusToUser = ICQ_STATUS_NA;
  if (myStatusOccupiedRadio->isChecked())
    statusToUser = ICQ_STATUS_OCCUPIED;
  if (myStatusDndRadio->isChecked())
    statusToUser = ICQ_STATUS_DND;
  user->SetStatusToUser(statusToUser);

  // Set auto response (empty string will disable custom auto response)
  user->SetCustomAutoResponse(myAutoRespEdit->toPlainText().trimmed().toLocal8Bit());
}

void UserPages::Settings::apply2(const QString& id, unsigned long ppid)
{
  if (myIsOwner)
    return;

  const ICQUser* u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);
  if (u == NULL)
    return;

  // Get current group memberships so we only set those that have actually changed
  int serverGroup = 0;
  if (u->GetSID() != 0)
    serverGroup = gUserManager.GetGroupFromID(u->GetGSID());
  const UserGroupList& userGroups = u->GetGroups();
  unsigned long systemGroups = u->GetSystemGroups();

  gUserManager.DropUser(u);

  // First set server group
  for (int i = 0; i < myGroupsTable->rowCount(); ++i)
  {
    unsigned short gid = myGroupsTable->item(i, 0)->data(Qt::UserRole).toUInt();

    if (dynamic_cast<QRadioButton*>(myGroupsTable->cellWidget(i, 2))->isChecked())
    {
      if (gid != serverGroup)
        gUserManager.SetUserInGroup(id.toLatin1().data(), ppid, GROUPS_USER, gid, true, true);
    }
  }

  // Set local user groups
  for (int i = 0; i < myGroupsTable->rowCount(); ++i)
  {
    unsigned short gid = myGroupsTable->item(i, 0)->data(Qt::UserRole).toUInt();

    bool inLocal = dynamic_cast<QCheckBox*>(myGroupsTable->cellWidget(i, 1))->isChecked();
    if ((userGroups.count(gid) > 0) != inLocal)
      gUserManager.SetUserInGroup(id.toLatin1().data(), ppid, GROUPS_USER, gid, inLocal, false);
  }

  // Set system groups
  for (unsigned short i = 1; i < NUM_GROUPS_SYSTEM_ALL; ++i)
  {
    bool inGroup = mySystemGroupCheck[i]->isChecked();
    if (((systemGroups & (1L << (i - 1))) != 0) != inGroup)
      gUserManager.SetUserInGroup(id.toLatin1().data(), ppid, GROUPS_SYSTEM, i, inGroup, true);
  }
}

void UserPages::Settings::userUpdated(const CICQSignal* sig, const ICQUser* user)
{
  switch (sig->SubSignal())
  {
    case USER_GENERAL:
      load(user);
      break;
  }
}

void UserPages::Settings::showAutoRespHints()
{
  AwayMsgDlg::showAutoResponseHints(dynamic_cast<UserDlg*>(parent()));
}
