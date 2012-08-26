/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq developers <licq-dev@googlegroups.com>
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

#include <gloox/vcard.h>
#include <licq/contactlist/user.h>

#include <cstdlib>
#include <iomanip>
#include <sstream>

using namespace LicqJabber;

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
  if (offset == Licq::User::TimezoneUnknown)
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

  return card;
}

bool VCardToUser::updateUser(Licq::User* user) const
{
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

  user->save(Licq::User::SaveLicqInfo);
  return true;
}
