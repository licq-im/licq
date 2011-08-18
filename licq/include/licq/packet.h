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
#ifndef LICQ_PACKET_H
#define LICQ_PACKET_H

#include <cstdlib> // NULL

class INetSocket;

namespace Licq
{
class Buffer;

class Packet
{
public:
  virtual ~Packet();

  Buffer *getBuffer()
  { return buffer; };

  virtual Buffer* Finalize(INetSocket*)
  { return NULL; }

  virtual unsigned short Sequence() = 0;
  virtual unsigned short SubSequence() = 0;
  virtual unsigned short SubCommand() = 0;

  virtual unsigned long  SNAC()      { return 0; }
  virtual unsigned short ExtraInfo() { return 0; }

  static void SetLocalIp(unsigned long n)  {  s_nLocalIp = n; }
  static void SetLocalPort(unsigned short n)  {  s_nLocalPort = n; }
  static void SetRealIp(unsigned long n)  {  s_nRealIp = n; }
  static bool Firewall() { return s_nLocalIp != s_nRealIp; }
  static unsigned long RealIp() { return s_nRealIp; }
  static unsigned long LocalIp() { return s_nLocalIp; }

protected:
  Packet();

  Buffer* buffer;
  unsigned short m_nSize;

  static unsigned long s_nLocalIp;
  static unsigned long s_nRealIp;
  static unsigned short s_nLocalPort;
};

} // namespace Licq

#endif
