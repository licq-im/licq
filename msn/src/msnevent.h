#ifndef __MSNDATAEVENT_H
#define __MSNDATAEVENT_H

#include <string>

#include "msnpacket.h"

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
      unsigned long baseId, const std::string& id, const std::string& fromId,
      const std::string& callId, CMSN* p);
  ~CMSNDataEvent();

  int ProcessPacket(CMSNBuffer *);

  int getSocket() { return m_nSocketDesc; }
  std::string getUser() const { return m_strId; }
  unsigned long getSessionId() { return m_nSessionId; }
  unsigned long getBaseId() { return m_nBaseId; }

  void setSocket(int _n) { m_nSocketDesc = _n; }

protected:
  CMSN *m_pMSN;
  CICQDaemon *m_pDaemon;

  int m_nSocketDesc;
  unsigned long m_nEvent;
  std::string m_strId,
         m_strFromId,
         m_strCallId;
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

#endif // __MSNDATAEVENT_H
