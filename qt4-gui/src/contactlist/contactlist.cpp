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

#include "contactlist.h"

#include <cstring>

#include <licq_events.h>
#include <licq_log.h>
#include <licq_user.h>

#include "config/contactlist.h"
#include "helpers/licqstrings.h"

#include "contactbar.h"
#include "contactgroup.h"
#include "contactuser.h"
#include "contactuserdata.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ContactListModel */

ContactListModel::ContactListModel(QObject* parent)
  : QAbstractItemModel(parent)
{
  // Create the system groups
  for (unsigned long i = 0; i < NUM_GROUPS_SYSTEM_ALL; ++i)
  {
    mySystemGroups[i] = new ContactGroup(SystemGroupOffset + i,
        LicqStrings::getSystemGroupName(i));
    connect(mySystemGroups[i], SIGNAL(dataChanged(ContactGroup*)),
        SLOT(groupDataChanged(ContactGroup*)));
    connect(mySystemGroups[i], SIGNAL(barDataChanged(ContactBar*, int)),
        SLOT(slot_barDataChanged(ContactBar*, int)));
  }

  // Get the entire contact list from the daemon
  reloadAll();

  connect(Config::ContactList::instance(), SIGNAL(listLayoutChanged()),
      SLOT(configUpdated()));
}

ContactListModel::~ContactListModel()
{
  // Delete all users and groups
  clear();

  // Delete the system groups
  for (unsigned long i = 0; i < NUM_GROUPS_SYSTEM_ALL; ++i)
    delete mySystemGroups[i];
}

void ContactListModel::slot_updatedList(CICQSignal* sig)
{
  switch(sig->SubSignal())
  {
    case LIST_ALL:
      reloadAll();
      break;

    case LIST_ADD:
    {
      ICQUser* u = gUserManager.FetchUser(sig->Id(), sig->PPID(), LOCK_R);
      if (u == NULL)
      {
        gLog.Warn("%sContactList::slot_updatedList(): Invalid user received: %lu, %s\n", L_ERRORxSTR, sig->PPID(), sig->Id());
        break;
      }
      addUser(u);
      gUserManager.DropUser(u);
      break;
    }
    case LIST_REMOVE:
      removeUser(sig->Id(), sig->PPID());
      break;
  }
}

void ContactListModel::slot_updatedUser(CICQSignal* sig)
{
  // Skip events for owners
  if (gUserManager.FindOwner(sig->Id(), sig->PPID()) != NULL)
    return;

  ContactUserData* user = findUser(sig->Id(), sig->PPID());
  if (user == NULL)
  {
    gLog.Warn("%sContactList::slot_updatedUser(): Invalid user received: %lu, %s\n", L_ERRORxSTR, sig->PPID(), sig->Id());
    return;
  }

  // Forward signal to the ContactUserData object
  user->update(sig);
}

void ContactListModel::updateUser(QString id, unsigned long ppid)
{
  ContactUserData* userData = findUser(id, ppid);
  if (userData == NULL)
    return;

  ICQUser* u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);
  if (u == NULL)
    return;

  userData->updateAll(u);
  updateUserGroups(userData, u);
  gUserManager.DropUser(u);
}

void ContactListModel::configUpdated()
{
  // Update number of columns
  int newColumnCount = Config::ContactList::instance()->columnCount();
  if (newColumnCount > myColumnCount)
  {
    emit layoutAboutToBeChanged();
    beginInsertColumns(QModelIndex(), myColumnCount, newColumnCount - 1);
    myColumnCount = newColumnCount;
    endInsertColumns();
    emit layoutChanged();
  }
  else if (newColumnCount < myColumnCount)
  {
    emit layoutAboutToBeChanged();
    beginRemoveColumns(QModelIndex(), newColumnCount, myColumnCount - 1);
    myColumnCount = newColumnCount;
    endRemoveColumns();
    emit layoutChanged();
  }

  // On all users, update cached data that is dependant on gui config
  foreach (ContactUserData* user, myUsers)
  {
    user->configUpdated();
  }
}

void ContactListModel::slot_userDataChanged(const ContactUserData* user)
{
  // Emit signal that the user has changed in all groups
  foreach (ContactUser* u, user->groupList())
  {
    int row = u->group()->indexOf(u);
    emit dataChanged(createIndex(row, 0, u), createIndex(row, myColumnCount - 1, u));
  }
}

void ContactListModel::groupDataChanged(ContactGroup* group)
{
  int groupRow = (group->groupId() < SystemGroupOffset
      ? myUserGroups.indexOf(group)
      : myUserGroups.size() + group->groupId() - SystemGroupOffset);

  emit dataChanged(createIndex(groupRow, 0, group), createIndex(groupRow, myColumnCount - 1, group));
}

void ContactListModel::slot_barDataChanged(ContactBar* bar, int row)
{
  emit dataChanged(createIndex(row, 0, bar), createIndex(row, myColumnCount - 1, bar));
}

void ContactListModel::reloadAll()
{
  // Clear the list of all old groups and users
  clear();
  myColumnCount = Config::ContactList::instance()->columnCount();

  // Add all groups
  GroupList* g = gUserManager.LockGroupList(LOCK_R);
  beginInsertRows(QModelIndex(), 0, g->size());

  ContactGroup* newGroup = new ContactGroup(0, tr("Other Users"));
  connect(newGroup, SIGNAL(dataChanged(ContactGroup*)),
      SLOT(groupDataChanged(ContactGroup*)));
  connect(newGroup, SIGNAL(barDataChanged(ContactBar*, int)),
      SLOT(slot_barDataChanged(ContactBar*, int)));
  myUserGroups.append(newGroup);

  for (unsigned short i = 0; i < g->size(); ++i)
  {
    newGroup = new ContactGroup(i+1, QString::fromLocal8Bit((*g)[i]));
    connect(newGroup, SIGNAL(dataChanged(ContactGroup*)),
        SLOT(groupDataChanged(ContactGroup*)));
    connect(newGroup, SIGNAL(barDataChanged(ContactBar*, int)),
        SLOT(slot_barDataChanged(ContactBar*, int)));
    myUserGroups.append(newGroup);
  }

  endInsertRows();
  gUserManager.UnlockGroupList();

  // Add all users
  FOR_EACH_USER_START(LOCK_R)
  {
    addUser(pUser);
  }
  FOR_EACH_USER_END

  // Tell views that we have done major changes
  reset();
}

ContactUserData* ContactListModel::findUser(QString id, unsigned long ppid) const
{
  foreach (ContactUserData* user, myUsers)
  {
    if (user->id() == id && user->ppid() == ppid)
      return user;
  }
  return 0;
}

void ContactListModel::addUser(ICQUser* licqUser)
{
  ContactUserData* newUser = new ContactUserData(licqUser, this);
  connect(newUser, SIGNAL(dataChanged(const ContactUserData*)),
      SLOT(slot_userDataChanged(const ContactUserData*)));
  connect(newUser, SIGNAL(updateUserGroups(ContactUserData*, ICQUser*)),
      SLOT(updateUserGroups(ContactUserData*, ICQUser*)));

  myUsers.append(newUser);
  updateUserGroups(newUser, licqUser);
}

void ContactListModel::updateUserGroups(ContactUserData* user, ICQUser* licqUser)
{
  // Check which user groups the user should be member of
  for (int i = 0; i < myUserGroups.size(); ++i)
  {
    ContactGroup* group = myUserGroups.at(i);
    unsigned short gid = group->groupId();
    bool shouldBeMember = (gid != 0 && licqUser->GetInGroup(GROUPS_USER, gid)) ||
        (gid == 0 && licqUser->GetGroups(GROUPS_USER) == 0 && !licqUser->IgnoreList());
    updateUserGroup(user, group, i, shouldBeMember);
  }

  // Check which system groups the user should be member of
  for (unsigned long i = 0; i < NUM_GROUPS_SYSTEM_ALL; ++i)
  {
    updateUserGroup(user, mySystemGroups[i], i, licqUser->GetInGroup(GROUPS_SYSTEM, i));
  }
}

void ContactListModel::updateUserGroup(ContactUserData* user, ContactGroup* group, int groupRow, bool shouldBeMember)
{
  ContactUser* member = group->user(user);

  // If membership is already correct then there is nothing to do
  if ((member != 0) == shouldBeMember)
    return;

  if (shouldBeMember)
  {
    // Add user to group
    beginInsertRows(createIndex(groupRow, 0, group), group->rowCount(), group->rowCount());
    new ContactUser(user, group);
    endInsertRows();
  }
  else
  {
    // Remove user from the group
    int index = group->indexOf(member);
    beginRemoveRows(createIndex(groupRow, 0, group), index, index);
    delete member;
    endRemoveRows();
  }
}

void ContactListModel::removeUser(QString id, unsigned long ppid)
{
  ContactUserData* user = findUser(id, ppid);
  if (user == NULL)
    return;

  foreach (ContactUser* u, user->groupList())
  {
    ContactGroup* group = u->group();
    int pos = group->indexOf(u);
    int groupRow = (group->groupId() < SystemGroupOffset ? myUserGroups.indexOf(group) : myUserGroups.size() + group->groupId() - SystemGroupOffset);
    beginRemoveRows(createIndex(groupRow, 0, group), pos, pos);
    delete u;
    endRemoveRows();
  }

  myUsers.removeAll(user);
  delete user;
}

void ContactListModel::clear()
{
  // Clear all users and user groups. System groups and their bars are never removed.

  while (!myUsers.isEmpty())
    delete myUsers.takeFirst();

  while (!myUserGroups.isEmpty())
    delete myUserGroups.takeFirst();
}

QModelIndex ContactListModel::index(int row, int column, const QModelIndex& parent) const
{
  // If root item is requested we return a group
  if (!parent.isValid())
  {
    ContactGroup* group;
    if (row < myUserGroups.size())
      group = myUserGroups.value(row);
    else if (row < myUserGroups.size() + static_cast<int>(NUM_GROUPS_SYSTEM_ALL))
      group = mySystemGroups[row - myUserGroups.size()];
    else
      return QModelIndex();

    return createIndex(row, column, group);
  }

  ContactGroup* group = static_cast<ContactGroup*>(parent.internalPointer());

  ContactItem* item = group->item(row);
  if (item == NULL)
    return QModelIndex();

  return createIndex(row, column, item);
}

QModelIndex ContactListModel::parent(const QModelIndex& index) const
{
  if (!index.isValid())
    return QModelIndex();

  ContactGroup* group;

  switch (static_cast<ContactItem*>(index.internalPointer())->itemType())
  {
    case UserItem:
      group = static_cast<ContactUser*>(index.internalPointer())->group();
      break;

    case BarItem:
      group = static_cast<ContactBar*>(index.internalPointer())->group();
      break;

    case GroupItem:
    default:
      return QModelIndex();
  }
  int groupRow = (group->groupId() < SystemGroupOffset ? myUserGroups.indexOf(group) : myUserGroups.size() + group->groupId() - SystemGroupOffset);
  return createIndex(groupRow, 0, group);
}

int ContactListModel::rowCount(const QModelIndex& parent) const
{
  // Root items requested so return number of groups
  if (!parent.isValid())
    return myUserGroups.size() + NUM_GROUPS_SYSTEM_ALL;

  // A group so return the number of users in that group
  if (static_cast<ContactItem*>(parent.internalPointer())->itemType() == GroupItem)
  {
    ContactGroup* group = static_cast<ContactGroup*>(parent.internalPointer());
    return group->rowCount();
  }

  // Users don't have any children in the tree
  return 0;
}

int ContactListModel::columnCount(const QModelIndex& /*parent*/) const
{
  return myColumnCount;
}

QVariant ContactListModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  return static_cast<ContactItem*>(index.internalPointer())->data(index.column(), role);
}

Qt::ItemFlags ContactListModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ContactListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  // No vertical header here
  if (orientation != Qt::Horizontal)
    return QVariant();

  if (role == Qt::DisplayRole && static_cast<unsigned short>(section) < myColumnCount)
    return Config::ContactList::instance()->columnHeading(section);

  return QVariant();
}

QModelIndex ContactListModel::userIndex(QString id, unsigned long ppid, int column) const
{
  ContactUserData* userData = findUser(id, ppid);
  if (userData != NULL)
  {
    // Find the user in the "All Users" group, this will not find users on ignore list
    ContactUser* user = mySystemGroups[GROUP_ALL_USERS]->user(userData);
    if (user != NULL)
      return createIndex(mySystemGroups[GROUP_ALL_USERS]->indexOf(user), column, user);
  }

  return QModelIndex();
}

QModelIndex ContactListModel::groupIndex(GroupType type, unsigned long id) const
{
  if (type == GROUPS_SYSTEM && id < NUM_GROUPS_SYSTEM_ALL)
  {
    return createIndex(myUserGroups.size() + id, 0, mySystemGroups[id]);
  }
  else if (type == GROUPS_USER && static_cast<int>(id) < myUserGroups.size())
  {
    for (int i = 0; i < myUserGroups.size(); ++i)
    {
      ContactGroup* group = myUserGroups.at(i);
      if (group->groupId() == id)
        return createIndex(i, 0, group);
    }
  }

  return QModelIndex();
}
