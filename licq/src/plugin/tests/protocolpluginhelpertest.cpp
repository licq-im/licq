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

#include <licq/plugin/protocolpluginhelper.h>
#include <licq/userid.h>

#include <gtest/gtest.h>
#include <unistd.h>

namespace LicqTest
{

class TestProtocolPluginHelper : public Licq::ProtocolPluginHelper
{
public:
  // From Licq::PluginInterface
  std::string name() const { return "test"; }
  std::string version() const { return "1.0"; }
  int run() { return 0; }
  void destructor() { }

  // From Licq::ProtocolPluginInterface
  unsigned long protocolId() const { return 0; }
  unsigned long capabilities() const { return 0; }

  // Make public
  using ProtocolPluginHelper::getReadPipe;
  using ProtocolPluginHelper::popSignal;
};

struct ProtocolPluginHelperFixture : public ::testing::Test
{
  TestProtocolPluginHelper helper;

  char getPipeChar()
  {
    char ch;
    ::read(helper.getReadPipe(), &ch, sizeof(ch));
    return ch;
  };
};

TEST_F(ProtocolPluginHelperFixture, init)
{
  EXPECT_TRUE(helper.init(0, NULL));
}

TEST_F(ProtocolPluginHelperFixture, shutdown)
{
  helper.shutdown();
  EXPECT_EQ('X', getPipeChar());
}

TEST_F(ProtocolPluginHelperFixture, popEmpty)
{
  EXPECT_TRUE(helper.popSignal() == NULL);
}

TEST_F(ProtocolPluginHelperFixture, pushPopSignal)
{
  using Licq::ProtocolSignal;
  ProtocolSignal* signal1 = reinterpret_cast<ProtocolSignal*>(10);
  ProtocolSignal* signal2 = reinterpret_cast<ProtocolSignal*>(11);

  helper.pushSignal(signal1);
  helper.pushSignal(signal2);

  EXPECT_EQ('S', getPipeChar());
  EXPECT_EQ(signal1, helper.popSignal());

  EXPECT_EQ('S', getPipeChar());
  EXPECT_EQ(signal2, helper.popSignal());
}

TEST_F(ProtocolPluginHelperFixture, createUserOwner)
{
  EXPECT_TRUE(helper.createUser(Licq::UserId(), false) == NULL);
  EXPECT_TRUE(helper.createOwner(Licq::UserId()) == NULL);
}

} // namespace LicqTest
