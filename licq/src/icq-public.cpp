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

/*
 * This file exists here instead of in ICQ protocol to include public ICQ
 * symbols in the daemon. This is needed so plugins (e.g. the GUI) can
 * run even if the ICQ protocol isn't loaded.
 *
 * Even though IcqUser and IcqOwner objects are only created and deleted by the
 * ICQ protocol itself, the vtable (which is built together with these
 * functions) is needed by other plugins to use dynamic_cast on User and Owner
 * objects.
 */

#include <licq/icq/user.h>
#include <licq/icq/owner.h>

using Licq::IcqOwner;
using Licq::IcqUser;


IcqUser::IcqUser(const Licq::UserId& id, bool temporary)
  : Licq::User(id, temporary)
{
  // Empty
}

IcqUser::~IcqUser()
{
  // Empty
}


IcqOwner::IcqOwner(const Licq::UserId& id)
  : Licq::User(id, false), Licq::IcqUser(id, false), Licq::Owner(id)
{
  // Empty
}

IcqOwner::~IcqOwner()
{
  // Empty
}
