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

#ifndef CONTACTBAR_H
#define CONTACTBAR_H

#include <QString>
#include <QVariant>

#include "contactitem.h"
#include "contactlist.h"


namespace LicqQtGui
{
class ContactGroup;

/**
 * A separator bar in the contact list used to separate online and offline users
 *
 * This class is used internally by CContactList and should not be accessed from any other class
 */
class ContactBar : public ContactItem
{
  Q_OBJECT

public:
  /**
   * Constructor, creates a bar
   */
  ContactBar(ContactListModel::SubGroupType subGroup, ContactGroup* group);

  /**
   * Destructor
   */
  virtual ~ContactBar() {}

  /**
   * Get the group this user instance belongs to
   */
  ContactGroup* group() const
  { return myGroup; }

  /**
   * Get sub group
   */
  ContactListModel::SubGroupType subGroup() const
  { return mySubGroup; }

  /**
   * Get current user count
   */
  int count() const
  { return myUserCount; }

  /**
   * Increase the current number of users in this sub group
   */
  void countIncrease();

  /**
   * Decrease the current number of users in this sub group
   */
  void countDecrease();

  /**
   * Update unread event counter for sub group
   *
   * @param counter Number to increase or decrease event counter by
   */
  void updateNumEvents(int counter);

  /**
   * Update visibility counter
   *
   * @param increase True if counter should be increased
   */
  void updateVisibility(bool increase);

  /**
   * Get data for this bar
   *
   * @param column A valid column in the contact list
   * @param role The qt role to get data for
   * @return Data for this bar
   */
  QVariant data(int column, int role) const;

private:
  ContactGroup* myGroup;
  ContactListModel::SubGroupType mySubGroup;
  QString myText;
  int myUserCount;
  int myEvents;
  int myVisibleContacts;
};

} // namespace LicqQtGui

#endif
