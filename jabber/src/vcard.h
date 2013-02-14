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

#ifndef JABBER_VCARD_H
#define JABBER_VCARD_H

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <string>

namespace gloox { class VCard; }

namespace LicqJabber
{

class User;

/**
 * Converts between a Licq user and a gloox vcard.
 */
class UserToVCard : private boost::noncopyable
{
public:
  explicit UserToVCard(const User* user) : myUser(user) {}
  boost::optional<std::string> pictureSha1() const;
  gloox::VCard* createVCard() const;

private:
  const User* myUser;
};

class VCardToUser : private boost::noncopyable
{
public:
  explicit VCardToUser(const gloox::VCard* vcard);
  boost::optional<std::string> pictureSha1() const;
  int updateUser(User* user) const;

private:
  std::string myPictureSha1;

  const gloox::VCard* myVCard;
};

} // namespace LicqJabber

#endif
