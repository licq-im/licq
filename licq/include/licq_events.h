#ifndef ICQEVENT_H
#define ICQEVENT_H

#include <list>
#include <pthread.h>
#include <string>

#include "licq_message.h"
#include "licq/userid.h"

class CUserEvent;

namespace LicqDaemon
{
class PluginEventHandler;
class User;
}

namespace Licq
{
class Packet;
class User;

typedef std::list<std::string> StringList;
}


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

  friend class IcqProtocol;
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
  const Licq::UserId& userId() const { return myUserId; }

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
  CSearchAck(const Licq::UserId& userId);

  Licq::UserId myUserId;
  char *m_szAlias;
  char *m_szFirstName;
  char *m_szLastName;
  char *m_szEmail;
  unsigned long m_nMore;
  char m_nStatus;
  char m_nGender;
  char m_nAge;
  char m_nAuth;

  friend class IcqProtocol;
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
  const Licq::UserId& userId() const { return myUserId; }

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
  const Licq::User* UnknownUser() const;

  // Returns the event and transfers ownership to the calling function
  CUserEvent *GrabUserEvent();
  CSearchAck *GrabSearchAck();
  Licq::User* GrabUnknownUser();

  //!Compare this event to the id to see if the plugin matches a waiting
  //!event with the event that the daemon has signaled to the plugin.
  bool Equals(unsigned long) const;

  ~LicqEvent();

protected:
  LicqEvent(unsigned long id, int _nSocketDesc, Licq::Packet* p, ConnectType _eConnect,
      const Licq::UserId& userId = Licq::UserId(), CUserEvent* e = NULL);
  LicqEvent(const LicqEvent* e);

  // Daemon only
  unsigned short SubType() const     { return m_nSubType; }
  unsigned short ExtraInfo() const   { return m_nExtraInfo; }
  void SetSubType(unsigned short nSubType) { m_nSubType = nSubType; }
  bool NoAck() const           { return m_NoAck; }
  void SetNoAck(bool NoAck)    { m_NoAck = NoAck; }
  bool IsCancelled() const     { return m_bCancelled; }

  void AttachPacket(Licq::Packet* p);

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
  Licq::UserId myUserId;
  Licq::Packet* m_pPacket;
  pthread_t      thread_send;
  bool           thread_running;
  pthread_t      thread_plugin;

  CUserEvent    *m_pUserEvent;
  CExtendedAck  *m_pExtendedAck;
  CSearchAck    *m_pSearchAck;
  LicqDaemon::User* m_pUnknownUser;

  unsigned long  m_nEventId;

friend class COscarService;
friend class CMSN;
  friend class IcqProtocol;
friend class Jabber;
friend class LicqDaemon::PluginEventHandler;
friend void *ProcessRunningEvent_Client_tep(void *p);
friend void *ProcessRunningEvent_Server_tep(void *p);
friend void *OscarServiceSendQueue_tep(void *p);
friend void *MonitorSockets_tep(void *p);
};

// Temporary until all occurenses of deprecated name ICQEvent have been removed
typedef LicqEvent ICQEvent;

#endif
