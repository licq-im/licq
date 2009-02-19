#ifndef USER_H
#define USER_H

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <boost/any.hpp>

#include "pthread_rdwr.h"

#include "licq_buffer.h"
#include "licq_history.h"
#include "licq_file.h"
#include "licq_icq.h"

// Added for plugin convenience
#include "licq_constants.h"

// Define for marking functions as deprecated
#ifndef LICQ_DEPRECATED
# if defined(__GNUC__) && !defined(__INTEL_COMPILER) && (__GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2))
#  define LICQ_DEPRECATED __attribute__ ((__deprecated__))
# elif defined(_MSC_VER) && (_MSC_VER >= 1300)
#  define LICQ_DEPRECATED __declspec(deprecated)
# else
#  define LICQ_DEPRECATED
# endif
#endif

class TCPSocket;

#define LICQ_PPID 0x4C696371  // "Licq"
// Returned value must be cleared with delete[]
extern char* PPIDSTRING(unsigned long ppid);

/*---------------------------------------------------------------------------
 * FOR_EACH_USER
 *
 * Macros to iterate through the entire list of users.  "pUser" will be a
 * pointer to the current user.
 *-------------------------------------------------------------------------*/
#define FOR_EACH_USER_START(x)                           \
  {                                                      \
    LicqUser* pUser;                                     \
    const UserMap* _ul_ = gUserManager.LockUserList(LOCK_R); \
    for (UserMap::const_iterator _i_ = _ul_->begin();    \
         _i_ != _ul_->end(); _i_++)                      \
    {                                                    \
      pUser = _i_->second;                               \
      pUser->Lock(x);                                    \
      {

#define FOR_EACH_PROTO_USER_START(x, y)                  \
  {                                                      \
    LicqUser* pUser;                                     \
    const UserAccountMap* _ul_ = gUserManager.lockUserAccounts(LOCK_R); \
    for (UserAccountMap::const_iterator _i_ = _ul_->begin(); \
         _i_ != _ul_->end(); _i_++)                      \
    {                                                    \
      if (_i_->first.second != x)                        \
        continue;                                        \
      pUser = _i_->second;                               \
      pUser->Lock(y);                                    \
      {

#define FOR_EACH_OWNER_START(x)                           \
  {                                                       \
    OwnerMap* _ol_ = gUserManager.LockOwnerList(LOCK_R);  \
    for (OwnerMap::const_iterator _i_ = _ol_->begin();    \
         _i_ != _ol_->end(); _i_++)                       \
    {                                                     \
      LicqOwner* pOwner = _i_->second;                    \
      pOwner->Lock(x);                                    \
      {

#define FOR_EACH_OWNER_END                                \
      }                                                   \
      pOwner->Unlock();                                   \
    }                                                     \
    gUserManager.UnlockOwnerList();                       \
  }                                                       \

#define FOR_EACH_OWNER_BREAK                              \
        {                                                 \
          gUserManager.DropOwner(pOwner->PPID());         \
          break;                                          \
        }

#define FOR_EACH_USER_END                \
      }                                  \
      pUser->Unlock();                   \
    }                                    \
    gUserManager.UnlockUserList();       \
  }

#define FOR_EACH_PROTO_USER_END FOR_EACH_USER_END

#define FOR_EACH_USER_BREAK              \
        {                                \
          gUserManager.DropUser(pUser);  \
          break;                         \
        }

#define FOR_EACH_PROTO_USER_BREAK        \
        {                                \
          gUserManager.DropUser(pUser);  \
          break;                         \
        }

#define FOR_EACH_PROTO_USER_CONTINUE     \
        {                                \
          gUserManager.DropUser(pUser);  \
          continue;                      \
        }

#define FOR_EACH_USER_CONTINUE           \
        {                                \
          gUserManager.DropUser(pUser);  \
          continue;                      \
        }

#define FOR_EACH_GROUP_START(x)                          \
  {                                                      \
    LicqGroup* pGroup;                                   \
    GroupMap* _gl_ = gUserManager.LockGroupList(LOCK_R); \
    for (GroupMap::iterator _i_ = _gl_->begin();         \
         _i_ != _gl_->end(); ++_i_)                      \
    {                                                    \
      pGroup = _i_->second;                              \
      pGroup->Lock(x);                                   \
      {

#define FOR_EACH_GROUP_START_SORTED(x)                                  \
  {                                                                     \
    LicqGroup* pGroup;                                                  \
    std::list<LicqGroup*> _sortedGroups_;                               \
    FOR_EACH_GROUP_START(LOCK_R)                                        \
      _sortedGroups_.push_back(pGroup);                                 \
      }                                                                 \
      pGroup->Unlock();                                                 \
     }                                                                  \
    }                                                                   \
    _sortedGroups_.sort(compare_groups);                                \
    for (std::list<LicqGroup*>::iterator _i_ = _sortedGroups_.begin();  \
        _i_ != _sortedGroups_.end(); ++_i_)                             \
    {                                                                   \
      pGroup = *_i_;                                                    \
      pGroup->Lock(x);                                                  \
      {

#define FOR_EACH_GROUP_CONTINUE          \
        {                                \
          pGroup->Unlock();              \
          continue;                      \
        }

#define FOR_EACH_GROUP_BREAK             \
        {                                \
          pGroup->Unlock();              \
          break;                         \
        }

#define FOR_EACH_GROUP_END               \
      }                                  \
      pGroup->Unlock();                  \
    }                                    \
    gUserManager.UnlockGroupList();      \
  }




#define FOR_EACH_PROTO_ID_START(x)                       \
  {                                                      \
    char *szId;                                          \
    UserMap* _ul_ = gUserManager.LockUserList(LOCK_R);   \
    for (UserMap::const_iterator _i_ = _ul_->begin();    \
         _i_ != _ul_->end(); _i_++)                      \
    {                                                    \
      if (_i_->first.second != x)                        \
        continue;                                        \
      szId = (*_i_)->IdString();                         \
      {

#define FOR_EACH_PROTO_ID_BREAK          \
        {                                \
          break;                         \
        }

#define FOR_EACH_PROTO_ID_CONTINUE       \
        {                                \
          continue;                      \
        }

class LicqUser;
class LicqOwner;
class LicqGroup;

typedef std::pair<std::string, unsigned long> UserAccountMapKey;
typedef std::map<UserAccountMapKey, class LicqUser*> UserAccountMap;
typedef std::map<int, class LicqUser*> UserMap;
typedef std::map<unsigned long, class LicqOwner*> OwnerMap;
typedef std::set<int> UserGroupList;
typedef std::map<int, LicqGroup*> GroupMap;
typedef std::map<int, std::string> GroupNameMap;
typedef std::vector <class CUserEvent *> UserEventList;
typedef std::map<std::string, boost::any> PropertyMap;
typedef std::map<unsigned int, std::string> UserCategoryMap;

// Cheap hack as I'm too lazy to move the relevant functions to user.cpp
extern "C" void SetString(char **, const char *);


//+++++STRUCTURES+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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

enum GroupType { GROUPS_SYSTEM, GROUPS_USER };

const int GROUP_ALL_USERS       = 0;
const int GROUP_ONLINE_NOTIFY   = 1;
const int GROUP_VISIBLE_LIST    = 2;
const int GROUP_INVISIBLE_LIST  = 3;
const int GROUP_IGNORE_LIST     = 4;
const int GROUP_NEW_USERS       = 5;

/**
 * The amount of registered system groups
 */
const int NUM_GROUPS_SYSTEM_ALL = 6;

/**
 * The amount of registered system groups, excluding the 'All Users' group.
 *
 * @deprecated Scheduled for removal, use NUM_GROUPS_SYSTEM_ALL instead.
 */
const int NUM_GROUPS_SYSTEM     = NUM_GROUPS_SYSTEM_ALL - 1;

extern const char *GroupsSystemNames[NUM_GROUPS_SYSTEM_ALL];

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

//+++++OBJECTS++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

//====ICQUserPhoneBook=========================================================
class ICQUserPhoneBook
{
public:
  ICQUserPhoneBook();
  ~ICQUserPhoneBook();
  void AddEntry(const struct PhoneBookEntry *entry);
  void SetEntry(const struct PhoneBookEntry *entry, unsigned long nEntry);
  void ClearEntry(unsigned long nEntry);
  void Clean();
  void SetActive(long nEntry);

  bool Get(unsigned long nEntry, const struct PhoneBookEntry** entry) const;

private:
  bool SaveToDisk  (CIniFile &m_fConf);
  bool LoadFromDisk(CIniFile &m_fConf);

  std::vector<struct PhoneBookEntry> PhoneBookVector;

  friend class LicqUser;
};


// Temporary until all occurenses of deprecated names ICQUser ICQOwner have been removed
typedef LicqUser ICQUser;
typedef LicqOwner ICQOwner;


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
class LicqUser
{
public:
  /**
   * Constructor to create a user object for an existing contact
   *
   * @param id User id, must be unique
   * @param accountId User account id string, protocol specific format
   * @param ppid Protocol id for user
   * @param filename Filename to read user data from
   */
  LicqUser(int id, const std::string& accountId, unsigned long ppid, const std::string& filename);

  /**
   * Constructor to create a user object for a new contact
   *
   * @param id User id, must be unique if user is in list. Use 0 for temporary objects
   * @param accountId User account id string, protocol specific format
   * @param ppid Protocol id for user
   * @param temporary False if user is added permanently to list
   */
  LicqUser(int id, const std::string& accountId, unsigned long ppid, bool temporary = false);

  /**
   * Constructor to create a temporary user object that will not be in user list
   * User id will be set to zero and user will be marked as not in list
   *
   * @param accountId User account id string, protocol specific format
   * @param ppid Protocol id for user
   */
  LicqUser(const std::string& accountId, unsigned long ppid);

  virtual ~LicqUser();
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
  int id() const                                { return myId; }

  /**
   * Get account id that server protocol uses to identify user
   * The format and usage of this string may vary between protocols
   *
   * @return account id
   */
  const std::string& accountId() const          { return myAccountId; }

  /**
   * Get protocol instance this user belongs to
   *
   * @return protocol instance id
   */
  unsigned long ppid() const                    { return myPpid; }

  /**
   * Get normalized account id that can be used when comparing ids
   *
   * @return normalized account id
   */
  const std::string& realAccountId() const      { return myRealAccountId; }

  // Old deprecated functions to get account id and protocol id, do not use in new code
  const char* IdString() const { return accountId().c_str(); }
  unsigned long PPID() const { return ppid(); }

  // General Info
  //!Retrieves the user's alias.
  const char* GetAlias() const                  { return m_szAlias; }
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
  bool KeepAliasOnUpdate() const                { return m_bKeepAliasOnUpdate; }
  char *CustomAutoResponse() const              { return m_szCustomAutoResponse; }
  bool NotInList() const                        { return m_bNotInList; }

  char* usprintf(const char* szFormat, unsigned long nFlags = 0) const;

  // General Info
  void SetAlias (const char *n);// {  SetString(&m_szAlias, n);  SaveGeneralInfo();  }
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

  // Status
  unsigned short Status() const;
  unsigned long StatusFull() const              { return m_nStatus; }
  bool StatusInvisible() const                  { return StatusOffline() ? false : m_nStatus & ICQ_STATUS_FxPRIVATE; }
  bool StatusWebPresence() const                { return m_nStatus & ICQ_STATUS_FxWEBxPRESENCE; }
  bool StatusHideIp() const                     { return m_nStatus & ICQ_STATUS_FxHIDExIP; }
  bool StatusBirthday() const                   { return m_nStatus & ICQ_STATUS_FxBIRTHDAY; }
  bool StatusOffline() const                    { return (unsigned short)m_nStatus == ICQ_STATUS_OFFLINE; }
  unsigned long PhoneFollowMeStatus() const     { return m_nPhoneFollowMeStatus; }
  unsigned long ICQphoneStatus() const          { return m_nICQphoneStatus; }
  unsigned long SharedFilesStatus() const       { return m_nSharedFilesStatus; }
  void SetStatus(unsigned long n)  {  m_nStatus = n;  }
  void SetPhoneFollowMeStatus(unsigned long n)  { m_nPhoneFollowMeStatus = n; SaveLicqInfo(); }
  void SetICQphoneStatus(unsigned long n)       { m_nICQphoneStatus = n; }
  void SetSharedFilesStatus(unsigned long n)    { m_nSharedFilesStatus = n; }  
  virtual void SetStatusOffline();
  const char* StatusStr() const;
  const char* StatusStrShort() const;
  bool Away() const;
  static const char* StatusToStatusStr(unsigned short n, bool b);
  static const char* StatusToStatusStrShort(unsigned short n, bool b);
  LICQ_DEPRECATED static char* MakeRealId(const std::string& accountId, unsigned long ppid, char *&);

  /**
   * Normalize an account id
   *
   * @param accountId Account id
   * @param ppid Protocol instance id
   * @return Normalized account id
   */
  static std::string normalizeId(const std::string& accountId, unsigned long ppid);

  /**
   * Convenience function get a normalized key for the account map
   *
   * @param accountId Account id
   * @param ppid Protocol instance id
   * @return A account map key with the account id normalized
   */
  static UserAccountMapKey normalizeIdMapKey(const std::string& accountId, unsigned long ppid)
  { return UserAccountMapKey(normalizeId(accountId, ppid), ppid); }

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
   * @pram groupId Id of group to leave
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

  virtual bool User() const                     { return true; }
  void Lock(unsigned short lockType = LOCK_R) const;
  void Unlock() const;

protected:
  LicqUser(int id) : myId(id) { /* LicqOwner inherited constructor - does nothing */ }
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

  const int myId;
  std::string myAccountId;
  unsigned long myPpid;
  std::string myRealAccountId;

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
  char *m_szAlias;
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

  mutable pthread_rdwr_t myMutex;
  mutable unsigned short myLockType;
  static pthread_mutex_t mutex_nNumUserEvents;

  friend class CUserManager;
  friend class CICQDaemon;
  friend class CMSN;
  friend class CSocketManager;
  friend void *MonitorSockets_tep(void *);
};


/**
 * A protocol account including all user information for that account
 *
 * Inherits LicqUser to hold all user information associated with the account.
 */
class LicqOwner : public LicqUser
{
public:
  /**
   * Constructor
   *
   * @param accountId User account id
   * @param ppid Protocol instance id
   */
  LicqOwner(const std::string& accountId, unsigned long ppid);

  virtual ~LicqOwner();
  bool Exception() const                        { return m_bException; }

  // Owner specific functions
  const char* Password() const                  { return(m_szPassword); }
  void SetPassword(const char *s) { SetString(&m_szPassword, s); SaveLicqInfo(); }
  void SetWebAware(bool b)     {  m_bWebAware = b; SaveLicqInfo(); }
  virtual void SetWebAwareStatus(char c) { SetWebAware(c); }
  void SetHideIp(bool b)       {  m_bHideIp = b; SaveLicqInfo(); }
  void SetSavePassword(bool b) {  m_bSavePassword = b; SaveLicqInfo(); }
  void SetRandomChatGroup(unsigned long n)  { m_nRandomChatGroup = n; SaveLicqInfo(); }
  bool WebAware() const                         { return m_bWebAware; }
  bool HideIp() const                           { return m_bHideIp; }
  bool SavePassword() const                     { return m_bSavePassword; }
  unsigned long RandomChatGroup() const         { return m_nRandomChatGroup; }
  unsigned long AddStatusFlags(unsigned long nStatus) const;

  // Server Side List functions
  time_t GetSSTime() const                      { return m_nSSTime; }
  void SetSSTime(time_t t)            { m_nSSTime = t; }
  unsigned short GetSSCount() const             { return m_nSSCount; }
  void SetSSCount(unsigned short n)   { m_nSSCount = n; }
  unsigned short GetPDINFO() const              { return m_nPDINFO; }
  void SetPDINFO(unsigned short n)    { m_nPDINFO = n; SaveLicqInfo(); }

  void SetPicture(const char *f);

  // Virtual overloaded functions
  virtual void SaveLicqInfo();
  virtual void SetStatusOffline();

  virtual bool User() const                     { return false; }
protected:
  char *m_szPassword;
  bool m_bException,
       m_bWebAware,
       m_bHideIp,
       m_bSavePassword;
  unsigned long  m_nRandomChatGroup;
  unsigned short m_nSSCount;
  time_t m_nSSTime;
  unsigned short m_nPDINFO;
};

/**
 * Class holding data for a user group in the contact list.
 * System groups only exists as a bitmask in LicqUser.
 *
 * Note: LicqGroup objects should only be created, deleted or modified from the
 * user manager. If set functions are called directly, plugins will not receive
 * any signal notifying them of the change.
 */
class LicqGroup
{
public:
  /**
   * Constructor, creates a new user group
   *
   * @param id Group id, must be unique
   * @param name Group name
   */
  LicqGroup(int id, const std::string& name);

  /**
   * Destructor
   */
  virtual ~LicqGroup();

  /**
   * Get id for group. This is an id used locally by Licq and is persistant for
   * each group.
   *
   * @return Group id
   */
  int id() const { return myId; }

  /**
   * Get name of group as should be displayed in the user interface
   *
   * @return Group name
   */
  const std::string& name() const { return myName; }

  /**
   * Get sorting index for the group. This is used by user interface plugins to
   * determine sorting order for the groups. Lower numbers should be displayed
   * higher in the list.
   *
   * @return Sorting index for this group
   */
  int sortIndex() const { return mySortIndex; }

  /**
   * Group id for this group in the ICQ server side list
   *
   * @return ICQ server group id or 0 if not set or not known
   */
  unsigned short icqGroupId() const { return myIcqGroupId; }

  /**
   * Set group name
   *
   * @param name New group name
   */
  void setName(const std::string& name) { myName = name; }

  /**
   * Set sorting index for group
   *
   * @param sortIndex Group sorting index
   */
  void setSortIndex(int sortIndex) { mySortIndex = sortIndex; }

  /**
   * Set group id in ICQ server side list
   *
   * @param icqGroupId ICQ server group id
   */
  void setIcqGroupId(unsigned short icqGroupId) { myIcqGroupId = icqGroupId; }

  /**
   * Lock group for access
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   */
  void Lock(unsigned short lockType = LOCK_R) const;

  /**
   * Release current lock for group
   */
  void Unlock() const;

private:
  int myId;
  std::string myName;
  int mySortIndex;
  unsigned short myIcqGroupId;

  mutable pthread_rdwr_t myMutex;
  mutable unsigned short myLockType;
};

/**
 * Helper function for sorting group list
 *
 * @param first Left hand group to compare
 * @param second Right hand group to compare
 * @return True if first has a lower sorting index than second
 */
bool compare_groups(const LicqGroup* first, const LicqGroup* second);

class CUserManager
{
public:
  CUserManager();
  ~CUserManager();
  bool Load();

  /**
   * Find and lock an user object
   *
   * @param userId User id
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @return The locked user object if id was valid, otherwise NULL
   */
  LicqUser* fetchUser(int userId, unsigned short lockType = LOCK_R);

  /**
   * Find and lock an user object based on account id
   *
   * @param accountId Server side account id of user
   * @param ppid Protocol instance id
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @param addUser True if user should be added (as temporary) if not found
   * @param retWasAdded If not null, will be set to true if user was added
   * @return The locked user object if user exist or was created, otherwise NULL
   */
  LicqUser* fetchUser(const std::string& accountId, unsigned long ppid,
      unsigned short lockType = LOCK_R, bool addUser = false, bool* retWasAdded = NULL);

  // For protocol plugins
  void AddOwner(const char *, unsigned long);
  void RemoveOwner(unsigned long);

  LicqUser* FetchUser(const char* idstring, unsigned long ppid, unsigned short lockType)
  { return fetchUser(idstring, ppid, lockType); }

  LicqOwner* FetchOwner(unsigned long ppid, unsigned short lockType);

  /**
   * Release owner lock
   */
  void DropOwner(const LicqOwner* owner);

  /**
   * Check if a user id is valid
   *
   * @param id User id to check
   * @return True if user id exists, otherwise false
   */
  bool userExists(int id);

  bool IsOnList(const char *, unsigned long);
  LICQ_DEPRECATED LicqOwner* FindOwner(const char* idstring, unsigned long ppid);

  /**
   * Get user id for an owner
   *
   * @param ppid Protocol id
   * @return User id of owner or empty string if no such owner exists
   */
  std::string OwnerId(unsigned long ppid);

  /**
   * Check if user is an owner
   *
   * @param userId Id of user to check
   * @return True if user id is valid and user is an owner
   */
  bool isOwner(int userId);

  /**
   * Get user id for an account
   *
   * @param accountId Server account id
   * @param ppid Protocol instance id
   * @return Local user id
   */
  int getUserFromAccount(const char* accountId, unsigned long ppid);

  // ICQ Protocol only (from original Licq)
  void DropUser(const LicqUser* user);

  /**
   * Convenience function to get icq owner as an unsigned long
   * Only meant to be used internally for icq protocol functions
   *
   * @return Icq owner
   */
  unsigned long icqOwnerUin();

  /**
   * Add a user to the contact list
   *
   * @param accountId Server account id
   * @param ppid Protocol instance id
   * @param permanent True if user should be added permanently to list and saved to disk
   * @param addToServer True if server should be notified (ignored for temporary users)
   * @param groupId Initial group to place user in or zero for no group
   * @return zero if account id is invalid or user is already in list, otherwise id of added user
   */
  int addUser(const std::string& accountId, unsigned long ppid,
      bool permanent = true, bool addToServer = true, unsigned short groupId = 0);

  /**
   * Remove a user from the list
   *
   * @param userId Id of user to remove
   */
  void removeUser(int userId);

  LICQ_DEPRECATED // use removeUser() instead
  void RemoveUser(const char* accountId, unsigned long ppid)
  { removeUser(getUserFromAccount(accountId, ppid)); }

  /**
   * Lock user list for access
   * call UnlockUserList when lock is no longer needed
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @return Map of all users indexed by user id
   */
  UserMap* LockUserList(unsigned short lockType = LOCK_R);

  /**
   * Lock and get account map
   * Same lock as LockUserList() but returns account map instead
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @return Map of all users indexed by UserAccountMapKey
   */
  UserAccountMap* lockUserAccounts(unsigned short lockType = LOCK_R)
  { LockUserList(lockType); return &myUserAccounts; }

  /**
   * Release user list lock
   */
  void UnlockUserList();

  /**
   * Save user list to configuration file
   * Note: This function assumes that the user list is already locked.
   */
  void saveUserList() const;

  /**
   * Lock group list for access
   * Call UnlockGroupList when lock is no longer needed
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @return Map of all user groups indexed by group ids
   */
  GroupMap* LockGroupList(unsigned short lockType = LOCK_R);

  /**
   * Release group list lock
   */
  void UnlockGroupList();

  /**
   * Lock owner list for access
   * Call UnlockOwnerList when lock is no longer needed
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @param Map of all owners indexed by protocol instance id
   */
  OwnerMap* LockOwnerList(unsigned short lockType = LOCK_R);

  /**
   * Release owner list lock
   */
  void UnlockOwnerList();

  /**
   * Find and lock a group
   * After use, the lock must be released by calling DropGroup()
   *
   * @param groupId Id of group to fetch
   * @param lockType Type of lock to get
   * @return The group if found no NULL if groupId was invalid
   */
  LicqGroup* FetchGroup(int groupId, unsigned short lockType = LOCK_R);

  /**
   * Release the lock for a group preivously returned by FetchGroup()
   *
   * @param group The group to unlock
   */
  void DropGroup(const LicqGroup* group);

  /**
   * Check if a group id is valid
   *
   * @param gtype Group type
   * @param groupId Id of group to check for
   * @return True if the group exists
   */
  bool groupExists(GroupType gtype, int groupId);

  /**
   * Add a user group
   *
   * @param name Group name, must be unique
   * @param icqGroupId ICQ server group id
   * @return Id of new group or zero if group could not be created
   */
  int AddGroup(const std::string& name, unsigned short icqGroupId = 0);

  /**
   * Remove a user group
   *
   * @param groupId Id of group to remove
   */
  void RemoveGroup(int groupId);

  /**
   * Rename a user group
   *
   * @param groupId Id of group to rename
   * @param name New group name, must be unique
   * @param sendUpdate True if server group should be updated
   * @return True if group was successfully renamed
   */
  bool RenameGroup(int groupId, const std::string& name, bool sendUpdate = true);

  /**
   * Get number of user groups
   *
   * @return Number of user groups
   */
  unsigned int NumGroups();

  /**
   * Save user group list to configuration file
   * Note: This function assumes that user group list is already locked.
   */
  void SaveGroups();

  /**
   * Move sorting position for a group
   * Sorting position for other groups may also be changed to make sure all
   * groups have unique sorting indexes.
   *
   * @param groupId Id of group to move
   * @param newIndex New sorting index where 0 is the top position
   */
  void ModifyGroupSorting(int groupId, int newIndex);

  /**
   * Change ICQ server group id for a user group
   *
   * @param name Name of group to change
   * @param icqGroupId ICQ server group id to set
   */
  void ModifyGroupID(const std::string& name, unsigned short icqGroupId);

  /**
   * Change ICQ server group id for a user group
   *
   * @param groupId Id of group to change
   * @param icqGroupId ICQ server group id to set
   */
  void ModifyGroupID(int groupId, unsigned short icqGroupId);

  /**
   * Get ICQ group id from group name
   *
   * @param name Group name
   * @return Id for ICQ server group or 0 if not found
   */
  unsigned short GetIDFromGroup(const std::string& name);

  /**
   * Get ICQ group id from group
   *
   * @param groupId Group
   * @return Id for iCQ server group or 0 if groupId was invalid
   */
  unsigned short GetIDFromGroup(int groupId);

  /**
   * Get group id from ICQ server group id
   *
   * @param icqGroupId ICQ server group id
   * @return Id for group or 0 if not found
   */
  int GetGroupFromID(unsigned short icqGroupId);

  /**
   * Find id for group with a given name
   *
   * @param name Name of the group
   * @return Id for the group or 0 if there is no group with that name
   */
  int GetGroupFromName(const std::string& name);

  unsigned short GenerateSID();

  /**
   * Set user group membership and (optionally) update server
   *
   * @param userId User id
   * @param groupType Group type
   * @param groupId Group id
   * @param inGroup True to add user to group or false to remove
   * @param updateServer True if server list should be updated
   */
  void setUserInGroup(int userId, GroupType groupType, int groupId, bool inGroup,
      bool updateServer = true);

  void SetUserInGroup(const char* id, unsigned long ppid, GroupType groupType,
      int groupId, bool inGroup, bool updateServer = true)
  { setUserInGroup(getUserFromAccount(id, ppid), groupType, groupId, inGroup, updateServer); }

  /**
   * Add user to a group and update server group
   *
   * @param userId User id
   * @param groupId Group id
   */
  void addUserToGroup(int userId, int groupId)
  { setUserInGroup(userId, GROUPS_USER, groupId, true, true); }

  void AddUserToGroup(const char* id, unsigned long ppid, int groupId)
  { SetUserInGroup(id, ppid, GROUPS_USER, groupId, true, true); }

  /**
   * Remove user from a group
   *
   * @param userId User id
   * @param groupId Group id
   */
  void removeUserFromGroup(int userId, int groupId)
  { setUserInGroup(userId, GROUPS_USER, groupId, false); }

  void RemoveUserFromGroup(const char* id, unsigned long ppid, int groupId)
  { SetUserInGroup(id, ppid, GROUPS_USER, groupId, false); }

  void SaveAllUsers();

  char* DefaultUserEncoding() { return m_szDefaultEncoding; }
  void SetDefaultUserEncoding(const char* defaultEncoding);

  bool UpdateUsersInGroups();

  unsigned short NumUsers();
  unsigned short NumOwners();

protected:
  pthread_rdwr_t mutex_grouplist, mutex_userlist, mutex_ownerlist;

  GroupMap myGroups;
  UserMap myUsers;
  UserAccountMap myUserAccounts;
  OwnerMap myOwners;
  unsigned short m_nUserListLockType;
  unsigned short myGroupListLockType;
  unsigned short m_nOwnerListLockType;
  bool m_bAllowSave;
  char* m_szDefaultEncoding;

  friend class CICQDaemon;
};


extern class CUserManager gUserManager;

#endif
