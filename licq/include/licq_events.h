#ifndef ICQEVENT_H
#define ICQEVENT_H

#include "pthread_rdwr.h"

class CPacket;
class CICQDaemon;
class CUserEvent;
class ICQUser;

//-----CExtendedAck----------------------------------------------------------

/*! \brief Information on file and chat request responses.

    This class will provide information on what the result of a chat or file
    transfer request was.
*/
class CExtendedAck
{
public:
  // Accessors
  
  //!Returns true if the remote end accepted the request, otherwise false.
  bool Accepted()         { return m_bAccepted; }
  //!The port to connect to if the request was accpeted.
  unsigned short Port()   { return m_nPort; }
  //!The reason for declining the request.
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

/*! \brief The response to a search request.

    Each search result is passed to the plugin as a CSearchAck.  If there
    was no search results, then a single CSearchAck is passed on with
    Result() returning EVENT_SUCCESS, which also signifies the search is
    complete.
*/
class CSearchAck
{
public:
  // Accessors
  //! Returns the UIN of the search result.
  unsigned long Uin()      { return m_nUin; }
  //! Returns the alias (nickname) of the search result.
  const char *Alias()      { return m_szAlias; }
  //! Returns the first name of the search result.
  const char *FirstName()  { return m_szFirstName; }
  //! Returns the last name of the search result.
  const char *LastName()   { return m_szLastName; }
  //! Returns the e-mail address of the search result.
  const char *Email()      { return m_szEmail; }
  //! If non-zero, the number of search results that were found that could not
  //! be displayed.  The server has a 40 user limit on search results.  This
  //! is valid when Result() is EVENT_SUCCESS.
  int More()     	   { return m_nMore; }
  //! The online status of the search result.
  char Status()            { return m_nStatus; }
  //! The gender of the search result.  Female = 1, Male = 2, Unknown = 3.
  char Gender()		   { return m_nGender; }
  //! The age of the serach result.
  char Age()		   { return m_nAge; }
  //! Non-zero if authorization is required to add this user.
  char Auth()		   { return m_nAuth; }

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
  char m_nGender;
  char m_nAge;
  char m_nAuth;

friend class CICQDaemon;
};



//=====ICQEvent====================================================================================

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

/*! \brief Plugin event messages

    This class is the main event class for talking to the ICQ server and to
    plugins.  Internally all messages/urls... become ICQEvents with the
    relevant data fields set.  A plugin will receive an event in response
    to any asynchronous function call (such as icqSendMessage) eventually.
*/
class ICQEvent
{
public:
  // Accessors

  //!This is the result of the event, can be EVENT_ACKED (this is what most
  //!events will return on success), EVENT_SUCCESS (this is returned when
  //!the command is extended such as searches or info updates and has been
  //!completed successfully), EVENT_FAILED (same, but returned when the
  //!event failed for some reason, should rarely happen), EVENT_TIMEDOUT
  //!(returned if the event timedout trying to talk to the server),
  //!EVENT_ERROR (if an error occured at some point), or EVENT_CANCELLED
  //!(the event was cancelled by a call to icqCancelEvent).
  EventResult Result()         { return m_eResult; }

  //!This will be either ICQ_TCPxACK_ACCEPT if the event was accepted by
  //!the other side, ICQ_TCPxACK_REJECT if the event was rejected by the
  //!other side (should never happen really), or ICQ_TCPxACK_RETURN if the
  //!other side returned the event (meaning they are in occupied or dnd so
  //!the message would need to be sent urgent or to contact list).  This
  //!field is only relevant if the command was ICQ_CMDxTCP_START (ie the
  //!message was sent direct).
  int SubResult()              { return m_nSubResult; }

  //!This is used to identify what channel the event was sent on.  This is
  //!only non-zero for server events.
  unsigned char Channel()      { return m_nChannel; }

  //!The SNAC returned as an unsigned long.  The upper 2 bytes is the family
  //!and the lower 2 bytes is the subtype.  To compare SNAC's use the SNAC
  //!macro to convert it to an unsigned long: MAKESNAC(family, subtype).
  unsigned long SNAC()         { return m_nSNAC; }

  //!The command, for example ICQ_CMDxTCP_START.  This is only non-zero
  //!for direct connection events.
  unsigned short Command()     { return m_nCommand; }

  //!The subcommand, relevant only if this was a message/url/chat/file,
  //!in which case Command() will be ICQ_CMDxTCP_START or SNAC() will be
  //!MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_CMDxSND_THRU_SERVER) and this
  //!field will be ICQ_CMDxSUB_MSG...
  unsigned short SubCommand()  { return m_nSubCommand; }

  //!This is used to identify events internally, but is necessary for
  //!accepting/rejecting chat or file requests.
  unsigned long Sequence()     { return m_nSequence; }

  //!This is used to identify events internally, is are necessary for
  //!accepting/rejecting chat or file requests.
  unsigned short SubSequence() { return m_nSubSequence; }

  //!The uin of the user the event was destined for.  Only relevant if
  //!this was a message/url...
  unsigned long Uin()          { return m_nDestinationUin; }

  //!Special structure containing information relevant if this is a
  //!search event.
  CSearchAck *SearchAck()      { return m_pSearchAck; }

  //!Special structure containing information relevant if this is a
  //!chat or file transfer accept or reject.
  CExtendedAck *ExtendedAck()  { return m_pExtendedAck; }
  
  //!Contains the actual CUserEvent containing the message/url...that was
  //!sent to Uin().  Can be used to resend the event.
  CUserEvent *UserEvent()      { return m_pUserEvent; }
  
  //!If the event was a user information update (basic/extended/meta) and
  //!the user does not exist on the contact list, this will return the user
  //!with the relevant fields set.  This is helpful in searches for example
  //!to avoid having to add the user to the list before checking their
  //!other information.
  ICQUser *UnknownUser()       { return m_pUnknownUser; }

  // Returns the event and transfers ownership to the calling function
  CUserEvent *GrabUserEvent();
  CSearchAck *GrabSearchAck();
  ICQUser *GrabUnknownUser();

  //!Compare this event to the id to see if the plugin matches a waiting
  //!event with the event that the daemon has signaled to the plugin.
  bool Equals(unsigned long) const;

  ~ICQEvent();

protected:
  ICQEvent(CICQDaemon *_xDaemon, int _nSocketDesc, CPacket *p, ConnectType _eConnect,
           unsigned long _nUin, CUserEvent *e);
  ICQEvent(ICQEvent *);

  // Daemon only
  unsigned short SubType()     { return m_nSubType; }
  unsigned short ExtraInfo()   { return m_nExtraInfo; }

  // Compare this event to another one
  bool CompareEvent(int, unsigned long) const;
  bool CompareEvent(unsigned long) const;
  bool CompareSubSequence(unsigned long) const;
  unsigned long EventId() const;

  ConnectType    m_eConnect;
  EventResult    m_eResult;
  int            m_nSubResult;
  bool           m_bCancelled : 1;
  bool           m_Deleted : 1;
  bool           m_NoAck : 1;
  unsigned char  m_nChannel;
  unsigned long  m_nSNAC;
  unsigned short m_nCommand;
  unsigned short m_nSubCommand;
  unsigned long  m_nDestinationUin;
  unsigned long  m_nSequence;
  unsigned short m_nSubSequence;
  unsigned short m_nSubType;
  unsigned short m_nExtraInfo;
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
 *	LOGOFF_REQUESTED - The user requeted to logoff and the action succeed.
 *	LOGOFF_RATE  - loging off: login failed: rate limit exceeded
 *	LOGOFF_PASSWORD - loging off: login failed: Invalid UIN and password
 *      combination.
 *
 *  SIGNAL_ONEVENT - FIXME: MISSING DESCRIPTION
 *
 *  SIGNAL_UI_VIEWEVENT - The UIN is that of the user we want to view the
 *  oldest event of or 0 if we just want to view the oldest pending event.
 *  All other parameters are 0.
 *
 *  SIGNAL_UI_MESSAGE - The UIN is that of the user we want to send a
 *  message. All other parameters are 0.
 *
 *  SIGNAL_ADDxSERVERxLIST - The UIN has been successfully added to the server
 *  list.  The server side alias is just the UIN as a string.  The plugin
 *  may call icqRenameUser after receiving this.  Sub-type is 0.
 *
 *-------------------------------------------------------------------------*/
const unsigned long SIGNAL_UPDATExLIST           = 0x00000001;
const unsigned long SIGNAL_UPDATExUSER           = 0x00000002;
const unsigned long SIGNAL_LOGON                 = 0x00000004;
const unsigned long SIGNAL_LOGOFF                = 0x00000008;
const unsigned long SIGNAL_ONEVENT               = 0x00000010;
const unsigned long SIGNAL_UI_VIEWEVENT          = 0x00000020;
const unsigned long SIGNAL_UI_MESSAGE            = 0x00000040;
const unsigned long SIGNAL_ADDxSERVERxLIST       = 0x00000080;
const unsigned long SIGNAL_ALL                   = 0xFFFFFFFF;

// logoff constants
const unsigned long LOGOFF_REQUESTED             = 0x00000000;
const unsigned long LOGOFF_RATE                  = 0x00000001;
const unsigned long LOGOFF_PASSWORD              = 0x00000002;

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

/*! \brief Plugin notification messages

    This class controls all asynchronous plugin notifications.  When a plugin
    registers with the Licq daemon it informs the daemon of what signals it
    is interested in.  From then on, at any time it may receive a signal
    from the following list.  Each signal contains the signal type, an
    optional sub-type, uin and signal specific argument.
*/
class CICQSignal
{
public:
  CICQSignal(unsigned long _nSignal, unsigned long _nSubSignal, unsigned long _nUin, int nArgument = 0, char *nParameters = 0);
  CICQSignal(CICQSignal *s);
  ~CICQSignal();
  
  //!Returns the signal being posted to the plugin.
  unsigned long Signal() { return m_nSignal; }
  //!Returns the sub-signal being posted to the plugin.
  unsigned long SubSignal() { return m_nSubSignal; }
  //!UIN that the signal is related.  See signals to understand how this
  //!value is set.
  unsigned long Uin() { return m_nUin; }
  int Argument() { return m_nArgument; }
  char *Parameters() { return m_szParameters; }
protected:
  unsigned long m_nSignal;
  unsigned long m_nSubSignal;
  unsigned long m_nUin;
  int m_nArgument;
  char * m_szParameters;
};

#endif
