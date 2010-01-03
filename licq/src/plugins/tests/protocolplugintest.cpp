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

#include "../protocolplugin.h"

// Both general and protocol plugins declare LP_Id. This hack is needed to be
// able to have both in the same binary.
extern unsigned short LP_Id;
#define LP_Id LP_DummyId
#include "licq_protoplugin.h"
#undef LP_Id

#include <gtest/gtest.h>

// Plugin API functions
#define STR_FUNC(name)                          \
  char* LProto_ ## name()                       \
  { static char name[] = #name; return name; }

STR_FUNC(Name);
STR_FUNC(Version);
STR_FUNC(PPID);

bool LProto_Init()
{
  return true;
}

unsigned long LProto_SendFuncs()
{
  return 42;
}

int LProto_Main(CICQDaemon* /*daemon*/)
{
  return 10;
}

using namespace LicqDaemon;

TEST(ProtocolPlugin, load)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  ASSERT_NO_THROW(ProtocolPlugin plugin(lib));
}

TEST(ProtocolPlugin, callApiFunctions)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  ProtocolPlugin plugin(lib);

  EXPECT_STREQ("Name", plugin.getName());
  EXPECT_STREQ("Version", plugin.getVersion());
  EXPECT_EQ('P' << 24 | 'P' << 16 | 'I' << 8 | 'D', plugin.getProtocolId());
  EXPECT_TRUE(plugin.init());
  EXPECT_EQ(42, plugin.getSendFunctions());
}

TEST(ProtocolPlugin, runPlugin)
{
  DynamicLibrary::Ptr lib(new DynamicLibrary(""));
  ProtocolPlugin plugin(lib);

  plugin.startThread(0);
  EXPECT_EQ(10, plugin.joinThread());
}
