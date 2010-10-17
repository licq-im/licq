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

#include "../plugin.h"

#include <gtest/gtest.h>
#include <list>

extern "C" {

// Plugin API functions
#define STR_FUNC(name)                          \
  const char* Test_ ## name()                   \
  { static char name[] = #name; return name; }

STR_FUNC(Name);
STR_FUNC(Version);

int Test_Main()
{
  return 5;
}

extern unsigned short LP_Id;

} // extern "C"

using namespace LicqDaemon;

class PluginTest : public Plugin
{
public:
  PluginTest(DynamicLibrary::Ptr lib, PluginThread::Ptr thread) :
    Plugin(lib, thread, "Test") { /* Empty */ }

private:
  bool initThreadEntry()
  {
    return true;
  }
};

struct PluginFixture : public ::testing::Test
{
  DynamicLibrary::Ptr myLib;
  PluginThread::Ptr myThread;
  PluginTest plugin;

  PluginFixture() :
    myLib(new DynamicLibrary("")),
    myThread(new PluginThread()),
    plugin(myLib, myThread)
  {
    // Empty
  }

  ~PluginFixture()
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

TEST(Plugin, load)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  PluginThread::Ptr thread(new PluginThread());
  ASSERT_NO_THROW(PluginTest plugin(lib, thread));
}

TEST_F(PluginFixture, callApiFunctions)
{
  EXPECT_STREQ("Name", plugin.getName());
  EXPECT_STREQ("Version", plugin.getVersion());
  EXPECT_EQ("", plugin.getLibraryName());
}

TEST_F(PluginFixture, getSetId)
{
  plugin.setId(1);
  EXPECT_EQ(1, plugin.getId());
  EXPECT_EQ(1, LP_Id);
}

TEST_F(PluginFixture, runPlugin)
{
  plugin.startThread();
  EXPECT_FALSE(plugin.isThisThread());
  EXPECT_EQ(5, plugin.joinThread());
}

static bool StartCallbackCalled = false;
static void startCallback(const Plugin&)
{
  StartCallbackCalled = true;
}

static bool ExitCallbackCalled = false;
static void exitCallback(const Plugin&)
{
  ExitCallbackCalled = true;
}

TEST_F(PluginFixture, runPluginWithCallbacks)
{
  StartCallbackCalled = false;
  ExitCallbackCalled = false;
  plugin.startThread(&startCallback, &exitCallback);
  EXPECT_FALSE(plugin.isThisThread());
  EXPECT_EQ(5, plugin.joinThread());
  EXPECT_TRUE(StartCallbackCalled);
  EXPECT_TRUE(ExitCallbackCalled);
}

TEST_F(PluginFixture, shutdown)
{
  plugin.shutdown();
  EXPECT_EQ('X', getPipeChar());
}

TEST_F(PluginFixture, signalmask)
{
  EXPECT_FALSE(plugin.wantSignal(1));
  plugin.setSignalMask(0xf);
  EXPECT_TRUE(plugin.wantSignal(1));
  EXPECT_FALSE(plugin.wantSignal(0x10));
}
