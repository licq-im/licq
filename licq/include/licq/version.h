/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Licq developers
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

#ifndef LICQ_VERSION_H
#define LICQ_VERSION_H

#include "licq/licqversion.h"

#define LICQ_MAKE_VERSION(a,b,c) ((((a) * 1000) + ((b) * 10) + (c)) & 0xffff)

#define LICQ_VERSION \
    LICQ_MAKE_VERSION(LICQ_VERSION_MAJOR,LICQ_VERSION_MINOR,LICQ_VERSION_RELEASE)

#define LICQ_IS_VERSION(a,b,c) (LICQ_VERSION >= LICQ_MAKE_VERSION(a,b,c))

namespace Licq
{

/// Returns the major version number from the encoded @a version.
inline unsigned int extractMajorVersion(unsigned int version)
{
  return ((version > 64999) ? 64 : (version / 1000));
}

/// Returns the minor version number from the encoded @a version.
inline unsigned int extractMinorVersion(unsigned int version)
{
  return ((version > 64999) ? 99 : ((version / 10) % 100));
}

/// Returns the release version number from the encoded @a version.
inline unsigned int extractReleaseVersion(unsigned int version)
{
  return ((version > 64999) ? 9 : (version % 10));
}

} // namespace Licq

#endif
