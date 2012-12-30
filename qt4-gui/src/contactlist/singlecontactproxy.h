/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef SINGLECONTACTPROXY_H
#define SINGLECONTACTPROXY_H

#include <QAbstractProxyModel>

#include <licq/userid.h>

#include "core/gui-defines.h"

namespace LicqQtGui
{
class ContactListModel;

/**
 * Proxy model to present a single user to a view
 * Used for floaties
 */
class SingleContactProxy : public QAbstractProxyModel
{
  Q_OBJECT
public:

  /**
   * Constructor
   *
   * @param contactList The contact list to get the user data from
   * @param userId Licq user id
   * @param parent Parent object
   */
  SingleContactProxy(ContactListModel* contactList, const Licq::UserId& userId, QObject* parent);

  /**
   * Destructor
   */
  virtual ~SingleContactProxy();

  /**
   * Get a model index for the user
   *
   * @param row Always zero to get the user
   * @param column A valid column for the model
   * @param parent An invalid model index to get the user
   * @return A model index for the user
   */
  virtual QModelIndex index(int row, int column, const QModelIndex& parent) const;

  /**
   * Get the parent of a model index
   *
   * @param index An index for the contact
   * @return Always an invalid index as the model only has one level
   */
  virtual QModelIndex parent(const QModelIndex& index) const;

  /**
   * Check if index has any children
   *
   * @param parent An index for the contact or an invalid index
   * @return True if the index invalid (e.g. the root index)
   */
  virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;

  /**
   * Get the number of rows in the view
   *
   * @param parent An index for the contact or an invalid index
   * @return 1 if the index is invalid or 0 otherwise
   */
  virtual int rowCount(const QModelIndex& parent) const;

  /**
   * Get the number of columns for the contact
   *
   * @param parent An invalid index
   * @return The number of columns in the contact list
   */
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

  /**
   * Get data for the contact
   *
   * @param index An index for the contact
   * @param role The qt role to get data for
   * @return The data for the given item and role
   */
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  /**
   * Get item flags for an item
   *
   * @param index An index for the contact
   * @return The item flags for the contact
   */
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  /**
   * Get header titles for the contact list
   *
   * @param section A column in the list
   * @param orientation Specify the horizontal or vertical header
   * @param role The qt role to get data for
   * @return Header data for the given column and role
   */
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  /**
   * Map a model index from the contact list model to the proxy model
   *
   * @param sourceIndex A valid index for the contact in the contact list model
   * @return An index for the contact in the proxy model or invalid index if the sourceIndex was for another contact
   */
  virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;

  /**
   * Map a the contact index to an item in the contact list model
   *
   * @param proxyIndex An index for the contact
   * @return An index for the contact in the contact list model
   */
  virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

private slots:
  /**
   * The stucture of the source has changed so refresh our pointer to the contact
   */
  void update();

  /**
   * Data in the source has changed, check if it is our contact
   *
   * @param topLeft Model index of top left item that has changed
   * @param bottomRight Model index of bottom right item that has changed
   */
  void slot_dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

private:
  ContactListModel* myContactList;
  Licq::UserId myUserId;
  QModelIndex mySourceIndex[MAX_COLUMNCOUNT];
};

} // namespace LicqQtGui

#endif
