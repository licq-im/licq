// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2011 Licq developers
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

#include "usermenu.h"

#include "config.h"

#include <boost/foreach.hpp>

#include <QActionGroup>
#include <QClipboard>

#include <licq/contactlist/group.h>
#include <licq/contactlist/usermanager.h>
#include <licq/icq.h>
#include <licq/daemon.h>
#include <licq/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/utility.h>

#include "config/iconmanager.h"
#include "config/shortcuts.h"

#include "contactlist/contactlist.h"

#include "dialogs/authuserdlg.h"
#include "dialogs/awaymsgdlg.h"
#include "dialogs/customautorespdlg.h"
#include "dialogs/gpgkeyselect.h"
#include "dialogs/historydlg.h"
#include "dialogs/keyrequestdlg.h"
#include "dialogs/reqauthdlg.h"
#include "dialogs/showawaymsgdlg.h"
#include "dialogs/utilitydlg.h"

#include "gui-defines.h"
#include "licqgui.h"
#include "messagebox.h"
#include "signalmanager.h"

using Licq::User;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserMenu */

UserMenu* LicqQtGui::gUserMenu = NULL;

UserMenu::UserMenu(QWidget* parent)
  : QMenu(parent)
{
  assert(gUserMenu == NULL);
  gUserMenu = this;

  QAction* a;

  // Sub menu Send
  mySendMenu = new QMenu(tr("Send"));
  connect(mySendMenu, SIGNAL(triggered(QAction*)), SLOT(send(QAction*)));
#define ADD_SEND(text, data) \
    a = mySendMenu->addAction(text); \
    mySendActions.insert(data, a); \
    a->setData(data);
  ADD_SEND(tr("Send &Message..."), SendMessage)
  ADD_SEND(tr("Send &URL..."), SendUrl)
  ADD_SEND(tr("Send &Chat Request..."), SendChat)
  ADD_SEND(tr("Send &File Transfer..."), SendFile)
  ADD_SEND(tr("Send Contact &List..."), SendContact)
  ADD_SEND(tr("Send &Authorization..."), SendAuthorize)
  ADD_SEND(tr("Send Authorization Re&quest..."), SendReqAuthorize)
  ADD_SEND(tr("Send &SMS..."), SendSms)
  mySendMenu->addSeparator();
  ADD_SEND(tr("Update Info Plugin List"), RequestUpdateInfoPlugin)
  ADD_SEND(tr("Update Status Plugin List"), RequestUpdateStatusPlugin)
  ADD_SEND(tr("Update Phone \"Follow Me\" Status"), RequestPhoneFollowMeStatus)
  ADD_SEND(tr("Update ICQphone Status"), RequestIcqphoneStatus)
  ADD_SEND(tr("Update File Server Status"), RequestFileServerStatus)
  mySendMenu->addSeparator();
  ADD_SEND(QString::null, SendKey)
#undef ADD_SEND

  // Sub menu Misc Modes
  myMiscModesMenu = new QMenu(tr("Misc Modes"));
  connect(myMiscModesMenu, SIGNAL(triggered(QAction*)), SLOT(toggleMiscMode(QAction*)));
#define ADD_MISCMODE(text, data) \
    a = myMiscModesMenu->addAction(text); \
    myMiscModesActions.insert(data, a);\
    a->setCheckable(true); \
    a->setData(data);
  ADD_MISCMODE(tr("Accept in Away"), ModeAcceptInAway)
  ADD_MISCMODE(tr("Accept in Not Available"), ModeAcceptInNa)
  ADD_MISCMODE(tr("Accept in Occupied"), ModeAcceptInOccupied)
  ADD_MISCMODE(tr("Accept in Do Not Disturb"), ModeAcceptInDnd)
  ADD_MISCMODE(tr("Auto Accept Files"), ModeAutoFileAccept)
  ADD_MISCMODE(tr("Auto Accept Chats"), ModeAutoChatAccept)
  ADD_MISCMODE(tr("Auto Request Secure"), ModeAutoSecure)
  ADD_MISCMODE(tr("Use GPG Encryption"), ModeUseGpg)
  if (!Licq::gDaemon.haveGpgSupport())
    a->setVisible(false);
  ADD_MISCMODE(tr("Use Real Ip (LAN)"), ModeUseRealIp)
  myMiscModesMenu->addSeparator();
  ADD_MISCMODE(tr("Online to User"), ModeStatusOnline)
  ADD_MISCMODE(tr("Away to User"), ModeStatusAway)
  ADD_MISCMODE(tr("Not Available to User"), ModeStatusNa)
  ADD_MISCMODE(tr("Occupied to User"), ModeStatusOccupied)
  ADD_MISCMODE(tr("Do Not Disturb to User"), ModeStatusDnd)
#undef ADD_MISCMODE

  // Sub menu Utilities
  myUtilitiesMenu = new QMenu(tr("U&tilities"));
  connect(myUtilitiesMenu, SIGNAL(triggered(QAction*)), SLOT(utility(QAction*)));
  for (int i = 0; i < Licq::gUtilityManager.numUtilities(); ++i)
  {
    myUtilitiesMenu->addAction(Licq::gUtilityManager.utility(i)->name().c_str())->setData(i);
  }

  // Sub menu User Group
  myGroupsMenu = new QMenu(tr("Edit User Group"));
  myUserGroupActions = new QActionGroup(this);
  myUserGroupActions->setExclusive(false);
  connect(myUserGroupActions, SIGNAL(triggered(QAction*)), SLOT(toggleUserGroup(QAction*)));
  mySystemGroupActions = new QActionGroup(this);
  mySystemGroupActions->setExclusive(false);
  connect(mySystemGroupActions, SIGNAL(triggered(QAction*)), SLOT(toggleSystemGroup(QAction*)));
  myServerGroupActions = new QActionGroup(this);
  connect(myServerGroupActions, SIGNAL(triggered(QAction*)), SLOT(setServerGroup(QAction*)));

  // System groups
#define ADD_SYSTEMGROUP(gid, data) \
    a = mySystemGroupActions->addAction(ContactListModel::systemGroupName(gid)); \
    myMiscModesActions.insert(data, a);\
    a->setCheckable(true); \
    a->setData(gid);
  ADD_SYSTEMGROUP(ContactListModel::OnlineNotifyGroupId, ModeOnlineNotify);
  ADD_SYSTEMGROUP(ContactListModel::VisibleListGroupId, ModeVisibleList);
  ADD_SYSTEMGROUP(ContactListModel::InvisibleListGroupId, ModeInvisibleList);
  ADD_SYSTEMGROUP(ContactListModel::IgnoreListGroupId, ModeIgnoreList);
  ADD_SYSTEMGROUP(ContactListModel::NewUsersGroupId, ModeNewUser);
#undef ADD_SYSTEMGROUP

  myServerGroupsMenu = new QMenu(tr("Server Group"));
  myGroupsMenu->addMenu(myServerGroupsMenu);
  myGroupsMenu->addSeparator();
  myGroupSeparator = myGroupsMenu->addSeparator();
  myGroupsMenu->addActions(mySystemGroupActions->actions());

  // User menu
  myViewEventAction = addAction(tr("&View Event..."), this, SLOT(viewEvent()));
  addMenu(mySendMenu);
  addMenu(myMiscModesMenu);
  addMenu(myUtilitiesMenu);
  //myCheckInvisibleAction = addAction(tr("Check If Invisible"), this, SLOT(checkInvisible()));
  myCheckArAction = addAction(QString::null, this, SLOT(checkAutoResponse()));
  myCustomArAction = addAction(tr("Custom Auto Response..."), this, SLOT(customAutoResponse()));
  myCustomArAction->setCheckable(true);
  addSeparator();
  myMakePermanentAction = addAction(tr("Add to List"), this, SLOT(makePermanent()));
  myToggleFloatyAction = addAction(tr("Toggle &Floaty"), this, SLOT(toggleFloaty()));
  addMenu(myGroupsMenu);
  myRemoveUserAction = addAction(tr("Remove From List"), this, SLOT(removeContact()));
  addSeparator();
  mySetKeyAction = addAction(tr("Set GPG key..."), this, SLOT(selectKey()));
  if (!Licq::gDaemon.haveGpgSupport())
    mySetKeyAction->setVisible(false);
  myCopyIdAction = addAction(tr("&Copy User ID"), this, SLOT(copyIdToClipboard()));
  myViewHistoryAction = addAction(tr("View &History..."), this, SLOT(viewHistory()));
  myViewGeneralAction = addAction(tr("&Info..."), this, SLOT(viewInfoGeneral()));

  connect(this, SIGNAL(aboutToShow()), SLOT(aboutToShowMenu()));

  updateIcons();
  updateGroups();

  connect(IconManager::instance(), SIGNAL(iconsChanged()), SLOT(updateIcons()));
}

UserMenu::~UserMenu()
{
  gUserMenu = NULL;
}

void UserMenu::updateIcons()
{
  IconManager* iconman = IconManager::instance();

  mySendActions[SendMessage]->setIcon(iconman->getIcon(IconManager::StandardMessageIcon));
  mySendActions[SendUrl]->setIcon(iconman->getIcon(IconManager::UrlMessageIcon));
  mySendActions[SendChat]->setIcon(iconman->getIcon(IconManager::ChatMessageIcon));
  mySendActions[SendFile]->setIcon(iconman->getIcon(IconManager::FileMessageIcon));
  mySendActions[SendContact]->setIcon(iconman->getIcon(IconManager::ContactMessageIcon));
  mySendActions[SendAuthorize]->setIcon(iconman->getIcon(IconManager::AuthorizeMessageIcon));
  mySendActions[SendReqAuthorize]->setIcon(iconman->getIcon(IconManager::ReqAuthorizeMessageIcon));
  mySendActions[SendSms]->setIcon(iconman->getIcon(IconManager::SmsMessageIcon));

  myCustomArAction->setIcon(iconman->getIcon(IconManager::CustomArIcon));
  myRemoveUserAction->setIcon(iconman->getIcon(IconManager::RemoveIcon));
  mySetKeyAction->setIcon(iconman->getIcon(IconManager::GpgKeyIcon));
  myViewHistoryAction->setIcon(iconman->getIcon(IconManager::HistoryIcon));
  myViewGeneralAction->setIcon(iconman->getIcon(IconManager::InfoIcon));
}

void UserMenu::updateGroups()
{
  QAction* a;

  // Clear old groups but leave system groups as they never change
  foreach (a, myUserGroupActions->actions())
    delete a;
  foreach (a, myServerGroupActions->actions())
    delete a;

  Licq::GroupListGuard groupList;
  BOOST_FOREACH(const Licq::Group* group, **groupList)
  {
    Licq::GroupReadGuard pGroup(group);

    QString name = QString::fromLocal8Bit(pGroup->name().c_str());
    name.replace("&", "&&");

    a = myUserGroupActions->addAction(name);
    a->setData(pGroup->id());
    a->setCheckable(true);

    a = myServerGroupActions->addAction(name);
    a->setData(pGroup->id());
    a->setCheckable(true);
  }

  // Add groups to menu
  myGroupsMenu->insertActions(myGroupSeparator, myUserGroupActions->actions());
  myServerGroupsMenu->addActions(myServerGroupActions->actions());
}

void UserMenu::aboutToShowMenu()
{
  Licq::UserReadGuard u(myUserId);

  unsigned status = (!u.isLocked() ? static_cast<unsigned>(User::OfflineStatus) : u->status());

  myCheckArAction->setEnabled(status != User::OfflineStatus);
  myMakePermanentAction->setVisible(!u.isLocked() ? false : u->NotInList());

  if (status & User::MessageStatuses)
    myCheckArAction->setText(tr("Check %1 Response...")
        .arg(u->statusString(true, false).c_str()));
  else
    myCheckArAction->setText(tr("Check Auto Response..."));

  if (!u.isLocked())
    return;

  myMiscModesActions[ModeAcceptInAway]->setChecked(u->AcceptInAway());
  myMiscModesActions[ModeAcceptInNa]->setChecked(u->AcceptInNA());
  myMiscModesActions[ModeAcceptInOccupied]->setChecked(u->AcceptInOccupied());
  myMiscModesActions[ModeAcceptInDnd]->setChecked(u->AcceptInDND());
  myMiscModesActions[ModeAutoFileAccept]->setChecked(u->AutoFileAccept());
  myMiscModesActions[ModeAutoChatAccept]->setChecked(u->AutoChatAccept());
  myMiscModesActions[ModeAutoSecure]->setChecked(u->AutoSecure());
  myMiscModesActions[ModeAutoSecure]->setEnabled(Licq::gDaemon.haveCryptoSupport());
  myMiscModesActions[ModeUseGpg]->setChecked(u->UseGPG());
  myMiscModesActions[ModeUseRealIp]->setChecked(u->SendRealIp());
  myMiscModesActions[ModeStatusOnline]->setChecked(u->statusToUser() == User::OnlineStatus);
  myMiscModesActions[ModeStatusAway]->setChecked(u->statusToUser() & User::AwayStatus);
  myMiscModesActions[ModeStatusNa]->setChecked(u->statusToUser() & User::NotAvailableStatus);
  myMiscModesActions[ModeStatusOccupied]->setChecked(u->statusToUser() & User::OccupiedStatus);
  myMiscModesActions[ModeStatusDnd]->setChecked(u->statusToUser() & User::DoNotDisturbStatus);

  myCustomArAction->setChecked(!u->customAutoResponse().empty());

  mySendActions[SendChat]->setEnabled(u->isOnline());
  mySendActions[SendFile]->setEnabled(u->isOnline());
  mySendActions[SendSms]->setEnabled(!u->getCellularNumber().empty());
  if (u->Secure())
  {
    mySendActions[SendKey]->setText(tr("Close &Secure Channel..."));
    mySendActions[SendKey]->setIcon(IconManager::instance()->getIcon(IconManager::SecureOnIcon));
  }
  else
  {
    mySendActions[SendKey]->setText(tr("Request &Secure Channel..."));
    mySendActions[SendKey]->setIcon(IconManager::instance()->getIcon(IconManager::SecureOffIcon));
  }

  unsigned long sendFuncs = 0;
  bool isIcq = myPpid == LICQ_PPID;

  Licq::ProtocolPlugin::Ptr protocol = Licq::gPluginManager.getProtocolPlugin(myPpid);
  if (protocol.get() != NULL)
    sendFuncs = protocol->getSendFunctions();

  mySendActions[SendMessage]->setVisible(sendFuncs & Licq::ProtocolPlugin::CanSendMsg);
  mySendActions[SendUrl]->setVisible(sendFuncs & Licq::ProtocolPlugin::CanSendUrl);
  mySendActions[SendChat]->setVisible(sendFuncs & Licq::ProtocolPlugin::CanSendChat);
  mySendActions[SendFile]->setVisible(sendFuncs & Licq::ProtocolPlugin::CanSendFile);
  mySendActions[SendContact]->setVisible(sendFuncs & Licq::ProtocolPlugin::CanSendContact);
  mySendActions[SendAuthorize]->setVisible(sendFuncs & Licq::ProtocolPlugin::CanSendAuth);
  mySendActions[SendReqAuthorize]->setVisible(sendFuncs & Licq::ProtocolPlugin::CanSendAuthReq);
  mySendActions[SendSms]->setVisible(sendFuncs & Licq::ProtocolPlugin::CanSendSms);
  mySendActions[SendKey]->setVisible(sendFuncs & Licq::ProtocolPlugin::CanSendSecure);
  myMiscModesActions[ModeAutoFileAccept]->setVisible(sendFuncs & Licq::ProtocolPlugin::CanSendFile);
  myMiscModesActions[ModeAutoChatAccept]->setVisible(sendFuncs & Licq::ProtocolPlugin::CanSendChat);
  myMiscModesActions[ModeAutoSecure]->setVisible(sendFuncs & Licq::ProtocolPlugin::CanSendSecure);
  myMiscModesActions[ModeOnlineNotify]->setChecked(u->OnlineNotify());
  myMiscModesActions[ModeVisibleList]->setChecked(u->VisibleList());
  myMiscModesActions[ModeInvisibleList]->setChecked(u->InvisibleList());
  myMiscModesActions[ModeIgnoreList]->setChecked(u->IgnoreList());
  myMiscModesActions[ModeNewUser]->setChecked(u->NewUser());

  // ICQ Protocol only
  mySendActions[RequestUpdateInfoPlugin]->setVisible(isIcq);
  mySendActions[RequestUpdateStatusPlugin]->setVisible(isIcq);
  mySendActions[RequestPhoneFollowMeStatus]->setVisible(isIcq);
  mySendActions[RequestIcqphoneStatus]->setVisible(isIcq);
  mySendActions[RequestFileServerStatus]->setVisible(isIcq);
//  myCheckInvisibleAction->setVisible(isIcq);
  myCheckArAction->setVisible(isIcq);
  myCustomArAction->setVisible(isIcq);
  myMiscModesActions[ModeUseRealIp]->setVisible(isIcq);
  myMiscModesActions[ModeStatusOnline]->setVisible(isIcq);
  myMiscModesActions[ModeStatusAway]->setVisible(isIcq);
  myMiscModesActions[ModeStatusNa]->setVisible(isIcq);
  myMiscModesActions[ModeStatusOccupied]->setVisible(isIcq);
  myMiscModesActions[ModeStatusDnd]->setVisible(isIcq);

  if (myShowShortcuts)
  {
    // Display shortcuts
    // Note: These shortcuts are only for display, the actual shortcuts for
    //       the main contact list are handled in mainwin.
    Config::Shortcuts* shortcuts = Config::Shortcuts::instance();

    myCheckArAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinUserCheckAutoresponse));
    mySendActions[SendChat]->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinUserSendChatRequest));
    mySendActions[SendMessage]->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinUserSendMessage));
    mySendActions[SendFile]->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinUserSendFile));
    mySendActions[SendUrl]->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinUserSendUrl));
    myViewHistoryAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinUserViewHistory));
    myViewEventAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinUserViewMessage));
  }
  else
  {
    // Hide the shortcuts (e.g. menu is displayed from a dialog)
    myCheckArAction->setShortcut(QKeySequence());
    mySendActions[SendChat]->setShortcut(QKeySequence());
    mySendActions[SendMessage]->setShortcut(QKeySequence());
    mySendActions[SendFile]->setShortcut(QKeySequence());
    mySendActions[SendUrl]->setShortcut(QKeySequence());
    myViewHistoryAction->setShortcut(QKeySequence());
    myViewEventAction->setShortcut(QKeySequence());
  }

  int serverGroup = (u->GetSID() ? Licq::gUserManager.GetGroupFromID(u->GetGSID()) : 0);

  // Update group memberships
  foreach (QAction* a, myUserGroupActions->actions())
  {
    int gid = a->data().toInt();
    bool inGroup = u->isInGroup(gid);
    a->setChecked(inGroup);

    // Don't allow leaving group if contact is member of the same group at the server side
    a->setEnabled(!inGroup || gid != serverGroup);
  }
  foreach (QAction* a, myServerGroupActions->actions())
    a->setChecked(a->data().toInt() == serverGroup);
}

void UserMenu::setUser(const Licq::UserId& userId, bool showShortcuts)
{
  myUserId = userId;
  myId = userId.accountId().c_str();
  myPpid = userId.protocolId();
  myShowShortcuts = showShortcuts;
}

void UserMenu::popup(QPoint pos, const Licq::UserId& userId, bool showShortcuts)
{
  setUser(userId, showShortcuts);
  QMenu::popup(pos);
}

void UserMenu::viewEvent()
{
  gLicqGui->showViewEventDialog(myUserId);
}

void UserMenu::checkInvisible()
{
  if (myPpid == LICQ_PPID)
    gLicqDaemon->icqCheckInvisible(myUserId);
}

void UserMenu::checkAutoResponse()
{
  new ShowAwayMsgDlg(myUserId, true);
}

void UserMenu::customAutoResponse()
{
  new CustomAutoRespDlg(myUserId);
}

void UserMenu::makePermanent()
{
  Licq::gUserManager.makeUserPermanent(myUserId);
}

void UserMenu::toggleFloaty()
{
  gLicqGui->toggleFloaty(myUserId);
}

void UserMenu::removeContact()
{
  gLicqGui->removeUserFromList(myUserId);
}

void UserMenu::selectKey()
{
  new GPGKeySelect(myUserId);
}

void UserMenu::copyIdToClipboard()
{
  QClipboard* clip = qApp->clipboard();
  QClipboard::Mode mode = QClipboard::Clipboard;

  if (clip->supportsSelection())
    mode = QClipboard::Selection;

  clip->setText(myId, mode);
}

void UserMenu::viewHistory()
{
  new HistoryDlg(myUserId);
}

void UserMenu::viewInfoGeneral()
{
  gLicqGui->showInfoDialog(mnuUserGeneral, myUserId);
}

void UserMenu::send(QAction* action)
{
  int index = action->data().toInt();

  switch (index)
  {
    case SendAuthorize:
      new AuthUserDlg(myUserId, true);
      break;

    case SendReqAuthorize:
      new ReqAuthDlg(myId, myPpid);
      break;

    case SendKey:
      new KeyRequestDlg(myUserId);
      break;

    case RequestUpdateInfoPlugin:
      if (myPpid == LICQ_PPID)
        gLicqDaemon->icqRequestInfoPluginList(myUserId, true);
      break;

    case RequestUpdateStatusPlugin:
      if (myPpid == LICQ_PPID)
        gLicqDaemon->icqRequestStatusPluginList(myUserId, true);
      break;

    case RequestPhoneFollowMeStatus:
      if (myPpid == LICQ_PPID)
        gLicqDaemon->icqRequestPhoneFollowMe(myUserId, true);
      break;

    case RequestIcqphoneStatus:
      if (myPpid == LICQ_PPID)
        gLicqDaemon->icqRequestICQphone(myUserId, true);
      break;

    case RequestFileServerStatus:
      if (myPpid == LICQ_PPID)
        gLicqDaemon->icqRequestSharedFiles(myUserId, true);
      break;

    default:
      gLicqGui->showEventDialog(index, myUserId);
  }
}

void UserMenu::toggleMiscMode(QAction* action)
{
  int mode = action->data().toInt();
  bool newState = action->isChecked();

  Licq::UserWriteGuard u(myUserId);
  if (!u.isLocked())
    return;

  switch (mode)
  {
    case ModeAcceptInAway:
      u->SetAcceptInAway(newState);
      break;

    case ModeAcceptInNa:
      u->SetAcceptInNA(newState);
      break;

    case ModeAcceptInOccupied:
      u->SetAcceptInOccupied(newState);
      break;

    case ModeAcceptInDnd:
      u->SetAcceptInDND(newState);
      break;

    case ModeAutoFileAccept:
      u->SetAutoFileAccept(newState);
      break;

    case ModeAutoChatAccept:
      u->SetAutoChatAccept(newState);
      break;

    case ModeAutoSecure:
      u->SetAutoSecure(newState);
      break;

    case ModeUseGpg:
    {
      if (!u->gpgKey().empty())
      {
        u->SetUseGPG(newState);
        u.unlock();

        // Notify all plugins (including ourselves)
        Licq::gUserManager.notifyUserUpdated(myUserId, Licq::PluginSignal::UserSecurity);
      }
      else
      {
        u.unlock();
        new GPGKeySelect(myUserId);
      }
      return;
    }

    case ModeUseRealIp:
      u->SetSendRealIp(newState);
      break;

    case ModeStatusOnline:
      u->setStatusToUser(newState ? User::OnlineStatus : User::OfflineStatus);
      break;

    case ModeStatusAway:
      u->setStatusToUser(newState ? User::AwayStatus | User::OnlineStatus : User::OfflineStatus);
      break;

    case ModeStatusNa:
      u->setStatusToUser(newState ? User::NotAvailableStatus | User::OnlineStatus : User::OfflineStatus);
      break;

    case ModeStatusOccupied:
      u->setStatusToUser(newState ? User::OccupiedStatus | User::OnlineStatus : User::OfflineStatus);
      break;

    case ModeStatusDnd:
      u->setStatusToUser(newState ? User::DoNotDisturbStatus | User::OnlineStatus : User::OfflineStatus);
      break;
  }
  u.unlock();

  // Notify all plugins (including ourselves)
  Licq::gUserManager.notifyUserUpdated(myUserId, Licq::PluginSignal::UserSettings);
}

void UserMenu::utility(QAction* action)
{
  int index = action->data().toUInt();

  Licq::Utility* u = Licq::gUtilityManager.utility(index);

  if (u != NULL)
    new UtilityDlg(u, myUserId);
}

void UserMenu::toggleUserGroup(QAction* action)
{
  int gid = action->data().toInt();
  Licq::gUserManager.setUserInGroup(myUserId, gid, action->isChecked(), false);
}

void UserMenu::toggleSystemGroup(QAction* action)
{
  int gid = action->data().toInt();

  if (gid == ContactListModel::IgnoreListGroupId && action->isChecked())
  {
    QString alias;
    {
      Licq::UserReadGuard u(myUserId);
      if (!u.isLocked())
        return;
      alias = u->getAlias().c_str();
    }

    if(!QueryYesNo(this, tr("Do you really want to add\n%1 (%2)\nto your ignore list?")
        .arg(alias).arg(myId)))
      return;
  }

  gLicqGui->setUserInGroup(myUserId, gid, action->isChecked(), true);
}

void UserMenu::setServerGroup(QAction* action)
{
  int gid = action->data().toInt();
  Licq::gUserManager.setUserInGroup(myUserId, gid, true, true);
}
