/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq developers
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

#include "../protocolplugin.h"
#include <licq/protocolbase.h>

#include <gtest/gtest.h>

// Plugin API functions
#define STR_FUNC(name)                          \
  const char* LProto_ ## name()                 \
  { static char name[] = #name; return name; }

STR_FUNC(Name);
STR_FUNC(Version);
STR_FUNC(ConfigFile);
STR_FUNC(PPID);
STR_FUNC(DefSrvHost);

bool LProto_Init()
{
  return true;
}

unsigned long LProto_SendFuncs()
{
  return 42;
}

int LProto_DefSrvPort()
{
  return 12345;
}

int LProto_Main()
{
  return 10;
}

using namespace LicqDaemon;

struct ProtocolPluginFixture : public ::testing::Test
{
  DynamicLibrary::Ptr myLib;
  PluginThread::Ptr myThread;
  ProtocolPlugin plugin;

  ProtocolPluginFixture() :
    myLib(new DynamicLibrary("")),
    myThread(new PluginThread()),
    plugin(myLib, myThread)
  {
    // Empty
  }

  ~ProtocolPluginFixture()
  {
    myThread->cancel();
  }

  char getPipeChar()
  {
    int fd = plugin.getReadPipe();
    char ch;
    read(fd, &ch, sizeof(ch));
    return ch;
  }
};

TEST(ProtocolPlugin, load)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  PluginThread::Ptr thread(new PluginThread());
  ASSERT_NO_THROW(ProtocolPlugin plugin(lib, thread));
}

TEST_F(ProtocolPluginFixture, callApiFunctions)
{
  EXPECT_EQ("Name", plugin.name());
  EXPECT_EQ("Version", plugin.version());
  EXPECT_EQ("ConfigFile", plugin.configFile());
  unsigned long ppid = 'P' << 24 | 'P' << 16 | 'I' << 8 | 'D';
  EXPECT_EQ(ppid, plugin.protocolId());
  EXPECT_TRUE(plugin.init());
  EXPECT_EQ(42u, plugin.capabilities());
  EXPECT_EQ("DefSrvHost", plugin.defaultServerHost());
  EXPECT_EQ(12345, plugin.defaultServerPort());
}

TEST_F(ProtocolPluginFixture, init)
{
  EXPECT_TRUE(plugin.init());
}

TEST_F(ProtocolPluginFixture, runPlugin)
{
  plugin.startThread();
  EXPECT_EQ(10, plugin.joinThread());
}

TEST_F(ProtocolPluginFixture, pushPopSignal)
{
  Licq::ProtocolSignal* signal = (Licq::ProtocolSignal*)10;
  plugin.pushSignal(signal);
  plugin.pushSignal(signal);

  EXPECT_EQ('S', getPipeChar());
  EXPECT_EQ(signal, plugin.popSignal());

  EXPECT_EQ('S', getPipeChar());
  EXPECT_EQ(signal, plugin.popSignal());
}

TEST_F(ProtocolPluginFixture, popSignalEmpty)
{
  EXPECT_EQ(static_cast<Licq::ProtocolSignal*>(NULL), plugin.popSignal());
}
