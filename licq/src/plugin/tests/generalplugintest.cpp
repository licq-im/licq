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

#include <licq/plugin/generalplugininterface.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using LicqDaemon::GeneralPlugin;
using LicqDaemon::DynamicLibrary;
using LicqDaemon::PluginThread;

using ::testing::InSequence;

namespace LicqTest
{

class MockGeneralPlugin : public Licq::GeneralPluginInterface
{
public:
  MOCK_CONST_METHOD0(name, std::string());
  MOCK_CONST_METHOD0(version, std::string());
  MOCK_METHOD2(init, bool(int argc, char** argv));
  MOCK_METHOD0(run, int());
  MOCK_METHOD0(shutdown, void());
  MOCK_METHOD0(destructor, void());

  MOCK_CONST_METHOD0(description, std::string());
  MOCK_CONST_METHOD0(usage, std::string());
  MOCK_CONST_METHOD0(configFile, std::string());
  MOCK_CONST_METHOD0(isEnabled, bool());
  MOCK_METHOD0(enable, void());
  MOCK_METHOD0(disable, void());
  MOCK_CONST_METHOD1(wantSignal, bool(unsigned long signalType));
  MOCK_METHOD1(pushSignal, void(Licq::PluginSignal* signal));
  MOCK_METHOD1(pushEvent, void(Licq::Event* event));
};

static void NullDeleter(void*) { /* Empty */ }

struct GeneralPluginFixture : public ::testing::Test
{
  DynamicLibrary::Ptr myLib;
  PluginThread::Ptr myThread;
  MockGeneralPlugin myMockInterface;
  GeneralPlugin plugin;

  GeneralPluginFixture() :
    myLib(new DynamicLibrary("")),
    myThread(new PluginThread()),
    plugin(1, myLib, myThread,
           boost::shared_ptr<MockGeneralPlugin>(
               &myMockInterface, &NullDeleter))
  {
    // Empty
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
  if (GetParam())
    plugin.setIsRunning(true);

  unsigned long signalType = 123;
  Licq::PluginSignal* signal = reinterpret_cast<Licq::PluginSignal*>(456);
  Licq::Event* event = reinterpret_cast<Licq::Event*>(789);

  InSequence dummy;
  EXPECT_CALL(myMockInterface, description());
  EXPECT_CALL(myMockInterface, usage());
  EXPECT_CALL(myMockInterface, configFile());
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
  plugin.isEnabled();
  plugin.enable();
  plugin.disable();
  plugin.wantSignal(signalType);
  plugin.pushSignal(signal);
  plugin.pushEvent(event);
}

INSTANTIATE_TEST_CASE_P(Running, RunnableGeneralPluginFixture,
                        ::testing::Bool());

} // namespace LicqTest
