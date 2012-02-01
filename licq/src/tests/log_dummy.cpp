/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2011-2012 Licq Developers <licq-dev@googlegroups.com>
 *
 * Please refer to the COPYRIGHT file distributed with this source
 * distribution for the names of the individual contributors.
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

#include <licq/logging/log.h>

namespace LicqTest {

class LogDummy : public Licq::Log
{
public:
  // Licq::Log
  void log(Level /*level*/, const std::string& /*msg*/) {}
  void packet(Level /*level*/, const uint8_t* /*data*/, size_t /*size*/,
              const std::string& /*msg*/) {}
};

} // namespace LicqTest

Licq::ThreadLog Licq::gLog;

Licq::Log* Licq::ThreadLog::getLog()
{
  static LicqTest::LogDummy log;
  return &log;
}
