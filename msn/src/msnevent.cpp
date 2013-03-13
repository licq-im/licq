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

#include "msn.h"
#include "msnevent.h"

#include <licq/logging/log.h>

#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <licq/contactlist/user.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/socket.h>

using namespace LicqMsn;
using Licq::UserId;
using Licq::gLog;
using std::string;

CMSNDataEvent::CMSNDataEvent(CMSN *p)
{
  m_pMSN = p;
  mySocketDesc = NULL;
  m_nEvent = 0;
  m_eState = STATE_WAITING_ACK;
  m_nFileDesc = -1;
  m_strFileName = "";
  m_nFilePos = 0;
  m_nBytesTransferred = 0;
  m_nStartTime = 0;
  m_nSessionId = 0;
  m_nBaseId = 0;
  m_nDataSize[0] = 0;
  m_nDataSize[1] = 0;
  m_strCallId = "";
}

CMSNDataEvent::CMSNDataEvent(unsigned long _nEvent, unsigned long _nSessionId,
    unsigned long _nBaseId, const Licq::UserId& userId, const Licq::UserId& fromId,
    const string &_strCallId, CMSN *p)
  : myUserId(userId),
    myFromId(fromId)
{
  m_pMSN = p;
  mySocketDesc = NULL;
  m_nEvent = _nEvent;
  m_eState = STATE_WAITING_ACK;
  m_nFileDesc = -1;
  {
    Licq::UserReadGuard u(userId);
    m_strFileName = u->pictureFileName();
  }
  m_nFilePos = 0;
  m_nBytesTransferred = 0;
  m_nStartTime = 0;
  m_nSessionId = _nSessionId;
  m_nBaseId = _nBaseId;
  m_nDataSize[0] = 0;
  m_nDataSize[1] = 0;
  m_strCallId = _strCallId;
}

CMSNDataEvent::~CMSNDataEvent()
{
  if (mySocketDesc != NULL)
    m_pMSN->closeSocket(mySocketDesc);

  if (m_nFileDesc)
    close(m_nFileDesc);
}

int CMSNDataEvent::ProcessPacket(CMSNBuffer *p)
{
  unsigned long nSessionId,
    nIdentifier,
    nOffset[2],
    nDataSize[2],
    nLen,
    nFlag,
    nAckId,
    nAckUniqueId,
    nAckSize[2];

  (*p) >> nSessionId >> nIdentifier >> nOffset[0] >> nOffset[1]
    >> nDataSize[0] >> nDataSize[1] >> nLen >> nFlag >> nAckId
    >> nAckUniqueId >> nAckSize[0] >> nAckSize[1];

//  printf("%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
//	 nSessionId, nIdentifier, nOffset[0], nOffset[1],
//	 nDataSize[0], nDataSize[1], nLen, nFlag, nAckId,
//	 nAckUniqueId, nAckSize[0], nAckSize[1]);

  switch (m_eState)
  {
    case STATE_WAITING_ACK:
    {
      if (m_nSessionId == 0)
      {
	if (nFlag == 0x00000002)
	{
	  gLog.info("Display Picture: Ack received");
	}
	else if (nFlag == 0)
	{
	  if (nSessionId)
	    m_nSessionId = nSessionId;
	  else
	  {
	    // Get it from the body
	    int nToRead = strstr(p->getDataPosRead(), "\r\n")+2-p->getDataPosRead();
	    string strStatus = p->unpackRawString(nToRead);
	    if (strStatus != "MSNSLP/1.0 200 OK\r\n")
	    {
	      gLog.error("Display Picture: Encountered an error before the "
                  "session id was received: %s", strStatus.c_str());
	      // close connection
	      return -1;
	    }
	    
	    p->ParseHeaders();
	    string strLen = p->GetValue("Content-Length");
	    int nConLen = atoi(strLen.c_str());
	    if (nConLen)
	    {
	      p->SkipRN();
	      p->ParseHeaders();
	      string strSessId = p->GetValue("SessionID");
	      m_nSessionId = strtoul(strSessId.c_str(), (char **)NULL, 10);
	    }
	  }

	  gLog.info("Display Picture: Session Id received (%ld)",
		    m_nSessionId);
	  CMSNPacket* pAck = new CPS_MSNP2PAck(myUserId.accountId(), m_nSessionId,
					       m_nBaseId-3, nIdentifier, nAckId,
					       nDataSize[1], nDataSize[0]);
          m_pMSN->Send_SB_Packet(myUserId, pAck, mySocketDesc);
	  m_eState = STATE_GOT_SID;
	}
      }


      break;
    }

    case STATE_GOT_SID:
    {
      CMSNPacket* pAck = new CPS_MSNP2PAck(myUserId.accountId(), m_nSessionId,
					   m_nBaseId-2, nIdentifier, nAckId,
					   nDataSize[1], nDataSize[0]);
      m_pMSN->Send_SB_Packet(myUserId, pAck, mySocketDesc);
      m_eState = STATE_RECV_DATA;

      gLog.info("Display Picture: Got data start message (%ld)",
		m_nSessionId);

      m_nFileDesc = open(m_strFileName.c_str(), O_WRONLY | O_CREAT, 00600);
      if (!m_nFileDesc)
      {
	gLog.error("Unable to create a file in your licq directory, check disk space");
	return -1;
      }

      break;
    }

    case STATE_RECV_DATA:
    {
      // Picture data has the 0x20 Flag, so only set data when we get the first picture data packet
      if (m_nDataSize[0] == 0 && nFlag == 0x00000020)
      {
	m_nDataSize[0] = nDataSize[0];
	m_nDataSize[1] = nDataSize[1];
	gLog.info("Display Picture: Expecting file of size %ld (Id: %ld)",
                  m_nDataSize[0], m_nSessionId);
      }

      if (nFlag != 0x00000020)
      {
        gLog.info("Display Picture: Skipping packet without 0x20 flag");
        break;
      }

      ssize_t nWrote = write(m_nFileDesc, p->getDataPosRead(), nLen);
      if (nWrote != (ssize_t)nLen)
      {
	gLog.error("Display Picture: Tried to write %ld, but wrote %ld (Id: %ld)",
		   nLen, (long)nWrote, m_nSessionId);
      }

      m_nBytesTransferred += nLen;

      gLog.info("Display Picture: Wrote %ld of %ld bytes",
                m_nBytesTransferred, m_nDataSize[0]);

      if (m_nBytesTransferred >= m_nDataSize[0])
      {
	if (m_nBytesTransferred == m_nDataSize[0])
	{
	  gLog.info("Display Picture: Successfully completed (%s)",
                    m_strFileName.c_str());
	}
	else
	{
	  gLog.error("Display Picture: Too much data received, ending transfer");
	}
	close(m_nFileDesc);
	m_nFileDesc = -1;
	m_eState = STATE_FINISHED;

        {
          Licq::UserWriteGuard u(myUserId);
          if (u.isLocked())
          {
            u->SetPicturePresent(true);
            Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                Licq::PluginSignal::SignalUser,
                Licq::PluginSignal::UserPicture, u->id()));
          }
        }

	// Ack that we got the data
	CMSNPacket* pAck = new CPS_MSNP2PAck(myUserId.accountId(), m_nSessionId,
					     m_nBaseId-1, nIdentifier, nAckId,
					     nDataSize[1], nDataSize[0]);
        m_pMSN->Send_SB_Packet(myUserId, pAck, mySocketDesc);

        // Send a bye command
        CMSNPacket* pBye = new CPS_MSNP2PBye(myUserId.accountId(), myFromId.accountId(),
            m_strCallId, m_nBaseId, nAckId, nDataSize[1], nDataSize[0]);
        m_pMSN->Send_SB_Packet(myUserId, pBye, mySocketDesc);
	return 0;
      }

      break;
    }

    case STATE_FINISHED:
    {
      // Don't have to send anything back, just return and close the socket.
      gLog.info("Display Picture: closing connection with %s", myUserId.accountId().c_str());
      return 10;
      break;
    }

    case STATE_CLOSED:
    {
      break;
    }
  }

  return 0;
}
