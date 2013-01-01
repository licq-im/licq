/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQICQ_SOCKET_H
#define LICQICQ_SOCKET_H

#include <licq/socket.h>

#include <licq/buffer.h>

namespace LicqIcq
{

class SrvSocket : public Licq::INetSocket
{
public:
  SrvSocket(const Licq::UserId& userId);
  virtual ~SrvSocket();

  /// Receive a FLAP packet
  bool receiveFlap(Licq::Buffer& buf);
};


/**
 * Socket for Direct Connections (Client-to-Client)
 */
class DcSocket : public Licq::TCPSocket
{
public:
  enum ChannelType
  {
    ChannelUnknown      = 0,
    ChannelNormal       = 1,
    ChannelInfo         = 2,
    ChannelStatus       = 3,
  };

  DcSocket();
  DcSocket(const Licq::UserId& userId);

  void TransferConnectionFrom(Licq::TCPSocket& from);

  /**
   * Receive a packet without blocking
   * Check RecvBufferFull() on return to determine if packet is complete
   *
   * @return False on any error
   */
  bool RecvPacket();

  void ClearRecvBuffer()
  { myRecvBuffer.Clear(); };

  bool RecvBufferFull() const
  { return myRecvBuffer.Full(); };

  Licq::Buffer& RecvBuffer()
  { return myRecvBuffer; };

  void setChannel(int channel) { myChannel = channel; }
  int channel() const { return myChannel; }

  unsigned long Version() const { return (myVersion); }
  void SetVersion(unsigned long version) { myVersion = version; }

private:
  Licq::Buffer myRecvBuffer;
  int myChannel;
  unsigned short myVersion;
};

} // namespace LicqIcq

#endif
