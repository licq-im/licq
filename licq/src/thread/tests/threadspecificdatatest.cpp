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

#include <licq/thread/threadspecificdata.h>

#include <gtest/gtest.h>

using namespace Licq;

TEST(ThreadSpecificData, create)
{
  ThreadSpecificData<int> data;
}

TEST(ThreadSpecificData, setAndGet)
{
  ThreadSpecificData<int> data;
  data.set(new int(5));
  EXPECT_EQ(5, *data.get());

  // Cleanup
  data.set(0);
}

struct DeleteLogger
{
  static int myCount;
  ~DeleteLogger() { myCount += 1; }
};

int DeleteLogger::myCount = 0;

static void* threadEntry(void* data)
{
  static_cast<ThreadSpecificData<DeleteLogger>*>(data)->
      set(new DeleteLogger());
  return 0;
}

TEST(ThreadSpecificData, dataDeletedWhenThreadExits)
{
  ThreadSpecificData<DeleteLogger> logger;
  DeleteLogger::myCount = 0;

  pthread_t thread;
  ::pthread_create(&thread, 0, &threadEntry, &logger);
  ::pthread_join(thread, 0);

  EXPECT_EQ(1, DeleteLogger::myCount);
}
