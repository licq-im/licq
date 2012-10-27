/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#include "ownercombobox.h"

#include <boost/foreach.hpp>

#include <licq/contactlist/usermanager.h>
#include <licq/contactlist/owner.h>

#include "contactlist/contactlist.h"

#include "config/iconmanager.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::OwnerComboBox */

OwnerComboBox::OwnerComboBox(const QString& extra, QWidget* parent)
  : QComboBox(parent)
{
  if (!extra.isNull())
    addItem(extra, QVariant::fromValue(Licq::UserId()));

  Licq::OwnerListGuard ownerList;
  BOOST_FOREACH(const Licq::Owner* owner, **ownerList)
  {
    Licq::OwnerReadGuard o(owner);
    addItem(IconManager::instance()->iconForStatus(Licq::User::OnlineStatus, o->id()),
        o->getAlias().c_str(), QVariant::fromValue(o->id()));
  }
}

Licq::UserId OwnerComboBox::currentOwnerId() const
{
  return itemData(currentIndex()).value<Licq::UserId>();
}

bool OwnerComboBox::setCurrentOwnerId(const Licq::UserId& ownerId)
{
  for (int index = 0; index < count(); ++index)
  {
    if (itemData(index).value<Licq::UserId>() == ownerId)
    {
      setCurrentIndex(index);
      return true;
    }
  }
  return false;
}
