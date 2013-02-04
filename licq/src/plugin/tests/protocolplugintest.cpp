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

#include <licq/plugin/protocolplugininterface.h>
#include <licq/userid.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using LicqDaemon::ProtocolPlugin;
using LicqDaemon::DynamicLibrary;
using LicqDaemon::PluginThread;

using ::testing::InSequence;

namespace LicqTest
{

class MockProtocolPlugin : public Licq::ProtocolPluginInterface
{
public:
  MOCK_CONST_METHOD0(name, std::string());
  MOCK_CONST_METHOD0(version, std::string());
  MOCK_METHOD2(init, bool(int argc, char** argv));
  MOCK_METHOD0(run, int());
  MOCK_METHOD0(shutdown, void());
  MOCK_METHOD0(destructor, void());

  MOCK_CONST_METHOD0(protocolId, unsigned long());
  MOCK_CONST_METHOD0(capabilities, unsigned long());
  MOCK_METHOD1(pushSignal,
               void(boost::shared_ptr<const Licq::ProtocolSignal> signal));
  MOCK_METHOD2(createUser, Licq::User*(const Licq::UserId& id, bool temporary));
  MOCK_METHOD1(createOwner, Licq::Owner*(const Licq::UserId& id));
};

static void NullDeleter(void*) { /* Empty */ }

struct ProtocolPluginFixture : public ::testing::Test
{
  DynamicLibrary::Ptr myLib;
  PluginThread::Ptr myThread;
  MockProtocolPlugin myMockInterface;
  ProtocolPlugin plugin;

  ProtocolPluginFixture() :
    myLib(new DynamicLibrary("")),
    myThread(new PluginThread()),
    plugin(1, myLib, myThread,
           boost::shared_ptr<MockProtocolPlugin>(
               &myMockInterface, &NullDeleter))
  {
    // Empty
  }

  ~ProtocolPluginFixture()
  {
    myThread->cancel();
  }
};

TEST_F(ProtocolPluginFixture, callApiFunctions)
{
  boost::shared_ptr<const Licq::ProtocolSignal> signal(
      reinterpret_cast<Licq::ProtocolSignal*>(123), &NullDeleter);
  Licq::UserId user;
  Licq::UserId owner;

  InSequence dummy;
  EXPECT_CALL(myMockInterface, protocolId());
  EXPECT_CALL(myMockInterface, capabilities());
  EXPECT_CALL(myMockInterface, pushSignal(signal));
  EXPECT_CALL(myMockInterface, createUser(user, false));
  EXPECT_CALL(myMockInterface, createOwner(owner));

  // Verify that the calls are forwarded to the interface
  plugin.protocolId();
  plugin.capabilities();
  plugin.pushSignal(signal);
  plugin.createUser(user, false);
  plugin.createOwner(owner);
}

} // namespace LicqTest
