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

#include "groupmenu.h"

#include "config.h"

#include <boost/foreach.hpp>

#include <licq/contactlist/group.h>
#include <licq/contactlist/usermanager.h>

#include "config/contactlist.h"
#include "config/iconmanager.h"
#include "dialogs/groupdlg.h"
#include "views/userview.h"

#include "licqgui.h"
#include "mainwin.h"
#include "messagebox.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::GroupMenu */

GroupMenu::GroupMenu(QWidget* parent)
  : QMenu(parent)
{
  QAction* a;

  // Sub menu Add to Group
  myGroupsMenu = new QMenu(tr("Add Users to"));
  myUserGroupActions = new QActionGroup(this);
  myUserGroupActions->setExclusive(false);
  connect(myUserGroupActions, SIGNAL(triggered(QAction*)), SLOT(addUsersToGroup(QAction*)));
  mySystemGroupActions = new QActionGroup(this);
  mySystemGroupActions->setExclusive(false);
  connect(mySystemGroupActions, SIGNAL(triggered(QAction*)), SLOT(addUsersToGroup(QAction*)));

  // System groups
  for (int i = ContactListModel::SystemGroupOffset; i <= ContactListModel::LastSystemGroup; ++i)
  {
    a = mySystemGroupActions->addAction(ContactListModel::systemGroupName(i));
    a->setData(i);
  }

  myGroupSeparator = myGroupsMenu->addSeparator();
  myGroupsMenu->addActions(mySystemGroupActions->actions());

  // Menu
  myMoveUpAction = addAction(tr("Move &Up"), this, SLOT(moveGroupUp()));
  myMoveDownAction = addAction(tr("Move &Down"), this, SLOT(moveGroupDown()));
  myRenameAction = addAction(tr("Rename"), this, SLOT(renameGroup()));
  mySettingsAction = addAction(tr("Sounds..."), this, SLOT(settings()));
  addMenu(myGroupsMenu);
  myRemoveGroupAction = addAction(tr("Remove Group"), this, SLOT(removeGroup()));

  updateIcons();
  updateGroups();

  connect(this, SIGNAL(aboutToShow()), SLOT(aboutToShowMenu()));
  connect(IconManager::instance(), SIGNAL(iconsChanged()), SLOT(updateIcons()));
}

void GroupMenu::updateIcons()
{
  IconManager* iconman = IconManager::instance();

  myRemoveGroupAction->setIcon(iconman->getIcon(IconManager::RemoveIcon));
}

void GroupMenu::updateGroups()
{
  QAction* a;

  // Clear old groups but leave system groups as they never change
  foreach (a, myUserGroupActions->actions())
    delete a;

  Licq::GroupListGuard groupList;
  BOOST_FOREACH(const Licq::Group* group, **groupList)
  {
    Licq::GroupReadGuard pGroup(group);

    QString name = QString::fromLocal8Bit(pGroup->name().c_str());
    name.replace("&", "&&");

    a = myUserGroupActions->addAction(name);
    a->setData(pGroup->id());

    myGroupsMenu->insertAction(myGroupSeparator, a);
  }

  // Add groups to menu
  myGroupsMenu->insertActions(myGroupSeparator, myUserGroupActions->actions());
}

void GroupMenu::aboutToShowMenu()
{
  // Hide current group from move to submenu
  foreach (QAction* a, mySystemGroupActions->actions())
    a->setVisible(a->data().toInt() != myGroupId);
  foreach (QAction* a, myUserGroupActions->actions())
    a->setVisible(a->data().toInt() != myGroupId);

  // Actions that are only available for user groups
  bool userGroup = (myGroupId < ContactListModel::SystemGroupOffset);
  myRenameAction->setEnabled(userGroup);
  myRemoveGroupAction->setEnabled(userGroup);

  // OnEvent manager in daemon does not handle gui system groups
  mySettingsAction->setEnabled(userGroup);

  mySortIndex = 0;
  if (userGroup)
  {
    Licq::GroupReadGuard group(myGroupId);
    if (group.isLocked())
    {
      mySortIndex = group->sortIndex();
      myGroupName = QString::fromLocal8Bit(group->name().c_str());
    }
  }

  myMoveUpAction->setEnabled(userGroup && mySortIndex > 0);
  myMoveDownAction->setEnabled(userGroup && static_cast<unsigned int>(mySortIndex) < Licq::gUserManager.NumGroups()-1);
}

void GroupMenu::setGroup(int groupId, bool online)
{
  myGroupId = groupId;
  myOnline = online;
}

void GroupMenu::popup(QPoint pos, int groupId, bool online)
{
  setGroup(groupId, online);
  QMenu::popup(pos);
}

void GroupMenu::moveGroupUp()
{
  if (mySortIndex == 0)
    return;

  Licq::gUserManager.ModifyGroupSorting(myGroupId, mySortIndex - 1);
}

void GroupMenu::moveGroupDown()
{
  Licq::gUserManager.ModifyGroupSorting(myGroupId, mySortIndex + 1);
}

void GroupMenu::renameGroup()
{
  gMainWindow->getUserView()->editGroupName(myGroupId, myOnline);
}

void GroupMenu::settings()
{
  new GroupDlg(myGroupId);
}

void GroupMenu::removeGroup()
{
  QString warning(tr("Are you sure you want to remove the group '%1'?")
      .arg(myGroupName));
  if (!QueryYesNo(this, warning))
    return;

  Licq::gUserManager.RemoveGroup(myGroupId);
}

void GroupMenu::addUsersToGroup(QAction* action)
{
  int groupId = action->data().toInt();

  QModelIndex groupIndex = gGuiContactList->groupIndex(myGroupId);
  int userCount = gGuiContactList->rowCount(groupIndex);

  for (int i = 0; i < userCount; ++i)
  {
    QModelIndex userIndex = gGuiContactList->index(i, 0, groupIndex);

    Licq::UserId userId = userIndex.data(ContactListModel::UserIdRole).value<Licq::UserId>();

    // Call function that knows how to handle system groups
    gLicqGui->setUserInGroup(userId, groupId, true, groupId >= ContactListModel::SystemGroupOffset);
  }
}
