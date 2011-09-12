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

// Steal the PluginManager's friend declaration
#define PluginManager PluginTest
#include "../plugin.h"
#undef PluginManager

#include <gtest/gtest.h>
#include <list>

#include "../../utils/dynamiclibrary.h"
#include "../pluginthread.h"

// licq.cpp
static const char* argv0 = "test";
char** global_argv = const_cast<char**>(&argv0);

using Licq::Plugin;
using LicqDaemon::DynamicLibrary;
using LicqDaemon::PluginThread;

namespace LicqDaemon
{

class PluginTest : public Plugin
{
public:
  PluginTest(Params& p) :
    Plugin(p) { /* Empty */ }

  bool callInit(int argc = 0, char** argv = NULL, void (*callback)(const Plugin&) = NULL)
  { return myPrivate->callInit(argc, argv, callback); }

  void startThread(void (*startCallback)(const Plugin&) = NULL,
      void (*exitCallback)(const Plugin&) = NULL)
  { myPrivate->startThread(startCallback, exitCallback); }

  int joinThread()
  { return myPrivate->joinThread(); }

  std::string name() const
  { return "Name"; }

  std::string version() const
  { return "Version"; }

  std::string configFile() const
  { return "ConfigFile"; }

  bool init(int, char**)
  { return true; }

  int run()
  { return 5; }

  void destructor()
  { delete this; }

  // Un-protect functions so we can test them without being the PluginManager
  using Plugin::getReadPipe;
};

} // namespace LicqDaemon

using LicqDaemon::PluginTest;

struct PluginFixture : public ::testing::Test
{
  DynamicLibrary::Ptr myLib;
  PluginThread::Ptr myThread;
  Plugin::Params myPluginParams;
  PluginTest plugin;

  PluginFixture() :
    myLib(new DynamicLibrary("")),
    myThread(new PluginThread()),
    myPluginParams(1, myLib, myThread),
    plugin(myPluginParams)
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
  Plugin::Params pluginParams(1, lib, thread);
  ASSERT_NO_THROW(PluginTest plugin(pluginParams));
}

TEST_F(PluginFixture, callApiFunctions)
{
  EXPECT_EQ(1, plugin.id());
  EXPECT_EQ("Name", plugin.name());
  EXPECT_EQ("Version", plugin.version());
  EXPECT_EQ("ConfigFile", plugin.configFile());
  EXPECT_EQ("", plugin.libraryName());
}

TEST_F(PluginFixture, runPlugin)
{
  plugin.startThread();
  EXPECT_FALSE(plugin.isThisThread());
  EXPECT_EQ(5, plugin.joinThread());
}

static bool InitCallbackCalled = false;
static void initCallback(const Plugin&)
{
  InitCallbackCalled = true;
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
  InitCallbackCalled = false;
  StartCallbackCalled = false;
  ExitCallbackCalled = false;
  plugin.callInit(0, NULL, &initCallback);
  plugin.startThread(&startCallback, &exitCallback);
  EXPECT_FALSE(plugin.isThisThread());
  EXPECT_EQ(5, plugin.joinThread());
  EXPECT_TRUE(InitCallbackCalled);
  EXPECT_TRUE(StartCallbackCalled);
  EXPECT_TRUE(ExitCallbackCalled);
}

TEST_F(PluginFixture, shutdown)
{
  plugin.shutdown();
  EXPECT_EQ('X', getPipeChar());
}
