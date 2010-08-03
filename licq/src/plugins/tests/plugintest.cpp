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

// Daemon stuff
extern pthread_cond_t LP_IdSignal;
extern pthread_mutex_t LP_IdMutex;
extern std::list<unsigned short> LP_Ids;

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

void Test_Exit(int result)
{
  int *p = (int *)malloc(sizeof(int));
  *p = result;
  pthread_mutex_lock(&LP_IdMutex);
  LP_Ids.push_back(1);
  pthread_mutex_unlock(&LP_IdMutex);
  pthread_cond_signal(&LP_IdSignal);
  pthread_exit(p);
}

void* Test_Main_tep()
{
  Test_Exit(Test_Main());
  return NULL;
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

static bool CallbackCalled = false;
static void startCallback(Plugin&)
{
  CallbackCalled = true;
}

TEST_F(PluginFixture, runPluginWithCallback)
{
  CallbackCalled = false;
  plugin.startThread(&startCallback);
  EXPECT_FALSE(plugin.isThisThread());
  EXPECT_EQ(5, plugin.joinThread());
  EXPECT_TRUE(CallbackCalled);
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
