#ifndef USER_H
#define USER_H

#include <vector>
#include <list>
#include <time.h>

#include "pthread_rdwr.h"

#include "licq_message.h"
#include "licq_history.h"
#include "licq_file.h"
#include "licq_icq.h"


class TCPSocket;

#define LICQ_PPID 0x4C696371  // "Licq"

/*---------------------------------------------------------------------------
 * FOR_EACH_USER
 *
 * Macros to iterate through the entire list of users.  "pUser" will be a
 * pointer to the current user.
 *-------------------------------------------------------------------------*/
#define FOR_EACH_USER_START(x)                           \
  {                                                      \
    ICQUser *pUser;                                      \
    UserList *_ul_ = gUserManager.LockUserList(LOCK_R);  \
    for (UserList::iterator _i_ = _ul_->begin();         \
         _i_ != _ul_->end(); _i_++)                      \
    {                                                    \
      pUser = *_i_;                                      \
      pUser->Lock(x);                                    \
      {

#define FOR_EACH_PROTO_USER_START(x, y)                  \
  {                                                      \
    ICQUser *pUser;                                      \
    UserList *_ul_ = gUserManager.LockUserList(LOCK_R);  \
    for (UserList::iterator _i_ = _ul_->begin();         \
         _i_ != _ul_->end(); _i_++)                      \
    {                                                    \
      pUser = *_i_;                                      \
      if (pUser->PPID() == x)                            \
      {                                                  \
        pUser->Lock(y);                                  \
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

#define FOR_EACH_PROTO_USER_END        \
        }                                \
        pUser->Unlock();                 \
      }                                  \
    }                                    \
    gUserManager.UnlockUserList();       \
  }

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
    UserList *_ul_ = gUserManager.LockUserList(LOCK_R);  \
    for (UserList::iterator _i_ = _ul_->begin();         \
         _i_ != _ul_->end(); _i_++)                      \
    {                                                    \
      nUin = (*_i_)->Uin();                              \
      {

#define FOR_EACH_PROTO_ID_START(x)                       \
  {                                                      \
    char *szId;                                          \
    UserList *_ul_ = gUserManager.LockUserList(LOCK_R);  \
    for (UserList::iterator _i_ = _ul_->begin();         \
         _i_ != _ul_->end(); _i_++)                      \
    {                                                    \
      if ((*_i_)->PPID() == x)                           \
      {                                                  \
        szId = (*_i_)->IdString();                       \
        {

#define FOR_EACH_UIN_END                 \
      }                                  \
    }                                    \
    gUserManager.UnlockUserList();       \
  }

#define FOR_EACH_PROTO_ID_END            \
        }                                \
      }                                  \
    }                                    \
    gUserManager.UnlockUserList();       \
  }

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

typedef std::list<ICQUser *> UserList;
typedef std::list<class ICQOwner *> OwnerList;
typedef std::vector<char *> GroupList;
typedef std::vector<unsigned short> GroupIDList;
typedef std::list<unsigned long> UinList;
typedef std::list<char *> UserStringList;
typedef std::vector <class CUserEvent *> UserEventList;

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

const unsigned long GROUP_ONLINE_NOTIFY   = 1;
const unsigned long GROUP_VISIBLE_LIST    = 2;
const unsigned long GROUP_INVISIBLE_LIST  = 3;
const unsigned long GROUP_IGNORE_LIST     = 4;
const unsigned long GROUP_NEW_USERS       = 5;
const unsigned long NUM_GROUPS_SYSTEM = 5;
// We stick in an empty group 0 so that the names correspond
// to the identifiers
extern const char *GroupsSystemNames[NUM_GROUPS_SYSTEM+1];

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

typedef enum
{
  CAT_INTERESTS,
  CAT_ORGANIZATION,
  CAT_BACKGROUND,
  CAT_MAX
} UserCat;

//+++++OBJECTS++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//====ICQUserCategory===========================================================
class ICQUserCategory
{
public:
  ICQUserCategory(UserCat uc);
  ~ICQUserCategory();
  bool AddCategory (unsigned short cat, const char *descr);
  void Clean();

  bool Get(unsigned d,unsigned short *id, char const ** descr) const;
  UserCat GetCategory() { return m_uc; }

  static const unsigned MAX_CATEGORIES = 4;

private:
  bool SaveToDisk(CIniFile &m_fConf,const char *const szN,
                  const char *const szCat,const char *const szDescr);
  bool LoadFromDisk(CIniFile &m_fConf, const char *const szN,
                    const char *const szCat,const char *const szDescr);

  unsigned short used;
  
  struct cat
  {       unsigned short id;
          char *descr;
  };
  struct cat data[MAX_CATEGORIES];
  UserCat m_uc;
  friend class ICQUser;
};

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

  bool Get(unsigned long nEntry, const struct PhoneBookEntry  **entry);

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
  ICQUser(unsigned long id, char *filename);
  ICQUser(unsigned long id);
  ICQUser(const char *id, unsigned long ppid, char *filename);
  ICQUser(const char *id, unsigned long ppid);
  virtual ~ICQUser();
  void RemoveFiles();

  virtual void SaveLicqInfo();
  void SaveGeneralInfo();
  void SaveMoreInfo();
  void SaveHomepageInfo();
  void SaveWorkInfo();
  void SaveAboutInfo();
  void SaveInterestsInfo();
  void SaveBackgroundsInfo();
  void SaveOrganizationsInfo();
  void SavePhoneBookInfo();
  void SavePictureInfo();
  void SaveExtInfo();
  void SaveNewMessagesInfo();

  // General Info
  //!Retrieves the user's alias.
  char *GetAlias()                      {  return m_szAlias;  }
  //!Retrieves the user's first name.
  char *GetFirstName()                  {  return m_szFirstName;  }
  //!Retrieves the user's last name.
  char *GetLastName()                   {  return m_szLastName;  }
  //!Retrieves the user's primary e-mail address.
  char *GetEmailPrimary()               {  return m_szEmailPrimary;  }
  //!Retrieves the user's secondary e-mail address.
  char *GetEmailSecondary()             {  return m_szEmailSecondary;  }
  //!Retrieves the user's old e-mail address.
  char *GetEmailOld()                   {  return m_szEmailOld;  }
  //!Retrieves the user's city.
  char *GetCity()                       {  return m_szCity;  }
  //!Retrieves the user's state.
  char *GetState()                      {  return m_szState;  }
  //!Retrieves the user's phone number.
  char *GetPhoneNumber()                {  return m_szPhoneNumber;  }
  //!Retrieves the user's fax number.
  char *GetFaxNumber()                  {  return m_szFaxNumber;  }
  //!Retrieves the user's street address.
  char *GetAddress()                    {  return m_szAddress;  }
  //!Retrieves the user's cellular phone number.
  char *GetCellularNumber()             {  return m_szCellularNumber;  }
  //!Retrieves the user's zip code.
  char *GetZipCode()                    {  return m_szZipCode;  }
  //!Retrieves the user's country code.  Used to lookup the country name.
  unsigned short GetCountryCode()       {  return m_nCountryCode;  }
  //!Retrieves the user's time code.
  char GetTimezone()                    {  return m_nTimezone;  }
  //!Returns true if the user requires you to be authorized to add
  //!them to anyone's ICQ list.
  bool GetAuthorization()               {  return m_bAuthorization;  }
  //!Retrieves the users's web status
  unsigned char GetWebAwareStatus()     {  return m_nWebAwareStatus; }
  //!Returns true if the user has attempted to hide the e-mail addresses
  //!provided in their information.
  bool GetHideEmail()                   {  return m_bHideEmail;  }

  // More Info
  //!Retrieves the user's age.
  unsigned short GetAge()               {  return m_nAge;  }
  //!Retrieves the user's gender.
  char GetGender()                      {  return m_nGender;  }
  //!Retrieves the user's homepage URL.
  char *GetHomepage()                   {  return m_szHomepage;  }
  //!Retrieves the user's year they were born ih.
  unsigned short GetBirthYear()         {  return m_nBirthYear;  }
  //!Retrieves the user's month they were born in.
  char GetBirthMonth()                  {  return m_nBirthMonth;  }
  //!Retrieves the user's day they were born in.
  char GetBirthDay()                    {  return m_nBirthDay;  }
  //!Retrieves the user's first language.
  char GetLanguage1()                   {  return m_nLanguage[0];  }
  //!Retrieves the user's second language.
  char GetLanguage2()                   {  return m_nLanguage[1];  }
  //!Retrieves the user's third language.
  char GetLanguage3()                   {  return m_nLanguage[2];  }
  //!Retrieves the user's language as specified by the parameter.
  //!Useful when retrieving their languages in a loop.
  char GetLanguage(unsigned char l)        {  return m_nLanguage[l];  }

  // Homepage Info
  //!Returns true if the user has entered a homepage category
  bool GetHomepageCatPresent()             {  return m_bHomepageCatPresent; }
  //!Retrieves the user's homepage category code
  unsigned short GetHomepageCatCode()      {  return m_nHomepageCatCode; }
  //!Retrivies the users homepage description
  char *GetHomepageDesc()               {  return m_szHomepageDesc; }
  //!Returns true if the user has an ICQ homepage (http://<uin>.home.icq.com/)
  bool GetICQHomepagePresent()          {  return m_bICQHomepagePresent; }

  // Work Info
  //!Retrieves the city of the company the user is employed by.
  char *GetCompanyCity()                {  return m_szCompanyCity;  }
  //!Retrieves the state of the company the user is employed by.
  char *GetCompanyState()               {  return m_szCompanyState;  }
  //!Retrieves the phone number of the company's phone number the user is employed by.
  char *GetCompanyPhoneNumber()         {  return m_szCompanyPhoneNumber; }
  //!Retrieves the fax bynber of the company the user is employed by.
  char *GetCompanyFaxNumber()           {  return m_szCompanyFaxNumber;  }
  //!Retrieves the street address of the company the user is employed by.
  char *GetCompanyAddress()             {  return m_szCompanyAddress;  }
  //!Retrieves the zip code of the company the user is employed by.
  char *GetCompanyZip()			{  return m_szCompanyZip;  }
  //!Retrieves the country code of the company the user is employed by.
  //!Used to look up the country name.
  unsigned short GetCompanyCountry()	{  return m_nCompanyCountry;  }
  //!Retrieves the name of the company the user is employed by.
  char *GetCompanyName()                {  return m_szCompanyName;  }
  //!Retrieves the department the user is in.
  char *GetCompanyDepartment()          {  return m_szCompanyDepartment;  }
  //!Retrieves the user's job title.
  char *GetCompanyPosition()            {  return m_szCompanyPosition;  }
  //!Retrieves the users's occupation code
  unsigned short GetCompanyOccupation() {  return m_nCompanyOccupation; }
  //!Retrieves the URL of the company the user is employed by.
  char *GetCompanyHomepage()            {  return m_szCompanyHomepage;  }

  // About Info
  //!Retrieves the self description of the user.
  char *GetAbout()                      { return m_szAbout; }

  // More2 Info
  //!Retrieves the user's interests
  ICQUserCategory *GetInterests()       { return m_Interests; }
  //!Retrieves the user's backgrounds
  ICQUserCategory *GetBackgrounds()     { return m_Backgrounds; }
  //!Retrieves the user's organizations
  ICQUserCategory *GetOrganizations()   { return m_Organizations; }

  // Phone Book Info
  //!Retrives the user's phone book
  ICQUserPhoneBook *GetPhoneBook()      { return m_PhoneBook; }

  // Picture Info
  bool GetPicturePresent()              { return m_bPicturePresent; }

  // Licq Info
  bool GetAwaitingAuth()                { return m_bAwaitingAuth; }
  unsigned short GetSID()               { return m_nSID[NORMAL_SID]; }
  unsigned short GetInvisibleSID()      { return m_nSID[INV_SID]; }
  unsigned short GetVisibleSID()        { return m_nSID[VIS_SID]; }
  unsigned short GetGSID()              { return m_nGSID; }
  unsigned short GetTyping()            { return m_nTyping; }
  //!Retrieves the user's auto response message that was last seen.
  char *AutoResponse()                  { return m_szAutoResponse; }
  //!Retrieves the encoding Licq uses for this user
  char *UserEncoding()                  { return m_szEncoding; }
  bool SendServer()                     { return m_bSendServer; }
  unsigned short SendLevel()            { return m_nSendLevel; }
  bool EnableSave()                     { return m_bEnableSave; }
  bool ShowAwayMsg()                    { return m_bShowAwayMsg; }
  unsigned long Uin()                   { return m_nUin; }
  char *UinString()                     { return m_szUinString; }
  unsigned short Sequence(bool = false);
  char Mode()                           { return m_nMode; }
  unsigned long Version()               { return m_nVersion; }
  char *ClientInfo()                    { return m_szClientInfo; }
  unsigned long ClientTimestamp()       { return m_nClientTimestamp; }
  unsigned long OurClientTimestamp()    { return m_nOurClientTimestamp; }
  unsigned long ClientInfoTimestamp()   { return m_nClientInfoTimestamp; }
  unsigned long OurClientInfoTimestamp() { return m_nOurClientInfoTimestamp; }
  unsigned long ClientStatusTimestamp() { return m_nClientStatusTimestamp; }
  unsigned long OurClientStatusTimestamp() { return m_nOurClientStatusTimestamp; }
  bool UserUpdated()                    { return m_bUserUpdated; }
  SecureChannelSupport_et SecureChannelSupport();
  unsigned short LicqVersion();
  unsigned short ConnectionVersion();
  time_t LastOnline()                   { return m_nLastCounters[LAST_ONLINE]; }
  time_t LastSentEvent()                { return m_nLastCounters[LAST_SENT_EVENT]; }
  time_t LastReceivedEvent()            { return m_nLastCounters[LAST_RECV_EVENT]; }
  time_t LastCheckedAutoResponse()      { return m_nLastCounters[LAST_CHECKED_AR]; }
  time_t OnlineSince()      		{ return m_nOnlineSince; }
  time_t IdleSince()                    { return m_nIdleSince; }
  bool AutoChatAccept()                 { return m_nAutoAccept & AUTO_ACCEPT_CHAT; }
  bool AutoFileAccept()                 { return m_nAutoAccept & AUTO_ACCEPT_FILE; }
  bool AutoSecure()                     { return m_nAutoAccept & AUTO_SECURE; }
  bool AcceptInAway()                   { return m_nAutoAccept & ACCEPT_IN_AWAY; }
  bool AcceptInNA()                     { return m_nAutoAccept & ACCEPT_IN_NA; }
  bool AcceptInOccupied()               { return m_nAutoAccept & ACCEPT_IN_OCCUPIED; }
  bool AcceptInDND()                    { return m_nAutoAccept & ACCEPT_IN_DND; }
  unsigned short StatusToUser()         { return m_nStatusToUser; }
  bool KeepAliasOnUpdate()              { return m_bKeepAliasOnUpdate; }
  char *CustomAutoResponse()            { return m_szCustomAutoResponse; }
  unsigned long PPID()                  { return m_nPPID; }
  char *IdString()                      { return m_szId; }

  char *usprintf(const char *szFormat, unsigned long nFlags = 0);

  // General Info
  void SetAlias (const char *n);// {  SetString(&m_szAlias, n);  SaveGeneralInfo();  }
  void SetFirstName (const char *n)          {  SetString(&m_szFirstName, n);  SaveGeneralInfo();  }
  void SetLastName (const char *n)           {  SetString(&m_szLastName, n);  SaveGeneralInfo();  }
  void SetEmailPrimary (const char *n)       {  SetString(&m_szEmailPrimary, n);  SaveGeneralInfo();  }
  void SetEmailSecondary (const char *n)     {  SetString(&m_szEmailSecondary, n);  SaveGeneralInfo();  }
  void SetEmailOld(const char *n)            {  SetString(&m_szEmailOld, n);  SaveGeneralInfo();  }
  void SetCity (const char *n)               {  SetString(&m_szCity, n);  SaveGeneralInfo();  }
  void SetState (const char *n)              {  SetString(&m_szState, n);  SaveGeneralInfo();  }
  void SetPhoneNumber (const char *n)        {  SetString(&m_szPhoneNumber, n);  SaveGeneralInfo();  }
  void SetFaxNumber (const char *n)          {  SetString(&m_szFaxNumber, n);  SaveGeneralInfo();  }
  void SetAddress (const char *n)            {  SetString(&m_szAddress, n);  SaveGeneralInfo();  }
  void SetCellularNumber (const char *n)     {  SetString(&m_szCellularNumber, n);  SaveGeneralInfo();  }
  void SetZipCode (const char *n)            {  SetString(&m_szZipCode, n);  SaveGeneralInfo();  }
  void SetCountryCode (unsigned short n)     {  m_nCountryCode = n;  SaveGeneralInfo();  }
  void SetTimezone (const char n)            {  m_nTimezone = n;  SaveGeneralInfo();  }
  void SetAuthorization (bool n)             {  m_bAuthorization = n;  SaveGeneralInfo();  }
  virtual void SetWebAwareStatus (char n)    {  m_nWebAwareStatus = n;  }
  void SetHideEmail (bool n)                 {  m_bHideEmail = n;  SaveGeneralInfo();  }

  // More Info
  void SetAge (unsigned short n)             {  m_nAge = n;  SaveMoreInfo();  }
  void SetGender (const char n)              {  m_nGender = n;  SaveMoreInfo();  }
  void SetHomepage (const char *n)           {  SetString(&m_szHomepage, n);  SaveMoreInfo();  }
  void SetBirthYear (unsigned short n)       {  m_nBirthYear = n;  SaveMoreInfo();  }
  void SetBirthMonth (const char n)          {  m_nBirthMonth = n;  SaveMoreInfo();  }
  void SetBirthDay (const char n)            {  m_nBirthDay = n;  SaveMoreInfo();  }
  void SetLanguage1 (const char n)           {  m_nLanguage[0] = n;  SaveMoreInfo();  }
  void SetLanguage2 (const char n)           {  m_nLanguage[1] = n;  SaveMoreInfo();  }
  void SetLanguage3 (const char n)           {  m_nLanguage[2] = n;  SaveMoreInfo();  }
  void SetLanguage (unsigned char l, char n) {  m_nLanguage[l] = n;  SaveMoreInfo();  }

  // Homepage Info
  void SetHomepageCatPresent(bool n)         {  m_bHomepageCatPresent = n; SaveHomepageInfo(); }
  void SetHomepageCatCode(unsigned short n)  {  m_nHomepageCatCode = n; SaveHomepageInfo(); }
  void SetHomepageDesc(const char *n)        {  SetString(&m_szHomepageDesc, n); SaveHomepageInfo(); }
  void SetICQHomepagePresent(bool n)         {  m_bICQHomepagePresent = n; SaveHomepageInfo(); }

  // Work Info
  void SetCompanyCity (const char *n)        {  SetString(&m_szCompanyCity, n);  SaveWorkInfo();  }
  void SetCompanyState (const char *n)       {  SetString(&m_szCompanyState, n);  SaveWorkInfo();  }
  void SetCompanyPhoneNumber (const char *n) {  SetString(&m_szCompanyPhoneNumber, n);  SaveWorkInfo();  }
  void SetCompanyFaxNumber (const char *n)   {  SetString(&m_szCompanyFaxNumber, n);  SaveWorkInfo();  }
  void SetCompanyAddress (const char *n)     {  SetString(&m_szCompanyAddress, n);  SaveWorkInfo();  }
  void SetCompanyZip (const char *n)	     {  SetString(&m_szCompanyZip, n);  SaveWorkInfo();  }
  void SetCompanyCountry (unsigned short n)  {  m_nCompanyCountry = n;  SaveWorkInfo();  }
  void SetCompanyName (const char *n)        {  SetString(&m_szCompanyName, n);  SaveWorkInfo();  }
  void SetCompanyDepartment (const char *n)  {  SetString(&m_szCompanyDepartment, n);  SaveWorkInfo();  }
  void SetCompanyPosition (const char *n)    {  SetString(&m_szCompanyPosition, n);  SaveWorkInfo();  }
  void SetCompanyOccupation (unsigned short n) {  m_nCompanyOccupation = n;  SaveWorkInfo();  }
  void SetCompanyHomepage (const char *n)    {  SetString(&m_szCompanyHomepage, n);  SaveWorkInfo();  }

  // About Info
  void SetAbout(const char *n)        {  SetString(&m_szAbout, n);  SaveAboutInfo();  }

  // Picture info
  void SetPicturePresent(bool b)      { m_bPicturePresent = b; SavePictureInfo(); }

  // Licq Info
  void SetAwaitingAuth(bool b)        { m_bAwaitingAuth = b; }
  void SetSID(unsigned short s)       { m_nSID[NORMAL_SID] = s; }
  void SetInvisibleSID(unsigned short s) { m_nSID[INV_SID] = s; }
  void SetVisibleSID(unsigned short s){ m_nSID[VIS_SID] = s; }
  void SetGSID(unsigned short s)      { m_nGSID = s; }
  void SetEnableSave(bool s)          { if (m_bOnContactList) m_bEnableSave = s; }
  void SetSendServer(bool s)          { m_bSendServer = s; }
  void SetSendLevel(unsigned short s) { m_nSendLevel = s; }
  void SetSequence(unsigned short s)  { m_nSequence = s; }
  void SetAutoResponse(const char *s) { SetString(&m_szAutoResponse, s); }
  void SetUserEncoding(const char* s) { SetString(&m_szEncoding, s); }
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
  void SetStatusToUser(unsigned short s)    { m_nStatusToUser = s; SaveLicqInfo(); }
  void SetKeepAliasOnUpdate(bool b)   { m_bKeepAliasOnUpdate = b; }
  void SetCustomAutoResponse(const char *s) { SetString(&m_szCustomAutoResponse, s); SaveLicqInfo(); }
  void ClearCustomAutoResponse()            { SetCustomAutoResponse(""); }
  void SetTyping(unsigned short nTyping)    { m_nTyping = nTyping; }
  void SetPPID(unsigned long n)       { m_nPPID = n; }
  void SetId(const char *s)            { SetString(&m_szId, s); SaveLicqInfo(); }
  void SetClientInfo(const char *s)
  { if(m_szClientInfo) free(m_szClientInfo); 
    if(s) m_szClientInfo = strdup(s); else m_szClientInfo = NULL; 
  }

  // Status
  unsigned short Status();
  unsigned long StatusFull()   {  return m_nStatus; }
  bool StatusInvisible()       {  return StatusOffline() ? false : m_nStatus & ICQ_STATUS_FxPRIVATE; }
  bool StatusWebPresence()     {  return m_nStatus & ICQ_STATUS_FxWEBxPRESENCE;  }
  bool StatusHideIp()          {  return m_nStatus & ICQ_STATUS_FxHIDExIP; }
  bool StatusBirthday()        {  return m_nStatus & ICQ_STATUS_FxBIRTHDAY;  }
  bool StatusOffline()         {  return (unsigned short)m_nStatus == ICQ_STATUS_OFFLINE;  }
  unsigned long PhoneFollowMeStatus() { return m_nPhoneFollowMeStatus; }
  unsigned long ICQphoneStatus()      { return m_nICQphoneStatus; }
  unsigned long SharedFilesStatus()   { return m_nSharedFilesStatus; }
  void SetStatus(unsigned long n)  {  m_nStatus = n;  }
  void SetPhoneFollowMeStatus(unsigned long n)  { m_nPhoneFollowMeStatus = n; SaveLicqInfo(); }
  void SetICQphoneStatus(unsigned long n)       { m_nICQphoneStatus = n; }
  void SetSharedFilesStatus(unsigned long n)    { m_nSharedFilesStatus = n; }  
  virtual void SetStatusOffline();
  const char *StatusStr();
  const char *StatusStrShort();
  bool Away();
  static const char *StatusToStatusStr(unsigned short, bool);
  static const char *StatusToStatusStrShort(unsigned short, bool);
  static char *MakeRealId(const char *, unsigned long, char *&);
  int Birthday(unsigned short nDayRange = 0);

  // Message/History functions
  unsigned short NewMessages()   { return(m_vcMessages.size()); }
  CUserEvent *EventPeek(unsigned short);
  CUserEvent *EventPeekId(int);
  CUserEvent *EventPeekFirst();
  CUserEvent *EventPeekLast();
  CUserEvent *EventPop();
  void EventClear(unsigned short);
  void EventClearId(int);
  void EventPush(CUserEvent *);
  void WriteToHistory(const char *);
  void SetHistoryFile(const char *);
  int GetHistory(HistoryList &h)  { return m_fHistory.Load(h); }
  static void ClearHistory(HistoryList &h)  { CUserHistory::Clear(h); }
  void SaveHistory(const char *buf)  { m_fHistory.Save(buf); }
  const char *HistoryName()      { return m_fHistory.Description(); }
  const char *HistoryFile()      { return m_fHistory.FileName(); }

  // Group functions
  unsigned long GetGroups(GroupType g)         { return(m_nGroups[g]); }
  void SetGroups(GroupType g, unsigned long s) { m_nGroups[g] = s; SaveLicqInfo(); }
  bool GetInGroup(GroupType, unsigned short);
  void SetInGroup(GroupType, unsigned short, bool);
  void AddToGroup(GroupType, unsigned short);
  void RemoveFromGroup(GroupType, unsigned short);
  // Short cuts to above functions
  bool InvisibleList() { return GetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST); }
  bool VisibleList()   { return GetInGroup(GROUPS_SYSTEM, GROUP_VISIBLE_LIST); }
  bool OnlineNotify()  { return GetInGroup(GROUPS_SYSTEM, GROUP_ONLINE_NOTIFY); }
  bool IgnoreList()    { return GetInGroup(GROUPS_SYSTEM, GROUP_IGNORE_LIST); }
  bool NewUser()       { return GetInGroup(GROUPS_SYSTEM, GROUP_NEW_USERS); }
  void SetInvisibleList(bool s)  { SetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST, s); }
  void SetVisibleList(bool s)    { SetInGroup(GROUPS_SYSTEM, GROUP_VISIBLE_LIST, s); }
  void SetOnlineNotify(bool s)   { SetInGroup(GROUPS_SYSTEM, GROUP_ONLINE_NOTIFY, s); }
  void SetIgnoreList(bool s)     { SetInGroup(GROUPS_SYSTEM, GROUP_IGNORE_LIST, s); }
  void SetNewUser(bool s)        { SetInGroup(GROUPS_SYSTEM, GROUP_NEW_USERS, s); }

  // Time
  time_t LocalTime();
  int LocalTimeGMTOffset();
  int LocalTimeOffset();
  static int SystemTimeGMTOffset();
  static char SystemTimezone();

  // Ip/Port functions
  unsigned long Ip()		{ return m_nIp; }
  unsigned long IntIp()		{ return m_nIntIp; }
  unsigned short Port()		{ return m_nPort; }
  unsigned short LocalPort()	{ return m_nLocalPort; }
  unsigned long Cookie()	{ return m_nCookie; }
  void SetIpPort(unsigned long nIp, unsigned short nPort);
  void SetIp(unsigned long nIp)		{ SetIpPort(nIp, Port()); }
  void SetPort(unsigned short nPort)	{ SetIpPort(Ip(), nPort); }
  void SetIntIp(unsigned long s)	{ m_nIntIp = s; }
  void SetCookie(unsigned long nCookie) { m_nCookie = nCookie; }

  bool SendIntIp()		{ return m_bSendIntIp; }
  void SetSendIntIp(bool s)	{ m_bSendIntIp = s; }

  // for backward compatibility
  unsigned long RealIp()         	{ return IntIp(); }
  void SetRealIp(unsigned long s) 	{ SetIntIp(s); }
  bool SendRealIp()			{ return SendIntIp(); }
  void SetSendRealIp(bool s)		{ SetSendIntIp(s); }
  
  char *IpStr(char *rbuf);
  char *IntIpStr(char *rbuf);
  char *PortStr(char *rbuf);
  
  // Don't call these:
  int SocketDesc(unsigned char);
  void ClearSocketDesc();
  void ClearSocketDesc(unsigned char);
  void SetSocketDesc(TCPSocket *);

  // Events functions
  static unsigned short getNumUserEvents();
  static void incNumUserEvents();
  static void decNumUserEvents();

  // Last event functions
  time_t Touched()       {  return m_nTouched; }
  void Touch()           {  m_nTouched = time(NULL); }

  // Crypto
  bool Secure() { return m_bSecure; }
    
  virtual bool User()  { return true; }
  void Lock(unsigned short);
  void Unlock();

protected:
  ICQUser() { /* ICQOwner inherited constructor - does nothing */ }
  void LoadGeneralInfo();
  void LoadMoreInfo();
  void LoadHomepageInfo();
  void LoadWorkInfo();
  void LoadAboutInfo();
  void LoadInterestsInfo();
  void LoadBackgroundsInfo();
  void LoadOrganizationsInfo();
  void LoadPhoneBookInfo();
  void LoadPictureInfo();
  void LoadLicqInfo();
  void Init(unsigned long nUin);
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

  CIniFile m_fConf;
  CUserHistory m_fHistory;
  int m_nNormalSocketDesc, m_nInfoSocketDesc, m_nStatusSocketDesc;
  time_t m_nTouched;
  time_t m_nLastCounters[4];
  time_t m_nOnlineSince;
  time_t m_nIdleSince;
  bool m_bOnContactList;
  unsigned long m_nIp, m_nIntIp, m_nVersion, m_nClientTimestamp, m_nCookie;
  unsigned long m_nClientInfoTimestamp, m_nClientStatusTimestamp;
  unsigned long m_nOurClientTimestamp, m_nOurClientInfoTimestamp;
  unsigned long m_nOurClientStatusTimestamp;
  bool m_bUserUpdated;
  unsigned short m_nPort, m_nLocalPort, m_nConnectionVersion;
  unsigned short m_nTyping;
  unsigned long m_nUin,
                m_nStatus,
                m_nGroups[2];
  unsigned short m_nSequence;
  unsigned long m_nPhoneFollowMeStatus, m_nICQphoneStatus, m_nSharedFilesStatus;
  char m_nMode;
  char *m_szClientInfo;
  char *m_szAutoResponse;
  char *m_szEncoding;
  char *m_szCustomAutoResponse;
  char *m_szId;
  char m_szUinString[13];
  bool m_bOnlineNotify,
       m_bSendIntIp,
       m_bSendServer,
       m_bEnableSave,
       m_bShowAwayMsg,
       m_bOfflineOnDisconnect,
       m_bConnectionInProgress,
       m_bSecure;
  unsigned short m_nStatusToUser, m_nSendLevel;
  bool m_bKeepAliasOnUpdate;
  unsigned short m_nAutoAccept;

  // General Info
  char *m_szAlias;
  char *m_szFirstName;
  char *m_szLastName;
  char *m_szEmailPrimary;
  char *m_szEmailSecondary;
  char *m_szEmailOld;
  char *m_szCity;
  char *m_szState;
  char *m_szPhoneNumber;
  char *m_szFaxNumber;
  char *m_szAddress;
  char *m_szCellularNumber;
  char *m_szZipCode;
  unsigned short m_nCountryCode;
  char m_nTimezone;
  bool m_bAuthorization;
  bool m_bHideEmail;
  unsigned char m_nWebAwareStatus;
  
  // More Info
  unsigned short m_nAge;
  char m_nGender;
  char *m_szHomepage;
  bool m_bHomepageCatPresent;
  unsigned short m_nHomepageCatCode;
  char *m_szHomepageDesc;
  bool m_bICQHomepagePresent;
  unsigned short m_nBirthYear;
  char m_nBirthMonth;
  char m_nBirthDay;
  char m_nLanguage[3];

  // Work Info
  char *m_szCompanyCity;
  char *m_szCompanyState;
  char *m_szCompanyPhoneNumber;
  char *m_szCompanyFaxNumber;
  char *m_szCompanyAddress;
  char *m_szCompanyZip;
  unsigned short m_nCompanyCountry;
  char *m_szCompanyName;
  char *m_szCompanyDepartment;
  char *m_szCompanyPosition;
  unsigned short m_nCompanyOccupation;
  char *m_szCompanyHomepage;

  // About Info
  char *m_szAbout;
  
  // More2 Info
  ICQUserCategory *m_Interests;
  ICQUserCategory *m_Backgrounds;
  ICQUserCategory *m_Organizations;

  // Phone Book Info
  ICQUserPhoneBook *m_PhoneBook;

  // Picture Info
  bool m_bPicturePresent;

  // Protocol ID
  unsigned long m_nPPID;

  // Server Side ID, Group SID
  bool m_bAwaitingAuth;
  unsigned short m_nSID[3];
  unsigned short m_nGSID;

  UserEventList m_vcMessages;

  static unsigned short s_nNumUserEvents;

  pthread_rdwr_t mutex_rw;
  unsigned short m_nLockType;
  static pthread_mutex_t mutex_nNumUserEvents;

  friend class CUserGroup;
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
  ICQOwner();
  ICQOwner(const char *, unsigned long);
  virtual ~ICQOwner();
  bool Exception()  { return m_bException; }

  // Owner specific functions
  char *Password() { return(m_szPassword); }
  void SetPassword(const char *s) { SetString(&m_szPassword, s); SaveLicqInfo(); }
  void SetWebAware(bool b)     {  m_bWebAware = b; SaveLicqInfo(); }
  virtual void SetWebAwareStatus(unsigned char c) { SetWebAware(c); }
  void SetHideIp(bool b)       {  m_bHideIp = b; SaveLicqInfo(); }
  void SetSavePassword(bool b) {  m_bSavePassword = b; SaveLicqInfo(); }
  void SetUin(unsigned long n) { m_nUin = n; SaveLicqInfo(); }
  void SetId(const char *s)    { SetString(&m_szId, s); SaveLicqInfo(); }
  void SetRandomChatGroup(unsigned long n)  { m_nRandomChatGroup = n; SaveLicqInfo(); }
  bool WebAware()             { return m_bWebAware; }
  bool HideIp()               { return m_bHideIp; }
  bool SavePassword()         { return m_bSavePassword; }
  unsigned long RandomChatGroup() { return m_nRandomChatGroup; }
  unsigned long AddStatusFlags(unsigned long nStatus);

  // Server Side List functions
  time_t GetSSTime()                  { return m_nSSTime; }
  void SetSSTime(time_t t)            { m_nSSTime = t; }
  unsigned short GetSSCount()         { return m_nSSCount; }
  void SetSSCount(unsigned short n)   { m_nSSCount = n; }
  unsigned short GetPDINFO()          { return m_nPDINFO; }
  void SetPDINFO(unsigned short n)    { m_nPDINFO = n; }

  void SetPicture(const char *f);
  
  // Virtual overloaded functions
  virtual void SaveLicqInfo();
  virtual void SetStatusOffline();

  virtual bool User()  { return false; }
protected:
  char *m_szPassword;
  bool m_bException,
       m_bWebAware,
       m_bHideIp,
       m_bSavePassword;
  unsigned long  m_nRandomChatGroup;
  unsigned short m_nSSCount;
  time_t unsigned long m_nSSTime;
  unsigned short m_nPDINFO;
};


//=====CUsers===================================================================

class CUserHashTable
{
public:
  CUserHashTable(unsigned short _nSize);
  
  // For protocol plugins
  ICQUser *Retrieve(const char *, unsigned long);
	void Store(ICQUser *, const char *, unsigned long);
  void Remove(const char *, unsigned long);

  // For ICQ (Licq way)
  ICQUser *Retrieve(unsigned long _nUin);
  void Store(ICQUser *u, unsigned long _nUin);
  void Remove(unsigned long _nUin);
protected:
	// For protocol plugin
  unsigned short HashValue(const char *);

  // For ICQ (Licq way)
  unsigned short HashValue(unsigned long _nUin);
  void Lock(unsigned short _nLockType);
  void Unlock();

  std::vector < UserList > m_vlTable;

  pthread_rdwr_t mutex_rw;
  unsigned short m_nLockType;
};

class CUserManager
{
public:
  CUserManager();
  ~CUserManager();
  bool Load();
  void SetOwnerUin(unsigned long _nUin);
  
  // For protocol plugins
  void AddOwner(const char *, unsigned long);
  void AddUser(ICQUser *, const char *, unsigned long);
  void RemoveUser(const char *, unsigned long);
  void RemoveOwner(unsigned long);
  ICQUser *FetchUser(const char *, unsigned long, unsigned short);
  ICQOwner *FetchOwner(unsigned long, unsigned short);
  void DropOwner(unsigned long);
  bool IsOnList(const char *, unsigned long);
  ICQOwner *FindOwner(const char *, unsigned long);

  // ICQ Protocol only (from original Licq)
  unsigned long AddUser(ICQUser *);
  void RemoveUser(unsigned long);
  ICQUser *FetchUser(unsigned long, unsigned short);
  void DropUser(ICQUser *);
  ICQOwner *FetchOwner(unsigned short);
  void DropOwner();
  unsigned long OwnerUin()  {return m_nOwnerUin; }
  bool IsOnList(unsigned long nUin);

  UserList *LockUserList(unsigned short);
  void UnlockUserList();
  GroupList *LockGroupList(unsigned short);
  void UnlockGroupList();
  GroupIDList *LockGroupIDList(unsigned short);
  void UnlockGroupIDList();
  OwnerList *LockOwnerList(unsigned short);
  void UnlockOwnerList();

  bool AddGroup(char *, unsigned short = 0);
  void RemoveGroup(unsigned short);
  void RenameGroup(unsigned short, const char *, bool = true);
  unsigned short NumGroups();
  void SaveGroups();
  void SwapGroups(unsigned short g1, unsigned short g2);

  void AddGroupID(unsigned short);
  void RemoveGroupID(unsigned short);
  void ModifyGroupID(char *, unsigned short);
  void SaveGroupIDs();
	unsigned short GetIDFromGroup(const char *);
  unsigned short GetGroupFromID(unsigned short);

  unsigned short GenerateSID();

  void AddUserToGroup(unsigned long _nUin, unsigned short _nGroup);
  void RemoveUserFromGroup(unsigned long _nUin, unsigned short _nGroup);
  void AddUserToGroup(const char *, unsigned long, unsigned short);
  void RemoveUserFromGroup(const char *, unsigned long, unsigned short);
  void SaveAllUsers();

  unsigned short NumUsers();
  unsigned short NumOwners();
  unsigned short DefaultGroup()  { return m_nDefaultGroup; }
  void SetDefaultGroup(unsigned short n)  { m_nDefaultGroup = n; SaveGroups(); }
  unsigned short NewUserGroup()  { return m_nNewUserGroup; }
  void SetNewUserGroup(unsigned short n)  { m_nNewUserGroup = n; SaveGroups(); }

protected:
  pthread_rdwr_t mutex_grouplist, mutex_userlist, mutex_groupidlist, mutex_ownerlist;

  GroupList m_vszGroups;
  UserList m_vpcUsers;
  OwnerList m_vpcOwners;
  GroupIDList m_vnGroupsID;
  CUserHashTable m_hUsers;
  ICQOwner *m_xOwner;
  unsigned long m_nOwnerUin;
  unsigned short m_nDefaultGroup, m_nNewUserGroup,
                 m_nUserListLockType, m_nGroupListLockType,
                 m_nGroupIDListLockType, m_nOwnerListLockType;
  bool m_bAllowSave;

  friend class CICQDaemon;
};


extern class CUserManager gUserManager;

#endif
