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

#ifndef USERMENU_H
#define USERMENU_H

#include "config.h"

#include <QMap>
#include <QMenu>

#include <licq/userid.h>

#include "gui-defines.h"

class QActionGroup;

namespace LicqQtGui
{
/**
 * User menu for contact list and user dialogs.
 *
 * The same menu is used for all contacts. Before displaying, the list is updated
 * with options and settings for the selected contact.
 */
class UserMenu : public QMenu
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param parent Parent widget
   */
  UserMenu(QWidget* parent = 0);

  ~UserMenu();

  /**
   * Update the list of groups from the daemon. This must be called when the
   * list of groups is changed.
   */
  void updateGroups();

  /**
   * Change which contact the menu will be displayed for.
   *
   * @param userId Contact id
   * @param showShortcuts Show key shortcuts in menu (only set in main window)
   */
  void setUser(const Licq::UserId& userId, bool showShortcuts = false);

  /**
   * Convenience function t set user and popup the menu on a given location.
   *
   * @param pos Posititon to show menu in global coordinates
   * @param userId Contact id
   * @param showShortcuts Show key shortcuts in menu (only set in main window)
   */
  void popup(QPoint pos, const Licq::UserId& userId, bool showShortcuts = false);

private slots:
  /**
   * Update icons in menu.
   */
  void updateIcons();

  void aboutToShowMenu();

  void viewEvent();
  void checkInvisible();
  void checkAutoResponse();
  void customAutoResponse();
  void makePermanent();
  void toggleFloaty();
  void removeContact();
  void selectKey();
  void copyIdToClipboard();
  void viewHistory();
  void viewInfoGeneral();

  void send(QAction* action);
  void toggleMiscMode(QAction* action);
  void utility(QAction* action);

  void toggleUserGroup(QAction* action);
  void toggleSystemGroup(QAction* action);
  void setServerGroup(QAction* action);

private:
  // Current contact
  Licq::UserId myUserId;
  QString myId;
  unsigned long myPpid;
  bool myShowShortcuts;

  // Internal numbering of send sub menu entries
  enum SendModes
  {
    SendMessage = MessageEvent,
    SendUrl = UrlEvent,
    SendChat = ChatEvent,
    SendFile = FileEvent,
    SendContact = ContactEvent,
    SendSms = SmsEvent,
    SendAuthorize,
    SendReqAuthorize,
    RequestUpdateInfoPlugin,
    RequestUpdateStatusPlugin,
    RequestPhoneFollowMeStatus,
    RequestIcqphoneStatus,
    RequestFileServerStatus,
    SendKey
  };

  // Internal numbering of misc modes sub menu entries
  enum MiscModes
  {
    ModeAcceptInAway,
    ModeAcceptInNa,
    ModeAcceptInOccupied,
    ModeAcceptInDnd,
    ModeAutoFileAccept,
    ModeAutoChatAccept,
    ModeAutoSecure,
    ModeUseGpg,
    ModeUseRealIp,
    ModeStatusOnline,
    ModeStatusAway,
    ModeStatusNa,
    ModeStatusOccupied,
    ModeStatusDnd,
    ModeOnlineNotify,
    ModeVisibleList,
    ModeInvisibleList,
    ModeIgnoreList,
    ModeNewUser,
  };

  // Actions not in any sub menu
  QAction* myViewEventAction;
  QAction* myCheckInvisibleAction;
  QAction* myCheckArAction;
  QAction* myCustomArAction;
  QAction* myMakePermanentAction;
  QAction* myToggleFloatyAction;
  QAction* myRemoveUserAction;
  QAction* mySetKeyAction;
  QAction* myCopyIdAction;
  QAction* myViewHistoryAction;
  QAction* myViewGeneralAction;

  // Sub menus
  QMenu* mySendMenu;
  QMenu* myMiscModesMenu;
  QMenu* myUtilitiesMenu;
  QMenu* myGroupsMenu;
  QMenu* myServerGroupsMenu;
  QAction* myGroupSeparator;

  // Containers for the group sub menu entries
  QActionGroup* myUserGroupActions;
  QActionGroup* mySystemGroupActions;
  QActionGroup* myServerGroupActions;

  // Maps for holding sub menu entries
  QMap<SendModes, QAction*> mySendActions;
  QMap<MiscModes, QAction*> myMiscModesActions;
};

extern UserMenu* gUserMenu;

} // namespace LicqQtGui

#endif
