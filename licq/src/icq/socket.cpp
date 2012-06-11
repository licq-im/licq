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
bool SrvSocket::RecvPacket()
{
  if (!myRecvBuffer.Empty())
  {
    gLog.error("Internal error: SrvSocket::RecvPacket(): Called with full buffer (%lu bytes).",
        myRecvBuffer.getDataSize());
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
#if 0
  // JON Recv size does not matter here i believe, m_xRevBuffer can create
  // a large enough packet, if there is enough memory.  It is not a static
  // buffer like RecvRaw
  if (nLen >= MAX_RECV_SIZE) {
    gLog.warning(tr("Server send bad packet with suspiciously large size: %d."), nLen);
    myErrorType = ErrorErrno;
    delete[] buffer;
    return false;
  }
#endif
  // push the 6 bytes at the beginning of the packet again..
  myRecvBuffer.Create(nLen + 6);
  myRecvBuffer.packRaw(buffer, 6);
  delete[] buffer;

  while (!myRecvBuffer.Full())
  {
    // Determine the number of bytes left to be read into the buffer
    unsigned long nBytesLeft = myRecvBuffer.getDataStart() +
        myRecvBuffer.getDataMaxSize() - myRecvBuffer.getDataPosWrite();

    nBytesReceived = read(myDescriptor, myRecvBuffer.getDataPosWrite(), nBytesLeft);
    if (nBytesReceived == 0 || (nBytesReceived < 0 && errno != EINTR))
    {
      myErrorType = ErrorErrno;
      return false;
    }
    myRecvBuffer.incDataPosWrite(nBytesReceived);
  }

  DumpPacket(&myRecvBuffer, true);

  return true;
}
