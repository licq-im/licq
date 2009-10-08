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

#include "mode2contactlistproxy.h"

#include "contactbar.h"
#include "contactlist.h"
#include "contactgroup.h"

using namespace LicqQtGui;


Mode2ContactListProxy::Mode2ContactListProxy(ContactListModel* contactList, QObject* parent)
  : QAbstractProxyModel(parent)
{
  setSourceModel(contactList);

  // Initial update
  reset();

  connect(contactList, SIGNAL(rowsAboutToBeInserted(const QModelIndex&, int, int)),
      SLOT(sourceRowsAboutToBeInserted(const QModelIndex&, int, int)));
  connect(contactList, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
      SLOT(sourceRowsAboutToBeRemoved(const QModelIndex&, int, int)));
  connect(contactList, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
      SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
  connect(contactList, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
      SLOT(sourceRowsRemoved(const QModelIndex&, int, int)));
  connect(contactList, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
      SLOT(sourceDataChanged(const QModelIndex&, const QModelIndex&)));
  connect(contactList, SIGNAL(modelReset()), SLOT(reset()));
  connect(contactList, SIGNAL(layoutChanged()), SLOT(reset()));
  // Column signals are not needed since source model also emit layoutChanged for those
}

Mode2ContactListProxy::~Mode2ContactListProxy()
{
  // Delete our local groups
  while (!myGroups.isEmpty())
    delete myGroups.takeFirst();
}

void Mode2ContactListProxy::reset()
{
  // Delete our local groups
  while (!myGroups.isEmpty())
    delete myGroups.takeFirst();

  // User items are owned by model so only clear the map, not the user objects
  myUserData.clear();

  myColumnCount = sourceModel()->columnCount();

  // Get bars from All Users system group
  QModelIndex allUsersIndex = dynamic_cast<ContactListModel*>(sourceModel())->groupIndex(ContactListModel::SystemGroupOffset + GROUP_ALL_USERS);
  for (int i = 0; i < NumBars; ++i)
    myBars[i] = static_cast<ContactBar*>(allUsersIndex.child(i, 0).internalPointer());

  int groupCount = sourceModel()->rowCount();

  for (int srcGroupRow = 0; srcGroupRow < groupCount; ++srcGroupRow)
  {
    QModelIndex groupIndex = sourceModel()->index(srcGroupRow, 0);

    // Skip the system groups since they'll be filtered by the next proxy anyway
    if (groupIndex.data(ContactListModel::GroupIdRole).toInt() >= ContactListModel::SystemGroupOffset)
      continue;

    addGroup(groupIndex);
  }
  QAbstractProxyModel::reset();
}

void Mode2ContactListProxy::sourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  switch (static_cast<ContactItem*>(topLeft.internalPointer())->itemType())
  {
    case ContactListModel::GroupItem:
    {
      if (topLeft.data(ContactListModel::GroupIdRole).toInt() >= ContactListModel::SystemGroupOffset)
        // Only system groups have changed, we don't have them so ignore signal
        return;

      // If signal includes system groups, make sure forwarded signal only contains normal groups
      int lastRow = bottomRight.row() * 2 + 1;
      if (lastRow >= myGroups.size())
        lastRow = myGroups.size() - 1;

      // One or more groups have changed, emit signal for both online and offline proxy groups
      emit dataChanged(createIndex(topLeft.row() * 2 + NumBars, topLeft.column(), myGroups.at(topLeft.row() * 2)),
          createIndex(lastRow + NumBars, bottomRight.column(), myGroups.at(lastRow)));
      return;
    }
    case ContactListModel::UserItem:
    {
      // Contact list model currently never emits dataChanged for multiple users at the same time
      Q_ASSERT(topLeft.row() == bottomRight.row());

      ContactUser* cu = static_cast<ContactUser*>(topLeft.internalPointer());

      // If user isn't in map, it's in a system group, which we don't care about, so just ignore signal
      if (!myUserData.contains(cu))
        return;

      int groupRow = myUserData[cu].groupRow;
      bool wasOnline = ((groupRow & 1) == 0);

      bool isOnline = (topLeft.data(ContactListModel::StatusRole) != ContactListModel::OfflineStatus);
      if (isOnline == wasOnline)
      {
        // Status hasn't changed, just forward signal with correct row in the proxy model
        int row = myUserData[cu].proxyRow;
        emit dataChanged(createIndex(row, topLeft.column(), cu), createIndex(row, bottomRight.column(), cu));

        bool emitGroupChanged = false;

        bool isVisible = topLeft.data(ContactListModel::VisibilityRole).toBool();
        if (isVisible != myUserData[cu].isVisible)
        {
          // Visibility has changed, update group
          myGroups.at(groupRow)->updateVisibility(isVisible ? 1 : -1);
          emitGroupChanged = true;
          myUserData[cu].isVisible = isVisible;
        }

        int unreadEvents = topLeft.data(ContactListModel::UnreadEventsRole).toInt();
        if (unreadEvents != myUserData[cu].unreadEvents)
        {
          // Unread events counter has changed, update group
          myGroups.at(groupRow)->updateEvents(unreadEvents - myUserData[cu].unreadEvents);
          emitGroupChanged = true;
          myUserData[cu].unreadEvents = unreadEvents;
        }

        if (emitGroupChanged)
          emit dataChanged(createIndex(groupRow + NumBars, 0, myGroups.at(groupRow)),
              createIndex(groupRow + NumBars, myColumnCount-1, myGroups.at(groupRow)));

        return;
      }

      // Status has changed, remove from current group and add to other group
      removeUser(cu);
      addUser(topLeft);
      return;
    }
    case ContactListModel::BarItem:
      // Contact list model currently never emits dataChanged for multiple bars at the same time
      Q_ASSERT(topLeft.row() == bottomRight.row());

      // The only bars we care about are the ones we're using from All Users group
      if (topLeft.internalPointer() == myBars[0])
        emit dataChanged(createIndex(0, 0, myBars[0]), createIndex(0, myColumnCount-1, myBars[0]));
      if (topLeft.internalPointer() == myBars[1])
        emit dataChanged(createIndex(1, 0, myBars[1]), createIndex(1, myColumnCount-1, myBars[1]));
      return;

    default:
    { } // Silence compiler warning
  }
}

void Mode2ContactListProxy::addUser(const QModelIndex& userIndex, bool emitSignals)
{
  // Get data for the user we want to add
  ContactUser* cu = static_cast<ContactUser*>(userIndex.internalPointer());
  bool isOnline = (userIndex.data(ContactListModel::StatusRole) != ContactListModel::OfflineStatus);
  bool isVisible = userIndex.data(ContactListModel::VisibilityRole).toBool();
  int unreadEvents = userIndex.data(ContactListModel::UnreadEventsRole).toInt();

  QModelIndex parent = sourceModel()->parent(userIndex);
  int groupRow = parent.row() * 2 + (isOnline ? 0 : 1);

  // Find next free row number to place user at
  int proxyRow = 0;
  QMap<ContactUser*, Mode2ProxyUserData>::const_iterator i;
  for (i = myUserData.begin(); i != myUserData.end(); ++i)
    if (i.value().groupRow == groupRow && i.value().proxyRow >= proxyRow)
      proxyRow = i.value().proxyRow + 1;

  // Add the user to the correct group and notify view
  if (emitSignals)
    beginInsertRows(createIndex(groupRow+NumBars, 0, myGroups.at(groupRow)), proxyRow, proxyRow);
  myUserData[cu].sourceRow = userIndex.row();
  myUserData[cu].proxyRow = proxyRow;
  myUserData[cu].groupRow = groupRow;
  myUserData[cu].unreadEvents = unreadEvents;
  myUserData[cu].isVisible = isVisible;
  myGroups[groupRow]->updateUserCount(1);
  myGroups[groupRow]->updateEvents(unreadEvents);
  if (isVisible)
    myGroups[groupRow]->updateVisibility(1);
  if (emitSignals)
    endInsertRows();

  // Group counters have changed
  if (emitSignals)
    emit dataChanged(createIndex(groupRow+NumBars, 0, myGroups.at(groupRow)),
        createIndex(groupRow+NumBars, myColumnCount-1, myGroups.at(groupRow)));
}

void Mode2ContactListProxy::removeUser(ContactUser* cu)
{
  // Don't try to remove what we don't have, i.e. ignore system groups
  if (!myUserData.contains(cu))
    return;

  int groupRow = myUserData[cu].groupRow;
  int proxyRow = myUserData[cu].proxyRow;

  beginRemoveRows(createIndex(groupRow+NumBars, 0, myGroups.at(groupRow)), proxyRow, proxyRow);

  // Remove user from map and update group counters
  myGroups[groupRow]->updateEvents(-myUserData[cu].unreadEvents);
  if (myUserData[cu].isVisible)
    myGroups[groupRow]->updateVisibility(-1);
  myUserData.remove(cu);
  myGroups[groupRow]->updateUserCount(-1);

  // Update rows for all following users in the same group
  QMap<ContactUser*, Mode2ProxyUserData>::iterator i;
  for (i = myUserData.begin(); i != myUserData.end(); ++i)
    if (i.value().groupRow == groupRow && i.value().proxyRow > proxyRow)
      --i.value().proxyRow;

  endRemoveRows();

  // Group counters have changed
  emit dataChanged(createIndex(groupRow+NumBars, 0, myGroups.at(groupRow)),
      createIndex(groupRow+NumBars, myColumnCount-1, myGroups.at(groupRow)));
}

void Mode2ContactListProxy::addGroup(const QModelIndex& groupIndex)
{
  int groupRow = groupIndex.row();

  // Add online and offline groups for new group
  ContactGroup* cg = static_cast<ContactGroup*>(groupIndex.internalPointer());
  myGroups.insert(groupRow * 2, new ContactProxyGroup(cg, true));
  myGroups.insert(groupRow * 2 + 1, new ContactProxyGroup(cg, false));

  // Map all the users
  int userCount = sourceModel()->rowCount(groupIndex);
  for (int userRow = 0; userRow < userCount; ++userRow)
  {
    QModelIndex userIndex = sourceModel()->index(userRow, 0, groupIndex);

    // Don't map the bars
    if (static_cast<ContactItem*>(userIndex.internalPointer())->itemType() != ContactListModel::UserItem)
      continue;

    addUser(userIndex, false);
  }
}

void Mode2ContactListProxy::sourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end)
{
  // For groups, we add two new entries for every source group
  if (!parent.isValid())
  {
    beginInsertRows(QModelIndex(), start * 2 + NumBars, end * 2 + NumBars + 1);
    // We can't actually insert the group until we get rowsInserted
    return;
  }

  // Don't do anything with users until we get rowsInserted signal
}

void Mode2ContactListProxy::sourceRowsInserted(const QModelIndex& parent, int start, int end)
{
  if (!parent.isValid())
  {
    // Update users data with new group rows
    QMap<ContactUser*, Mode2ProxyUserData>::iterator i;
    for (i = myUserData.begin(); i != myUserData.end(); ++i)
      if (i.value().groupRow >= end*2)
        i.value().groupRow += 2*(end - start + 1);

    for (int row = start; row <= end; ++row)
      addGroup(sourceModel()->index(row, 0));

    endInsertRows();
    return;
  }

  // New user(s) added, add them to appropriate group (unless it was in a system group)
  if (parent.data(ContactListModel::GroupIdRole).toInt() < ContactListModel::SystemGroupOffset)
    for (int row = start; row <= end; ++row)
      addUser(sourceModel()->index(row, 0, parent));
}

void Mode2ContactListProxy::sourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
  // For groups, we'll have to remove both our local proxy groups
  if (!parent.isValid())
  {
    beginRemoveRows(QModelIndex(), start * 2 + NumBars, end * 2 + NumBars + 1);

    // Remove the proxy groups
    for (int row = end; row >= start; --row)
    {
      delete myGroups.takeAt(row * 2 + 1);
      delete myGroups.takeAt(row * 2);
    }

    // Remove user data for the removed groups and adjust groupRow for later groups
    QMap<ContactUser*, Mode2ProxyUserData>::iterator i;
    for (i = myUserData.begin(); i != myUserData.end(); ++i)
    {
      if (i.value().groupRow/2 > end)
        i.value().groupRow -= 2*(end - start + 1);
      else if (i.value().groupRow/2 >= start && i.value().groupRow/2 <= end)
      {
        // User for the removed group, remove it but make sure iterator isn't broken
        QMap<ContactUser*, Mode2ProxyUserData>::iterator j = i;
        --i;
        myUserData.erase(j);
      }
    }

    // Don't emit rowsRemoved until source model has also removed the group
    return;
  }

  // Remove users from proxy groups
  for (int row = start; row <= end; ++row)
    removeUser(static_cast<ContactUser*>(sourceModel()->index(row, 0, parent).internalPointer()));
}

void Mode2ContactListProxy::sourceRowsRemoved(const QModelIndex& parent, int start, int end)
{
  Q_UNUSED(start);
  Q_UNUSED(end);

  if (!parent.isValid())
  {
    // Groups already removed as part of rowsAboutToBeRemoved, only remains to emit signal
    endRemoveRows();
    return;
  }

  // Users already removed as part of rowsAboutToBeRemoved
}

QModelIndex Mode2ContactListProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid())
    return QModelIndex();

  switch (static_cast<ContactItem*>(sourceIndex.internalPointer())->itemType())
  {
    case ContactListModel::GroupItem:
      // Groups are listed twice, group from source maps to online group in proxy
      // but we have no mapping for system groups
      if (sourceIndex.row() * 2 < myGroups.size())
        return createIndex(sourceIndex.row() * 2 + NumBars, sourceIndex.column(), sourceIndex.internalPointer());

    case ContactListModel::UserItem:
    {
      // Find proxy row for the source user
      ContactUser* cu = static_cast<ContactUser*>(sourceIndex.internalPointer());
      if (myUserData.contains(cu))
        return createIndex(myUserData[cu].proxyRow, sourceIndex.column(), cu);
      break;
    }

    case ContactListModel::BarItem:
      // The only bars we handle are the ones from All Users group
      for (int i = 0; i < NumBars; ++i)
        if (sourceIndex.internalPointer() == myBars[i])
          return createIndex(i, sourceIndex.column(), myBars[i]);
      break;

    default:
    { } // Silence compiler warning
  }

  return QModelIndex();
}

QModelIndex Mode2ContactListProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  // TODO: This function uses createIndex for the proxy but returned modelindexes should belong to source model

  if (!proxyIndex.isValid())
    return QModelIndex();

  switch (static_cast<ContactItem*>(proxyIndex.internalPointer())->itemType())
  {
    case ContactListModel::GroupItem:
    {
      // Both proxy groups maps back to source group
      const ContactProxyGroup* g = static_cast<ContactProxyGroup*>(proxyIndex.internalPointer());
      return createIndex((proxyIndex.row() - NumBars) / 2, proxyIndex.column(), g->sourceGroup());
    }
    case ContactListModel::UserItem:
    {
      // Find source row for the proxy user
      ContactUser* cu = static_cast<ContactUser*>(proxyIndex.internalPointer());
      if (myUserData.contains(cu))
        return createIndex(myUserData[cu].sourceRow, proxyIndex.column(), cu);
    }

    case ContactListModel::BarItem:
      // The only bars we handle are the ones from All Users group
      for (int i = 0; i < NumBars; ++i)
        if (proxyIndex.internalPointer() == myBars[i])
          return createIndex(i, proxyIndex.column(), myBars[i]);
      break;

    default:
    { } // Silence compiler warning
  }

  return QModelIndex();
}

QVariant Mode2ContactListProxy::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  // Since contact list model only uses internalPointer we can bypass the proxy
  //   mapping and just call the data objects directly.
  return static_cast<ContactItem*>(index.internalPointer())->data(index.column(), role);
}

QModelIndex Mode2ContactListProxy::index(int row, int column, const QModelIndex& parent) const
{
  if (row < 0 || column < 0 || column >= myColumnCount)
    return QModelIndex();

  // Top level items, i.e. bars and groups
  if (!parent.isValid())
  {
    if (row < NumBars)
      return createIndex(row, column, myBars[row]);

    if (row >= myGroups.size() + NumBars)
      return QModelIndex();

    return createIndex(row, column, myGroups.at(row-NumBars));
  }

  // Groups are the only items with sub items
  if (static_cast<ContactItem*>(parent.internalPointer())->itemType() != ContactListModel::GroupItem)
    return QModelIndex();

  // Find user data for the requested row
  int groupRow = parent.row() - NumBars;
  QMap<ContactUser*, Mode2ProxyUserData>::const_iterator i;
  for (i = myUserData.begin(); i != myUserData.end(); ++i)
    if (i.value().groupRow == groupRow && i.value().proxyRow == row)
      return createIndex(row, column, i.key());

  return QModelIndex();
}

QModelIndex Mode2ContactListProxy::parent(const QModelIndex& index) const
{
  // Invalid, bar or group index
  if (!index.isValid() ||
      static_cast<ContactItem*>(index.internalPointer())->itemType() != ContactListModel::UserItem)
    return QModelIndex();

  // User index, find parent group
  ContactUser* cu = static_cast<ContactUser*>(index.internalPointer());
  if (!myUserData.contains(cu))
    return QModelIndex();
  int groupRow = myUserData[cu].groupRow;
  return createIndex(groupRow+NumBars, 0, myGroups.at(groupRow));
}

int Mode2ContactListProxy::rowCount(const QModelIndex& parent) const
{
  // Top level, there is always the bars plus the all the of groups
  if (!parent.isValid())
    return myGroups.size() + NumBars;

  // Not a group so no sub items
  if (static_cast<ContactItem*>(parent.internalPointer())->itemType() != ContactListModel::GroupItem)
    return 0;

  if (parent.row() < NumBars || parent.row() >= myGroups.size()+NumBars)
    return 0;

  // Get row number for group
  return myGroups.at(parent.row()-NumBars)->userCount();
}

int Mode2ContactListProxy::columnCount(const QModelIndex& /* parent */) const
{
  // We can use our cached value since source model won't change without signalling us
  return myColumnCount;
}


ContactProxyGroup::ContactProxyGroup(ContactGroup* sourceGroup, bool isOnline)
  : ContactItem(ContactListModel::GroupItem),
    mySourceGroup(sourceGroup),
    myIsOnline(isOnline),
    myUserCount(0),
    myVisibleCount(0),
    myEventCount(0)
{
}

QVariant ContactProxyGroup::data(int column, int role) const
{
  // Override any roles that needs to be different from ContactGroup
  switch (role)
  {
    case Qt::DisplayRole:
    {
      QString name = mySourceGroup->data(column, ContactListModel::NameRole).toString();
      if (myUserCount)
        return name + " (" + QString::number(myUserCount) + ")";
      else
        return name;
    }
    case ContactListModel::SortPrefixRole:
      // Same sort prefixes used within groups, allows same sorting to work on top level
      return (myIsOnline ? 1 : 3);

    case ContactListModel::UserCountRole:
      return myUserCount;

    case ContactListModel::VisibilityRole:
      return (myVisibleCount > 0);

    case ContactListModel::UnreadEventsRole:
      return myEventCount;

    default:
      // Anything not handled here, forward to source model class
      return mySourceGroup->data(column, role);
  }
}
