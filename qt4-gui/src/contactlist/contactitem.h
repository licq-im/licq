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

#ifndef CONTACTITEM_H
#define CONTACTITEM_H

#include <QObject>
#include <QVariant>

#include "contactlist.h"

namespace LicqQtGui
{
/**
 * Virtual base class for groups and users in the contact list
 *
 * This class is used internally by CContactList and should not be accessed from any other class
 */
class ContactItem : public QObject
{
  Q_OBJECT

public:
  /**
   * Constructor
   */
  ContactItem(ContactListModel::ItemType type);

  /**
   * Destructor
   */
  virtual ~ContactItem() {};

  /**
   * Get the type for this item
   *
   * @return An item type
   */
  ContactListModel::ItemType itemType() const
  { return myItemType; }

  /**
   * Get data for this item
   *
   * @param column A valid column in the contact list
   * @param role The qt role to get data for
   * @return Data for this item
   */
  virtual QVariant data(int column, int role) const = 0;

  /**
   * Set data for this item
   *
   * @param value New value to set
   * @param role Role to set
   * @return True if any data was changed
   */
  virtual bool setData(const QVariant& value, int role = ContactListModel::NameRole);

private:
  ContactListModel::ItemType myItemType;
};

} // namespace LicqQtGui

#endif
