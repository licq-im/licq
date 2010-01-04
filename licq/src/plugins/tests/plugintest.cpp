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

#include "../plugin.h"

#include <gtest/gtest.h>

extern "C" {

// Daemon stuff
extern pthread_cond_t LP_IdSignal;
extern pthread_mutex_t LP_IdMutex;
extern std::list<unsigned short> LP_Ids;

// Plugin API functions
#define STR_FUNC(name)                          \
  const char* Test_ ## name()                   \
  { static char name[] = #name; return name; }

STR_FUNC(Name);
STR_FUNC(Version);

int Test_Main(CICQDaemon* /*daemon*/)
{
  return 5;
}

void Test_Exit(int result)
{
  int *p = (int *)malloc(sizeof(int));
  *p = result;
  pthread_mutex_lock(&LP_IdMutex);
  LP_Ids.push_back(1);
  pthread_mutex_unlock(&LP_IdMutex);
  pthread_cond_signal(&LP_IdSignal);
  pthread_exit(p);
}

void* Test_Main_tep(void* daemon)
{
  Test_Exit(Test_Main((CICQDaemon*)daemon));
  return NULL;
}

extern unsigned short LP_Id;

} // extern "C"

using namespace LicqDaemon;

static char getPipeChar(const Plugin& plugin)
{
  int fd = plugin.getReadPipe();
  char ch;
  read(fd, &ch, sizeof(ch));
  return ch;
}

TEST(Plugin, load)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  ASSERT_NO_THROW(Plugin plugin(lib, "Test"));
}

TEST(Plugin, callApiFunctions)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  Plugin plugin(lib, "Test");

  EXPECT_STREQ("Name", plugin.getName());
  EXPECT_STREQ("Version", plugin.getVersion());
  EXPECT_EQ("", plugin.getLibraryName());
}

TEST(Plugin, getSetId)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  Plugin plugin(lib, "Test");

  plugin.setId(1);
  EXPECT_EQ(1, plugin.getId());
  EXPECT_EQ(1, LP_Id);
}

TEST(Plugin, runPlugin)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  Plugin plugin(lib, "Test");

  plugin.startThread(0);
  EXPECT_FALSE(plugin.isThisThread());
  EXPECT_EQ(5, plugin.joinThread());
}

TEST(Plugin, shutdown)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  Plugin plugin(lib, "Test");

  plugin.shutdown();
  EXPECT_EQ('X', getPipeChar(plugin));
}

TEST(Plugin, pushPopSignal)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  Plugin plugin(lib, "Test");

  LicqSignal* signal = (LicqSignal*)10;
  plugin.pushSignal(signal);
  plugin.pushSignal(signal);

  EXPECT_EQ('S', getPipeChar(plugin));
  EXPECT_EQ(signal, plugin.popSignal());

  EXPECT_EQ('S', getPipeChar(plugin));
  EXPECT_EQ(signal, plugin.popSignal());
}

TEST(Plugin, popSignalEmpty)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  Plugin plugin(lib, "Test");

  EXPECT_EQ(NULL, plugin.popSignal());
}
