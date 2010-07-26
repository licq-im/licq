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

#include <licq/logging/logutils.h>

#include <cstdio>
#include <iomanip>
#include <sstream>

extern const unsigned int PacketBit;

using namespace Licq;

bool LogUtils::levelInBitmask(Log::Level level, unsigned int bitmask)
{
  return bitmask & (1 << level);
}

bool LogUtils::packetInBitmask(unsigned int bitmask)
{
  return bitmask & PacketBit;
}

unsigned int LogUtils::convertOldBitmaskToNew(int levels)
{
  unsigned int mask = 0;
  mask |= (levels & 0x1) ? (1 << Log::Info) : 0;
  mask |= (levels & 0x2) ? (1 << Log::Unknown) : 0;
  mask |= (levels & 0x4) ? (1 << Log::Error) : 0;
  mask |= (levels & 0x8) ? (1 << Log::Warning) : 0;

  mask |= (levels & 0x10) ? (1 << Log::Debug) : 0;
  mask |= (levels & 0x10) ? PacketBit : 0;

  return mask;
}

const char* LogUtils::levelToString(Log::Level level)
{
  switch (level)
  {
    case Log::Unknown:
      return "unknown";
    case Log::Info:
      return "info";
    case Log::Warning:
      return "warning";
    case Log::Error:
      return "error";
    case Log::Debug:
      return "debug";
  }
  return "?????";
}

const char* LogUtils::levelToShortString(Log::Level level)
{
  switch (level)
  {
    case Log::Unknown:
      return "UNK";
    case Log::Info:
      return "INF";
    case Log::Warning:
      return "WAR";
    case Log::Error:
      return "ERR";
    case Log::Debug:
      return "DBG";
  }
  return "???";
}

std::string LogUtils::timeToString(const LogSink::Message::Time& msgTime)
{
  time_t t = msgTime.sec;
  tm time;
  ::localtime_r(&t, &time);

  char buffer[8 + 4 + 1];
  ::strftime(buffer, 8 + 1, "%H:%M:%S", &time);
  ::snprintf(&buffer[8], 4 + 1, ".%03u", msgTime.msec);

  return buffer;
}

static std::ostream& packetToString(
    std::ostream& os, LogSink::Message::Ptr message)
{
  const std::vector<uint8_t>& packet = message->packet;

  const size_t bytesPerRow = 16;
  const size_t maxRows = 512;
  const size_t bytesToPrint = std::min(bytesPerRow * maxRows, packet.size());

  const std::string prefix(5, ' ');

  char ascii[bytesPerRow + 1];
  ascii[bytesPerRow] = '\0';

  // Save current flags
  const std::ios_base::fmtflags flags = os.flags();

  os << std::hex << std::uppercase;
  using std::setw; using std::setfill;

  for (size_t addr = 0; addr < bytesToPrint; addr++)
  {
    const size_t pos = addr % bytesPerRow;

    // Print the address at the start of the row
    if (pos == 0)
      os << prefix << setw(4) << setfill('0') << addr << ':';
    // Or an extra space in the middle
    else if (pos == (bytesPerRow / 2))
      os << ' ';

    const uint8_t byte = packet[addr];

    // Print the byte in hex
    os << ' ' << setw(2) << static_cast<uint16_t>(byte);

    // Save the ascii representation (if available; otherwise '.')
    ascii[pos] = std::isprint(byte) ? byte : '.';

    // Print the ascii version at the end of the row
    if (pos + 1 == bytesPerRow || addr + 1 == bytesToPrint)
    {
      ascii[pos + 1] = '\0';

      // Number of bytes needed for a "full" row
      const size_t padBytes = bytesPerRow - (pos + 1);

      // Print 3 spaces for each missing byte
      size_t padding = padBytes * 3;

      // Add one extra space to compensate for the extra space in the middle
      if (pos <= (bytesPerRow / 2))
        padding += 1;

      // Separate hex and ascii version with 3 spaces
      padding += 3;

      os << std::string(padding, ' ') << ascii;

      // Print newline on all but the last line
      if (addr + 1 != bytesToPrint)
        os << "\n";
    }
  }

  // Print the address range for bytes not printed
  if (bytesToPrint != packet.size())
  {
    os << "\n" << prefix << setw(4) << setfill('0') << bytesToPrint
       << " - " << setw(4) << packet.size() - 1 << ": ...";
  }

  // Restore flags
  os.flags(flags);
  return os;
}

std::string LogUtils::packetToString(LogSink::Message::Ptr message)
{
  std::ostringstream ss;
  ::packetToString(ss, message);
  return ss.str();
}
