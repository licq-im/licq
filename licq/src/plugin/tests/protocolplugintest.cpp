/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq Developers <licq-dev@googlegroups.com>
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
#define PluginManager ProtocolPluginTest
#include "../protocolplugin.h"
#include <licq/plugin/protocolbase.h>
#undef PluginManager

#include <gtest/gtest.h>

#include "../../utils/dynamiclibrary.h"
#include "../plugin.h"
#include "../pluginthread.h"

#define PPID_TEST ('P' << 24 | 'P' << 16 | 'I' << 8 | 'D')

using Licq::ProtocolPlugin;
using LicqDaemon::DynamicLibrary;
using LicqDaemon::PluginThread;

namespace LicqDaemon
{

class ProtocolPluginTest : public ProtocolPlugin
{
public:
  ProtocolPluginTest(Params& p) :
      ProtocolPlugin(p)
  { /* Empty */ }

  bool callInit(int argc = 0, char** argv = NULL, void (*callback)(const Plugin&) = NULL)
  { return basePrivate()->callInit(argc, argv, callback); }

  void startThread(void (*startCallback)(const Plugin&) = NULL,
      void (*exitCallback)(const Plugin&) = NULL)
  { basePrivate()->startThread(startCallback, exitCallback); }

  int joinThread()
  { return basePrivate()->joinThread(); }

  std::string name() const
  { return "Name"; }

  std::string version() const
  { return "Version"; }

  std::string configFile() const
  { return "ConfigFile"; }

  unsigned long protocolId() const
  { return PPID_TEST; }

  std::string defaultServerHost() const
  { return "DefSrvHost"; }

  int defaultServerPort() const
  { return 12345; }

  unsigned long capabilities() const
  { return 42; }

  bool init(int, char**)
  { return true; }

  int run()
  { return 10; }

  void destructor()
  { delete this; }

  // Un-protect functions so we can test them without being the PluginManager
  using ProtocolPlugin::getReadPipe;
  using ProtocolPlugin::popSignal;
};

} // namespace LicqDaemon

using LicqDaemon::ProtocolPluginTest;

namespace LicqTest {

struct ProtocolPluginFixture : public ::testing::Test
{
  DynamicLibrary::Ptr myLib;
  PluginThread::Ptr myThread;
  ProtocolPlugin::Params myPluginParams;
  ProtocolPluginTest plugin;

  ProtocolPluginFixture() :
    myLib(new DynamicLibrary("")),
    myThread(new PluginThread()),
    myPluginParams(1, myLib, myThread),
    plugin(myPluginParams)
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
  ProtocolPlugin::Params pluginParams(1, lib, thread);
  ASSERT_NO_THROW(ProtocolPluginTest plugin(pluginParams));
}

TEST_F(ProtocolPluginFixture, callApiFunctions)
{
  EXPECT_EQ("Name", plugin.name());
  EXPECT_EQ("Version", plugin.version());
  EXPECT_EQ("ConfigFile", plugin.configFile());
  unsigned long ppid = PPID_TEST;
  EXPECT_EQ(ppid, plugin.protocolId());
  EXPECT_TRUE(plugin.callInit());
  EXPECT_EQ(42u, plugin.capabilities());
  EXPECT_EQ("DefSrvHost", plugin.defaultServerHost());
  EXPECT_EQ(12345, plugin.defaultServerPort());
}

TEST_F(ProtocolPluginFixture, init)
{
  EXPECT_TRUE(plugin.callInit());
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

} // namespace LicqTest
