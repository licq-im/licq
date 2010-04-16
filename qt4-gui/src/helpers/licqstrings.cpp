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
