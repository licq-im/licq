#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef HAVE_INET_ATON
#include <arpa/inet.h>
#endif

#include "constants.h"
#include "user.h"
#include "countrycodes.h"
#include "log.h"
#include "icqpacket.h"

class CUserManager gUserManager;


/*---------------------------------------------------------------------------
 * ICQUser::Lock
 *-------------------------------------------------------------------------*/
void ICQUser::Lock(unsigned short _nLockType)
{
  switch (_nLockType)
  {
  case LOCK_R:
    pthread_rdwr_rlock_np (&mutex_rw);
    break;
  case LOCK_W:
    pthread_rdwr_wlock_np(&mutex_rw);
    break;
  default:
    break;
  }
  m_nLockType = _nLockType;
}


/*---------------------------------------------------------------------------
 * ICQUser::Unlock
 *-------------------------------------------------------------------------*/
void ICQUser::Unlock(void)
{
  unsigned short nLockType = m_nLockType;
  m_nLockType = LOCK_R;
  switch (nLockType)
  {
  case LOCK_R:
    pthread_rdwr_runlock_np(&mutex_rw);
    break;
  case LOCK_W:
    pthread_rdwr_wunlock_np(&mutex_rw);
    break;
  default:
    break;
  }
}



//=====CUserManager=============================================================
CUserManager::CUserManager(void) : m_hUsers(USER_HASH_SIZE)
{
  // Set up the basic all users and new users group
  pthread_rdwr_init_np(&mutex_grouplist, NULL);
  pthread_rdwr_init_np(&mutex_userlist, NULL);
  m_nUserListLockType = m_nGroupListLockType = LOCK_N;

  m_xOwner = NULL;
  m_nOwnerUin = 0;
  ICQUser::SetSortKey(SORT_STATUS);
}

void CUserManager::SetOwnerUin(unsigned long _nUin)
{
  char buf[24];
  sprintf(buf, "%ld", _nUin);
  ICQOwner *o = FetchOwner(LOCK_W);
  o->setUin(_nUin);
  o->setAlias(buf);
  DropOwner();
  m_nOwnerUin = _nUin;
}


/*---------------------------------------------------------------------------
 * CUserManager::Load
 *-------------------------------------------------------------------------*/
bool CUserManager::Load(void)
{
  // Create the owner
  m_xOwner = new ICQOwner;
  if (m_xOwner->Exception())
    return false;
  m_nOwnerUin = m_xOwner->getUin();

  gLog.Info("%sUser configuration.\n", L_INITxSTR);

  // Load the group info from licq.conf
  char filename[MAX_FILENAME_LEN];
  sprintf(filename, "%s/licq.conf", BASE_DIR);
  CIniFile licqConf(INI_FxERROR | INI_FxFATAL);
  licqConf.LoadFile(filename);

  unsigned short nGroups;
  licqConf.SetSection("groups");
  licqConf.ReadNum("NumOfGroups", nGroups);

  m_bAllowSave = false;
  char sGroupKey[16], sGroupName[32];
  for (unsigned short i = 1; i <= nGroups; i++)
  {
     sprintf(sGroupKey, "Group%d.name", i);
     licqConf.ReadStr(sGroupKey, sGroupName);
     AddGroup(strdup(sGroupName));
  }
  m_bAllowSave = true;

  licqConf.ReadNum("DefaultGroup", m_nDefaultGroup, 0);
  if(m_nDefaultGroup >=  1024)
      m_nDefaultGroup = 0;
  licqConf.ReadNum("NewUserGroup", m_nNewUserGroup, 0);
  licqConf.CloseFile();

  // Load users from users.conf
  sprintf(filename, "%s/users.conf", BASE_DIR);
  CIniFile usersConf(INI_FxFATAL | INI_FxERROR);
  usersConf.LoadFile(filename);

  unsigned short nUsers;
  usersConf.SetSection("users");
  usersConf.ReadNum("NumOfUsers", nUsers);
  gLog.Info("%sLoading %d users.\n", L_INITxSTR, nUsers);

  char sUserKey[16];
  unsigned long nUserUin;
  usersConf.SetFlags(INI_FxWARN);
  for(unsigned short i = 1; i <= nUsers; i++)
  {
     sprintf(sUserKey, "User%d", i);
     if (!usersConf.ReadNum(sUserKey, nUserUin))
     {
        gLog.Warn("%sSkipping user %i, UIN not found.\n", L_WARNxSTR, i);
        continue;
     }
     sprintf(filename, "%s/%s/%li.uin", BASE_DIR, USER_DIR, nUserUin);
     AddUser(new ICQUser(nUserUin, filename));
  }

  return true;
}


/*---------------------------------------------------------------------------
 * CUserManager::AddUser
 *-------------------------------------------------------------------------*/
unsigned long CUserManager::AddUser(ICQUser *_pcUser)
{
  _pcUser->Lock(LOCK_R);
  unsigned long nUin = _pcUser->getUin();
  // Store the user in the hash table
  m_hUsers.Store(_pcUser, nUin);

  // Reorder the user to the correct place
  Reorder(_pcUser, false);
  _pcUser->Unlock();

  return nUin;
}


/*---------------------------------------------------------------------------
 * CUserManager::RemoveUser
 *-------------------------------------------------------------------------*/
void CUserManager::RemoveUser(unsigned long _nUin)
{
  ICQUser *u = FetchUser(_nUin, LOCK_W);
  if (u == NULL) return;
  u->RemoveFiles();
  LockUserList(LOCK_W);
  UserListIter iter = m_vpcUsers.begin();
  while (iter != m_vpcUsers.end() && u != (*iter)) iter++;
  if (iter == m_vpcUsers.end())
    gLog.Warn("%sInteral Error: CUserManager::RemoveUser():\n"
              "%sUser \"%s\" (%ld) not found in vector.\n",
              L_WARNxSTR, L_BLANKxSTR, u->getAlias(), u->getUin());
  else
    m_vpcUsers.erase(iter);
  DropUser(u);
  m_hUsers.Remove(_nUin);
  UnlockUserList();
  delete u;
}


/*---------------------------------------------------------------------------
 * CUserManager::AddGroup
 *-------------------------------------------------------------------------*/
void CUserManager::AddGroup(char *_szName)
{
  LockGroupList(LOCK_W);
  m_vszGroups.push_back(_szName);
  SaveGroups();
  UnlockGroupList();
}



/*---------------------------------------------------------------------------
 * CUserManager::RemoveGroup
 *
 * Removes a group (note groups are numbered 1 to NumGroups() )
 *-------------------------------------------------------------------------*/
void CUserManager::RemoveGroup(unsigned short n)
{
  // Don't delete the all users or new users groups
  if (n >= NumGroups()) return;

  GroupList *g = LockGroupList(LOCK_W);
  // Erase the group from the vector
  GroupListIter iter = m_vszGroups.begin();
  for (int i = 1; i < n; i++) iter++;
  free (*iter);
  m_vszGroups.erase(iter);

  unsigned short j;
  FOR_EACH_USER_START(LOCK_W)
  {
    for (j = n; j < g->size() + 1; j++)
      pUser->SetInGroup(GROUPS_USER, j, pUser->GetInGroup(GROUPS_USER, j + 1));
  }
  pUser->SetInGroup(GROUPS_USER, j, false);
  FOR_EACH_USER_END;
  if (m_nDefaultGroup >= n) m_nDefaultGroup--;
  if (m_nNewUserGroup >= n) m_nNewUserGroup--;
  SaveGroups();
  UnlockGroupList();
}


/*---------------------------------------------------------------------------
 * CUserManager::SwapGroups
 *-------------------------------------------------------------------------*/
void CUserManager::SwapGroups(unsigned short g1, unsigned short g2)
{
  LockGroupList(LOCK_W);

  // validate the group numbers
  if (g1 > m_vszGroups.size() || g1 < 1 || g2 > m_vszGroups.size() || g2 < 1)
  {
    UnlockGroupList();
    return;
  }

  // move the actual group
  char *g = m_vszGroups[g1 - 1];
  m_vszGroups[g1 - 1] = m_vszGroups[g2 - 1];
  m_vszGroups[g2 - 1] = g;
  if (m_nDefaultGroup == g1) m_nDefaultGroup = g2;
  else if (m_nDefaultGroup == g2) m_nDefaultGroup = g1;
  if (m_nNewUserGroup == g1) m_nNewUserGroup = g2;
  else if (m_nNewUserGroup == g2) m_nNewUserGroup = g1;
  SaveGroups();
  UnlockGroupList();

  // adjust all the users
  bool bInG1;
  FOR_EACH_USER_START(LOCK_W)
  {
    bInG1 = pUser->GetInGroup(GROUPS_USER, g1);
    pUser->SetInGroup(GROUPS_USER, g1, pUser->GetInGroup(GROUPS_USER, g2));
    pUser->SetInGroup(GROUPS_USER, g2, bInG1);
  }
  FOR_EACH_USER_END
}


/*---------------------------------------------------------------------------
 * CUserManager::RenameGroup
 *-------------------------------------------------------------------------*/
void CUserManager::RenameGroup(unsigned short n, const char *_sz)
{
  if (n < 1 || n > NumGroups()) return;
  GroupList *g = LockGroupList(LOCK_W);
  free((*g)[n - 1]);
  (*g)[n - 1] = strdup(_sz);
  SaveGroups();
  UnlockGroupList();
}


/*---------------------------------------------------------------------------
 * CUserManager::SaveGroups
 *
 * Assumes a lock on the group list.
 *-------------------------------------------------------------------------*/
void CUserManager::SaveGroups(void)
{
  if (!m_bAllowSave) return;

  // Load the group info from licq.conf
  char filename[MAX_FILENAME_LEN];
  sprintf(filename, "%s/licq.conf", BASE_DIR);
  CIniFile licqConf(INI_FxERROR | INI_FxFATAL);
  licqConf.LoadFile(filename);

  licqConf.SetSection("groups");
  licqConf.WriteNum("NumOfGroups", NumGroups());

  char sGroupKey[16];
  //LockGroupList(LOCK_R);
  for (unsigned short i = 0; i < m_vszGroups.size(); i++)
  {
     sprintf(sGroupKey, "Group%d.name", i + 1);
     licqConf.WriteStr(sGroupKey, m_vszGroups[i]);
  }
  //UnlockGroupList();

  licqConf.WriteNum("DefaultGroup", m_nDefaultGroup);
  licqConf.WriteNum("NewUserGroup", m_nNewUserGroup);
  licqConf.FlushFile();
  licqConf.CloseFile();
}



/*---------------------------------------------------------------------------
 * CUserManager::FetchUser
 *-------------------------------------------------------------------------*/
ICQUser *CUserManager::FetchUser(unsigned long _nUin, unsigned short _nLockType)
{
  ICQUser *u = NULL;
  if (_nUin == m_nOwnerUin)
    u = m_xOwner;
  else
    u = m_hUsers.Retrieve(_nUin);
  if (u != NULL)
  {
    u->Lock(_nLockType);
    if (_nUin != u->getUin())
      gLog.Error("%sInternal error: CUserManager::FetchUser(): Looked for %d, found %d.\n",
                 _nUin, u->getUin());
  }
  return u;
}



/*---------------------------------------------------------------------------
 * CUserManager::DropUser
 *-------------------------------------------------------------------------*/
void CUserManager::DropUser(ICQUser *u)
{
  u->Unlock();
}

/*---------------------------------------------------------------------------
 * CUserManager::FetchOwner
 *-------------------------------------------------------------------------*/
ICQOwner *CUserManager::FetchOwner(unsigned short _nLockType)
{
  m_xOwner->Lock(_nLockType);
  return m_xOwner;
}


/*---------------------------------------------------------------------------
 * CUserManager::DropOwner
 *-------------------------------------------------------------------------*/
void CUserManager::DropOwner(void)
{
  m_xOwner->Unlock();
}


/*---------------------------------------------------------------------------
 * CUserManager::Reorder
 *
 * Assumes a read lock on the user.
 *-------------------------------------------------------------------------*/
void CUserManager::Reorder(ICQUser *_pcUser, bool _bOnList)
{
  UserList *ul = LockUserList(LOCK_W);
  UserListIter iter;

  if (_bOnList)
  {
    iter = ul->begin();
    while (iter != ul->end() && *iter != _pcUser) iter++;
    if (iter == ul->end())
    {
      gLog.Warn("%sInternal Error: CUserManager::Reorder():\n"
                "%sGiven user \"%s\" (%ld) not found.\n", L_WARNxSTR,
                L_BLANKxSTR, _pcUser->getAlias(), _pcUser->getUin());
      UnlockUserList();
      return;
    }
    // Now iter is the user to move
    ul->erase(iter);
  }

  // Now we reinsert the user in the correct place
  pthread_mutex_lock(&ICQUser::mutex_sortkey);
  for (iter = ul->begin(); iter != ul->end(); iter++)
  {
    if (_pcUser->SortKey() <= (*iter)->SortKey())
    {
      ul->insert(iter, _pcUser);
      break;
    }
  }
  // Check if we have to add to the end
  if (iter == ul->end()) ul->push_back(_pcUser);
  pthread_mutex_unlock(&ICQUser::mutex_sortkey);

  UnlockUserList();
}


/*---------------------------------------------------------------------------
 * CUserManager::SaveAllUsers
 *-------------------------------------------------------------------------*/
void CUserManager::SaveAllUsers(void)
{
  FOR_EACH_USER_START(LOCK_R)
  {
    pUser->saveInfo();
    pUser->saveBasicInfo();
    pUser->saveExtInfo();
  }
  FOR_EACH_USER_END
}


/*---------------------------------------------------------------------------
 * CUserManager::NumUsers
 *-------------------------------------------------------------------------*/
unsigned short CUserManager::NumUsers(void)
{
  //LockUserList(LOCK_R);
  unsigned short n = m_vpcUsers.size();
  //UnlockUserList();
  return n;
}


/*---------------------------------------------------------------------------
 * CUserManager::NumGroups
 *-------------------------------------------------------------------------*/
unsigned short CUserManager::NumGroups(void)
{
  //LockGroupList(LOCK_R);
  unsigned short n = m_vszGroups.size();
  //UnlockGroupList();
  return n;
}


/*---------------------------------------------------------------------------
 * LockUserList
 *
 * Locks the entire user list for iterating through...
 *-------------------------------------------------------------------------*/
UserList *CUserManager::LockUserList(unsigned short _nLockType)
{
  switch (_nLockType)
  {
  case LOCK_R:
    pthread_rdwr_rlock_np (&mutex_userlist);
    break;
  case LOCK_W:
    pthread_rdwr_wlock_np(&mutex_userlist);
    break;
  default:
    break;
  }
  m_nUserListLockType = _nLockType;
  return &m_vpcUsers;
}





/*---------------------------------------------------------------------------
 * CUserManager::UnlockUserList
 *-------------------------------------------------------------------------*/
void CUserManager::UnlockUserList(void)
{
  unsigned short nLockType = m_nUserListLockType;
  m_nUserListLockType = LOCK_R;
  switch (nLockType)
  {
  case LOCK_R:
    pthread_rdwr_runlock_np(&mutex_userlist);
    break;
  case LOCK_W:
    pthread_rdwr_wunlock_np(&mutex_userlist);
    break;
  default:
    break;
  }
}

/*---------------------------------------------------------------------------
 * LockGroupList
 *
 * Locks the entire group list for iterating through...
 *-------------------------------------------------------------------------*/
GroupList *CUserManager::LockGroupList(unsigned short _nLockType)
{
  switch (_nLockType)
  {
  case LOCK_R:
    pthread_rdwr_rlock_np (&mutex_grouplist);
    break;
  case LOCK_W:
    pthread_rdwr_wlock_np(&mutex_grouplist);
    break;
  default:
    break;
  }
  m_nGroupListLockType = _nLockType;
  return &m_vszGroups;
}



/*---------------------------------------------------------------------------
 * CUserManager::UnlockGroupList
 *-------------------------------------------------------------------------*/
void CUserManager::UnlockGroupList(void)
{
  unsigned short nLockType = m_nGroupListLockType;
  m_nGroupListLockType = LOCK_R;
  switch (nLockType)
  {
  case LOCK_R:
    pthread_rdwr_runlock_np(&mutex_grouplist);
    break;
  case LOCK_W:
    pthread_rdwr_wunlock_np(&mutex_grouplist);
    break;
  default:
    break;
  }
}


/*---------------------------------------------------------------------------
 * CUserManager::AddUserToGroup
 *-------------------------------------------------------------------------*/
void CUserManager::AddUserToGroup(unsigned long _nUin, unsigned short _nGroup)
{
  ICQUser *u = FetchUser(_nUin, LOCK_W);
  if (u == NULL) return;
  u->AddToGroup(GROUPS_USER, _nGroup);
  DropUser(u);
}


/*---------------------------------------------------------------------------
 * CUserManager::RemoveUserFromGroup
 *-------------------------------------------------------------------------*/
void CUserManager::RemoveUserFromGroup(unsigned long _nUin, unsigned short _nGroup)
{
  ICQUser *u = FetchUser(_nUin, LOCK_W);
  if (u == NULL) return;
  u->RemoveFromGroup(GROUPS_USER, _nGroup);
  DropUser(u);
}


//=====CUserHashTable===========================================================
CUserHashTable::CUserHashTable(unsigned short _nSize) : m_vlTable(_nSize)
{
  pthread_rdwr_init_np(&mutex_rw, NULL);
  m_nLockType = LOCK_R;
}


ICQUser *CUserHashTable::Retrieve(unsigned long _nUin)
{
  Lock(LOCK_R);

  ICQUser *u = NULL;
  UserList &l = m_vlTable[HashValue(_nUin)];
  // If no users, this is bad
  if (l.size() == 0)
    u = NULL;
/*
  // if only one, assume it's the right one (saves having to lock the user
  // to check)
  else if (l.size() == 1)
    u = *(l.begin());*/

  // Otherwise, start iterating and comparing uins
  else
  {
    unsigned long nUin;
    UserListIter iter;
    for (iter = l.begin(); iter != l.end(); iter++)
    {
      (*iter)->Lock(LOCK_R);
      nUin = (*iter)->getUin();
      (*iter)->Unlock();
      if (nUin == _nUin)
      {
        u = (*iter);
        break;
      }
    }
    if (iter == l.end()) u =  NULL;
  }

  Unlock();
  return u;
}

void CUserHashTable::Store(ICQUser *u, unsigned long _nUin)
{
  Lock(LOCK_W);
  UserList &l = m_vlTable[HashValue(_nUin)];
  l.push_front(u);
  Unlock();
}

void CUserHashTable::Remove(unsigned long _nUin)
{
  Lock(LOCK_W);

  UserList &l = m_vlTable[HashValue(_nUin)];
  unsigned long nUin;
  UserListIter iter;
  for (iter = l.begin(); iter != l.end(); iter++)
  {
    (*iter)->Lock(LOCK_R);
    nUin = (*iter)->getUin();
    (*iter)->Unlock();
    if (nUin == _nUin)
    {
      l.erase(iter);
      break;
    }
  }
  Unlock();
}

unsigned short CUserHashTable::HashValue(unsigned long _nUin)
{
  //return _nUin % m_vlTable.size();
  return _nUin & (unsigned long)(USER_HASH_SIZE - 1);
}

void CUserHashTable::Lock(unsigned short _nLockType)
{
  switch (_nLockType)
  {
  case LOCK_R:
    pthread_rdwr_rlock_np (&mutex_rw);
    break;
  case LOCK_W:
    pthread_rdwr_wlock_np(&mutex_rw);
    break;
  default:
    break;
  }
  m_nLockType = _nLockType;
}

void CUserHashTable::Unlock(void)
{
  unsigned short nLockType = m_nLockType;
  m_nLockType = LOCK_R;
  switch (nLockType)
  {
  case LOCK_R:
    pthread_rdwr_runlock_np(&mutex_rw);
    break;
  case LOCK_W:
    pthread_rdwr_wunlock_np(&mutex_rw);
    break;
  default:
    break;
  }
}




















//=====CUser====================================================================

unsigned short ICQUser::s_nNumUserEvents = 0;
pthread_mutex_t ICQUser::mutex_nNumUserEvents = PTHREAD_MUTEX_INITIALIZER;
ESortKey ICQUser::s_eSortKey;
pthread_mutex_t ICQUser::mutex_sortkey;


//-----ICQUser::constructor-----------------------------------------------------
ICQUser::ICQUser(unsigned long _nUin, char *_szFilename)
// Called when first constructing our known users
{
  setEnableSave(false);
  Init(_nUin);
  m_fConf.SetFlags(INI_FxWARN);
  m_fConf.SetFileName(_szFilename);
  if (!LoadData())
  {
    gLog.Error("%sUnable to load user info from '%s'.  Using default values.\n",
               L_ERRORxSTR, _szFilename, L_BLANKxSTR);
    SetDefaults();
  }
  m_fConf.CloseFile();
  m_fConf.SetFlags(INI_FxWARN | INI_FxALLOWxCREATE);
  setEnableSave(true);
}


ICQUser::ICQUser(unsigned long _nUin)
{
  setEnableSave(false);
  Init(_nUin);
  SetDefaults();
  char szFilename[MAX_FILENAME_LEN];
  sprintf(szFilename, "%s/%s/%ld.uin", BASE_DIR, USER_DIR, _nUin);
  m_fConf.SetFileName(szFilename);
  m_fConf.SetFlags(INI_FxWARN | INI_FxALLOWxCREATE);
  setEnableSave(true);
  saveInfo();
  saveBasicInfo();
  saveExtInfo();
}



//-----ICQUser::LoadData--------------------------------------------------------
bool ICQUser::LoadData(void)
{
  if (!m_fConf.ReloadFile()) return (false);

  // read in the fields, checking for errors each time
  char sTemp[MAX_DATA_LEN];
  bool bTemp;
  unsigned short nTemp, nNewMessages;
  m_fConf.SetFlags(0);
  m_fConf.SetSection("user");
  m_fConf.ReadStr("Alias", sTemp, "Unknown");
  setAlias(sTemp);
  m_fConf.ReadStr("FirstName", sTemp, "");
  setFirstName(sTemp);
  m_fConf.ReadStr("LastName", sTemp, "");
  setLastName(sTemp);
  m_fConf.ReadStr("EMail", sTemp, "");
  setEmail(sTemp);
  m_fConf.ReadBool("Authorization", bTemp, false);
  setAuthorization(bTemp);
  m_fConf.ReadStr("History", sTemp, "default");
  if (sTemp[0] == '\0') strcpy(sTemp, "default");
  setHistoryFile(sTemp);
  //m_fConf.ReadBool("OnlineNotify", bTemp, false);
  //SetOnlineNotify(bTemp);
  m_fConf.ReadNum("NewMessages", nTemp, 0);
  nNewMessages = nTemp;
  m_fConf.ReadBool("NewUser", bTemp, false);
  setIsNew(bTemp);
  m_fConf.ReadNum("Groups.System", m_nGroups[GROUPS_SYSTEM], 0);
  m_fConf.ReadNum("Groups.User", m_nGroups[GROUPS_USER], 0);
  m_fConf.ReadStr("Ip", sTemp, "0.0.0.0");
  struct in_addr in;
  nTemp = inet_aton(sTemp, &in);
  if (nTemp != 0) nTemp = in.s_addr;
  unsigned short nPort;
  m_fConf.ReadNum("Port", nPort, 0);
  SetIpPort(nTemp, nPort);
  //m_fConf.ReadBool("VisibleList", bTemp, false);
  //setVisibleList(bTemp);
  //m_fConf.ReadBool("InvisibleList", bTemp, false);
  //setInvisibleList(bTemp);
  m_fConf.ReadStr("City", sTemp, "Unknown");
  setCity(sTemp);
  m_fConf.ReadStr("State", sTemp, "Unknown");
  setState(sTemp);
  m_fConf.ReadNum("Country", nTemp, 0xFFFF);
  setCountry(nTemp);
  m_fConf.ReadNum("Timezone", nTemp, 0x00);
  setTimezone(nTemp);
  m_fConf.ReadNum("Zipcode", m_nZipcode, 0);
  m_fConf.ReadStr("PhoneNumber", sTemp, "");
  setPhoneNumber(sTemp);
  m_fConf.ReadNum("Age", nTemp, 0);
  setAge(nTemp);
  m_fConf.ReadNum("Sex", nTemp, 0);
  setSex(nTemp);
  m_fConf.ReadStr("Homepage", sTemp, "Unknown");
  setHomepage(sTemp);
  m_fConf.ReadStr("About", sTemp, "");
  setAbout(sTemp);

  if (nNewMessages > 0)
  {
     m_vcMessages.push_back(new CEventSaved(nNewMessages));
     incNumUserEvents();
  }
  return (true);
}



//-----ICQUser::destructor------------------------------------------------------
ICQUser::~ICQUser(void)
{
  while (getNumMessages() > 0) ClearEvent(0);
/*
  // Destroy the mutex
  int nResult = 0;
  do
  {
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    nResult = pthread_mutex_destroy(&mutex);
  } while (nResult != 0);
*/
}


//-----ICQUser::RemoveFiles-----------------------------------------------------
void ICQUser::RemoveFiles(void)
{
  remove(m_fConf.FileName());
  remove(m_fHistory.FileName());
}


void ICQUser::Init(unsigned long _nUin)
{
  m_sAlias = NULL;
  m_sFirstName = NULL;
  m_sLastName = NULL;
  m_sEmail = NULL;
  m_szAutoResponse = NULL;
  m_sCity = NULL;
  m_sState = NULL;
  m_sPhoneNumber = NULL;
  m_sHomepage = NULL;
  m_sAbout = NULL;

  setUin(_nUin);
  setStatus(ICQ_STATUS_OFFLINE);
  SetAutoResponse("");
  setSendServer(false);
  setShowAwayMsg(true);
  setSequence(1);
  ClearSocketDesc();
  fcnDlg = NULL;
  m_nIp = m_nPort = 0;

  pthread_rdwr_init_np (&mutex_rw, NULL);
}

//-----ICQUser::SetDefaults-----------------------------------------------------
void ICQUser::SetDefaults(void)
{
  char szAlias[12];
  sprintf(szAlias, "%ld", getUin());
  setAlias(szAlias);
  setFirstName("");
  setLastName("");
  setEmail("");
  setHistoryFile("default");
  setCity("");
  setState("");
  setSex(0);
  setAge(0xFFFF);
  setCountry(0xFFFF);
  setZipcode(0);
  setHomepage("");
  setPhoneNumber("");
  setAbout("");
  SetGroups(GROUPS_SYSTEM, 0);
  SetGroups(GROUPS_USER, gUserManager.NewUserGroup());
  setAuthorization(false);
  setIsNew(true);
}


char *ICQUser::getCountry(char *buf)
{
  if (m_nCountryCode == COUNTRY_UNSPECIFIED)
  {
     strcpy(buf, "Unspecified");
     return (buf);
  }

  const char *szCountry = GetCountryByCode(m_nCountryCode);
  if (szCountry == NULL)
    sprintf(buf, "Unknown (%d)", m_nCountryCode);
  else
    strcpy(buf, szCountry);
  return (buf);
}


char *ICQUser::getSex(char *buf)
{
  if (m_nSex == 1)
     strcpy(buf, "Female");
  else if (m_nSex == 2)
     strcpy(buf, "Male");
  else
     strcpy(buf, "Unknown");
  return(buf);
}


inline bool ICQUser::getStatusOffline(void)
{
  unsigned short nStatusShort = (unsigned short)m_nStatus;
  return (nStatusShort == ICQ_STATUS_OFFLINE);
}

unsigned short ICQUser::getStatus(void)
// guarantees to return a unique status that switch can be run on
{
   if (getStatusOffline()) return ICQ_STATUS_OFFLINE;
   else if (m_nStatus & ICQ_STATUS_DND) return ICQ_STATUS_DND;
   else if (m_nStatus & ICQ_STATUS_OCCUPIED) return ICQ_STATUS_OCCUPIED;
   else if (m_nStatus & ICQ_STATUS_NA) return ICQ_STATUS_NA;
   else if (m_nStatus & ICQ_STATUS_AWAY) return ICQ_STATUS_AWAY;
   else if (m_nStatus & ICQ_STATUS_FREEFORCHAT) return ICQ_STATUS_FREEFORCHAT;
   else if (m_nStatus << 24 == 0x00) return ICQ_STATUS_ONLINE;
   else return (ICQ_STATUS_OFFLINE - 1);
}

inline bool ICQUser::getStatusInvisible(void)
{
   return (getStatusOffline() ? false : m_nStatus & ICQ_STATUS_FxPRIVATE);
}

inline bool ICQUser::getStatusWebPresence(void)
{
   return (m_nStatus & ICQ_STATUS_FxWEBxPRESENCE);
}

inline bool ICQUser::getStatusHideIp(void)
{
   return (m_nStatus & ICQ_STATUS_FxHIDExIP);
}

inline bool ICQUser::getStatusBirthday(void)
{
   return (m_nStatus & ICQ_STATUS_FxBIRTHDAY);
}

unsigned long ICQUser::SortKey(void)
{
  switch (s_eSortKey)
  {
  case SORT_STATUS:
  {
    unsigned long s = getStatus();
    return (s == ICQ_STATUS_FREEFORCHAT ? ICQ_STATUS_ONLINE : s);
  }
  case SORT_ONLINE:
    return (getStatusOffline() ? 1 : 0);
  }
  return 0;
}

void ICQUser::SetSortKey(ESortKey _eSortKey)
{
  pthread_mutex_lock(&mutex_sortkey);
  s_eSortKey = _eSortKey;
  pthread_mutex_unlock(&mutex_sortkey);
}

unsigned long ICQUser::getSequence(bool increment = false)
{
   if (increment)
      return (m_nSequence++);
   else
      return (m_nSequence);
}

void ICQUser::setAlias(const char *s)
{
  if (s[0] == '\0')
  {
    char sz[12];
    sprintf(sz, "%ld", getUin());
    SetString(&m_sAlias, sz);
  }
  else
    SetString(&m_sAlias, s);
  saveBasicInfo();
}

void ICQUser::setStatus(unsigned long s)
{
  m_nStatus = s;
}

bool ICQUser::isAway(void)
{
   return (getStatus() == ICQ_STATUS_AWAY || getStatus() == ICQ_STATUS_NA ||
           getStatus() == ICQ_STATUS_DND || getStatus() == ICQ_STATUS_OCCUPIED);
}

void ICQUser::setHistoryFile(const char *s)
{
  m_fHistory.SetFile(s, m_nUin);
  saveBasicInfo();
}


void ICQUser::SetIpPort(unsigned long _nIp, unsigned short _nPort)
{
  if (SocketDesc() != -1 && (Ip() != _nIp || Port() != _nPort))
  {
    // Close our socket, but don't let socket manager try and clear
    // our socket descriptor
    gSocketManager.CloseSocket(SocketDesc(), false);
    ClearSocketDesc();
  }
  m_nIp = _nIp;
  m_nPort = _nPort;
  saveInfo();
}


void ICQUser::getStatusStr(char *sz)
{
  StatusStr(getStatus(), getStatusInvisible(), sz);
}


void ICQUser::StatusStr(unsigned short n, bool b, char *sz)
{
  switch(n)
  {
  case ICQ_STATUS_OFFLINE:     strcpy(sz, "Offline");  break;
  case ICQ_STATUS_ONLINE:      strcpy(sz, "Online");   break;
  case ICQ_STATUS_AWAY:        strcpy(sz, "Away");     break;
  case ICQ_STATUS_NA:          strcpy(sz, "Not Available"); break;
  case ICQ_STATUS_OCCUPIED:    strcpy(sz, "Occupied"); break;
  case ICQ_STATUS_DND:         strcpy(sz, "Do Not Disturb"); break;
  case ICQ_STATUS_FREEFORCHAT: strcpy(sz, "Free for Chat"); break;
  default:                     sprintf(sz, "0x%04X", n); break;
  }
  if (b)
  {
    memmove(sz + 1, sz, strlen(sz) + 1);
    sz[0] = '(';
    strcat(sz, ")");
  }
}


//-----ICQUser::getBasicInfo----------------------------------------------------
void ICQUser::getBasicInfo(struct UserBasicInfo &us)
{
   strcpy(us.alias, getAlias());
   sprintf(us.uin, "%ld", getUin());
   strcpy(us.firstname, getFirstName());
   strcpy(us.lastname, getLastName());
   sprintf(us.name, "%s %s", us.firstname, us.lastname);
   strcpy(us.email, getEmail());
   getStatusStr(us.status);

   // Track down the current ip and port
   char buf[32];
   if (SocketDesc() > 0)    // First check if we are connected
   {
     INetSocket *s = gSocketManager.FetchSocket(SocketDesc());
     if (s != NULL)
     {
       if (User())
       {
         strcpy(us.ip, s->RemoteIpStr(buf));
         sprintf(us.port, "%d", s->RemotePort());
       }
       else
       {
         strcpy(us.ip, s->LocalIpStr(buf));
         sprintf(us.port, "%d", s->LocalPort());
       }
       gSocketManager.DropSocket(s);
     }
     else
     {
       strcpy(us.ip, "invalid");
       strcpy(us.port, "invalid");
     }
   }
   else
   {
     if (Ip() > 0)     // Default to the given ip
       strcpy(us.ip, inet_ntoa_r(*(struct in_addr *)&m_nIp, buf));
     else                   // Otherwise we don't know
       strcpy(us.ip, "???");

     if (Port() > 0)
       sprintf(us.port, "%d", Port());
     else
       strcpy(us.port, "??");
   }

   if (getStatusHideIp())
   {
     strcpy(buf, us.ip);
     sprintf(us.ip, "(%s)", buf);
   }
   sprintf(us.ip_port, "%s:%s", us.ip, us.port);
   strcpy(us.history, m_fHistory.Description());
   strncpy(us.awayMessage, AutoResponse(), MAX_MESSAGE_SIZE);
}


//-----ICQUser::getExtInfo---------------------------------------------------
void ICQUser::getExtInfo(struct UserExtInfo &ud)
{
   strcpy(ud.city, getCity());
   strcpy(ud.state, getState());
   getCountry(ud.country);
   sprintf(ud.timezone, "%d", getTimezone());
   strcpy(ud.phone, getPhoneNumber());
   if (getAge() == 0 || getAge() == 0xFFFF)
      strcpy(ud.age, "N/A");
   else
      sprintf(ud.age, "%d", getAge());
   getSex(ud.sex);
   strcpy(ud.homepage, getHomepage());
   strcpy(ud.about, getAbout());
   sprintf(ud.zipcode, "%05ld", getZipcode());
}


void ICQUser::usprintf(char *_sz, const char *_szFormat, bool bAllowFieldWidth)
{
  bool bLeft = false;
  unsigned short i = 0, j, nField = 0, nPos = 0;
  char szTemp[128], *sz;
  _sz[0] = '\0';
  while(_szFormat[i] != '\0')
  {
    if (_szFormat[i] == '%')
    {
      i++;
      if (bAllowFieldWidth)
      {
        if (_szFormat[i] == '-')
        {
          i++;
          bLeft = true;
        }
        j = nField = 0;
        while (isdigit(_szFormat[i]))
          szTemp[j++] = _szFormat[i++];
        if (j > 0) nField = atoi(szTemp);
      }
      else
      {
        if (isdigit(_szFormat[i]))
        {
          strncat(_sz, &_szFormat[i - 1], 2);
          i++;
          continue;
        }
      }
      switch(_szFormat[i])
      {
      case 'i':
        char buf[32];
        strcpy(szTemp, inet_ntoa_r(*(struct in_addr *)&m_nIp, buf));
        sz = szTemp;
        break;
      case 'p':
        sprintf(szTemp, "%d", Port());
        sz = szTemp;
        break;
      case 'e':
        sz = getEmail();
        break;
      case 'n':
        sprintf(szTemp, "%s %s", getFirstName(), getLastName());
        sz = szTemp;
        break;
      case 'f':
        sz = getFirstName();
        break;
      case 'l':
        sz = getLastName();
        break;
      case 'a':
        sz = getAlias();
        break;
      case 'u':
        sprintf(szTemp, "%ld", getUin());
        sz = szTemp;
        break;
      case 'w':
        sz = getHomepage();
        break;
      case 'h':
        sz = getPhoneNumber();
        break;
      default:
        gLog.Warn("%sWarning: Invalid qualifier in command: %%%c.\n",
                  L_WARNxSTR, _szFormat[i]);
        sz = NULL;
        break;
      }
      // Now append sz to the string using the given field width and alignment
      if (nField == 0)
      {
        j = 0;
        while(sz[j] != '\0') _sz[nPos++] = sz[j++];
      }
      else
      {
        if (bLeft)
        {
          j = 0;
          while(sz[j] != '\0') _sz[nPos++] = sz[j++];
          while(j++ < nField) _sz[nPos++] = ' ';
        }
        else
        {
          int nLen = nField - strlen(sz);
          for (j = 0; j < nLen; j++) _sz[nPos++] = ' ';
          j = 0;
          while(sz[j] != '\0') _sz[nPos++] = sz[j++];
        }
      }
      i++;
    }
    else
    {
      _sz[nPos++] = _szFormat[i++];
    }
  }
  _sz[nPos] = '\0';

}


//-----ICQUser::saveBasicInfo---------------------------------------------------
void ICQUser::saveBasicInfo(void)
{
  if (!getEnableSave()) return;

  if (!m_fConf.ReloadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n", 
                L_ERRORxSTR, m_fConf.FileName(),  L_BLANKxSTR);
     return; 
  }
  m_fConf.SetSection("user");
  m_fConf.WriteStr("Alias", getAlias());
  m_fConf.WriteStr("FirstName", getFirstName());
  m_fConf.WriteStr("LastName", getLastName());
  m_fConf.WriteStr("EMail", getEmail());
  m_fConf.WriteBool("Authorization", getAuthorization());
  m_fConf.WriteStr("History", m_fHistory.Description());
  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
               L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
    return;
  }

  m_fConf.CloseFile();
}


//-----ICQUser::saveInfo--------------------------------------------------------
void ICQUser::saveInfo(void)
{
   if (!getEnableSave()) return;

   if (!m_fConf.ReloadFile())
   {
      gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                 L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
      return;
   }
   m_fConf.SetSection("user");
   m_fConf.WriteNum("Groups.System", GetGroups(GROUPS_SYSTEM));
   m_fConf.WriteNum("Groups.User", GetGroups(GROUPS_USER));
   char buf[64];
   m_fConf.WriteStr("Ip", inet_ntoa_r(*(struct in_addr *)&m_nIp, buf));
   m_fConf.WriteNum("Port", Port());
   m_fConf.WriteBool("NewUser", getIsNew());
   m_fConf.WriteNum("NewMessages", getNumMessages());
   if (!m_fConf.FlushFile())
   {
     gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
     return;
   }

   m_fConf.CloseFile();
}


//-----ICQUser::saveExtInfo--------------------------------------------------
void ICQUser::saveExtInfo(void)
{
   if (!getEnableSave()) return;

   if (!m_fConf.ReloadFile())
   {
      gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
                 L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
      return;
   }
   m_fConf.SetSection("user");
   m_fConf.WriteStr("Homepage", getHomepage());
   m_fConf.WriteStr("City", getCity());
   m_fConf.WriteStr("State", getState());
   m_fConf.WriteNum("Country", getCountryCode());
   m_fConf.WriteNum("Timezone", getTimezone());
   m_fConf.WriteNum("Zipcode", getZipcode());
   m_fConf.WriteStr("PhoneNumber", getPhoneNumber());
   m_fConf.WriteNum("Age", getAge());
   m_fConf.WriteNum("Sex", getSexNum());
   m_fConf.WriteStr("About", getAbout());
   if (!m_fConf.FlushFile())
   {
     gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
     return;
   }

   m_fConf.CloseFile();

}


//-----ICQUser::AddEvent--------------------------------------------------------
void ICQUser::AddEvent(CUserEvent *e)
{
   m_vcMessages.push_back(e);
   incNumUserEvents();
   saveInfo();
}


void ICQUser::WriteToHistory(const char *_szText)
{
  m_fHistory.Append(_szText);
}



//-----ICQUser::GetEvent--------------------------------------------------------
CUserEvent *ICQUser::GetEvent(unsigned short index)
{
   if (index >= getNumMessages() || getNumMessages() == 0) return (NULL);
   return (m_vcMessages[index]);
}


//-----ICQUser::ClearEvent------------------------------------------------------
void ICQUser::ClearEvent(unsigned short index)
{
   delete m_vcMessages[index];
   for (unsigned short i = index; i < m_vcMessages.size() - 1; i++)
      m_vcMessages[i] = m_vcMessages[i + 1];
   m_vcMessages.pop_back();
   decNumUserEvents();
   saveInfo();
}


bool ICQUser::GetInGroup(GroupType g, unsigned short _nGroup)
{
  if (_nGroup == 0) return true;
  return (GetGroups(g) & (unsigned long)(1 << (_nGroup - 1)));
}

void ICQUser::SetInGroup(GroupType g, unsigned short _nGroup, bool _bIn)
{
  if (_bIn)
    AddToGroup(g, _nGroup);
  else
    RemoveFromGroup(g, _nGroup);
}

void ICQUser::AddToGroup(GroupType g, unsigned short _nGroup)
{
  if (_nGroup == 0) return;
  SetGroups(g, GetGroups(g) | (unsigned long)(1 << (_nGroup - 1)));
}


void ICQUser::RemoveFromGroup(GroupType g, unsigned short _nGroup)
{
  if (_nGroup == 0) return;
  SetGroups(g, GetGroups(g) & (0xFFFFFFFF - (unsigned long)(1 << (_nGroup - 1))));
}


unsigned short ICQUser::getNumUserEvents(void)
{
  pthread_mutex_lock(&mutex_nNumUserEvents);
  unsigned short n = s_nNumUserEvents;
  pthread_mutex_unlock(&mutex_nNumUserEvents);
  return n;
}

void ICQUser::incNumUserEvents(void)
{
  pthread_mutex_lock(&mutex_nNumUserEvents);
  s_nNumUserEvents++;
  pthread_mutex_unlock(&mutex_nNumUserEvents);
}

void ICQUser::decNumUserEvents(void)
{
  pthread_mutex_lock(&mutex_nNumUserEvents);
  s_nNumUserEvents--;
  pthread_mutex_unlock(&mutex_nNumUserEvents);
}


//=====ICQOwner=================================================================

//-----ICQOwner::constructor----------------------------------------------------
ICQOwner::ICQOwner(void)
{
  gLog.Info("%sOwner configuration.\n", L_INITxSTR);
  unsigned long nTemp;
  bool bTemp;
  char szTemp[32];
  char filename[MAX_FILENAME_LEN];
  m_bException = false;
  m_szPassword = NULL;

  setEnableSave(false);
  Init(0);

  // Get data from the config file
  sprintf(filename, "%s/owner.uin", BASE_DIR);

  // Make sure owner.uin is mode 0600
  if (chmod(filename, S_IRUSR | S_IWUSR) == -1)
  {
    gLog.Warn("%sUnable to set %s to mode 0600.  Your ICQ password is vulnerable.\n",
                 L_WARNxSTR, filename);
  }

  m_fConf.SetFileName(filename);
  LoadData();
  m_fConf.SetFlags(INI_FxWARN);
  m_fConf.ReadNum("Uin", nTemp);
  setUin(nTemp);
  m_fConf.ReadStr("Password", szTemp);
  SetPassword(szTemp);
  m_fConf.ReadBool("WebPresence", bTemp, true);
  if (bTemp) setStatus(m_nStatus | ICQ_STATUS_FxWEBxPRESENCE);
  m_fConf.ReadBool("HideIP", bTemp, false);
  if (bTemp) setStatus(m_nStatus | ICQ_STATUS_FxHIDExIP);


  m_fConf.CloseFile();
  struct timezone tz;
  gettimeofday(NULL, &tz);
  setTimezone(tz.tz_minuteswest / 60);

  if (strlen(Password()) > 8)
  {
    gLog.Error("%sPassword must be 8 characters or less.  Check %s.\n", L_ERRORxSTR, m_fConf.FileName());
    m_bException = true;
    return;
  }

  sprintf(filename, "%s/%s/owner.history", BASE_DIR, HISTORY_DIR);
  setHistoryFile(filename);

  setEnableSave(true);
}


//-----ICQOwner::getBasicInfo---------------------------------------------------
void ICQOwner::getBasicInfo(struct UserBasicInfo &us)
{
  ICQUser::getBasicInfo(us);
}


//-----ICQOwner::getExtInfo----------------------------------------------------------------------
void ICQOwner::getExtInfo(struct UserExtInfo &ud)
{
  ICQUser::getExtInfo(ud);
}


//-----ICQOwner::saveInfo--------------------------------------------------------
void ICQOwner::saveInfo(void)
{
  if (!getEnableSave()) return;

  ICQUser::saveInfo();

  if (!m_fConf.ReloadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
     return;
  }
  m_fConf.SetSection("user");
  m_fConf.WriteNum("Uin", getUin());
  m_fConf.WriteStr("Password", Password());
  m_fConf.WriteBool("WebPresence", getStatusWebPresence());
  m_fConf.WriteBool("HideIP", getStatusHideIp());

  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
               L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
    return;
  }

  m_fConf.CloseFile();
}




