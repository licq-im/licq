#ifndef ICQEVENT_H
#define ICQEVENT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "socket.h"
#include "message.h"

class CPacket;

struct SExtendedAck
{
  bool bAccepted;
  unsigned short nPort;
  char *szResponse;
};

struct SSearchAck
{
  struct UserBasicInfo *sBasicInfo;
  char cMore;
};


//=====ICQEvent====================================================================================
// wraps a timer event so that the timeout will return the socket and sequence of the packet
// that timed out
enum EConnect {CONNECT_SERVER, CONNECT_USER, CONNECT_NONE };
enum EEventResult { EVENT_ACKED, EVENT_SUCCESS, EVENT_FAILED, EVENT_TIMEDOUT, EVENT_ERROR, EVENT_CANCELLED };

class ICQEvent
{
public:
  ICQEvent(int _nSocketDesc, CPacket *p, EConnect _eConnect, unsigned long _nUin, CUserEvent *e);
  ICQEvent(ICQEvent *);
  ~ICQEvent(void);

  bool CompareEvent(int, unsigned long);

  EConnect       m_eConnect;
  EEventResult   m_eResult;
  int            m_nSubResult;
  unsigned short m_nCommand;
  unsigned short m_nSubCommand;
  unsigned long  m_nDestinationUin;
  unsigned long  m_nSequence;
  unsigned short m_nSubSequence;
  int            m_nSocketDesc;
  CPacket        *m_xPacket;
  pthread_t      thread_send;
  pthread_t      thread_plugin;

  CUserEvent    *m_xUserEvent;
  SExtendedAck  *m_sExtendedAck;
  SSearchAck    *m_sSearchAck;
};



//=====CICQSignal===============================================================
const unsigned long SIGNAL_UPDATExUSERS          = 0x00000001;
//const unsigned long SIGNAL_REORDERxUSER          = 0x00000002;
const unsigned long SIGNAL_UPDATExUSER           = 0x00000002;
const unsigned long SIGNAL_LOGON                 = 0x00000004;
const unsigned long SIGNAL_ALL                   = 0xFFFFFFFF;

// User information update constants
const unsigned short UPDATE_STATUS                 = 1;
const unsigned short UPDATE_EVENTS                 = 2;
const unsigned short UPDATE_BASIC                  = 3;
const unsigned short UPDATE_EXT                    = 4;

typedef unsigned long ESignalType;

class CICQSignal
{
public:
  CICQSignal(ESignalType _eSignalType, unsigned long _nData1, unsigned long _nData2);
  CICQSignal(CICQSignal *s);
  ~CICQSignal(void);

  ESignalType m_eSignalType;
  unsigned long m_nData1;
  unsigned long m_nData2;
};

#endif
