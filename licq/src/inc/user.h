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

//-----UserBasicInfo------------------------------------------------------------
struct UserBasicInfo
{
     char alias[32];
     char uin[16];
     char name[64];
     char firstname[32];
     char lastname[32];
     char email[64];
     char status[16];
     char ip[16];
     char port[6];
     char ip_port[24];
     char history[MAX_FILENAME_LEN + 32];
     char awayMessage[MAX_MESSAGE_SIZE];
};

//-----UserExtInfo--------------------------------------------------------------
struct UserExtInfo
{
   char city[64];
   char state[32];
   char country[64];
   char timezone[4];
   char zipcode[8];
   char phone[32];
   char age[16];
   char sex[10];
   char homepage[256];
   char about[MAX_MESSAGE_SIZE];
};

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

  // user info functions
  virtual void getBasicInfo(struct UserBasicInfo &us);
  virtual void getExtInfo(struct UserExtInfo &ud);
  virtual void saveInfo(void);
  void saveBasicInfo(void);
  void saveExtInfo(void);
  bool isAway(void);

  // Accessors
  char *getAlias(void)         { return(m_sAlias); }
  char *getLastName(void)      { return(m_sLastName); }
  char *getFirstName(void)     { return(m_sFirstName); }
  char *getEmail(void)         { return(m_sEmail); }
  char *AutoResponse(void)     { return(m_szAutoResponse); }
  char *getCity(void)          { return(m_sCity); }
  char *getState(void)         { return(m_sState); }
  char *getPhoneNumber(void)   { return(m_sPhoneNumber); }
  char *getAbout(void)         { return(m_sAbout); }
  char *getHomepage(void)      { return(m_sHomepage); }
  bool getIsNew(void)         { return(m_bIsNew); }
  bool getSendServer(void)    { return(m_bSendServer); }
  bool getEnableSave(void)    { return(m_bEnableSave); }
  bool getAuthorization(void)  { return m_bAuthorization; }
  bool ShowAwayMsg(void)      { return m_bShowAwayMsg; }
  unsigned long getUin(void)           { return(m_nUin); }
  unsigned long getStatusFull(void)  { return m_nStatus; }
  unsigned long getStatusFlags(void)  { return m_nStatus & ICQ_STATUS_FxFLAGS; }
  unsigned long getZipcode(void)           { return(m_nZipcode); }
  unsigned short getCountryCode(void)  { return m_nCountryCode; }
  unsigned short getAge(void)           { return(m_nAge); }
  unsigned short getSexNum(void)        { return(m_nSex); }
  unsigned short getStatus(void);
  signed short getTimezone(void)    { return m_nTimezone; }
  bool getStatusInvisible(void);
  bool getStatusWebPresence(void);
  bool getStatusHideIp(void);
  bool getStatusBirthday(void);
  bool getStatusOffline(void);
  unsigned long getSequence(bool = false);
  char *getSex(char *);
  char *getCountry(char *);
  void getStatusStr(char *);
  static void StatusStr(unsigned short, bool, char *);

  int GetHistory(HistoryList &h)  { return m_fHistory.Load(h); }
  void SaveHistory(const char *buf)  { m_fHistory.Save(buf); }
  unsigned long SortKey(void);
  static void SetSortKey(ESortKey);
  void usprintf(char *_sz, const char *_szFormat, bool _bAllowFieldWidth = true);

   // Settors
  void setEnableSave(bool s)          { m_bEnableSave = s; }
  void setSendServer(bool s)          { m_bSendServer = s; }
  void setSequence(unsigned long s)   { m_nSequence = s; }

  void setIsNew(bool s)               { m_bIsNew = s; saveInfo(); }

  void setAuthorization(bool s)       { m_bAuthorization = s; saveBasicInfo(); }
  void SetAutoResponse(const char *s) { SetString(&m_szAutoResponse, s); }
  void setUin(unsigned long s)        { m_nUin = s; }
  void setStatusOffline(void)         { setStatus(m_nStatus | ICQ_STATUS_OFFLINE); };
  void setStatusFlag(unsigned long s) { setStatus(m_nStatus | s); }
  void clearStatusFlag(unsigned long s) { setStatus(m_nStatus & ~s); }
  void setAlias(const char *s);
  void setEmail(const char *s)        { SetString(&m_sEmail, s); saveBasicInfo(); }
  void setFirstName(const char *s)    { SetString(&m_sFirstName, s); saveBasicInfo(); }
  void setLastName(const char *s)     { SetString(&m_sLastName, s); saveBasicInfo(); }
  void setAge(unsigned short s)       { m_nAge = s; saveExtInfo(); }
  void setCity(const char *s)         { SetString(&m_sCity, s); saveExtInfo(); }
  void setState(const char *s)        { SetString(&m_sState, s); saveExtInfo(); }
  void setPhoneNumber(const char *s)  { SetString(&m_sPhoneNumber, s); saveExtInfo(); }
  void setHomepage(const char *s)     { SetString(&m_sHomepage, s); saveExtInfo(); }
  void setAbout(const char *s)        { SetString(&m_sAbout, s); saveExtInfo(); }
  void setSex(unsigned short s)       { m_nSex = s; saveExtInfo(); }
  void setCountry(unsigned short s)   { m_nCountryCode = s; saveExtInfo(); }
  void setTimezone(signed short s)    { m_nTimezone = s; saveExtInfo(); }
  void setShowAwayMsg(bool s)         { m_bShowAwayMsg = s; }
  void setZipcode(unsigned long s)    { m_nZipcode = s; saveExtInfo(); }

  void setStatus(unsigned long);
  void setHistoryFile(const char *);

  // Message functions
  unsigned short getNumMessages(void)   { return(m_vcMessages.size()); }
  CUserEvent *GetEvent(unsigned short);
  void ClearEvent(unsigned short);
  void AddEvent(CUserEvent *);
  void WriteToHistory(const char *);

  // Group functions
  unsigned long GetGroups(GroupType g)         { return(m_nGroups[g]); }
  void SetGroups(GroupType g, unsigned long s) { m_nGroups[g] = s; saveInfo(); }
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
  bool LoadData(void);
  void Init(unsigned long _nUin);
  void SetDefaults(void);

  CIniFile m_fConf;
  CUserHistory m_fHistory;
  int m_nSocketDesc;
  unsigned long m_nIp;
  unsigned short m_nPort;
  unsigned long m_nUin,
                m_nStatus,
                m_nSequence,
                m_nZipcode,
                m_nGroups[2];
  unsigned short m_nSex,
                 m_nCountryCode,
                 m_nAge;
  char *m_sAlias,
       *m_sFirstName,
       *m_sLastName,
       *m_sEmail,
       *m_szAutoResponse,
       *m_sCity,
       *m_sState,
       *m_sPhoneNumber,
       *m_sHomepage,
       *m_sAbout;
  signed short m_nTimezone;
  bool m_bOnline,
       m_bIsNew,
       m_bOnlineNotify,
       m_bSendServer,
       m_bVisibleList,
       m_bInvisibleList,
       m_bEnableSave,
       m_bAuthorization,
       m_bShowAwayMsg;
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
  void SetPassword(const char *s) { SetString(&m_szPassword, s); saveInfo();}

  // Virtual overloaded functions
  virtual void getBasicInfo(struct UserBasicInfo &us);
  virtual void getExtInfo(struct UserExtInfo &us);
  virtual void saveInfo(void);

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
