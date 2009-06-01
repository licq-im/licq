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

#include "licqstrings.h"

#include <QApplication>

#include "licq_user.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::LicqStrings */

QString LicqStrings::getStatus(unsigned short status, bool invisible)
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
  else if (status == ICQ_STATUS_FxPRIVATE && !invisible)  // In case we just
    str = qApp->translate("Status", "Invisible");         // wanna get a string
  else if ((status & 0xFF) == 0)
    str = qApp->translate("Status", "Online");
  else
    str = qApp->translate("Status", "Unknown");

  if (invisible)
    str = QString("(%1)").arg(str);

  return str;
}

QString LicqStrings::getStatus(const LicqUser* user, bool useInvisible)
{
  if (user == NULL)
    return QString();

  return getStatus(user->Status(), useInvisible && user->StatusInvisible());
}

QString LicqStrings::getShortStatus(unsigned short status, bool invisible)
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
  else if (status == ICQ_STATUS_FxPRIVATE && !invisible)  // In case we just
    str = qApp->translate("Status", "Inv");               // wanna get a string
  else if ((status & 0xFF) == 0)
    str = qApp->translate("Status", "On");
  else
    str = qApp->translate("Status", "???");

  if (invisible)
    str = QString("(%1)").arg(str);

  return str;
}

QString LicqStrings::getShortStatus(const LicqUser* user, bool useInvisible)
{
  if (user == NULL)
    return QString();

  return getShortStatus(user->Status(), useInvisible && user->StatusInvisible());
}

QString LicqStrings::getSystemGroupName(unsigned long group)
{
  switch (group)
  {
  case GROUP_ALL_USERS:
    return qApp->translate("Groups", "All Users");
  case GROUP_ONLINE_NOTIFY:
    return qApp->translate("Groups", "Online Notify");
  case GROUP_VISIBLE_LIST:
    return qApp->translate("Groups", "Visible List");
  case GROUP_INVISIBLE_LIST:
    return qApp->translate("Groups", "Invisible List");
  case GROUP_IGNORE_LIST:
    return qApp->translate("Groups", "Ignore List");
  case GROUP_NEW_USERS:
    return qApp->translate("Groups", "New Users");
  }

  return qApp->translate("Groups", "Unknown");
}
