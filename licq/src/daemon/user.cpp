#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <netinet/in.h>
#include <math.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef HAVE_INET_ATON
#include <arpa/inet.h>
#endif

#include "support.h"
#include "user.h"
#include "countrycodes.h"
#include "log.h"
#include "icqpacket.h"

class CUserManager gUserManager;

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
  SetLockType(_nLockType);
}

void ICQUser::Unlock(void)
{
  unsigned short nLockType = LockType();
  SetLockType(LOCK_R);
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



//=====CUserGroup===============================================================
CUserGroup::CUserGroup(char *_sName, bool _bOwner)
{
  m_szName = new char[strlen(_sName) + 1];
  strcpy(m_szName, _sName);
  m_bIsOwner = _bOwner;
  pthread_rdwr_init_np (&mutex_rw, NULL);
}

CUserGroup::~CUserGroup(void)
{
  delete[] m_szName;
  // if we are the owner of the pointers delete all the users
  ICQUser *u = NULL;
  if (m_bIsOwner)
    for (unsigned short i = 0; i < NumUsers(); i++)
    {
      u = FetchUser(i, LOCK_W);
      delete u;
    }

  // Destroy the mutex
/*  int nResult = 0;
  do
  {
    pthread_rdwr_wlock_np(&mutex_rw);
    pthread_rdwr_wunlock(&mutex_rw);
    nResult = pthread_mutex_destroy(&mutex);
  } while (nResult != 0);*/
}


//-----CUserGroup::AddUser------------------------------------------------------
void CUserGroup::AddUser(ICQUser *_pcUser)
{
  m_vpcUsers.push_back(_pcUser);
  Reorder(_pcUser);
}

//-----CUserGroup::RemoveUser---------------------------------------------------
void CUserGroup::RemoveUser(ICQUser *_pcUser)
{
  vector<ICQUser *>::iterator iter = m_vpcUsers.begin();
  while (iter != m_vpcUsers.end() && _pcUser != (*iter)) iter++;
  if (iter == m_vpcUsers.end())
  {
    gLog.Warn("%sAttempt to remove user (%s) from group (%s) they do not belong to.\n",
              L_WARNxSTR, _pcUser->getAlias(), Name());
  }
  else
    m_vpcUsers.erase(iter);
}


//-----CUserGroup::ShiftUser----------------------------------------------------
void CUserGroup::ShiftUserUp(unsigned short i)
{
   if (i < NumUsers() - 1) m_vpcUsers[i] = m_vpcUsers[i + 1];
}

void CUserGroup::ShiftUserDown(unsigned short i)
{
   if (i < NumUsers() && i > 0) m_vpcUsers[i] = m_vpcUsers[i - 1];
}


//-----CUserGroup::FetchUser----------------------------------------------------
ICQUser *CUserGroup::FetchUser(unsigned short i, unsigned short _nLockType)
{
  if (i >= NumUsers())
    return NULL;
  else
  {
    m_vpcUsers[i]->Lock(_nLockType);
    return m_vpcUsers[i];
  }
}

void CUserGroup::DropUser(ICQUser *u)
{
  if (u == NULL) return;
  u->Unlock();
}

//-----CUserGroup---------------------------------------------------------------
void CUserGroup::Reorder(ICQUser *_pcUser)
{
  unsigned short nPos = 0;
  while (nPos < NumUsers() && _pcUser != m_vpcUsers[nPos])
     nPos++;
  if (nPos == NumUsers() || NumUsers() == 1) return;

  // Now nPos is the position of the user to be reordered
  // => _pcUser = m_vpcUsers[nPos]

  bool bGoingDown = false;
  ICQUser *u = NULL;
  pthread_mutex_lock(&ICQUser::mutex_eSortKey);
  if (nPos != 0)
  {
    u = FetchUser(nPos - 1, LOCK_R);
    bGoingDown = _pcUser->SortKey() <= u->SortKey();
    DropUser(u);
  }
  unsigned long nSortKey;
  if (nPos != 0 && bGoingDown)
  {
     // Move user down the list (up the gui list)
     unsigned short i;
     for (i = nPos; i > 0; i--)
     {
        u = FetchUser(i - 1, LOCK_R);
        nSortKey = u->SortKey();
        DropUser(u);
        if (_pcUser->SortKey() > nSortKey) break;
        ShiftUserDown(i);
     }
     m_vpcUsers[i] = _pcUser;
  }
  else
  {
     // Move user up the list
     unsigned short i;
     for (i = nPos; i < NumUsers() - 1; i++)
     {
        u = FetchUser(i + 1, LOCK_R);
        nSortKey = u->SortKey();
        DropUser(u);
        if (_pcUser->SortKey() <= nSortKey) break;
        ShiftUserUp(i);
     }
     m_vpcUsers[i] = _pcUser;
  }
  pthread_mutex_unlock(&ICQUser::mutex_eSortKey);
}

//-----CUserGroup::Lock---------------------------------------------------------
void CUserGroup::Lock(unsigned short _nLockType)
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


//-----CUserGroup::Unlock-------------------------------------------------------
void CUserGroup::Unlock(void)
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
  pthread_mutex_init(&mutex_groups, NULL);
  m_bAllowSave = false;
  AddGroup(new CUserGroup("All Users", true));
  AddGroup(new CUserGroup("New Users", false));
  m_bAllowSave = true;

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

//-----CUserManager::Load-------------------------------------------------------
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
  sprintf(filename, "%s%s", BASE_DIR, "licq.conf");
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
     AddGroup(new CUserGroup(sGroupName, false));
  }
  m_bAllowSave = true;

  licqConf.ReadNum("DefaultGroup", m_nDefaultGroup, 0);
  licqConf.CloseFile();

  // Load users from users.conf
  sprintf(filename, "%s%s", BASE_DIR, "users.conf");
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
     sprintf(filename, "%s%s%li%s", BASE_DIR, USER_DIR, nUserUin, ".uin");
     AddUser(new ICQUser(nUserUin, filename));
  }

  return true;
}


//-----CUserManager::AddUser----------------------------------------------------
unsigned long CUserManager::AddUser(ICQUser *_pcUser)
{
  // go through all the groups, add the user to those they belong to
  // will always add them to group 0
  CUserGroup *g;
  _pcUser->Lock(LOCK_R);
  unsigned long nUin = _pcUser->getUin();
  m_hUsers.Store(_pcUser, nUin);
  for (unsigned short i = 0; i < NumGroups(); i++)
  {
    if (_pcUser->getIsInGroup(i))
    {
      g = FetchGroup(i, LOCK_W);
      g->AddUser(_pcUser);
      DropGroup(g);
    }
  }
  _pcUser->Unlock();
  return nUin;
}


//-----CUserManager::RemoveUser-------------------------------------------------
void CUserManager::RemoveUser(unsigned long _nUin)
{
  ICQUser *u = FetchUser(_nUin, LOCK_W);
  if (u == NULL) return;
  // Remove the user from all the groups they belong to
  for (unsigned short i = 0; i < NumGroups(); i++)
  {
    if (u->getIsInGroup(i))
    {
      CUserGroup *g = FetchGroup(i, LOCK_W);
      g->RemoveUser(u);
      DropGroup(g);
    }
  }

  u->RemoveFiles();
  DropUser(u);
  m_hUsers.Remove(_nUin);
  delete u;
}


//-----CUserManager::AddGroup---------------------------------------------------
void CUserManager::AddGroup(CUserGroup *_pcGroup)
{
  pthread_mutex_lock(&mutex_groups);
  m_vpcGroups.push_back(_pcGroup);
  SaveGroups();
  pthread_mutex_unlock(&mutex_groups);
}


void CUserManager::RemoveGroup(unsigned short n)
{
  // Don't delete the all users or new users groups
  if (n >= NumGroups() || n < 2)
  {
    pthread_mutex_unlock(&mutex_groups);
    return;
  }

  pthread_mutex_lock(&mutex_groups);
  // Erase the group from the vector
  CUserGroup *g = FetchGroup(n, LOCK_W);
  vector<CUserGroup *>::iterator iter = m_vpcGroups.begin();
  for (int i = 0; i < n; i++) iter++;
  m_vpcGroups.erase(iter);
  delete g;

  // Adjust all the users
  ICQUser *u;
  g = FetchGroup(0, LOCK_R);
  for (int i = 0; i < g->NumUsers(); i++)
  {
    u = g->FetchUser(i, LOCK_W);
    if (u == NULL) continue;
    // shift groups up
    for (int j = n + 1; j < NumGroups(); j++)
    {
      u->setIsInGroup(j - 1, u->getIsInGroup(j));
    }
    g->DropUser(u);
  }
  DropGroup(g);
  if (m_nDefaultGroup >= n) m_nDefaultGroup--;
  SaveGroups();
  pthread_mutex_unlock(&mutex_groups);
}

void CUserManager::SwapGroups(unsigned short g1, unsigned short g2)
{
  pthread_mutex_lock(&mutex_groups);

  // validate the group numbers
  if (g1 >= NumGroups() || g1 < 2 || g2 >= NumGroups() || g2 < 2)
  {
    pthread_mutex_unlock(&mutex_groups);
    return;
  }

  // move the actual group
  CUserGroup *g = m_vpcGroups[g1];
  m_vpcGroups[g1] = m_vpcGroups[g2];
  m_vpcGroups[g2] = g;

  // adjust all the users
  ICQUser *u;
  bool bInG1;
  g = FetchGroup(0, LOCK_R);
  for (int i = 0; i < g->NumUsers(); i++)
  {
    u = g->FetchUser(i, LOCK_W);
    if (u == NULL) continue;
    // swap the groups
    bInG1 = u->getIsInGroup(g1);
    u->setIsInGroup(g1, u->getIsInGroup(g2));
    u->setIsInGroup(g2, bInG1);
    g->DropUser(u);
  }
  DropGroup(g);
  if (m_nDefaultGroup == g1) m_nDefaultGroup = g2;
  else if (m_nDefaultGroup == g2) m_nDefaultGroup = g1;
  SaveGroups();
  pthread_mutex_unlock(&mutex_groups);

}

//-----CUserManager::RenameGroup-----------------------------------------------
void CUserManager::RenameGroup(unsigned short n, const char *_sz)
{
  pthread_mutex_lock(&mutex_groups);
  CUserGroup *g = FetchGroup(n, LOCK_W);
  g->SetName(_sz);
  DropGroup(g);
  SaveGroups();
  pthread_mutex_unlock(&mutex_groups);
}


//-----CUserManager::SaveGroups------------------------------------------------
void CUserManager::SaveGroups(void)
{
  if (!m_bAllowSave) return;

  // Load the group info from licq.conf
  char filename[MAX_FILENAME_LEN];
  sprintf(filename, "%s%s", BASE_DIR, "licq.conf");
  CIniFile licqConf(INI_FxERROR | INI_FxFATAL);
  licqConf.LoadFile(filename);

  licqConf.SetSection("groups");
  licqConf.WriteNum("NumOfGroups", (unsigned short)(NumGroups() - 2));

  char sGroupKey[16];
  CUserGroup *g;
  for (unsigned short i = 1; i <= NumGroups() - 2; i++)
  {
     sprintf(sGroupKey, "Group%d.name", i);
     g = FetchGroup(i + 1, LOCK_R);
     licqConf.WriteStr(sGroupKey, g->Name());
     DropGroup(g);
  }

  licqConf.WriteNum("DefaultGroup", m_nDefaultGroup);
  licqConf.FlushFile();
  licqConf.CloseFile();
}


//-----CUserManager::FetchGroup-------------------------------------------------
CUserGroup *CUserManager::FetchGroup(unsigned short i, unsigned short _nLockType)
{
  if (i < NumGroups())
  {
    m_vpcGroups[i]->Lock(_nLockType);
    return m_vpcGroups[i];
  }
  else
  {
    return NULL;
  }
}


//-----CUserManager:::DropGroup-------------------------------------------------
void CUserManager::DropGroup(CUserGroup *g)
{
  g->Unlock();
}


//-----CUserManager::FetchUser--------------------------------------------------
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

void CUserManager::DropUser(ICQUser *u)
{
  //if (u != NULL)
    u->Unlock();
}

ICQOwner *CUserManager::FetchOwner(unsigned short _nLockType)
{
  m_xOwner->Lock(_nLockType);
  return m_xOwner;
}

void CUserManager::DropOwner(void)
{
  m_xOwner->Unlock();
}


//-----CUserManager::Reorder----------------------------------------------------
void CUserManager::Reorder(ICQUser *_pcUser)
// Call reorder for every group the user is in, assumes a read lock on the user
{
  if (_pcUser == NULL) return;
  for (unsigned short i = 0; i < NumGroups(); i++)
    if (_pcUser->getIsInGroup(i))
    {
      CUserGroup *g = FetchGroup(i, LOCK_W);
      g->Reorder(_pcUser);
      DropGroup(g);
    }
}


//-----CUserManager::SaveAllUsers-----------------------------------------------
void CUserManager::SaveAllUsers(void)
{
  ICQUser *u = NULL;
  CUserGroup *g = FetchGroup(0, LOCK_R);
  unsigned short nNumUsers = g->NumUsers();
  for (unsigned short i = 0; i < nNumUsers; i++)
  {
    u = g->FetchUser(i, LOCK_R);
    u->saveInfo();
    u->saveBasicInfo();
    u->saveExtInfo();
    g->DropUser(u);
  }
  DropGroup(g);
}


//-----CUserManager::AddUserToGroup---------------------------------------------
void CUserManager::AddUserToGroup(unsigned long _nUin, unsigned short _nGroup)
{
  ICQUser *u = FetchUser(_nUin, LOCK_W);
  if (u == NULL) return;
  CUserGroup *g = FetchGroup(_nGroup, LOCK_W);
  if (g == NULL)
  {
    DropUser(u);
    gLog.Warn("%sInvalid group identifier: %d.\n", L_WARNxSTR, _nGroup);
    return;
  }

  u->AddToGroup(_nGroup);
  g->AddUser(u);

  DropUser(u);
  DropGroup(g);
}


//-----CUserManager::RemoveUserFromGroup----------------------------------------
void CUserManager::RemoveUserFromGroup(unsigned long _nUin, unsigned short _nGroup)
{
  ICQUser *u = FetchUser(_nUin, LOCK_W);
  if (u == NULL) return;
  CUserGroup *g = FetchGroup(_nGroup, LOCK_W);
  if (g == NULL)
  {
    DropUser(u);
    gLog.Warn("%sInvalid group identifier: %d.\n", L_WARNxSTR, _nGroup);
    return;
  }

  u->RemoveFromGroup(_nGroup);
  g->RemoveUser(u);

  DropUser(u);
  DropGroup(g);
}


//=====CUserHashTable===========================================================
CUserHashTable::CUserHashTable(unsigned short _nSize) : m_vlTable(_nSize)
{
}


ICQUser *CUserHashTable::Retrieve(unsigned long _nUin)
{
  Lock(LOCK_R);

  ICQUser *u = NULL;
  list <ICQUser *> &l = m_vlTable[HashValue(_nUin)];
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
    list<ICQUser *>::iterator iter;
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
  list<ICQUser *> &l = m_vlTable[HashValue(_nUin)];
  l.push_front(u);
  Unlock();
}

void CUserHashTable::Remove(unsigned long _nUin)
{
  Lock(LOCK_W);

  list<ICQUser *> &l = m_vlTable[HashValue(_nUin)];
  unsigned long nUin;
  list<ICQUser *>::iterator iter;
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
  return _nUin % m_vlTable.size();
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
  m_nLockType = LOCK_N;
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
pthread_mutex_t ICQUser::mutex_eSortKey;


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
  sprintf(szFilename, "%s%s%ld.uin", BASE_DIR, USER_DIR, _nUin);
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
  m_fConf.ReadStr("History", sTemp, "none");
  setHistoryFile(sTemp);
  m_fConf.ReadBool("OnlineNotify", bTemp, false);
  setOnlineNotify(bTemp);
  m_fConf.ReadNum("NewMessages", nTemp, 0);
  nNewMessages = nTemp;
  m_fConf.ReadBool("NewUser", bTemp, false);
  setIsNew(bTemp);
  if (!m_fConf.ReadNum("Group", nTemp)) setGroup(getIsNew() ? 1 : 0);
  else setGroup(nTemp);
  m_fConf.ReadStr("Ip", sTemp, "0.0.0.0");
  struct in_addr in;
  nTemp = inet_aton(sTemp, &in);
  if (nTemp != 0) nTemp = in.s_addr;
  unsigned short nPort;
  m_fConf.ReadNum("Port", nPort, 0);
  SetIpPort(nTemp, nPort);
  m_fConf.ReadBool("VisibleList", bTemp, false);
  setVisibleList(bTemp);
  m_fConf.ReadBool("InvisibleList", bTemp, false);
  setInvisibleList(bTemp);
  m_fConf.ReadStr("City", sTemp, "Unknown");
  setCity(sTemp);
  m_fConf.ReadStr("State", sTemp, "Unknown");
  setState(sTemp);
  m_fConf.ReadNum("Country", nTemp, 0xFFFF);
  setCountry(nTemp);
  m_fConf.ReadNum("Timezone", nTemp, 0x00);
  setTimezone(nTemp);
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
  m_sAwayMessage = NULL;
  m_sCity = NULL;
  m_sState = NULL;
  m_sPhoneNumber = NULL;
  m_sHomepage = NULL;
  m_sAbout = NULL;

  setUin(_nUin);
  setStatus(ICQ_STATUS_OFFLINE);
  setAwayMessage("");
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
  setHomepage("");
  setPhoneNumber("");
  setAbout("");
  setGroup(1);
  setOnlineNotify(false);
  setAuthorization(false);
  setInvisibleList(false);
  setVisibleList(false);
  setOnlineNotify(false);
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
   return (/*getStatusOffline() ? false :*/ m_nStatus & ICQ_STATUS_FxWEBxPRESENCE);
}

inline bool ICQUser::getStatusHideIp(void)
{
   return (/*getStatusOffline() ? false :*/ m_nStatus & ICQ_STATUS_FxHIDExIP);
}

inline bool ICQUser::getStatusBirthday(void)
{
   return (/*getStatusOffline() ? false :*/ m_nStatus & ICQ_STATUS_FxBIRTHDAY);
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
  pthread_mutex_lock(&mutex_eSortKey);
  s_eSortKey = _eSortKey;
  pthread_mutex_unlock(&mutex_eSortKey);
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
  m_nIp = _nIp;
  m_nPort = _nPort;
  saveInfo();
}

void ICQUser::getStatusStr(char *sz)
{
  switch(getStatus())
  {
  case ICQ_STATUS_OFFLINE:     strcpy(sz, "Offline");  break;
  case ICQ_STATUS_ONLINE:      strcpy(sz, "Online");   break;
  case ICQ_STATUS_AWAY:        strcpy(sz, "Away");     break;
  case ICQ_STATUS_NA:          strcpy(sz, "Not Available"); break;
  case ICQ_STATUS_OCCUPIED:    strcpy(sz, "Occupied"); break;
  case ICQ_STATUS_DND:         strcpy(sz, "Do Not Disturb"); break;
  case ICQ_STATUS_FREEFORCHAT: strcpy(sz, "Free for Chat"); break;
  default:                     sprintf(sz, "(0x%04X)", getStatus()); break;
  }
  if (getStatusInvisible())
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
   strncpy(us.awayMessage, getAwayMessage(), MAX_MESSAGE_SIZE);
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
}


void ICQUser::usprintf(char *_sz, const char *_szFormat)
{
  _sz[0] = '\0';
  unsigned short nLen = strlen(_szFormat), i = 0;
  char szTemp[128];
  while(i < nLen)
  {
    if (_szFormat[i] == '%')
    {
      i++;
      if (isdigit(_szFormat[i]))
      {
        strncat(_sz, &_szFormat[i - 1], 2);
        i++;
        continue;
      }
      switch(_szFormat[i])
      {
      case 'i':
        char buf[32];
        strcat(_sz, inet_ntoa_r(*(struct in_addr *)&m_nIp, buf));
        break;
      case 'p':
        sprintf(szTemp, "%d", Port());
        strcat(_sz, szTemp);
        break;
      case 'e':
        strcat(_sz, getEmail());
        break;
      case 'n':
        sprintf(szTemp, "%s %s", getFirstName(), getLastName());
        strcat(_sz, szTemp);
        break;
      case 'f':
        strcat(_sz, getFirstName());
        break;
      case 'l':
        strcat(_sz, getLastName());
        break;
      case 'a':
        strcat(_sz, getAlias());
        break;
      case 'u':
        sprintf(szTemp, "%ld", getUin());
        strcat(_sz, szTemp);
        break;
      case 'w':
        strcat(_sz, getHomepage());
        break;
      case 'h':
        strcat(_sz, getPhoneNumber());
        break;
      default:
        gLog.Warn("%sWarning: Invalid qualifier in command: %%%c.\n",
                  L_WARNxSTR, _szFormat[i]);
        break;
      }
      i++;
    }
    else
    {
      strncat(_sz, &_szFormat[i], 1);
      i++;
    }
  }

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
   m_fConf.WriteBool("OnlineNotify", getOnlineNotify());
   m_fConf.WriteBool("InvisibleList", getInvisibleList());
   m_fConf.WriteBool("VisibleList", getVisibleList());
   m_fConf.WriteNum("Group", getGroup());
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


bool ICQUser::getIsInGroup(unsigned short _nGroup)
{
   if (_nGroup == 0) return true;
   return (getGroup() & (unsigned long)pow(2, _nGroup - 1));
}

void ICQUser::setIsInGroup(unsigned short _nGroup, bool _bIn)
{
  if (_bIn) AddToGroup(_nGroup);
  else RemoveFromGroup(_nGroup);
}

void ICQUser::AddToGroup(unsigned short _nGroup)
{
   if (_nGroup == 0) return;
   setGroup(getGroup() | (unsigned short)pow(2, _nGroup - 1));
}


void ICQUser::RemoveFromGroup(unsigned short _nGroup)
{
   if (_nGroup == 0) return;
   setGroup(getGroup() & (0xFFFF - (unsigned short)pow(2, _nGroup - 1)));
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
  char sTemp[16];
  char filename[MAX_FILENAME_LEN];
  m_bException = false;
  m_sPassword = NULL;

  setEnableSave(false);
  Init(0);

  // Get data from the config file
  sprintf(filename, "%s%s", BASE_DIR, "owner.uin");
  m_fConf.SetFileName(filename);
  LoadData();
  m_fConf.SetFlags(INI_FxWARN);
  m_fConf.ReadNum("Uin", nTemp);
  setUin(nTemp);
  m_fConf.ReadStr("Password", sTemp);
  setPassword(sTemp);
  m_fConf.ReadBool("WebPresence", bTemp);
  if (bTemp) setStatus(getStatus() | ICQ_STATUS_FxWEBxPRESENCE);
  m_fConf.ReadBool("HideIP", bTemp);
  if (bTemp) setStatus(getStatus() | ICQ_STATUS_FxHIDExIP);
  m_fConf.CloseFile();
  struct timezone tz;
  gettimeofday(NULL, &tz);
  setTimezone(tz.tz_minuteswest / 60);

  if (strlen(getPassword()) > 8)
  {
    gLog.Error("%sPassword must be 8 characters or less.  Check %s.\n", L_ERRORxSTR, m_fConf.FileName());
    m_bException = true;
    return;
  }

  sprintf(filename, "%s%s%s", BASE_DIR, HISTORY_DIR, "owner.history");
  setHistoryFile(filename);

  setEnableSave(true);
}

/*
void ICQOwner::Register(unsigned long n, const char *s)
{
  setEnableSave(false);
  setUin(n);
  char buf[24];
  sprintf(buf, "%ld");
  setAlias(buf);
  setPassword(s);
  setEnableSave(true);
  saveInfo();
}
*/

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
  m_fConf.WriteStr("Password", getPassword());
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

