/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_CONTACTLIST_USER_H
#define LICQ_CONTACTLIST_USER_H

#include <boost/noncopyable.hpp>
#include <ctime>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "../macro.h"
#include "../thread/lockable.h"
#include "../userid.h"

namespace LicqIcq
{
class IcqProtocol;
void* MonitorSockets_func();
}

namespace LicqMsn
{
class CMSN;
}

namespace LicqJabber
{
class Plugin;
}

namespace LicqDaemon
{
class UserManager;
}

namespace Licq
{
class INetSocket;
class IniFile;
class UserEvent;

const unsigned short LAST_ONLINE        = 0;
const unsigned short LAST_RECV_EVENT    = 1;
const unsigned short LAST_SENT_EVENT    = 2;
const unsigned short LAST_CHECKED_AR    = 3;

typedef std::vector <class UserEvent*> UserEventList;
typedef std::set<int> UserGroupList;
typedef std::list<UserEvent*> HistoryList;


/**
 * A contact in the Licq user list including all information for that user
 *
 * Unless otherwise noted, members should only be accessed while holding a lock
 * to avoid race conditions between threads. User objects are normally
 * retrieved by instansiating a UserReadGuard or a UserWriteGuard which will
 * lock the user upon creation and release it when destroyed.
 * Modifying a user object will not trigger any server side changes or generate
 * events to notify plugins, that is the responsibility of the caller. For some
 * members (such as group memberships) user manager functions exist that should
 * be used for this purpose.
 */
class User : public Lockable
{
public:
  enum Gender
  {
    GenderUnspecified = 0,
    GenderFemale = 1,
    GenderMale = 2,
  };

  static const unsigned short AgeUnspecified = 0xFFFF;
  static const int TimezoneUnknown = 48*3600;

  enum SaveGroups
  {
    SaveAll             = 0xFFFF, // Save everything
    SaveUserInfo        = 0x0001, // Save contact information (name, alias,...)
    SaveLicqInfo        = 0x0002, // Save Licq specific data (timestamps, groups,...)
    SaveOwnerInfo       = 0x0004, // Save owner data (awaymsg, server,...)
    SaveNewMessagesInfo = 0x0008, // Save status of unread messages
    SavePictureInfo     = 0x0010, // Save picture information
  };

  /**
   * Write user data to file
   *
   * @param group Sub part of data to update (from SaveGroups above)
   */
  void save(unsigned group = SaveAll);

  /**
   * Get id for user. This is an id used locally by Licq and is persistant for
   * each user.
   *
   * @return User id
   */
  const UserId& id() const                      { return myId; }

  /**
   * Get account id that server protocol uses to identify user
   * The format and usage of this string may vary between protocols
   *
   * @return account id
   */
  const std::string& accountId() const          { return myId.accountId(); }

  /**
   * Get protocol instance this user belongs to
   *
   * @return protocol instance id
   */
  unsigned long protocolId() const
  { return myId.protocolId(); }

  /// Capabilities for protocol (cached here for convenience)
  unsigned long protocolCapabilities() const { return myProtocolCapabilities; }

  // General Info
  //!Retrieves the user's alias.
  const std::string& getAlias() const           { return myAlias; }
  //!Retrieves the user's first name.
  std::string getFirstName() const              { return getUserInfoString("FirstName"); }
  //!Retrieves the user's last name.
  std::string getLastName() const               { return getUserInfoString("LastName"); }

  /**
   * Convenience function for getting full name of user
   *
   * @return First name plus last name separated by a space if both are set
   */
  std::string getFullName() const;

  /**
   * Convenience function for getting email address for user
   *
   * @return Email address for user if available, empty string otherwise
   */
  std::string getEmail() const;

  //!Returns true if the user requires you to be authorized to add
  //!them to anyone's ICQ list.
  bool GetAuthorization() const                 { return m_bAuthorization; }
  //!Retrieves the user's cellular phone number.
  std::string getCellularNumber() const         { return getUserInfoString("CellularNumber"); }

  // Picture Info
  bool GetPicturePresent() const                { return m_bPicturePresent; }

  /**
   * Get path for file with user picture
   * Note: This function will not check if file actually (should) exist
   *
   * @return Path to user picture
   */
  const std::string& pictureFileName() const    { return myPictureFileName; }

  /**
   * Read the user's picture and store the data in @a pictureData
   * @return True if the picture could be read
   */
  bool readPictureData(std::string& pictureData) const;

  /**
   * Write @a pictureData to the user's picture file
   * @return True if the picture could be written
   */
  bool writePictureData(const std::string& pictureData) const;

  /**
   * Removes the user's picture file
   * @return True if the picture could be removed
   */
  bool deletePictureData() const;

  // Licq Info
  bool GetAwaitingAuth() const                  { return m_bAwaitingAuth; }

  //!Retrieves the user's auto response message that was last seen.
  const std::string& autoResponse() const       { return myAutoResponse; }
  //!Retrieves the encoding Licq uses for this user
  const std::string& userEncoding() const;
  bool SendServer() const                       { return m_bSendServer; }
  bool EnableSave() const                       { return m_bEnableSave; }
  bool ShowAwayMsg() const                      { return m_bShowAwayMsg; }
  const std::string& clientInfo() const         { return myClientInfo; }
  bool UserUpdated() const                      { return m_bUserUpdated; }
  time_t LastOnline() const                     { return m_nLastCounters[LAST_ONLINE]; }
  time_t LastSentEvent() const                  { return m_nLastCounters[LAST_SENT_EVENT]; }
  time_t LastReceivedEvent() const              { return m_nLastCounters[LAST_RECV_EVENT]; }
  time_t LastCheckedAutoResponse() const        { return m_nLastCounters[LAST_CHECKED_AR]; }
  time_t OnlineSince() const                    { return m_nOnlineSince; }
  time_t IdleSince() const                      { return m_nIdleSince; }
  time_t RegisteredTime() const                 { return m_nRegisteredTime; }
  bool UseGPG() const                           { return m_bUseGPG; }
  const std::string& gpgKey() const             { return myGpgKey; }
  bool AutoChatAccept() const                   { return myAutoAcceptChat; }
  bool AutoFileAccept() const                   { return myAutoAcceptFile; }
  bool AutoSecure() const                       { return myAutoSecure; }
  bool AcceptInAway() const                     { return myAcceptInAway; }
  bool AcceptInNA() const                       { return myAcceptInNotAvailable; }
  bool AcceptInOccupied() const                 { return myAcceptInOccupied; }
  bool AcceptInDND() const                      { return myAcceptInDoNotDisturb; }
  unsigned statusToUser() const                 { return myStatusToUser; }
  bool KeepAliasOnUpdate() const                { return m_bKeepAliasOnUpdate; }
  const std::string& customAutoResponse() const { return myCustomAutoResponse; }
  bool NotInList() const                        { return m_bNotInList; }

  /// Time when user went away or zero if online/offline or time not known
  time_t awaySince() const
  { return myAwaySince; }

  /// Set time when user went away
  void setAwaySince(time_t t)
  { myAwaySince = t; }

  /// Convert a unixtime to a relative string (e.g. "1 Hour 47 Minutes")
  static std::string RelativeStrTime(time_t t);

  enum usprintf_quotes
  {
    usprintf_quotenone  = 0,
    usprintf_quotepipe  = 1,
    usprintf_quoteall   = 2,
  };

  /**
   * Perform printf style convertion of a string using data from the user object
   *
   * @param format Input string
   * @param quotes Add quotes around all parameters, never or just on lines starting with pipe
   * @param toDos Add carrige return for all newlines
   * @param allowFieldWidth True to allow width parameter for fields
   * @return Input string with parameters replaced
   */
  std::string usprintf(const std::string& format, int quotes = usprintf_quotenone, bool toDos = false, bool allowFieldWidth = true) const;

  // General Info
  virtual void setAlias(const std::string& alias);
  void SetAuthorization (bool n)             {  m_bAuthorization = n; save(SaveUserInfo);  }

  /**
   * Get string user info
   *
   * @param key Name of property to get
   * @return Property value if string, otherwise empty
   */
  std::string getUserInfoString(const std::string& key) const;

  /**
   * Get numeric user info
   *
   * @param key Name of property to get
   * @return Property value if unsigned int, otherwise 0
   */
  unsigned int getUserInfoUint(const std::string& key) const;

  /**
   * Get boolean user info
   *
   * @param key Name of property to get
   * @return Property value if bool, otherwise false
   */
  bool getUserInfoBool(const std::string& key) const;

  /**
   * Set string user info
   *
   * @param key Name of property to set, must already exist
   * @param value New value for property
   */
  void setUserInfoString(const std::string& key, const std::string& value);

  /**
   * Set numeric user info
   *
   * @param key Name of property to set, must already exist
   * @param value New value for property
   */
  void setUserInfoUint(const std::string& key, unsigned int value);

  /**
   * Set bool user info
   *
   * @param key Name of property to set, must already exist
   * @param value New value for property
   */
  void setUserInfoBool(const std::string& key, bool value);

  /// Get timezone (as seconds from GMT)
  int timezone() const { return myTimezone; }

  /// Set timezone
  void setTimezone(int tz) { myTimezone = tz; save(SaveUserInfo); }

  // Picture info
  void SetPicturePresent(bool b)      { m_bPicturePresent = b; save(SavePictureInfo); }

  // Licq Info
  void SetAwaitingAuth(bool b)        { m_bAwaitingAuth = b; }
  void SetEnableSave(bool s)          { if (m_bOnContactList) m_bEnableSave = s; }
  void SetSendServer(bool s)          { m_bSendServer = s; save(SaveLicqInfo); }
  void setAutoResponse(const std::string& s) { myAutoResponse = s; }
  void setUserEncoding(const std::string& s) { myEncoding = s; }
  void SetShowAwayMsg(bool s)         { m_bShowAwayMsg = s; }
  void setClientInfo(const std::string& s)      { myClientInfo = s; }
  void SetUserUpdated(bool s)         { m_bUserUpdated = s; }
  void SetAutoChatAccept(bool s)        { myAutoAcceptChat = s; save(SaveLicqInfo); }
  void SetAutoFileAccept(bool s)        { myAutoAcceptFile = s; save(SaveLicqInfo); }
  void SetAutoSecure(bool s)            { myAutoSecure = s; save(SaveLicqInfo); }
  void SetAcceptInAway(bool s)          { myAcceptInAway = s; save(SaveLicqInfo); }
  void SetAcceptInNA(bool s)            { myAcceptInNotAvailable = s; save(SaveLicqInfo); }
  void SetAcceptInOccupied(bool s)      { myAcceptInOccupied = s; save(SaveLicqInfo); }
  void SetAcceptInDND(bool s)           { myAcceptInDoNotDisturb = s; save(SaveLicqInfo); }
  void SetUseGPG(bool b)                        { m_bUseGPG = b; save(SaveLicqInfo); }
  void setGpgKey(const std::string& c) { myGpgKey = c; save(SaveLicqInfo); }
  void setStatusToUser(unsigned s)      { myStatusToUser = s; save(SaveLicqInfo); }
  void SetKeepAliasOnUpdate(bool b)   { m_bKeepAliasOnUpdate = b; }
  void setCustomAutoResponse(const std::string& s) { myCustomAutoResponse = s; save(SaveLicqInfo); }
  void clearCustomAutoResponse()            { setCustomAutoResponse(""); }

  /**
   * Is user writing a message?
   *
   * @return True if user is currently typing a message
   */
  bool isTyping() const
  { return myIsTyping; }

  /**
   * Set typing flag for user
   * This function should only be called by protocol plugins
   *
   * @param True if user is currently typing a message
   */
  void setIsTyping(bool isTyping)
  { myIsTyping = isTyping; }

  enum StatusFlags
  {
    // Status flags (multiple can be set if supported by protocol)
    OnlineStatus        = 1<<0,         // Always set unless offline
    IdleStatus          = 1<<1,
    InvisibleStatus     = 1<<2,

    // Away flags (maximum one should be set)
    AwayStatus          = 1<<8,
    NotAvailableStatus  = 1<<9,
    OccupiedStatus      = 1<<10,
    DoNotDisturbStatus  = 1<<11,
    FreeForChatStatus   = 1<<12,

    // Convenience constants and masks for testing
    OfflineStatus       = 0,
    AwayStatuses        = AwayStatus | NotAvailableStatus | OccupiedStatus | DoNotDisturbStatus,
    MessageStatuses     = AwayStatuses | FreeForChatStatus,
  };

  /**
   * Get current status for user
   *
   * @return Mask of flags from StatusFlags
   */
  unsigned status() const
  { return myStatus; }

  /**
   * Change status for a user (or owner) and signal plugins
   * This function is used by protocol plugins to report status changes
   * A PluginSignal for the status changed is automatically sent and if
   * changing from offline, an OnEventOnline is also triggered.
   *
   * @param newStatus New status for user
   * @param onlineSince Time user came online or zero for now. Ignored unless changing from offline
   */
  void statusChanged(unsigned newStatus, time_t onlineSince = 0);

  /**
   * Convenience function to check if if user is online
   *
   * @return True if user is online
   */
  bool isOnline() const
  { return (myStatus & OnlineStatus) != 0; }

  /**
   * Convenience function to check if user is away
   *
   * @return True if user is away/na/occupied/dnd
   */
  bool isAway() const
  { return (status() & AwayStatuses) != 0; }

  /**
   * Convenience function to check if user is invisible
   *
   * @return True if user is online and invisible
   */
  bool isInvisible() const
  { return (myStatus & InvisibleStatus) != 0; }

  bool webPresence() const                      { return myWebPresence; }
  bool hideIp() const                           { return myHideIp; }
  bool birthdayFlag() const                     { return myBirthdayFlag; }
  bool homepageFlag() const                     { return myHomepageFlag; }
  void setWebPresence(bool f)                   { myWebPresence = f; }
  void setHideIp(bool f)                        { myHideIp = f; }
  void setBirthdayFlag(bool f)                  { myBirthdayFlag = f; }
  void setHomepageFlag(bool f)                  { myHomepageFlag = f; }

  enum SecureChannelSupport
  {
    SecureChannelUnknown        = 0,
    SecureChannelNotSupported   = 1,
    SecureChannelSupported      = 2,
  };
  unsigned secureChannelSupport() const         { return mySecureChannelSupport; }
  void setSecureChannelSupport(unsigned n)      { mySecureChannelSupport = n; }

  /**
   * Convert user status to a string
   *
   * @param full False to get a short abbreviation
   * @param markInvisible False to handle invisible as a separate status
   * @return A string representing the status of the user
   */
  std::string statusString(bool full = true, bool markInvisible = true) const
  { return statusToString(status(), full, markInvisible); }

  /**
   * Convert status to a string
   *
   * @param status Status to convert
   * @param full False to get a short abbreviation
   * @param markInvisible False to handle invisible as a separate status
   * @return A string representing the status
   */
  static std::string statusToString(unsigned status, bool full = true, bool markInvisible = true);

  /**
   * Parse status from a string
   *
   * @param strStatus String to parse
   * @param retStatus Variable to put parsed status in
   * @return True if strStatus was a valid status, false if parse failed
   */
  static bool stringToStatus(const std::string& strStatus, unsigned& retStatus);

  /**
   * Get the most relevant status flag from a status
   *
   * @param status Status to convert
   * @return A status with (maximum) one flag set
   */
  static unsigned singleStatus(unsigned status);

  int Birthday(unsigned short nDayRange = 0) const;

  // Message/History functions
  unsigned short NewMessages() const            { return(m_vcMessages.size()); }
  void CancelEvent(unsigned short index);
  const UserEvent* EventPeek(unsigned short index) const;
  const UserEvent* EventPeekId(int id) const;
  const UserEvent* EventPeekFirst() const;
  const UserEvent* EventPeekLast() const;
  UserEvent *EventPop();
  void EventClear(unsigned short);
  void EventClearId(int);
  void EventPush(UserEvent *);
  int GetHistory(HistoryList& history) const;
  static void ClearHistory(HistoryList& h);

  /**
   * Get user groups this user is member of
   *
   * @return List of groups
   */
  const UserGroupList& GetGroups() const { return myGroups; }

  /**
   * Set user groups this user is member of
   *
   * @param groups List of groups
   */
  void SetGroups(const UserGroupList& groups) { myGroups = groups; }

  /**
   * Check if user is member of a group
   *
   * @param groupId Id of a user group to check
   * @return True if group exists and user is member
   */
  bool isInGroup(int groupId) const;

  /**
   * Convenience function to set membership of user for a group
   *
   * @param groupId Id of user group
   * @param member True to add user to group, false to remove user from group
   */
  void setInGroup(int groupId, bool member);

  /**
   * Add user to a group
   *
   * @param groupId Id of user group to add
   */
  void addToGroup(int groupId);

  /**
   * Remove user from a group
   *
   * @param groupId Id of user group to leave
   * @return True if group was valid and user was a member
   */
  bool removeFromGroup(int groupId);

  /**
   * Get server group
   *
   * @return Current server group or -1 if protocol allows multiple groups
   */
  int serverGroup() const
  { return myServerGroup; }

  /**
   * Set server group
   *
   * @param groupId New server group for user
   */
  void setServerGroup(int groupId)
  { if (myServerGroup > -1) myServerGroup = groupId; }

  // Short cuts to above functions
  bool InvisibleList() const    { return myOnInvisibleList; }
  bool VisibleList() const      { return myOnVisibleList; }
  bool OnlineNotify() const     { return myOnlineNotify; }
  bool IgnoreList() const       { return myOnIgnoreList; }
  bool NewUser() const          { return myNewUser; }
  void SetInvisibleList(bool s) { myOnInvisibleList = s;; }
  void SetVisibleList(bool s)   { myOnVisibleList = s; }
  void SetOnlineNotify(bool s)  { myOnlineNotify = s; }
  void SetIgnoreList(bool s)    { myOnIgnoreList = s; }
  void SetNewUser(bool s)       { myNewUser = s; }

  /**
   * Enable/disable on event blocking for this user
   * This function can be used by UI plugins to block sounds when a user window has focus
   *
   * @param block True to block on events, false to enable
   */
  void setOnEventsBlocked(bool block) { myOnEventsBlocked = block; }

  /**
   * Check if on events should be blocked for this user
   *
   * @return True if on event should be blocked
   */
  bool onEventsBlocked() const { return myOnEventsBlocked; }

  // Time
  time_t LocalTime() const;
  int LocalTimeOffset() const;
  static int systemTimezone();

  // Ip/Port functions
  unsigned long Ip() const                      { return m_nIp; }
  unsigned long IntIp() const                   { return m_nIntIp; }
  unsigned short Port() const                   { return m_nPort; }
  unsigned short LocalPort() const              { return m_nLocalPort; }
  virtual void SetIpPort(unsigned long nIp, unsigned short nPort);
  void SetIp(unsigned long nIp)                 { SetIpPort(nIp, Port()); }
  void SetPort(unsigned short nPort)            { SetIpPort(Ip(), nPort); }
  void SetIntIp(unsigned long s)                { m_nIntIp = s; }

  bool SendIntIp() const                        { return m_bSendIntIp; }
  void SetSendIntIp(bool s)                     { m_bSendIntIp = s; }

  // for backward compatibility
  unsigned long RealIp() const                  { return IntIp(); }
  void SetRealIp(unsigned long s) { SetIntIp(s); }
  bool SendRealIp() const                       { return SendIntIp(); }
  void SetSendRealIp(bool s)      { SetSendIntIp(s); }

  std::string ipToString() const;
  virtual std::string internalIpToString() const;
  std::string portToString() const;

  /**
   * Check if a direct connection might be possible
   *
   * @return False if direct connection can't or shouldn't be done
   */
  virtual bool canSendDirect() const;

  /**
   * Clear a user specific socket descriptor
   * Called by socket manager when a socket is closed
   * Base implementation does nothing, reimplement in subclass if needed
   *
   * @param s Socket that was closed and is about to be deleted
   */
  virtual void clearSocketDesc(INetSocket* s);

  // Events functions
  static unsigned short getNumUserEvents();
  static void incNumUserEvents();
  static void decNumUserEvents();

  // Last event functions
  time_t Touched() const                        { return m_nTouched; }
  void Touch()           {  m_nTouched = time(NULL); }

  // Crypto
  bool Secure() const                           { return m_bSecure; }

  void SetOfflineOnDisconnect(bool b) { m_bOfflineOnDisconnect = b; }
  bool OfflineOnDisconnect() { return m_bOfflineOnDisconnect; }

  bool isUser() const
  { return !myId.isOwner(); }

protected:
  /// Constructor
  User(const UserId& id, bool temporary = false);

  /// Destructor
  virtual ~User();

  /// Get the config file for this user
  IniFile& userConf();

  virtual void saveLicqInfo();
  virtual void saveUserInfo();
  virtual void saveOwnerInfo();
  virtual void saveNewMessagesInfo();
  virtual void savePictureInfo();

  void SetSecure(bool s) { m_bSecure = s; }
  bool ConnectionInProgress() const { return m_bConnectionInProgress; }
  void SetConnectionInProgress(bool c)  { m_bConnectionInProgress = c; }

  void SetLastSentEvent()           { m_nLastCounters[LAST_SENT_EVENT] = time(NULL); }
  void SetLastReceivedEvent()       { m_nLastCounters[LAST_RECV_EVENT] = time(NULL); }
  void SetLastCheckedAutoResponse() { m_nLastCounters[LAST_CHECKED_AR] = time(NULL); }

  void SetIdleSince(time_t t)       { m_nIdleSince = t; }
  void SetRegisteredTime(time_t t)  { m_nRegisteredTime = t; }

  const UserId myId;
  unsigned long myProtocolCapabilities;

  time_t m_nTouched;
  time_t m_nLastCounters[4];
  time_t m_nOnlineSince;
  time_t m_nIdleSince;
  time_t myAwaySince;
  time_t m_nRegisteredTime;
  bool m_bOnContactList;
  unsigned long m_nIp, m_nIntIp;
  bool m_bUserUpdated;
  unsigned m_nPort, m_nLocalPort;
  bool myIsTyping;
  unsigned myStatus;
  UserGroupList myGroups;               /**< List of user groups */
  int myServerGroup;
  bool myWebPresence;
  bool myHideIp;
  bool myBirthdayFlag;
  bool myHomepageFlag;
  unsigned mySecureChannelSupport;
  std::string myClientInfo;
  std::string myAutoResponse;
  std::string myEncoding;
  std::string myCustomAutoResponse;
  bool myOnlineNotify;
  bool myOnVisibleList;
  bool myOnInvisibleList;
  bool myOnIgnoreList;
  bool myNewUser;
  bool m_bSendIntIp,
       m_bSendServer,
       m_bEnableSave,
       m_bShowAwayMsg,
       m_bOfflineOnDisconnect,
       m_bConnectionInProgress,
       m_bSecure,
       m_bNotInList;
  unsigned myStatusToUser;
  bool m_bKeepAliasOnUpdate;
  bool myOnEventsBlocked;

  bool myAutoAcceptChat;
  bool myAutoAcceptFile;
  bool myAutoSecure;
  bool myAcceptInAway;
  bool myAcceptInNotAvailable;
  bool myAcceptInOccupied;
  bool myAcceptInDoNotDisturb;

  // GPG data
  bool m_bUseGPG;
  std::string myGpgKey;

  // General Info
  std::string myAlias;
  int myTimezone;
  bool m_bAuthorization;

  // Picture Info
  std::string myPictureFileName;
  bool m_bPicturePresent;

  // Server Side ID, Group SID
  bool m_bAwaitingAuth;

  UserEventList m_vcMessages;

  static unsigned short s_nNumUserEvents;

  static pthread_mutex_t mutex_nNumUserEvents;

  friend class LicqIcq::IcqProtocol;
  friend void* LicqIcq::MonitorSockets_func();
  friend class LicqMsn::CMSN;
  friend class LicqJabber::Plugin;

private:
  LICQ_DECLARE_PRIVATE();

  // Allow the user manager to access private members
  friend class LicqDaemon::UserManager;
};


/**
 * Read mutex guard for Licq::User
 */
class UserReadGuard : public ReadMutexGuard<User>
{
public:
  /**
   * Constructor, will fetch and lock a user based on user id
   * Note: Always check that the user was actually fetched before using
   *
   * @param userId Id of user to fetch
   * @param addUser True if user should be added (as temporary) if not found
   * @param retWasAdded If not null, will be set to true if user was added
   */
  UserReadGuard(const UserId& userId, bool addUser = false, bool* retWasAdded = NULL);

  // Derived constructors
  UserReadGuard(const User* user, bool locked = false)
    : ReadMutexGuard<User>(user, locked)
  { }
  UserReadGuard(ReadMutexGuard<User>* guard)
    : ReadMutexGuard<User>(guard)
  { }
};

/**
 * Write mutex guard for Licq::User
 */
class UserWriteGuard : public WriteMutexGuard<User>
{
public:
  /**
   * Constructor, will fetch and lock a user based on user id
   * Note: Always check that the user was actually fetched before using
   *
   * @param userId Id of user to fetch
   * @param addUser True if user should be added (as temporary) if not found
   * @param retWasAdded If not null, will be set to true if user was added
   */
  UserWriteGuard(const UserId& userId, bool addUser = false, bool* retWasAdded = NULL);

  // Derived constructors
  UserWriteGuard(User* user, bool locked = false)
    : WriteMutexGuard<User>(user, locked)
  { }
  UserWriteGuard(WriteMutexGuard<User>* guard)
    : WriteMutexGuard<User>(guard)
  { }
};

} // namespace Licq

#endif // LICQ_CONTACTLIST_USER_H
