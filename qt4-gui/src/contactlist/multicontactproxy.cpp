// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2009 Licq developers
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

#include "multicontactproxy.h"

using namespace LicqQtGui;
using std::set;


MultiContactProxy::MultiContactProxy(ContactListModel* contactList, QObject* parent)
  : SortedContactListProxy(contactList, parent)
{
}

void MultiContactProxy::clear()
{
  myContacts.clear();
  invalidateFilter();
}

void MultiContactProxy::add(const UserId& userId)
{
  myContacts.insert(userId);
  invalidateFilter();
}

void MultiContactProxy::remove(const QModelIndexList& indexes)
{
  foreach (QModelIndex i, indexes)
  {
    UserId userId = i.data(ContactListModel::UserIdRole).value<UserId>();
    myContacts.erase(userId);
  }
  invalidateFilter();
}

void MultiContactProxy::remove(const UserId& userId)
{
  myContacts.erase(userId);
  invalidateFilter();
}

void MultiContactProxy::crop(const QModelIndexList& indexes)
{
  // Make a new set with the contacts to keep
  set<UserId> newList;

  foreach (QModelIndex i, indexes)
  {
    UserId userId = i.data(ContactListModel::UserIdRole).value<UserId>();
    newList.insert(userId);
  }

  // Activate the new cropped list
  myContacts = newList;
  invalidateFilter();
}

void MultiContactProxy::addGroup(GroupType groupType, unsigned long groupId)
{
  QModelIndex groupIndex = dynamic_cast<ContactListModel*>(sourceModel())->groupIndex(groupType, groupId);
  int numUsers = sourceModel()->rowCount(groupIndex);
  for (int i = 0; i < numUsers; ++i)
  {
    QModelIndex userIndex = sourceModel()->index(i, 0, groupIndex);

    if (static_cast<ContactListModel::ItemType>(userIndex.data(ContactListModel::ItemTypeRole).toInt()) == ContactListModel::UserItem)
    {
      UserId userId = userIndex.data(ContactListModel::UserIdRole).value<UserId>();
      myContacts.insert(userId);
    }
  }
  invalidateFilter();
}

QModelIndex MultiContactProxy::rootIndex() const
{
  return mapFromSource(dynamic_cast<ContactListModel*>(sourceModel())->groupIndex(GROUPS_SYSTEM, 0));
}

bool MultiContactProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  QModelIndex item = sourceModel()->index(source_row, 0, source_parent);

  switch (static_cast<ContactListModel::ItemType>(item.data(ContactListModel::ItemTypeRole).toInt()))
  {
    case ContactListModel::GroupItem:
    {
      // We only want the "All users" group
      if (item.data(ContactListModel::GroupIdRole).toInt() != ContactListModel::SystemGroupOffset)
        return false;

      break;
    }
    case ContactListModel::UserItem:
    {
      // Check if the contact is in our list
      UserId userId = item.data(ContactListModel::UserIdRole).value<UserId>();
      if (myContacts.count(userId) == 0)
        return false;

      break;
    }
    case ContactListModel::BarItem:
      return false;

    default:
      // Unknown item type so we probably don't want to show it
      return false;
  }
  return true;
}
