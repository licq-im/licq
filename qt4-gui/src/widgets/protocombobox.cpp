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

#include "protocombobox.h"

#include <licq_icqd.h>

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
  unsigned long ppid;

  FOR_EACH_PROTO_PLUGIN_START(gLicqDaemon)
  {
    ppid = (*_ppit)->PPID();
    const ICQOwner* o = gUserManager.FetchOwner(ppid, LOCK_R);
    if (o == NULL)
      id = "0";
    else
    {
      if (skipExisting)
      {
        gUserManager.DropOwner(o);
        continue;
      }
      else
      {
        id = o->IdString();
        gUserManager.DropOwner(o);
      }
    }

    addItem(
        IconManager::instance()->iconForStatus(ICQ_STATUS_ONLINE, id, ppid), // icon
        (*_ppit)->Name(), // protocol name
        QString::number(ppid) // user data
        );
  }
  FOR_EACH_PROTO_PLUGIN_END
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
