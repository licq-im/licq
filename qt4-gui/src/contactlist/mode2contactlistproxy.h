// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2009 Licq developers
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

#ifndef MODE2CONTACTLISTPROXY_H
#define MODE2CONTACTLISTPROXY_H

#include <QAbstractProxyModel>
#include <QList>
#include <QMap>

#include "contactitem.h"

namespace LicqQtGui
{
class ContactBar;
class ContactListModel;
class ContactUser;
class ContactProxyGroup;


// Data used internally by proxy to keep track of each user
struct Mode2ProxyUserData
{
  int sourceRow; // Row number in the source model
  int proxyRow; // Row number presented by the proxy model
  int groupRow; // Offset in myGroups of parent group for this user. Note: Not source or proxy rows
  int unreadEvents; // Number of unread events
  bool isVisible; // Visible flag for user
};

/**
 * Proxy for ContactListModel that converts it into a mode 2 list.
 *
 * This proxy duplicates all groups making separate groups for online users
 * and offline users. A MainContactListProxy is still needed to sort and
 * filter the list before presenting it to a view.
 *
 * Note: This proxy is dependant on the internal structure and implementation
 * of the contact list model. This includes how signals are emitted and usage
 * of internalPointer in the model indexes. Asserts exits in some places to
 * catch if model changes to break these dependancies.
 *
 * Top level consists of two bars (online at row 0 and offline at row 1)
 * followed by two groups for each source model group (online at even row
 * numbers and offline at odd rows). Groups are in same order as in model but
 * proxy assigns row numbers for users dynamically.
 */
class Mode2ContactListProxy : public QAbstractProxyModel
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param contactList The contact list to convert to mode 2
   * @param parent Parent object
   */
  Mode2ContactListProxy(ContactListModel* contactList, QObject* parent = NULL);

  /**
   * Destructor
   */
  virtual ~Mode2ContactListProxy();

  /**
   * Map a source model index into a proxy index
   *
   * @param sourceIndex Index from source mode
   * @return An index for this proxy
   */
  virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;

  /**
   * Map a proxy index into a source model index
   *
   * @param proxyIndex An index for this proxy
   * @return An index for the source model
   */
  virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

  /**
   * Get data for an item
   *
   * @param index Index of the item
   * @param role Type of data to get
   * @return The requested data
   */
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  /**
   * Get a proxy model index for an item
   *
   * @param row A row in the model
   * @param column A column in the model
   * @param parent Index for a group to get a user, or an invalid index to get a group or bar
   * @return A model index for the requested group or user if it exists
   */
  virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

  /**
   * Get parent index for an item in the model
   *
   * @param index An index for the proxy model
   * @return A group index if a user is provided or an invalid index if a group or bar is provided
   */
  virtual QModelIndex parent(const QModelIndex& index) const;

  /**
   * Get number of groups or number of users in a group
   *
   * @param parent Index of a group to get users or invalid to get groups and bars
   * @return Number of users in the group or the number of groups and bars
   */
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

  /**
   * Get number of columns in the model
   *
   * @param parent Ignored
   * @return Number of columns in the model
   */
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

private slots:
  /**
   * Drop and reread all the mappings
   */
  void reset();

  /**
   * Data in source model has changed
   *
   * @param topLeft Index of top left position changed
   * @param bottomRight Index of lower right position changed
   */
  void sourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

  /**
   * Source model is about to insert rows
   *
   * @param parent Index of group to insert user or invalid to insert group
   * @param start First row to insert
   * @param end Last row to insert
   */
  void sourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end);

  /**
   * Source model has inserted rows
   *
   * @param parent Index of group for inserted user or invalid for inserted group
   * @param start First row inserted
   * @param end Last row inserted
   */
  void sourceRowsInserted(const QModelIndex& parent, int start, int end);

  /**
   * Source model is about to remove rows
   *
   * @param parent Index of group to remove user or invalid to remove group
   * @param start First row to remove
   * @param end Last row to remove
   */
  void sourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);

  /**
   * Source model has removed rows
   *
   * @param parent Index of group for removed user or invalid for removed group
   * @param start First row to removed
   * @param end Last row to removed
   */
  void sourceRowsRemoved(const QModelIndex& parent, int start, int end);

private:
  /**
   * Add a source model user to the proxy and emit insert signals
   *
   * @param userIndex Index (from source model) for user item
   * @param emitSignals True to emit signals for changes
   */
  void addUser(const QModelIndex& userIndex, bool emitSignals = true);

  /**
   * Remove a user from the proxy model and emit remove signals
   *
   * @param userIndex User object from source model
   */
  void removeUser(ContactUser* cu);

  /**
   * Add a source model group and all its users to the proxy
   * Will not emit any signals
   *
   * @param groupIndex Index (from source model) for group item
   */
  void addGroup(const QModelIndex& groupIndex);

  // Number of bars, must be an even number.
  // Number should never change, constant is here to make the code more readable
  static const int NumBars = 2;

  ContactBar* myBars[NumBars];
  QList<ContactProxyGroup*> myGroups;
  QMap<ContactUser*, Mode2ProxyUserData> myUserData;
  int myColumnCount;
};


/**
 * Group item used by the mode 2 proxy to hold half a group, either the online
 * or the offline part of a contact list group.
 * This class is basically a wrapper for ContactGroup overriding parts as needed.
 */
class ContactProxyGroup : public ContactItem
{
  Q_OBJECT

public:
  ContactProxyGroup(ContactGroup* sourceGroup, bool isOnline);
  virtual QVariant data(int column, int role) const;
  ContactGroup* sourceGroup() const { return mySourceGroup; }
  int userCount() const { return myUserCount; }
  void updateUserCount(int counter) { myUserCount += counter; }
  void updateVisibility(int counter) { myVisibleCount += counter; }
  void updateEvents(int counter) { myEventCount += counter; }
private:
  ContactGroup* mySourceGroup;
  const bool myIsOnline;
  int myUserCount;
  int myVisibleCount;
  int myEventCount;
};


} // namespace LicqQtGui

#endif
