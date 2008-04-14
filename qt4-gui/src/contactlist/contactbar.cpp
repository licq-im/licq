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

#include "contactbar.h"

#include "contactgroup.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ContactBar */

ContactBar::ContactBar(ContactListModel::SubGroupType subGroup, ContactGroup* group)
  : ContactItem(ContactListModel::BarItem),
    myGroup(group),
    mySubGroup(subGroup),
    myUserCount(0),
    myEvents(0)
{
  switch (mySubGroup)
  {
    case ContactListModel::OnlineSubGroup:
      myText = tr("Online");
      break;
    case ContactListModel::OfflineSubGroup:
      myText = tr("Offline");
      break;
    case ContactListModel::NotInListSubGroup:
    default:
      myText = tr("Not In List");
      break;
  }
}

void ContactBar::countIncrease()
{
  myUserCount++;
}

void ContactBar::countDecrease()
{
  myUserCount--;
}

void ContactBar::updateNumEvents(int counter)
{
  myEvents += counter;
}

QVariant ContactBar::data(int column, int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
      if (column == 0)
        return myText;
      break;

    case ContactListModel::NameRole:
      return myText;

    case ContactListModel::ItemTypeRole:
      return ContactListModel::BarItem;

    case ContactListModel::SortPrefixRole:
      return 2 * mySubGroup;

    case ContactListModel::SortRole:
      return "";

    case ContactListModel::GroupIdRole:
      return myGroup->groupId();

    case ContactListModel::SubGroupRole:
      return mySubGroup;

    case ContactListModel::UserCountRole:
      return myUserCount;

    case ContactListModel::UnreadEventsRole:
      return myEvents;
  }

  return QVariant();
}
