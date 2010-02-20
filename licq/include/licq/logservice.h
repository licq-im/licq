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

#ifndef LICQ_LOGSERVICE_H
#define LICQ_LOGSERVICE_H

#include "log.h"
#include "logsink.h"

#include <boost/noncopyable.hpp>

namespace Licq
{

class LogService : private boost::noncopyable
{
public:
  virtual Log::Ptr createLog(const std::string& name) = 0;
  virtual void createThreadLog(const std::string& name) = 0;

  virtual void registerLogSink(LogSink::Ptr logSink) = 0;
  virtual void unregisterLogSink(LogSink::Ptr logSink) = 0;

protected:
  virtual ~LogService() { /* Empty */ }
};

} // namespace Licq

#endif
