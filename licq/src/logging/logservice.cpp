/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq developers
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

#include "logservice.h"

#include <cassert>

using namespace LicqDaemon;


// Declare global LogService (internal for daemon)
LicqDaemon::LogService LicqDaemon::gLogService;

// Declare global Licq::LogService to refer to the internal LogService
Licq::LogService& Licq::gLogService(LicqDaemon::gLogService);


LogService::LogService() :
  myLog("licq", myLogDistributor)
{
  // Empty
}

LogService::~LogService()
{
  // Empty
}

Log* LogService::getThreadLog() const
{
  return myThreadLogs.get();
}

void LogService::registerDefaultLogSink(Licq::AdjustableLogSink::Ptr logSink)
{
  assert(!myDefaultLogSink);
  myDefaultLogSink = logSink;
  registerLogSink(logSink);
}

Log::Ptr LogService::createLog(const std::string& name)
{
  return Log::Ptr(new Log(name, myLogDistributor));
}

void LogService::createThreadLog(const std::string& name)
{
  myThreadLogs.set(new Log(name, myLogDistributor));
}

void LogService::registerLogSink(Licq::LogSink::Ptr logSink)
{
  myLogDistributor.registerSink(logSink);
}

void LogService::unregisterLogSink(Licq::LogSink::Ptr logSink)
{
  myLogDistributor.unregisterSink(logSink);
  if (logSink == myDefaultLogSink)
    myDefaultLogSink.reset();
}

Licq::AdjustableLogSink::Ptr LogService::getDefaultLogSink()
{
  assert(myDefaultLogSink);
  return myDefaultLogSink;
}
