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
/*---------------------------------------------------------------------------
 * ICQEvent
 *
 * This class is the main event class for talking to the ICQ server and to
 * plugins.  Internally all messages/urls...become ICQEvents with the
 * relevant data fields set.  A plugin will receive an event in response
 * to any asynchronous function call (such as icqSendMessage) eventually.
 * The useful fields are as follows:
 *
 * EventResult Result()
 *   This is the result of the event, can be EVENT_ACKED (this is what most
 *   events will return on success), EVENT_SUCCESS (this is returned when
 *   the command is extended such as searches or info updates and has been
 *   completed successfully), EVENT_FAILED (same, but returned when the
 *   event failed for some reason, should rarely happen), EVENT_TIMEDOUT
 *   (returned if the event timedout trying to talk to the server),
 *   EVENT_ERROR (if an error occured at some point), or EVENT_CANCELLED
 *   (the event was cancelled by a call to icqCancelEvent).
 *
 * int SubResult()
 *   This will be either ICQ_TCPxACK_ACCEPT if the event was accepted by
 *   the other side, ICQ_TCPxACK_REJECT if the event was rejected by the
 *   other side (should never happen really), or ICQ_TCPxACK_RETURN if the
 *   other side returned the event (meaning they are in occupied or dnd so
 *   the message would need to be sent urgent or to contact list).  This
 *   field is only relevant if the command was ICQ_CMDxTCP_START (ie the
 *   message was sent direct).
 *
 * unsigned short Command()
 *   The command, for example ICQ_CMDxTCP_START, or ICQ_CMDxSND_THRUxSERVER,
 *   or ICQ_CMDxSND_META.
 *
 * unsigned short SubCommand()
 *   The subcommand, relevant only if this was a message/url/chat/file,
 *   in which case Command() will be ICQ_CMDxTCP_START or
 *   ICQ_CMDxSND_THRU_SERVER and this field will be ICQ_CMDxSUB_MSG...
 *
 * unsigned long Sequence()
 * unsigned short SubSequence()
 *   These are used to identify events internally, but are necessary for
 *   accepting/rejecting chat or file requests.
 *
 * unsigned long Uin()
 *   The uin of the user the event was destined for.  Only relevant if
 *   this was a message/url...
 *
 * CSearchAck *SearchAck()
 *   Special structure containing information relevant if this is a
 *   search event.
 *
 * CExtendedAck *ExtendedAck()
 *   Special structure containing information relevant if this is a
 *   chat or file transfer accept or reject.
 *
 * CUserEvent *UserEvent()
 *   Contains the actual CUserEvent containing the message/url...that was
 *   sent to Uin().  Can be used to resend the event.
 *
 * ICQUser *UnknownUser()
 *   If the event was a user information update (basic/extended/meta) and
 *   the user does not exist on the contact list, this will return the user
 *   with the relevant fields set.  This is helpful in searches for example
 *   to avoid having to add the user to the list before checking their
 *   other information.
 *
 *-------------------------------------------------------------------------*/

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

  // Returns the event and transfers ownership to the calling function
  CUserEvent *GrabUserEvent()  { CUserEvent *e = m_pUserEvent; m_pUserEvent = NULL; return e; }
  CSearchAck *GrabSearchAck()  { CSearchAck *a = m_pSearchAck; m_pSearchAck = NULL; return a; }
  ICQUser *GrabUnknownUser()   { ICQUser *u = m_pUnknownUser; m_pUnknownUser = NULL; return u; }

  ~ICQEvent();

protected:
  ICQEvent(CICQDaemon *_xDaemon, int _nSocketDesc, CPacket *p, ConnectType _eConnect,
           unsigned long _nUin, CUserEvent *e);
  ICQEvent(ICQEvent *);

  // Compare this event to another one
  bool CompareEvent(int, unsigned long) const;
  bool CompareEvent(unsigned long) const;

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

  unsigned long  m_nEventId;
  static unsigned long s_nNextEventId;

friend class CICQDaemon;
friend void *ProcessRunningEvent_tep(void *p);
friend void *MonitorSockets_tep(void *p);
friend class CICQEventTag;
};


//=====CICQEventTag==========================================================
class CICQEventTag
{
public:
  /* Called to verify if this is the tag for the given event.  Note that
   * (NULL)->Equals(e) is a valid call and returns true if e is also NULL. */
  bool Equals(const ICQEvent *e);

  unsigned long Uin() { return m_nUin; }
protected:
  CICQEventTag(const ICQEvent *e);

  unsigned long m_nEventId;
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
  CICQSignal(unsigned long _nSignal, unsigned long _nSubSignal, unsigned long _nUin, int nArgument = 0);
  CICQSignal(CICQSignal *s);
  ~CICQSignal();
  unsigned long Signal() { return m_nSignal; }
  unsigned long SubSignal() { return m_nSubSignal; }
  unsigned long Uin() { return m_nUin; }
  int Argument() { return m_nArgument; }
protected:
  unsigned long m_nSignal;
  unsigned long m_nSubSignal;
  unsigned long m_nUin;
  int m_nArgument;
};

#endif
