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

#ifndef LICQ_CRYPTO_H
#define LICQ_CRYPTO_H

#include <string>
#include <stdint.h>

namespace Licq
{

/// Returns @a length bytes of @a data as a hex string
std::string toHexString(const uint8_t* data, size_t length);

namespace Md5
{
static const size_t DIGEST_LENGTH = 16;

/**
 * Calculate the MD5 hash of @a data and store it in @a digest.
 *
 * @param data The data to hash.
 * @param length The length of @a data.
 * @param digest Result buffer with space for at least DIGEST_LENGTH bytes.
 */
void hash(const uint8_t* data, size_t length, uint8_t* digest);
inline void hash(const std::string& data, uint8_t* digest);

/**
 * Calculate the MD5 hash of @a data and return it as a hex string.
 *
 * @param data The data to hash.
 * @param length The length of @a data.
 */
std::string hashToHexString(const uint8_t* data, size_t length);
inline std::string hashToHexString(const std::string& data);

} // namespace Md5

namespace Sha1
{

static const size_t DIGEST_LENGTH = 20;

/// Return true if Licq supports SHA1
bool supported();

/**
 * Calculate the SHA1 hash of @a data and store it in @a digest.
 *
 * @param data The data to hash.
 * @param length The length of @a data.
 * @param digest Result buffer with space for at least DIGEST_LENGTH bytes.
 */
void hash(const uint8_t* data, size_t length, uint8_t* digest);
inline void hash(const std::string& data, uint8_t* digest);

/**
 * Calculate the SHA1 hash of @a data and return it as a hex string.
 *
 * @param data The data to hash.
 * @param length The length of @a data.
 */
std::string hashToHexString(const uint8_t* data, size_t length);
inline std::string hashToHexString(const std::string& data);

} // namespace Sha1

} // namespace Licq

inline void Licq::Md5::hash(const std::string& data, uint8_t* digest)
{
  hash(reinterpret_cast<const uint8_t*>(data.data()), data.size(), digest);
}

inline std::string Licq::Md5::hashToHexString(const std::string& data)
{
  return hashToHexString(
      reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

inline void Licq::Sha1::hash(const std::string& data, uint8_t* digest)
{
  hash(reinterpret_cast<const uint8_t*>(data.data()), data.size(), digest);
}

inline std::string Licq::Sha1::hashToHexString(const std::string& data)
{
  return hashToHexString(
      reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

#endif
