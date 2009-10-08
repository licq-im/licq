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

#include "contactuser.h"

#include "contactgroup.h"
#include "contactuserdata.h"

using namespace LicqQtGui;

ContactUser::ContactUser(ContactUserData* userData, ContactGroup* group)
  : ContactItem(ContactListModel::UserItem),
    myUserData(userData),
    myGroup(group)
{
  myUserData->addGroup(this);
  myGroup->addUser(this, myUserData->subGroup());
}

ContactUser::~ContactUser()
{
  // Remove references to ourselves
  myGroup->removeUser(this, myUserData->subGroup());
  myUserData->removeGroup(this);
}

QVariant ContactUser::data(int column, int role) const
{
  switch (role)
  {
    case ContactListModel::GroupIdRole:
      return myGroup->groupId();
  }

  return myUserData->data(column, role);
}

bool ContactUser::setData(const QVariant& value, int role)
{
  return myUserData->setData(value, role);
}
