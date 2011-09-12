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
#define PluginManager GeneralPluginTest
#include "../generalplugin.h"
#include <licq/plugin/generalbase.h>
#undef PluginManager

#include <licq/pluginsignal.h>

#include <gtest/gtest.h>

#include "../../utils/dynamiclibrary.h"
#include "../plugin.h"
#include "../pluginthread.h"

using Licq::GeneralPlugin;
using LicqDaemon::DynamicLibrary;
using LicqDaemon::PluginThread;

namespace LicqDaemon
{

class GeneralPluginTest : public GeneralPlugin
{
public:
  GeneralPluginTest(Params& p) :
      GeneralPlugin(p)
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

  std::string description() const
  { return "Description"; }

  std::string usage() const
  { return "Usage"; }

  std::string configFile() const
  { return "ConfigFile"; }

  bool isEnabled() const
  { return false; }

  bool init(int, char**)
  { return true; }

  int run()
  { return 20; }

  void destructor()
  { delete this; }

  // Un-protect functions so we can test them without being the PluginManager
  using GeneralPlugin::getReadPipe;
  using GeneralPlugin::popEvent;
  using GeneralPlugin::popSignal;
  using GeneralPlugin::setSignalMask;
};

} // namespace LicqDaemon

using LicqDaemon::GeneralPluginTest;

struct GeneralPluginFixture : public ::testing::Test
{
  DynamicLibrary::Ptr myLib;
  PluginThread::Ptr myThread;
  GeneralPlugin::Params myPluginParams;
  GeneralPluginTest plugin;

  GeneralPluginFixture() :
    myLib(new DynamicLibrary("")),
    myThread(new PluginThread()),
    myPluginParams(1, myLib, myThread),
    plugin(myPluginParams)
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
  GeneralPlugin::Params pluginParams(1, lib, thread);
  ASSERT_NO_THROW(GeneralPluginTest plugin(pluginParams));
}

TEST_F(GeneralPluginFixture, callApiFunctions)
{
  EXPECT_EQ("Name", plugin.name());
  EXPECT_EQ("Version", plugin.version());
  EXPECT_FALSE(plugin.isEnabled());
  EXPECT_EQ("Description", plugin.description());
  EXPECT_EQ("Usage", plugin.usage());
  EXPECT_EQ("ConfigFile", plugin.configFile());

  EXPECT_TRUE(plugin.callInit(0, 0));
}

TEST_F(GeneralPluginFixture, init)
{
  EXPECT_TRUE(plugin.callInit(0, NULL));
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

TEST_F(GeneralPluginFixture, signalmask)
{
  EXPECT_FALSE(plugin.wantSignal(1));
  plugin.setSignalMask(0xf);
  EXPECT_TRUE(plugin.wantSignal(1));
  EXPECT_FALSE(plugin.wantSignal(0x10));
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
