#ifndef USER_H
#define USER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector.h>
#include <list.h>

#include "pthread_rdwr.h"
#include "message.h"
#include "history.h"
#include "userfcndlg.h"
#include "file.h"
#include "icq-defines.h"
#include "support.h"

#define FOR_EACH_USER_START(x)                           \
  {                                                      \
    ICQUser *pUser;                                      \
    UserList *_ul_ = gUserManager.LockUserList(LOCK_R);  \
    for (UserListIter _i_ = _ul_->begin();               \
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

typedef list<ICQUser *> UserList;
typedef list<ICQUser *>::iterator UserListIter;
typedef vector<char *> GroupList;
typedef vector<char *>::iterator GroupListIter;
typedef vector<unsigned long> UinList;


//+++++STRUCTURES+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum EGender
{
  GENDER_UNSPECIFIED = 0,
  GENDER_FEMALE = 1,
  GENDER_MALE = 2
} Gender;
const unsigned short AGE_UNSPECIFIED = 0xFFFF;

enum ESortKey { SORT_STATUS, SORT_ONLINE };
enum GroupType { GROUPS_SYSTEM, GROUPS_USER };

const unsigned long GROUP_ONLINE_NOTIFY   = 1;
const unsigned long GROUP_VISIBLE_LIST    = 2;
const unsigned long GROUP_INVISIBLE_LIST  = 3;
const unsigned long GROUP_IGNORE_LIST     = 4;


//+++++OBJECTS++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//=====ICQUser==================================================================
class ICQUser
{
public:
  ICQUser(unsigned long id, char *filename);
  ICQUser(unsigned long id);
  virtual ~ICQUser(void);
  void RemoveFiles(void);

  CUserFunctionDlg *fcnDlg;

  virtual void SaveLicqInfo(void);
  void SaveGeneralInfo(void);
  void SaveMoreInfo(void);
  void SaveWorkInfo(void);
  void SaveBasicInfo(void);
  void SaveExtInfo(void);

  // General Info
  char *GetAlias(void)                      {  return m_szAlias;  }
  char *GetFirstName(void)                  {  return m_szFirstName;  }
  char *GetLastName(void)                   {  return m_szLastName;  }
  char *GetEmail1(void)                     {  return m_szEmail1;  }
  char *GetEmail2(void)                     {  return m_szEmail2;  }
  char *GetCity(void)                       {  return m_szCity;  }
  char *GetState(void)                      {  return m_szState;  }
  char *GetPhoneNumber(void)                {  return m_szPhoneNumber;  }
  char *GetFaxNumber(void)                  {  return m_szFaxNumber;  }
  char *GetAddress(void)                    {  return m_szAddress;  }
  char *GetCellularNumber(void)             {  return m_szCellularNumber;  }
  unsigned long GetZipCode(void)            {  return m_nZipCode;  }
  unsigned short GetCountryCode(void)       {  return m_nCountryCode;  }
  char GetTimezone(void)                    {  return m_nTimezone;  }
  bool GetAuthorization(void)               {  return m_bAuthorization;  }
  bool GetHideEmail(void)                   {  return m_bHideEmail;  }

  // More Info
  unsigned short GetAge(void)               {  return m_nAge;  }
  char GetGender(void)                      {  return m_nGender;  }
  char *GetHomepage(void)                   {  return m_szHomepage;  }
  char GetBirthYear(void)                   {  return m_nBirthYear;  }
  char GetBirthMonth(void)                  {  return m_nBirthMonth;  }
  char GetBirthDay(void)                    {  return m_nBirthDay;  }
  char GetLanguage1(void)                   {  return m_nLanguage1;  }
  char GetLanguage2(void)                   {  return m_nLanguage2;  }
  char GetLanguage3(void)                   {  return m_nLanguage3;  }

  // Work Info
  char *GetCompanyCity(void)                {  return m_szCompanyCity;  }
  char *GetCompanyState(void)               {  return m_szCompanyState;  }
  char *GetCompanyPhoneNumber(void)         {  return m_szCompanyPhoneNumber; }
  char *GetCompanyFaxNumber(void)           {  return m_szCompanyFaxNumber;  }
  char *GetComparyAddress(void)             {  return m_szComparyAddress;  }
  char *GetCompanyName(void)                {  return m_szCompanyName;  }
  char *GetCompanyDepartment(void)          {  return m_szCompanyDepartment;  }
  char *GetCompanyPosition(void)            {  return m_szCompanyPosition;  }
  char *GetCompanyHomepage(void)            {  return m_szCompanyHomepage;  }

  // Licq Info
  char *AutoResponse(void)                  { return m_szAutoResponse; }
  char *GetAbout(void)                      { return m_szAbout; }
  bool NewUser(void)                        { return m_bNewUser; }
  bool SendServer(void)                     { return m_bSendServer; }
  bool EnableSave(void)                     { return m_bEnableSave; }
  bool ShowAwayMsg(void)                    { return m_bShowAwayMsg; }
  unsigned long Uin(void)                   { return m_nUin; }
  unsigned long Sequence(bool = false);

  unsigned long SortKey(void);
  static void SetSortKey(ESortKey);
  void usprintf(char *_sz, const char *_szFormat, bool _bAllowFieldWidth = true);
  char *IpPortStr(char *rbuf);

  // General Info
  void SetAlias (const char *n);//              {  SetString(&m_szAlias, n);  SaveGeneralInfo();  }
  void SetFirstName (const char *n)          {  SetString(&m_szFirstName, n);  SaveGeneralInfo();  }
  void SetLastName (const char *n)           {  SetString(&m_szLastName, n);  SaveGeneralInfo();  }
  void SetEmail1 (const char *n)             {  SetString(&m_szEmail1, n);  SaveGeneralInfo();  }
  void SetEmail2 (const char *n)             {  SetString(&m_szEmail2, n);  SaveGeneralInfo();  }
  void SetCity (const char *n)               {  SetString(&m_szCity, n);  SaveGeneralInfo();  }
  void SetState (const char *n)              {  SetString(&m_szState, n);  SaveGeneralInfo();  }
  void SetPhoneNumber (const char *n)        {  SetString(&m_szPhoneNumber, n);  SaveGeneralInfo();  }
  void SetFaxNumber (const char *n)          {  SetString(&m_szFaxNumber, n);  SaveGeneralInfo();  }
  void SetAddress (const char *n)            {  SetString(&m_szAddress, n);  SaveGeneralInfo();  }
  void SetCellularNumber (const char *n)     {  SetString(&m_szCellularNumber, n);  SaveGeneralInfo();  }
  void SetZipCode (unsigned long n)          {  m_nZipCode = n;  SaveGeneralInfo();  }
  void SetCountryCode (unsigned short n)     {  m_nCountryCode = n;  SaveGeneralInfo();  }
  void SetTimezone (const char n)            {  m_nTimezone = n;  SaveGeneralInfo();  }
  void SetAuthorization (bool n)             {  m_bAuthorization = n;  SaveGeneralInfo();  }
  void SetHideEmail (bool n)                 {  m_bHideEmail = n;  SaveGeneralInfo();  }

  // More Info
  void SetAge (unsigned short n)             {  m_nAge = n;  SaveMoreInfo();  }
  void SetGender (const char n)              {  m_nGender = n;  SaveMoreInfo();  }
  void SetHomepage (const char *n)           {  SetString(&m_szHomepage, n);  SaveMoreInfo();  }
  void SetBirthYear (const char n)           {  m_nBirthYear = n;  SaveMoreInfo();  }
  void SetBirthMonth (const char n)          {  m_nBirthMonth = n;  SaveMoreInfo();  }
  void SetBirthDay (const char n)            {  m_nBirthDay = n;  SaveMoreInfo();  }
  void SetLanguage1 (const char n)           {  m_nLanguage1 = n;  SaveMoreInfo();  }
  void SetLanguage2 (const char n)           {  m_nLanguage2 = n;  SaveMoreInfo();  }
  void SetLanguage3 (const char n)           {  m_nLanguage3 = n;  SaveMoreInfo();  }

  // Work Info
  void SetCompanyCity (const char *n)        {  SetString(&m_szCompanyCity, n);  SaveWorkInfo();  }
  void SetCompanyState (const char *n)       {  SetString(&m_szCompanyState, n);  SaveWorkInfo();  }
  void SetCompanyPhoneNumber (const char *n) {  SetString(&m_szCompanyPhoneNumber, n);  SaveWorkInfo();  }
  void SetCompanyFaxNumber (const char *n)   {  SetString(&m_szCompanyFaxNumber, n);  SaveWorkInfo();  }
  void SetComparyAddress (const char *n)     {  SetString(&m_szComparyAddress, n);  SaveWorkInfo();  }
  void SetCompanyName (const char *n)        {  SetString(&m_szCompanyName, n);  SaveWorkInfo();  }
  void SetCompanyDepartment (const char *n)  {  SetString(&m_szCompanyDepartment, n);  SaveWorkInfo();  }
  void SetCompanyPosition (const char *n)    {  SetString(&m_szCompanyPosition, n);  SaveWorkInfo();  }
  void SetCompanyHomepage (const char *n)    {  SetString(&m_szCompanyHomepage, n);  SaveWorkInfo();  }

  // Licq Info
  void SetEnableSave(bool s)          { m_bEnableSave = s; }
  void SetSendServer(bool s)          { m_bSendServer = s; }
  void SetSequence(unsigned long s)   { m_nSequence = s; }
  void SetNewUser(bool s)             { m_bNewUser = s; SaveLicqInfo(); }
  void SetAutoResponse(const char *s) { SetString(&m_szAutoResponse, s); }
  void SetShowAwayMsg(bool s)         { m_bShowAwayMsg = s; }
  void SetAbout(const char *n)        {  SetString(&m_szAbout, n);  SaveLicqInfo();  }

  // Status
  unsigned short Status(void);
  unsigned long StatusFull(void)   {  return m_nStatus; }
  unsigned long StatusFlags(void)  {  return m_nStatus & ICQ_STATUS_FxFLAGS; }
  bool StatusInvisible(void)       {  return StatusOffline() ? false : m_nStatus & ICQ_STATUS_FxPRIVATE; }
  bool StatusWebPresence(void)     {  return m_nStatus & ICQ_STATUS_FxWEBxPRESENCE;  }
  bool StatusHideIp(void)          {  return m_nStatus & ICQ_STATUS_FxHIDExIP; }
  bool StatusBirthday(void)        {  return m_nStatus & ICQ_STATUS_FxBIRTHDAY;  }
  bool StatusOffline(void)         {  return (unsigned short)m_nStatus == ICQ_STATUS_OFFLINE;  }
  void SetStatus(unsigned long n)  {  m_nStatus = n;  }
  void SetStatusOffline(void)           { SetStatus(m_nStatus | ICQ_STATUS_OFFLINE); };
  void SetStatusFlag(unsigned long s)   { SetStatus(m_nStatus | s); }
  void ClearStatusFlag(unsigned long s) { SetStatus(m_nStatus & ~s); }
  char *StatusStr(char *);
  char *StatusStrShort(char *);
  bool Away(void);
  static char *StatusToStatusStr(unsigned short, bool, char *);
  static char *StatusToStatusStrShort(unsigned short, bool, char *);

  // Message/History functions
  unsigned short NewMessages(void)   { return(m_vcMessages.size()); }
  CUserEvent *GetEvent(unsigned short);
  void ClearEvent(unsigned short);
  void AddEvent(CUserEvent *);
  void WriteToHistory(const char *);
  void SetHistoryFile(const char *);
  int GetHistory(HistoryList &h)  { return m_fHistory.Load(h); }
  void SaveHistory(const char *buf)  { m_fHistory.Save(buf); }
  const char *HistoryName(void)      { return m_fHistory.Description(); }

  // Group functions
  unsigned long GetGroups(GroupType g)         { return(m_nGroups[g]); }
  void SetGroups(GroupType g, unsigned long s) { m_nGroups[g] = s; SaveLicqInfo(); }
  bool GetInGroup(GroupType, unsigned short);
  void SetInGroup(GroupType, unsigned short, bool);
  void AddToGroup(GroupType, unsigned short);
  void RemoveFromGroup(GroupType, unsigned short);
  // Short cuts to above functions
  bool InvisibleList(void) { return GetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST); }
  bool VisibleList(void)   { return GetInGroup(GROUPS_SYSTEM, GROUP_VISIBLE_LIST); }
  bool OnlineNotify(void)  { return GetInGroup(GROUPS_SYSTEM, GROUP_ONLINE_NOTIFY); }
  bool IgnoreList(void)    { return GetInGroup(GROUPS_SYSTEM, GROUP_IGNORE_LIST); }
  void SetInvisibleList(bool s)  { SetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST, s); }
  void SetVisibleList(bool s)    { SetInGroup(GROUPS_SYSTEM, GROUP_VISIBLE_LIST, s); }
  void SetOnlineNotify(bool s)   { SetInGroup(GROUPS_SYSTEM, GROUP_ONLINE_NOTIFY, s); }
  void SetIgnoreList(bool s)     { SetInGroup(GROUPS_SYSTEM, GROUP_IGNORE_LIST, s); }

  // Socket functions
  int SocketDesc(void)          { return m_nSocketDesc; }
  void ClearSocketDesc(void)    { SetSocketDesc(-1); }
  void SetSocketDesc(int _nSd)  { m_nSocketDesc = _nSd; }
  unsigned long Ip(void)        { return m_nIp; }
  unsigned short Port(void)     { return m_nPort; }
  void SetIpPort(unsigned long _nIp, unsigned short _nPort);

  // Events functions
  static unsigned short getNumUserEvents(void);
  static void incNumUserEvents(void);
  static void decNumUserEvents(void);

  virtual bool User(void)  { return true; }
  void Lock(unsigned short);
  void Unlock(void);

protected:
  ICQUser(void) { /* ICQOwner inherited constructor - does nothing */ };
  void StupidLinkageFix(void);
  void LoadGeneralInfo(void);
  void LoadMoreInfo(void);
  void LoadWorkInfo(void);
  void LoadLicqInfo(void);
  void Init(unsigned long _nUin);
  bool LoadInfo(void);
  void SetDefaults(void);

  CIniFile m_fConf;
  CUserHistory m_fHistory;
  int m_nSocketDesc;
  unsigned long m_nIp;
  unsigned short m_nPort;
  unsigned long m_nUin,
                m_nStatus,
                m_nSequence,
                m_nGroups[2];
  char *m_szAutoResponse,
       *m_szAbout;
  bool m_bOnline,
       m_bNewUser,
       m_bOnlineNotify,
       m_bSendServer,
       m_bVisibleList,
       m_bInvisibleList,
       m_bEnableSave,
       m_bShowAwayMsg;

  // General Info
  char *m_szAlias;
  char *m_szFirstName;
  char *m_szLastName;
  char *m_szEmail1;
  char *m_szEmail2;
  char *m_szCity;
  char *m_szState;
  char *m_szPhoneNumber;
  char *m_szFaxNumber;
  char *m_szAddress;
  char *m_szCellularNumber;
  unsigned long m_nZipCode;
  unsigned short m_nCountryCode;
  char m_nTimezone;
  bool m_bAuthorization;
  bool m_bHideEmail;

  // More Info
  unsigned short m_nAge;
  char m_nGender;
  char *m_szHomepage;
  char m_nBirthYear;
  char m_nBirthMonth;
  char m_nBirthDay;
  char m_nLanguage1;
  char m_nLanguage2;
  char m_nLanguage3;

  // Work Info
  char *m_szCompanyCity;
  char *m_szCompanyState;
  char *m_szCompanyPhoneNumber;
  char *m_szCompanyFaxNumber;
  char *m_szComparyAddress;
  char *m_szCompanyName;
  char *m_szCompanyDepartment;
  char *m_szCompanyPosition;
  char *m_szCompanyHomepage;

  vector <class CUserEvent *> m_vcMessages;

  static unsigned short s_nNumUserEvents;
  static ESortKey s_eSortKey;

  pthread_rdwr_t mutex_rw;
  unsigned short m_nLockType;
  static pthread_mutex_t mutex_nNumUserEvents;
  static pthread_mutex_t mutex_sortkey;

  friend class CUserGroup;
  friend class CUserManager;
};


//=====ICQOwner=================================================================
class ICQOwner : public ICQUser
{
public:
  ICQOwner(void);
  bool Exception(void)  { return m_bException; }

  // Owner specific functions
  char *Password(void) { return(m_szPassword); }
  void SetPassword(const char *s) { SetString(&m_szPassword, s); SaveLicqInfo(); }
  void SetUin(unsigned long n)    { m_nUin = n; SaveLicqInfo(); }

  // Virtual overloaded functions
  virtual void SaveLicqInfo(void);

  virtual bool User(void)  { return false; }
protected:
  char *m_szPassword;
  bool m_bException;
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
  void Unlock(void);

  vector < UserList > m_vlTable;

  pthread_rdwr_t mutex_rw;
  unsigned short m_nLockType;
};


class CUserManager
{
public:
  CUserManager(void);
  ~CUserManager(void) {};
  bool Load(void);

  unsigned long AddUser(ICQUser *);
  void RemoveUser(unsigned long);
  ICQUser *FetchUser(unsigned long, unsigned short);
  void DropUser(ICQUser *);
  ICQOwner *FetchOwner(unsigned short);
  void DropOwner(void);
  unsigned long OwnerUin(void)  { return m_nOwnerUin; }
  void SetOwnerUin(unsigned long _nUin);

  UserList *LockUserList(unsigned short);
  void UnlockUserList(void);
  GroupList *LockGroupList(unsigned short);
  void UnlockGroupList(void);

  void AddGroup(char *);
  void RemoveGroup(unsigned short);
  void RenameGroup(unsigned short, const char *);
  unsigned short NumGroups();
  void SaveGroups(void);
  void SwapGroups(unsigned short g1, unsigned short g2);

  void AddUserToGroup(unsigned long _nUin, unsigned short _nGroup);
  void RemoveUserFromGroup(unsigned long _nUin, unsigned short _nGroup);
  void Reorder(ICQUser *_pcUser, bool _bOnList = true);
  void SaveAllUsers(void);

  unsigned short NumUsers(void);
  unsigned short DefaultGroup(void)  { return m_nDefaultGroup; }
  void SetDefaultGroup(unsigned short n)  { m_nDefaultGroup = n; SaveGroups(); }
  unsigned short NewUserGroup(void)  { return m_nNewUserGroup; }
  void SetNewUserGroup(unsigned short n)  { m_nNewUserGroup = n; SaveGroups(); }

protected:
  pthread_rdwr_t mutex_grouplist, mutex_userlist;
  GroupList m_vszGroups;
  UserList m_vpcUsers;
  CUserHashTable m_hUsers;
  ICQOwner *m_xOwner;
  unsigned long m_nOwnerUin;
  unsigned short m_nDefaultGroup, m_nNewUserGroup,
                 m_nUserListLockType, m_nGroupListLockType;
  bool m_bAllowSave;
};


extern class CUserManager gUserManager;

#endif
