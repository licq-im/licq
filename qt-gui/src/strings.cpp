/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Erik Johansson <erijo@licq.org>
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

#include "strings.h"
#include "licq_user.h"

#include <qapplication.h>

QString Strings::getStatus(unsigned short status, bool invisible)
{
  QString str;

  if (status == ICQ_STATUS_OFFLINE)
    str = qApp->translate("Status", "Offline");
  else if (status & ICQ_STATUS_DND)
    str = qApp->translate("Status", "Do Not Disturb");
  else if (status & ICQ_STATUS_OCCUPIED)
    str = qApp->translate("Status", "Occupied");
  else if (status & ICQ_STATUS_NA)
    str = qApp->translate("Status", "Not Available");
  else if (status & ICQ_STATUS_AWAY)
    str = qApp->translate("Status", "Away");
  else if (status & ICQ_STATUS_FREEFORCHAT)
    str = qApp->translate("Status", "Free for Chat");
  else if ((status & 0xFF) == 0)
    str = qApp->translate("Status", "Online");
  else
    str = qApp->translate("Status", "Unknown");

  if (invisible)
    str = QString("(%1)").arg(str);

  return str;
}

QString Strings::getStatus(ICQUser* user, bool useInvisible)
{
  if (user == NULL)
    return QString();

  return getStatus(user->Status(), useInvisible && user->StatusInvisible());
}

QString Strings::getShortStatus(unsigned short status, bool invisible)
{
  QString str;

  if (status == ICQ_STATUS_OFFLINE)
    str = qApp->translate("Status", "Off");
  else if (status & ICQ_STATUS_DND)
    str = qApp->translate("Status", "DND");
  else if (status & ICQ_STATUS_OCCUPIED)
    str = qApp->translate("Status", "Occ");
  else if (status & ICQ_STATUS_NA)
    str = qApp->translate("Status", "N/A");
  else if (status & ICQ_STATUS_AWAY)
    str = qApp->translate("Status", "Away");
  else if (status & ICQ_STATUS_FREEFORCHAT)
    str = qApp->translate("Status", "FFC");
  else if ((status & 0xFF) == 0)
    str = qApp->translate("Status", "On");
  else
    str = qApp->translate("Status", "???");

  if (invisible)
    str = QString("(%1)").arg(str);

  return str;
}

QString Strings::getShortStatus(ICQUser* user, bool useInvisible)
{
  if (user == NULL)
    return QString();

  return getShortStatus(user->Status(), useInvisible && user->StatusInvisible());
}
