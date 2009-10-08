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

#ifndef MULTICONTACTPROXY_H
#define MULTICONTACTPROXY_H

#include <licq_types.h>

#include <set>

#include "contactlist.h"
#include "sortedcontactlistproxy.h"


namespace LicqQtGui
{
class ContactListModel;

/**
 * Proxy for ContactListModel that presents a sorted list of custom contats
 *
 * This proxy presents only those contacts specified by the user. Used for
 * listing multiple recipients in the send dialog.
 * Sorting is inherited from SortedContactList.
 */
class MultiContactProxy : public SortedContactListProxy
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param contactList The contact list to use contacts from
   * @param parent Parent object
   */
  MultiContactProxy(ContactListModel* contactList, QObject* parent = NULL);

  /**
   * Destructor
   */
  virtual ~MultiContactProxy() {}

  /**
   * Clear the list
   */
  void clear();

  /**
   * Add a contact to the list
   *
   * @param userId User id
   */
  void add(const UserId& userId);

  /**
   * Remove a contact from the list
   *
   * @param userId User id
   */
  void remove(const UserId& userId);

  /**
   * Remove a list of contacts from the list
   *
   * @param indexes Indexes of the contacts to remove
   */
  void remove(const QModelIndexList& indexes);

  /**
   * Crop the list to containt only the specified list
   *
   * @param indexes Indexes of the contacts to keep
   */
  void crop(const QModelIndexList& indexes);

  /**
   * Add all contacts from a group
   *
   * @param groupType System or user group
   * @param groupId Group id
   */
  void addGroup(GroupType groupType, unsigned long groupId);

  /**
   * Return the current list of contacts
   *
   * @return A set containing the contacts
   */
  const std::set<UserId>& contacts() const
  { return myContacts; }

  /**
   * Get root index the view should use
   *
   * @return Root index
   */
  QModelIndex rootIndex() const;

private:
  /**
   * Check if an item should be present in the list or if it should be hidden
   * from the view
   *
   * @param source_row A row in the source model to test
   * @param source_parent Parent item for the row to test
   * @return True if the item should be visible in the view
   */
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

  std::set<UserId> myContacts;
};

} // namespace LicqQtGui

#endif
