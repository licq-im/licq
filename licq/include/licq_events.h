#ifndef ICQEVENT_H
#define ICQEVENT_H

#include "pthread_rdwr.h"

class CPacket;
class CICQDaemon;
class CUserEvent;
class ICQUser;

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
// Values for the Status() field of the SearchAck
const unsigned short  SA_OFFLINE = 0;
const unsigned short  SA_ONLINE = 1;
const unsigned short  SA_DISABLED = 2;

class CSearchAck
{
public:
  // Accessors
  unsigned long Uin()      { return m_nUin; }
  const char *Alias()      { return m_szAlias; }
  const char *FirstName()  { return m_szFirstName; }
  const char *LastName()   { return m_szLastName; }
  const char *Email()      { return m_szEmail; }
  int More()     { return m_nMore; }
  char Status()            { return m_nStatus; }

  ~CSearchAck();

protected:
  CSearchAck(unsigned long _nUin);

  unsigned long m_nUin;
  char *m_szAlias;
  char *m_szFirstName;
  char *m_szLastName;
  char *m_szEmail;
  int m_nMore;
  char m_nStatus;

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
  CUserEvent *GrabUserEvent();
  CSearchAck *GrabSearchAck();
  ICQUser *GrabUnknownUser();

  // Compare this event to the id, can be called with this==NULL (returns false)
  bool Equals(unsigned long) const;

  ~ICQEvent();

protected:
  ICQEvent(CICQDaemon *_xDaemon, int _nSocketDesc, CPacket *p, ConnectType _eConnect,
           unsigned long _nUin, CUserEvent *e);
  ICQEvent(ICQEvent *);

  // Compare this event to another one
  bool CompareEvent(int, unsigned long) const;
  bool CompareEvent(unsigned long) const;
  unsigned long EventId() const;

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
friend void *ProcessRunningEvent_Client_tep(void *p);
friend void *ProcessRunningEvent_Server_tep(void *p);
friend void *MonitorSockets_tep(void *p);
friend class CICQEventTag;
};



//=====CICQSignal============================================================
/*---------------------------------------------------------------------------
 * CICQSignal
 *
 * This class controls all asynchronous plugin notifications.  When a plugin
 * registers with the Licq daemon it informs the daemon of what signals it
 * is interested in.  From then on, at any time it may receive a signal
 * from the following list.  Each signal contains the signal type, an
 * optional sub-type, uin and signal specific argument.
 *
 * SIGNAL_UPDATExLIST -
 *   Indicates that the user list has changed in some way.  The sub-type
 *   will be one of the following.  In all cases the argument is 0.
 *     LIST_ADD - A user was added to the list.  The UIN will be that of
 *     the new user.
 *     LIST_REMOVE - A user was removed from the list.  The UIN will be
 *     that of the removed user.
 *     LIST_ALL - The entire list has been changed.  The UIN will be 0.
 *
 *  SIGNAL_UPDATExUSER - The user has been modified in some way.  The UIN
 *  is that of the relevant user, and the sub-type indicates what type of
 *  information was changed.  In all cases except the following the argument
 *  is 0:
 *    USER_STATUS - Indicates that the users status has changed.  The
 *    argument will be negative if the user went offline, positive if they
 *    went online, and 0 if they stayed online (for example went from NA
 *    to Away).
 *    USER_EVENTS - Indicates that the user has received or deleted an
 *    event (message, url...).  The argument will be the event id of the
 *    added event, or the negative of the event id if an event was deleted.
 *    If the user checked our auto-response then the argument will be 0.
 *    USER_SECURITY - The users security status changed.  If the arg is
 *    1 the user is now secure, if it's 0, the user is no longer secure.
 *
 *  SIGNAL_LOGON - Indicates that we have successfully logged on to the
 *  icq network.  UIN and argument are both 0, as is the sub-type.
 *
 *  SIGNAL_LOGOFF - Indicates that we logged off.  All parameters are 0.
 *
 *-------------------------------------------------------------------------*/
const unsigned long SIGNAL_UPDATExLIST           = 0x00000001;
const unsigned long SIGNAL_UPDATExUSER           = 0x00000002;
const unsigned long SIGNAL_LOGON                 = 0x00000004;
const unsigned long SIGNAL_LOGOFF                = 0x00000008;
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
const unsigned long USER_SECURITY               = 9;

const unsigned long LIST_ADD                     = 1;
const unsigned long LIST_REMOVE                  = 2;
const unsigned long LIST_ALL                     = 3;


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
