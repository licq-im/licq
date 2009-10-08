/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2008-2009 Licq developers
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

#ifndef GROUPMENU_H
#define GROUPMENU_H

#include "config.h"

#include <QMenu>

class QActionGroup;

namespace LicqQtGui
{
/**
 * Group menu for contact list
 *
 * The same menu is used for all groups. Before displaying, the list is updated
 * with options and settings for the selected group.
 */
class GroupMenu : public QMenu
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param parent Parent widget
   */
  GroupMenu(QWidget* parent = 0);

  /**
   * Update the list of groups from the daemon. This must be called when the
   * list of groups is changed.
   */
  void updateGroups();

  /**
   * Change which group the menu will be displayed for.
   *
   * @param groupId Group id
   * @param online True if menu is opened for single or online instance of group
   */
  void setGroup(int groupId, bool online);

  /**
   * Convenience function to set group and popup the menu on a given location.
   *
   * @param pos Posititon to show menu in global coordinates
   * @param groupId Group id
   * @param online True if menu is opened for single or online instance of group
   */
  void popup(QPoint pos, int groupId, bool online);

private slots:
  /**
   * Update icons in menu.
   */
  void updateIcons();

  /**
   * Prepare menu for display
   */
  void aboutToShowMenu();

  /**
   * Move the current group up one step in the list
   */
  void moveGroupUp();

  /**
   * Move the current group down one step in the list
   */
  void moveGroupDown();

  /**
   * Rename the current group
   */
  void renameGroup();

  /**
   * Delete the group from the list
   */
  void removeGroup();

  /**
   * Add all contacts of the current group to the selected group
   */
  void addUsersToGroup(QAction* action);

private:
  // Current group
  int myGroupId;
  bool myOnline;
  int mySortIndex;
  QString myGroupName;

  // Actions not in any sub menu
  QAction* myMoveUpAction;
  QAction* myMoveDownAction;
  QAction* myRenameAction;
  QAction* myRemoveGroupAction;

  // Sub menus
  QMenu* myGroupsMenu;
  QAction* myGroupSeparator;

  // Containers for the group sub menu entries
  QActionGroup* myUserGroupActions;
  QActionGroup* mySystemGroupActions;
};

} // namespace LicqQtGui

#endif
