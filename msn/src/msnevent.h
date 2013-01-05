/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2005-2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQMSN_MSNEVENT_H
#define LICQMSN_MSNEVENT_H

#include <string>

#include <licq/userid.h>

#include "msnpacket.h"

namespace Licq
{
class TCPSocket;
class UserId;
}

namespace LicqMsn
{

class CMSN;

enum ESTATE
{
  STATE_WAITING_ACK = 0,
  STATE_GOT_SID,
  STATE_RECV_DATA,
  STATE_FINISHED,
  STATE_CLOSED
};

class CMSNDataEvent
{
public:
  CMSNDataEvent(CMSN *);
  CMSNDataEvent(unsigned long event, unsigned long sessionId,
      unsigned long baseId, const Licq::UserId& userId, const Licq::UserId& fromId,
      const std::string& callId, CMSN* p);
  ~CMSNDataEvent();

  int ProcessPacket(CMSNBuffer *);

  Licq::TCPSocket* getSocket() { return mySocketDesc; }
  const Licq::UserId& userId() const { return myUserId; }
  unsigned long getSessionId() { return m_nSessionId; }
  unsigned long getBaseId() { return m_nBaseId; }

  void setSocket(Licq::TCPSocket* s) { mySocketDesc = s; }

protected:
  CMSN *m_pMSN;

  Licq::TCPSocket* mySocketDesc;
  unsigned long m_nEvent;
  Licq::UserId myUserId;
  Licq::UserId myFromId;
  std::string m_strCallId;
  ESTATE m_eState;
  int m_nFileDesc;
  std::string m_strFileName;
  unsigned long m_nFilePos,
    m_nBytesTransferred;
  time_t m_nStartTime;

  // Binary header
  unsigned long m_nSessionId;
  unsigned long m_nBaseId;
  unsigned long m_nDataSize[2];
};

} // namespace LicqMsn

#endif
