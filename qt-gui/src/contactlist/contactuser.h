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

#ifndef CONTACTUSER_H
#define CONTACTUSER_H

#include <QVariant>

#include "contactitem.h"
#include "contactlist.h"
#include "contactuserdata.h"

namespace LicqQtGui
{
class ContactGroup;
class ContactUserData;

/**
 * A user in a group
 * As a user can be in multiple groups the actual user data is located in ContactUserData and shared between all ContactUser objects
 *
 * This class is used internally by ContactList and should not be accessed from any other class
 */
class ContactUser : public ContactItem
{
  Q_OBJECT

public:
  /**
   * Constructor, adds a user to a group
   */
  ContactUser(ContactUserData* userData, ContactGroup* group);

  /**
   * Destructor, removes the user from the group it is located in
   */
  virtual ~ContactUser();

  /**
   * Get the group this user instance bolong to
   */
  ContactGroup* group() const
  { return myGroup; }

  /**
   * Get the user data for this user instance
   */
  ContactUserData* userData() const
  { return myUserData; }

  /**
   * Get licq user id
   */
  const Licq::UserId& userId() const
  { return myUserData->userId(); }

  /**
   * Get number of unread events for this user
   */
  int numEvents() const
  { return myUserData->numEvents(); }

  /**
   * Get visibility
   */
  bool visibility() const
  { return myUserData->visibility(); }

  /**
   * Get data for this user
   *
   * @param column A valid column in the contact list
   * @param role The qt role to get data for
   * @return Data for this user
   */
  QVariant data(int column, int role) const;

  /**
   * Set data for this user
   *
   * @param value New value to set
   * @param role Role to set
   * @return True if any data was changed
   */
  virtual bool setData(const QVariant& value, int role = ContactListModel::NameRole);

private:
  ContactUserData* myUserData;
  ContactGroup* myGroup;
};

} // namespace LicqQtGui

#endif
