/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq developers <licq-dev@googlegroups.com>
 *
 * Please refer to the COPYRIGHT file distributed with this source
 * distribution for the names of the individual contributors.
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

#include "vcard.h"

#include "user.h"

#include <gloox/vcard.h>

#include <licq/crypto.h>
#include <licq/logging/log.h>

#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <sstream>

using namespace LicqJabber;
using Licq::gLog;

namespace
{
static std::string guessPictureMimeType(const std::string& bindata)
{
  if (bindata.size() > 4 && bindata.substr(1, 3) == "PNG")
    return "image/png";

  if (bindata.size() > 11
      && (bindata.substr(0, 11).find("JFIF") != std::string::npos
          || bindata.substr(0, 11).find("Exif") != std::string::npos))
    return "image/jpeg";

  if (bindata.size() > 3 && bindata.substr(0, 3) == "GIF")
    return "image/gif";

  return "";
}

} // namespace

boost::optional<std::string> UserToVCard::pictureSha1() const
{
  if (Licq::Sha1::supported())
    return myUser->pictureSha1();
  else
    return boost::none;
}

gloox::VCard* UserToVCard::createVCard() const
{
  gloox::VCard* card = new gloox::VCard;

  card->setJabberid(myUser->accountId());
  card->setNickname(myUser->getAlias());
  card->setFormattedname(myUser->getFullName());
  card->setName(myUser->getLastName(), myUser->getFirstName());
  if (!myUser->getEmail().empty())
    card->addEmail(myUser->getEmail(), gloox::VCard::AddrTypePref);

  std::ostringstream tz;
  int offset = myUser->timezone();
  if (offset == User::TimezoneUnknown)
    tz << "-00:00";
  else
  {
    tz << (offset >= 0 ? '+' : '-')
       << std::setw(2) << std::setfill('0')
       << std::abs(offset) / 3600
       << ':'
       << std::setw(2) << std::setfill('0')
       << std::abs(offset / 60) % 60;
  }
  card->setTz(tz.str());

  if (myUser->GetPicturePresent())
  {
    std::string pictureData;
    if (myUser->readPictureData(pictureData))
    {
      // Only 8 KiB allowed according to XEP-0153
      const size_t maxSize = 8 * 1024;

      if (pictureData.size() < maxSize)
        card->setPhoto(guessPictureMimeType(pictureData), pictureData);
      else
        gLog.error("Picture is too large (%zu bytes); must be less than %zu",
                   pictureData.size(), maxSize);
    }
  }

  return card;
}


VCardToUser::VCardToUser(const gloox::VCard* vcard)
  : myVCard(vcard)
{
  if (Licq::Sha1::supported())
  {
    const gloox::VCard::Photo& photo = myVCard->photo();
    if (!photo.binval.empty())
      myPictureSha1 = Licq::Sha1::hashToHexString(photo.binval);
  }
}

boost::optional<std::string> VCardToUser::pictureSha1() const
{
  if (Licq::Sha1::supported())
    return myPictureSha1;
  else
    return boost::none;
}

int VCardToUser::updateUser(User* user) const
{
  int saveGroup = User::SaveUserInfo;
  user->SetEnableSave(false);

  if (!user->KeepAliasOnUpdate())
  {
    if (!myVCard->nickname().empty())
      user->setAlias(myVCard->nickname());
    else if (!myVCard->formattedname().empty())
      user->setAlias(myVCard->formattedname());
  }

  const gloox::VCard::Name& name = myVCard->name();
  user->setUserInfoString("FirstName", name.given);
  user->setUserInfoString("LastName", name.family);

  // Bug in gloox: emailAddresses should be const
  const gloox::VCard::EmailList& emails =
      const_cast<gloox::VCard*>(myVCard)->emailAddresses();
  if (emails.begin() != emails.end())
    user->setUserInfoString("Email1", emails.begin()->userid);

  const gloox::VCard::Photo& photo = myVCard->photo();
  if (!photo.binval.empty())
  {
    saveGroup |= User::SavePictureInfo;

    // Store hash of too large picture as well to avoid downloading them again
    if (Licq::Sha1::supported())
      user->setPictureSha1(myPictureSha1);

    if (photo.binval.size() <= 100 * 1024)
      user->SetPicturePresent(user->writePictureData(photo.binval));
    else
    {
      gLog.error("Picture for %s is too big (%zu bytes)",
                 user->id().accountId().c_str(), photo.binval.size());

      user->SetPicturePresent(false);
      user->deletePictureData();
    }
  }
  else if (user->GetPicturePresent())
  {
    saveGroup |= User::SavePictureInfo;

    user->setPictureSha1("");
    user->SetPicturePresent(false);
    user->deletePictureData();
  }

  user->SetEnableSave(true);
  user->save(saveGroup);

  return saveGroup;
}
