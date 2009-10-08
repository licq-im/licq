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

#ifndef SORTEDCONTACTLISTPROXY_H
#define SORTEDCONTACTLISTPROXY_H

#include <QSortFilterProxyModel>

namespace LicqQtGui
{
class ContactListModel;

/**
 * Proxy for ContactListModel that keeps the list sorted
 *
 * The list is always sorted on the SortPrefix role and can secondarily be
 * sorted on any column and role.
 *
 * This class is primarily meant as a base class for other contact list proxies
 * as some filtering will also be needed before presenting the list in a view.
 */
class SortedContactListProxy : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param contactList The contact list to perform sorting on
   * @param parent Parent object
   */
  SortedContactListProxy(ContactListModel* contactList, QObject* parent = NULL);

  /**
   * Destructor
   */
  virtual ~SortedContactListProxy() {}

  /**
   * Convenience function to set sorting role and sort items
   *
   * @param column Column to sort on
   * @param role Item role to use for sorting
   * @param order Order to sort
   */
  void sort(int column = 0, int role = Qt::DisplayRole, Qt::SortOrder order = Qt::AscendingOrder);

  /**
   * Same sort as above but with same parameters as base class so we overload
   * it. Also this sort() has no default values to sovle problems when calls
   * to sort() would be ambiguous with base sort() accessable.
   *
   * @param column Column to sort on
   * @param order Order to sort
   */
  virtual void sort(int column, Qt::SortOrder order)
  { sort(column, Qt::DisplayRole, order); }

private:
  /**
   * Compare sorting order of two items
   *
   * @param left The first item to compare
   * @param right The second item to compare
   * @return True if the left item should be above the right item
   */
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

  Qt::SortOrder mySortOrder;
};

} // namespace LicqQtGui

#endif
