#ifndef LICQ_CONTACTLIST_USER_H
#define LICQ_CONTACTLIST_USER_H

#include <boost/any.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <string>
#include <vector>

#include "licq_buffer.h"
#include "licq_file.h"
#include "licq_history.h"
#include "licq_icq.h"
#include "licq_mutex.h"
#include "../types.h"
#include "../userid.h"

class CICQDaemon;
class CMSN;
class CSocketManager;
class TCPSocket;
void* MonitorSockets_tep(void *);

// Cheap hack as I'm too lazy to move the relevant functions to user.cpp
extern "C" void SetString(char **, const char *);

namespace LicqDaemon
{
class UserManager;
}

namespace Licq
{

typedef enum EGender
{
  GENDER_UNSPECIFIED = 0,
  GENDER_FEMALE = 1,
  GENDER_MALE = 2
} Gender;

const unsigned short AGE_UNSPECIFIED = 0xFFFF;
const char TIMEZONE_UNKNOWN = (char)-100;
const unsigned short LICQ_VERSION_UNKNOWN = 0;

typedef enum SecureChannelSupport_et_ {
  SECURE_CHANNEL_UNKNOWN = 0,
  SECURE_CHANNEL_NOTSUPPORTED = 1,
  SECURE_CHANNEL_SUPPORTED = 2
} SecureChannelSupport_et;

const unsigned short NORMAL_SID         = 0;
const unsigned short INV_SID            = 1;
const unsigned short VIS_SID            = 2;

const unsigned short ACCEPT_IN_AWAY     = 0x0001;
const unsigned short ACCEPT_IN_NA       = 0x0002;
const unsigned short ACCEPT_IN_OCCUPIED = 0x0004;
const unsigned short ACCEPT_IN_DND      = 0x0008;
const unsigned short AUTO_ACCEPT_CHAT   = 0x0100;
const unsigned short AUTO_ACCEPT_FILE   = 0x0200;
const unsigned short AUTO_SECURE        = 0x0400;

const unsigned short USPRINTF_NTORN     = 1;
const unsigned short USPRINTF_NOFW      = 2;
const unsigned short USPRINTF_LINEISCMD = 4;
const unsigned short USPRINTF_PIPEISCMD = 8;

const unsigned short LAST_ONLINE        = 0;
const unsigned short LAST_RECV_EVENT    = 1;
const unsigned short LAST_SENT_EVENT    = 2;
const unsigned short LAST_CHECKED_AR    = 3;

const unsigned short MAX_CATEGORY_SIZE  = 60;
const unsigned int MAX_CATEGORIES = 4;

typedef enum
{
  CAT_INTERESTS,
  CAT_ORGANIZATION,
  CAT_BACKGROUND,
  CAT_MAX
} UserCat;

typedef std::vector <class CUserEvent*> UserEventList;
typedef std::map<std::string, boost::any> PropertyMap;

struct PhoneBookEntry
{
  char *szDescription;
  char *szAreaCode;
  char *szPhoneNumber;
  char *szExtension;
  char *szCountry;
  unsigned long nActive;
  unsigned long nType;
  char *szGateway;
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
  TYPE_PHONE,
  TYPE_CELLULAR,
  TYPE_CELLULARxSMS,
  TYPE_FAX,
  TYPE_PAGER,
  TYPE_MAX
};

enum EGatewayType
{
  GATEWAY_BUILTIN = 1,
  GATEWAY_CUSTOM
};

enum EPublish
{
  PUBLISH_ENABLE = 1,
  PUBLISH_DISABLE
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
  bool SaveToDisk(CIniFile& m_fConf);
  bool LoadFromDisk(CIniFile& m_fConf);

  std::vector<struct PhoneBookEntry> PhoneBookVector;

  friend class User;
};


/**
 * A contact in the Licq user list including all information for that user
 *
 * Unless otherwise noted, members should only be accessed while holding a lock
 * to avoid race conditions between threads. User objects are normally
 * retrieved using the user manager function FetchUser() which will lock the
 * user before returning it.
 * Modifying a user object will not trigger any server side changes or generate
 * events to notify plugins, that is the responsibility of the caller. For some
 * members (such as group memberships) user manager functions exist that should
 * be used for this purpose.
 */
class User : public Lockable
{
public:
  /**
   * Build user id from protocol id and account id
   * The user id is unique for each user object.
   *
   * @param accountId User account id string, protocol specific format
   * @param ppid Protocol id for user
   * @return a unique user id for referring to the user
   */
  static UserId makeUserId(const std::string& accountId, unsigned long ppid)
  { return UserId(accountId, ppid); }

  /**
   * Get account id from user a id
   *
   * @param userId User id
   * @return account id for user
   */
  static std::string getUserAccountId(const UserId& userId)
  { return userId.accountId(); }

  /**
   * Get protocol id from user a id
   *
   * @param userId User id
   * @return protocol id for user
   */
  static unsigned long getUserProtocolId(const UserId& userId)
  { return userId.protocolId(); }


  /**
   * Constructor to create a user object for an existing contact
   *
   * @param id User id
   * @param filename Filename to read user data from
   */
  User(const UserId& id, const std::string& filename);

  /**
   * Constructor to create a user object for a new contact
   *
   * @param id User id
   * @param temporary False if user is added permanently to list
   */
  User(const UserId& id, bool temporary = false);

  virtual ~User();
  void RemoveFiles();

  void saveAll();
  virtual void SaveLicqInfo();
  void saveUserInfo();
  void SavePhoneBookInfo();
  void SavePictureInfo();
  void SaveNewMessagesInfo();

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
  unsigned long ppid() const                    { return myId.protocolId(); }

  /**
   * Get normalized account id that can be used when comparing ids
   *
   * @return normalized account id
   */
  const std::string& realAccountId() const      { return accountId(); }

  // Old deprecated functions to get account id and protocol id, do not use in new code
  const char* IdString() const { return accountId().c_str(); }
  unsigned long PPID() const { return ppid(); }

  // General Info
  //!Retrieves the user's alias.
  //LICQ_DEPRECATED // Use getAlias() instead
  const char* GetAlias() const                  { return myAlias.c_str(); }
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
  unsigned short BuddyIconType() const          { return m_nBuddyIconType; }
  char BuddyIconHashType() const                { return m_nBuddyIconHashType; }
  const char* BuddyIconHash() const             { return m_szBuddyIconHash; }
  const char* OurBuddyIconHash() const          { return m_szOurBuddyIconHash; }

  // Dynamic info fields for protocol plugins
  std::string GetPPField(const std::string &);

  // Licq Info
  bool GetAwaitingAuth() const                  { return m_bAwaitingAuth; }
  unsigned short GetSID() const                 { return m_nSID[NORMAL_SID]; }
  unsigned short GetInvisibleSID() const        { return m_nSID[INV_SID]; }
  unsigned short GetVisibleSID() const          { return m_nSID[VIS_SID]; }
  unsigned short GetGSID() const                { return m_nGSID; }
  unsigned short GetTyping() const              { return m_nTyping; }
  //!Retrieves the user's auto response message that was last seen.
  const char* AutoResponse() const              { return m_szAutoResponse; }
  //!Retrieves the encoding Licq uses for this user
  const char* UserEncoding() const;
  //!True if they have sent the UTF8 Cap
  bool SupportsUTF8() const                     { return m_bSupportsUTF8; }
  bool SendServer() const                       { return m_bSendServer; }
  unsigned short SendLevel() const              { return m_nSendLevel; }
  bool EnableSave() const                       { return m_bEnableSave; }
  bool ShowAwayMsg() const                      { return m_bShowAwayMsg; }
  unsigned short Sequence(bool = false);
  char Mode() const                             { return m_nMode; }
  unsigned long Version() const                 { return m_nVersion; }
  const char* ClientInfo() const                { return m_szClientInfo; }
  unsigned long ClientTimestamp() const         { return m_nClientTimestamp; }
  unsigned long OurClientTimestamp() const      { return m_nOurClientTimestamp; }
  unsigned long ClientInfoTimestamp() const     { return m_nClientInfoTimestamp; }
  unsigned long OurClientInfoTimestamp() const  { return m_nOurClientInfoTimestamp; }
  unsigned long ClientStatusTimestamp() const   { return m_nClientStatusTimestamp; }
  unsigned long OurClientStatusTimestamp() const { return m_nOurClientStatusTimestamp; }
  bool UserUpdated() const                      { return m_bUserUpdated; }
  SecureChannelSupport_et SecureChannelSupport() const;
  unsigned short LicqVersion() const;
  unsigned short ConnectionVersion() const;
  time_t LastOnline() const                     { return m_nLastCounters[LAST_ONLINE]; }
  time_t LastSentEvent() const                  { return m_nLastCounters[LAST_SENT_EVENT]; }
  time_t LastReceivedEvent() const              { return m_nLastCounters[LAST_RECV_EVENT]; }
  time_t LastCheckedAutoResponse() const        { return m_nLastCounters[LAST_CHECKED_AR]; }
  time_t OnlineSince() const                    { return m_nOnlineSince; }
  time_t IdleSince() const                      { return m_nIdleSince; }
  time_t RegisteredTime() const                 { return m_nRegisteredTime; }
  bool UseGPG() const                           { return m_bUseGPG; }
  const char* GPGKey() const                    { return m_szGPGKey; }
  bool AutoChatAccept() const                   { return m_nAutoAccept & AUTO_ACCEPT_CHAT; }
  bool AutoFileAccept() const                   { return m_nAutoAccept & AUTO_ACCEPT_FILE; }
  bool AutoSecure() const                       { return m_nAutoAccept & AUTO_SECURE; }
  bool AcceptInAway() const                     { return m_nAutoAccept & ACCEPT_IN_AWAY; }
  bool AcceptInNA() const                       { return m_nAutoAccept & ACCEPT_IN_NA; }
  bool AcceptInOccupied() const                 { return m_nAutoAccept & ACCEPT_IN_OCCUPIED; }
  bool AcceptInDND() const                      { return m_nAutoAccept & ACCEPT_IN_DND; }
  unsigned short StatusToUser() const           { return m_nStatusToUser; }
  unsigned statusToUser() const                 { return statusFromIcqStatus(m_nStatusToUser); }
  bool KeepAliasOnUpdate() const                { return m_bKeepAliasOnUpdate; }
  char *CustomAutoResponse() const              { return m_szCustomAutoResponse; }
  bool NotInList() const                        { return m_bNotInList; }

  char* usprintf(const char* szFormat, unsigned long nFlags = 0) const;

  // General Info
  void setAlias(const std::string& alias);
  void SetTimezone (const char n)            {  m_nTimezone = n; saveUserInfo();  }
  void SetAuthorization (bool n)             {  m_bAuthorization = n; saveUserInfo();  }

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

  // Picture info
  void SetPicturePresent(bool b)      { m_bPicturePresent = b; SavePictureInfo(); }
  void SetBuddyIconType(unsigned short s) { m_nBuddyIconType = s; }
  void SetBuddyIconHashType(char s)   { m_nBuddyIconHashType = s; }
  void SetBuddyIconHash(char *s)      { SetString(&m_szBuddyIconHash, s); }
  void SetOurBuddyIconHash(char *s)   { SetString(&m_szOurBuddyIconHash, s); }

  // Licq Info
  void SetAwaitingAuth(bool b)        { m_bAwaitingAuth = b; }
  void SetSID(unsigned short s)       { m_nSID[NORMAL_SID] = s; }
  void SetInvisibleSID(unsigned short s) { m_nSID[INV_SID] = s; }
  void SetVisibleSID(unsigned short s){ m_nSID[VIS_SID] = s; }
  void SetGSID(unsigned short s)      { m_nGSID = s; }
  void SetEnableSave(bool s)          { if (m_bOnContactList) m_bEnableSave = s; }
  void SetSendServer(bool s)          { m_bSendServer = s; SaveLicqInfo(); }
  void SetSendLevel(unsigned short s) { m_nSendLevel = s; }
  void SetSequence(unsigned short s)  { m_nSequence = s; }
  void SetAutoResponse(const char *s) { SetString(&m_szAutoResponse, s); }
  void SetUserEncoding(const char* s) { SetString(&m_szEncoding, s); }
  void SetSupportsUTF8(bool b)        { m_bSupportsUTF8 = b; }
  void SetShowAwayMsg(bool s)         { m_bShowAwayMsg = s; }
  void SetMode(char s)                { m_nMode = s; }
  void SetVersion(unsigned long s)    { m_nVersion = s; }
  void SetClientInfo(char *s)         { SetString(&m_szClientInfo, s); }
  void SetClientTimestamp(unsigned long s) { m_nClientTimestamp = s; }
  void SetOurClientTimestamp(unsigned long s) { m_nOurClientTimestamp = s; }
  void SetClientInfoTimestamp(unsigned long s) { m_nClientInfoTimestamp = s; }
  void SetOurClientInfoTimestamp(unsigned long s) { m_nOurClientInfoTimestamp = s; }
  void SetClientStatusTimestamp(unsigned long s) { m_nClientStatusTimestamp = s; }
  void SetOurClientStatusTimestamp(unsigned long s) { m_nOurClientStatusTimestamp = s; }
  void SetUserUpdated(bool s)         { m_bUserUpdated = s; }
  void SetConnectionVersion(unsigned short s)    { m_nConnectionVersion = s; }
  void SetAutoChatAccept(bool s)      { s ? m_nAutoAccept |= AUTO_ACCEPT_CHAT : m_nAutoAccept &= ~AUTO_ACCEPT_CHAT; SaveLicqInfo(); }
  void SetAutoFileAccept(bool s)      { s ? m_nAutoAccept |= AUTO_ACCEPT_FILE : m_nAutoAccept &= ~AUTO_ACCEPT_FILE; SaveLicqInfo(); }
  void SetAutoSecure(bool s)          { s ? m_nAutoAccept |= AUTO_SECURE : m_nAutoAccept &= ~AUTO_SECURE; SaveLicqInfo(); }
  void SetAcceptInAway(bool s)        { s ? m_nAutoAccept |= ACCEPT_IN_AWAY : m_nAutoAccept &= ~ACCEPT_IN_AWAY; SaveLicqInfo(); }
  void SetAcceptInNA(bool s)          { s ? m_nAutoAccept |= ACCEPT_IN_NA : m_nAutoAccept &= ~ACCEPT_IN_NA; SaveLicqInfo(); }
  void SetAcceptInOccupied(bool s)    { s ? m_nAutoAccept |= ACCEPT_IN_OCCUPIED : m_nAutoAccept &= ~ACCEPT_IN_OCCUPIED; SaveLicqInfo(); }
  void SetAcceptInDND(bool s)         { s ? m_nAutoAccept |= ACCEPT_IN_DND : m_nAutoAccept &= ~ACCEPT_IN_DND; SaveLicqInfo(); }
  void SetUseGPG(bool b)                        { m_bUseGPG = b; SaveLicqInfo(); }
  void SetGPGKey(const char *c)                 { SetString(&m_szGPGKey, c); SaveLicqInfo(); }
  void SetStatusToUser(unsigned short s)    { m_nStatusToUser = s; SaveLicqInfo(); }
  void setStatusToUser(unsigned s) { SetStatusToUser(icqStatusFromStatus(s)); }
  void SetKeepAliasOnUpdate(bool b)   { m_bKeepAliasOnUpdate = b; }
  void SetCustomAutoResponse(const char *s) { SetString(&m_szCustomAutoResponse, s); SaveLicqInfo(); }
  void ClearCustomAutoResponse()            { SetCustomAutoResponse(""); }
  void SetTyping(unsigned short nTyping)    { m_nTyping = nTyping; }
  void SetClientInfo(const char *s)
  {
    free(m_szClientInfo);
    m_szClientInfo = (s ? strdup(s) : NULL);
  }
  void SetPermanent();

  // Dynamic info fields for protocol plugins
  bool SetPPField(const std::string &, const std::string &);


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
   * Set status
   * Note: This should only be called from protocol plugin owning the user
   *
   * @param status New status for user
   */
  void setStatus(unsigned status);

  /**
   * Convenience function to check if if user is online
   *
   * @return True if user is online
   */
  bool isOnline() const
  { return (myStatus & OnlineStatus) != 0; }

  /**
   * Convenience function to check if user is invisible
   *
   * @return True if user is online and invisible
   */
  bool isInvisible() const
  { return (myStatus & InvisibleStatus) != 0; }

  // Functions for use during transition from ICQ status
  static unsigned short icqStatusFromStatus(unsigned status);
  static unsigned statusFromIcqStatus(unsigned short icqStatus);

  /// Get switch-able version of ICQ status
  unsigned short Status() const;
  /// Get ICQ status flags (mask of ICQ_STATUS_xxx flags)
  unsigned long StatusFull() const              { return m_nStatus; }
  bool StatusWebPresence() const                { return m_nStatus & ICQ_STATUS_FxWEBxPRESENCE; }
  bool StatusHideIp() const                     { return m_nStatus & ICQ_STATUS_FxHIDExIP; }
  bool StatusBirthday() const                   { return m_nStatus & ICQ_STATUS_FxBIRTHDAY; }
  unsigned long PhoneFollowMeStatus() const     { return m_nPhoneFollowMeStatus; }
  unsigned long ICQphoneStatus() const          { return m_nICQphoneStatus; }
  unsigned long SharedFilesStatus() const       { return m_nSharedFilesStatus; }
  /// Set ICQ status flags (also updates generic status flags)
  void SetStatus(unsigned long n);
  void SetPhoneFollowMeStatus(unsigned long n)  { m_nPhoneFollowMeStatus = n; SaveLicqInfo(); }
  void SetICQphoneStatus(unsigned long n)       { m_nICQphoneStatus = n; }
  void SetSharedFilesStatus(unsigned long n)    { m_nSharedFilesStatus = n; }  
  virtual void SetStatusOffline();
  const char* StatusStr() const;
  const char* StatusStrShort() const;
  bool Away() const;
  static const char* StatusToStatusStr(unsigned short n, bool b);
  static const char* StatusToStatusStrShort(unsigned short n, bool b);

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
  const CUserEvent* EventPeek(unsigned short index) const;
  const CUserEvent* EventPeekId(int id) const;
  const CUserEvent* EventPeekFirst() const;
  const CUserEvent* EventPeekLast() const;
  CUserEvent *EventPop();
  void EventClear(unsigned short);
  void EventClearId(int);
  void EventPush(CUserEvent *);
  void WriteToHistory(const char *);
  void SetHistoryFile(const char *);
  int GetHistory(HistoryList& history) const    { return m_fHistory.Load(history); }
  static void ClearHistory(HistoryList &h)  { CUserHistory::Clear(h); }
  void SaveHistory(const char *buf)  { m_fHistory.Save(buf); }
  const char* HistoryName() const               { return m_fHistory.Description(); }
  const char* HistoryFile() const               { return m_fHistory.FileName(); }

  /**
   * Get user groups this user is member of
   *
   * @return List of groups
   */
  const UserGroupList& GetGroups() const { return myGroups; }

  /**
   * Get system groups this user is member of
   *
   * @return Bitmask of server groups
   */
  unsigned long GetSystemGroups() const { return mySystemGroups; }

  /**
   * Set user groups this user is member of
   *
   * @param groups List of groups
   */
  void SetGroups(const UserGroupList& groups) { myGroups = groups; }

  /**
   * Set system groups this user is member of
   *
   * @param groups Bitmask of server groups
   */
  void SetSystemGroups(unsigned long groups)    { mySystemGroups = groups; }

  /**
   * Check if user is member of a group
   *
   * @param gtype Group type (GROUPS_SYSTEM or GROUPS_USER)
   * @param groupId Id of group to check
   * @return True if group exists and user is member
   */
  bool GetInGroup(GroupType gtype, int groupId) const;

  /**
   * Convenience function to set membership of user for a group
   *
   * @param gtype Group type (GROUPS_SYSTEM or GROUPS_USER)
   * @param groupId Id of group
   * @param member True to add user to group, false to remove user from group
   */
  void SetInGroup(GroupType gtype, int groupId, bool member);

  /**
   * Add user to a group
   *
   * @param gtype Group type (GROUPS_SYSTEM or GROUPS_USER)
   * @param groupId Id of group to add
   */
  void AddToGroup(GroupType gtype, int groupId);

  /**
   * Remove user from a group
   *
   * @param gtype Group type (GROUPS_SYSTEM or GROUPS_USER)
   * @param groupId Id of group to leave
   * @return True if group was valid and user was a member
   */
  bool RemoveFromGroup(GroupType gtype, int groupId);

  // Short cuts to above functions
  bool InvisibleList() const    { return GetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST); }
  bool VisibleList() const      { return GetInGroup(GROUPS_SYSTEM, GROUP_VISIBLE_LIST); }
  bool OnlineNotify() const     { return GetInGroup(GROUPS_SYSTEM, GROUP_ONLINE_NOTIFY); }
  bool IgnoreList() const       { return GetInGroup(GROUPS_SYSTEM, GROUP_IGNORE_LIST); }
  bool NewUser() const          { return GetInGroup(GROUPS_SYSTEM, GROUP_NEW_USERS); }
  void SetInvisibleList(bool s)  { SetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST, s); }
  void SetVisibleList(bool s)    { SetInGroup(GROUPS_SYSTEM, GROUP_VISIBLE_LIST, s); }
  void SetOnlineNotify(bool s)   { SetInGroup(GROUPS_SYSTEM, GROUP_ONLINE_NOTIFY, s); }
  void SetIgnoreList(bool s)     { SetInGroup(GROUPS_SYSTEM, GROUP_IGNORE_LIST, s); }
  void SetNewUser(bool s)        { SetInGroup(GROUPS_SYSTEM, GROUP_NEW_USERS, s); }

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

  char* IpStr(char* rbuf) const;
  char* IntIpStr(char* rbuf) const;
  char* PortStr(char* rbuf) const;

  // User TLV List handling
  void AddTLV(TLVPtr);
  void RemoveTLV(unsigned long);
  void SetTLVList(TLVList& tlvs);
  TLVList GetTLVList()                          { return myTLVs; }
  const TLVList GetTLVList() const              { return myTLVs; }


  // Don't call these:
  int SocketDesc(unsigned char channel) const;
  void ClearSocketDesc(unsigned char nChannel = 0x00);
  void SetSocketDesc(TCPSocket *);

  // Events functions
  static unsigned short getNumUserEvents();
  static void incNumUserEvents();
  static void decNumUserEvents();

  // Last event functions
  time_t Touched() const                        { return m_nTouched; }
  void Touch()           {  m_nTouched = time(NULL); }

  // Crypto
  bool Secure() const                           { return m_bSecure; }

  virtual bool isUser() const                   { return true; }

protected:
  void loadUserInfo();

  /**
   * Save a category list
   *
   * @param category The category map to save
   * @param file User file, must already be open
   * @param key Base name of key in file for entries
   */
  void saveCategory(const UserCategoryMap& category, CIniFile& file,
      const std::string& key);

  /**
   * Load a category list
   *
   * @param category The category map to save
   * @param file User file, must already be open
   * @param key Base name of key in file for entries
   */
  void loadCategory(UserCategoryMap& category, CIniFile& file,
      const std::string& key);

  void LoadPhoneBookInfo();
  void LoadPictureInfo();
  void LoadLicqInfo();

  /**
   * Initialize all user object. Contains common code for all constructors
   */
  void Init();
  bool LoadInfo();
  void SetDefaults();
  void AddToContactList();

  void SetSecure(bool s) { m_bSecure = s; }
  void SetOfflineOnDisconnect(bool b) { m_bOfflineOnDisconnect = b; }
  bool OfflineOnDisconnect() { return m_bOfflineOnDisconnect; }
  bool ConnectionInProgress() { return m_bConnectionInProgress; }
  void SetConnectionInProgress(bool c)  { m_bConnectionInProgress = c; }

  void SetLastSentEvent()           { m_nLastCounters[LAST_SENT_EVENT] = time(NULL); }
  void SetLastReceivedEvent()       { m_nLastCounters[LAST_RECV_EVENT] = time(NULL); }
  void SetLastCheckedAutoResponse() { m_nLastCounters[LAST_CHECKED_AR] = time(NULL); }

  void SetOnlineSince(time_t t)     { m_nOnlineSince = t; }
  void SetIdleSince(time_t t)       { m_nIdleSince = t; }
  void SetRegisteredTime(time_t t)  { m_nRegisteredTime = t; }

  const UserId myId;

  CIniFile m_fConf;
  CUserHistory m_fHistory;
  int m_nNormalSocketDesc, m_nInfoSocketDesc, m_nStatusSocketDesc;
  time_t m_nTouched;
  time_t m_nLastCounters[4];
  time_t m_nOnlineSince;
  time_t m_nIdleSince;
  time_t m_nRegisteredTime;
  bool m_bOnContactList;
  unsigned long m_nIp, m_nIntIp, m_nVersion, m_nClientTimestamp, m_nCookie;
  unsigned long m_nClientInfoTimestamp, m_nClientStatusTimestamp;
  unsigned long m_nOurClientTimestamp, m_nOurClientInfoTimestamp;
  unsigned long m_nOurClientStatusTimestamp;
  bool m_bUserUpdated;
  unsigned short m_nPort, m_nLocalPort, m_nConnectionVersion;
  unsigned short m_nTyping;
  unsigned long m_nStatus;
  unsigned myStatus;
  UserGroupList myGroups;               /**< List of user groups */
  unsigned long mySystemGroups;         /**< Bitmask for system groups */
  unsigned short m_nSequence;
  unsigned long m_nPhoneFollowMeStatus, m_nICQphoneStatus, m_nSharedFilesStatus;
  char m_nMode;
  char *m_szClientInfo;
  char *m_szAutoResponse;
  char *m_szEncoding;
  bool m_bSupportsUTF8;
  char *m_szCustomAutoResponse;
  bool m_bOnlineNotify,
       m_bSendIntIp,
       m_bSendServer,
       m_bEnableSave,
       m_bShowAwayMsg,
       m_bOfflineOnDisconnect,
       m_bConnectionInProgress,
       m_bSecure,
       m_bNotInList;
  unsigned short m_nStatusToUser, m_nSendLevel;
  bool m_bKeepAliasOnUpdate;
  unsigned short m_nAutoAccept;
  bool myOnEventsBlocked;

  // GPG data
  bool m_bUseGPG;
  char *m_szGPGKey;

  // General Info
  std::string myAlias;
  char m_nTimezone;
  bool m_bAuthorization;

  // myUserInfo holds user information like email, address, homepage etc...
  PropertyMap myUserInfo;

  // More2 Info
  UserCategoryMap myInterests;
  UserCategoryMap myBackgrounds;
  UserCategoryMap myOrganizations;

  // Phone Book Info
  ICQUserPhoneBook *m_PhoneBook;

  // Picture Info
  bool m_bPicturePresent;
  unsigned short m_nBuddyIconType;
  char m_nBuddyIconHashType;
  char *m_szBuddyIconHash, *m_szOurBuddyIconHash;

  // Dynamic info fields for protocol plugins
  std::map<std::string, std::string> m_mPPFields;

  // Server Side ID, Group SID
  bool m_bAwaitingAuth;
  unsigned short m_nSID[3];
  unsigned short m_nGSID;

  // Extra TLVs attached to this user's SSI info
  // We use a map to allow fast access to the TLV by type, even though the
  // actual type is in SOscarTLV as well. Which should make it obvious
  // that the TLV handling should be fixed in licq_buffer.h/buffer.cpp
  TLVList myTLVs;

  UserEventList m_vcMessages;

  static unsigned short s_nNumUserEvents;

  static pthread_mutex_t mutex_nNumUserEvents;

  friend class LicqDaemon::UserManager;
  friend class ::CICQDaemon;
  friend class ::CMSN;
  friend class ::CSocketManager;
  friend void* ::MonitorSockets_tep(void *);
};


} // namespace Licq

#endif // LICQ_CONTACTLIST_USER_H
