/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2010, 2013 Licq developers <licq-dev@googlegroups.com>
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

#include "groupcombobox.h"

#include <boost/foreach.hpp>

#include <licq/contactlist/group.h>
#include <licq/contactlist/usermanager.h>

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::GroupComboBox */

GroupComboBox::GroupComboBox(bool groupPos, QWidget* parent)
  : QComboBox(parent)
{
  if (groupPos)
    addItem(tr("First"), -1);

  Licq::GroupListGuard groupList(true);
  BOOST_FOREACH(const Licq::Group* group, **groupList)
  {
    Licq::GroupReadGuard pGroup(group);
    QString text = QString::fromLocal8Bit(pGroup->name().c_str());
    if (groupPos)
      text.prepend(tr("After "));
    addItem(text, pGroup->id());
  }
}

int GroupComboBox::currentGroupId() const
{
  return itemData(currentIndex()).toInt();
}

bool GroupComboBox::setCurrentGroupId(int groupId)
{
  int index = findData(groupId);

  if (index == -1)
    return false;

  setCurrentIndex(index);

  return true;
}

bool GroupComboBox::setCurrentGroupName(const QString& groupName)
{
  int index = findText(groupName);

  if (index == -1)
    return false;

  setCurrentIndex(index);

  return true;
}
