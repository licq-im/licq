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

#ifndef LICQDAEMON_STATISTICS_H
#define LICQDAEMON_STATISTICS_H

#define SAVE_STATS

#include <licq/statistics.h>

#include <licq/thread/mutex.h>

namespace LicqDaemon
{

class Statistics : public Licq::Statistics
{
public:
  Statistics();
  ~Statistics();

  /**
   * Initialize statistics
   */
  void initialize();

  /**
   * Save statistics counters to config file
   */
  void flush();

  // From Licq::Statistics
  int get(int counter, bool today) const;
  std::string name(int counter) const;
  time_t resetTime() const;
  time_t startTime() const;
  void reset();
  void increase(int counter);

private:
  static const char* const CounterNames[NumCounters];
  static const char* const CounterTags[NumCounters];

  void writeCounters();

  time_t myResetTime;
  time_t myStartTime;
  int myTodayCounters[NumCounters];
  int myTotalCounters[NumCounters];
  bool myWriteNeeded;
  mutable Licq::Mutex myMutex;
};

extern Statistics gStatistics;

} // namespace LicqDaemon

#endif
