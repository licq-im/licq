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

#include "statistics.h"

#include <cassert>

#ifdef SAVE_STATS
# include <licq/inifile.h>
#endif
#include <licq/thread/mutexlocker.h>

#include "gettext.h"

using namespace LicqDaemon;


// Declare global Statistics (internal for daemon)
LicqDaemon::Statistics LicqDaemon::gStatistics;

// Declare global Licq::Statistics to refer to the internal Statistics
Licq::Statistics& Licq::gStatistics(LicqDaemon::gStatistics);

const char* const Statistics::CounterNames[Statistics::NumCounters] =
    { tr("Events Sent"), tr("Events Received"), tr("Events Rejected"), tr("Auto Response Checked") };

const char* const Statistics::CounterTags[Statistics::NumCounters] =
    { "Sent", "Recv", "Reject", "ARC" };

Statistics::Statistics()
  : myStartTime(time(NULL)),
    myWriteNeeded(false)
{
  // Empty;
}

Statistics::~Statistics()
{
  // Empty
}

void Statistics::initialize()
{
#ifdef SAVE_STATS
  Licq::IniFile licqConf("licq.conf");
  licqConf.loadFile();
  licqConf.setSection("stats");

  // Use temporary as time_t type might vary and not work with IniFile
  unsigned long resetTime;
  licqConf.get("Reset", resetTime, myStartTime);
  myResetTime = resetTime;

  for (int i = 0; i < NumCounters; ++i)
  {
    myTodayCounters[i] = 0;
    licqConf.get(CounterTags[i], myTotalCounters[i], 0);
  }
#else
  myResetTime = myStartTime;
  for (int i = 0; i < NumCounters; ++i)
  {
    myTodayCounters[i] = 0;
    myTotalCounters[i] = 0;
  }
#endif
}

void Statistics::flush()
{
  Licq::MutexLocker mutexGuard(myMutex);

  if (myWriteNeeded)
    writeCounters();
}

void Statistics::writeCounters()
{
#ifdef SAVE_STATS
  Licq::IniFile licqConf("licq.conf");
  licqConf.loadFile();
  licqConf.setSection("stats");
  licqConf.set("Reset", static_cast<unsigned long>(myResetTime));
  for (int i = 0; i < NumCounters; ++i)
    licqConf.set(CounterTags[i], myTotalCounters[i]);
  if (licqConf.writeFile())
    myWriteNeeded = false;
#else
  myWriteNeeded = false;
#endif
}

int Statistics::get(int counter, bool today) const
{
  assert(counter >= 0 && counter < NumCounters);
  Licq::MutexLocker mutexGuard(myMutex);

  if (today)
    return myTodayCounters[counter];
  else
    return myTotalCounters[counter];
}

std::string Statistics::name(int counter) const
{
  assert(counter >= 0 && counter < NumCounters);
  return CounterNames[counter];
}

time_t Statistics::resetTime() const
{
  return myResetTime;
}

time_t Statistics::startTime() const
{
  return myStartTime;
}

void Statistics::reset()
{
  Licq::MutexLocker mutexGuard(myMutex);

  for (int i = 0; i < NumCounters; ++i)
  {
    myTodayCounters[i] = 0;
    myTotalCounters[i] = 0;
  }
  myResetTime = time(NULL);
  myWriteNeeded = true;
  writeCounters();
}

void Statistics::increase(int counter)
{
  assert(counter >= 0 && counter < NumCounters);
  Licq::MutexLocker mutexGuard(myMutex);

  ++myTodayCounters[counter];
  ++myTotalCounters[counter];
  myWriteNeeded = true;
}
