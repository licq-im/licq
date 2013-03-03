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
#include "../plugininstance.h"

#include <licq/plugin/pluginfactory.h>
#include <licq/plugin/plugininterface.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

// licq.cpp
static const char* argv0 = "test";
char** global_argv = const_cast<char**>(&argv0);

using LicqDaemon::Plugin;
using LicqDaemon::PluginInstance;
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

class MockPluginFactory : public Licq::PluginFactory
{
public:
  MOCK_CONST_METHOD0(name, std::string());
  MOCK_CONST_METHOD0(version, std::string());
  MOCK_METHOD1(destroyPlugin, void(Licq::PluginInterface* plugin));
};

class MockPlugin : public Licq::PluginInterface,
                   public InternalPluginInterface
{
public:
  MOCK_METHOD2(init, bool(int argc, char** argv));
  MOCK_METHOD0(run, int());
  MOCK_METHOD0(shutdown, void());
};

class TestPlugin : public Plugin
{
public:
  typedef boost::shared_ptr<TestPlugin> Ptr;

  TestPlugin(DynamicLibrary::Ptr lib,
             boost::shared_ptr<Licq::PluginFactory> factory)
    : Plugin(lib),
      myFactory(factory)
  {
    // Empty
  }

  boost::shared_ptr<Licq::PluginFactory> factory()
  {
    return myFactory;
  }

protected:
  // From Plugin
  boost::shared_ptr<const Licq::PluginFactory> factory() const
  {
    return myFactory;
  }

private:
  boost::shared_ptr<Licq::PluginFactory> myFactory;
};

class TestPluginInstance : public PluginInstance
{
public:
  bool myIsCreated;

  TestPluginInstance(
      int id, TestPlugin::Ptr plugin, PluginThread::Ptr thread,
      boost::shared_ptr<Licq::PluginInterface> interface)
    : PluginInstance(id, thread),
      myIsCreated(false),
      myPlugin(plugin),
      myInterface(interface)
  {
    // Empty
  }

  ~TestPluginInstance()
  {
    if (myInterface)
      myPlugin->factory()->destroyPlugin(myInterface.get());
  }

protected:
  // From PluginInstance
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
  TestPlugin::Ptr myPlugin;
  boost::shared_ptr<Licq::PluginInterface> myInterface;
};

static void nullDeleter(void*) { /* Empty */ }

struct PluginFixture : public ::testing::Test
{
  DynamicLibrary::Ptr myLib;
  PluginThread::Ptr myThread;
  MockPluginFactory myMockFactory;
  MockPlugin myMockInterface;
  TestPlugin plugin;
  TestPluginInstance instance;

  pthread_t myPluginThreadId;

  PluginFixture() :
    myLib(new DynamicLibrary("")),
    myThread(new PluginThread()),
    plugin(myLib,
           boost::shared_ptr<MockPluginFactory>(&myMockFactory, &nullDeleter)),
    instance(1, boost::shared_ptr<TestPlugin>(&plugin, &nullDeleter),
             myThread,
             boost::shared_ptr<MockPlugin>(&myMockInterface, &nullDeleter)),
    myPluginThreadId(0)
  {
    EXPECT_CALL(myMockFactory, destroyPlugin(&myMockInterface));
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
  EXPECT_CALL(myMockFactory, name());
  EXPECT_CALL(myMockFactory, version());
  EXPECT_CALL(myMockInterface, shutdown());

  // Verify that the calls are forwarded to the interface
  plugin.name();
  plugin.version();
  instance.shutdown();
}

TEST_F(PluginFixture, castToInternalInterface)
{
  PluginInstance::Ptr ptr(&instance, &nullDeleter);

  EXPECT_FALSE(plugin_internal_cast<UnImplementedInterface>(ptr));
  EXPECT_EQ(plugin_internal_cast<InternalPluginInterface>(ptr).get(),
            &myMockInterface);
}

static int DeleteCount = 0;
void countingNullDeleter(void*)
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
      PluginInstance::Ptr ptr(&instance, &countingNullDeleter);
      interface = plugin_internal_cast<InternalPluginInterface>(ptr);
    }
    EXPECT_EQ(0, DeleteCount);
  }

  EXPECT_EQ(1, DeleteCount);
}

TEST_F(PluginFixture, create)
{
  EXPECT_FALSE(instance.myIsCreated);
  EXPECT_TRUE(instance.create());
  EXPECT_TRUE(instance.myIsCreated);
}

struct CallbackData
{
  bool myCalled;
  const PluginInstance* myInstance;
  pthread_t myThreadId;

  CallbackData() : myCalled(false), myInstance(NULL), myThreadId(0) { }
};

static CallbackData InitCallbackData;
static void initCallback(const PluginInstance& instance)
{
  InitCallbackData.myCalled = true;
  InitCallbackData.myInstance = &instance;
  InitCallbackData.myThreadId = ::pthread_self();
}

TEST_F(PluginFixture, init)
{
  EXPECT_CALL(myMockInterface, init(1, _));
  InitCallbackData = CallbackData();

  instance.init(0, NULL, &initCallback);

  EXPECT_TRUE(InitCallbackData.myCalled);
  EXPECT_EQ(&instance, InitCallbackData.myInstance);
  EXPECT_TRUE(myThread->isThread(InitCallbackData.myThreadId));
}

static CallbackData StartCallbackData;
static void startCallback(const PluginInstance& instance)
{
  StartCallbackData.myCalled = true;
  StartCallbackData.myInstance = &instance;
  StartCallbackData.myThreadId = ::pthread_self();
}

static CallbackData ExitCallbackData;
static void exitCallback(const PluginInstance& instance)
{
  ExitCallbackData.myCalled = true;
  ExitCallbackData.myInstance = &instance;
  ExitCallbackData.myThreadId = ::pthread_self();
}

TEST_F(PluginFixture, run)
{
  EXPECT_CALL(myMockInterface, run())
      .WillOnce(Invoke(this, &PluginFixture::compareThread));
  myPluginThreadId = 0;
  StartCallbackData = CallbackData();
  ExitCallbackData = CallbackData();

  instance.run(&startCallback, &exitCallback);
  EXPECT_EQ(5, instance.joinThread());

  EXPECT_TRUE(StartCallbackData.myCalled);
  EXPECT_EQ(&instance, StartCallbackData.myInstance);
  EXPECT_TRUE(::pthread_equal(myPluginThreadId, StartCallbackData.myThreadId));

  EXPECT_TRUE(ExitCallbackData.myCalled);
  EXPECT_EQ(&instance, ExitCallbackData.myInstance);
  EXPECT_TRUE(::pthread_equal(myPluginThreadId, ExitCallbackData.myThreadId));
}

} // namespace LicqTest
