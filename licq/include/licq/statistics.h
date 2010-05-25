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

#ifndef LICQ_STATISTICS_H
#define LICQ_STATISTICS_H

#include <boost/noncopyable.hpp>
#include <ctime>
#include <string>

namespace Licq
{

/**
 * Various statistics counters in Licq
 */
class Statistics : private boost::noncopyable
{
public:
  enum Counter
  {
    EventsSentCounter = 0,
    EventsReceivedCounter = 1,
    EventsRejectedCounter = 2,
    AutoResponseCheckedCounter = 3,
  };

  static const int NumCounters = 4;

  /**
   * Get value of a statistics counter
   *
   * @param counter Counter value to get
   * @param today True to only get counter for today, false for total
   * @return Current value of counter
   */
  virtual int get(int counter, bool today) const = 0;

  /**
   * Get name of a counter
   *
   * @param counter Counter to get name for
   * @return Name of counter for use towards user
   */
  virtual std::string name(int counter) const = 0;

  /**
   * Get time of last reset
   *
   * @return Time statistics were last reset
   */
  virtual time_t resetTime() const = 0;

  /**
   * Get time of start
   *
   * @return Time Licq was started
   */
  virtual time_t startTime() const = 0;

  /**
   * Reset all statistics counters
   */
  virtual void reset() = 0;

  /**
   * Increase a counter
   * Note: Should only be called from protocols
   *
   * @param counter Counter to increase (by one)
   */
  virtual void increase(int counter) = 0;

protected:
  virtual ~Statistics() { /* Empty */ }
};

extern Statistics& gStatistics;

} // namespace Licq

#endif
