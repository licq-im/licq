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

#include "maincontactlistproxy.h"

#include "config/contactlist.h"

#include "contactlist.h"

using namespace LicqQtGui;

MainContactListProxy::MainContactListProxy(ContactListModel* contactList, QObject* parent)
  : SortedContactListProxy(contactList, parent),
    myThreadedView(false)
{
  // Update filter when list configuration changes
  connect(Config::ContactList::instance(), SIGNAL(currentListChanged()), SLOT(configUpdated()));
}

void MainContactListProxy::setThreadedView(bool enable)
{
  myThreadedView = enable;
  invalidateFilter();
}

void MainContactListProxy::configUpdated()
{
  invalidateFilter();
}

bool MainContactListProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  QModelIndex item = sourceModel()->index(source_row, 0, source_parent);

  switch (static_cast<ContactListModel::ItemType>(item.data(ContactListModel::ItemTypeRole).toInt()))
  {
    case ContactListModel::GroupItem:
    {
      // Filter system groups
      if (myThreadedView && item.data(ContactListModel::GroupIdRole).toInt() >= ContactListModel::SystemGroupOffset)
        return false;

      // Filter empty groups in threaded view
      if (myThreadedView && !Config::ContactList::instance()->showEmptyGroups())
      {
        // Check for empty groups
        if (item.data(ContactListModel::UserCountRole).toInt() == 0)
          return false;

        // If not showing offline contacts a group with > 0 contacts may also be empty
        if (!Config::ContactList::instance()->showOffline() &&
            !item.data(ContactListModel::VisibilityRole).toBool())
         return false;
      }

      break;
    }
    case ContactListModel::UserItem:
    {
      // Filter ignored users from all groups except "Ignore List"
      if ((item.data(ContactListModel::ExtendedStatusRole).toUInt() & ContactListModel::IgnoreStatus) &&
          item.data(ContactListModel::GroupIdRole).toUInt() != ContactListModel::SystemGroupOffset + GROUP_IGNORE_LIST)
        return false;

      // Filter offline users unless "Show Offline Users" are enabled
      if (!Config::ContactList::instance()->showOffline() &&
          !item.data(ContactListModel::VisibilityRole).toBool())
        return false;

      break;
    }
    case ContactListModel::BarItem:
    {
      // Filter all sub group headers
      if (myThreadedView)
        return false;

      ContactListModel::SubGroupType subGroup = static_cast<ContactListModel::SubGroupType>(item.data(ContactListModel::SubGroupRole).toInt());

      // Filter header bars if config says so, but keep the Not In List Bar (if needed)
      if (!Config::ContactList::instance()->showDividers() &&
          subGroup != ContactListModel::NotInListSubGroup)
        return false;

      // Filter header for sub groups with no users in them
      if (item.data(ContactListModel::UserCountRole).toInt() <= 0)
        return false;

      // Filter the offline header if offline users are filtered, but keep header if any user has unread events
      if (!Config::ContactList::instance()->showOffline() &&
          subGroup == ContactListModel::OfflineSubGroup &&
          item.data(ContactListModel::UnreadEventsRole).toInt() == 0)
        return false;

      break;
    }
    default:
      // Unknown item type so we probably don't want to show it
      return false;
  }
  return true;
}