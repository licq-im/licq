#ifndef ICQEVENT_H
#define ICQEVENT_H

#include <pthread.h>

#include "licq_message.h"
#include "licq_types.h"

class CPacket;
class CICQDaemon;
class CUserEvent;

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
  bool Accepted() const { return m_bAccepted; }
  //!The port to connect to if the request was accpeted.
  unsigned short Port() const { return m_nPort; }
  //!The reason for declining the request.
  const char* Response() const { return m_szResponse; }

  ~CExtendedAck();

protected:
  CExtendedAck(bool bAccepted, unsigned short nPort, const char* szResponse);

  bool m_bAccepted;
  unsigned short m_nPort;
  char *m_szResponse;

friend class CICQDaemon;
};


//-----CSearchAck------------------------------------------------------------
// Values for the Status() field of the SearchAck
//! User is offline.
const unsigned short  SA_OFFLINE = 0;
//! User is online.
const unsigned short  SA_ONLINE = 1;
//! User has disabled online awareness.
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
  //! Returns the alias (nickname) of the search result.
  const char* Alias() const { return m_szAlias; }
  //! Returns the first name of the search result.
  const char* FirstName() const { return m_szFirstName; }
  //! Returns the last name of the search result.
  const char* LastName() const { return m_szLastName; }
  //! Returns the e-mail address of the search result.
  const char* Email() const { return m_szEmail; }

  /**
   * Get the user id
   *
   * @return User id of search match
   */
  const UserId& userId() const { return myUserId; }

  //! If non-zero, the number of search results that were found that could not
  //! be displayed.  The server has a 40 user limit on search results.  This
  //! is valid when Result() is EVENT_SUCCESS.
  unsigned long More() const { return m_nMore; }
  //! The online status of the search result.
  char Status() const { return m_nStatus; }
  //! The gender of the search result.  Female = 1, Male = 2, Unknown = 3.
  char Gender() const { return m_nGender; }
  //! The age of the serach result.
  char Age() const { return m_nAge; }
  //! Non-zero if authorization is required to add this user.
  char Auth() const { return m_nAuth; }

  ~CSearchAck();

protected:
  CSearchAck(const UserId& userId);

  UserId myUserId;
  char *m_szAlias;
  char *m_szFirstName;
  char *m_szLastName;
  char *m_szEmail;
  unsigned long m_nMore;
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
  //! The event has been acked by the reciepient. Most events will
  //! return this.
  EVENT_ACKED,
  //! The event was succcessfully sent.  This is used by extended
  //! events.  Such as searching for users or updating user info.
  EVENT_SUCCESS,
  //! The event failed for some reason.  This should rarely happen.
  EVENT_FAILED,
  //! The event timed out while communicating with the remote socket.
  EVENT_TIMEDOUT,
  //! The event had some kind of error at some point.
  EVENT_ERROR,
  //! The event was concelled by the user and was not sent.
  //! A call to icqCancelEvent was made.
  EVENT_CANCELLED
};

/*! \brief Plugin event messages

    This class is the main event class for talking to the ICQ server and to
    plugins.  Internally all messages/urls... become ICQEvents with the
    relevant data fields set.  A plugin will receive an event in response
    to any asynchronous function call (such as icqSendMessage) eventually.
*/
class LicqEvent
{
public:
  // Accessors

  //!This is the result of the event.
  EventResult Result() const { return m_eResult; }

  //!This will be either ICQ_TCPxACK_ACCEPT if the event was accepted by
  //!the other side, ICQ_TCPxACK_REJECT if the event was rejected by the
  //!other side (should never happen really), or ICQ_TCPxACK_RETURN if the
  //!other side returned the event (meaning they are in occupied or dnd so
  //!the message would need to be sent urgent or to contact list).  This
  //!field is only relevant if the command was ICQ_CMDxTCP_START (ie the
  //!message was sent direct).
  int SubResult() const { return m_nSubResult; }

  //!This is used to identify what channel the event was sent on.  This is
  //!only non-zero for server events.
  unsigned char Channel() const { return m_nChannel; }

  //!The SNAC returned as an unsigned long.  The upper 2 bytes is the family
  //!and the lower 2 bytes is the subtype.  To compare SNAC's use the SNAC
  //!macro to convert it to an unsigned long: MAKESNAC(family, subtype).
  unsigned long SNAC() const { return m_nSNAC; }

  //!The command, for example ICQ_CMDxTCP_START.  This is only non-zero
  //!for direct connection events.
  unsigned short Command() const { return m_nCommand; }

  //!The subcommand, relevant only if this was a message/url/chat/file,
  //!in which case Command() will be ICQ_CMDxTCP_START or SNAC() will be
  //!MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_CMDxSND_THRU_SERVER) and this
  //!field will be ICQ_CMDxSUB_MSG...
  unsigned short SubCommand() const { return m_nSubCommand; }

  //!This is used to identify events internally, but is necessary for
  //!accepting/rejecting chat or file requests.
  unsigned short Sequence() const { return m_nSequence; }

  //!This is used to identify events internally, is are necessary for
  //!accepting/rejecting chat or file requests.
  unsigned short SubSequence() const { return m_nSubSequence; }

  /**
   * Get user id the event was destined for.
   *
   * @return User id for event if relevant
   */
  const UserId& userId() const { return myUserId; }

  //!Special structure containing information relevant if this is a
  //!search event.
  const CSearchAck* SearchAck() const { return m_pSearchAck; }

  //!Special structure containing information relevant if this is a
  //!chat or file transfer accept or reject.
  const CExtendedAck* ExtendedAck() const { return m_pExtendedAck; }

  //!Contains the actual CUserEvent containing the message/url...that was
  //!sent to Uin().  Can be used to resend the event.
  const CUserEvent* UserEvent() const { return m_pUserEvent; }

  //!If the event was a user information update (basic/extended/meta) and
  //!the user does not exist on the contact list, this will return the user
  //!with the relevant fields set.  This is helpful in searches for example
  //!to avoid having to add the user to the list before checking their
  //!other information.
  const LicqUser* UnknownUser() const { return m_pUnknownUser; }

  // Returns the event and transfers ownership to the calling function
  CUserEvent *GrabUserEvent();
  CSearchAck *GrabSearchAck();
  LicqUser* GrabUnknownUser();

  //!Compare this event to the id to see if the plugin matches a waiting
  //!event with the event that the daemon has signaled to the plugin.
  bool Equals(unsigned long) const;

  ~LicqEvent();

protected:
  LicqEvent(CICQDaemon *_xDaemon, int _nSocketDesc, CPacket *p, ConnectType _eConnect,
      const UserId& userId = USERID_NONE, CUserEvent* e = NULL);
  LicqEvent(const LicqEvent* e);

  // Daemon only
  unsigned short SubType() const     { return m_nSubType; }
  unsigned short ExtraInfo() const   { return m_nExtraInfo; }
  void SetSubType(unsigned short nSubType) { m_nSubType = nSubType; }
  bool NoAck() const           { return m_NoAck; }
  void SetNoAck(bool NoAck)    { m_NoAck = NoAck; }
  bool IsCancelled() const     { return m_bCancelled; }

  void AttachPacket(CPacket *p);

  // Compare this event to another one
  bool CompareEvent(int, unsigned short) const;
  bool CompareEvent(unsigned short) const;
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
  unsigned short m_nSequence;
  unsigned short m_nSubSequence;
  unsigned short m_nSubType;
  unsigned short m_nExtraInfo;
  int            m_nSocketDesc;
  UserId         myUserId;
  CPacket        *m_pPacket;
  pthread_t      thread_send;
  bool           thread_running;
  pthread_t      thread_plugin;

  CUserEvent    *m_pUserEvent;
  CExtendedAck  *m_pExtendedAck;
  CSearchAck    *m_pSearchAck;
  LicqUser* m_pUnknownUser;

  CICQDaemon    *m_pDaemon;

  unsigned long  m_nEventId;
  static unsigned long s_nNextEventId;

friend class CICQDaemon;
friend class COscarService;
friend class CMSN;
friend void *ProcessRunningEvent_Client_tep(void *p);
friend void *ProcessRunningEvent_Server_tep(void *p);
friend void *OscarServiceSendQueue_tep(void *p);
friend void *MonitorSockets_tep(void *p);
};

// Temporary until all occurenses of deprecated name ICQEvent have been removed
typedef LicqEvent ICQEvent;


//=====CICQSignal============================================================

/*
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
 *    The parameter will be the socket that the message was sent over.
 *    If the user checked our auto-response then the argument will be 0.
 *    USER_SECURITY - The users security status changed.  If the arg is
 *    1 the user is now secure, if it's 0, the user is no longer secure.
 *    USER_TYPING - Indicates the typing status of the user has changed.
 *
 *  SIGNAL_ONEVENT - FIXME: MISSING DESCRIPTION
 *-------------------------------------------------------------------------*/
/** Indicates that the user list has changed in some way */
const unsigned long SIGNAL_UPDATExLIST           = 0x00000001;
const unsigned long SIGNAL_UPDATExUSER           = 0x00000002;
//! Indicates that we have successfully logged on to the specified
//! network.  Argument is the Protocol ID.  All other arguments are
//! 0.
const unsigned long SIGNAL_LOGON                 = 0x00000004;
//! Indicates that we have logged off.  UserId is owner. Argument is ppid.
const unsigned long SIGNAL_LOGOFF                = 0x00000008;
const unsigned long SIGNAL_ONEVENT               = 0x00000010;
//! The UIN is that of the user we want to view the oldest event of
//! or 0 if we just want to view the oldest pending event. All other
//! parameters are 0.
const unsigned long SIGNAL_UI_VIEWEVENT          = 0x00000020;
//! The UIN is that of the user we want to send a message.  All other
//! parameters are 0.
const unsigned long SIGNAL_UI_MESSAGE            = 0x00000040;
//! The UIN has been successfully added to the server list.  The
//! server side alias is set to the UIN.  The plugin may call 
//! icqRenameUser after receiving this.  Sub-type is 0.
const unsigned long SIGNAL_ADDxSERVERxLIST       = 0x00000080;
//! Indicates new protocol plugin has been successfully loaded.
//! The sub-type is the id of the plugin.  This is used as a parameter
//! for all functions dealing with this new protocol.  The UIN and all
//! other parameters are 0.
const unsigned long SIGNAL_NEWxPROTO_PLUGIN      = 0x00000100;
//! Returns the event tag of an event from a protocol plugin.
//! The UI plugin will use this event tag to keep track of 
//! all sent events.
const unsigned long SIGNAL_EVENTxID              = 0x00000200;
//! A user has joined a conversation. m_nArgument is the socket
//! of the conversation that the user has joined.
const unsigned long SIGNAL_CONVOxJOIN            = 0x00000400;
//! A user has left a conversation. m_nArgument is the socket
//! of the conversation that the user has left.
const unsigned long SIGNAL_CONVOxLEAVE           = 0x00000800;
//! A socket has been set for this user now.
const unsigned long SIGNAL_SOCKET                = 0x00010000;
//! A verification image is available for the gui to use
//! that is located at BASE_DIR + "verify.jpg"
const unsigned long SIGNAL_VERIFY_IMAGE          = 0x00020000;
//! A new user has been successfully registered.
const unsigned long SIGNAL_NEW_OWNER             = 0x00040000;
//! Used by a UI plugin to tell the daemon that all signals should be
//! sent to the UI plugin.
const unsigned long SIGNAL_ALL                   = 0xFFFFFFFF;

// logoff constants
//! The user requested to logoff and the action succeeded.
const unsigned long LOGOFF_REQUESTED             = 0x00000000;
//! The user requested to logon but failed.  The rate limit of the
//! server has been exceeded.
const unsigned long LOGOFF_RATE                  = 0x00000001;
//! The user requested to logon but failed.  The username and password
//! combination is invalid.
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
const unsigned long USER_MORE2                  = 10;
const unsigned long USER_HP                     = 11;
const unsigned long USER_PHONExBOOK             = 12;
const unsigned long USER_PICTURE                = 13;
const unsigned long USER_TYPING                 = 14;
const unsigned long USER_PLUGIN_STATUS          = 15; // Which Plugin?

/**
 * Sub signals for SIGNAL_UPDATExLIST
 *
 * For contact updates, ppid and id are valid. For group updates, argument
 * holds the group id except for reordered which applies to entire list.
 */
enum SubSignals_UPDATExLIST
{
  LIST_CONTACT_ADDED = 1,       /**< A contact was added to the list */
  LIST_CONTACT_REMOVED = 2,     /**< A contact was removed from the list */
  LIST_INVALIDATE = 3,          /**< List must be reloaded */
  LIST_GROUP_ADDED = 4,         /**< A group was added to the list */
  LIST_GROUP_REMOVED = 5,       /**< A group was removed from the list */
  LIST_GROUP_CHANGED = 6,       /**< Data for a group has changed */
  LIST_GROUP_REORDERED = 7,     /**< Group sorting has changed */
};

// Deprecated SIGNAL_UPDATExLIST sub signal names, do not use
const unsigned long LIST_ADD                     = LIST_CONTACT_ADDED;
const unsigned long LIST_REMOVE                  = LIST_CONTACT_REMOVED;
const unsigned long LIST_ALL                     = LIST_INVALIDATE;


/*! \brief Plugin notification messages

    This class controls all asynchronous plugin notifications.  When a plugin
    registers with the Licq daemon it informs the daemon of what signals it
    is interested in.  From then on, at any time it may receive a signal
    from the following list.  Each signal contains the signal type, an
    optional sub-type, uin and signal specific argument.
*/
class LicqSignal
{
public:
  /**
   * Constructor
   *
   * @param signal Signal type
   * @param subSignal Signal sub type
   * @param userId Id of user affected by signal if applicable
   * @param argument Additional data for signal, usage is signal dependant
   * @param cid Conversation id, if applicable
   */
  LicqSignal(unsigned long signal, unsigned long subSignal,
      const UserId& userId = USERID_NONE, int argument = 0, unsigned long cid = 0);

  /**
   * Copy constructor
   *
   * @param s LicqSignal object to copy fields from
   */
  LicqSignal(const LicqSignal* s);

  //!Returns the signal being posted to the plugin.
  unsigned long Signal() const { return mySignal; }
  //!Returns the sub-signal being posted to the plugin.
  unsigned long SubSignal() const { return mySubSignal; }

  /**
   * Get id for user related to this signal
   *
   * @return user id if relevant, otherwise zero
   */
  const UserId& userId() const { return myUserId; }

  int Argument() const { return myArgument; }
  unsigned long CID() const { return myCid; }

protected:
  const unsigned long mySignal;
  const unsigned long mySubSignal;
  const UserId myUserId;
  const int myArgument;
  const unsigned long myCid;
};

//! Signals that can be sent to protocol plugins.
enum SIGNAL_TYPE
{
  //! The user requested this protocol to logon.
  PROTOxLOGON = 1,
  //! The user requested this protocol to log off.
  PROTOxLOGOFF,
  //! The user requested this protocol to change status.
  PROTOxCHANGE_STATUS,
  //! The user requested this protocol to add a new user.
  PROTOxADD_USER,
  //! The user requested this protocol to remove a user.
  PROTOxREM_USER,
  //! The user requested this protocol to rename the user on the server's list.
  PROTOxRENAME_USER,
  //! The user requested this protocol to send a message.
  PROTOxSENDxMSG,
  //! The user has started typing and wants to let the remote user know about
  //! about it.
  PROTOxSENDxTYPING_NOTIFICATION,
  //! The user granted authorization for a user to add them to his
  //! contact list.
  PROTOxSENDxGRANTxAUTH,
  //! The user refused authorization for a user to add them to his
  //! contact list.
  PROTOxSENDxREFUSExAUTH,
  //! The user has requested the user's profile/information.
  PROTOxREQUESTxINFO,
  //! The user has requested to update the owner's profile/information.
  PROTOxUPDATExINFO,
  //! The user has requested the user's picture/icon/avatar/etc..
  PROTOxREQUESTxPICTURE,
  //! The user has requested this user be added to the Invisible/Block list.
  PROTOxBLOCKxUSER,
  //! The user has requested this user be removed from the Invisible/Block
  //! list,
  PROTOxUNBLOCKxUSER,
  //! The user has requested this user be added to the Visible/Accept list.
  PROTOxACCEPTxUSER,
  //! The user has requested this user be removed fromt he Visible/Accept
  //! list.
  PROTOxUNACCEPTxUSER,
  //! The user has requsted this user be put on the Ignore list.
  PROTOxIGNORExUSER,
  //! The user has requested this user be removed fromt he Ignore list.
  PROTOxUNIGNORExUSER,
  //! The user has requested to send a file to this user.
  PROTOxSENDxFILE,
  //! The user has requested to send a chat invitation to this user.
  PROTOxSENDxCHAT,
  //! The user is cancelling an event (chat, secure, or file for ICQ)
  PROTOxCANCELxEVENT,
  //! The user has requested to send an accept/refuse reply to a file/chat
  //! request
  PROTOxSENDxEVENTxREPLY,
  //! The user has opened a chat window with this user
  PROTOxOPENEDxWINDOW,
  //! The user has closed a chat window with this user
  PROTOxCLOSEDxWINDOW,
  //! The user has requested an SSL secure channel with this user
  PROTOxOPENxSECURE,
  //! The user has requested to close the SSL secure channel with this user
  PROTOxCLOSExSECURE
};

//! The class that gets passed to protocol plugins when a signal
//! is sent.
class CSignal
{
public:
  CSignal(SIGNAL_TYPE, const char *, unsigned long nCID = 0);
  CSignal(CSignal *);
  virtual ~CSignal();

  //! The signal is being sent to the plugin.
  SIGNAL_TYPE Type() const { return m_eType; }
  //! The user id that this signal is being used for.
  const char* Id() const { return m_szId; }
  //! The conversation id to use (gets the socket).
  unsigned long CID() const { return m_nCID; }
  //! The calling thread.
  pthread_t Thread() const { return thread_plugin; }

private:
  char  *m_szId;
  SIGNAL_TYPE m_eType;
  unsigned long m_nCID;
  pthread_t thread_plugin;
};

class CLogonSignal : public CSignal
{
public:
  CLogonSignal(unsigned long);
  virtual ~CLogonSignal();
  //! The requested initial status.
  unsigned long LogonStatus() const { return m_nLogonStatus; }

private:
  unsigned long m_nLogonStatus;
};

class CLogoffSignal : public CSignal
{
public:
  CLogoffSignal();
  virtual ~CLogoffSignal();
};

class CChangeStatusSignal : public CSignal
{
public:
  CChangeStatusSignal(unsigned long);
  virtual ~CChangeStatusSignal();
  //! The requested status.
  unsigned long Status() const { return m_nStatus; }

private:
  unsigned long m_nStatus;
};

class CAddUserSignal : public CSignal
{
public:
  CAddUserSignal(const char *, bool);
  virtual ~CAddUserSignal();
  //! True if authorization is required to add this user.
  bool AuthRequired() const { return m_bAuthRequired; }

private:
  bool m_bAuthRequired;
};

class CRemoveUserSignal : public CSignal
{
public:
  CRemoveUserSignal(const char *);
  virtual ~CRemoveUserSignal();
};

class CRenameUserSignal : public CSignal
{
public:
  CRenameUserSignal(const char *);
  virtual ~CRenameUserSignal();
};

/*! \brief Signal to a protocol plugin to send a messasge

    This is sent when ProtoSendMessage has been called. It
    contains the information necessary for a message to be
    sent by a protocol plugin
*/
class CSendMessageSignal : public CSignal
{
public:
  CSendMessageSignal(const char *szId, const char *szMsg, unsigned long nCID = 0);
  virtual ~CSendMessageSignal();
  //! The message to be sent
  const char* Message() const { return m_szMsg; }

private:
  char *m_szMsg;
};

class CTypingNotificationSignal : public CSignal
{
public:
  CTypingNotificationSignal(const char *szId, bool bActive, unsigned long nCID = 0);
  virtual ~CTypingNotificationSignal();
  bool Active() const { return m_bActive; }
private:
  bool m_bActive;
};

class CGrantAuthSignal : public CSignal
{
public:
  CGrantAuthSignal(const char *, const char *);
  virtual ~CGrantAuthSignal();
  const char* Message() const { return m_szMsg; }

private:
  char *m_szMsg;
};

class CRefuseAuthSignal : public CSignal
{
public:
  CRefuseAuthSignal(const char *, const char *);
  virtual ~CRefuseAuthSignal();

private:
  char *m_szMsg;
};

class CRequestInfo : public CSignal
{
public:
  CRequestInfo(const char *);
  virtual ~CRequestInfo();
};

class CUpdateInfoSignal : public CSignal
{
public:
  CUpdateInfoSignal(const char *szAlias, const char *szFirstName,
    const char *szLastName, const char *szEmail,
    const char *szCity,
    const char *szState, const char *szPhoneNumber,
    const char *szFaxNumber, const char *szAddress,
    const char *szCellularNumber, const char *szZipCode);
  virtual ~CUpdateInfoSignal();

  const char* Alias() const { return m_szAlias; }
  const char* FirstName() const { return m_szFirstName; }
  const char* LastName() const { return m_szLastName; }
  const char* Email() const { return m_szEmail; }
  const char* City() const { return m_szCity; }
  const char* State() const { return m_szState; }
  const char* PhoneNumber() const { return m_szPhoneNumber; }
  const char* FaxNumber() const { return m_szFaxNumber; }
  const char* Address() const { return m_szAddress; }
  const char* CellNumber() const { return m_szCellNumber; }
  const char* ZipCode() const { return m_szZipCode; }

private:
  char *m_szAlias,
       *m_szFirstName,
       *m_szLastName,
       *m_szEmail,
       *m_szCity,
       *m_szState,
       *m_szPhoneNumber,
       *m_szFaxNumber,
       *m_szAddress,
       *m_szCellNumber,
       *m_szZipCode;
};

class CRequestPicture : public CSignal
{
public:
  CRequestPicture(const char *);
  virtual ~CRequestPicture();
};

class CBlockUserSignal : public CSignal
{
public:
  CBlockUserSignal(const char *);
  virtual ~CBlockUserSignal();
};

class CUnblockUserSignal : public CSignal
{
public:
  CUnblockUserSignal(const char *);
  virtual ~CUnblockUserSignal();
};

class CAcceptUserSignal : public CSignal
{
public:
  CAcceptUserSignal(const char *);
  virtual ~CAcceptUserSignal();
};

class CUnacceptUserSignal : public CSignal
{
public:
  CUnacceptUserSignal(const char *);
  virtual ~CUnacceptUserSignal();
};

class CIgnoreUserSignal : public CSignal
{
public:
  CIgnoreUserSignal(const char *);
  virtual ~CIgnoreUserSignal();
};

class CUnignoreUserSignal : public CSignal
{
public:
  CUnignoreUserSignal(const char *);
  virtual ~CUnignoreUserSignal();
};

class CSendFileSignal : public CSignal
{
public:
  CSendFileSignal(const char *_szUser, const char *_szFile,
                  const char *_szMessage, ConstFileList &_lFileList);
  virtual ~CSendFileSignal();

  const char* GetFileName() const { return m_szFile; }
  const char* GetMessage() const { return m_szMessage; }
  ConstFileList GetFileList() const { return m_lFileList; }

private:
  char *m_szFile,
       *m_szMessage;
  ConstFileList m_lFileList;
};

class CSendChatSignal : public CSignal
{
public:
  CSendChatSignal(const char *_szUser, const char *_szMessage);
  virtual ~CSendChatSignal();

  const char* GetMessage() const { return m_szMessage; }

private:
  char *m_szMessage;
};

class CCancelEventSignal : public CSignal
{
public:
  CCancelEventSignal(const char *_szUser, unsigned long _nFlag);
  virtual ~CCancelEventSignal();

  unsigned long GetFlag() const { return m_nFlag; }

private:
  unsigned long m_nFlag;
};

class CSendEventReplySignal : public CSignal
{
public:
  CSendEventReplySignal(const char *_szUser, const char *_szMessage,
                        bool _bAccepted, unsigned short nPort,
                        unsigned long nSequence = 0,
                        unsigned long nFlag1 = 0, unsigned long nFlag2 = 0,
                        bool bDirect = false);
  virtual ~CSendEventReplySignal();

  const char* GetMessage() const { return m_szMessage; }
  bool GetAccept() const { return m_bAccept; }
  bool GetDirect() const { return m_bDirect; }
  unsigned short GetPort() const { return m_nPort; }
  unsigned long GetSequence() const { return m_nSequence; }
  unsigned long GetFlag() const { return m_nFlag; }
  unsigned long GetFlag2() const { return m_nFlag2; }

private:
  char *m_szMessage;
  bool m_bAccept,
       m_bDirect;
  unsigned short m_nPort;
  unsigned long m_nSequence,
                m_nFlag,
                m_nFlag2;
};

class COpenedWindowSignal : public CSignal
{
public:
  COpenedWindowSignal(const char *);
  virtual ~COpenedWindowSignal();
};

class CClosedWindowSignal : public CSignal
{
public:
  CClosedWindowSignal(const char *);
  virtual ~CClosedWindowSignal();
};

class COpenSecureSignal : public CSignal
{
public:
  COpenSecureSignal(const char *);
  virtual ~COpenSecureSignal();
};

class CCloseSecureSignal : public CSignal
{
public:
  CCloseSecureSignal(const char *);
  virtual ~CCloseSecureSignal();
};

#endif

