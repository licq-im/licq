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

#ifndef LICQDAEMON_LOGSERVICE_H
#define LICQDAEMON_LOGSERVICE_H

#include "licq/logservice.h"
#include "licq/thread/threadspecificdata.h"
#include "utils/log.h"
#include "utils/logdistributor.h"

namespace LicqDaemon
{

class LogService : public Licq::LogService
{
public:
  static LogService& instance();

  LogService();
  ~LogService();

  Log& getLog() { return myLog; }
  Log* getPluginLog() const;

  // From Licq::LogService
  Licq::Log::Ptr createLog(const std::string& name);
  void createPluginLog(const std::string& name);
  void registerLogSink(Licq::LogSink::Ptr logSink);
  void unregisterLogSink(Licq::LogSink::Ptr logSink);

private:
  LogDistributor myLogDistributor;
  Log myLog;
  Licq::ThreadSpecificData<Log> myPluginLogs;
};

} // namespace LicqDaemon

#endif
