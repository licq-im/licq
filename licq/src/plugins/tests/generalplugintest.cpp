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

TEST(GeneralPlugin, load)
{
  boost::shared_ptr<DynamicLibrary> lib(new DynamicLibrary(""));
  ASSERT_NO_THROW(GeneralPlugin plugin(lib));
}

TEST(GeneralPlugin, callApiFunctions)
{
  boost::shared_ptr<DynamicLibrary> lib(new DynamicLibrary(""));
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

TEST(GeneralPlugin, id)
{
  boost::shared_ptr<DynamicLibrary> lib(new DynamicLibrary(""));
  GeneralPlugin plugin(lib);

  plugin.setId(2);
  EXPECT_EQ(2, plugin.getId());
  EXPECT_EQ(2, LP_Id);
}

TEST(GeneralPlugin, run)
{
  boost::shared_ptr<DynamicLibrary> lib(new DynamicLibrary(""));
  GeneralPlugin plugin(lib);

  plugin.startThread(0);
  EXPECT_EQ(20, plugin.joinThread());
}
