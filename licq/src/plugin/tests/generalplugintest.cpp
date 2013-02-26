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

#include "../generalplugin.h"
#include "../generalplugininstance.h"

#include <licq/plugin/generalpluginfactory.h>
#include <licq/plugin/generalplugininterface.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using LicqDaemon::GeneralPlugin;
using LicqDaemon::GeneralPluginInstance;
using LicqDaemon::DynamicLibrary;
using LicqDaemon::PluginThread;

using ::testing::InSequence;
using ::testing::Return;

namespace LicqTest
{

class MockGeneralPluginFactory : public Licq::GeneralPluginFactory
{
public:
  MOCK_CONST_METHOD0(name, std::string());
  MOCK_CONST_METHOD0(version, std::string());
  MOCK_METHOD1(destroyPlugin, void(Licq::PluginInterface* plugin));

  MOCK_CONST_METHOD0(description, std::string());
  MOCK_CONST_METHOD0(usage, std::string());
  MOCK_CONST_METHOD0(configFile, std::string());
  MOCK_METHOD0(createPlugin, Licq::GeneralPluginInterface*());
};

class MockGeneralPlugin : public Licq::GeneralPluginInterface
{
public:
  MOCK_METHOD2(init, bool(int argc, char** argv));
  MOCK_METHOD0(run, int());
  MOCK_METHOD0(shutdown, void());

  MOCK_CONST_METHOD0(isEnabled, bool());
  MOCK_METHOD0(enable, void());
  MOCK_METHOD0(disable, void());
  MOCK_CONST_METHOD1(wantSignal, bool(unsigned long signalType));
  MOCK_METHOD1(pushSignal,
               void(boost::shared_ptr<const Licq::PluginSignal> signal));
  MOCK_METHOD1(pushEvent, void(boost::shared_ptr<const Licq::Event> event));
};

static void nullDeleter(void*) { /* Empty */ }

struct GeneralPluginFixture : public ::testing::Test
{
  DynamicLibrary::Ptr myLib;
  PluginThread::Ptr myThread;
  MockGeneralPluginFactory myMockFactory;
  MockGeneralPlugin myMockInterface;
  GeneralPlugin plugin;
  GeneralPluginInstance instance;

  GeneralPluginFixture() :
    myLib(new DynamicLibrary("")),
    myThread(new PluginThread()),
    plugin(myLib, boost::shared_ptr<MockGeneralPluginFactory>(
               &myMockFactory, &nullDeleter), myThread),
    instance(1, boost::shared_ptr<GeneralPlugin>(&plugin, &nullDeleter),
             myThread)
  {
    EXPECT_CALL(myMockFactory, createPlugin())
        .WillOnce(Return(&myMockInterface));
    EXPECT_CALL(myMockFactory, destroyPlugin(&myMockInterface));

    EXPECT_TRUE(instance.create(NULL));
  }

  ~GeneralPluginFixture()
  {
    myThread->cancel();
  }
};

struct RunnableGeneralPluginFixture
  : public GeneralPluginFixture,
    public ::testing::WithParamInterface<bool>
{
  // Empty
};

TEST_P(RunnableGeneralPluginFixture, callApiFunctions)
{
  using boost::shared_ptr;

  if (GetParam())
    instance.setIsRunning(true);

  unsigned long signalType = 123;
  shared_ptr<const Licq::PluginSignal> signal(
      reinterpret_cast<Licq::PluginSignal*>(456), &nullDeleter);
  shared_ptr<const Licq::Event> event(
      reinterpret_cast<Licq::Event*>(789), &nullDeleter);

  InSequence dummy;
  EXPECT_CALL(myMockFactory, description());
  EXPECT_CALL(myMockFactory, usage());
  EXPECT_CALL(myMockFactory, configFile());
  EXPECT_CALL(myMockInterface, isEnabled());
  if (GetParam())
  {
    EXPECT_CALL(myMockInterface, enable());
    EXPECT_CALL(myMockInterface, disable());
  }
  EXPECT_CALL(myMockInterface, wantSignal(signalType));
  if (GetParam())
  {
    EXPECT_CALL(myMockInterface, pushSignal(signal));
    EXPECT_CALL(myMockInterface, pushEvent(event));
  }

  // Verify that the calls are forwarded to the interface
  plugin.description();
  plugin.usage();
  plugin.configFile();
  instance.isEnabled();
  instance.enable();
  instance.disable();
  instance.wantSignal(signalType);
  instance.pushSignal(signal);
  instance.pushEvent(event);
}

INSTANTIATE_TEST_CASE_P(Running, RunnableGeneralPluginFixture,
                        ::testing::Bool());

} // namespace LicqTest
