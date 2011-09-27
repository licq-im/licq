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

#include "contactgroup.h"

#include <licq/contactlist/group.h>
#include <licq/contactlist/usermanager.h>

#include "contactbar.h"
#include "contactlist.h"
#include "contactuser.h"

using namespace LicqQtGui;

ContactGroup::ContactGroup(int id, const QString& name, unsigned showMask, unsigned hideMask)
  : ContactItem(ContactListModel::GroupItem),
    myGroupId(id),
    myName(name),
    myEvents(0),
    myVisibleContacts(0),
    myShowMask(showMask),
    myHideMask(hideMask)
{
  // Put "Other Users" last when sorting
  if (myGroupId == ContactListModel::OtherUsersGroupId)
    mySortKey = 65535;
  else
    mySortKey = myGroupId;

  update();

  for (int i = 0; i < 3; ++i)
    myBars[i] = new ContactBar(static_cast<ContactListModel::SubGroupType>(i), this);
}

ContactGroup::ContactGroup(const Licq::Group* group)
  : ContactItem(ContactListModel::GroupItem),
    myGroupId(group->id()),
    myName(QString::fromLocal8Bit(group->name().c_str())),
    mySortKey(group->sortIndex()),
    myEvents(0),
    myVisibleContacts(0),
    myShowMask(0),
    myHideMask(ContactListModel::IgnoreStatus)
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
  if (myGroupId == ContactListModel::OtherUsersGroupId ||
      myGroupId >= ContactListModel::SystemGroupOffset)
    return;

  {
    Licq::GroupReadGuard g(myGroupId);
    if (!g.isLocked())
      return;

    myName = QString::fromLocal8Bit(g->name().c_str());
    mySortKey = g->sortIndex();
  }

  emit dataChanged(this);
}

void ContactGroup::updateSortKey()
{
  // System groups and "Other users" aren't present in daemon group list
  if (myGroupId == ContactListModel::OtherUsersGroupId ||
      myGroupId >= ContactListModel::SystemGroupOffset)
    return;

  Licq::GroupReadGuard g(myGroupId);
  if (!g.isLocked())
    return;

  mySortKey = g->sortIndex();
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
  // Insert user in model
  emit beginInsert(this, rowCount());
  myUsers.append(user);
  emit endInsert();

  // Update group data
  myEvents += user->numEvents();
  if (user->visibility())
    myVisibleContacts++;
  emit dataChanged(this);

  // Update bar data
  myBars[subGroup]->countIncrease();
  myBars[subGroup]->updateNumEvents(user->numEvents());
  if (user->visibility())
    myBars[subGroup]->updateVisibility(true);
  emit barDataChanged(myBars[subGroup], subGroup);
}

void ContactGroup::removeUser(ContactUser* user, ContactListModel::SubGroupType subGroup)
{
  // Update bar data
  myBars[subGroup]->countDecrease();
  myBars[subGroup]->updateNumEvents(-user->numEvents());
  if (user->visibility())
    myBars[subGroup]->updateVisibility(false);
  emit barDataChanged(myBars[subGroup], subGroup);

  // Remove user from model
  emit beginRemove(this, indexOf(user));
  myUsers.removeAll(user);
  emit endRemove();

  // Update group data
  myEvents -= user->numEvents();
  if (user->visibility())
    myVisibleContacts--;
  emit dataChanged(this);
}

bool ContactGroup::acceptUser(unsigned extendedStatus)
{
  // User must not match any bits in the hide mask
  if (myHideMask != 0 && (extendedStatus & myHideMask))
    return false;

  // User must match at least one bit in the show mask
  if (myShowMask != 0 && !(extendedStatus & myShowMask))
    return false;

  // Default, accept user
  return true;
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

  // Update bar data
  myBars[subGroup]->updateNumEvents(counter);
  emit barDataChanged(myBars[subGroup], subGroup);

  // Update group data
  myEvents += counter;
  emit dataChanged(this);
}

void ContactGroup::updateVisibility(bool increase, ContactListModel::SubGroupType subGroup)
{
  // Update bar data
  myBars[subGroup]->updateVisibility(increase);
  emit barDataChanged(myBars[subGroup], subGroup);

  // Update group data
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
  if (myGroupId == ContactListModel::OtherUsersGroupId ||
      myGroupId >= ContactListModel::SystemGroupOffset)
    return false;

  QString newName = value.toString();
  if (newName == myName)
    return true;

  // Don't save new name here, daemon will signal us when name has changed
  Licq::gUserManager.RenameGroup(myGroupId, newName.toLocal8Bit().constData());

  return true;
}
