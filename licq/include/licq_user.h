#ifndef USER_H
#define USER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector.h>
#include <list.h>
#include <time.h>

#include "pthread_rdwr.h"

#include "licq_message.h"
#include "licq_history.h"
#include "licq_file.h"
#include "licq_icq.h"


class TCPSocket;


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

#define FOR_EACH_USER_END                \
      }                                  \
      pUser->Unlock();                   \
    }                                    \
    gUserManager.UnlockUserList();       \
  }

#define FOR_EACH_USER_BREAK              \
        {                                \
          gUserManager.DropUser(pUser);  \
          break;                         \
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

#define FOR_EACH_UIN_END                 \
      }                                  \
    }                                    \
    gUserManager.UnlockUserList();       \
  }

#define FOR_EACH_UIN_BREAK               \
        {                                \
          break;                         \
        }

#define FOR_EACH_UIN_CONTINUE            \
        {                                \
          continue;                      \
        }

typedef list<ICQUser *> UserList;
typedef vector<char *> GroupList;
typedef list<unsigned long> UinList;
typedef vector <class CUserEvent *> UserEventList;

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

const unsigned short ACCEPT_IN_AWAY     = 0x0001;
const unsigned short ACCEPT_IN_NA       = 0x0002;
const unsigned short ACCEPT_IN_OCCUPIED = 0x0004;
const unsigned short ACCEPT_IN_DND      = 0x0008;
const unsigned short AUTO_ACCEPT_CHAT   = 0x0100;
const unsigned short AUTO_ACCEPT_FILE   = 0x0200;
const unsigned short AUTO_SECURE        = 0x0400;

const unsigned short USPRINTF_NTORN     = 1;
const unsigned short USPRINTF_NOFW      = 2;

const unsigned short LAST_ONLINE        = 0;
const unsigned short LAST_RECV_EVENT    = 1;
const unsigned short LAST_SENT_EVENT    = 2;
const unsigned short LAST_CHECKED_AR    = 3;


//+++++OBJECTS++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//=====ICQUser==================================================================
class ICQUser
{
public:
  ICQUser(unsigned long id, char *filename);
  ICQUser(unsigned long id);
  virtual ~ICQUser();
  void RemoveFiles();

  virtual void SaveLicqInfo();
  void SaveGeneralInfo();
  void SaveMoreInfo();
  void SaveWorkInfo();
  void SaveAboutInfo();
  void SaveBasicInfo();
  void SaveExtInfo();
  void SaveNewMessagesInfo();

  // General Info
  char *GetAlias()                      {  return m_szAlias;  }
  char *GetFirstName()                  {  return m_szFirstName;  }
  char *GetLastName()                   {  return m_szLastName;  }
  char *GetEmailPrimary()               {  return m_szEmailPrimary;  }
  char *GetEmailSecondary()             {  return m_szEmailSecondary;  }
  char *GetEmailOld()                   {  return m_szEmailOld;  }
  char *GetCity()                       {  return m_szCity;  }
  char *GetState()                      {  return m_szState;  }
  char *GetPhoneNumber()                {  return m_szPhoneNumber;  }
  char *GetFaxNumber()                  {  return m_szFaxNumber;  }
  char *GetAddress()                    {  return m_szAddress;  }
  char *GetCellularNumber()             {  return m_szCellularNumber;  }
  char *GetZipCode()                    {  return m_szZipCode;  }
  unsigned short GetCountryCode()       {  return m_nCountryCode;  }
  char GetTimezone()                    {  return m_nTimezone;  }
  bool GetAuthorization()               {  return m_bAuthorization;  }
  bool GetHideEmail()                   {  return m_bHideEmail;  }

  // More Info
  unsigned short GetAge()               {  return m_nAge;  }
  char GetGender()                      {  return m_nGender;  }
  char *GetHomepage()                   {  return m_szHomepage;  }
  unsigned short GetBirthYear()         {  return m_nBirthYear;  }
  char GetBirthMonth()                  {  return m_nBirthMonth;  }
  char GetBirthDay()                    {  return m_nBirthDay;  }
  char GetLanguage1()                   {  return m_nLanguage[0];  }
  char GetLanguage2()                   {  return m_nLanguage[1];  }
  char GetLanguage3()                   {  return m_nLanguage[2];  }
  char GetLanguage(const char l)            {  return m_nLanguage[l];  }

  // Work Info
  char *GetCompanyCity()                {  return m_szCompanyCity;  }
  char *GetCompanyState()               {  return m_szCompanyState;  }
  char *GetCompanyPhoneNumber()         {  return m_szCompanyPhoneNumber; }
  char *GetCompanyFaxNumber()           {  return m_szCompanyFaxNumber;  }
  char *GetCompanyAddress()             {  return m_szCompanyAddress;  }
  char *GetCompanyName()                {  return m_szCompanyName;  }
  char *GetCompanyDepartment()          {  return m_szCompanyDepartment;  }
  char *GetCompanyPosition()            {  return m_szCompanyPosition;  }
  char *GetCompanyHomepage()            {  return m_szCompanyHomepage;  }

  // About Info
  char *GetAbout()                      { return m_szAbout; }

  // Licq Info
  char *AutoResponse()                  { return m_szAutoResponse; }
  bool SendRealIp()			{ return m_bSendRealIp; }
  bool SendServer()                     { return m_bSendServer; }
  unsigned short SendLevel()            { return m_nSendLevel; }
  bool EnableSave()                     { return m_bEnableSave; }
  bool ShowAwayMsg()                    { return m_bShowAwayMsg; }
  unsigned long Uin()                   { return m_nUin; }
  unsigned long Sequence(bool = false);
  unsigned long RealIp()                { return m_nRealIp; }
  char Mode()                           { return m_nMode; }
  unsigned long Version()               { return m_nVersion; }
  unsigned long ClientTimestamp()       { return m_nClientTimestamp; }
  SecureChannelSupport_et SecureChannelSupport();
  unsigned short LicqVersion();
  unsigned short ConnectionVersion();
  time_t LastOnline()                   { return m_nLastCounters[LAST_ONLINE]; }
  time_t LastSentEvent()                { return m_nLastCounters[LAST_SENT_EVENT]; }
  time_t LastReceivedEvent()            { return m_nLastCounters[LAST_RECV_EVENT]; }
  time_t LastCheckedAutoResponse()      { return m_nLastCounters[LAST_CHECKED_AR]; }
  bool AutoChatAccept()                 { return m_nAutoAccept & AUTO_ACCEPT_CHAT; }
  bool AutoFileAccept()                 { return m_nAutoAccept & AUTO_ACCEPT_FILE; }
  bool AutoSecure()                     { return m_nAutoAccept & AUTO_SECURE; }
  bool AcceptInAway()                   { return m_nAutoAccept & ACCEPT_IN_AWAY; }
  bool AcceptInNA()                     { return m_nAutoAccept & ACCEPT_IN_NA; }
  bool AcceptInOccupied()               { return m_nAutoAccept & ACCEPT_IN_OCCUPIED; }
  bool AcceptInDND()                    { return m_nAutoAccept & ACCEPT_IN_DND; }
  unsigned short StatusToUser()         { return m_nStatusToUser; }
  char *CustomAutoResponse()            { return m_szCustomAutoResponse; }

  void usprintf(char *sz, const char *szFormat, unsigned long nFlags = 0);
  char *IpPortStr(char *rbuf);

  // General Info
  void SetAlias (const char *n);//              {  SetString(&m_szAlias, n);  SaveGeneralInfo();  }
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
  void SetLanguage (const char l, const char n) {  m_nLanguage[l] = n;  SaveMoreInfo();  }

  // Work Info
  void SetCompanyCity (const char *n)        {  SetString(&m_szCompanyCity, n);  SaveWorkInfo();  }
  void SetCompanyState (const char *n)       {  SetString(&m_szCompanyState, n);  SaveWorkInfo();  }
  void SetCompanyPhoneNumber (const char *n) {  SetString(&m_szCompanyPhoneNumber, n);  SaveWorkInfo();  }
  void SetCompanyFaxNumber (const char *n)   {  SetString(&m_szCompanyFaxNumber, n);  SaveWorkInfo();  }
  void SetCompanyAddress (const char *n)     {  SetString(&m_szCompanyAddress, n);  SaveWorkInfo();  }
  void SetCompanyName (const char *n)        {  SetString(&m_szCompanyName, n);  SaveWorkInfo();  }
  void SetCompanyDepartment (const char *n)  {  SetString(&m_szCompanyDepartment, n);  SaveWorkInfo();  }
  void SetCompanyPosition (const char *n)    {  SetString(&m_szCompanyPosition, n);  SaveWorkInfo();  }
  void SetCompanyHomepage (const char *n)    {  SetString(&m_szCompanyHomepage, n);  SaveWorkInfo();  }

  // About Info
  void SetAbout(const char *n)        {  SetString(&m_szAbout, n);  SaveAboutInfo();  }

  // Licq Info
  void SetEnableSave(bool s)          { if (m_bOnContactList) m_bEnableSave = s; }
  void SetSendRealIp(bool s)	      { m_bSendRealIp = s; }
  void SetSendServer(bool s)          { m_bSendServer = s; }
  void SetSendLevel(unsigned short s) { m_nSendLevel = s; }
  void SetSequence(unsigned long s)   { m_nSequence = s; }
  void SetAutoResponse(const char *s) { SetString(&m_szAutoResponse, s); }
  void SetShowAwayMsg(bool s)         { m_bShowAwayMsg = s; }
  void SetRealIp(unsigned long s)     { m_nRealIp = s; }
  void SetMode(char s)                { m_nMode = s; }
  void SetVersion(unsigned long s)    { m_nVersion = s; }
  void SetClientTimestamp(unsigned long s) { m_nClientTimestamp = s; }
  void SetConnectionVersion(unsigned short s)    { m_nConnectionVersion = s; }
  void SetAutoChatAccept(bool s)      { s ? m_nAutoAccept |= AUTO_ACCEPT_CHAT : m_nAutoAccept &= ~AUTO_ACCEPT_CHAT; SaveLicqInfo(); }
  void SetAutoFileAccept(bool s)      { s ? m_nAutoAccept |= AUTO_ACCEPT_FILE : m_nAutoAccept &= ~AUTO_ACCEPT_FILE; SaveLicqInfo(); }
  void SetAutoSecure(bool s)          { s ? m_nAutoAccept |= AUTO_SECURE : m_nAutoAccept &= ~AUTO_SECURE; SaveLicqInfo(); }
  void SetAcceptInAway(bool s)        { s ? m_nAutoAccept |= ACCEPT_IN_AWAY : m_nAutoAccept &= ~ACCEPT_IN_AWAY; SaveLicqInfo(); }
  void SetAcceptInNA(bool s)          { s ? m_nAutoAccept |= ACCEPT_IN_NA : m_nAutoAccept &= ~ACCEPT_IN_NA; SaveLicqInfo(); }
  void SetAcceptInOccupied(bool s)    { s ? m_nAutoAccept |= ACCEPT_IN_OCCUPIED : m_nAutoAccept &= ~ACCEPT_IN_OCCUPIED; SaveLicqInfo(); }
  void SetAcceptInDND(bool s)         { s ? m_nAutoAccept |= ACCEPT_IN_DND : m_nAutoAccept &= ~ACCEPT_IN_DND; SaveLicqInfo(); }
  void SetStatusToUser(unsigned short s)    { m_nStatusToUser = s; SaveLicqInfo(); }
  void SetCustomAutoResponse(const char *s) { SetString(&m_szCustomAutoResponse, s); SaveLicqInfo(); }
  void ClearCustomAutoResponse()            { SetCustomAutoResponse(""); }

  // Status
  unsigned short Status();
  unsigned long StatusFull()   {  return m_nStatus; }
  bool StatusInvisible()       {  return StatusOffline() ? false : m_nStatus & ICQ_STATUS_FxPRIVATE; }
  bool StatusWebPresence()     {  return m_nStatus & ICQ_STATUS_FxWEBxPRESENCE;  }
  bool StatusHideIp()          {  return m_nStatus & ICQ_STATUS_FxHIDExIP; }
  bool StatusBirthday()        {  return m_nStatus & ICQ_STATUS_FxBIRTHDAY;  }
  bool StatusOffline()         {  return (unsigned short)m_nStatus == ICQ_STATUS_OFFLINE;  }
  void SetStatus(unsigned long n)  {  m_nStatus = n;  }
  virtual void SetStatusOffline();
  const char *StatusStr();
  const char *StatusStrShort();
  bool Away();
  static const char *StatusToStatusStr(unsigned short, bool);
  static const char *StatusToStatusStrShort(unsigned short, bool);
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
  unsigned long Ip()        { return m_nIp; }
  unsigned short Port()     { return m_nPort; }
  unsigned short LocalPort()     { return m_nLocalPort; }
  void SetIpPort(unsigned long nIp, unsigned short nPort);
  void SetIp(unsigned long nIp) { SetIpPort(nIp, Port()); }
  // Don't call these:
  int SocketDesc()          { return m_nSocketDesc; }
  void ClearSocketDesc();
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
  ICQUser() { /* ICQOwner inherited constructor - does nothing */ };
  void StupidLinkageFix();
  void LoadGeneralInfo();
  void LoadMoreInfo();
  void LoadWorkInfo();
  void LoadAboutInfo();
  void LoadLicqInfo();
  void Init(unsigned long nUin);
  bool LoadInfo();
  void SetDefaults();
  void AddToContactList();

  void SetOfflineOnDisconnect(bool b) { m_bOfflineOnDisconnect = b; }
  bool OfflineOnDisconnect() { return m_bOfflineOnDisconnect; }
  bool ConnectionInProgress() { return m_bConnectionInProgress; }
  void SetConnectionInProgress(bool c)  { m_bConnectionInProgress = c; }
  void SetSecure(bool s) { m_bSecure = s; }

  void SetLastSentEvent()           { m_nLastCounters[LAST_SENT_EVENT] = time(NULL); }
  void SetLastReceivedEvent()       { m_nLastCounters[LAST_RECV_EVENT] = time(NULL); }
  void SetLastCheckedAutoResponse() { m_nLastCounters[LAST_CHECKED_AR] = time(NULL); }

  CIniFile m_fConf;
  CUserHistory m_fHistory;
  int m_nSocketDesc;
  time_t m_nTouched;
  time_t m_nLastCounters[4];
  bool m_bOnContactList;
  unsigned long m_nIp, m_nRealIp, m_nVersion, m_nClientTimestamp;
  unsigned short m_nPort, m_nLocalPort, m_nConnectionVersion;
  unsigned long m_nUin,
                m_nStatus,
                m_nSequence,
                m_nGroups[2];
  char m_nMode;
  char *m_szAutoResponse;
  char *m_szCustomAutoResponse;
  bool m_bOnlineNotify,
       m_bSendRealIp,
       m_bSendServer,
       m_bEnableSave,
       m_bShowAwayMsg,
       m_bOfflineOnDisconnect,
       m_bConnectionInProgress,
       m_bSecure;
  unsigned short m_nStatusToUser, m_nSendLevel;
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

  // More Info
  unsigned short m_nAge;
  char m_nGender;
  char *m_szHomepage;
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
  char *m_szCompanyName;
  char *m_szCompanyDepartment;
  char *m_szCompanyPosition;
  char *m_szCompanyHomepage;

  // About Info
  char *m_szAbout;

  UserEventList m_vcMessages;

  static unsigned short s_nNumUserEvents;

  pthread_rdwr_t mutex_rw;
  unsigned short m_nLockType;
  static pthread_mutex_t mutex_nNumUserEvents;

  friend class CUserGroup;
  friend class CUserManager;
  friend class CICQDaemon;
  friend class CSocketManager;
};


//=====ICQOwner=================================================================
class ICQOwner : public ICQUser
{
public:
  ICQOwner();
  virtual ~ICQOwner();
  bool Exception()  { return m_bException; }

  // Owner specific functions
  char *Password() { return(m_szPassword); }
  void SetPassword(const char *s) { SetString(&m_szPassword, s); SaveLicqInfo(); }
  void SetWebAware(bool b)  {  m_bWebAware = b; SaveLicqInfo(); }
  void SetHideIp(bool b)    {  m_bHideIp = b; SaveLicqInfo(); }
  void SetUin(unsigned long n)    { m_nUin = n; SaveLicqInfo(); }
  void SetRandomChatGroup(unsigned long n)  { m_nRandomChatGroup = n; SaveLicqInfo(); }
  bool WebAware()             { return m_bWebAware; }
  bool HideIp()               { return m_bHideIp; }
  unsigned long RandomChatGroup() { return m_nRandomChatGroup; }
  unsigned long AddStatusFlags(unsigned long nStatus);

  // Virtual overloaded functions
  virtual void SaveLicqInfo();
  virtual void SetStatusOffline();

  virtual bool User()  { return false; }
protected:
  char *m_szPassword;
  bool m_bException,
       m_bWebAware,
       m_bHideIp;
  unsigned long m_nRandomChatGroup;
};


//=====CUsers===================================================================

class CUserHashTable
{
public:
  CUserHashTable(unsigned short _nSize);
  ICQUser *Retrieve(unsigned long _nUin);
  void Store(ICQUser *u, unsigned long _nUin);
  void Remove(unsigned long _nUin);
protected:
  unsigned short HashValue(unsigned long _nUin);
  void Lock(unsigned short _nLockType);
  void Unlock();

  vector < UserList > m_vlTable;

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

  unsigned long AddUser(ICQUser *);
  void RemoveUser(unsigned long);
  ICQUser *FetchUser(unsigned long, unsigned short);
  void DropUser(ICQUser *);
  ICQOwner *FetchOwner(unsigned short);
  void DropOwner();
  unsigned long OwnerUin()  { return m_nOwnerUin; }
  bool IsOnList(unsigned long nUin);

  UserList *LockUserList(unsigned short);
  void UnlockUserList();
  GroupList *LockGroupList(unsigned short);
  void UnlockGroupList();

  void AddGroup(char *);
  void RemoveGroup(unsigned short);
  void RenameGroup(unsigned short, const char *);
  unsigned short NumGroups();
  void SaveGroups();
  void SwapGroups(unsigned short g1, unsigned short g2);

  void AddUserToGroup(unsigned long _nUin, unsigned short _nGroup);
  void RemoveUserFromGroup(unsigned long _nUin, unsigned short _nGroup);
  void SaveAllUsers();

  unsigned short NumUsers();
  unsigned short DefaultGroup()  { return m_nDefaultGroup; }
  void SetDefaultGroup(unsigned short n)  { m_nDefaultGroup = n; SaveGroups(); }
  //unsigned short NewUserGroup()  { return m_nNewUserGroup; }
  //void SetNewUserGroup(unsigned short n)  { m_nNewUserGroup = n; SaveGroups(); }

protected:
  pthread_rdwr_t mutex_grouplist, mutex_userlist;
  GroupList m_vszGroups;
  UserList m_vpcUsers;
  CUserHashTable m_hUsers;
  ICQOwner *m_xOwner;
  unsigned long m_nOwnerUin;
  unsigned short m_nDefaultGroup, //m_nNewUserGroup,
                 m_nUserListLockType, m_nGroupListLockType;
  bool m_bAllowSave;

  friend class CICQDaemon;
};


extern class CUserManager gUserManager;

#endif
