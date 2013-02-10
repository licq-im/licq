/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2013 Licq Developers <licq-dev@googlegroups.com>
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

#include "config.h"

#include <licq/crypto.h>
#include <licq/logging/log.h>

#include "md5.h"

#include <cassert>

#ifdef USE_OPENSSL
#include <openssl/sha.h>
#endif

std::string Licq::toHexString(const uint8_t* data, size_t length)
{
  const char* hexDigits = "0123456789abcdef";

  std::string hexString;
  hexString.reserve(length * 2);

  for (size_t i = 0; i < length; ++i) {
    hexString += hexDigits[(data[i] >> 4) & 0xf];
    hexString += hexDigits[data[i] & 0xf];
  }

  return hexString;
}

void Licq::Md5::hash(const uint8_t* data, size_t length, uint8_t* digest)
{
  using namespace LicqDaemon;

  Md5Context context;
  md5Init(&context);
  md5Update(&context, data, length);
  md5Final(&context, digest);
}

std::string Licq::Md5::hashToHexString(const uint8_t* data, size_t length)
{
  uint8_t digest[DIGEST_LENGTH];
  hash(data, length, digest);
  return toHexString(digest, DIGEST_LENGTH);
}

bool Licq::Sha1::supported()
{
#ifdef USE_OPENSSL
  return true;
#else
  return false;
#endif
}

void Licq::Sha1::hash(const uint8_t* data, size_t length, uint8_t* digest)
{
#ifdef USE_OPENSSL
  SHA1(data, length, digest);
#else
  (void)data;
  (void)length;
  (void)digest;

  assert(false);
#endif
}

std::string Licq::Sha1::hashToHexString(const uint8_t* data, size_t length)
{
  uint8_t digest[DIGEST_LENGTH];
  hash(data, length, digest);
  return toHexString(digest, DIGEST_LENGTH);
}
