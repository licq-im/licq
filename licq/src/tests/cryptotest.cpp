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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::ContainerEq;

TEST(Crypto, toHexString)
{
  using Licq::toHexString;

  EXPECT_EQ("", toHexString(0, 0));

  {
    const uint8_t data[] = { 0x12, 0x34 };
    EXPECT_EQ("1234", toHexString(data, sizeof(data)));
  }

  {
    const uint8_t data[] = { 0xab, 0xff, 0, 1, 2, 0x80 };
    EXPECT_EQ("abff00010280", toHexString(data, sizeof(data)));
  }
}

TEST(Crypto, md5Hash)
{
  using namespace Licq::Md5;

  uint8_t digest[DIGEST_LENGTH] = { 0 };
  hash("foobar", digest);

  uint8_t expected[DIGEST_LENGTH] = {
    0x38, 0x58, 0xf6, 0x22, 0x30, 0xac, 0x3c, 0x91,
    0x5f, 0x30, 0x0c, 0x66, 0x43, 0x12, 0xc6, 0x3f
  };
  EXPECT_THAT(digest, ContainerEq(expected));
}

TEST(Crypto, md5HashToHexString)
{
  using namespace Licq::Md5;

  EXPECT_EQ("4933f598bc49f873dae7c5f8817050a0",
            hashToHexString("Licq"));
}

TEST(Crypto, sha1Supported)
{
#ifdef USE_OPENSSL
  EXPECT_TRUE(Licq::Sha1::supported());
#else
  EXPECT_FALSE(Licq::Sha1::supported());
#endif
}

#ifdef USE_OPENSSL

TEST(Crypto, sha1Hash)
{
  using namespace Licq::Sha1;

  uint8_t digest[DIGEST_LENGTH] = { 0 };
  hash("foobar", digest);

  uint8_t expected[DIGEST_LENGTH] = {
    0x88, 0x43, 0xd7, 0xf9, 0x24, 0x16, 0x21, 0x1d, 0xe9, 0xeb,
    0xb9, 0x63, 0xff, 0x4c, 0xe2, 0x81, 0x25, 0x93, 0x28, 0x78
  };
  EXPECT_THAT(digest, ContainerEq(expected));
}

TEST(Crypto, sha1HashToHexString)
{
  using namespace Licq::Sha1;

  EXPECT_EQ("f4151fa94c74c488dc110f8e79119654b1c00e0e",
            hashToHexString("Licq"));
}

#endif
