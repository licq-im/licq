#ifndef ICQEVENT_H
#define ICQEVENT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "licq_socket.h"
#include "licq_message.h"

class CPacket;
class CICQDaemon;

//-----CExtendedAck----------------------------------------------------------
class CExtendedAck
{
public:
  // Accessors
  bool Accepted()         { return m_bAccepted; }
  unsigned short Port()   { return m_nPort; }
  const char *Response()  { return m_szResponse; }

  ~CExtendedAck();

protected:
  CExtendedAck(bool, unsigned short, char *);

  bool m_bAccepted;
  unsigned short m_nPort;
  char *m_szResponse;

friend class CICQDaemon;
};


//-----CSearchAck------------------------------------------------------------
class CSearchAck
{
public:
  // Accessors
  unsigned long Uin()      { return m_nUin; }
  const char *Alias()      { return m_szAlias; }
  const char *FirstName()  { return m_szFirstName; }
  const char *LastName()   { return m_szLastName; }
  const char *Email()      { return m_szEmail; }
  bool More()              { return m_bMore; }

  ~CSearchAck();

protected:
  CSearchAck(unsigned long _nUin);

  unsigned long m_nUin;
  char *m_szAlias;
  char *m_szFirstName;
  char *m_szLastName;
  char *m_szEmail;
  bool m_bMore;

friend class CICQDaemon;
};



//=====ICQEvent====================================================================================
// wraps a timer event so that the timeout will return the socket and sequence of the packet
// that timed out
enum ConnectType
{
  CONNECT_SERVER,
  CONNECT_USER,
  CONNECT_NONE
};

enum EventResult
{
  EVENT_ACKED,
  EVENT_SUCCESS,
  EVENT_FAILED,
  EVENT_TIMEDOUT,
  EVENT_ERROR,
  EVENT_CANCELLED
};

class ICQEvent
{
public:
  // Accessors
  EventResult Result()         { return m_eResult; }
  int SubResult()              { return m_nSubResult; }
  unsigned short Command()     { return m_nCommand; }
  unsigned short SubCommand()  { return m_nSubCommand; }
  unsigned long Sequence()     { return m_nSequence; }
  unsigned short SubSequence() { return m_nSubSequence; }
  unsigned long Uin()          { return m_nDestinationUin; }
  CSearchAck *SearchAck()      { return m_pSearchAck; }
  CExtendedAck *ExtendedAck()  { return m_pExtendedAck; }
  CUserEvent *UserEvent()      { return m_pUserEvent; }
  ICQUser *UnknownUser()       { return m_pUnknownUser; }

  ~ICQEvent();

protected:
  ICQEvent(CICQDaemon *_xDaemon, int _nSocketDesc, CPacket *p, ConnectType _eConnect,
           unsigned long _nUin, CUserEvent *e);
  ICQEvent(ICQEvent *);

  // Compare this event to another one
  bool CompareEvent(int, unsigned long) const;

  ConnectType    m_eConnect;
  EventResult    m_eResult;
  int            m_nSubResult;
  bool           m_bCancelled;
  unsigned short m_nCommand;
  unsigned short m_nSubCommand;
  unsigned long  m_nDestinationUin;
  unsigned long  m_nSequence;
  unsigned short m_nSubSequence;
  int            m_nSocketDesc;
  CPacket        *m_pPacket;
  pthread_t      thread_send;
  pthread_t      thread_plugin;

  CUserEvent    *m_pUserEvent;
  CExtendedAck  *m_pExtendedAck;
  CSearchAck    *m_pSearchAck;
  ICQUser       *m_pUnknownUser;

  CICQDaemon    *m_pDaemon;

friend class CICQDaemon;
friend void *ProcessRunningEvent_tep(void *p);
friend void *MonitorSockets_tep(void *p);
friend class CICQEventTag;
};


//=====CICQEventTag==========================================================
class CICQEventTag
{
public:
  bool Equals(const ICQEvent *e);

  unsigned long Uin() { return m_nUin; }
protected:
  CICQEventTag(const ICQEvent *e);

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
