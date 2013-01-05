/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "user.h"

#include <licq/inifile.h>
#include <licq/socket.h>

using namespace LicqMsn;

User::User(const Licq::UserId& id, bool temporary)
  : Licq::User(id, temporary),
    myNormalSocketDesc(NULL),
    myInfoSocketDesc(NULL)
{
  Licq::IniFile& conf(userConf());
  conf.get("PictureObject", myPictureObject);
}

User::~User()
{
  // Empty
}

void User::savePictureInfo()
{
  Licq::User::savePictureInfo();

  Licq::IniFile& conf(userConf());
  conf.set("PictureObject", myPictureObject);
}

void User::clearSocketDesc(Licq::INetSocket* s)
{
  if (s == NULL || s == myNormalSocketDesc)
    myNormalSocketDesc = NULL;
  if (s == NULL || s == myInfoSocketDesc)
    myInfoSocketDesc = NULL;
}
