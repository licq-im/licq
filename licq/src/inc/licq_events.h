#ifndef ICQEVENT_H
#define ICQEVENT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "licq_socket.h"
#include "licq_message.h"

class CPacket;
class CICQDaemon;

struct SExtendedAck
{
  bool bAccepted;
  unsigned short nPort;
  char *szResponse;
};

class CSearchAck
{
public:
  CSearchAck(unsigned long _nUin)
  {
    nUin = _nUin;
    szAlias = szFirstName = szLastName = szEmail = NULL;
  }
  ~CSearchAck()
  {
    if (szAlias != NULL) free(szAlias);
    if (szFirstName != NULL) free(szFirstName);
    if (szLastName != NULL) free(szLastName);
    if (szEmail != NULL) free(szEmail);
  }

  unsigned long nUin;
  char *szAlias;
  char *szFirstName;
  char *szLastName;
  char *szEmail;
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
  ICQEvent(CICQDaemon *_xDaemon, int _nSocketDesc, CPacket *p, EConnect _eConnect,
           unsigned long _nUin, CUserEvent *e);
  ICQEvent(ICQEvent *);
  ~ICQEvent();

  bool CompareEvent(int, unsigned long) const;

  EConnect       m_eConnect;
  EEventResult   m_eResult;
  int            m_nSubResult;
  bool           m_bCancelled;
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
  CSearchAck    *m_sSearchAck;

  CICQDaemon    *m_xDaemon;
};


//=====CICQEventTag==========================================================
class CICQEventTag
{
public:
  //CICQEventTag(int sd, unsigned long se) : m_nSocketDesc(sd), m_nSequence(se) {}
  CICQEventTag(const ICQEvent *e);
  bool Equals(const ICQEvent *e);
protected:
  int m_nSocketDesc;
  unsigned long m_nSequence;
  unsigned long m_nUin;

friend class CICQDaemon;
};



//=====CICQSignal============================================================
const unsigned long SIGNAL_UPDATExLIST           = 0x00000001;
const unsigned long SIGNAL_UPDATExUSER           = 0x00000002;
const unsigned long SIGNAL_LOGON                 = 0x00000004;
const unsigned long SIGNAL_ALL                   = 0xFFFFFFFF;

// User information update constants
const unsigned long USER_STATUS                 = 1;
const unsigned long USER_EVENTS                 = 2;
const unsigned long USER_BASIC                  = 3;
const unsigned long USER_EXT                    = 4;
const unsigned long USER_GENERAL                = 5;
const unsigned long USER_MORE                   = 6;
const unsigned long USER_WORK                   = 7;
const unsigned long USER_ABOUT                  = 8;

const unsigned long LIST_ADD                     = 1;
const unsigned long LIST_REMOVE                  = 2;
const unsigned long LIST_REORDER                 = 3;
const unsigned long LIST_ALL                     = 4;


class CICQSignal
{
public:
  CICQSignal(unsigned long _nSignal, unsigned long _nSubSignal, unsigned long _nUin, unsigned long nArgument = 0);
  CICQSignal(CICQSignal *s);
  ~CICQSignal();
  unsigned long Signal() { return m_nSignal; }
  unsigned long SubSignal() { return m_nSubSignal; }
  unsigned long Uin() { return m_nUin; }
  // Argument depends on what the signal/subsignal is:
  // UPDATExUSER/USER_EVENTS - event id
  unsigned long Argument() { return m_nArgument; }
protected:
  unsigned long m_nSignal;
  unsigned long m_nSubSignal;
  unsigned long m_nUin;
  unsigned long m_nArgument;
};

#endif
