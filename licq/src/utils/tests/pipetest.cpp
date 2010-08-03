/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#include <licq/pipe.h>

#include <gtest/gtest.h>

using Licq::Pipe;

TEST(Pipe, create)
{
  Pipe pipe;
}

TEST(Pipe, readWrite)
{
  Pipe pipe;

  const char out[] = "hello";
  EXPECT_EQ(6, pipe.write(out, sizeof(out)));

  char in[sizeof(out)];
  EXPECT_EQ(6, pipe.read(in, sizeof(in)));

  EXPECT_STREQ("hello", in);
}

TEST(Pipe, readWriteChar)
{
  Pipe pipe;

  pipe.putChar('a');
  EXPECT_EQ('a', pipe.getChar());
}

TEST(Pipe, readWriteMultipleChars)
{
  Pipe pipe;

  pipe.putChar('a');
  pipe.putChar('b');
  EXPECT_EQ('a', pipe.getChar());
  pipe.putChar('c');
  EXPECT_EQ('b', pipe.getChar());
  EXPECT_EQ('c', pipe.getChar());
}
