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

#ifndef MAINCONTACTLISTPROXY_H
#define MAINCONTACTLISTPROXY_H

#include "sortedcontactlistproxy.h"

namespace LicqQtGui
{
class ContactListModel;

/**
 * Proxy for ContactListModel that presents a sorted and filtered list
 *
 * This proxy presents the full list as a tree or a all contacts of a single
 * group as used by the contact list in the main window.
 * Sorting is inherited from SortedContactList.
 *
 * System groups can be filtered to get a tree with only the normal groups.
 * Offline contacts and bars are filtered according to gui config.
 * Ignored users are always filetered from the "All Users" group.
 */
class MainContactListProxy : public SortedContactListProxy
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param contactList The contact list to perform sorting and filtering on
   * @param parent Parent object
   */
  MainContactListProxy(ContactListModel* contactList, QObject* parent = NULL);

  /**
   * Destructor
   */
  virtual ~MainContactListProxy() {}

  /**
   * Change filter for threaded or flat view
   *
   * @param enable True for threaded view, false for flat group view
   * @param mode2 Use mode 2 view, ignored if threaded view is disabled
   */
  void setThreadedView(bool enable, bool mode2);

private slots:
  /**
   * Update according to new GUI coniguration
   */
  void configUpdated();

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

  ContactListModel* myContactList;
  bool myThreadedView;
  bool myMode2View;
  QAbstractProxyModel* myProxy;
};

} // namespace LicqQtGui

#endif
