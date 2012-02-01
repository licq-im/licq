/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010,2012 Licq Developers <licq-dev@googlegroups.com>
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

#include <licq/thread/condition.h>

#include <gtest/gtest.h>

using namespace Licq;

namespace LicqTest {

TEST(Condition, waitWithTimeout)
{
  Mutex mutex;
  Condition condition;

  mutex.lock();
  EXPECT_FALSE(condition.wait(mutex, 0));
  EXPECT_FALSE(condition.wait(mutex, 200));
  mutex.unlock();
}

struct ThreadData
{
  Mutex myMutex;
  Condition myCondition;
};

static void* threadEntry(void* ptr)
{
  ThreadData* data = static_cast<ThreadData*>(ptr);
  data->myMutex.lock();

  // Notify main thread that this thread is running
  data->myCondition.signal();

  // Wait for thread to be cancelled
  data->myCondition.wait(data->myMutex);

  return NULL;
}

TEST(Condition, mutexUnlockedWhenCancelled)
{
  ThreadData data;
  data.myMutex.lock();

  // Create thread and wait for it to start
  pthread_t thread;
  pthread_create(&thread, NULL, threadEntry, &data);
  data.myCondition.wait(data.myMutex);
  data.myMutex.unlock();

  // Cancel thread
  pthread_cancel(thread);
  pthread_join(thread, NULL);

  EXPECT_TRUE(data.myMutex.tryLock());
  data.myMutex.unlock();
}

} // namespace LicqTest
