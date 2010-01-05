/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Erik Johansson <erijo@licq.org>
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
#include "licq_plugin.h"

#include <gtest/gtest.h>
#include <list>
#include <pthread.h>

// Deamon stuff
pthread_cond_t LP_IdSignal = PTHREAD_COND_INITIALIZER;
pthread_mutex_t LP_IdMutex = PTHREAD_MUTEX_INITIALIZER;
std::list<unsigned short> LP_Ids;

// Plugin API functions
#define STR_FUNC(name)                          \
  const char* LP_ ## name()                     \
  { static char name[] = #name; return name; }

STR_FUNC(Name);
STR_FUNC(Version);
STR_FUNC(Status);
STR_FUNC(Description);
STR_FUNC(Usage);
STR_FUNC(ConfigFile);

bool LP_Init(int /*argc*/, char** /*argv*/)
{
  return true;
}

int LP_Main(CICQDaemon* /*daemon*/)
{
  return 20;
}

using namespace LicqDaemon;

static char getPipeChar(const Plugin& plugin)
{
  int fd = plugin.getReadPipe();
  char ch;
  read(fd, &ch, sizeof(ch));
  return ch;
}

TEST(GeneralPlugin, load)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  ASSERT_NO_THROW(GeneralPlugin plugin(lib));
}

TEST(GeneralPlugin, callApiFunctions)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  GeneralPlugin plugin(lib);

  EXPECT_STREQ("Name", plugin.getName());
  EXPECT_STREQ("Version", plugin.getVersion());
  EXPECT_STREQ("Status", plugin.getStatus());
  EXPECT_STREQ("Description", plugin.getDescription());
  EXPECT_STREQ("Usage", plugin.getUsage());
  EXPECT_STREQ("ConfigFile", plugin.getConfigFile());
  EXPECT_TRUE(plugin.getBuildDate());
  EXPECT_TRUE(plugin.getBuildTime());

  EXPECT_TRUE(plugin.init(0, 0));
}

TEST(GeneralPlugin, runPlugin)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  GeneralPlugin plugin(lib);

  plugin.startThread(0);
  EXPECT_EQ(20, plugin.joinThread());
}

TEST(GeneralPlugin, enableDisable)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  GeneralPlugin plugin(lib);

  plugin.enable();
  EXPECT_EQ('1', getPipeChar(plugin));
  plugin.disable();
  EXPECT_EQ('0', getPipeChar(plugin));
}

TEST(GeneralPlugin, pushPopSignal)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  GeneralPlugin plugin(lib);

  LicqSignal* signal = (LicqSignal*)10;
  plugin.pushSignal(signal);
  plugin.pushSignal(signal);

  EXPECT_EQ('S', getPipeChar(plugin));
  EXPECT_EQ(signal, plugin.popSignal());

  EXPECT_EQ('S', getPipeChar(plugin));
  EXPECT_EQ(signal, plugin.popSignal());
}

TEST(GeneralPlugin, popSignalEmpty)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  GeneralPlugin plugin(lib);

  EXPECT_EQ(NULL, plugin.popSignal());
}

TEST(GeneralPlugin, pushPopEvent)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  GeneralPlugin plugin(lib);

  LicqEvent* event = (LicqEvent*)20;
  plugin.pushEvent(event);
  plugin.pushEvent(event);

  EXPECT_EQ('E', getPipeChar(plugin));
  EXPECT_EQ(event, plugin.popEvent());

  EXPECT_EQ('E', getPipeChar(plugin));
  EXPECT_EQ(event, plugin.popEvent());
}

TEST(GeneralPlugin, popEventEmpty)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  GeneralPlugin plugin(lib);

  EXPECT_EQ(NULL, plugin.popEvent());
}
