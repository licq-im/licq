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
  : QAbstractItemModel(parent),
    myBlockUpdates(false)
{
  // Create the system groups
  for (unsigned long i = 0; i < NUM_GROUPS_SYSTEM_ALL; ++i)
  {
    mySystemGroups[i] = new ContactGroup(SystemGroupOffset + i,
        LicqStrings::getSystemGroupName(i));
    connectGroup(mySystemGroups[i]);
  }

  // Get the entire contact list from the daemon
  reloadAll();

  connect(Config::ContactList::instance(), SIGNAL(listLayoutChanged()),
      SLOT(configUpdated()));
}

void ContactListModel::connectGroup(ContactGroup* group)
{
  connect(group, SIGNAL(dataChanged(ContactGroup*)),
      SLOT(groupDataChanged(ContactGroup*)));
  connect(group, SIGNAL(barDataChanged(ContactBar*, int)),
      SLOT(barDataChanged(ContactBar*, int)));
  connect(group, SIGNAL(beginInsert(ContactGroup*, int)),
      SLOT(groupBeginInsert(ContactGroup*, int)));
  connect(group, SIGNAL(endInsert()), SLOT(groupEndInsert()));
  connect(group, SIGNAL(beginRemove(ContactGroup*, int)),
      SLOT(groupBeginRemove(ContactGroup*, int)));
  connect(group, SIGNAL(endRemove()), SLOT(groupEndRemove()));
}

ContactListModel::~ContactListModel()
{
  // Delete all users and groups
  clear();

  // Delete the system groups
  for (unsigned long i = 0; i < NUM_GROUPS_SYSTEM_ALL; ++i)
    delete mySystemGroups[i];
}

void ContactListModel::listUpdated(CICQSignal* sig)
{
  switch(sig->SubSignal())
  {
    case LIST_INVALIDATE:
      reloadAll();
      break;

    case LIST_CONTACT_ADDED:
    {
      const ICQUser* u = gUserManager.FetchUser(sig->Id(), sig->PPID(), LOCK_R);
      if (u == NULL)
      {
        char* ppidString = PPIDSTRING(sig->PPID());
        gLog.Warn("%sContactList::listUpdated(): Invalid user received: %s (%s)\n",
            L_ERRORxSTR, sig->Id(), ppidString);
        delete[] ppidString;
        break;
      }
      addUser(u);
      gUserManager.DropUser(u);
      break;
    }
    case LIST_CONTACT_REMOVED:
      removeUser(sig->Id(), sig->PPID());
      break;

    case LIST_GROUP_ADDED:
    {
      unsigned short gid = sig->Argument();

      // Set inital expanded state for new group
      Config::ContactList::instance()->setGroupState(gid, true);

      ContactGroup* newGroup = new ContactGroup(gid);
      connectGroup(newGroup);
      beginInsertRows(QModelIndex(), myUserGroups.size(), myUserGroups.size());
      myUserGroups.append(newGroup);
      endInsertRows();
      break;
    }

    case LIST_GROUP_REMOVED:
    {
      unsigned short gid = sig->Argument();

      for (int i = 0; i < myUserGroups.size(); ++i)
      {
        ContactGroup* group = myUserGroups.at(i);
        if (group->groupId() == gid)
        {
          beginRemoveRows(QModelIndex(), i, i);
          myUserGroups.removeAll(group);
          endRemoveRows();
          delete group;
        }
      }
      break;
    }

    case LIST_GROUP_CHANGED:
    {
      unsigned short gid = sig->Argument();

      for (int i = 0; i < myUserGroups.size(); ++i)
      {
        ContactGroup* group = myUserGroups.at(i);
        if (group->groupId() == gid)
          group->update();
      }
      break;
    }

    case LIST_GROUP_REORDERED:
    {
      // Get new sort keys for all groups
      for (int i = 0; i < myUserGroups.size(); ++i)
        myUserGroups.at(i)->updateSortKey();

      // Send one changed signal for all groups
      emit dataChanged(createIndex(0, 0, myUserGroups.at(0)),
          createIndex(myUserGroups.size() + NUM_GROUPS_SYSTEM_ALL - 1, myColumnCount - 1,
          mySystemGroups[NUM_GROUPS_SYSTEM_ALL-1]));

      break;
    }
  }
}

void ContactListModel::userUpdated(CICQSignal* sig)
{
  // Skip events for owners
  if (gUserManager.FindOwner(sig->Id(), sig->PPID()) != NULL)
    return;

  ContactUserData* user = findUser(sig->Id(), sig->PPID());
  if (user == NULL)
  {
    char* ppidString = PPIDSTRING(sig->PPID());
    gLog.Warn("%sContactList::userUpdated(): Invalid user received: %s (%s)\n",
        L_ERRORxSTR, sig->Id(), ppidString);
    delete[] ppidString;
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

  const ICQUser* u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);
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

void ContactListModel::userDataChanged(const ContactUserData* user)
{
  if (myBlockUpdates)
    return;

  // Emit signal that the user has changed in all groups
  foreach (ContactUser* u, user->groupList())
  {
    int row = u->group()->indexOf(u);
    emit dataChanged(createIndex(row, 0, u), createIndex(row, myColumnCount - 1, u));
  }
}

void ContactListModel::groupDataChanged(ContactGroup* group)
{
  if (myBlockUpdates)
    return;

  int row = groupRow(group);
  emit dataChanged(createIndex(row, 0, group), createIndex(row, myColumnCount - 1, group));
}

void ContactListModel::barDataChanged(ContactBar* bar, int row)
{
  if (myBlockUpdates)
    return;

  emit dataChanged(createIndex(row, 0, bar), createIndex(row, myColumnCount - 1, bar));
}

void ContactListModel::groupBeginInsert(ContactGroup* group, int row)
{
  beginInsertRows(createIndex(groupRow(group), 0, group), row, row);
}

void ContactListModel::groupEndInsert()
{
  endInsertRows();
}

void ContactListModel::groupBeginRemove(ContactGroup* group, int row)
{
  beginRemoveRows(createIndex(groupRow(group), 0, group), row, row);
}

void ContactListModel::groupEndRemove()
{
  endRemoveRows();
}

void ContactListModel::reloadAll()
{
  // Don't send out signals while reloading, the reset at the end will be enough
  myBlockUpdates = true;

  // Clear the list of all old groups and users
  clear();
  myColumnCount = Config::ContactList::instance()->columnCount();

  // Add all groups
  ContactGroup* newGroup = new ContactGroup(0, tr("Other Users"));
  connectGroup(newGroup);
  myUserGroups.append(newGroup);

  FOR_EACH_GROUP_START(LOCK_R)
  {
    ContactGroup* group = new ContactGroup(pGroup);
    connectGroup(group);
    myUserGroups.append(group);
  }
  FOR_EACH_GROUP_END

  // Add all users
  FOR_EACH_USER_START(LOCK_R)
  {
    addUser(pUser);
  }
  FOR_EACH_USER_END

  // Tell views that we have done major changes
  myBlockUpdates = false;
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

int ContactListModel::groupRow(ContactGroup* group) const
{
  unsigned short groupId = group->groupId();

  if (groupId < SystemGroupOffset)
    return myUserGroups.indexOf(group);
  else if (groupId <= SystemGroupOffset + NUM_GROUPS_SYSTEM_ALL)
    return myUserGroups.size() + groupId - SystemGroupOffset;
  else
    return -1;
}

void ContactListModel::addUser(const ICQUser* licqUser)
{
  ContactUserData* newUser = new ContactUserData(licqUser, this);
  connect(newUser, SIGNAL(dataChanged(const ContactUserData*)),
      SLOT(userDataChanged(const ContactUserData*)));
  connect(newUser, SIGNAL(updateUserGroups(ContactUserData*, const ICQUser*)),
      SLOT(updateUserGroups(ContactUserData*, const ICQUser*)));

  myUsers.append(newUser);
  updateUserGroups(newUser, licqUser);
}

void ContactListModel::updateUserGroups(ContactUserData* user, const ICQUser* licqUser)
{
  // Check which user groups the user should be member of
  for (int i = 0; i < myUserGroups.size(); ++i)
  {
    ContactGroup* group = myUserGroups.at(i);
    unsigned short gid = group->groupId();
    bool shouldBeMember = (gid != 0 && licqUser->GetInGroup(GROUPS_USER, gid)) ||
        (gid == 0 && licqUser->GetGroups().empty() && !licqUser->IgnoreList());
    updateUserGroup(user, group, shouldBeMember);
  }

  // Check which system groups the user should be member of
  for (unsigned long i = 0; i < NUM_GROUPS_SYSTEM_ALL; ++i)
  {
    updateUserGroup(user, mySystemGroups[i], licqUser->GetInGroup(GROUPS_SYSTEM, i));
  }
}

void ContactListModel::updateUserGroup(ContactUserData* user, ContactGroup* group, bool shouldBeMember)
{
  ContactUser* member = group->user(user);

  // If membership is already correct then there is nothing to do
  if ((member != 0) == shouldBeMember)
    return;

  // Adding or removing the user is enough here, signals will be sent from group
  if (shouldBeMember)
    new ContactUser(user, group);
  else
    delete member;
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
    beginRemoveRows(createIndex(groupRow(group), 0, group), pos, pos);
    delete u;
    endRemoveRows();
  }

  myUsers.removeAll(user);
  delete user;
}

void ContactListModel::clear()
{
  // Clear all users and user groups.
  // System groups and their bars are never removed.
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
  return createIndex(groupRow(group), 0, group);
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

  Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  ItemType itemType = static_cast<ContactItem*>(index.internalPointer())->itemType();

  // Only editing of alias for contacts
  if (itemType == UserItem && Config::ContactList::instance()->columnFormat(index.column()) == "%a")
    f |= Qt::ItemIsEditable;

  // Group names are editable in first column unless it's a system group
  if (itemType == GroupItem && index.column() == 0 && index.row() != 0 && index.row() < myUserGroups.size())
    f |= Qt::ItemIsEditable;

  return f;
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
  else if (type == GROUPS_USER)
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

QModelIndex ContactListModel::groupIndex(unsigned long id) const
{
  if (id >= SystemGroupOffset)
    return groupIndex(GROUPS_SYSTEM, id - SystemGroupOffset);
  else
    return groupIndex(GROUPS_USER, id);
}

bool ContactListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  return static_cast<ContactItem*>(index.internalPointer())->setData(value, role);
}
