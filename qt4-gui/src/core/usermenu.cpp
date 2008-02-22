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

#include "usermenu.h"

#include "config.h"

#include <QActionGroup>

#include <licq_events.h>
#include <licq_icqd.h>
#include <licq_user.h>
#include <licq_utility.h>

#include "config/iconmanager.h"

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

#include "helpers/licqstrings.h"

#include "gui-defines.h"
#include "licqgui.h"
#include "messagebox.h"
#include "signalmanager.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserMenu */

UserMenu::UserMenu(QWidget* parent)
  : QMenu(parent)
{
  QAction* a;

  // Sub menu Send
  mySendMenu = new QMenu(tr("Send"));
  connect(mySendMenu, SIGNAL(triggered(QAction*)), SLOT(send(QAction*)));
#define ADD_SEND(text, data) \
    a = mySendMenu->addAction(text); \
    mySendActions.insert(data, a); \
    a->setData(data);
  ADD_SEND(tr("Send &Message"), SendMessage)
  ADD_SEND(tr("Send &URL"), SendUrl)
  ADD_SEND(tr("Send &Chat Request"), SendChat)
  ADD_SEND(tr("Send &File Transfer"), SendFile)
  ADD_SEND(tr("Send Contact &List"), SendContact)
  ADD_SEND(tr("Send &Authorization"), SendAuthorize)
  ADD_SEND(tr("Send Authorization Re&quest"), SendReqAuthorize)
  ADD_SEND(tr("Send &SMS"), SendSms)
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
#ifdef HAVE_LIBGPGME
  ADD_MISCMODE(tr("Use GPG Encryption"), ModeUseGpg)
#endif
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
  for (int i = 0; i < gUtilityManager.NumUtilities(); ++i)
  {
    myUtilitiesMenu->addAction(gUtilityManager.Utility(i)->Name())->setData(i);
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
  for (unsigned int i = 1; i < NUM_GROUPS_SYSTEM_ALL; ++i)
  {
    a = mySystemGroupActions->addAction(LicqStrings::getSystemGroupName(i));
    a->setData(i);
    a->setCheckable(true);
  }

  myServerGroupsMenu = new QMenu(tr("Server Group"));
  myGroupsMenu->addMenu(myServerGroupsMenu);
  myGroupsMenu->addSeparator();
  myGroupSeparator = myGroupsMenu->addSeparator();
  myGroupsMenu->addActions(mySystemGroupActions->actions());

  // User menu
  myViewEventAction = addAction(tr("&View Event"), this, SLOT(viewEvent()));
  addMenu(mySendMenu);
  addMenu(myMiscModesMenu);
  addMenu(myUtilitiesMenu);
  //myCheckInvisibleAction = addAction(tr("Check If Invisible"), this, SLOT(checkInvisible()));
  myCheckArAction = addAction(QString::null, this, SLOT(checkAutoResponse()));
  myCustomArAction = addAction(tr("Custom Auto Response..."), this, SLOT(customAutoResponse()));
  myCustomArAction->setCheckable(true);
  addSeparator();
  myToggleFloatyAction = addAction(tr("Toggle &Floaty"), this, SLOT(toggleFloaty()));
  addMenu(myGroupsMenu);
  myRemoveUserAction = addAction(tr("Remove From List"), this, SLOT(removeContact()));
  addSeparator();
#ifdef HAVE_LIBGPGME
  mySetKeyAction = addAction(tr("Set GPG key"), this, SLOT(selectKey()));
#endif
  myViewHistoryAction = addAction(tr("View &History"), this, SLOT(viewHistory()));
  myViewGeneralAction = addAction(tr("&Info"), this, SLOT(viewInfoGeneral()));

  connect(this, SIGNAL(aboutToShow()), SLOT(aboutToShowMenu()));

  updateIcons();
  updateGroups();

  connect(IconManager::instance(), SIGNAL(iconsChanged()), SLOT(updateIcons()));
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
#ifdef HAVE_LIBGPGME
  mySetKeyAction->setIcon(iconman->getIcon(IconManager::GpgKeyIcon));
#endif
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

  GroupList* g = gUserManager.LockGroupList(LOCK_R);
  for (unsigned int i = 0; i < g->size(); ++i)
  {
    QString name = QString::fromLocal8Bit((*g)[i]);

    a = myUserGroupActions->addAction(name);
    a->setData(i + 1);
    a->setCheckable(true);

    a = myServerGroupActions->addAction(name);
    a->setData(i + 1);
    a->setCheckable(true);
  }
  gUserManager.UnlockGroupList();

  // Add groups to menu
  myGroupsMenu->insertActions(myGroupSeparator, myUserGroupActions->actions());
  myServerGroupsMenu->addActions(myServerGroupActions->actions());
}

void UserMenu::aboutToShowMenu()
{
  ICQUser* u = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_R);

  int status = (u == NULL ? ICQ_STATUS_OFFLINE : u->Status());

  myCheckArAction->setEnabled(status != ICQ_STATUS_OFFLINE);

  if (status == ICQ_STATUS_OFFLINE || status == ICQ_STATUS_ONLINE)
    myCheckArAction->setText(tr("Check Auto Response"));
  else
    myCheckArAction->setText(tr("Check %1 Response")
        .arg(LicqStrings::getShortStatus(u, false)));

  if (u == NULL)
    return;

  myMiscModesActions[ModeAcceptInAway]->setChecked(u->AcceptInAway());
  myMiscModesActions[ModeAcceptInNa]->setChecked(u->AcceptInNA());
  myMiscModesActions[ModeAcceptInOccupied]->setChecked(u->AcceptInOccupied());
  myMiscModesActions[ModeAcceptInDnd]->setChecked(u->AcceptInDND());
  myMiscModesActions[ModeAutoFileAccept]->setChecked(u->AutoFileAccept());
  myMiscModesActions[ModeAutoChatAccept]->setChecked(u->AutoChatAccept());
  myMiscModesActions[ModeAutoSecure]->setChecked(u->AutoSecure());
  myMiscModesActions[ModeAutoSecure]->setEnabled(gLicqDaemon->CryptoEnabled());
#ifdef HAVE_LIBGPGME
  myMiscModesActions[ModeUseGpg]->setChecked(u->UseGPG());
#endif
  myMiscModesActions[ModeUseRealIp]->setChecked(u->SendRealIp());
  myMiscModesActions[ModeStatusOnline]->setChecked(u->StatusToUser() == ICQ_STATUS_ONLINE);
  myMiscModesActions[ModeStatusAway]->setChecked(u->StatusToUser() == ICQ_STATUS_AWAY);
  myMiscModesActions[ModeStatusNa]->setChecked(u->StatusToUser() == ICQ_STATUS_NA);
  myMiscModesActions[ModeStatusOccupied]->setChecked(u->StatusToUser() == ICQ_STATUS_OCCUPIED);
  myMiscModesActions[ModeStatusDnd]->setChecked(u->StatusToUser() == ICQ_STATUS_DND);

  myCustomArAction->setChecked(u->CustomAutoResponse()[0] != '\0');

  mySendActions[SendChat]->setEnabled(!u->StatusOffline());
  mySendActions[SendFile]->setEnabled(!u->StatusOffline());
  mySendActions[SendSms]->setEnabled(u->GetCellularNumber()[0] != '\0');
  if (u->Secure())
  {
    mySendActions[SendKey]->setText(tr("Close &Secure Channel"));
    mySendActions[SendKey]->setIcon(IconManager::instance()->getIcon(IconManager::SecureOnIcon));
  }
  else
  {
    mySendActions[SendKey]->setText(tr("Request &Secure Channel"));
    mySendActions[SendKey]->setIcon(IconManager::instance()->getIcon(IconManager::SecureOffIcon));
  }

  unsigned long sendFuncs = 0xFFFFFFFF;
  bool isIcq = myPpid == LICQ_PPID;

  if (!isIcq)
  {
    FOR_EACH_PROTO_PLUGIN_START(gLicqDaemon)
    {
      if ((*_ppit)->PPID() == myPpid)
      {
        sendFuncs = (*_ppit)->SendFunctions();
        break;
      }
    }
    FOR_EACH_PROTO_PLUGIN_END
  }

  mySendActions[SendMessage]->setVisible(sendFuncs & PP_SEND_MSG);
  mySendActions[SendUrl]->setVisible(sendFuncs & PP_SEND_URL);
  mySendActions[SendChat]->setVisible(sendFuncs & PP_SEND_CHAT);
  mySendActions[SendFile]->setVisible(sendFuncs & PP_SEND_FILE);
  mySendActions[SendContact]->setVisible(sendFuncs & PP_SEND_CONTACT);
  mySendActions[SendAuthorize]->setVisible(sendFuncs & PP_SEND_AUTH);
  mySendActions[SendReqAuthorize]->setVisible(sendFuncs & PP_SEND_AUTHxREQ);
  mySendActions[SendSms]->setVisible(sendFuncs & PP_SEND_SMS);
  mySendActions[SendKey]->setVisible(sendFuncs & PP_SEND_SECURE);
  myMiscModesActions[ModeAutoFileAccept]->setVisible(sendFuncs & PP_SEND_FILE);
  myMiscModesActions[ModeAutoChatAccept]->setVisible(sendFuncs & PP_SEND_CHAT);
  myMiscModesActions[ModeAutoSecure]->setVisible(sendFuncs & PP_SEND_SECURE);

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

  // Update group memberships
  foreach (QAction* a, myUserGroupActions->actions())
    a->setChecked(u->GetInGroup(GROUPS_USER, a->data().toUInt()));
  foreach (QAction* a, mySystemGroupActions->actions())
    a->setChecked(u->GetInGroup(GROUPS_SYSTEM, a->data().toUInt()));
  foreach (QAction* a, myServerGroupActions->actions())
    a->setChecked(u->GetSID() && gUserManager.GetGroupFromID(u->GetGSID()) == a->data().toUInt());

  gUserManager.DropUser(u);
}

void UserMenu::setUser(QString id, unsigned long ppid)
{
  myId = id;
  myPpid = ppid;
}

void UserMenu::popup(QPoint pos, QString id, unsigned ppid)
{
  setUser(id, ppid);
  QMenu::popup(pos);
}

void UserMenu::viewEvent()
{
  LicqGui::instance()->showEventDialog(mnuUserView, myId, myPpid);
}

void UserMenu::checkInvisible()
{
  if (myPpid == LICQ_PPID)
    gLicqDaemon->icqCheckInvisible(myId.toLatin1());
}

void UserMenu::checkAutoResponse()
{
  new ShowAwayMsgDlg(myId, myPpid, true);
}

void UserMenu::customAutoResponse()
{
  new CustomAutoRespDlg(myId, myPpid);
}

void UserMenu::toggleFloaty()
{
  LicqGui::instance()->toggleFloaty(myId, myPpid);
}

void UserMenu::removeContact()
{
  LicqGui::instance()->removeUserFromList(myId, myPpid);
}

void UserMenu::selectKey()
{
#ifdef HAVE_LIBGPGME
  new GPGKeySelect(myId, myPpid);
#endif
}

void UserMenu::viewHistory()
{
  new HistoryDlg(myId, myPpid);
}

void UserMenu::viewInfoGeneral()
{
  LicqGui::instance()->showInfoDialog(mnuUserGeneral, myId, myPpid);
}

void UserMenu::send(QAction* action)
{
  int index = action->data().toInt();

  switch (index)
  {
    case SendAuthorize:
      new AuthUserDlg(myId, myPpid, true);
      break;

    case SendReqAuthorize:
      new ReqAuthDlg(myId, myPpid);
      break;

    case SendKey:
      new KeyRequestDlg(myId, myPpid);
      break;

    case RequestUpdateInfoPlugin:
      if (myPpid == LICQ_PPID)
        gLicqDaemon->icqRequestInfoPluginList(myId.toLatin1(), true);
      break;

    case RequestUpdateStatusPlugin:
      if (myPpid == LICQ_PPID)
        gLicqDaemon->icqRequestStatusPluginList(myId.toLatin1(), true);
      break;

    case RequestPhoneFollowMeStatus:
      if (myPpid == LICQ_PPID)
        gLicqDaemon->icqRequestPhoneFollowMe(myId.toLatin1(), true);
      break;

    case RequestIcqphoneStatus:
      if (myPpid == LICQ_PPID)
        gLicqDaemon->icqRequestICQphone(myId.toLatin1(), true);
      break;

    case RequestFileServerStatus:
      if (myPpid == LICQ_PPID)
        gLicqDaemon->icqRequestSharedFiles(myId.toLatin1(), true);
      break;

    default:
      LicqGui::instance()->showEventDialog(index, myId, myPpid);
  }
}

void UserMenu::toggleMiscMode(QAction* action)
{
  int mode = action->data().toInt();
  bool newState = action->isChecked();

  ICQUser* u = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_W);
  if (u == NULL)
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

#ifdef HAVE_LIBGPGME
    case ModeUseGpg:
    {
      if (strcmp(u->GPGKey(), "") != 0)
      {
        u->SetUseGPG(newState);
        gUserManager.DropUser(u);
      }
      else
      {
        gUserManager.DropUser(u);
        new GPGKeySelect(myId, myPpid);
      }
      // update icon
      LicqGui::instance()->updateUserData(myId, myPpid);
      return;
    }
#endif

    case ModeUseRealIp:
      u->SetSendRealIp(newState);
      break;

    case ModeStatusOnline:
      u->SetStatusToUser(newState ? ICQ_STATUS_ONLINE : ICQ_STATUS_OFFLINE);
      break;

    case ModeStatusAway:
      u->SetStatusToUser(newState ? ICQ_STATUS_AWAY : ICQ_STATUS_OFFLINE);
      break;

    case ModeStatusNa:
      u->SetStatusToUser(newState ? ICQ_STATUS_NA : ICQ_STATUS_OFFLINE);
      break;

    case ModeStatusOccupied:
      u->SetStatusToUser(newState ? ICQ_STATUS_OCCUPIED : ICQ_STATUS_OFFLINE);
      break;

    case ModeStatusDnd:
      u->SetStatusToUser(newState ? ICQ_STATUS_DND : ICQ_STATUS_OFFLINE);
      break;
  }
  gUserManager.DropUser(u);
}

void UserMenu::utility(QAction* action)
{
  int index = action->data().toUInt();

  CUtility* u = gUtilityManager.Utility(index);

  if (u != NULL)
    new UtilityDlg(u, myId, myPpid);
}

void UserMenu::toggleUserGroup(QAction* action)
{
  unsigned int gid = action->data().toUInt();

  ICQUser* u = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_W);

  if (u == NULL)
    return;

  // Don't let the user to be deleted from local group
  // if he is a member of the same server group
  if (!action->isChecked() &&                             // delete
      u->GetSID() &&                                      // server-side
      gUserManager.GetGroupFromID(u->GetGSID()) == gid)   // same group
  {
    gUserManager.DropUser(u);
    return;
  }

  u->SetInGroup(GROUPS_USER, gid, action->isChecked());

  gUserManager.DropUser(u);

  // The daemon does not send an update when group membership changes
  // so tell the contactList it needs to update
  LicqGui::instance()->contactList()->updateUser(myId, myPpid);
}

void UserMenu::toggleSystemGroup(QAction* action)
{
  unsigned int gid = action->data().toUInt();

  ICQUser* u = NULL;
  if (gid == GROUP_NEW_USERS || gid == GROUP_ONLINE_NOTIFY || gid == GROUP_IGNORE_LIST)
  {
    u = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_W);
    if (u == NULL)
      return;
  }

  switch (gid)
  {
    case GROUP_NEW_USERS:
      u->SetNewUser(action->isChecked());
      break;

    case GROUP_ONLINE_NOTIFY:
      u->SetOnlineNotify(action->isChecked());
      break;

    case GROUP_VISIBLE_LIST:
      gLicqDaemon->ProtoToggleVisibleList(myId.toLatin1(), myPpid);
      break;

    case GROUP_INVISIBLE_LIST:
      gLicqDaemon->ProtoToggleInvisibleList(myId.toLatin1(), myPpid);
      break;

    case GROUP_IGNORE_LIST:
      if (!action->isChecked() ||
          QueryYesNo(this, tr("Do you really want to add\n%1 (%2)\nto your ignore list?")
            .arg(QString::fromUtf8(u->GetAlias())).arg(myId)))
      {
        u->SetIgnoreList(action->isChecked());
        gUserManager.DropUser(u);
        u = NULL;
        gLicqDaemon->icqToggleIgnoreList(myId.toLatin1(), myPpid); // network only
      }
      break;
  }

  if (u != NULL)
    gUserManager.DropUser(u);

  // The daemon does not send an update when group membership changes
  // so tell the contactList it needs to update
  LicqGui::instance()->contactList()->updateUser(myId, myPpid);
}

void UserMenu::setServerGroup(QAction* action)
{
  gUserManager.AddUserToGroup(myId.toLatin1(), myPpid, action->data().toUInt());

  // The daemon does not send an update when group membership changes
  // so tell the contactList it needs to update
  LicqGui::instance()->contactList()->updateUser(myId, myPpid);
}