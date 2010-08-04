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

#include "singlecontactproxy.h"

#include "contactlist.h"

using namespace LicqQtGui;

SingleContactProxy::SingleContactProxy(ContactListModel* contactList, const Licq::UserId& userId, QObject* parent)
  : QAbstractProxyModel(parent),
    myContactList(contactList),
    myUserId(userId)
{
  connect(myContactList, SIGNAL(columnsInserted(const QModelIndex&, int, int)), SLOT(update()));
  connect(myContactList, SIGNAL(columnsRemoved(const QModelIndex&, int, int)), SLOT(update()));
  connect(myContactList, SIGNAL(layoutChanged()), SLOT(update()));
  connect(myContactList, SIGNAL(modelReset()), SLOT(update()));
  connect(myContactList, SIGNAL(rowsInserted(const QModelIndex&, int, int)), SLOT(update()));
  connect(myContactList, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SLOT(update()));
  update();
  connect(myContactList, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
      SLOT(slot_dataChanged(const QModelIndex&, const QModelIndex&)));
}

SingleContactProxy::~SingleContactProxy()
{
}

void SingleContactProxy::update()
{
  for (int i = 0; i < MAX_COLUMNCOUNT; ++i)
    mySourceIndex[i] = myContactList->userIndex(myUserId, i);
}

void SingleContactProxy::slot_dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  // Check if this affects us
  int groupId = topLeft.data(ContactListModel::GroupIdRole).toInt();
  if (groupId == ContactListModel::AllUsersGroupId)
    if (topLeft.row() <= mySourceIndex[0].row() && bottomRight.row() >= mySourceIndex[0].row())
      emit dataChanged(createIndex(0, 0, 0), createIndex(0, myContactList->columnCount() - 1, 0));
}

QModelIndex SingleContactProxy::index(int row, int column, const QModelIndex& parent) const
{
  // Only one top level item so return invalid for anything else
  if (!parent.isValid() && row == 0 && column >= 0 && column < MAX_COLUMNCOUNT)
    return createIndex(0, column, 0);

  return QModelIndex();
}

QModelIndex SingleContactProxy::parent(const QModelIndex& /* index */) const
{
  // Only one level so either parent is the root or request is invalid, same response in either case
  return QModelIndex();
}

int SingleContactProxy::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
    return 1;
  return 0;
}

int SingleContactProxy::columnCount(const QModelIndex& /* parent */) const
{
  return myContactList->columnCount(QModelIndex());
}

QVariant SingleContactProxy::data(const QModelIndex& index, int role) const
{
  if (index.isValid() && index.row() == 0 && index.column() >= 0 && index.column() < MAX_COLUMNCOUNT)
    return myContactList->data(mySourceIndex[index.column()], role);

  return QVariant();
}

Qt::ItemFlags SingleContactProxy::flags(const QModelIndex& index) const
{
  if (!index.isValid() || index.row() != 0 || index.column() < 0 || index.column() >= MAX_COLUMNCOUNT)
    return 0;

  return myContactList->flags(mySourceIndex[index.column()]);
}

QVariant SingleContactProxy::headerData(int section, Qt::Orientation orientation, int role) const
{
  return myContactList->headerData(section, orientation, role);
}

QModelIndex SingleContactProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (sourceIndex.column() < MAX_COLUMNCOUNT && sourceIndex == mySourceIndex[sourceIndex.column()])
    return createIndex(0, sourceIndex.column(), 0);

  return QModelIndex();
}

QModelIndex SingleContactProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.parent().isValid() && proxyIndex.row() == 0 && proxyIndex.column() < MAX_COLUMNCOUNT)
    return mySourceIndex[proxyIndex.column()];

  return QModelIndex();
}
