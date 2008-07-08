#include "msn.h"
#include "msnevent.h"

#include "licq_constants.h"
#include "licq_log.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

CMSNDataEvent::CMSNDataEvent(CMSN *p)
{
  m_pMSN = p;
  m_nSocketDesc = -1;
  m_nEvent = 0;
  m_strId = "";
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
  m_strFromId = "";
  m_strCallId = "";
}

CMSNDataEvent::CMSNDataEvent(unsigned long _nEvent, unsigned long _nSessionId,
			     unsigned long _nBaseId, const string &_strId,
			     const string &_strFromId, const string &_strCallId,
                             CMSN *p)
{
  m_pMSN = p;
  m_nSocketDesc = -1;
  m_nEvent = _nEvent;
  m_strId = _strId;
  m_eState = STATE_WAITING_ACK;
  m_nFileDesc = -1;
  m_strFileName = BASE_DIR;
  m_strFileName += "/";
  m_strFileName += USER_DIR;
  m_strFileName += "/" +_strId + ".pic";
  m_nFilePos = 0;
  m_nBytesTransferred = 0;
  m_nStartTime = 0;
  m_nSessionId = _nSessionId;
  m_nBaseId = _nBaseId;
  m_nDataSize[0] = 0;
  m_nDataSize[1] = 0;
  m_strFromId = _strFromId;
  m_strCallId = _strCallId;
}

CMSNDataEvent::~CMSNDataEvent()
{
  if (m_nSocketDesc)
  {
    INetSocket *s = gSocketMan.FetchSocket(m_nSocketDesc);
    gSocketMan.DropSocket(s);
    gSocketMan.CloseSocket(m_nSocketDesc);
  }

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
	  gLog.Info("%sDisplay Picture: Ack received\n", L_MSNxSTR);
	}
	else if (nFlag == 0)
	{
	  if (nSessionId)
	    m_nSessionId = nSessionId;
	  else
	  {
	    // Get it from the body
	    char szStatusLine[128];
	    int nToRead = strstr(p->getDataPosRead(), "\r\n")+2-p->getDataPosRead();
	    if (nToRead > 128)
	    {
	      gLog.Warn("%sDisplay Picture: Received unusually long status line, aborting\n", L_WARNxSTR);
	      // close connection
	      return -1;
	    }
	    p->UnpackRaw(szStatusLine, nToRead);
	    string strStatus(szStatusLine);
	    if (strStatus != "MSNSLP/1.0 200 OK\r\n")
	    {
	      gLog.Error("%sDisplay Picture: Encountered an error before the session id was received: %s", L_ERRORxSTR, szStatusLine);
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

	  gLog.Info("%sDisplay Picture: Session Id received (%ld)\n",
		    L_MSNxSTR, m_nSessionId);
	  CMSNPacket *pAck = new CPS_MSNP2PAck(m_strId.c_str(), m_nSessionId,
					       m_nBaseId-3, nIdentifier, nAckId,
					       nDataSize[1], nDataSize[0]);
	  m_pMSN->Send_SB_Packet(m_strId, pAck, m_nSocketDesc);
	  m_eState = STATE_GOT_SID;
	}
      }


      break;
    }

    case STATE_GOT_SID:
    {
      CMSNPacket *pAck = new CPS_MSNP2PAck(m_strId.c_str(), m_nSessionId,
					   m_nBaseId-2, nIdentifier, nAckId,
					   nDataSize[1], nDataSize[0]);
      m_pMSN->Send_SB_Packet(m_strId, pAck, m_nSocketDesc);
      m_eState = STATE_RECV_DATA;

      gLog.Info("%sDisplay Picture: Got data start message (%ld)\n",
		L_MSNxSTR, m_nSessionId);

      m_nFileDesc = open(m_strFileName.c_str(), O_WRONLY | O_CREAT, 00600);
      if (!m_nFileDesc)
      {
	gLog.Error("%sUnable to create a file in your licq directory, check disk space.\n",
		   L_ERRORxSTR);
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
	gLog.Info("%sDisplay Picture: Expecting file of size %ld (Id: %ld).\n",
		  L_MSNxSTR, m_nDataSize[0], m_nSessionId);
      }

      if (nFlag != 0x00000020)
      {
        gLog.Info("%sDisplay Picture: Skipping packet without 0x20 flag.\n", L_MSNxSTR);
        break;
      }

      ssize_t nWrote = write(m_nFileDesc, p->getDataPosRead(), nLen);
      if (nWrote != (ssize_t)nLen)
      {
	gLog.Error("%sDisplay Picture: Tried to write %ld, but wrote %ld (Id: %ld).\n",
		   L_MSNxSTR, nLen, (long)nWrote, m_nSessionId);
      }

      m_nBytesTransferred += nLen;

      gLog.Info("%sDisplay Picture: Wrote %ld of %ld bytes.\n",
          L_MSNxSTR, m_nBytesTransferred, m_nDataSize[0]);

      if (m_nBytesTransferred >= m_nDataSize[0])
      {
	if (m_nBytesTransferred == m_nDataSize[0])
	{
	  gLog.Info("%sDisplay Picture: Successfully completed (%s).\n",
		    L_MSNxSTR, m_strFileName.c_str());
	}
	else
	{
	  gLog.Error("%sDisplay Picture: Too much data received, ending transfer.\n",
		     L_MSNxSTR);
	}
	close(m_nFileDesc);
	m_nFileDesc = -1;
	m_eState = STATE_FINISHED;
	ICQUser *u = gUserManager.FetchUser(m_strId.c_str(), MSN_PPID, LOCK_W);
	if (u)
	{
	  u->SetPicturePresent(true);
	  gUserManager.DropUser(u);
	  m_pMSN->PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
						  USER_PICTURE,
						  m_strId.c_str(), MSN_PPID));
	}

	// Ack that we got the data
	CMSNPacket *pAck = new CPS_MSNP2PAck(m_strId.c_str(), m_nSessionId,
					     m_nBaseId-1, nIdentifier, nAckId,
					     nDataSize[1], nDataSize[0]);
	m_pMSN->Send_SB_Packet(m_strId, pAck, m_nSocketDesc);

        // Send a bye command
        CMSNPacket *pBye = new CPS_MSNP2PBye(m_strId.c_str(),
					     m_strFromId.c_str(),
					     m_strCallId.c_str(),
	  				     m_nBaseId, nAckId,
					     nDataSize[1], nDataSize[0]);
        m_pMSN->Send_SB_Packet(m_strId, pBye, m_nSocketDesc);        
	return 0;
      }

      break;
    }

    case STATE_FINISHED:
    {
      // Don't have to send anything back, just return and close the socket.
      gLog.Info("%s Display Picture: closing connection with %s\n", L_MSNxSTR,
                m_strId.c_str());
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
