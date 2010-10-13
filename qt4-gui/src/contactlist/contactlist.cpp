// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2010 Licq developers
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

#include <boost/foreach.hpp>
#include <cstring>

#include <QHash>

#include <licq/logging/log.h>
#include <licq/contactlist/group.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/pluginsignal.h>

#include "config/contactlist.h"

#include "contactbar.h"
#include "contactgroup.h"
#include "contactuser.h"
#include "contactuserdata.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ContactListModel */

ContactListModel* LicqQtGui::gGuiContactList = NULL;

QString ContactListModel::systemGroupName(int groupId)
{
  switch (groupId)
  {
    case OnlineNotifyGroupId:
      return tr("Online Notify");
    case VisibleListGroupId:
      return tr("Visible List");
    case InvisibleListGroupId:
      return tr("Invisible List");
    case IgnoreListGroupId:
      return tr("Ignore List");
    case NewUsersGroupId:
      return tr("New Users");
    case AwaitingAuthGroupId:
      return tr("Awaiting Authorization");

    case AllUsersGroupId:
    case MostUsersGroupId:
      return tr("All Users");
    case AllGroupsGroupId:
      return tr("All Groups (Threaded)");
  }
  return QString();
}

ContactListModel::ContactListModel(QObject* parent)
  : QAbstractItemModel(parent),
    myBlockUpdates(false)
{
  assert(gGuiContactList == NULL);
  gGuiContactList = this;

  ContactGroup* group;
#define CREATE_SYSTEMGROUP(gid, showMask, hideMask) \
  group = new ContactGroup(gid, systemGroupName(gid), showMask, hideMask); \
  connectGroup(group); \
  myGroups.append(group);

  CREATE_SYSTEMGROUP(AllUsersGroupId, 0, 0);
  myAllUsersGroup = group;
  CREATE_SYSTEMGROUP(MostUsersGroupId, 0, IgnoreStatus);
  CREATE_SYSTEMGROUP(OnlineNotifyGroupId, OnlineNotifyStatus, IgnoreStatus);
  CREATE_SYSTEMGROUP(VisibleListGroupId, VisibleListStatus, IgnoreStatus);
  CREATE_SYSTEMGROUP(InvisibleListGroupId, InvisibleListStatus, IgnoreStatus);
  CREATE_SYSTEMGROUP(IgnoreListGroupId, IgnoreStatus, 0);
  CREATE_SYSTEMGROUP(NewUsersGroupId, NewUserStatus, IgnoreStatus);
  CREATE_SYSTEMGROUP(AwaitingAuthGroupId, AwaitingAuthStatus, IgnoreStatus);
#undef CREATE_SYSTEMGROUP

  // reloadAll will compare column count to old value so must set an initial
  // value before calling
  myColumnCount = Config::ContactList::instance()->columnCount();

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
  while (!myUsers.isEmpty())
    delete myUsers.takeFirst();

  while (!myGroups.isEmpty())
    delete myGroups.takeFirst();

  gGuiContactList = NULL;
}

void ContactListModel::listUpdated(unsigned long subSignal, int argument, const Licq::UserId& userId)
{
  switch(subSignal)
  {
    case Licq::PluginSignal::ListInvalidate:
      reloadAll();
      break;

    case Licq::PluginSignal::ListUserAdded:
    {
      Licq::UserReadGuard u(userId);
      if (!u.isLocked())
      {
        Licq::gLog.warning("ContactList::listUpdated(): Invalid user received: %s",
            userId.toString().c_str());
        break;
      }
      addUser(*u);
      break;
    }
    case Licq::PluginSignal::ListUserRemoved:
      removeUser(userId);
      break;

    case Licq::PluginSignal::ListGroupAdded:
    {
      // argument is group id

      // Set inital expanded state for new group
      Config::ContactList::instance()->setGroupState(argument, true, true);
      Config::ContactList::instance()->setGroupState(argument, false, true);

      ContactGroup* newGroup = new ContactGroup(argument);
      connectGroup(newGroup);
      beginInsertRows(QModelIndex(), myGroups.size(), myGroups.size());
      myGroups.append(newGroup);
      endInsertRows();
      break;
    }

    case Licq::PluginSignal::ListGroupRemoved:
    {
      // argument is group id

      for (int i = 0; i < myGroups.size(); ++i)
      {
        ContactGroup* group = myGroups.at(i);
        if (group->groupId() == argument)
        {
          beginRemoveRows(QModelIndex(), i, i);
          myGroups.removeAll(group);
          endRemoveRows();
          delete group;
        }
      }
      break;
    }

    case Licq::PluginSignal::ListGroupChanged:
    {
      // argument is group id

      for (int i = 0; i < myGroups.size(); ++i)
      {
        ContactGroup* group = myGroups.at(i);
        if (group->groupId() == argument)
          group->update();
      }
      break;
    }

    case Licq::PluginSignal::ListGroupsReordered:
    {
      // Get new sort keys for all groups
      for (int i = 0; i < myGroups.size(); ++i)
        myGroups.at(i)->updateSortKey();

      // Send one changed signal for all groups
      emit dataChanged(createIndex(0, 0, myGroups.at(0)),
          createIndex(myGroups.size() - 1, myColumnCount - 1,
          myGroups.at(myGroups.size()-1)));

      break;
    }
  }
}

void ContactListModel::userUpdated(const Licq::UserId& userId, unsigned long subSignal, int argument)
{
  // Skip events for owners
  if (Licq::gUserManager.isOwner(userId))
    return;

  ContactUserData* user = findUser(userId);
  if (user == NULL)
  {
    Licq::gLog.warning("ContactList::userUpdated(): Invalid user received: %s",
        userId.toString().c_str());
    return;
  }

  // Forward signal to the ContactUserData object
  user->update(subSignal, argument);
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
  if (myBlockUpdates)
    return;

  beginInsertRows(createIndex(groupRow(group), 0, group), row, row);
}

void ContactListModel::groupEndInsert()
{
  if (myBlockUpdates)
    return;

  endInsertRows();
}

void ContactListModel::groupBeginRemove(ContactGroup* group, int row)
{
  if (myBlockUpdates)
    return;

  beginRemoveRows(createIndex(groupRow(group), 0, group), row, row);
}

void ContactListModel::groupEndRemove()
{
  if (myBlockUpdates)
    return;

  endRemoveRows();
}

void ContactListModel::reloadAll()
{
  // Don't send out signals while reloading, the reset at the end will be enough
  myBlockUpdates = true;

  // Clear all old users
  while (!myUsers.isEmpty())
    delete myUsers.takeFirst();

  // Clear old user groups, but keep the system groups
  QList<ContactGroup*>::iterator i;
  for (i = myGroups.begin(); i != myGroups.end(); )
  {
    if ((*i)->groupId() < SystemGroupOffset)
      i = myGroups.erase(i);
    else
      ++i;
  }

  // Make sure column count is correct
  configUpdated();

  // Add all groups
  ContactGroup* newGroup = new ContactGroup(OtherUsersGroupId, tr("Other Users"));
  connectGroup(newGroup);
  myGroups.append(newGroup);

  {
    Licq::GroupListGuard groupList;
    BOOST_FOREACH(const Licq::Group* g, **groupList)
    {
      Licq::GroupReadGuard pGroup(g);
      ContactGroup* group = new ContactGroup(*pGroup);
      connectGroup(group);
      myGroups.append(group);
    }
  }

  // Add all users
  {
    Licq::UserListGuard userList;
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      Licq::UserReadGuard u(user);
      addUser(*u);
    }
  }

  // Tell views that we have done major changes
  myBlockUpdates = false;

  reset();
}

ContactUserData* ContactListModel::findUser(const Licq::UserId& userId) const
{
  foreach (ContactUserData* user, myUsers)
  {
    if (user->userId() == userId)
      return user;
  }
  return 0;
}

int ContactListModel::groupRow(ContactGroup* group) const
{
  return myGroups.indexOf(group);
}

void ContactListModel::addUser(const Licq::User* licqUser)
{
  ContactUserData* newUser = new ContactUserData(licqUser, this);
  connect(newUser, SIGNAL(dataChanged(const ContactUserData*)),
      SLOT(userDataChanged(const ContactUserData*)));
  connect(newUser, SIGNAL(updateUserGroups(ContactUserData*, const Licq::User*)),
      SLOT(updateUserGroups(ContactUserData*, const Licq::User*)));

  myUsers.append(newUser);
  updateUserGroups(newUser, licqUser);
}

void ContactListModel::updateUserGroups(ContactUserData* user, const Licq::User* licqUser)
{
  // Check which user groups the user should be member of
  for (int i = 0; i < myGroups.size(); ++i)
  {
    ContactGroup* group = myGroups.at(i);
    int gid = group->groupId();
    bool shouldBeMember;
    if (!group->acceptUser(user->extendedStatus()))
      shouldBeMember = false;
    else if (gid >= SystemGroupOffset)
      shouldBeMember = true;
    else if (gid == OtherUsersGroupId)
      shouldBeMember = licqUser->GetGroups().empty();
    else if (gid > 0)
      shouldBeMember = licqUser->isInGroup(gid);
    else
      shouldBeMember = false;

    updateUserGroup(user, group, shouldBeMember);
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

void ContactListModel::removeUser(const Licq::UserId& userId)
{
  ContactUserData* user = findUser(userId);
  if (user == NULL)
    return;

  foreach (ContactUser* u, user->groupList())
  {
    // Only delete the contact here, the group will trigger signals to be emitted
    delete u;
  }

  myUsers.removeAll(user);
  delete user;
}

QModelIndex ContactListModel::index(int row, int column, const QModelIndex& parent) const
{
  if (row < 0 || column < 0 || column >= myColumnCount)
    return QModelIndex();

  // If root item is requested we return a group
  if (!parent.isValid())
  {
    ContactGroup* group;
    if (row < myGroups.size())
      group = myGroups.value(row);
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
    return myGroups.size();

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
    return 0;

  Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  ItemType itemType = static_cast<ContactItem*>(index.internalPointer())->itemType();

  // Only editing of alias for contacts
  if (itemType == UserItem && Config::ContactList::instance()->columnFormat(index.column()) == "%a")
    f |= Qt::ItemIsEditable;

  // Group names are editable in first column unless it's a system group
  if (itemType == GroupItem && index.column() == 0 &&
      index.row() >= 0 && index.row() < myGroups.size() &&
      myGroups[index.row()]->groupId() != OtherUsersGroupId &&
      myGroups[index.row()]->groupId() < SystemGroupOffset)
    f |= Qt::ItemIsEditable;

  return f;
}

QVariant ContactListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  // No vertical header here
  if (orientation != Qt::Horizontal)
    return QVariant();

  if (role == Qt::DisplayRole && section < myColumnCount)
    return Config::ContactList::instance()->columnHeading(section);

  return QVariant();
}

QModelIndex ContactListModel::userIndex(const Licq::UserId& userId, int column) const
{
  ContactUserData* userData = findUser(userId);
  if (userData != NULL)
  {
    // Find the user in the "All Users" group
    ContactUser* user = myAllUsersGroup->user(userData);
    if (user != NULL)
      return createIndex(myAllUsersGroup->indexOf(user), column, user);
  }

  return QModelIndex();
}

QModelIndex ContactListModel::groupIndex(int id) const
{
  for (int i = 0; i < myGroups.size(); ++i)
  {
    ContactGroup* group = myGroups.at(i);
    if (group->groupId() == id)
      return createIndex(i, 0, group);
  }

  return QModelIndex();
}

QString ContactListModel::groupName(int groupId) const
{
  for (int i = 0; i < myGroups.size(); ++i)
  {
    ContactGroup* group = myGroups.at(i);
    if (group->groupId() == groupId)
      return group->name();
  }

  return QString();
}

bool ContactListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid())
    return false;

  return static_cast<ContactItem*>(index.internalPointer())->setData(value, role);
}

uint qHash(const Licq::UserId& userId)
{
  return qHash(userId.toString().c_str());
}
