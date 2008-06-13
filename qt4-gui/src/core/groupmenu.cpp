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

#include "groupmenu.h"

#include "config.h"

#include <licq_icqd.h>
#include <licq_user.h>

#include "config/iconmanager.h"
#include "contactlist/contactlist.h"
#include "helpers/licqstrings.h"

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
  for (unsigned int i = 1; i < NUM_GROUPS_SYSTEM_ALL; ++i)
  {
    a = mySystemGroupActions->addAction(LicqStrings::getSystemGroupName(i));
    a->setData(i + ContactListModel::SystemGroupOffset);
  }

  myGroupSeparator = myGroupsMenu->addSeparator();
  myGroupsMenu->addActions(mySystemGroupActions->actions());

  // Menu
  myMoveUpAction = addAction(tr("Move &Up"), this, SLOT(moveGroupUp()));
  myMoveDownAction = addAction(tr("Move &Down"), this, SLOT(moveGroupDown()));
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

  FOR_EACH_GROUP_START_SORTED(LOCK_R)
  {
    QString name = QString::fromLocal8Bit(pGroup->name().c_str());

    a = myUserGroupActions->addAction(name);
    a->setData(pGroup->id());

    myGroupsMenu->insertAction(myGroupSeparator, a);
  }
  FOR_EACH_GROUP_END

  // Add groups to menu
  myGroupsMenu->insertActions(myGroupSeparator, myUserGroupActions->actions());
}

void GroupMenu::aboutToShowMenu()
{
  // Hide current group from move to submenu
  foreach (QAction* a, mySystemGroupActions->actions())
    a->setVisible(a->data().toUInt() != myGroupId);
  foreach (QAction* a, myUserGroupActions->actions())
    a->setVisible(a->data().toUInt() != myGroupId);

  // Actions that are only available for user groups
  bool special = (myGroupId >= ContactListModel::SystemGroupOffset || myGroupId == 0);
  myMoveUpAction->setEnabled(!special && myGroupId > 1);
  myMoveDownAction->setEnabled(!special && myGroupId < gUserManager.NumGroups());
  myRemoveGroupAction->setEnabled(!special);

  mySortIndex = 0;
  if (!special)
  {
    LicqGroup* group = gUserManager.FetchGroup(myGroupId, LOCK_R);
    if (group != NULL)
    {
      mySortIndex = group->sortIndex();
      myGroupName = QString::fromLocal8Bit(group->name().c_str());
      gUserManager.DropGroup(group);
    }
  }
}

void GroupMenu::setGroup(unsigned int groupId)
{
  myGroupId = groupId;
}

void GroupMenu::popup(QPoint pos, unsigned int groupId)
{
  setGroup(groupId);
  QMenu::popup(pos);
}

void GroupMenu::moveGroupUp()
{
  if (mySortIndex == 0)
    return;

  gUserManager.ModifyGroupSorting(myGroupId, mySortIndex - 1);
}

void GroupMenu::moveGroupDown()
{
  gUserManager.ModifyGroupSorting(myGroupId, mySortIndex + 1);
}

void GroupMenu::removeGroup()
{
  QString warning(tr("Are you sure you want to remove the group '%1'?")
      .arg(myGroupName));
  if (!QueryYesNo(this, warning))
    return;

  gUserManager.RemoveGroup(myGroupId);
}

void GroupMenu::addUsersToGroup(QAction* action)
{
  // Group id used by model
  unsigned int groupId = action->data().toUInt();

  // Group id used by daemon
  GroupType gtype = (groupId < ContactListModel::SystemGroupOffset ? GROUPS_USER : GROUPS_SYSTEM);
  unsigned int gid = (groupId < ContactListModel::SystemGroupOffset ? groupId - 1 : groupId - ContactListModel::SystemGroupOffset);

  ContactListModel* list = LicqGui::instance()->contactList();
  QModelIndex groupIndex = list->groupIndex(myGroupId);
  int userCount = list->rowCount(groupIndex);

  for (int i = 0; i < userCount; ++i)
  {
    QModelIndex userIndex = list->index(i, 0, groupIndex);

    QString id = userIndex.data(ContactListModel::UserIdRole).toString();
    unsigned long ppid = userIndex.data(ContactListModel::PpidRole).toUInt();

    gUserManager.SetUserInGroup(id.toLatin1(), ppid, gtype, gid, true,
        gtype == GROUPS_SYSTEM);
  }
}
