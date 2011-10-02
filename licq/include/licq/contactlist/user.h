/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq developers
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
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../buffer.h"
#include "../thread/lockable.h"
#include "../userid.h"

class CMSN;
class CSocketManager;
class IcqProtocol;
namespace Jabber { class Plugin; }
void* MonitorSockets_func();


namespace LicqDaemon
{
class User;
class UserManager;
}

namespace Licq
{
class IniFile;
class TCPSocket;
class UserEvent;

const unsigned short NORMAL_SID         = 0;
const unsigned short INV_SID            = 1;
const unsigned short VIS_SID            = 2;

const unsigned short LAST_ONLINE        = 0;
const unsigned short LAST_RECV_EVENT    = 1;
const unsigned short LAST_SENT_EVENT    = 2;
const unsigned short LAST_CHECKED_AR    = 3;

const unsigned short MAX_CATEGORY_SIZE  = 60;
const unsigned int MAX_CATEGORIES = 4;

typedef enum
{
  CAT_INTERESTS         = 0,
  CAT_ORGANIZATION      = 1,
  CAT_BACKGROUND        = 2,
  CAT_MAX               = 3,
} UserCat;

typedef std::vector <class UserEvent*> UserEventList;
typedef std::map<unsigned int, std::string> UserCategoryMap;
typedef std::set<int> UserGroupList;
typedef std::list<UserEvent*> HistoryList;

struct PhoneBookEntry
{
  std::string description;
  std::string areaCode;
  std::string phoneNumber;
  std::string extension;
  std::string country;
  unsigned long nActive;
  unsigned long nType;
  std::string gateway;
  unsigned long nGatewayType;
  unsigned long nSmsAvailable;
  unsigned long nRemoveLeading0s;
  unsigned long nPublish;
};

const unsigned short MAX_DESCRIPTION_SIZE  = 16;
const unsigned short MAX_AREAxCODE_SIZE    =  5;
const unsigned short MAX_PHONExNUMBER_SIZE = 16;
const unsigned short MAX_EXTENSION_SIZE    = 20;
const unsigned short MAX_GATEWAY_SIZE      = 64;
const unsigned short MAX_PICTURE_SIZE      = 8081;

enum EPhoneType
{
  TYPE_PHONE            = 0,
  TYPE_CELLULAR         = 1,
  TYPE_CELLULARxSMS     = 2,
  TYPE_FAX              = 3,
  TYPE_PAGER            = 4,
  TYPE_MAX              = 5
};

enum EGatewayType
{
  GATEWAY_BUILTIN = 1,
  GATEWAY_CUSTOM = 2,
};

enum EPublish
{
  PUBLISH_ENABLE = 1,
  PUBLISH_DISABLE = 2,
};

class ICQUserPhoneBook
{
public:
  ICQUserPhoneBook();
  ~ICQUserPhoneBook();
  void AddEntry(const struct PhoneBookEntry* entry);
  void SetEntry(const struct PhoneBookEntry* entry, unsigned long nEntry);
  void ClearEntry(unsigned long nEntry);
  void Clean();
  void SetActive(long nEntry);

  bool Get(unsigned long nEntry, const struct PhoneBookEntry** entry) const;

private:
  bool SaveToDisk(IniFile& conf);
  bool LoadFromDisk(IniFile& conf);

  std::vector<struct PhoneBookEntry> PhoneBookVector;

  friend class LicqDaemon::User;
};


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
  static const char TimezoneUnknown = (char)-100;

  enum SaveGroups
  {
    SaveAll             = 0xFFFF, // Save everything
    SaveUserInfo        = 0x0001, // Save contact information (name, alias,...)
    SaveLicqInfo        = 0x0002, // Save Licq specific data (timestamps, groups,...)
    SaveOwnerInfo       = 0x0004, // Save owner data (awaymsg, server,...)
    SaveNewMessagesInfo = 0x0008, // Save status of unread messages
    SavePictureInfo     = 0x0010, // Save picture information
    SavePhoneBook       = 0x0020, // Save ICQ Phone Book
  };

  /**
   * Write user data to file
   *
   * @param group Sub part of data to update (from SaveGroups above)
   */
  virtual void save(unsigned group = SaveAll) = 0;

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

  //!Retrieves the user's time code.
  char GetTimezone() const                      { return m_nTimezone; }
  //!Returns true if the user requires you to be authorized to add
  //!them to anyone's ICQ list.
  bool GetAuthorization() const                 { return m_bAuthorization; }
  //!Retrieves the user's cellular phone number.
  std::string getCellularNumber() const         { return getUserInfoString("CellularNumber"); }

  // More2 Info
  //!Retrieves the user's interests
  UserCategoryMap& getInterests()               { return myInterests; }
  const UserCategoryMap& getInterests() const   { return myInterests; }
  //!Retrieves the user's backgrounds
  UserCategoryMap& getBackgrounds()             { return myBackgrounds; }
  const UserCategoryMap& getBackgrounds() const { return myBackgrounds; }
  //!Retrieves the user's organizations
  UserCategoryMap& getOrganizations()           { return myOrganizations; }
  const UserCategoryMap& getOrganizations() const { return myOrganizations; }

  // Phone Book Info
  //!Retrives the user's phone book
  ICQUserPhoneBook *GetPhoneBook()      { return m_PhoneBook; }
  const ICQUserPhoneBook* GetPhoneBook() const  { return m_PhoneBook; }

  // Picture Info
  bool GetPicturePresent() const                { return m_bPicturePresent; }
  unsigned buddyIconType() const                { return myBuddyIconType; }
  char buddyIconHashType() const                { return myBuddyIconHashType; }
  const std::string& buddyIconHash() const      { return myBuddyIconHash; }
  const std::string& ourBuddyIconHash() const   { return myOurBuddyIconHash; }

  /**
   * Get path for file with user picture
   * Note: This function will not check if file actually (should) exist
   *
   * @return Path to user picture
   */
  const std::string& pictureFileName() const    { return myPictureFileName; }

  // Dynamic info fields for protocol plugins
  std::string GetPPField(const std::string &);

  // Licq Info
  bool GetAwaitingAuth() const                  { return m_bAwaitingAuth; }
  unsigned short GetSID() const                 { return m_nSID[NORMAL_SID]; }
  unsigned short GetInvisibleSID() const        { return m_nSID[INV_SID]; }
  unsigned short GetVisibleSID() const          { return m_nSID[VIS_SID]; }
  unsigned short GetGSID() const                { return m_nGSID; }

  //!Retrieves the user's auto response message that was last seen.
  const std::string& autoResponse() const       { return myAutoResponse; }
  //!Retrieves the encoding Licq uses for this user
  const std::string& userEncoding() const;
  //!True if they have sent the UTF8 Cap
  bool SupportsUTF8() const                     { return m_bSupportsUTF8; }
  bool SendServer() const                       { return m_bSendServer; }
  unsigned short SendLevel() const              { return m_nSendLevel; }
  bool EnableSave() const                       { return m_bEnableSave; }
  bool ShowAwayMsg() const                      { return m_bShowAwayMsg; }
  unsigned short Sequence(bool = false);
  bool directMode() const                       { return myDirectMode; }
  unsigned long Version() const                 { return m_nVersion; }
  const std::string& clientInfo() const         { return myClientInfo; }
  unsigned long ClientTimestamp() const         { return m_nClientTimestamp; }
  unsigned long OurClientTimestamp() const      { return m_nOurClientTimestamp; }
  unsigned long ClientInfoTimestamp() const     { return m_nClientInfoTimestamp; }
  unsigned long OurClientInfoTimestamp() const  { return m_nOurClientInfoTimestamp; }
  unsigned long ClientStatusTimestamp() const   { return m_nClientStatusTimestamp; }
  unsigned long OurClientStatusTimestamp() const { return m_nOurClientStatusTimestamp; }
  bool UserUpdated() const                      { return m_bUserUpdated; }
  unsigned short ConnectionVersion() const;
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
  void setAlias(const std::string& alias);
  void SetTimezone (const char n)            {  m_nTimezone = n; save(SaveUserInfo);  }
  void SetAuthorization (bool n)             {  m_bAuthorization = n; save(SaveUserInfo);  }

  /**
   * Get string user info
   *
   * @param key Name of property to get
   * @return Property value if string, otherwise empty
   */
  virtual std::string getUserInfoString(const std::string& key) const = 0;

  /**
   * Get numeric user info
   *
   * @param key Name of property to get
   * @return Property value if unsigned int, otherwise 0
   */
  virtual unsigned int getUserInfoUint(const std::string& key) const = 0;

  /**
   * Get boolean user info
   *
   * @param key Name of property to get
   * @return Property value if bool, otherwise false
   */
  virtual bool getUserInfoBool(const std::string& key) const = 0;

  /**
   * Set string user info
   *
   * @param key Name of property to set, must already exist
   * @param value New value for property
   */
  virtual void setUserInfoString(const std::string& key, const std::string& value) = 0;

  /**
   * Set numeric user info
   *
   * @param key Name of property to set, must already exist
   * @param value New value for property
   */
  virtual void setUserInfoUint(const std::string& key, unsigned int value) = 0;

  /**
   * Set bool user info
   *
   * @param key Name of property to set, must already exist
   * @param value New value for property
   */
  virtual void setUserInfoBool(const std::string& key, bool value) = 0;

  // Picture info
  void SetPicturePresent(bool b)      { m_bPicturePresent = b; save(SavePictureInfo); }
  void setBuddyIconType(unsigned s)     { myBuddyIconType = s; }
  void setBuddyIconHashType(char s)     { myBuddyIconHashType = s; }
  void setBuddyIconHash(const std::string& s) { myBuddyIconHash = s; }
  void setOurBuddyIconHash(const std::string& s) { myOurBuddyIconHash = s; }

  // Licq Info
  void SetAwaitingAuth(bool b)        { m_bAwaitingAuth = b; }
  void SetSID(unsigned short s)       { m_nSID[NORMAL_SID] = s; }
  void SetInvisibleSID(unsigned short s) { m_nSID[INV_SID] = s; }
  void SetVisibleSID(unsigned short s){ m_nSID[VIS_SID] = s; }
  void SetGSID(unsigned short s)      { m_nGSID = s; }
  void SetEnableSave(bool s)          { if (m_bOnContactList) m_bEnableSave = s; }
  void SetSendServer(bool s)          { m_bSendServer = s; save(SaveLicqInfo); }
  void SetSendLevel(unsigned short s) { m_nSendLevel = s; }
  void SetSequence(unsigned short s)  { m_nSequence = s; }
  void setAutoResponse(const std::string& s) { myAutoResponse = s; }
  void setUserEncoding(const std::string& s) { myEncoding = s; }
  void SetSupportsUTF8(bool b)        { m_bSupportsUTF8 = b; }
  void SetShowAwayMsg(bool s)         { m_bShowAwayMsg = s; }
  void setDirectMode(bool direct)       { myDirectMode = direct; }
  void SetVersion(unsigned long s)    { m_nVersion = s; }
  void setClientInfo(const std::string& s)      { myClientInfo = s; }
  void SetClientTimestamp(unsigned long s) { m_nClientTimestamp = s; }
  void SetOurClientTimestamp(unsigned long s) { m_nOurClientTimestamp = s; }
  void SetClientInfoTimestamp(unsigned long s) { m_nClientInfoTimestamp = s; }
  void SetOurClientInfoTimestamp(unsigned long s) { m_nOurClientInfoTimestamp = s; }
  void SetClientStatusTimestamp(unsigned long s) { m_nClientStatusTimestamp = s; }
  void SetOurClientStatusTimestamp(unsigned long s) { m_nOurClientStatusTimestamp = s; }
  void SetUserUpdated(bool s)         { m_bUserUpdated = s; }
  void SetConnectionVersion(unsigned short s)    { m_nConnectionVersion = s; }
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

  // Dynamic info fields for protocol plugins
  bool SetPPField(const std::string &, const std::string &);

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

  unsigned phoneFollowMeStatus() const          { return myPhoneFollowMeStatus; }
  unsigned icqPhoneStatus() const               { return myIcqPhoneStatus; }
  unsigned sharedFilesStatus() const            { return mySharedFilesStatus; }
  void setPhoneFollowMeStatus(unsigned n)       { myPhoneFollowMeStatus = n; save(SaveLicqInfo); }
  void setIcqPhoneStatus(unsigned n)            { myIcqPhoneStatus = n; }
  void setSharedFilesStatus(unsigned n)         { mySharedFilesStatus = n; }

  bool webPresence() const                      { return myWebPresence; }
  bool hideIp() const                           { return myHideIp; }
  bool birthdayFlag() const                     { return myBirthdayFlag; }
  bool homepageFlag() const                     { return myHomepageFlag; }
  void setWebPresence(bool f)                   { myWebPresence = f; }
  void setHideIp(bool f)                        { myHideIp = f; }
  void setBirthdayFlag(bool f)                  { myBirthdayFlag = f; }
  void setHomepageFlag(bool f)                  { myHomepageFlag = f; }

  enum DirectFlags
  {
    DirectDisabled      = 0,    // Direct contact not possible
    DirectAnyone        = 1,    // Direct contact with anyone
    DirectListed        = 2,    // Direct contact only with contacts in list
    DirectAuth          = 3,    // Direct contact only with authorized contacts
  };
  unsigned directFlag() const                   { return myDirectFlag; }
  void setDirectFlag(unsigned n)                { myDirectFlag = n; }

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
  virtual int GetHistory(HistoryList& history) const = 0;
  static void ClearHistory(HistoryList& h);
  virtual const std::string& historyName() const = 0;
  virtual const std::string& historyFile() const = 0;

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
  int LocalTimeGMTOffset() const;
  int LocalTimeOffset() const;
  static int SystemTimeGMTOffset();
  static char SystemTimezone();

  // Ip/Port functions
  unsigned long Ip() const                      { return m_nIp; }
  unsigned long IntIp() const                   { return m_nIntIp; }
  unsigned short Port() const                   { return m_nPort; }
  unsigned short LocalPort() const              { return m_nLocalPort; }
  unsigned long Cookie() const                  { return m_nCookie; }
  void SetIpPort(unsigned long nIp, unsigned short nPort);
  void SetIp(unsigned long nIp)                 { SetIpPort(nIp, Port()); }
  void SetPort(unsigned short nPort)            { SetIpPort(Ip(), nPort); }
  void SetIntIp(unsigned long s)                { m_nIntIp = s; }
  void SetCookie(unsigned long nCookie) { m_nCookie = nCookie; }

  bool SendIntIp() const                        { return m_bSendIntIp; }
  void SetSendIntIp(bool s)                     { m_bSendIntIp = s; }

  // for backward compatibility
  unsigned long RealIp() const                  { return IntIp(); }
  void SetRealIp(unsigned long s) { SetIntIp(s); }
  bool SendRealIp() const                       { return SendIntIp(); }
  void SetSendRealIp(bool s)      { SetSendIntIp(s); }

  std::string ipToString() const;
  std::string internalIpToString() const;
  std::string portToString() const;

  // User TLV List handling
  void AddTLV(TlvPtr);
  void RemoveTLV(unsigned long);
  void SetTLVList(TlvList& tlvs);
  TlvList GetTLVList()                          { return myTLVs; }
  const TlvList GetTLVList() const              { return myTLVs; }

  int socketDesc(int channel) const;
  void clearSocketDesc(int channel);
  void clearAllSocketDesc();
  void setSocketDesc(TCPSocket* s);

  // Convenience functions so plugins don't need to know TCPSocket::ChannelTypes
  int normalSocketDesc() const                  { return myNormalSocketDesc; }
  int infoSocketDesc() const                    { return myInfoSocketDesc; }
  void clearNormalSocketDesc();

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

  virtual bool isUser() const                   { return true; }

protected:
  virtual ~User() { /* Empty */ }

  void SetSecure(bool s) { m_bSecure = s; }
  bool ConnectionInProgress() const { return m_bConnectionInProgress; }
  void SetConnectionInProgress(bool c)  { m_bConnectionInProgress = c; }

  void SetLastSentEvent()           { m_nLastCounters[LAST_SENT_EVENT] = time(NULL); }
  void SetLastReceivedEvent()       { m_nLastCounters[LAST_RECV_EVENT] = time(NULL); }
  void SetLastCheckedAutoResponse() { m_nLastCounters[LAST_CHECKED_AR] = time(NULL); }

  void SetIdleSince(time_t t)       { m_nIdleSince = t; }
  void SetRegisteredTime(time_t t)  { m_nRegisteredTime = t; }

  UserId myId;

  int myNormalSocketDesc;
  int myInfoSocketDesc;
  int myStatusSocketDesc;
  time_t m_nTouched;
  time_t m_nLastCounters[4];
  time_t m_nOnlineSince;
  time_t m_nIdleSince;
  time_t myAwaySince;
  time_t m_nRegisteredTime;
  bool m_bOnContactList;
  unsigned long m_nIp, m_nIntIp, m_nVersion, m_nClientTimestamp, m_nCookie;
  unsigned long m_nClientInfoTimestamp, m_nClientStatusTimestamp;
  unsigned long m_nOurClientTimestamp, m_nOurClientInfoTimestamp;
  unsigned long m_nOurClientStatusTimestamp;
  bool m_bUserUpdated;
  unsigned m_nPort, m_nLocalPort, m_nConnectionVersion;
  bool myIsTyping;
  unsigned myStatus;
  UserGroupList myGroups;               /**< List of user groups */
  unsigned short m_nSequence;
  unsigned myPhoneFollowMeStatus, myIcqPhoneStatus, mySharedFilesStatus;
  bool myWebPresence;
  bool myHideIp;
  bool myBirthdayFlag;
  bool myHomepageFlag;
  unsigned myDirectFlag;
  bool myDirectMode;
  unsigned mySecureChannelSupport;
  std::string myClientInfo;
  std::string myAutoResponse;
  std::string myEncoding;
  bool m_bSupportsUTF8;
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
  unsigned short m_nSendLevel;
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
  char m_nTimezone;
  bool m_bAuthorization;

  // More2 Info
  UserCategoryMap myInterests;
  UserCategoryMap myBackgrounds;
  UserCategoryMap myOrganizations;

  // Phone Book Info
  ICQUserPhoneBook *m_PhoneBook;

  // Picture Info
  std::string myPictureFileName;
  bool m_bPicturePresent;
  unsigned myBuddyIconType;
  unsigned myBuddyIconHashType;
  std::string myBuddyIconHash;
  std::string myOurBuddyIconHash;

  // Dynamic info fields for protocol plugins
  std::map<std::string, std::string> m_mPPFields;

  // Server Side ID, Group SID
  bool m_bAwaitingAuth;
  unsigned m_nSID[3];
  unsigned m_nGSID;

  // Extra TLVs attached to this user's SSI info
  // We use a map to allow fast access to the TLV by type, even though the
  // actual type is in SOscarTLV as well. Which should make it obvious
  // that the TLV handling should be fixed in licq_buffer.h/buffer.cpp
  TlvList myTLVs;

  UserEventList m_vcMessages;

  static unsigned short s_nNumUserEvents;

  static pthread_mutex_t mutex_nNumUserEvents;

  friend class ::CMSN;
  friend class ::IcqProtocol;
  friend class Jabber::Plugin;
  friend void* ::MonitorSockets_func();
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
