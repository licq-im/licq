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

#include "../generalplugin.h"
#include <licq/pluginbase.h>

#include <gtest/gtest.h>
#include <list>
#include <pthread.h>

// Deamon stuff
pthread_cond_t LP_IdSignal = PTHREAD_COND_INITIALIZER;
pthread_mutex_t LP_IdMutex = PTHREAD_MUTEX_INITIALIZER;
std::list<unsigned short> LP_Ids;

// licq.cpp
static const char* argv0 = "test";
char** global_argv = const_cast<char**>(&argv0);

// Plugin API functions
#define STR_FUNC(name)                          \
  const char* LP_ ## name()                     \
  { static char name[] = #name; return name; }

STR_FUNC(Name);
STR_FUNC(Version);
STR_FUNC(Status);
STR_FUNC(Description);
STR_FUNC(Usage);
STR_FUNC(ConfigFile);

bool LP_Init(int /*argc*/, char** /*argv*/)
{
  return true;
}

int LP_Main()
{
  return 20;
}

using namespace LicqDaemon;

struct GeneralPluginFixture : public ::testing::Test
{
  DynamicLibrary::Ptr myLib;
  PluginThread::Ptr myThread;
  GeneralPlugin plugin;

  GeneralPluginFixture() :
    myLib(new DynamicLibrary("")),
    myThread(new PluginThread()),
    plugin(myLib, myThread)
  {
    // Empty
  }

  ~GeneralPluginFixture()
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

TEST(GeneralPlugin, load)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  PluginThread::Ptr thread(new PluginThread());
  ASSERT_NO_THROW(GeneralPlugin plugin(lib, thread));
}

TEST_F(GeneralPluginFixture, callApiFunctions)
{
  EXPECT_STREQ("Name", plugin.getName());
  EXPECT_STREQ("Version", plugin.getVersion());
  EXPECT_STREQ("Status", plugin.getStatus());
  EXPECT_STREQ("Description", plugin.getDescription());
  EXPECT_STREQ("Usage", plugin.getUsage());
  EXPECT_STREQ("ConfigFile", plugin.getConfigFile());
  EXPECT_TRUE(plugin.getBuildDate());
  EXPECT_TRUE(plugin.getBuildTime());

  EXPECT_TRUE(plugin.init(0, 0));
}

TEST_F(GeneralPluginFixture, init)
{
  EXPECT_TRUE(plugin.init(0, NULL));
}

TEST_F(GeneralPluginFixture, runPlugin)
{
  plugin.startThread();
  EXPECT_EQ(20, plugin.joinThread());
}

TEST_F(GeneralPluginFixture, enableDisable)
{
  plugin.enable();
  EXPECT_EQ('1', getPipeChar());
  plugin.disable();
  EXPECT_EQ('0', getPipeChar());
}

TEST_F(GeneralPluginFixture, pushPopSignal)
{
  Licq::PluginSignal* signal = (Licq::PluginSignal*)10;
  plugin.pushSignal(signal);
  plugin.pushSignal(signal);

  EXPECT_EQ('S', getPipeChar());
  EXPECT_EQ(signal, plugin.popSignal());

  EXPECT_EQ('S', getPipeChar());
  EXPECT_EQ(signal, plugin.popSignal());
}

TEST_F(GeneralPluginFixture, popSignalEmpty)
{
  EXPECT_EQ(static_cast<Licq::PluginSignal*>(NULL), plugin.popSignal());
}

TEST_F(GeneralPluginFixture, pushPopEvent)
{
  Licq::Event* event = (Licq::Event*)20;
  plugin.pushEvent(event);
  plugin.pushEvent(event);

  EXPECT_EQ('E', getPipeChar());
  EXPECT_EQ(event, plugin.popEvent());

  EXPECT_EQ('E', getPipeChar());
  EXPECT_EQ(event, plugin.popEvent());
}

TEST_F(GeneralPluginFixture, popEventEmpty)
{
  EXPECT_EQ(static_cast<Licq::Event*>(NULL), plugin.popEvent());
}
