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

#include "sortedcontactlistproxy.h"

#include "contactlist.h"

using namespace LicqQtGui;

SortedContactListProxy::SortedContactListProxy(ContactListModel* contactList, QObject* parent)
  : QSortFilterProxyModel(parent),
    mySortOrder(Qt::AscendingOrder)
{
  setSourceModel(contactList);
  setSortCaseSensitivity(Qt::CaseInsensitive);
  setDynamicSortFilter(true);
}

void SortedContactListProxy::sort(int column, int role, Qt::SortOrder order)
{
  // To get bars and online/offline always in the same order we always tell the base class to do ascending sort
  setSortRole(role);
  mySortOrder = order;

  // The value of m_sortOrder affects the behaviour of our lessThan() method
  //   so tell QSortFilterProxyModel to forget any cached values from lessThan()
  invalidate();
  QSortFilterProxyModel::sort(column, Qt::AscendingOrder);
}

bool SortedContactListProxy::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  int prefixDiff = left.data(ContactListModel::SortPrefixRole).toInt() - right.data(ContactListModel::SortPrefixRole).toInt();

  // First sort on prefixes
  if (prefixDiff != 0)
    return (prefixDiff < 0);

  // Prefixes are same so use the normal sorting
  // Base class always sorts ascending so invert the value get descending sort
  if (mySortOrder == Qt::AscendingOrder)
    return QSortFilterProxyModel::lessThan(left, right);
  else
    return QSortFilterProxyModel::lessThan(right, left);
}
