/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef LICQ_EVENT_H
#define LICQ_EVENT_H

#include <pthread.h>
#include <string>

#include "userid.h"

class CMSN;
class COscarService;
class IcqProtocol;
namespace Jabber { class Plugin; }
void* ProcessRunningEvent_Client_tep(void* p);
void* ProcessRunningEvent_Server_tep(void* p);
void* OscarServiceSendQueue_tep(void* p);

namespace LicqDaemon
{
class PluginEventHandler;
}

namespace Licq
{
class Packet;
class UserEvent;

//-----CExtendedAck----------------------------------------------------------

/*! \brief Information on file and chat request responses.

    This class will provide information on what the result of a chat or file
    transfer request was.
*/
class ExtendedData
{
public:
  // Accessors

  //!Returns true if the remote end accepted the request, otherwise false.
  bool accepted() const { return myAccepted; }
  //!The port to connect to if the request was accpeted.
  unsigned short port() const { return myPort; }
  //!The reason for declining the request.
  const std::string& response() const { return myResponse; }

protected:
  ExtendedData(bool accepted, unsigned short port, const std::string& response)
    : myAccepted(accepted), myPort(port), myResponse(response)
  { /* Empty */ }

  bool myAccepted;
  unsigned short myPort;
  std::string myResponse;

  friend class ::IcqProtocol;
};


//-----CSearchAck------------------------------------------------------------
/*! \brief The response to a search request.

    Each search result is passed to the plugin as a CSearchAck.  If there
    was no search results, then a single CSearchAck is passed on with
    Result() returning EVENT_SUCCESS, which also signifies the search is
    complete.
*/
class SearchData
{
public:
  enum Status
  {
    StatusOffline = 0,  // User is offline
    StatusOnline = 1,   // User is online
    StatusDisabled = 2, // User has disabled online awareness
  };

  // Accessors
  //! Returns the alias (nickname) of the search result.
  const std::string& alias() const { return myAlias; }
  //! Returns the first name of the search result.
  const std::string& firstName() const { return myFirstName; }
  //! Returns the last name of the search result.
  const std::string& lastName() const { return myLastName; }
  //! Returns the e-mail address of the search result.
  const std::string& email() const { return myEmail; }

  /**
   * Get the user id
   *
   * @return User id of search match
   */
  const UserId& userId() const { return myUserId; }

  //! If non-zero, the number of search results that were found that could not
  //! be displayed.  The server has a 40 user limit on search results.  This
  //! is valid when Result() is EVENT_SUCCESS.
  unsigned long more() const { return myMore; }
  //! The online status of the search result.
  char status() const { return myStatus; }
  //! The gender of the search result.  Female = 1, Male = 2, Unknown = 3.
  char gender() const { return myGender; }
  //! The age of the serach result.
  char age() const { return myAge; }
  //! Non-zero if authorization is required to add this user.
  char auth() const { return myAuth; }

protected:
  SearchData(const UserId& userId)
    : myUserId(userId)
  { /* Empty */ }

  UserId myUserId;
  std::string myAlias;
  std::string myFirstName;
  std::string myLastName;
  std::string myEmail;
  unsigned long myMore;
  char myStatus;
  char myGender;
  char myAge;
  char myAuth;

  friend class ::IcqProtocol;
};


/*! \brief Plugin event messages

    This class is the main event class for talking to the ICQ server and to
    plugins.  Internally all messages/urls... become ICQEvents with the
    relevant data fields set.  A plugin will receive an event in response
    to any asynchronous function call (such as icqSendMessage) eventually.
*/
class Event
{
public:
  enum ConnectType
  {
    ConnectServer,
    ConnectUser,
    ConnectNone,
  };

  enum ResultType
  {
    ResultAcked,        // Event acked by reciepient
    ResultSuccess,      // Event successfully sent
    ResultFailed,       // Event failed
    ResultTimedout,     // Time out while communicating with remote socket
    ResultError,        // Other error
    ResultCancelled,    // Event cancelled by the user
  };

  // Accessors

  //!This is the result of the event.
  ResultType Result() const { return m_eResult; }

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
  const SearchData* SearchAck() const { return m_pSearchAck; }

  //!Special structure containing information relevant if this is a
  //!chat or file transfer accept or reject.
  const ExtendedData* ExtendedAck() const { return m_pExtendedAck; }

  //!Contains the actual CUserEvent containing the message/url...that was
  //!sent to Uin().  Can be used to resend the event.
  const UserEvent* userEvent() const { return m_pUserEvent; }

  // Returns the event and transfers ownership to the calling function
  UserEvent* GrabUserEvent();
  SearchData* GrabSearchAck();

  //!Compare this event to the id to see if the plugin matches a waiting
  //!event with the event that the daemon has signaled to the plugin.
  bool Equals(unsigned long) const;

  ~Event();

protected:
  Event(unsigned long id, int _nSocketDesc, Packet* p, ConnectType _eConnect,
      const UserId& userId = UserId(), UserEvent* e = NULL);
  Event(const Event* e);

  // Daemon only
  unsigned short SubType() const     { return m_nSubType; }
  unsigned short ExtraInfo() const   { return m_nExtraInfo; }
  void SetSubType(unsigned short nSubType) { m_nSubType = nSubType; }
  bool NoAck() const           { return m_NoAck; }
  void SetNoAck(bool NoAck)    { m_NoAck = NoAck; }
  bool IsCancelled() const     { return m_bCancelled; }

  void AttachPacket(Packet* p);

  // Compare this event to another one
  bool CompareEvent(int, unsigned short) const;
  bool CompareEvent(unsigned short) const;
  bool CompareSubSequence(unsigned long) const;
  unsigned long EventId() const;

  ConnectType    m_eConnect;
  ResultType m_eResult;
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
  UserId myUserId;
  Packet* m_pPacket;
  pthread_t      thread_send;
  bool           thread_running;
  pthread_t      thread_plugin;

  UserEvent* m_pUserEvent;
  ExtendedData* m_pExtendedAck;
  SearchData* m_pSearchAck;

  unsigned long  m_nEventId;

  friend class ::COscarService;
  friend class ::CMSN;
  friend class ::IcqProtocol;
  friend class Jabber::Plugin;
  friend class LicqDaemon::PluginEventHandler;
  friend void* ::ProcessRunningEvent_Client_tep(void* p);
  friend void* ::ProcessRunningEvent_Server_tep(void* p);
  friend void* ::OscarServiceSendQueue_tep(void* p);
//friend void *MonitorSockets_tep(void *p);
};

} // namespace Licq

#endif
