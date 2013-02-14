/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2013 Licq developers <licq-dev@googlegroups.com>
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

#include <licq/crypto.h>
#include <licq/inifile.h>

using namespace LicqJabber;

namespace
{

const char KEY_PICTURE_SHA1[] = "JabberPictureSha1";

} // namespace

User::User(const Licq::UserId& id, bool temporary)
  : Licq::User(id, temporary)
{
  Licq::IniFile& conf(userConf());

  conf.get(KEY_PICTURE_SHA1, myPictureSha1);
}

User::~User()
{
  // Empty
}

void User::savePictureInfo()
{
  Licq::User::savePictureInfo();

  if (GetPicturePresent() && myPictureSha1.empty() && Licq::Sha1::supported())
  {
    std::string pictureData;
    if (readPictureData(pictureData))
      myPictureSha1 = Licq::Sha1::hashToHexString(pictureData);
  }

  Licq::IniFile& conf(userConf());

  if (!myPictureSha1.empty())
    conf.set(KEY_PICTURE_SHA1, myPictureSha1);
  else
    conf.unset(KEY_PICTURE_SHA1);
}
