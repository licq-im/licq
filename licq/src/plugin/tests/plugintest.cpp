/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq Developers <licq-dev@googlegroups.com>
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

#include <licq/plugin/plugininterface.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

// licq.cpp
static const char* argv0 = "test";
char** global_argv = const_cast<char**>(&argv0);

using LicqDaemon::Plugin;
using LicqDaemon::DynamicLibrary;
using LicqDaemon::PluginThread;

using ::testing::_;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Return;

namespace LicqTest
{

// Two dummy interfaces
class InternalPluginInterface { };
class UnImplementedInterface { };

class MockPlugin : public Licq::PluginInterface,
                   public InternalPluginInterface
{
public:
  MOCK_CONST_METHOD0(name, std::string());
  MOCK_CONST_METHOD0(version, std::string());
  MOCK_METHOD2(init, bool(int argc, char** argv));
  MOCK_METHOD0(run, int());
  MOCK_METHOD0(shutdown, void());
  MOCK_METHOD0(destructor, void());
};

class TestPlugin : public Plugin
{
public:
  bool myIsCreated;

  TestPlugin(int id, DynamicLibrary::Ptr lib, PluginThread::Ptr thread,
             boost::shared_ptr<Licq::PluginInterface> interface)
    : Plugin(id, lib, thread),
      myIsCreated(false),
      myInterface(interface)
  {
    // Empty
  }

protected:
  // From Plugin
  void createInterface() { myIsCreated = true; }

  boost::shared_ptr<Licq::PluginInterface> interface()
  {
    return myInterface;
  }

  boost::shared_ptr<const Licq::PluginInterface> interface() const
  {
    return myInterface;
  }

private:
  boost::shared_ptr<Licq::PluginInterface> myInterface;
};

static void NullDeleter(void*) { /* Empty */ }

struct PluginFixture : public ::testing::Test
{
  DynamicLibrary::Ptr myLib;
  PluginThread::Ptr myThread;
  MockPlugin myMockInterface;
  TestPlugin plugin;

  pthread_t myPluginThreadId;

  PluginFixture() :
    myLib(new DynamicLibrary("")),
    myThread(new PluginThread()),
    plugin(1, myLib, myThread,
           boost::shared_ptr<MockPlugin>(&myMockInterface, &NullDeleter)),
    myPluginThreadId(0)
  {
    // Empty
  }

  ~PluginFixture()
  {
    myThread->cancel();
  }

  int compareThread()
  {
    myPluginThreadId = ::pthread_self();
    return myThread->isThread(::pthread_self()) ? 5 : -5;
  }
};

TEST_F(PluginFixture, callApiFunctions)
{
  InSequence dummy;
  EXPECT_CALL(myMockInterface, name());
  EXPECT_CALL(myMockInterface, version());
  EXPECT_CALL(myMockInterface, shutdown());

  // Verify that the calls are forwarded to the interface
  plugin.name();
  plugin.version();
  plugin.shutdown();
}

TEST_F(PluginFixture, castToInternalInterface)
{
  Plugin::Ptr ptr(&plugin, &NullDeleter);

  EXPECT_FALSE(plugin_internal_cast<UnImplementedInterface>(ptr));
  EXPECT_EQ(plugin_internal_cast<InternalPluginInterface>(ptr).get(),
            &myMockInterface);
}

static int DeleteCount = 0;
void CountingNullDeleter(void*)
{
  DeleteCount += 1;
}

TEST_F(PluginFixture, lifeTimeOfCastedObject)
{
  DeleteCount = 0;

  // The plugin should not be "deleted" until both the plugin and the interface
  // has gone out of scope.

  {
    boost::shared_ptr<InternalPluginInterface> interface;
    {
      Plugin::Ptr ptr(&plugin, &CountingNullDeleter);
      interface = plugin_internal_cast<InternalPluginInterface>(ptr);
    }
    EXPECT_EQ(0, DeleteCount);
  }

  EXPECT_EQ(1, DeleteCount);
}

TEST_F(PluginFixture, create)
{
  EXPECT_FALSE(plugin.myIsCreated);
  EXPECT_TRUE(plugin.create());
  EXPECT_TRUE(plugin.myIsCreated);
}

struct CallbackData
{
  bool myCalled;
  const Plugin* myPlugin;
  pthread_t myThreadId;

  CallbackData() : myCalled(false), myPlugin(NULL), myThreadId(0) { }
};

static CallbackData InitCallbackData;
static void initCallback(const Plugin& plugin)
{
  InitCallbackData.myCalled = true;
  InitCallbackData.myPlugin = &plugin;
  InitCallbackData.myThreadId = ::pthread_self();
}

TEST_F(PluginFixture, init)
{
  EXPECT_CALL(myMockInterface, init(1, _));
  InitCallbackData = CallbackData();

  plugin.init(0, NULL, &initCallback);

  EXPECT_TRUE(InitCallbackData.myCalled);
  EXPECT_EQ(&plugin, InitCallbackData.myPlugin);
  EXPECT_TRUE(myThread->isThread(InitCallbackData.myThreadId));
}

static CallbackData StartCallbackData;
static void startCallback(const Plugin& plugin)
{
  StartCallbackData.myCalled = true;
  StartCallbackData.myPlugin = &plugin;
  StartCallbackData.myThreadId = ::pthread_self();
}

static CallbackData ExitCallbackData;
static void exitCallback(const Plugin& plugin)
{
  ExitCallbackData.myCalled = true;
  ExitCallbackData.myPlugin = &plugin;
  ExitCallbackData.myThreadId = ::pthread_self();
}

TEST_F(PluginFixture, run)
{
  EXPECT_CALL(myMockInterface, run())
      .WillOnce(Invoke(this, &PluginFixture::compareThread));
  myPluginThreadId = 0;
  StartCallbackData = CallbackData();
  ExitCallbackData = CallbackData();

  plugin.run(&startCallback, &exitCallback);
  EXPECT_EQ(5, plugin.joinThread());

  EXPECT_TRUE(StartCallbackData.myCalled);
  EXPECT_EQ(&plugin, StartCallbackData.myPlugin);
  EXPECT_TRUE(::pthread_equal(myPluginThreadId, StartCallbackData.myThreadId));

  EXPECT_TRUE(ExitCallbackData.myCalled);
  EXPECT_EQ(&plugin, ExitCallbackData.myPlugin);
  EXPECT_TRUE(::pthread_equal(myPluginThreadId, ExitCallbackData.myThreadId));
}

} // namespace LicqTest
