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

#include "contactgroup.h"

#include <licq_user.h>

#include "contactbar.h"
#include "contactuser.h"

using namespace LicqQtGui;

ContactGroup::ContactGroup(unsigned short id, QString name)
  : ContactItem(ContactListModel::GroupItem),
    myGroupId(id),
    myName(name),
    myEvents(0),
    myVisibleContacts(0)
{
  if (myGroupId != 0)
    mySortKey = myGroupId;
  else
    // Put "Other Users" last when sorting
    mySortKey = 65535;

  update();

  for (int i = 0; i < 3; ++i)
    myBars[i] = new ContactBar(static_cast<ContactListModel::SubGroupType>(i), this);
}

ContactGroup::ContactGroup(const LicqGroup* group)
  : ContactItem(ContactListModel::GroupItem),
    myGroupId(group->id()),
    myName(QString::fromLocal8Bit(group->name().c_str())),
    mySortKey(group->sortIndex()),
    myEvents(0),
    myVisibleContacts(0)
{
  for (int i = 0; i < 3; ++i)
    myBars[i] = new ContactBar(static_cast<ContactListModel::SubGroupType>(i), this);
}

ContactGroup::~ContactGroup()
{
  // Remove all user instances in this group
  while (!myUsers.isEmpty())
    delete myUsers.takeFirst();

  for (int i = 0; i < 3; ++i)
    delete myBars[i];
}

void ContactGroup::update()
{
  // System groups and "Other users" aren't present in daemon group list
  if (myGroupId == 0 || myGroupId >= ContactListModel::SystemGroupOffset)
    return;

  LicqGroup* g = gUserManager.FetchGroup(myGroupId, LOCK_R);
  if (g == NULL)
    return;

  myName = QString::fromLocal8Bit(g->name().c_str());
  mySortKey = g->sortIndex();
  gUserManager.DropGroup(g);

  emit dataChanged(this);
}

void ContactGroup::updateSortKey()
{
  // System groups and "Other users" aren't present in daemon group list
  if (myGroupId == 0 || myGroupId >= ContactListModel::SystemGroupOffset)
    return;

  LicqGroup* g = gUserManager.FetchGroup(myGroupId, LOCK_R);
  if (g == NULL)
    return;

  mySortKey = g->sortIndex();
  gUserManager.DropGroup(g);
}

ContactItem* ContactGroup::item(int row) const
{
  if (row < 3)
    return myBars[row];
  else
    return myUsers.value(row - 3);
}

ContactUser* ContactGroup::user(ContactUserData* u) const
{
  foreach (ContactUser* instance, myUsers)
  {
    if (instance->userData() == u)
      return instance;
  }

  return 0;
}

int ContactGroup::rowCount() const
{
  // Add the separator bars
  return myUsers.size() + 3;
}

int ContactGroup::indexOf(ContactUser* user) const
{
  // The separator bars come first so add three to the index
  return myUsers.indexOf(user) + 3;
}

void ContactGroup::addUser(ContactUser* user, ContactListModel::SubGroupType subGroup)
{
  // Signal that we are about to add a row
  emit beginInsert(this, rowCount());

  myUsers.append(user);
  if (user->visibility())
    myVisibleContacts++;
  myBars[subGroup]->countIncrease();
  myEvents += user->numEvents();
  myBars[subGroup]->updateNumEvents(user->numEvents());

  // Signal that we're done adding
  emit endInsert();

  // Update group and bar as counters may have changed
  emit barDataChanged(myBars[subGroup], subGroup);
  emit dataChanged(this);
}

void ContactGroup::removeUser(ContactUser* user, ContactListModel::SubGroupType subGroup)
{
  // Signal that we are about to remove a row
  emit beginRemove(this, indexOf(user));

  myUsers.removeAll(user);
  if (user->visibility())
    myVisibleContacts--;
  myBars[subGroup]->countDecrease();
  myEvents -= user->numEvents();
  myBars[subGroup]->updateNumEvents(-user->numEvents());

  // Signal that we're done removing
  emit endRemove();

  // Update group and bar as counters may have changed
  emit barDataChanged(myBars[subGroup], subGroup);
  emit dataChanged(this);
}

void ContactGroup::updateSubGroup(ContactListModel::SubGroupType oldSubGroup, ContactListModel::SubGroupType newSubGroup, int eventCounter)
{
  myBars[oldSubGroup]->countDecrease();
  myBars[oldSubGroup]->updateNumEvents(-eventCounter);
  emit barDataChanged(myBars[oldSubGroup], oldSubGroup);

  myBars[newSubGroup]->countIncrease();
  myBars[newSubGroup]->updateNumEvents(eventCounter);
  emit barDataChanged(myBars[newSubGroup], newSubGroup);
}

void ContactGroup::updateNumEvents(int counter, ContactListModel::SubGroupType subGroup)
{
  if (counter == 0)
    return;

  myEvents += counter;
  myBars[subGroup]->updateNumEvents(counter);

  emit dataChanged(this);
}

void ContactGroup::updateVisibility(bool increase)
{
  if (increase)
    myVisibleContacts++;
  else
    myVisibleContacts--;

  emit dataChanged(this);
}

QVariant ContactGroup::data(int column, int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
      if (column == 0)
      {
        int onlineCount = myBars[ContactListModel::OnlineSubGroup]->count();
        if (onlineCount > 0)
          return myName + " (" + QString::number(onlineCount) + ")";
        else
          return myName;
      }
      break;

    case ContactListModel::NameRole:
      return myName;

    case ContactListModel::ItemTypeRole:
      return ContactListModel::GroupItem;

    case ContactListModel::SortPrefixRole:
      return 0;

    case ContactListModel::SortRole:
      return mySortKey;

    case ContactListModel::UnreadEventsRole:
      return myEvents;

    case ContactListModel::GroupIdRole:
      return myGroupId;

    case ContactListModel::UserCountRole:
      return myUsers.size();

    case ContactListModel::VisibilityRole:
      return (myVisibleContacts > 0);
  }

  return QVariant();
}

bool ContactGroup::setData(const QVariant& value, int role)
{
  if (role != ContactListModel::NameRole || !value.isValid())
    return false;

  // Don't allow system groups or "Other users" to be renamed this way
  if (myGroupId == 0 || myGroupId >= ContactListModel::SystemGroupOffset)
    return false;

  QString newName = value.toString();
  if (newName == myName)
    return true;

  // Don't save new name here, daemon will signal us when name has changed
  gUserManager.RenameGroup(myGroupId, newName.toLocal8Bit().data());

  return true;
}
