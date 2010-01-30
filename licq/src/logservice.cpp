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

#include "logservice.h"
#include "licq_log.h"

#include <cassert>

using namespace LicqDaemon;

namespace
{

class OldLogServiceProxy : public CLogService
{
public:
  OldLogServiceProxy(Log& log) :
    CLogService(L_ALL),
    myLog(log)
  {
    m_nServiceType = 0xff;
  }

  // From CLogService
  void LogMessage(const char* /*prefix*/, const char* msg,
                  unsigned short type)
  {
    Log::Level level = Log::Unknown;
    switch (type)
    {
      case L_INFO:
        level = Log::Info;
        break;
      case L_UNKNOWN:
        level = Log::Unknown;
        break;
      case L_ERROR:
        level = Log::Error;
        break;
      case L_WARN:
        level = Log::Warning;
        break;
    }

    myLog.log(level, msg);
  }

private:
  Log& myLog;
};

}

static LogService* gLogService = NULL;

LogService& LogService::instance()
{
  assert(gLogService != NULL);
  return *gLogService;
}

LogService::LogService() :
  myLog("licq", myLogDistributor)
{
  assert(gLogService == NULL);
  gLogService = this;

  gLog.AddService(new OldLogServiceProxy(myLog));
}

LogService::~LogService()
{
  gLog.ModifyService(0xff, L_NONE);

  assert(gLogService == this);
  gLogService = NULL;
}

Log* LogService::getPluginLog() const
{
  return myPluginLogs.get();
}

Log::Ptr LogService::createLog(const std::string& name)
{
  return Log::Ptr(new Log(name, myLogDistributor));
}

void LogService::createPluginLog(const std::string& name)
{
  myPluginLogs.set(new Log(name, myLogDistributor));
}

void LogService::registerLogSink(Licq::LogSink::Ptr logSink)
{
  myLogDistributor.registerSink(logSink);
}

void LogService::unregisterLogSink(Licq::LogSink::Ptr logSink)
{
  myLogDistributor.unregisterSink(logSink);
}
