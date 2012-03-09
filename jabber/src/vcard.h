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

#ifndef JABBER_VCARD_H
#define JABBER_VCARD_H

#include <boost/noncopyable.hpp>

namespace Licq { class User; }
namespace gloox { class VCard; }

namespace LicqJabber
{

/**
 * Converts between a Licq user and a gloox vcard.
 */
class UserToVCard : private boost::noncopyable
{
public:
  explicit UserToVCard(const Licq::User* user) : myUser(user) {}
  gloox::VCard* createVCard() const;

private:
  const Licq::User* myUser;
};

class VCardToUser : private boost::noncopyable
{
public:
  explicit VCardToUser(const gloox::VCard* vcard) : myVCard(vcard) {}
  bool updateUser(Licq::User* user) const;

private:
  const gloox::VCard* myVCard;
};

} // namespace LicqJabber

#endif
