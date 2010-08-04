// -*- c-basic-offset: 2 -*-
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

#include "protocombobox.h"

#include <boost/foreach.hpp>

#include <licq/contactlist/usermanager.h>
#include <licq/pluginmanager.h>

#include "config/iconmanager.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ProtoComboBox */

ProtoComboBox::ProtoComboBox(QWidget* parent)
  : QComboBox(parent)
{
  fillComboBox();
}

ProtoComboBox::ProtoComboBox(bool skipExisting, QWidget* parent)
  : QComboBox(parent)
{
  fillComboBox(skipExisting);
}

void ProtoComboBox::fillComboBox(bool skipExisting)
{
  QString id;

  Licq::ProtocolPluginsList protocols;
  Licq::gPluginManager.getProtocolPluginsList(protocols);
  BOOST_FOREACH(Licq::ProtocolPlugin::Ptr protocol, protocols)
  {
    unsigned long ppid = protocol->getProtocolId();
    Licq::UserId userId = Licq::gUserManager.ownerUserId(ppid);
    if (userId.isValid() && skipExisting)
      continue;

    addItem(
        IconManager::instance()->iconForStatus(Licq::User::OnlineStatus, userId), // icon
        protocol->getName(), // protocol name
        QString::number(ppid) // user data
        );
  }
}

unsigned long ProtoComboBox::currentPpid() const
{
  return itemData(currentIndex()).toString().toULong();
}

bool ProtoComboBox::setCurrentPpid(unsigned long ppid)
{
  int index = findData(QString::number(ppid));

  if (index == -1)
    return false;

  setCurrentIndex(index);

  return true;
}
