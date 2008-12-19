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
    ICQUser *pUser;                                      \
    const UserMap* _ul_ = gUserManager.LockUserList(LOCK_R); \
    for (UserMap::const_iterator _i_ = _ul_->begin();    \
         _i_ != _ul_->end(); _i_++)                      \
    {                                                    \
      pUser = _i_->second;                               \
      pUser->Lock(x);                                    \
      {

#define FOR_EACH_PROTO_USER_START(x, y)                  \
  {                                                      \
    ICQUser *pUser;                                      \
    const UserMap* _ul_ = gUserManager.LockUserList(LOCK_R); \
    for (UserMap::const_iterator _i_ = _ul_->begin();    \
         _i_ != _ul_->end(); _i_++)                      \
    {                                                    \
      if (_i_->first.second != x)                        \
        continue;                                        \
      pUser = _i_->second;                               \
      pUser->Lock(y);                                    \
      {

#define FOR_EACH_OWNER_START(x)                           \
  {                                                       \
    ICQOwner *pOwner;                                     \
    OwnerList *_ol_ = gUserManager.LockOwnerList(LOCK_R); \
    for (OwnerList::iterator _i_ = _ol_->begin();         \
         _i_ != _ol_->end(); _i_++)                       \
    {                                                     \
      pOwner = *_i_;                                      \
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



/*---------------------------------------------------------------------------
 * FOR_EACH_UIN
 *
 * Macros to iterate through the entire list of uins.  "nUin" will be the
 * current uin.  Useful for situations when just the uin is necessary and
 * each user does not need to be locked.  Note the corresponding user can be
 * fetched and locked inside the loop.
 *-------------------------------------------------------------------------*/
#define FOR_EACH_UIN_START                               \
  {                                                      \
    unsigned long nUin;                                  \
    const UserMap* _ul_ = gUserManager.LockUserList(LOCK_R); \
    for (UserMap::const_iterator _i_ = _ul_->begin();    \
         _i_ != _ul_->end(); _i_++)                      \
    {                                                    \
      nUin = _i_->second->Uin();                         \
      {

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

#define FOR_EACH_UIN_END                 \
      }                                  \
    }                                    \
    gUserManager.UnlockUserList();       \
  }

#define FOR_EACH_PROTO_ID_END FOR_EACH_UIN_END

#define FOR_EACH_UIN_BREAK               \
        {                                \
          break;                         \
        }

#define FOR_EACH_PROTO_ID_BREAK          \
        {                                \
          break;                         \
        }

#define FOR_EACH_UIN_CONTINUE            \
        {                                \
          continue;                      \
        }

#define FOR_EACH_PROTO_ID_CONTINUE       \
        {                                \
          continue;                      \
        }

class ICQUser;
class ICQOwner;
class LicqGroup;

typedef std::pair<std::string, unsigned long> UserMapKey;
typedef std::map<UserMapKey, class ICQUser*> UserMap;
typedef std::list<class ICQOwner *> OwnerList;
typedef std::set<unsigned short> UserGroupList;
typedef std::map<unsigned short, LicqGroup*> GroupMap;
typedef std::map<unsigned short, std::string> GroupNameMap;
typedef std::list<unsigned long> UinList;
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

const unsigned long GROUP_ALL_USERS       = 0;
const unsigned long GROUP_ONLINE_NOTIFY   = 1;
const unsigned long GROUP_VISIBLE_LIST    = 2;
const unsigned long GROUP_INVISIBLE_LIST  = 3;
const unsigned long GROUP_IGNORE_LIST     = 4;
const unsigned long GROUP_NEW_USERS       = 5;

/**
 * The amount of registered system groups
 */
const unsigned long NUM_GROUPS_SYSTEM_ALL = 6;

/**
 * The amount of registered system groups, excluding the 'All Users' group.
 *
 * @deprecated Scheduled for removal, use NUM_GROUPS_SYSTEM_ALL instead.
 */
const unsigned long NUM_GROUPS_SYSTEM     = NUM_GROUPS_SYSTEM_ALL - 1;

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

  friend class ICQUser;
};

//=====ICQUser==================================================================
/*! \brief Details about an ICQ user and operations to perform on them.

    This class contains all the information about an ICQ user.  It can be
    retrieved with a read lock (LOCK_R) and may be set with a write lock (LOCK_W).
    Everything about an ICQ user is in this class.
*/
class ICQUser
{
public:
  ICQUser(const char *id, unsigned long ppid, char *filename);
  ICQUser(const char *id, unsigned long ppid, bool bTempUser = false);
  virtual ~ICQUser();
  void RemoveFiles();

  void saveAll();
  virtual void SaveLicqInfo();
  void saveUserInfo();
  void SavePhoneBookInfo();
  void SavePictureInfo();
  void SaveNewMessagesInfo();

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
  unsigned long PPID() const                    { return m_nPPID; }
  const char* IdString() const                  { return m_szId; }
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
  void SetPPID(unsigned long n)       { m_nPPID = n; }
  void SetId(const char *s)            { SetString(&m_szId, s); SaveLicqInfo(); }
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
  static char *MakeRealId(const char *, unsigned long, char *&);
  int Birthday(unsigned short nDayRange = 0) const;

  // Message/History functions
  unsigned short NewMessages() const            { return(m_vcMessages.size()); }
  CUserEvent *EventPeek(unsigned short);
  const CUserEvent* EventPeek(unsigned short index) const;
  CUserEvent *EventPeekId(int);
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
  bool GetInGroup(GroupType gtype, unsigned short groupId) const;

  /**
   * Convenience function to set membership of user for a group
   *
   * @param gtype Group type (GROUPS_SYSTEM or GROUPS_USER)
   * @param groupId Id of group
   * @param member True to add user to group, false to remove user from group
   */
  void SetInGroup(GroupType gtype, unsigned short groupId, bool member);

  /**
   * Add user to a group
   *
   * @param gtype Group type (GROUPS_SYSTEM or GROUPS_USER)
   * @param groupId Id of group to add
   */
  void AddToGroup(GroupType gtype, unsigned short groupId);

  /**
   * Remove user from a group
   *
   * @param gtype Group type (GROUPS_SYSTEM or GROUPS_USER)
   * @pram groupId Id of group to leave
   * @return True if group was valid and user was a member
   */
  bool RemoveFromGroup(GroupType gtype, unsigned short groupId);

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
  void Lock(unsigned short lockType) const;
  void Unlock() const;

  // Deprecated functions, to be removed
  ICQUser(unsigned long id, char *filename) LICQ_DEPRECATED;
  ICQUser(unsigned long id) LICQ_DEPRECATED;
  LICQ_DEPRECATED unsigned long Uin() const;
  LICQ_DEPRECATED const char* UinString() const { return m_szId; }

protected:
  ICQUser() { /* ICQOwner inherited constructor - does nothing */ }
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
  void Init(const char *, unsigned long);
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

  // Deprecated functions, to be removed
  LICQ_DEPRECATED void Init(unsigned long nUin);

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
  char *m_szId;
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

  // Protocol ID
  unsigned long m_nPPID;

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


//=====ICQOwner=================================================================
class ICQOwner : public ICQUser
{
public:
  ICQOwner(const char *, unsigned long);
  virtual ~ICQOwner();
  bool Exception() const                        { return m_bException; }

  // Owner specific functions
  const char* Password() const                  { return(m_szPassword); }
  void SetPassword(const char *s) { SetString(&m_szPassword, s); SaveLicqInfo(); }
  void SetWebAware(bool b)     {  m_bWebAware = b; SaveLicqInfo(); }
  virtual void SetWebAwareStatus(char c) { SetWebAware(c); }
  void SetHideIp(bool b)       {  m_bHideIp = b; SaveLicqInfo(); }
  void SetSavePassword(bool b) {  m_bSavePassword = b; SaveLicqInfo(); }
  void SetId(const char *s)    { SetString(&m_szId, s); SaveLicqInfo(); }
  void SetRandomChatGroup(unsigned long n)  { m_nRandomChatGroup = n; SaveLicqInfo(); }
  bool WebAware() const                         { return m_bWebAware; }
  bool HideIp() const                           { return m_bHideIp; }
  bool SavePassword() const                     { return m_bSavePassword; }
  unsigned long RandomChatGroup() const         { return m_nRandomChatGroup; }
  unsigned long AddStatusFlags(unsigned long nStatus) const;

  // Deprecated functions, to be removed
  LICQ_DEPRECATED void SetUin(unsigned long n);

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
 * System groups only exists as a bitmask in ICQUser.
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
  LicqGroup(unsigned short id, const std::string& name);

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
  unsigned short id() const { return myId; }

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
  unsigned short sortIndex() const { return mySortIndex; }

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
  void setSortIndex(unsigned short sortIndex) { mySortIndex = sortIndex; }

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
  void Lock(unsigned short lockType) const;

  /**
   * Release current lock for group
   */
  void Unlock() const;

private:
  unsigned short myId;
  std::string myName;
  unsigned short mySortIndex;
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

  // For protocol plugins
  void AddOwner(const char *, unsigned long);
  void AddUser(ICQUser *, const char *, unsigned long);
  void RemoveUser(const char *, unsigned long);
  void RemoveOwner(unsigned long);
  ICQUser *FetchUser(const char *, unsigned long, unsigned short);
  ICQOwner *FetchOwner(unsigned long, unsigned short);

  /**
   * Release owner lock
   */
  void DropOwner(const ICQOwner* owner);

  bool IsOnList(const char *, unsigned long);
  ICQOwner *FindOwner(const char *, unsigned long);

  /**
   * Get user id for an owner
   *
   * @param ppid Protocol id
   * @return User id of owner or empty string if no such owner exists
   */
  std::string OwnerId(unsigned long ppid);

  // ICQ Protocol only (from original Licq)
  void AddUser(ICQUser *);
  void DropUser(const ICQUser* user);

  // Deprecated user functions, to be removed
  LICQ_DEPRECATED ICQUser *FetchUser(unsigned long, unsigned short);
  LICQ_DEPRECATED void RemoveUser(unsigned long);
  LICQ_DEPRECATED bool IsOnList(unsigned long nUin);

  // Deprecated owner functions, to be removed
  LICQ_DEPRECATED void SetOwnerUin(unsigned long _nUin);
  LICQ_DEPRECATED unsigned long OwnerUin() { return icqOwnerUin(); }
  LICQ_DEPRECATED ICQOwner *FetchOwner(unsigned short);
  LICQ_DEPRECATED void DropOwner();
  LICQ_DEPRECATED void DropOwner(unsigned long);

  /**
   * Convenience function to get icq owner as an unsigned long
   * Only meant to be used internally for icq protocol functions
   *
   * @return Icq owner
   */
  unsigned long icqOwnerUin();

  /**
   * Lock user list for access
   * call UnlockUserList when lock is no longer needed
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @return Map of all users indexed by UserMapKey
   */
  UserMap* LockUserList(unsigned short lockType);

  /**
   * Release user list lock
   */
  void UnlockUserList();

  /**
   * Lock group list for access
   * Call UnlockGroupList when lock is no longer needed
   *
   * @param lockType Type of lock (LOCK_R or LOCK_W)
   * @return Map of all user groups indexed by group ids
   */
  GroupMap* LockGroupList(unsigned short lockType);

  /**
   * Release group list lock
   */
  void UnlockGroupList();

  OwnerList *LockOwnerList(unsigned short);
  void UnlockOwnerList();

  /**
   * Find and lock a group
   * After use, the lock must be released by calling DropGroup()
   *
   * @param groupId Id of group to fetch
   * @param lockType Type of lock to get
   * @return The group if found no NULL if groupId was invalid
   */
  LicqGroup* FetchGroup(unsigned short groupId, unsigned short lockType);

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
  bool groupExists(GroupType gtype, unsigned short groupId);

  /**
   * Add a user group
   *
   * @param name Group name, must be unique
   * @param icqGroupId ICQ server group id
   * @return Id of new group or zero if group could not be created
   */
  unsigned short AddGroup(const std::string& name, unsigned short icqGroupId = 0);

  /**
   * Remove a user group
   *
   * @param groupId Id of group to remove
   */
  void RemoveGroup(unsigned short groupId);

  /**
   * Rename a user group
   *
   * @param groupId Id of group to rename
   * @param name New group name, must be unique
   * @param sendUpdate True if server group should be updated
   * @return True if group was successfully renamed
   */
  bool RenameGroup(unsigned short groupId, const std::string& name, bool sendUpdate = true);

  /**
   * Get number of user groups
   *
   * @return Number of user groups
   */
  unsigned short NumGroups();

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
  void ModifyGroupSorting(unsigned short groupId, unsigned short newIndex);

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
  void ModifyGroupID(unsigned short groupId, unsigned short icqGroupId);

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
  unsigned short GetIDFromGroup(unsigned short groupId);

  /**
   * Get group id from ICQ server group id
   *
   * @param icqGroupId ICQ server group id
   * @return Id for group or 0 if not found
   */
  unsigned short GetGroupFromID(unsigned short icqGroupId);

  /**
   * Find id for group with a given name
   *
   * @param name Name of the group
   * @return Id for the group or 0 if there is no group with that name
   */
  unsigned short GetGroupFromName(const std::string& name);

  unsigned short GenerateSID();

  /**
   * Set user group membership and (optionally) update server
   *
   * @param id User id
   * @param ppid User protocol id
   * @param groupType Group type
   * @param groupId Group id
   * @param inGroup True to add user to group or false to remove
   * @param updateServer True if server list should be updated
   */
  void SetUserInGroup(const char* id, unsigned long ppid, GroupType groupType,
      unsigned short groupId, bool inGroup, bool updateServer = true);

  // Deprecated group manipulation functions
  LICQ_DEPRECATED void AddUserToGroup(unsigned long _nUin, unsigned short _nGroup);
  LICQ_DEPRECATED void RemoveUserFromGroup(unsigned long _nUin, unsigned short _nGroup);

  /**
   * Add user to a group and update server group
   *
   * @param id User id
   * @param ppid User protocol id
   * @param groupId Group id
   */
  void AddUserToGroup(const char* id, unsigned long ppid, unsigned short groupId)
  { SetUserInGroup(id, ppid, GROUPS_USER, groupId, true, true); }

  /**
   * Remove user from a group
   *
   * @param id User id
   * @param ppid User protocol id
   * @param groupId Group id
   */
  void RemoveUserFromGroup(const char* id, unsigned long ppid, unsigned short groupId)
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
  OwnerList m_vpcOwners;
  ICQOwner *m_xOwner;
  unsigned short m_nUserListLockType;
  unsigned short myGroupListLockType;
  unsigned short m_nOwnerListLockType;
  bool m_bAllowSave;
  char* m_szDefaultEncoding;

  friend class CICQDaemon;
};


extern class CUserManager gUserManager;

#endif
