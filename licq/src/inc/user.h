#ifndef USER_H
#define USER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector.h>
#include <list.h>

#include "pthread_rdwr.h"
#include "constants.h"
#include "icq-defines.h"
#include "file.h"
#include "message.h"
#include "history.h"
#include "userfcndlg.h"
#include "support.h"


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
   char phone[32];
   char age[16];
   char sex[10];
   char homepage[256];
   char about[MAX_MESSAGE_SIZE];
};

enum ESortKey { SORT_STATUS, SORT_ONLINE };


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
  char *getAwayMessage(void)   { return(m_sAwayMessage); }
  char *getCity(void)          { return(m_sCity); }
  char *getState(void)         { return(m_sState); }
  char *getPhoneNumber(void)   { return(m_sPhoneNumber); }
  char *getAbout(void)         { return(m_sAbout); }
  char *getHomepage(void)      { return(m_sHomepage); }
  bool getIsNew(void)         { return(m_bIsNew); }
  bool getInvisibleList(void) { return(m_bInvisibleList); }
  bool getVisibleList(void)   { return(m_bVisibleList); }
  bool getOnlineNotify(void)  { return(m_bOnlineNotify); }
  bool getSendServer(void)    { return(m_bSendServer); }
  bool getEnableSave(void)    { return(m_bEnableSave); }
  bool getAuthorization(void)  { return m_bAuthorization; }
  bool ShowAwayMsg(void)      { return m_bShowAwayMsg; }
  unsigned long getUin(void)           { return(m_nUin); }
  unsigned long getGroup(void)         { return(m_nGroup); }
  unsigned long getStatusFull(void)  { return m_nStatus; }
  unsigned long getStatusFlags(void)  { return m_nStatus & ICQ_STATUS_FxFLAGS; }
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

  void GetHistory(char *&buf)  { m_fHistory.Load(buf); }
  void SaveHistory(const char *buf)  { m_fHistory.Save(buf); }
  unsigned long SortKey(void);
  static void SetSortKey(ESortKey);
  void usprintf(char *_sz, const char *_szFormat);

   // Settors
  void setEnableSave(bool s)          { m_bEnableSave = s; }
  void setSendServer(bool s)          { m_bSendServer = s; }
  void setSequence(unsigned long s)   { m_nSequence = s; }
  void setIsNew(bool s)               { m_bIsNew = s; saveInfo(); }
  void setInvisibleList(bool s)       { m_bInvisibleList = s; saveInfo(); }
  void setVisibleList(bool s)         { m_bVisibleList = s; saveInfo(); }
  void setOnlineNotify(bool s)        { m_bOnlineNotify = s; saveInfo(); }
  void setAuthorization(bool s)       { m_bAuthorization = s; saveBasicInfo(); }
  void setGroup(unsigned long s)      { m_nGroup = s; saveInfo(); }
  void setAwayMessage(const char *s)  { SetString(&m_sAwayMessage, s); }
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
  void setCountry(unsigned short s)   { m_nCountryCode = s; saveExtInfo(); };
  void setTimezone(signed short s)    { m_nTimezone = s; saveExtInfo(); };
  void setShowAwayMsg(bool s)         { m_bShowAwayMsg = s; };

  void setStatus(unsigned long);
  void setHistoryFile(const char *);

  // Message functions
  unsigned short getNumMessages(void)   { return(m_vcMessages.size()); }
  CUserEvent *GetEvent(unsigned short);
  void ClearEvent(unsigned short);
  void AddEvent(CUserEvent *);
  void WriteToHistory(const char *);

  // Group functions
  bool getIsInGroup(unsigned short);
  void setIsInGroup(unsigned short, bool);
  void AddToGroup(unsigned short);
  void RemoveFromGroup(unsigned short);

  // Socket functions
  int SocketDesc(void)          { return m_nSocketDesc; }
  void ClearSocketDesc(void)    { SetSocketDesc(-1); }
  void SetSocketDesc(int _nSd)  { m_nSocketDesc = _nSd; }
  unsigned long Ip(void)        { return m_nIp; }
  unsigned short Port(void)     { return m_nPort; }
  void SetIpPort(unsigned long _nIp, unsigned short _nPort);

  // Lock functions
  unsigned short LockType(void)        { return m_nLockType; }
  void SetLockType(unsigned short _n)  { m_nLockType = _n; }
  void ClearLockType(void)             { SetLockType(LOCK_N); }

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
                m_nSequence;
  unsigned short m_nGroup,
                 m_nSex,
                 m_nCountryCode,
                 m_nAge;
  char *m_sAlias,
       *m_sFirstName,
       *m_sLastName,
       *m_sEmail,
       *m_sAwayMessage,
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
  static pthread_mutex_t mutex_eSortKey;

  friend class CUserGroup;
  friend class CUserManager;
};


//=====ICQOwner=================================================================
class ICQOwner : public ICQUser
{
public:
  ICQOwner(void);
  bool Exception(void)  { return m_bException; }

  //void Register(unsigned long n, const char *s);
  char *getPassword(void) { return(m_sPassword); }
  void setPassword(const char *s) { SetString(&m_sPassword, s); saveInfo();}
  virtual void getBasicInfo(struct UserBasicInfo &us);
  virtual void getExtInfo(struct UserExtInfo &us);
  virtual void saveInfo(void);

  virtual bool User(void)  { return false; }
protected:
  char *m_sPassword;
  bool m_bException;
};


//=====CUserGroup===============================================================

class CUserGroup
{
public:
  CUserGroup(char *_sName, bool _bOwner);
  ~CUserGroup(void);

  void AddUser(ICQUser *);
  void RemoveUser(ICQUser *);
  ICQUser *FetchUser(unsigned short, unsigned short);
  void DropUser(ICQUser *u);

  void Lock(unsigned short _nLockType);
  void Unlock(void);

  unsigned short NumUsers(void)  { return m_vpcUsers.size(); }
  const char *Name(void)  { return m_szName; }
  void SetName(const char *_sz)  { SetString(&m_szName, _sz); }
  void Reorder(ICQUser *_pcUser);
protected:
  void ShiftUserUp(unsigned short i);
  void ShiftUserDown(unsigned short i);

  char *m_szName;
  vector <ICQUser *> m_vpcUsers;
  bool m_bIsOwner;

  pthread_rdwr_t mutex_rw;
  unsigned short m_nLockType;
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

  vector < list<ICQUser *> > m_vlTable;

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

  // The following 2 functions are *not* thread safe...
  void AddGroup(CUserGroup *);
  void RemoveGroup(unsigned short);
  void RenameGroup(unsigned short, const char *);
  unsigned short NumGroups(void)  { return m_vpcGroups.size(); };
  void SaveGroups(void);
  void SwapGroups(unsigned short g1, unsigned short g2);

  CUserGroup *FetchGroup(unsigned short, unsigned short);
  void DropGroup(CUserGroup *);

  void AddUserToGroup(unsigned long _nUin, unsigned short _nGroup);
  void RemoveUserFromGroup(unsigned long _nUin, unsigned short _nGroup);
  void Reorder(ICQUser *_pcUser);
  void SaveAllUsers(void);

  unsigned short NumUsers(void)
  {
    CUserGroup *g = FetchGroup(0, LOCK_R);
    unsigned short n = g->NumUsers();
    DropGroup(g);
    return n;
  }
  unsigned short DefaultGroup(void)  { return m_nDefaultGroup; }
  void SetDefaultGroup(unsigned short n)  { m_nDefaultGroup = n; SaveGroups(); }

protected:
  pthread_mutex_t mutex_groups;
  vector <CUserGroup *> m_vpcGroups;
  CUserHashTable m_hUsers;
  ICQOwner *m_xOwner;
  unsigned long m_nOwnerUin;
  unsigned short m_nDefaultGroup;
  bool m_bAllowSave;
};

extern class CUserManager gUserManager;

#endif
