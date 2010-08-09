/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq Developers <licq-dev@googlegroups.com>
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

#include <ctime>
#include <licq/contactlist/user.h>
#include <gloox/vcard.h>

gloox::VCard* UserToVCard::createVCard() const
{
  gloox::VCard* card = new gloox::VCard;

  card->setJabberid(myUser->accountId());
  card->setNickname(myUser->getAlias());
  card->setName(myUser->getLastName(), myUser->getFirstName());
  if (!myUser->getEmail().empty())
    card->addEmail(myUser->getEmail(), gloox::VCard::AddrTypePref);

  struct tm tm;
  time_t now = ::time(0);
  ::localtime_r(&now, &tm);

  char tz[10];
  strftime(tz, sizeof(tz), "%z", &tm);
  card->setTz(tz);

  return card;
}