/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq Developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_LOGUTILS_H
#define LICQ_LOGUTILS_H

#include "log.h"
#include "logsink.h"

#include <string>

namespace Licq
{

namespace LogUtils
{

bool levelInBitmask(Log::Level level, unsigned int bitmask);
bool packetInBitmask(unsigned int bitmask);

/**
 * Converts a bitmask in the old format to a new that can be passed to
 * AdjustableLogSink::setLogLevelsFromBitmask().
 * @param levels A bitmask indicating which levels to log:
 *   0x01 - Log::Info
 *   0x02 - Log::Unknown
 *   0x04 - Log::Error
 *   0x08 - Log::Warning
 *   0x10 - Log::Debug and packets
 */
unsigned int convertOldBitmaskToNew(int levels);

const char* levelToString(Log::Level level);
const char* levelToShortString(Log::Level level);
std::string timeToString(const LogSink::Message::Time& msgTime);

/**
 * Pretty-print a packet to a string
 *
 * @param message Message with the packet to print
 * @return A printable string containing the packet data
 */
std::string packetToString(LogSink::Message::Ptr message);

} // namespace LogUtils

} // namespace Licq

#endif
