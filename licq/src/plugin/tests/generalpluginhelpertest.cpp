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

#include <licq/plugin/generalpluginhelper.h>

#include <gtest/gtest.h>
#include <unistd.h>

namespace LicqTest
{

class TestGeneralPluginHelper : public Licq::GeneralPluginHelper
{
public:
  // From Licq::PluginInterface
  std::string name() const { return "test"; }
  std::string version() const { return "1.0"; }
  int run() { return 0; }
  void destructor() { }

  // From Licq::GeneralPluginInterface
  std::string description() const { return "test plugin"; }
  std::string usage() const { return ""; }
  bool isEnabled() const { return true; }

  // Make public
  using GeneralPluginHelper::getReadPipe;
  using GeneralPluginHelper::setSignalMask;
  using GeneralPluginHelper::popSignal;
  using GeneralPluginHelper::popEvent;
};

struct GeneralPluginHelperFixture : public ::testing::Test
{
  TestGeneralPluginHelper helper;

  char getPipeChar()
  {
    char ch;
    ::read(helper.getReadPipe(), &ch, sizeof(ch));
    return ch;
  };
};

TEST_F(GeneralPluginHelperFixture, init)
{
  EXPECT_TRUE(helper.init(0, NULL));
}

TEST_F(GeneralPluginHelperFixture, shutdown)
{
  helper.shutdown();
  EXPECT_EQ('X', getPipeChar());
}

TEST_F(GeneralPluginHelperFixture, configFile)
{
  EXPECT_EQ(std::string(), helper.configFile());
}

TEST_F(GeneralPluginHelperFixture, enable)
{
  helper.enable();
  EXPECT_EQ('1', getPipeChar());
}

TEST_F(GeneralPluginHelperFixture, disable)
{
  helper.disable();
  EXPECT_EQ('0', getPipeChar());
}

TEST_F(GeneralPluginHelperFixture, signalMask)
{
  EXPECT_FALSE(helper.wantSignal(0x1));
  helper.setSignalMask(0xf);
  EXPECT_TRUE(helper.wantSignal(0x1));
  EXPECT_FALSE(helper.wantSignal(0x10));
}

TEST_F(GeneralPluginHelperFixture, popEmpty)
{
  EXPECT_TRUE(helper.popSignal() == NULL);
  EXPECT_TRUE(helper.popEvent() == NULL);
}

TEST_F(GeneralPluginHelperFixture, pushPopSignal)
{
  using Licq::PluginSignal;
  PluginSignal* signal1 = reinterpret_cast<PluginSignal*>(10);
  PluginSignal* signal2 = reinterpret_cast<PluginSignal*>(11);

  helper.pushSignal(signal1);
  helper.pushSignal(signal2);

  EXPECT_EQ('S', getPipeChar());
  EXPECT_EQ(signal1, helper.popSignal());

  EXPECT_EQ('S', getPipeChar());
  EXPECT_EQ(signal2, helper.popSignal());
}

TEST_F(GeneralPluginHelperFixture, pushPopEvent)
{
  using Licq::Event;
  Event* event1 = reinterpret_cast<Event*>(10);
  Event* event2 = reinterpret_cast<Event*>(11);

  helper.pushEvent(event1);
  helper.pushEvent(event2);

  EXPECT_EQ('E', getPipeChar());
  EXPECT_EQ(event1, helper.popEvent());

  EXPECT_EQ('E', getPipeChar());
  EXPECT_EQ(event2, helper.popEvent());
}

} // namespace LicqTest
