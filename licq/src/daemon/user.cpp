#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_INET_ATON
#include <arpa/inet.h>
#endif

#include "licq_constants.h"
#include "licq_user.h"
#include "licq_countrycodes.h"
#include "licq_languagecodes.h"
#include "licq_log.h"
#include "licq_packets.h"
#include "licq_icqd.h"
#include "support.h"
#include "pthread_rdwr.h"

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
void ICQUser::Unlock()
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
CUserManager::CUserManager() : m_hUsers(USER_HASH_SIZE)
{
  // Set up the basic all users and new users group
  pthread_rdwr_init_np(&mutex_grouplist, NULL);
  pthread_rdwr_init_np(&mutex_userlist, NULL);
  m_nUserListLockType = m_nGroupListLockType = LOCK_N;

  m_xOwner = NULL;
  m_nOwnerUin = 0;
}


CUserManager::~CUserManager()
{
  /* while technically we should do this, nothing interesting happens
     in the user destructor so we don't bother
  UserList::iterator iter;
  for (iter = m_vpcUsers.begin(); iter != m_vpcUsers.end(); iter++)
  {
    delete *iter;
  }
  */

  // Owner destructor saves the current auto response though
  delete m_xOwner;
}


void CUserManager::SetOwnerUin(unsigned long _nUin)
{
  char buf[24];
  sprintf(buf, "%ld", _nUin);
  ICQOwner *o = FetchOwner(LOCK_W);
  o->SetUin(_nUin);
  o->SetAlias(buf);
  DropOwner();
  m_nOwnerUin = _nUin;
}



/*---------------------------------------------------------------------------
 * CUserManager::Load
 *-------------------------------------------------------------------------*/
bool CUserManager::Load()
{
  // Create the owner
  m_xOwner = new ICQOwner;
  if (m_xOwner->Exception())
    return false;
  m_nOwnerUin = m_xOwner->Uin();

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
  char sGroupKey[MAX_KEYxNAME_LEN], sGroupName[MAX_LINE_LEN];
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
  //licqConf.ReadNum("NewUserGroup", m_nNewUserGroup, 0);
  licqConf.CloseFile();

  // Load users from users.conf
  sprintf(filename, "%s/users.conf", BASE_DIR);
  CIniFile usersConf(INI_FxFATAL | INI_FxERROR);
  usersConf.LoadFile(filename);

  unsigned short nUsers;
  usersConf.SetSection("users");
  usersConf.ReadNum("NumOfUsers", nUsers);
  gLog.Info("%sLoading %d users.\n", L_INITxSTR, nUsers);

  char sUserKey[MAX_KEYxNAME_LEN];
  unsigned long nUserUin;
  ICQUser *u;
  usersConf.SetFlags(INI_FxWARN);
  for(unsigned short i = 1; i <= nUsers; i++)
  {
     sprintf(sUserKey, "User%d", i);
     if (!usersConf.ReadNum(sUserKey, nUserUin))
     {
       gLog.Warn("%sSkipping user %i, UIN not found.\n", L_WARNxSTR, i);
       continue;
     }
     if (nUserUin == 0)
     {
       gLog.Warn("%sSkipping user %i, invalid uin %ld.\n", L_WARNxSTR, i, nUserUin);
       continue;
     }
     sprintf(filename, "%s/%s/%li.uin", BASE_DIR, USER_DIR, nUserUin);

     u = new ICQUser(nUserUin, filename);
     u->AddToContactList();
     //u->SetEnableSave(true);
     // Store the user in the hash table
     m_hUsers.Store(u, nUserUin);
     // Add the user to the list
     m_vpcUsers.push_back(u);
  }

  return true;
}


/*---------------------------------------------------------------------------
 * CUserManager::AddUser
 *-------------------------------------------------------------------------*/
unsigned long CUserManager::AddUser(ICQUser *pUser)
{
  pUser->Lock(LOCK_R);
  unsigned long nUin = pUser->Uin();

  // Set this user to be on the contact list
  pUser->AddToContactList();
  //pUser->SetEnableSave(true);
  pUser->SaveLicqInfo();
  pUser->SaveGeneralInfo();
  pUser->SaveMoreInfo();
  pUser->SaveWorkInfo();

  // Store the user in the hash table
  m_hUsers.Store(pUser, nUin);
  // Reorder the user to the correct place
  m_vpcUsers.push_back(pUser);

  pUser->Unlock();

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
  UserList::iterator iter = m_vpcUsers.begin();
  while (iter != m_vpcUsers.end() && u != (*iter)) iter++;
  if (iter == m_vpcUsers.end())
    gLog.Warn("%sInteral Error: CUserManager::RemoveUser():\n"
              "%sUser \"%s\" (%ld) not found in vector.\n",
              L_WARNxSTR, L_BLANKxSTR, u->GetAlias(), u->Uin());
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
  // Don't delete the all users group
  if(n < 1 || n > NumGroups()) {
    return;
  }

  GroupList *g = LockGroupList(LOCK_W);
  // Erase the group from the vector
  m_vszGroups.erase(m_vszGroups.begin()+n-1);

  unsigned short j;
  FOR_EACH_USER_START(LOCK_W)
  {
    for (j = n; j < g->size() + 1; j++)
      pUser->SetInGroup(GROUPS_USER, j, pUser->GetInGroup(GROUPS_USER, j + 1));
  }
  pUser->SetInGroup(GROUPS_USER, j, false);
  FOR_EACH_USER_END;
  if (m_nDefaultGroup >= n) m_nDefaultGroup--;
  //if (m_nNewUserGroup >= n) m_nNewUserGroup--;
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
  //if (m_nNewUserGroup == g1) m_nNewUserGroup = g2;
  //else if (m_nNewUserGroup == g2) m_nNewUserGroup = g1;
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
void CUserManager::SaveGroups()
{
  if (!m_bAllowSave) return;

  // Load the group info from licq.conf
  char filename[MAX_FILENAME_LEN];
  sprintf(filename, "%s/licq.conf", BASE_DIR);
  CIniFile licqConf(INI_FxERROR | INI_FxFATAL);
  licqConf.LoadFile(filename);

  licqConf.SetSection("groups");
  licqConf.WriteNum("NumOfGroups", NumGroups());

  char sGroupKey[MAX_KEYxNAME_LEN];
  //LockGroupList(LOCK_R);
  for (unsigned short i = 0; i < m_vszGroups.size(); i++)
  {
     sprintf(sGroupKey, "Group%d.name", i + 1);
     licqConf.WriteStr(sGroupKey, m_vszGroups[i]);
  }
  //UnlockGroupList();

  licqConf.WriteNum("DefaultGroup", m_nDefaultGroup);
  //licqConf.WriteNum("NewUserGroup", m_nNewUserGroup);
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
    if (_nUin != u->Uin())
      gLog.Error("%sInternal error: CUserManager::FetchUser(): Looked for %ld, found %ld.\n",
                 L_ERRORxSTR, _nUin, u->Uin());
  }
  return u;
}



/*---------------------------------------------------------------------------
 * CUserManager::DropUser
 *-------------------------------------------------------------------------*/
void CUserManager::DropUser(ICQUser *u)
{
  if (u == NULL) return;
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
void CUserManager::DropOwner()
{
  m_xOwner->Unlock();
}


/*---------------------------------------------------------------------------
 * CUserManager::SaveAllUsers
 *-------------------------------------------------------------------------*/
void CUserManager::SaveAllUsers()
{
  FOR_EACH_USER_START(LOCK_R)
  {
    pUser->SaveLicqInfo();
    pUser->SaveGeneralInfo();
    pUser->SaveMoreInfo();
    pUser->SaveWorkInfo();
  }
  FOR_EACH_USER_END
}


/*---------------------------------------------------------------------------
 * CUserManager::NumUsers
 *-------------------------------------------------------------------------*/
unsigned short CUserManager::NumUsers()
{
  //LockUserList(LOCK_R);
  unsigned short n = m_vpcUsers.size();
  //UnlockUserList();
  return n;
}


/*---------------------------------------------------------------------------
 * CUserManager::NumGroups
 *-------------------------------------------------------------------------*/
unsigned short CUserManager::NumGroups()
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
void CUserManager::UnlockUserList()
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
void CUserManager::UnlockGroupList()
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

  unsigned long nUin;
  UserList::iterator iter;
  for (iter = l.begin(); iter != l.end(); iter++)
  {
    (*iter)->Lock(LOCK_R);
    nUin = (*iter)->Uin();
    (*iter)->Unlock();
    if (nUin == _nUin)
    {
      u = (*iter);
      break;
    }
  }
  if (iter == l.end()) u =  NULL;

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
  UserList::iterator iter;
  for (iter = l.begin(); iter != l.end(); iter++)
  {
    (*iter)->Lock(LOCK_R);
    nUin = (*iter)->Uin();
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

void CUserHashTable::Unlock()
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


//-----ICQUser::constructor-----------------------------------------------------
ICQUser::ICQUser(unsigned long _nUin, char *_szFilename)
// Called when first constructing our known users
{
  Init(_nUin);
  m_fConf.SetFlags(INI_FxWARN);
  m_fConf.SetFileName(_szFilename);
  if (!LoadInfo())
  {
    gLog.Error("%sUnable to load user info from '%s'.\n%sUsing default values.\n",
               L_ERRORxSTR, _szFilename, L_BLANKxSTR);
    SetDefaults();
  }
  m_fConf.CloseFile();
  m_fConf.SetFlags(INI_FxWARN | INI_FxALLOWxCREATE);
}


ICQUser::ICQUser(unsigned long nUin)
{
  Init(nUin);
  SetDefaults();
  char szFilename[MAX_FILENAME_LEN];
  sprintf(szFilename, "%s/%s/%ld.uin", BASE_DIR, USER_DIR, nUin);
  m_fConf.SetFileName(szFilename);
  m_fConf.SetFlags(INI_FxWARN | INI_FxALLOWxCREATE);
}


void ICQUser::AddToContactList()
{
  m_bOnContactList = m_bEnableSave = true;

  // Check for old history file
  if (access(m_fHistory.FileName(), F_OK) == -1)
  {
    char szFilename[MAX_FILENAME_LEN];
    sprintf(szFilename, "%s/%s/%ld.%s", BASE_DIR, HISTORY_DIR, m_nUin, HISTORYxOLD_EXT);
    if (access(szFilename, F_OK) == 0)
    {
      if (rename(szFilename, m_fHistory.FileName()) == -1)
      {
        gLog.Warn("%sFailed to rename old history file (%s):\n%s%s\n", L_WARNxSTR,
            szFilename, L_BLANKxSTR, strerror(errno));
      }
    }
  }
}


//-----ICQUser::LoadInfo-----------------------------------------------------
bool ICQUser::LoadInfo()
{
  if (!m_fConf.ReloadFile()) return (false);
  m_fConf.SetFlags(0);
  m_fConf.SetSection("user");

  LoadGeneralInfo();
  LoadMoreInfo();
  LoadWorkInfo();
  LoadAboutInfo();
  LoadLicqInfo();

  return true;
}


//-----ICQUser::LoadGeneralInfo----------------------------------------------
void ICQUser::LoadGeneralInfo()
{
  // read in the fields, checking for errors each time
  char szTemp[MAX_DATA_LEN];
  m_fConf.ReadStr("Alias", szTemp, "Unknown");  SetAlias(szTemp);
  m_fConf.ReadStr("FirstName", szTemp, "");  SetFirstName(szTemp);
  m_fConf.ReadStr("LastName", szTemp, "");  SetLastName(szTemp);
  m_fConf.ReadStr("Email1", szTemp, "");  SetEmail1(szTemp);
  m_fConf.ReadStr("Email2", szTemp, "");  SetEmail2(szTemp);
  m_fConf.ReadStr("City", szTemp, "");  SetCity(szTemp);
  m_fConf.ReadStr("State", szTemp, "");  SetState(szTemp);
  m_fConf.ReadStr("PhoneNumber", szTemp, "");  SetPhoneNumber(szTemp);
  m_fConf.ReadStr("FaxNumber", szTemp, "");  SetFaxNumber(szTemp);
  m_fConf.ReadStr("Address", szTemp, "");  SetAddress(szTemp);
  m_fConf.ReadStr("CellularNumber", szTemp, "");  SetCellularNumber(szTemp);
  m_fConf.ReadNum("Zipcode", m_nZipCode, 0);
  m_fConf.ReadNum("Country", m_nCountryCode, 0);
  m_fConf.ReadNum("Timezone", m_nTimezone, TIMEZONE_UNKNOWN);
  m_fConf.ReadBool("Authorization", m_bAuthorization, false);
  m_fConf.ReadBool("HideEmail", m_bHideEmail, false);
}


//-----ICQUser::LoadMoreInfo-------------------------------------------------
void ICQUser::LoadMoreInfo()
{
  // read in the fields, checking for errors each time
  char szTemp[MAX_DATA_LEN];
  m_fConf.ReadNum("Age", m_nAge, 0);
  m_fConf.ReadNum("Gender", m_nGender, GENDER_UNSPECIFIED);
  m_fConf.ReadStr("Homepage", szTemp, "<none>");  SetHomepage(szTemp);
  m_fConf.ReadNum("BirthYear", m_nBirthYear, 0);
  m_fConf.ReadNum("BirthMonth", m_nBirthMonth, 0);
  m_fConf.ReadNum("BirthDay", m_nBirthDay, 0);
  m_fConf.ReadNum("Language1", m_nLanguage[0], 0);
  m_fConf.ReadNum("Language2", m_nLanguage[1], 0);
  m_fConf.ReadNum("Language3", m_nLanguage[2], 0);
}


//-----ICQUser::LoadWorkInfo-------------------------------------------------
void ICQUser::LoadWorkInfo()
{
  // read in the fields, checking for errors each time
  char szTemp[MAX_DATA_LEN];
  m_fConf.ReadStr("CompanyCity", szTemp, "");  SetCompanyCity(szTemp);
  m_fConf.ReadStr("CompanyState", szTemp, "");  SetCompanyState(szTemp);
  m_fConf.ReadStr("CompanyPhoneNumber", szTemp, "");  SetCompanyPhoneNumber(szTemp);
  m_fConf.ReadStr("CompanyFaxNumber", szTemp, "");  SetCompanyFaxNumber(szTemp);
  m_fConf.ReadStr("CompanyAddress", szTemp, "");  SetCompanyAddress(szTemp);
  m_fConf.ReadStr("CompanyName", szTemp, "");  SetCompanyName(szTemp);
  m_fConf.ReadStr("CompanyDepartment", szTemp, "");  SetCompanyDepartment(szTemp);
  m_fConf.ReadStr("CompanyPosition", szTemp, "");  SetCompanyPosition(szTemp);
  m_fConf.ReadStr("CompanyHomepage", szTemp, "");  SetCompanyHomepage(szTemp);
}


//-----ICQUser::LoadAboutInfo-------------------------------------------------
void ICQUser::LoadAboutInfo()
{
  // read in the fields, checking for errors each time
  char szTemp[MAX_DATA_LEN];
  m_fConf.SetSection("user");
  m_fConf.ReadStr("About", szTemp, ""); SetAbout(szTemp);
}

//-----ICQUser::LoadLicqInfo-------------------------------------------------
void ICQUser::LoadLicqInfo()
{
  // read in the fields, checking for errors each time
  char szTemp[MAX_DATA_LEN];
  unsigned short nNewMessages;
  unsigned long nLastOnline;
  m_fConf.SetSection("user");
  m_fConf.ReadNum("Groups.System", m_nGroups[GROUPS_SYSTEM], 0);
  m_fConf.ReadNum("Groups.User", m_nGroups[GROUPS_USER], 0);
  m_fConf.ReadStr("Ip", szTemp, "0.0.0.0");
  struct in_addr in;
  m_nIp = inet_aton(szTemp, &in);
  if (m_nIp != 0) m_nIp = in.s_addr;
  m_fConf.ReadNum("Port", m_nPort, 0);
  //m_fConf.ReadBool("NewUser", m_bNewUser, false);
  m_fConf.ReadNum("NewMessages", nNewMessages, 0);
  m_fConf.ReadNum("LastOnline", nLastOnline, 0);
  m_nLastOnline = nLastOnline;
  m_fConf.ReadNum("AutoAccept", m_nAutoAccept, 0);
  m_fConf.ReadNum("StatusToUser", m_nStatusToUser, ICQ_STATUS_OFFLINE);
  m_fConf.ReadStr("CustomAutoRsp", szTemp, "");
  m_fConf.ReadBool("SendRealIp", m_bSendRealIp, false);
  SetCustomAutoResponse(szTemp);

  m_fConf.ReadStr("History", szTemp, "default");
  if (szTemp[0] == '\0') strcpy(szTemp, "default");
  SetHistoryFile(szTemp);

  if (nNewMessages > 0)
  {
    HistoryList hist;
    if (GetHistory(hist))
    {
      HistoryListIter it;
      if (hist.size() < nNewMessages)
        it = hist.begin();
      else
      {
        it = hist.end();
        while (nNewMessages > 0 && it != hist.begin())
        {
          it--;
          nNewMessages--;
        }
      }
      while (it != hist.end())
      {
        m_vcMessages.push_back( (*it)->Copy() );
        incNumUserEvents();
        it++;
      }
    }
    ClearHistory(hist);
  }
}


//-----ICQUser::destructor------------------------------------------------------
ICQUser::~ICQUser()
{
  while (NewMessages() > 0) delete EventPop();

  /* FIXME memory leak, but requires calling free, and that's a lot of typing
  m_szAutoResponse = NULL;
  m_szAlias = NULL;
  m_szFirstName = NULL;
  m_szLastName = NULL;
  m_szEmail1 = NULL;
  m_szEmail2 = NULL;
  m_szCity = NULL;
  m_szState = NULL;
  m_szPhoneNumber = NULL;
  m_szFaxNumber = NULL;
  m_szAddress = NULL;
  m_szCellularNumber = NULL;
  m_szHomepage = NULL;
  m_szCompanyCity = NULL;
  m_szCompanyState = NULL;
  m_szCompanyPhoneNumber = NULL;
  m_szCompanyFaxNumber = NULL;
  m_szCompanyAddress = NULL;
  m_szCompanyName = NULL;
  m_szCompanyDepartment = NULL;
  m_szCompanyPosition = NULL;
  m_szCompanyHomepage = NULL;
  m_szAbout = NULL;
  m_szCustomAutoResponse = NULL;*/

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
void ICQUser::RemoveFiles()
{
  remove(m_fConf.FileName());

  // Check for old history file and back up
  struct stat buf;
  if (stat(m_fHistory.FileName(), &buf) == 0 && buf.st_size > 0)
  {
    char szFilename[MAX_FILENAME_LEN];
    sprintf(szFilename, "%s/%s/%ld.%s", BASE_DIR, HISTORY_DIR, m_nUin, HISTORYxOLD_EXT);
    if (rename(m_fHistory.FileName(), szFilename) == -1)
    {
      gLog.Warn("%sFailed to rename history file (%s):\n%s%s\n", L_WARNxSTR,
          szFilename, L_BLANKxSTR, strerror(errno));
      remove(m_fHistory.FileName());
    }
  }
}


void ICQUser::Init(unsigned long _nUin)
{
  //SetOnContactList(false);
  m_bOnContactList = m_bEnableSave = false;
  m_szAutoResponse = NULL;

  // General Info
  m_szAlias = NULL;
  m_szFirstName = NULL;
  m_szLastName = NULL;
  m_szEmail1 = NULL;
  m_szEmail2 = NULL;
  m_szCity = NULL;
  m_szState = NULL;
  m_szPhoneNumber = NULL;
  m_szFaxNumber = NULL;
  m_szAddress = NULL;
  m_szCellularNumber = NULL;
  m_nZipCode = 0;
  m_nCountryCode = COUNTRY_UNSPECIFIED;
  m_nTimezone = TIMEZONE_UNKNOWN;
  m_bAuthorization = false;
  m_bHideEmail = false;

  // More Info
  m_nAge = 0xffff;
  m_nGender = 0;
  m_szHomepage = NULL;
  m_nBirthYear = 0;
  m_nBirthMonth = 0;
  m_nBirthDay = 0;
  m_nLanguage[0] = 0;
  m_nLanguage[1] = 0;
  m_nLanguage[2] = 0;

  // Work Info
  m_szCompanyCity = NULL;
  m_szCompanyState = NULL;
  m_szCompanyPhoneNumber = NULL;
  m_szCompanyFaxNumber = NULL;
  m_szCompanyAddress = NULL;
  m_szCompanyName = NULL;
  m_szCompanyDepartment = NULL;
  m_szCompanyPosition = NULL;
  m_szCompanyHomepage = NULL;

  // About
  m_szAbout = NULL;

  m_nUin = _nUin;
  SetStatus(ICQ_STATUS_OFFLINE);
  SetAutoResponse("");
  SetSendServer(false);
  SetSendRealIp(false);
  SetShowAwayMsg(false);
  SetSequence(0xFFFFFFFF);
  SetOfflineOnDisconnect(false);
  ClearSocketDesc();
  m_nIp = m_nPort = m_nRealIp = 0;
  m_nMode = MODE_DIRECT;
  m_nVersion = 0x03;
  Touch();
  m_nLastOnline = 0;
  m_nStatusToUser = ICQ_STATUS_OFFLINE;
  m_nAutoAccept = 0;
  m_szCustomAutoResponse = NULL;

  pthread_rdwr_init_np (&mutex_rw, NULL);
}

//-----ICQUser::SetDefaults-----------------------------------------------------
void ICQUser::SetDefaults()
{
  char szTemp[12];
  sprintf(szTemp, "%ld", Uin());
  SetAlias(szTemp);
  SetHistoryFile("default");
  //SetGroups(GROUPS_SYSTEM, 0);
  //SetGroups(GROUPS_USER, gUserManager.NewUserGroup());
  SetGroups(GROUPS_USER, 0);
  SetGroups(GROUPS_SYSTEM, 0);
  SetNewUser(true);
  SetAuthorization(false);
  SetNewUser(true);

  szTemp[0] = '\0';
  SetFirstName(szTemp);
  SetLastName(szTemp);
  SetEmail1(szTemp);
  SetEmail2(szTemp);
  SetCity(szTemp);
  SetState(szTemp);
  SetPhoneNumber(szTemp);
  SetFaxNumber(szTemp);
  SetAddress(szTemp);
  SetCellularNumber(szTemp);
  SetHomepage(szTemp);
  SetCompanyCity(szTemp);
  SetCompanyState(szTemp);
  SetCompanyPhoneNumber(szTemp);
  SetCompanyFaxNumber(szTemp);
  SetCompanyAddress(szTemp);
  SetCompanyName(szTemp);
  SetCompanyDepartment(szTemp);
  SetCompanyPosition(szTemp);
  SetCompanyHomepage(szTemp);
  SetAbout(szTemp);
  SetCustomAutoResponse(szTemp);
}



unsigned short ICQUser::Status()
// guarantees to return a unique status that switch can be run on
{
   if (StatusOffline()) return ICQ_STATUS_OFFLINE;
   else if (m_nStatus & ICQ_STATUS_DND) return ICQ_STATUS_DND;
   else if (m_nStatus & ICQ_STATUS_OCCUPIED) return ICQ_STATUS_OCCUPIED;
   else if (m_nStatus & ICQ_STATUS_NA) return ICQ_STATUS_NA;
   else if (m_nStatus & ICQ_STATUS_AWAY) return ICQ_STATUS_AWAY;
   else if (m_nStatus & ICQ_STATUS_FREEFORCHAT) return ICQ_STATUS_FREEFORCHAT;
   else if ((m_nStatus & 0xFF) == 0x00) return ICQ_STATUS_ONLINE;
   else return (ICQ_STATUS_OFFLINE - 1);
}

void ICQUser::SetStatusOffline()
{
  if (!StatusOffline())
  {
    m_nLastOnline = time(NULL);
    SaveLicqInfo();
  }
  SetStatus(m_nStatus | ICQ_STATUS_OFFLINE);
}


unsigned long ICQUser::Sequence(bool increment)
{
   if (increment)
      return (m_nSequence--);
   else
      return (m_nSequence);
}

void ICQUser::SetAlias(const char *s)
{
  if (s[0] == '\0')
  {
    if (m_szFirstName != NULL && m_szFirstName[0] != '\0')
      SetString(&m_szAlias, m_szFirstName);
    else
    {
      char sz[12];
      sprintf(sz, "%ld", Uin());
      SetString(&m_szAlias, sz);
    }
  }
  else
    SetString(&m_szAlias, s);

  SaveGeneralInfo();
}


bool ICQUser::Away()
{
   unsigned short n = Status();
   return (n == ICQ_STATUS_AWAY || n == ICQ_STATUS_NA ||
           n == ICQ_STATUS_DND || n == ICQ_STATUS_OCCUPIED);
}

void ICQUser::SetHistoryFile(const char *s)
{
  m_fHistory.SetFile(s, m_nUin);
  SaveLicqInfo();
}


void ICQUser::SetIpPort(unsigned long _nIp, unsigned short _nPort)
{
  if (SocketDesc() != -1 &&
      ( (Ip() != 0 && Ip() != _nIp) || (Port() != 0 && Port() != _nPort)) )
  {
    // Close our socket, but don't let socket manager try and clear
    // our socket descriptor
    gSocketManager.CloseSocket(SocketDesc(), false);
    ClearSocketDesc();
  }
  m_nIp = _nIp;
  m_nPort = _nPort;
  SaveLicqInfo();
}


inline int ICQUser::LocalTimeGMTOffset()
{
  return GetTimezone() * 1800;
}


int ICQUser::SystemTimeGMTOffset()
{
  time_t t = time(NULL);
  struct tm *tzone = localtime(&t);
#ifdef USE_GMTOFF
  return -(tzone->tm_gmtoff); // seconds _east_ of UTC
#elif defined(USE_TIMEZONE)
  return timezone - (tzone->tm_isdst == 1 ? 3600 : 0); // seconds _west_ of UTC
#else
#warning Unable to determine local timezone
  return 0;
#endif
}

char ICQUser::SystemTimezone()
{
  char nTimezone = SystemTimeGMTOffset() / 1800;
  if (nTimezone > 23)
    return 23 - nTimezone;
  return nTimezone;
}


int ICQUser::LocalTimeOffset()
{
  return SystemTimeGMTOffset() - LocalTimeGMTOffset();
}


inline time_t ICQUser::LocalTime()
{
  return time(NULL) + LocalTimeOffset();
}


const char *ICQUser::StatusStr()
{
  return StatusToStatusStr(m_nStatus, StatusInvisible());
}

const char *ICQUser::StatusStrShort()
{
  return StatusToStatusStrShort(m_nStatus, StatusInvisible());
}


const char *ICQUser::StatusToStatusStr(unsigned short n, bool b)
{
  if (n == ICQ_STATUS_OFFLINE) return b ? "(Offline)" : "Offline";
  else if (n & ICQ_STATUS_DND) return b ? "(Do Not Disturb)" : "Do Not Disturb";
  else if (n & ICQ_STATUS_OCCUPIED) return b ? "(Occupied)" : "Occupied";
  else if (n & ICQ_STATUS_NA) return b ? "(Not Available)" : "Not Available";
  else if (n & ICQ_STATUS_AWAY) return b ? "(Away)" : "Away";
  else if (n & ICQ_STATUS_FREEFORCHAT) return b ? "(Free for Chat)" : "Free for Chat";
  else if (n << 24 == 0x00) return b ? "(Online)" : "Online";
  else return "Unknown";
}

const char *ICQUser::StatusToStatusStrShort(unsigned short n, bool b)
{
  if (n == ICQ_STATUS_OFFLINE) return b ? "(Off)" : "Off";
  else if (n & ICQ_STATUS_DND) return b ? "(DND)" : "DND";
  else if (n & ICQ_STATUS_OCCUPIED) return b ? "(Occ)" : "Occ";
  else if (n & ICQ_STATUS_NA) return b ? "(N/A)" : "N/A";
  else if (n & ICQ_STATUS_AWAY) return b ? "(Away)" : "Away";
  else if (n & ICQ_STATUS_FREEFORCHAT) return b ? "(FFC)" : "FFC";
  else if (n << 24 == 0x00) return b ? "(On)" : "On";
  else return "???";
}


char *ICQUser::IpPortStr(char *rbuf)
{
  // Track down the current ip and port
  char buf[32], ip[32], port[32];
  if (SocketDesc() > 0)    // First check if we are connected
  {
    INetSocket *s = gSocketManager.FetchSocket(SocketDesc());
    if (s != NULL)
    {
      if (User())
      {
        strcpy(ip, s->RemoteIpStr(buf));
        sprintf(port, "%d", s->RemotePort());
      }
      else
      {
        strcpy(ip, s->LocalIpStr(buf));
        sprintf(port, "%d", s->LocalPort());
      }
      gSocketManager.DropSocket(s);
    }
    else
    {
      strcpy(ip, "invalid");
      strcpy(port, "invalid");
    }
  }
  else
  {
    if (Ip() > 0)     // Default to the given ip
      strcpy(ip, inet_ntoa_r(*(struct in_addr *)&m_nIp, buf));
    else                   // Otherwise we don't know
      strcpy(ip, "???");

    if (Port() > 0)
      sprintf(port, "%d", Port());
    else
      strcpy(port, "??");
  }

  if (StatusHideIp())
  {
    strcpy(buf, ip);
    sprintf(ip, "(%s)", buf);
  }
  sprintf(rbuf, "%s:%s", ip, port);
  return rbuf;
}


void ICQUser::usprintf(char *_sz, const char *_szFormat, bool bAllowFieldWidth)
{
  bool bLeft = false;
  unsigned short i = 0, j, nField = 0, nPos = 0;
  char szTemp[128];
  const char *sz;
  _sz[0] = '\0';
  while(_szFormat[i] != '\0')
  {
    if (_szFormat[i] == '`')
    {
        _sz[nPos++] = '`';
        i++;
        while(_szFormat[i] != '`' && _szFormat[i] != '\0')
            _sz[nPos++] = _szFormat[i++];
        _sz[nPos++] = '`';
        i++;
    }
    else if (_szFormat[i] == '%')
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
        szTemp[j] = '\0';
        if (j > 0) nField = atoi(szTemp);
      }
      else
      {
        if (isdigit(_szFormat[i]))
        {
          _sz[nPos++] = _szFormat[i - 1];
          _sz[nPos++] = _szFormat[i++];
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
        sz = GetEmail1();
        break;
      case 'n':
        sprintf(szTemp, "%s %s", GetFirstName(), GetLastName());
        sz = szTemp;
        break;
      case 'f':
        sz = GetFirstName();
        break;
      case 'l':
        sz = GetLastName();
        break;
      case 'a':
        sz = GetAlias();
        break;
      case 'u':
        sprintf(szTemp, "%ld", Uin());
        sz = szTemp;
        break;
      case 'w':
        sz = GetHomepage();
        break;
      case 'h':
        sz = GetPhoneNumber();
        break;
      case 'S':
        sz = StatusStrShort();
        break;
      case 's':
        sz = StatusStr();
        break;
      case 'o':
      {
        //strftime(szTemp, 128, "%c", localtime(&t));
        strftime(szTemp, 128, "%b %d %R", localtime(&m_nLastOnline));
        sz = szTemp;
        break;
      }
      default:
        gLog.Warn("%sWarning: Invalid qualifier in command: %%%c.\n",
                  L_WARNxSTR, _szFormat[i]);
        sprintf(szTemp, "%s%d%%%c", (bLeft ? "-" : ""), nField, _szFormat[i]);
        sz = szTemp;
        bLeft = false;
        nField = 0;
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
          if (nLen < 0)
          {
            j = 0;
            while(j < nField) _sz[nPos++] = sz[j++];
          }
          else
          {
            for (j = 0; j < nLen; j++) _sz[nPos++] = ' ';
            j = 0;
            while(sz[j] != '\0') _sz[nPos++] = sz[j++];
          }
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


//-----ICQUser::SaveGeneralInfo----------------------------------------------
void ICQUser::SaveGeneralInfo()
{
  if (!EnableSave()) return;

  if (!m_fConf.ReloadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(),  L_BLANKxSTR);
     return;
  }
  m_fConf.SetSection("user");
  m_fConf.WriteStr("Alias", m_szAlias);
  m_fConf.WriteStr("FirstName", m_szFirstName);
  m_fConf.WriteStr("LastName", m_szLastName);
  m_fConf.WriteStr("Email1", m_szEmail1);
  m_fConf.WriteStr("Email2", m_szEmail2);
  m_fConf.WriteStr("City", m_szCity);
  m_fConf.WriteStr("State", m_szState);
  m_fConf.WriteStr("PhoneNumber", m_szPhoneNumber);
  m_fConf.WriteStr("FaxNumber", m_szFaxNumber);
  m_fConf.WriteStr("Address", m_szAddress);
  m_fConf.WriteStr("CellularNumber", m_szCellularNumber);
  m_fConf.WriteNum("Zipcode", m_nZipCode);
  m_fConf.WriteNum("Country", m_nCountryCode);
  m_fConf.WriteNum("Timezone", m_nTimezone);
  m_fConf.WriteBool("Authorization", m_bAuthorization);
  m_fConf.WriteBool("HideEmail", m_bHideEmail);

  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
               L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
    return;
  }

  m_fConf.CloseFile();
}


//-----ICQUser::SaveMoreInfo----------------------------------------------
void ICQUser::SaveMoreInfo()
{
  if (!EnableSave()) return;

  if (!m_fConf.ReloadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(),  L_BLANKxSTR);
     return;
  }
  m_fConf.SetSection("user");
  m_fConf.WriteNum("Age", m_nAge);
  m_fConf.WriteNum("Gender", m_nGender);
  m_fConf.WriteStr("Homepage", m_szHomepage);
  m_fConf.WriteNum("BirthYear", m_nBirthYear);
  m_fConf.WriteNum("BirthMonth", m_nBirthMonth);
  m_fConf.WriteNum("BirthDay", m_nBirthDay);
  m_fConf.WriteNum("Language1", m_nLanguage[0]);
  m_fConf.WriteNum("Language2", m_nLanguage[1]);
  m_fConf.WriteNum("Language3", m_nLanguage[2]);

  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
               L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
    return;
  }

  m_fConf.CloseFile();
}


//-----ICQUser::SaveWorkInfo----------------------------------------------
void ICQUser::SaveWorkInfo()
{
  if (!EnableSave()) return;

  if (!m_fConf.ReloadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(),  L_BLANKxSTR);
     return;
  }
  m_fConf.SetSection("user");
  m_fConf.WriteStr("CompanyCity", m_szCompanyCity);
  m_fConf.WriteStr("CompanyState", m_szCompanyState);
  m_fConf.WriteStr("CompanyPhoneNumber", m_szCompanyPhoneNumber);
  m_fConf.WriteStr("CompanyFaxNumber", m_szCompanyFaxNumber);
  m_fConf.WriteStr("CompanyAddress", m_szCompanyAddress);
  m_fConf.WriteStr("CompanyName", m_szCompanyName);
  m_fConf.WriteStr("CompanyDepartment", m_szCompanyDepartment);
  m_fConf.WriteStr("CompanyPosition", m_szCompanyPosition);
  m_fConf.WriteStr("CompanyHomepage", m_szCompanyHomepage);

  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
               L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
    return;
  }

  m_fConf.CloseFile();
}


//-----ICQUser::SaveAboutInfo-------------------------------------------------
void ICQUser::SaveAboutInfo()
{
   if (!EnableSave()) return;

   if (!m_fConf.ReloadFile())
   {
      gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                 L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
      return;
   }
   m_fConf.SetSection("user");
   m_fConf.WriteStr("About", m_szAbout);
   if (!m_fConf.FlushFile())
   {
     gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
     return;
   }

   m_fConf.CloseFile();
}


//-----ICQUser::SaveLicqInfo-------------------------------------------------
void ICQUser::SaveLicqInfo()
{
   if (!EnableSave()) return;

   if (!m_fConf.ReloadFile())
   {
      gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                 L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
      return;
   }
   char buf[64];
   m_fConf.SetSection("user");
   m_fConf.WriteStr("History", HistoryName());
   m_fConf.WriteNum("Groups.System", GetGroups(GROUPS_SYSTEM));
   m_fConf.WriteNum("Groups.User", GetGroups(GROUPS_USER));
   m_fConf.WriteStr("Ip", inet_ntoa_r(*(struct in_addr *)&m_nIp, buf));
   m_fConf.WriteNum("Port", Port());
   //m_fConf.WriteBool("NewUser", NewUser());
   m_fConf.WriteNum("NewMessages", NewMessages());
   m_fConf.WriteNum("LastOnline", (unsigned long)LastOnline());
   m_fConf.WriteNum("AutoAccept", m_nAutoAccept);
   m_fConf.WriteNum("StatusToUser", m_nStatusToUser);
   m_fConf.WriteStr("CustomAutoRsp", CustomAutoResponse());
   m_fConf.WriteBool("SendRealIp", m_bSendRealIp);

   if (!m_fConf.FlushFile())
   {
     gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
     return;
   }

   m_fConf.CloseFile();
}



void ICQUser::SaveNewMessagesInfo()
{
   if (!EnableSave()) return;

   if (!m_fConf.ReloadFile())
   {
      gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                 L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
      return;
   }
   m_fConf.SetSection("user");
   m_fConf.WriteNum("NewMessages", NewMessages());
   if (!m_fConf.FlushFile())
   {
     gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
     return;
   }

   m_fConf.CloseFile();
}



//-----ICQUser::SaveBasicInfo---------------------------------------------------
void ICQUser::SaveBasicInfo()
{
  if (!EnableSave()) return;

  if (!m_fConf.ReloadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(),  L_BLANKxSTR);
     return;
  }
  m_fConf.SetSection("user");
  m_fConf.WriteStr("Alias", GetAlias());
  m_fConf.WriteStr("FirstName", GetFirstName());
  m_fConf.WriteStr("LastName", GetLastName());
  m_fConf.WriteStr("Email1", GetEmail1());
  m_fConf.WriteBool("Authorization", GetAuthorization());
  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
               L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
    return;
  }

  m_fConf.CloseFile();
}

/*
//-----ICQUser::saveInfo--------------------------------------------------------
void ICQUser::saveInfo()
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
*/

//-----ICQUser::SaveExtInfo--------------------------------------------------
void ICQUser::SaveExtInfo()
{
   if (!EnableSave()) return;

   if (!m_fConf.ReloadFile())
   {
      gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
                 L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
      return;
   }
   m_fConf.SetSection("user");
   m_fConf.WriteStr("Homepage", GetHomepage());
   m_fConf.WriteStr("City", GetCity());
   m_fConf.WriteStr("State", GetState());
   m_fConf.WriteNum("Country", GetCountryCode());
   m_fConf.WriteNum("Timezone", (signed short)GetTimezone());
   m_fConf.WriteNum("Zipcode", GetZipCode());
   m_fConf.WriteStr("PhoneNumber", GetPhoneNumber());
   m_fConf.WriteNum("Age", GetAge());
   m_fConf.WriteNum("Sex", (unsigned short)GetGender());
   m_fConf.WriteStr("About", GetAbout());
   if (!m_fConf.FlushFile())
   {
     gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
     return;
   }

   m_fConf.CloseFile();

}


//-----ICQUser::EventPush--------------------------------------------------------
void ICQUser::EventPush(CUserEvent *e)
{
  m_vcMessages.push_back(e);
  incNumUserEvents();
  SaveNewMessagesInfo();
  Touch();

  gLicqDaemon->PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
     USER_EVENTS, m_nUin, e->Id()));
}


void ICQUser::WriteToHistory(const char *_szText)
{
  m_fHistory.Append(_szText);
}



//-----ICQUser::GetEvent--------------------------------------------------------
CUserEvent *ICQUser::EventPeek(unsigned short index)
{
  if (index >= NewMessages()) return (NULL);
  return (m_vcMessages[index]);
}

CUserEvent *ICQUser::EventPeekId(int id)
{
  if (m_vcMessages.size() == 0) return NULL;
  CUserEvent *e = NULL;
  UserEventList::iterator iter;
  for (iter = m_vcMessages.begin(); iter != m_vcMessages.end(); iter++)
  {
    if ((*iter)->Id() == id)
    {
      e = *iter;
      break;
    }
  }
  return e;
}

//-----ICQUser::EventPeekLast----------------------------------------------------
CUserEvent *ICQUser::EventPeekLast()
{
  if (m_vcMessages.size() == 0) return (NULL);
  return (m_vcMessages[m_vcMessages.size() - 1]);
}

//-----ICQUser::EventPeekFirst----------------------------------------------------
CUserEvent *ICQUser::EventPeekFirst()
{
  if (m_vcMessages.size() == 0) return (NULL);
  return (m_vcMessages[0]);
}

//-----ICQUser::EventPop-----------------------------------------------------
CUserEvent *ICQUser::EventPop()
{
  if (m_vcMessages.size() == 0) return NULL;
  CUserEvent *e = m_vcMessages[0];
  for (unsigned short i = 0; i < m_vcMessages.size() - 1; i++)
    m_vcMessages[i] = m_vcMessages[i + 1];
  m_vcMessages.pop_back();
  decNumUserEvents();
  SaveNewMessagesInfo();

  gLicqDaemon->PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
     USER_EVENTS, m_nUin, e->Id()));

  return e;
}

//-----ICQUser::EventClear----------------------------------------------------
void ICQUser::EventClear(unsigned short index)
{
  if (index >= m_vcMessages.size()) return;

  unsigned long id = m_vcMessages[index]->Id();

  delete m_vcMessages[index];
  for (unsigned short i = index; i < m_vcMessages.size() - 1; i++)
    m_vcMessages[i] = m_vcMessages[i + 1];
  m_vcMessages.pop_back();
  decNumUserEvents();
  SaveNewMessagesInfo();

  gLicqDaemon->PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
     USER_EVENTS, m_nUin, id));
}


void ICQUser::EventClearId(int id)
{
  UserEventList::iterator iter;
  for (iter = m_vcMessages.begin(); iter != m_vcMessages.end(); iter++)
  {
    if ((*iter)->Id() == id)
    {
      delete *iter;
      m_vcMessages.erase(iter);
      decNumUserEvents();
      SaveNewMessagesInfo();
      gLicqDaemon->PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
         USER_EVENTS, m_nUin, id));
      break;
    }
  }
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


unsigned short ICQUser::getNumUserEvents()
{
  pthread_mutex_lock(&mutex_nNumUserEvents);
  unsigned short n = s_nNumUserEvents;
  pthread_mutex_unlock(&mutex_nNumUserEvents);
  return n;
}

void ICQUser::incNumUserEvents()
{
  pthread_mutex_lock(&mutex_nNumUserEvents);
  s_nNumUserEvents++;
  pthread_mutex_unlock(&mutex_nNumUserEvents);
}

void ICQUser::decNumUserEvents()
{
  pthread_mutex_lock(&mutex_nNumUserEvents);
  s_nNumUserEvents--;
  pthread_mutex_unlock(&mutex_nNumUserEvents);
}


//=====ICQOwner=================================================================

//-----ICQOwner::constructor----------------------------------------------------
ICQOwner::ICQOwner()
{
  gLog.Info("%sOwner configuration.\n", L_INITxSTR);
  char szTemp[MAX_DATA_LEN];
  char filename[MAX_FILENAME_LEN];
  m_bException = false;
  m_szPassword = NULL;

  Init(0);
  //SetOnContactList(true);
  m_bOnContactList = true;

  // Get data from the config file
  sprintf(filename, "%s/owner.uin", BASE_DIR);

  // Make sure owner.uin is mode 0600
  if (chmod(filename, S_IRUSR | S_IWUSR) == -1)
  {
    gLog.Warn("%sUnable to set %s to mode 0600.  Your ICQ password is vulnerable.\n",
                 L_WARNxSTR, filename);
  }

  m_fConf.SetFileName(filename);
  LoadInfo();
  m_fConf.ReadNum("Uin", m_nUin);
  m_fConf.ReadStr("Password", szTemp);
  SetPassword(szTemp);
  m_fConf.ReadBool("WebPresence", m_bWebAware, false);
  m_fConf.ReadBool("HideIP", m_bHideIp, false);
  m_fConf.ReadNum("RCG", m_nRandomChatGroup, ICQ_RANDOMxCHATxGROUP_NONE);
  m_fConf.ReadStr("AutoResponse", szTemp, "");
  SetAutoResponse(szTemp);

  m_fConf.CloseFile();

  if (strlen(Password()) > 8)
  {
    gLog.Error("%sPassword must be 8 characters or less.  Check %s.\n", L_ERRORxSTR, m_fConf.FileName());
    m_bException = true;
    return;
  }

  sprintf(filename, "%s/%s/owner.history", BASE_DIR, HISTORY_DIR);
  SetHistoryFile(filename);

  if (m_nTimezone != SystemTimezone())
  {
    gLog.Warn("%sCurrent Licq GMT offset (%d) does not match system GMT offset (%d).\n"
              "%sUpdate general info on server to fix.\n",
       L_WARNxSTR, m_nTimezone, SystemTimezone(), L_BLANKxSTR);
  }

  SetEnableSave(true);
}


ICQOwner::~ICQOwner()
{
  // Save the current auto response
  if (!m_fConf.ReloadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
     return;
  }
  m_fConf.SetSection("user");
  m_fConf.WriteStr("AutoResponse", AutoResponse());
  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
               L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
    return;
  }
  m_fConf.CloseFile();

}



unsigned long ICQOwner::AddStatusFlags(unsigned long s)
{
  s &= 0x0000FFFF;

  if (WebAware())
    s |= ICQ_STATUS_FxWEBxPRESENCE;
  if (HideIp())
    s |= ICQ_STATUS_FxHIDExIP;

  // check if today is user's birthday
  time_t t = LocalTime();
  struct tm *tzone = localtime(&t);
  if(tzone->tm_mon+1 == GetBirthMonth() &&
     tzone->tm_mday == GetBirthDay())
    s |= ICQ_STATUS_FxBIRTHDAY;

  return s;
}


//-----ICQOwner::saveInfo--------------------------------------------------------
void ICQOwner::SaveLicqInfo()
{
  if (!EnableSave()) return;

  ICQUser::SaveLicqInfo();

  if (!m_fConf.ReloadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
     return;
  }
  m_fConf.SetSection("user");
  m_fConf.WriteNum("Uin", Uin());
  m_fConf.WriteStr("Password", Password());
  m_fConf.WriteBool("WebPresence", WebAware());
  m_fConf.WriteBool("HideIP", HideIp());
  m_fConf.WriteNum("RCG", RandomChatGroup());

  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
               L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
    return;
  }

  m_fConf.CloseFile();
}


void ICQUser::StupidLinkageFix()
  {  printf("%s", gCountries[0].szName);
     printf("%s", gLanguages[0].szName); }

