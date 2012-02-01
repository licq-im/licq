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

#ifndef MOCKLOGSINK_H
#define MOCKLOGSINK_H

#include <licq/logging/logsink.h>

#include "gmock/gmock.h"

namespace LicqTest {

class MockLogSink : public Licq::LogSink
{
public:
  MOCK_CONST_METHOD1(isLogging, bool(Licq::Log::Level level));
  MOCK_CONST_METHOD0(isLoggingPackets, bool());
  MOCK_METHOD1(log, void(Message::Ptr message));
};

} // namespace LicqTest

#endif
