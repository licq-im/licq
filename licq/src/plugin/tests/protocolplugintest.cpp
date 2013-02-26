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

#include "../protocolplugin.h"
#include "../protocolplugininstance.h"

#include <licq/plugin/protocolpluginfactory.h>
#include <licq/plugin/protocolplugininterface.h>
#include <licq/userid.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using LicqDaemon::ProtocolPlugin;
using LicqDaemon::ProtocolPluginInstance;
using LicqDaemon::DynamicLibrary;
using LicqDaemon::PluginThread;

using ::testing::InSequence;
using ::testing::Return;

namespace LicqTest
{

class MockProtocolPluginFactory : public Licq::ProtocolPluginFactory
{
public:
  MOCK_CONST_METHOD0(name, std::string());
  MOCK_CONST_METHOD0(version, std::string());
  MOCK_METHOD1(destroyPlugin, void(Licq::PluginInterface* plugin));

  MOCK_CONST_METHOD0(protocolId, unsigned long());
  MOCK_CONST_METHOD0(capabilities, unsigned long());
  MOCK_METHOD0(createPlugin, Licq::ProtocolPluginInterface*());
  MOCK_METHOD2(createUser, Licq::User*(const Licq::UserId& id, bool temporary));
  MOCK_METHOD1(createOwner, Licq::Owner*(const Licq::UserId& id));
};

class MockProtocolPlugin : public Licq::ProtocolPluginInterface
{
public:
  MOCK_METHOD2(init, bool(int argc, char** argv));
  MOCK_METHOD0(run, int());
  MOCK_METHOD0(shutdown, void());

  MOCK_METHOD1(pushSignal,
               void(boost::shared_ptr<const Licq::ProtocolSignal> signal));
};

static void nullDeleter(void*) { /* Empty */ }

struct ProtocolPluginFixture : public ::testing::Test
{
  DynamicLibrary::Ptr myLib;
  PluginThread::Ptr myThread;
  MockProtocolPluginFactory myMockFactory;
  MockProtocolPlugin myMockInterface;
  ProtocolPlugin plugin;
  ProtocolPluginInstance instance;

  ProtocolPluginFixture() :
    myLib(new DynamicLibrary("")),
    myThread(new PluginThread()),
    plugin(myLib, boost::shared_ptr<MockProtocolPluginFactory>(
               &myMockFactory, &nullDeleter), myThread),
    instance(1, Licq::UserId(),
             boost::shared_ptr<ProtocolPlugin>(&plugin, &nullDeleter),
             myThread)
  {
    EXPECT_CALL(myMockFactory, createPlugin())
        .WillOnce(Return(&myMockInterface));
    EXPECT_CALL(myMockFactory, destroyPlugin(&myMockInterface));

    EXPECT_TRUE(instance.create(NULL));
  }

  ~ProtocolPluginFixture()
  {
    myThread->cancel();
  }
};

struct RunnableProtocolPluginFixture
  : public ProtocolPluginFixture,
    public ::testing::WithParamInterface<bool>
{
  // Empty
};

TEST_P(RunnableProtocolPluginFixture, callApiFunctions)
{
  if (GetParam())
    instance.setIsRunning(true);

  boost::shared_ptr<const Licq::ProtocolSignal> signal(
      reinterpret_cast<Licq::ProtocolSignal*>(123), &nullDeleter);
  Licq::UserId user;
  Licq::UserId owner;

  InSequence dummy;
  EXPECT_CALL(myMockFactory, protocolId());
  EXPECT_CALL(myMockFactory, capabilities());
  if (GetParam())
    EXPECT_CALL(myMockInterface, pushSignal(signal));
  EXPECT_CALL(myMockFactory, createUser(user, false));
  EXPECT_CALL(myMockFactory, createOwner(owner));

  // Verify that the calls are forwarded to the interface
  plugin.protocolId();
  plugin.capabilities();
  instance.pushSignal(signal);
  plugin.createUser(user, false);
  plugin.createOwner(owner);
}

INSTANTIATE_TEST_CASE_P(Running, RunnableProtocolPluginFixture,
                        ::testing::Bool());

} // namespace LicqTest
