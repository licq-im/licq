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

#include "socket.h"

#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <licq/buffer.h>
#include <licq/logging/log.h>

#include "../gettext.h"

using namespace LicqIcq;
using Licq::Buffer;
using Licq::gLog;

SrvSocket::SrvSocket(const Licq::UserId& userId)
  : Licq::INetSocket(SOCK_STREAM, "SRV", userId)
{
  // Empty
}

SrvSocket::~SrvSocket()
{
  // Empty
}

/*-----SrvSocket::ReceivePacket------------------------------------------------
 * Receive data on the socket.  Checks the buffer to see if it is empty, if
 * so, then it will create it using either the size read in from the socket
 * (the first two bytes available) or the given size.
 *---------------------------------------------------------------------------*/
bool SrvSocket::receiveFlap(Licq::Buffer& buf)
{
  if (!buf.Empty())
  {
    gLog.error("Internal error: SrvSocket::RecvPacket(): Called with full buffer (%lu bytes).",
        buf.getDataSize());
    return true;
  }

  int nBytesReceived = 0;
  errno = 0;

  // Check if the buffer is empty
  char *buffer = new char[6];
  int nSixBytes = 0;
  while (nSixBytes != 6)
  {
    nBytesReceived = read(myDescriptor, buffer + nSixBytes, 6 - nSixBytes);
    if (nBytesReceived <= 0)
    {
      if (nBytesReceived == 0)
        gLog.warning(tr("server socket was closed!!!\n"));
      else
      {
        myErrorType = ErrorErrno;
        gLog.warning(tr("Error during receiving from server socket:\n%s"),
            errorStr().c_str());
      }
      delete[] buffer;
      return false;
    }
    nSixBytes += nBytesReceived;
  }

  // now we start to verify the FLAP header
  if (buffer[0] != 0x2a)
  {
    gLog.warning("Server send bad packet start code: %d.", buffer[0]);
    gLog.warning("Sixbyte: %02x %02x %02x %02x %02x %02x",
        buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
    myErrorType = ErrorErrno;
    delete[] buffer;
    return false;
  }

  // DAW maybe verify sequence number ?

  unsigned short nLen = ((unsigned char)buffer[5]) + (((unsigned char)buffer[4]) << 8);

  // push the 6 bytes at the beginning of the packet again..
  buf.Create(nLen + 6);
  buf.packRaw(buffer, 6);
  delete[] buffer;

  while (!buf.Full())
  {
    ssize_t bytesReceived = read(myDescriptor, buf.getDataPosWrite(), buf.remainingDataToWrite());
    if (bytesReceived == 0 || (bytesReceived < 0 && errno != EINTR))
    {
      myErrorType = ErrorErrno;
      return false;
    }
    buf.incDataPosWrite(bytesReceived);
  }

  DumpPacket(&buf, true);

  return true;
}

DcSocket::DcSocket(const Licq::UserId& userId)
  : TCPSocket(userId),
    myVersion(0)
{
  // Empty
}

DcSocket::DcSocket()
{
  // Empty
}

void DcSocket::TransferConnectionFrom(Licq::TCPSocket& from)
{
  DcSocket* dcfrom = dynamic_cast<DcSocket*>(&from);
  if (dcfrom != NULL)
  {
    myVersion = dcfrom->myVersion;
  }

  Licq::TCPSocket::TransferConnectionFrom(from);
}

bool DcSocket::RecvPacket()
{
  if (myRecvBuffer.Full())
    return true;

  if (myRecvBuffer.Empty() || myRecvBuffer.getDataSize() < 2)
  {
    // Get header for next packet
    if (!receive(myRecvBuffer, 2, false))
      return false;

    if (myRecvBuffer.getDataSize() < 2)
    {
      // Didn't get a full header, retry later
      return true;
    }

    // Parse packet header
    int length = myRecvBuffer.unpackUInt16LE();

    // Handle empty packets in case we ever get one
    if (length == 0)
    {
      DumpPacket(&myRecvBuffer, true);
      return true;
    }

    // Resize buffer to hold entire packet including the header
    myRecvBuffer.Create(length + 2);
    myRecvBuffer.packUInt16LE(length);
  }

  if (!receive(myRecvBuffer, myRecvBuffer.remainingDataToWrite()))
    return false;

  return true;
}
