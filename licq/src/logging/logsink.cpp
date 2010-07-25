/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Erik Johansson <erijo@licq.org>
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

#include <licq/logging/logsink.h>

#include <iomanip>
#include <ostream>
#include <sstream>

using namespace std;

string Licq::packetToString(const uint8_t* packet, size_t size, size_t bytesToPrint)
{
  ostringstream os;
  packetToString(os, packet, size, bytesToPrint);
  return os.str();
}

std::ostream& Licq::packetToString(
    std::ostream& os, const Licq::LogSink::Message& message)
{
  const size_t maxRows = 512;

  return packetToString(os, &(message.packet[0]), message.packet.size(), maxRows*16);
}

ostream& Licq::packetToString(ostream& os, const uint8_t* packet, size_t size, size_t bytesToPrint)
{
  const size_t bytesPerRow = 16;
  const std::string prefix(5, ' ');

  if (bytesToPrint == 0 || bytesToPrint > size)
    bytesToPrint = size;

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
  if (bytesToPrint != size)
  {
    os << "\n" << prefix << setw(4) << setfill('0') << bytesToPrint
       << " - " << setw(4) << size - 1 << ": ...";
  }

  // Restore flags
  os.flags(flags);
  return os;
}
