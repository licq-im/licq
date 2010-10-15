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

#include "../pluginthread.h"

#include <gtest/gtest.h>

using namespace LicqDaemon;

struct PluginThreadFixture : public ::testing::Test
{
  PluginThread thread;
};

static bool init(void* argument)
{
  *static_cast<pthread_t*>(argument) = ::pthread_self();
  return true;
}

static void* start(void* argument)
{
  *static_cast<pthread_t*>(argument) = ::pthread_self();
  return (void*)42;
}

TEST_F(PluginThreadFixture, join)
{
  thread.stop();
  EXPECT_EQ(thread.join(), (void*)NULL);
}

TEST_F(PluginThreadFixture, cancel)
{
  thread.cancel();
  EXPECT_EQ(thread.join(), PTHREAD_CANCELED);
}

TEST_F(PluginThreadFixture, isThread)
{
  EXPECT_FALSE(thread.isThread(::pthread_self()));
}

TEST_F(PluginThreadFixture, loadPlugin)
{
  EXPECT_TRUE(thread.loadPlugin(""));
}

TEST_F(PluginThreadFixture, loadMissingPlugin)
{
  EXPECT_THROW(thread.loadPlugin("nosuchplugin.foo"),
               DynamicLibrary::Exception);
}

TEST_F(PluginThreadFixture, initPlugin)
{
  pthread_t id = ::pthread_self();
  EXPECT_TRUE(thread.initPlugin(&init, &id));
  EXPECT_FALSE(::pthread_equal(id, ::pthread_self()));
  EXPECT_TRUE(thread.isThread(id));
}

TEST_F(PluginThreadFixture, startPlugin)
{
  pthread_t id = ::pthread_self();
  thread.startPlugin(&start, &id);
  EXPECT_EQ(thread.join(), (void*)42);
  EXPECT_FALSE(::pthread_equal(id, ::pthread_self()));
}
