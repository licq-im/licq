// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licq_user.h"

#include <arpa/inet.h>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <unistd.h>

#include <boost/algorithm/string.hpp>

// Localization
#include "gettext.h"

#include "licq_constants.h"
#include "licq_countrycodes.h"
#include "licq_occupationcodes.h"
#include "licq_languagecodes.h"
#include "licq_log.h"
#include "licq_mutex.h"
#include "licq_packets.h"
#include "licq_icqd.h"
#include "licq_socket.h"
#include "support.h"

using namespace std;
using boost::any;
using boost::any_cast;
using boost::bad_any_cast;


ICQUserPhoneBook::ICQUserPhoneBook()
{
}

ICQUserPhoneBook::~ICQUserPhoneBook()
{
  Clean();
}

void ICQUserPhoneBook::AddEntry(const struct PhoneBookEntry *entry)
{
  struct PhoneBookEntry new_entry = *entry;
  new_entry.szDescription = strdup(entry->szDescription);
  new_entry.szAreaCode = strdup(entry->szAreaCode);
  new_entry.szPhoneNumber = strdup(entry->szPhoneNumber);
  new_entry.szExtension = strdup(entry->szExtension);
  new_entry.szCountry = strdup(entry->szCountry);
  new_entry.szGateway = strdup(entry->szGateway);

  PhoneBookVector.push_back(new_entry);
}

void ICQUserPhoneBook::SetEntry(const struct PhoneBookEntry *entry,
                                unsigned long nEntry)
{
  if (nEntry >= PhoneBookVector.size())
  {
    AddEntry(entry);
    return;
  }

  free(PhoneBookVector[nEntry].szDescription);
  free(PhoneBookVector[nEntry].szAreaCode);
  free(PhoneBookVector[nEntry].szPhoneNumber);
  free(PhoneBookVector[nEntry].szExtension);
  free(PhoneBookVector[nEntry].szCountry);
  free(PhoneBookVector[nEntry].szGateway);

  PhoneBookVector[nEntry] = *entry;
  PhoneBookVector[nEntry].szDescription = strdup(entry->szDescription);
  PhoneBookVector[nEntry].szAreaCode = strdup(entry->szAreaCode);
  PhoneBookVector[nEntry].szPhoneNumber = strdup(entry->szPhoneNumber);
  PhoneBookVector[nEntry].szExtension = strdup(entry->szExtension);
  PhoneBookVector[nEntry].szCountry = strdup(entry->szCountry);
  PhoneBookVector[nEntry].szGateway = strdup(entry->szGateway);
}

void ICQUserPhoneBook::ClearEntry(unsigned long nEntry)
{
  if (nEntry >= PhoneBookVector.size())
    return;

  vector<struct PhoneBookEntry>::iterator i = PhoneBookVector.begin();
  for (;nEntry > 0; nEntry--, ++i)
    ;

  free((*i).szDescription);
  free((*i).szAreaCode);
  free((*i).szPhoneNumber);
  free((*i).szExtension);
  free((*i).szCountry);
  free((*i).szGateway);

  PhoneBookVector.erase(i);
}

void ICQUserPhoneBook::Clean()
{
  while (PhoneBookVector.size() > 0)
    ClearEntry(PhoneBookVector.size() - 1);
}

void ICQUserPhoneBook::SetActive(long nEntry)
{
  vector<struct PhoneBookEntry>::iterator iter;
  long i;
  for (i = 0, iter = PhoneBookVector.begin(); iter != PhoneBookVector.end()
                                                 ; i++, ++iter)
    (*iter).nActive = (i == nEntry);
}

bool ICQUserPhoneBook::Get(unsigned long nEntry,
    const struct PhoneBookEntry **entry) const
{
  if (nEntry >= PhoneBookVector.size())
    return false;

  *entry = &PhoneBookVector[nEntry];
  return true;
}

bool ICQUserPhoneBook::SaveToDisk(CIniFile &m_fConf)
{
  char buff[40];

  if (!m_fConf.ReloadFile())
  {
    gLog.Error("%sError opening '%s' for reading.\n"
               "%sSee log for details.\n", L_ERRORxSTR, m_fConf.FileName(),
               L_BLANKxSTR);
    return false;
  }

  m_fConf.SetSection("user");

  m_fConf.WriteNum("PhoneEntries", (unsigned long)PhoneBookVector.size());

  for (unsigned long i = 0 ; i < PhoneBookVector.size(); i++)
  {
    snprintf(buff, sizeof(buff), "PhoneDescription%lu", i);
    m_fConf.WriteStr(buff, PhoneBookVector[i].szDescription);

    snprintf(buff, sizeof(buff), "PhoneAreaCode%lu", i);
    m_fConf.WriteStr(buff, PhoneBookVector[i].szAreaCode);

    snprintf(buff, sizeof(buff), "PhoneNumber%lu", i);
    m_fConf.WriteStr(buff, PhoneBookVector[i].szPhoneNumber);

    snprintf(buff, sizeof(buff), "PhoneExtension%lu", i);
    m_fConf.WriteStr(buff, PhoneBookVector[i].szExtension);

    snprintf(buff, sizeof(buff), "PhoneCountry%lu", i);
    m_fConf.WriteStr(buff, PhoneBookVector[i].szCountry);

    snprintf(buff, sizeof(buff), "PhoneActive%lu", i);
    m_fConf.WriteNum(buff, PhoneBookVector[i].nActive);

    snprintf(buff, sizeof(buff), "PhoneType%lu", i);
    m_fConf.WriteNum(buff, PhoneBookVector[i].nType);

    snprintf(buff, sizeof(buff), "PhoneGateway%lu", i);
    m_fConf.WriteStr(buff, PhoneBookVector[i].szGateway);

    snprintf(buff, sizeof(buff), "PhoneGatewayType%lu", i);
    m_fConf.WriteNum(buff, PhoneBookVector[i].nGatewayType);

    snprintf(buff, sizeof(buff), "PhoneSmsAvailable%lu", i);
    m_fConf.WriteNum(buff, PhoneBookVector[i].nSmsAvailable);

    snprintf(buff, sizeof(buff), "PhoneRemoveLeading0s%lu", i);
    m_fConf.WriteNum(buff, PhoneBookVector[i].nRemoveLeading0s);

    snprintf(buff, sizeof(buff), "PhonePublish%lu", i);
    m_fConf.WriteNum(buff, PhoneBookVector[i].nPublish);
  }

  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n"
               "%sSee log for details.\n", L_ERRORxSTR,
               m_fConf.FileName(), L_BLANKxSTR);
    return false;
  }

  m_fConf.CloseFile();
  return true;
}

bool ICQUserPhoneBook::LoadFromDisk(CIniFile &m_fConf)
{
  char buff[40];
  char szDescription[MAX_LINE_LEN];
  char szAreaCode[MAX_LINE_LEN];
  char szPhoneNumber[MAX_LINE_LEN];
  char szExtension[MAX_LINE_LEN];
  char szCountry[MAX_LINE_LEN];
  char szGateway[MAX_LINE_LEN];
  struct PhoneBookEntry entry = {
                                  szDescription,
                                  szAreaCode,
                                  szPhoneNumber,
                                  szExtension,
                                  szCountry,
                                  0, 0,
                                  szGateway,
                                  0, 0, 0, 0
                                };

  Clean();
  m_fConf.SetSection("user");

  unsigned long nNumEntries;
  m_fConf.ReadNum("PhoneEntries", nNumEntries);
  for (unsigned long i = 0; i < nNumEntries; i++)
  {
    snprintf(buff, sizeof(buff), "PhoneDescription%lu", i);
    m_fConf.ReadStr(buff, entry.szDescription, "");

    snprintf(buff, sizeof(buff), "PhoneAreaCode%lu", i);
    m_fConf.ReadStr(buff, entry.szAreaCode, "");

    snprintf(buff, sizeof(buff), "PhoneNumber%lu", i);
    m_fConf.ReadStr(buff, entry.szPhoneNumber, "");

    snprintf(buff, sizeof(buff), "PhoneExtension%lu", i);
    m_fConf.ReadStr(buff, entry.szExtension, "");

    snprintf(buff, sizeof(buff), "PhoneCountry%lu", i);
    m_fConf.ReadStr(buff, entry.szCountry, "");

    snprintf(buff, sizeof(buff), "PhoneActive%lu", i);
    m_fConf.ReadNum(buff, entry.nActive, 0);

    snprintf(buff, sizeof(buff), "PhoneType%lu", i);
    m_fConf.ReadNum(buff, entry.nType, 0);

    snprintf(buff, sizeof(buff), "PhoneGateway%lu", i);
    m_fConf.ReadStr(buff, entry.szGateway, "");

    snprintf(buff, sizeof(buff), "PhoneGatewayType%lu", i);
    m_fConf.ReadNum(buff, entry.nGatewayType, 1);

    snprintf(buff, sizeof(buff), "PhoneSmsAvailable%lu", i);
    m_fConf.ReadNum(buff, entry.nSmsAvailable, 0);

    snprintf(buff, sizeof(buff), "PhoneRemoveLeading0s%lu", i);
    m_fConf.ReadNum(buff, entry.nRemoveLeading0s, 1);

    snprintf(buff, sizeof(buff), "PhonePublish%lu", i);
    m_fConf.ReadNum(buff, entry.nPublish, 2);

    AddEntry(&entry);
  }

  return true;
}
//===========================================================================


class CUserManager gUserManager;

/* This array is provided for plugin writers' convenience only.
 * No translation is done here. Thus, if your plugin wishes to translate
 * these names, replicate the array into your plugin and do it there.
 */
const char *GroupsSystemNames[NUM_GROUPS_SYSTEM_ALL] = {
  "All Users",
  "Online Notify",
  "Visible List",
  "Invisible List",
  "Ignore List",
  "New Users"
};

char *PPIDSTRING(unsigned long id)
{
  char *p = new char[5];
  p[0] = ((id & 0xFF000000) >> 24);
  p[1] = ((id & 0x00FF0000) >> 16);
  p[2] = ((id & 0x0000FF00) >> 8);
  p[3] = ((id & 0x000000FF));
  p[4] = '\0';
  return p;
}

//=====CUserManager=============================================================
CUserManager::CUserManager()
  : m_szDefaultEncoding(NULL)
{
  // Set up the basic all users and new users group
  myGroupListMutex.setName("grouplist");
  myUserListMutex.setName("userlist");
  myOwnerListMutex.setName("ownerlist");

  m_nOwnerListLockType = LOCK_N;
  m_nUserListLockType = LOCK_N;
  myGroupListLockType = LOCK_N;
}


CUserManager::~CUserManager()
{
  UserMap::iterator iter;
  for (iter = myUsers.begin(); iter != myUsers.end(); ++iter)
    delete iter->second;

  GroupMap::iterator g_iter;
  for (g_iter = myGroups.begin(); g_iter != myGroups.end(); ++g_iter)
    delete g_iter->second;

  OwnerMap::iterator o_iter;
  for (o_iter = myOwners.begin(); o_iter != myOwners.end(); ++o_iter)
    delete o_iter->second;

  if (m_szDefaultEncoding != NULL)
    free(m_szDefaultEncoding);
}

void CUserManager::AddOwner(const char *_szId, unsigned long _nPPID)
{
  ICQOwner *o = new ICQOwner(_szId, _nPPID);

  LockOwnerList(LOCK_W);
  myOwners[_nPPID] = o;
  UnlockOwnerList();
}

/*---------------------------------------------------------------------------
 * CUserManager::Load
 *-------------------------------------------------------------------------*/
bool CUserManager::Load()
{
  gLog.Info(tr("%sUser configuration.\n"), L_INITxSTR);

  // Load the group info from licq.conf
  char filename[MAX_FILENAME_LEN];
  filename[MAX_FILENAME_LEN - 1] = '\0';
  snprintf(filename, MAX_FILENAME_LEN - 1, "%s/licq.conf", BASE_DIR);
  CIniFile licqConf(INI_FxERROR | INI_FxFATAL);
  licqConf.LoadFile(filename);

  unsigned short nOwners;
  licqConf.SetSection("owners");
  licqConf.ReadNum("NumOfOwners", nOwners, 0);

  m_bAllowSave = false;
  char sOwnerIDKey[MAX_KEYxNAME_LEN], sOwnerID[MAX_KEYxNAME_LEN],
       sOwnerPPIDKey[MAX_KEYxNAME_LEN], sOwnerPPID[MAX_KEYxNAME_LEN];
  unsigned long nPPID;

  //TODO Check for loaded plugins before the owner, so we can see
  //which owner(s) to load
  for (unsigned short i = 1; i <= nOwners; i++)
  {
    sprintf(sOwnerIDKey, "Owner%d.Id", i);
    licqConf.ReadStr(sOwnerIDKey, sOwnerID);
    sprintf(sOwnerPPIDKey, "Owner%d.PPID", i);
    licqConf.ReadStr(sOwnerPPIDKey, sOwnerPPID);
    nPPID = (sOwnerPPID[0] << 24) | (sOwnerPPID[1] << 16) |
            (sOwnerPPID[2] << 8) | (sOwnerPPID[3]);
    AddOwner(sOwnerID, nPPID);
  }

  unsigned int nGroups;
  licqConf.SetSection("groups");
  licqConf.ReadNum("NumOfGroups", nGroups);

  GroupMap* groups = LockGroupList(LOCK_W);
  m_bAllowSave = false;
  char key[MAX_KEYxNAME_LEN], groupName[MAX_LINE_LEN];
  int groupId, sortIndex;
  unsigned short icqGroupId;
  for (unsigned int i = 1; i <= nGroups; i++)
  {
    sprintf(key, "Group%d.name", i);
    licqConf.ReadStr(key, groupName);

    licqConf.SetFlags(0);

    sprintf(key, "Group%d.id", i);
    licqConf.ReadNum(key, groupId, 0);

    sprintf(key, "Group%d.Sorting", i);
    bool newConfig = licqConf.ReadNum(key, sortIndex, i-1);

    sprintf(key, "Group%d.IcqServerId", i);
    licqConf.ReadNum(key, icqGroupId, 0);

    // Sorting and IcqServerId did not exist in older versions.
    // If they are missing, assume that we are reading an old configuration
    // where id parameter is ICQ server side group id.
    if (!newConfig)
    {
      icqGroupId = groupId;
      groupId = i;
    }

    licqConf.SetFlags(INI_FxFATAL | INI_FxERROR | INI_FxWARN);

    LicqGroup* newGroup = new LicqGroup(groupId, groupName);
    newGroup->setIcqGroupId(icqGroupId);
    newGroup->setSortIndex(sortIndex);
    (*groups)[groupId] = newGroup;
  }
  m_bAllowSave = true;
  UnlockGroupList();


  char szTemp[MAX_LINE_LEN];
  licqConf.SetSection("network");
  licqConf.SetFlags(0);
  licqConf.ReadStr("DefaultUserEncoding", szTemp, "");
  SetString(&m_szDefaultEncoding, szTemp);
  licqConf.SetFlags(INI_FxERROR | INI_FxFATAL);
  licqConf.CloseFile();

  // Load users from users.conf
  snprintf(filename, MAX_FILENAME_LEN - 1, "%s/users.conf", BASE_DIR);
  CIniFile usersConf(INI_FxFATAL | INI_FxERROR);
  usersConf.LoadFile(filename);

  unsigned short nUsers;
  usersConf.SetSection("users");
  usersConf.ReadNum("NumOfUsers", nUsers);
  gLog.Info(tr("%sLoading %d users.\n"), L_INITxSTR, nUsers);

  // TODO: We need to only load users of protocol plugins that are loaded!
  char sUserKey[MAX_KEYxNAME_LEN];
  char szFile[MAX_LINE_LEN];
  char szId[MAX_LINE_LEN];
  char *sz;
  ICQUser *u;
  usersConf.SetFlags(INI_FxWARN);
  LockUserList(LOCK_W);
  for (unsigned short i = 1; i<= nUsers; i++)
  {
    sprintf(sUserKey, "User%d", i);
    if (!usersConf.ReadStr(sUserKey, szFile, ""))
    {
      gLog.Warn(tr("%sSkipping user %i, empty key.\n"), L_WARNxSTR, i);
      continue;
    }
    snprintf(filename, MAX_FILENAME_LEN - 1, "%s/%s/%s", BASE_DIR, USER_DIR,
             szFile);
    sz = strrchr(szFile, '.');
    if( !sz )
    {
      gLog.Error(tr("%sFatal error reading protocol information for User%d with ID '%s'.\n"
                    "%sPlease check \"%s/users.conf\".\n"), L_ERRORxSTR, i, szFile, 
                    L_BLANKxSTR, BASE_DIR);
      exit(1);
    }
    strncpy(szId, szFile, sz - szFile);
    szId[sz - szFile] = '\0';
    nPPID = (*(sz+1)) << 24 | (*(sz+2)) << 16 | (*(sz+3)) << 8 | (*(sz+4));

    u = new LicqUser(szId, nPPID, string(filename));
    u->AddToContactList();
    myUsers[u->id()] = u;
  }
  UnlockUserList();

  return true;
}

void CUserManager::saveUserList() const
{
  string filename = string(BASE_DIR) + "/users.conf";

  CIniFile usersConf(INI_FxERROR | INI_FxFATAL | INI_FxALLOWxCREATE);
  usersConf.LoadFile(filename.c_str());
  usersConf.SetSection("users");

  char key[MAX_KEYxNAME_LEN];
  int count = 0;
  for (UserMap::const_iterator i = myUsers.begin(); i != myUsers.end(); ++i)
  {
    i->second->Lock(LOCK_R);
    bool temporary = i->second->NotInList();
    string accountId = i->second->accountId();
    unsigned long ppid = i->second->ppid();
    i->second->Unlock();

    // Only save users that's been permanently added
    if (temporary)
      continue;
    ++count;

    sprintf(key, "User%i", count);

    char* ps = PPIDSTRING(ppid);
    usersConf.writeString(key, accountId + "." + ps);
    delete [] ps;
  }
  usersConf.WriteNum("NumOfUsers", count);
  usersConf.FlushFile();
  usersConf.CloseFile();
}

bool CUserManager::addUser(const UserId& uid,
    bool permanent, bool addToServer, unsigned short groupId)
{
  if (!USERID_ISVALID(uid))
    return false;

  if (isOwner(uid))
    return false;

  LockUserList(LOCK_W);

  // Make sure user isn't already in the list
  UserMap::const_iterator iter = myUsers.find(uid);
  if (iter != myUsers.end())
  {
    UnlockUserList();
    return false;
  }

  string accountId = LicqUser::getUserAccountId(uid);
  unsigned long ppid = LicqUser::getUserProtocolId(uid);

  LicqUser* pUser = new LicqUser(accountId, ppid, !permanent);
  pUser->Lock(LOCK_W);

  if (permanent)
  {
    // Set this user to be on the contact list
    pUser->AddToContactList();
    //pUser->SetEnableSave(true);
    pUser->saveAll();
  }

  // Store the user in the lookup map
  myUsers[uid] = pUser;

  pUser->Unlock();

  if (permanent)
    saveUserList();

  UnlockUserList();

  // Notify plugins that user was added
  // Send this before adding user to server side as protocol code may generate updated signals
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_ADD, uid, groupId));

  // Add user to server side list
  if (permanent && addToServer)
    gLicqDaemon->protoAddUser(accountId.c_str(), ppid, groupId);

  // Set initial group membership, also sets server group for user
  if (groupId != 0)
    setUserInGroup(uid, GROUPS_USER, groupId, true, permanent && addToServer);

  return true;
}

void CUserManager::removeUser(const UserId& userId)
{
  // Remove the user from the server side list first
  gLicqDaemon->protoRemoveUser(userId);

  // List should only be locked when not holding any user lock to avoid
  // deadlock, so we cannot call FetchUser here.
  LockUserList(LOCK_W);
  UserMap::iterator iter = myUsers.find(userId);
  if (iter == myUsers.end())
  {
    UnlockUserList();
    return;
  }

  LicqUser* u = iter->second;
  u->Lock(LOCK_W);
  myUsers.erase(iter);
  if (!u->NotInList())
  {
    u->RemoveFiles();
    saveUserList();
  }
  UnlockUserList();
  u->Unlock();
  delete u;

  // Notify plugins about the removed user
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_REMOVE, userId));
}

// Need to call CICQDaemon::SaveConf() after this
void CUserManager::RemoveOwner(unsigned long ppid)
{
  // List should only be locked when not holding any user lock to avoid
  // deadlock, so we cannot call FetchOwner here.
  LockOwnerList(LOCK_W);
  OwnerMap::iterator iter = myOwners.find(ppid);
  if (iter == myOwners.end())
  {
    UnlockOwnerList();
    return;
  }

  LicqOwner* o = iter->second;
  o->Lock(LOCK_W);
  myOwners.erase(iter);
  o->RemoveFiles();
  UnlockOwnerList();
  o->Unlock();
  delete o;
}

LicqUser* CUserManager::fetchUser(const UserId& userId,
    unsigned short lockType, bool addUser, bool* retWasAdded)
{
  if (retWasAdded != NULL)
    *retWasAdded = false;

  LicqUser* user = NULL;

  if (!USERID_ISVALID(userId))
    return NULL;

  // Check for an owner first
  LockOwnerList(LOCK_R);
  OwnerMap::iterator iter_o = myOwners.find(LicqUser::getUserProtocolId(userId));
  if (iter_o != myOwners.end())
  {
    if (iter_o->second->id() == userId)
    {
      user = iter_o->second;
      user->Lock(lockType);
    }
  }
  UnlockOwnerList();

  if (user != NULL)
    return user;

  LockUserList(LOCK_R);
  UserMap::const_iterator iter = myUsers.find(userId);
  if (iter != myUsers.end())
    user = iter->second;

    // If allowed by caller, add user if it wasn't found in list
    if (user == NULL && addUser)
    {
      // Relock user list for writing
      UnlockUserList();
      LockUserList(LOCK_W);

      // Create a temporary user
      user = new LicqUser(LicqUser::getUserAccountId(userId), LicqUser::getUserProtocolId(userId), true);

      // Store the user in the lookup map
      myUsers[userId] = user;

      // Notify plugins that we added user to list
      gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_ADD, userId));

      if (retWasAdded != NULL)
        *retWasAdded = true;
    }

    // Lock the user and release the lock on the list
    if (user != NULL)
      user->Lock(lockType);
    UnlockUserList();

  return user;
}

bool CUserManager::userExists(const UserId& userId)
{
  bool exists = false;

  LockOwnerList(LOCK_R);
  OwnerMap::iterator iter_o = myOwners.find(LicqUser::getUserProtocolId(userId));
  if (iter_o != myOwners.end())
  {
    if (iter_o->second->id() == userId)
      exists = true;
  }
  UnlockOwnerList();
  if (exists)
    return true;

  LockUserList(LOCK_R);
  UserMap::const_iterator iter = myUsers.find(userId);
  if (iter != myUsers.end())
    exists = true;
  UnlockUserList();
  return exists;
}

// This differs by FetchOwner by requiring an Id.  This isn't used to
// fetch an owner, but for interal use only to see if the given id and ppid
// is an owner.  (It can be used to fetch an owner by calling FetchUser with
// an owner's id and ppid.
LicqOwner* CUserManager::FindOwner(const char* accountId, unsigned long ppid)
{
/*
  // Strip spaces if ICQ protocol
  char *szId = new char[strlen(accountId)];
  if (ppid == LICQ_PPID)
  {
    for (int i = 0; i < strlen(accountId); i++)
      if (accountId[i] != ' ')
        *szId++ = accountId[i];
  }
  else
    strcpy(szId, accountId);
*/

  ICQOwner *o = NULL;
  bool found = false;

  LockOwnerList(LOCK_R);
  OwnerMap::iterator iter = myOwners.find(ppid);
  if (iter != myOwners.end())
  {
    o = iter->second;
    o->Lock(LOCK_R);
    if (o->accountId() == accountId /* || o->accountId() == szId */)
      found = true;
    o->Unlock();
  }
  UnlockOwnerList();

  //delete [] szId;

  return (found ? o : NULL);
}

UserId CUserManager::ownerUserId(unsigned long ppid)
{
  const ICQOwner* owner = FetchOwner(ppid, LOCK_R);
  if (owner == NULL)
    return "";

  UserId ret = owner->id();
  DropOwner(owner);
  return ret;
}

string CUserManager::OwnerId(unsigned long ppid)
{
  const ICQOwner* owner = FetchOwner(ppid, LOCK_R);
  if (owner == NULL)
    return "";

  string ret = owner->accountId();
  DropOwner(owner);
  return ret;
}

bool CUserManager::isOwner(const UserId& userId)
{
  bool exists = false;

  LockOwnerList(LOCK_R);
  OwnerMap::iterator iter_o = myOwners.find(LicqUser::getUserProtocolId(userId));
  if (iter_o != myOwners.end())
  {
    if (iter_o->second->id() == userId)
      exists = true;
  }
  UnlockOwnerList();

  return exists;
}

unsigned long CUserManager::icqOwnerUin()
{
  return strtoul(OwnerId(LICQ_PPID).c_str(), (char**)NULL, 10);
}

LicqGroup* CUserManager::FetchGroup(int group, unsigned short lockType)
{
  GroupMap* groups = LockGroupList(LOCK_R);
  GroupMap::const_iterator iter = groups->find(group);
  LicqGroup* g = NULL;
  if (iter != groups->end())
  {
    g = iter->second;
    g->Lock(lockType);
  }
  UnlockGroupList();
  return g;
}

void CUserManager::DropGroup(const LicqGroup* group)
{
  if (group != NULL)
    group->Unlock();
}

bool CUserManager::groupExists(GroupType gtype, int groupId)
{
  // Is it a valid system group?
  if (gtype == GROUPS_SYSTEM)
    return (groupId < NUM_GROUPS_SYSTEM_ALL);

  // Is it an invalid group type?
  if (gtype != GROUPS_USER)
    return false;

  // Does the user group exist in the list?
  GroupMap* groups = LockGroupList(LOCK_R);
  GroupMap::const_iterator iter = groups->find(groupId);
  bool found = (iter != groups->end());
  UnlockGroupList();
  return found;
}

/*---------------------------------------------------------------------------
 * CUserManager::AddGroup
 *-------------------------------------------------------------------------*/
int CUserManager::AddGroup(const string& name, unsigned short icqGroupId)
{
  if (name.empty())
    return 0;

  if (GetGroupFromName(name) != 0)
  {
    // Don't allow a duplicate name
    gLog.Warn(tr("%sGroup %s is already in list.\n"), L_WARNxSTR, name.c_str());
    return 0;
  }

  GroupMap* groups = LockGroupList(LOCK_W);

  // Find first free group id
  int gid;
  for (gid = 1; groups->count(gid) != 0 ; ++gid)
    ;

  LicqGroup* newGroup = new LicqGroup(gid, name);
  newGroup->setIcqGroupId(icqGroupId);
  newGroup->setSortIndex(groups->size());
  (*groups)[gid] = newGroup;

  SaveGroups();
  UnlockGroupList();

  if (gLicqDaemon != NULL)
  {
    if (icqGroupId == 0)
      gLicqDaemon->icqAddGroup(name.c_str());
    else
      gLog.Info(tr("%sAdded group %s (%u) to list from server.\n"),
          L_SRVxSTR, name.c_str(), icqGroupId);

    // Send signal to let plugins know of the new group
    gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_GROUP_ADDED, USERID_NONE, gid));
  }

  return gid;
}

/*---------------------------------------------------------------------------
 * CUserManager::RemoveGroup
 *-------------------------------------------------------------------------*/
void CUserManager::RemoveGroup(int groupId)
{
  LicqGroup* group = FetchGroup(groupId, LOCK_R);
  if (group == NULL)
    return;

  string name = group->name();
  int sortIndex = group->sortIndex();
  DropGroup(group);

  // Must be called when there are no locks on GroupID and Group lists
  gLicqDaemon->icqRemoveGroup(name.c_str());

  // Lock it back up
  GroupMap* g = LockGroupList(LOCK_W);
  group->Lock(LOCK_W);

  // Erase the group
  g->erase(groupId);
  group->Unlock();
  delete group;


  // Decrease sorting index for higher groups so we don't leave a gap
  GroupMap::const_iterator iter;
  for (iter = g->begin(); iter != g->end(); ++iter)
  {
    iter->second->Lock(LOCK_W);
    int si = iter->second->sortIndex();
    if (si > sortIndex)
      iter->second->setSortIndex(si - 1);
    iter->second->Unlock();
  }

  // Remove group from users
  FOR_EACH_USER_START(LOCK_W)
  {
    if (pUser->RemoveFromGroup(GROUPS_USER, groupId))
      gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_GENERAL,
          pUser->id()));
  }
  FOR_EACH_USER_END;

  SaveGroups();
  UnlockGroupList();

  // Send signal to let plugins know of the removed group
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_GROUP_REMOVED, USERID_NONE, groupId));

  // Send signal to let plugins know that sorting indexes may have changed
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_GROUP_REORDERED));
}

void CUserManager::ModifyGroupSorting(int groupId, int newIndex)
{
  LicqGroup* group = FetchGroup(groupId, LOCK_R);
  if (group == NULL)
    return;

  if (newIndex < 0)
    newIndex = 0;
  if (static_cast<unsigned int>(newIndex) >= NumGroups())
    newIndex = NumGroups() - 1;

  int oldIndex = group->sortIndex();
  DropGroup(group);

  GroupMap* g = LockGroupList(LOCK_R);

  // Move all groups between new and old position one step
  for (GroupMap::iterator i = g->begin(); i != g->end(); ++i)
  {
    i->second->Lock(LOCK_W);
    int si = i->second->sortIndex();
    if (newIndex < oldIndex && si >= newIndex && si < oldIndex)
      i->second->setSortIndex(si + 1);
    else if (newIndex > oldIndex && si > oldIndex && si <= newIndex)
      i->second->setSortIndex(si - 1);
    i->second->Unlock();
  }

  group->Lock(LOCK_W);
  group->setSortIndex(newIndex);
  group->Unlock();

  SaveGroups();
  UnlockGroupList();

  // Send signal to let plugins know that sorting indexes have changed
  if (gLicqDaemon != NULL)
    gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_GROUP_REORDERED));
}

/*---------------------------------------------------------------------------
 * CUserManager::RenameGroup
 *-------------------------------------------------------------------------*/
bool CUserManager::RenameGroup(int groupId, const string& name, bool sendUpdate)
{
  int foundGroupId = GetGroupFromName(name);

  if (foundGroupId == groupId)
    // Name isn't changed so nothing to do here
    return true;

  if (foundGroupId != 0)
  {
    // Don't allow a duplicate name
    gLog.Warn(tr("%sGroup name %s is already in list.\n"), L_WARNxSTR, name.c_str());
    return false;
  }

  LicqGroup* group = FetchGroup(groupId, LOCK_W);
  if (group == NULL)
  {
    gLog.Warn(tr("%sRenaming request for invalid group %u.\n"), L_WARNxSTR, groupId);
    return false;
  }

  group->setName(name);
  unsigned short icqGroupId = group->icqGroupId();
  DropGroup(group);

  LockGroupList(LOCK_R);
  SaveGroups();
  UnlockGroupList();

  if (gLicqDaemon != NULL)
  {
    // If we rename a group on logon, don't send the rename packet
    if (sendUpdate)
      gLicqDaemon->icqRenameGroup(name.c_str(), icqGroupId);

    // Send signal to let plugins know the group has changed
    gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_GROUP_CHANGED, USERID_NONE, groupId));
  }

  return true;
}

/*---------------------------------------------------------------------------
 * CUserManager::SaveGroups
 *
 * Assumes a lock on the group list
 *-------------------------------------------------------------------------*/
void CUserManager::SaveGroups()
{
  if (!m_bAllowSave) return;

  // Load the group info from licq.conf
  char filename[MAX_FILENAME_LEN];
  snprintf(filename, MAX_FILENAME_LEN, "%s/licq.conf", BASE_DIR);
  filename[MAX_FILENAME_LEN - 1] = '\0';
  CIniFile licqConf(INI_FxERROR | INI_FxFATAL);
  licqConf.LoadFile(filename);

  licqConf.SetSection("groups");
  GroupMap::size_type count = myGroups.size();
  licqConf.WriteNum("NumOfGroups", static_cast<unsigned int>(count));

  char key[MAX_KEYxNAME_LEN];
  int i = 1;
  for (GroupMap::iterator group = myGroups.begin(); group != myGroups.end(); ++group)
  {
    group->second->Lock(LOCK_R);

    sprintf(key, "Group%d.name", i);
    licqConf.WriteStr(key, group->second->name().c_str());

    sprintf(key, "Group%d.id", i);
    licqConf.WriteNum(key, group->second->id());

    sprintf(key, "Group%d.IcqServerId", i);
    licqConf.WriteNum(key, group->second->icqGroupId());

    sprintf(key, "Group%d.Sorting", i);
    licqConf.WriteNum(key, group->second->sortIndex());

    group->second->Unlock();
    ++i;
  }

  licqConf.FlushFile();
  licqConf.CloseFile();
}

/*---------------------------------------------------------------------------
 * CUserManager::GetIDFromGroup
 *-------------------------------------------------------------------------*/
unsigned short CUserManager::GetIDFromGroup(const string& name)
{
  int groupId = GetGroupFromName(name);
  if (groupId == 0)
    return 0;

  return GetIDFromGroup(groupId);
}

unsigned short CUserManager::GetIDFromGroup(int groupId)
{
  LicqGroup* group = gUserManager.FetchGroup(groupId, LOCK_R);
  if (group == NULL)
    return 0;

  unsigned short icqGroupId = group->icqGroupId();
  DropGroup(group);

  return icqGroupId;
}

/*---------------------------------------------------------------------------
 * CUserManager::GetGroupFromID
 *-------------------------------------------------------------------------*/
int CUserManager::GetGroupFromID(unsigned short icqGroupId)
{
  const GroupMap* groups = LockGroupList(LOCK_R);
  GroupMap::const_iterator iter;
  int groupId = 0;
  for (iter = groups->begin(); iter != groups->end(); ++iter)
  {
    iter->second->Lock(LOCK_R);
    if (iter->second->icqGroupId() == icqGroupId)
      groupId = iter->first;
    iter->second->Unlock();
  }
  UnlockGroupList();

  return groupId;
}

int CUserManager::GetGroupFromName(const string& name)
{
  const GroupMap* groups = LockGroupList(LOCK_R);
  GroupMap::const_iterator iter;
  int id = 0;
  for (iter = groups->begin(); iter != groups->end(); ++iter)
  {
    iter->second->Lock(LOCK_R);
    if (iter->second->name() == name)
      id = iter->first;
    iter->second->Unlock();
  }
  UnlockGroupList();

  return id;
}

/*---------------------------------------------------------------------------
 * CUserManager::ModifyGroupID
 *-------------------------------------------------------------------------*/
void CUserManager::ModifyGroupID(const string& name, unsigned short icqGroupId)
{
  int id = GetGroupFromName(name);
  if (id != 0)
    ModifyGroupID(id, icqGroupId);
}

void CUserManager::ModifyGroupID(int groupId, unsigned short icqGroupId)
{
  LicqGroup* group = FetchGroup(groupId, LOCK_W);
  if (group == NULL)
    return;

  group->setIcqGroupId(icqGroupId);
  DropGroup(group);

  LockGroupList(LOCK_R);
  SaveGroups();
  UnlockGroupList();
}

/*---------------------------------------------------------------------------
 * CUserManager::GenerateSID
 *
 * Generate a random number.  Make sure no user has that random number.  If
 * a user does have that number, increment and check against all users.  When
 * a number is created that is not owned by a current user, check against all
 * the groups.  If it is owned by a group, increment and start back with checking
 * the users.
 *-------------------------------------------------------------------------*/
unsigned short CUserManager::GenerateSID()
{
  bool bCheckGroup, bDone;
  int nSID;
  unsigned short nOwnerPDINFO;

  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  nOwnerPDINFO = o->GetPDINFO();
  gUserManager.DropOwner(o);

  // Generate a SID
  srand(time(NULL));
  nSID = 1+(int)(65535.0*rand()/(RAND_MAX+1.0));

  nSID &= 0x7FFF; // server limit it looks like

  // Make sure we have a unique number - a map would be better
  do
  {
    bDone = true;
    bCheckGroup = true;

    if (nSID == 0) nSID++;
    if (nSID == nOwnerPDINFO) nSID++;
    FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_R)
    {
      if (pUser->GetSID() == nSID  || pUser->GetInvisibleSID() == nSID ||
          pUser->GetVisibleSID() == nSID)
      {
        if (nSID == 0x7FFF)
          nSID = 1;
        else
          nSID++;
        bDone = false;	// Restart
        bCheckGroup = false;	// Don't waste time now
        FOR_EACH_PROTO_USER_BREAK;
      }
    }
    FOR_EACH_PROTO_USER_END

    if (bCheckGroup)
    {
      // Check our groups too!
      FOR_EACH_GROUP_START(LOCK_R)
      {
        unsigned short icqGroupId = pGroup->icqGroupId();
        if (icqGroupId == nSID)
        {
          if (nSID == 0x7FFF)
            nSID = 1;
          else
            nSID++;
          bDone = false;
          FOR_EACH_GROUP_BREAK
        }
      }
      FOR_EACH_GROUP_END
    }

  } while (!bDone);

  return nSID;
}

void CUserManager::DropUser(const ICQUser* u)
{
  if (u == NULL) return;
  u->Unlock();
}

LicqOwner* CUserManager::FetchOwner(unsigned long ppid, unsigned short lockType)
{
  LicqOwner* o = NULL;

  LockOwnerList(LOCK_R);
  OwnerMap::iterator iter = myOwners.find(ppid);
  if (iter != myOwners.end())
  {
    o = iter->second;
    o->Lock(lockType);
  }
  UnlockOwnerList();

  return o;
}

void CUserManager::DropOwner(const ICQOwner* owner)
{
  if (owner == NULL)
    return;
  owner->Unlock();
}

/*---------------------------------------------------------------------------
 * CUserManager::SaveAllUsers
 *-------------------------------------------------------------------------*/
void CUserManager::SaveAllUsers()
{
  FOR_EACH_USER_START(LOCK_R)
  {
    if (!pUser->NotInList())
      pUser->saveAll();
  }
  FOR_EACH_USER_END
}

/*---------------------------------------------------------------------------
 * CUserManager::UpdateUsersInGroups
 *-------------------------------------------------------------------------*/
bool CUserManager::UpdateUsersInGroups()
{
  bool bDid = false;
  
  FOR_EACH_PROTO_USER_START(LICQ_PPID, LOCK_W)
  {
    unsigned short nGSID = pUser->GetGSID();
    if (nGSID)
    {
      int nInGroup = gUserManager.GetGroupFromID(nGSID);
      if (nInGroup != 0)
      {
        pUser->AddToGroup(GROUPS_USER, nInGroup);
        bDid = true;
      }
    }
  }
  FOR_EACH_PROTO_USER_END
  
  return bDid;
}

/*---------------------------------------------------------------------------
 * CUserManager::NumUsers
 *-------------------------------------------------------------------------*/
unsigned short CUserManager::NumUsers()
{
  //LockUserList(LOCK_R);
  unsigned short n = myUsers.size();
  //UnlockUserList();
  return n;
}


/*---------------------------------------------------------------------------
 * CUserManager::NumOwners
 *-------------------------------------------------------------------------*/
unsigned short CUserManager::NumOwners()
{
  unsigned short n = myOwners.size();
  return n;
}

/*---------------------------------------------------------------------------
 * CUserManager::NumGroups
 *-------------------------------------------------------------------------*/
unsigned int CUserManager::NumGroups()
{
  //LockGroupList(LOCK_R);
  unsigned int n = myGroups.size();
  //UnlockGroupList();
  return n;
}


/*---------------------------------------------------------------------------
 * LockUserList
 *
 * Locks the entire user list for iterating through...
 *-------------------------------------------------------------------------*/
UserMap* CUserManager::LockUserList(unsigned short _nLockType)
{
  switch (_nLockType)
  {
    case LOCK_R:
      myUserListMutex.lockRead();
      break;
    case LOCK_W:
      myUserListMutex.lockWrite();
      break;
    default:
      assert(false);
      return NULL;
  }
  m_nUserListLockType = _nLockType;
  return &myUsers;
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
      myUserListMutex.unlockRead();
      break;
    case LOCK_W:
      myUserListMutex.unlockWrite();
      break;
    default:
      assert(false);
      break;
  }
}

/*---------------------------------------------------------------------------
 * LockGroupList
 *
 * Locks the entire group list for iterating through...
 *-------------------------------------------------------------------------*/
GroupMap* CUserManager::LockGroupList(unsigned short lockType)
{
  switch (lockType)
  {
    case LOCK_R:
      myGroupListMutex.lockRead();
      break;
    case LOCK_W:
      myGroupListMutex.lockWrite();
      break;
    default:
      assert(false);
      return NULL;
  }
  myGroupListLockType = lockType;
  return &myGroups;
}

/*---------------------------------------------------------------------------
 * CUserManager::UnlockGroupList
 *-------------------------------------------------------------------------*/
void CUserManager::UnlockGroupList()
{
  unsigned short lockType = myGroupListLockType;
  myGroupListLockType = LOCK_R;
  switch (lockType)
  {
    case LOCK_R:
      myGroupListMutex.unlockRead();
      break;
    case LOCK_W:
      myGroupListMutex.unlockWrite();
      break;
    default:
      assert(false);
      break;
  }
}

OwnerMap* CUserManager::LockOwnerList(unsigned short _nLockType)
{
  switch (_nLockType)
  {
    case LOCK_R:
      myOwnerListMutex.lockRead();
      break;
    case LOCK_W:
      myOwnerListMutex.lockWrite();
      break;
    default:
      assert(false);
      return NULL;
  }
  m_nOwnerListLockType = _nLockType;
  return &myOwners;
}

void CUserManager::UnlockOwnerList()
{
  unsigned short nLockType = m_nOwnerListLockType;
  m_nOwnerListLockType = LOCK_R;
  switch (nLockType)
  {
    case LOCK_R:
      myOwnerListMutex.unlockRead();
      break;
    case LOCK_W:
      myOwnerListMutex.unlockWrite();
      break;
    default:
      assert(false);
      break;
  }
}

void CUserManager::setUserInGroup(const UserId& userId,
    GroupType groupType, int groupId, bool inGroup, bool updateServer)
{
  // User group 0 is invalid and system group 0 is All Users
  if (groupId == 0)
    return;

  LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);
  if (u == NULL)
    return;

  int gsid = u->GetGSID();

  if (groupType == GROUPS_USER && !inGroup && u->GetSID() != 0 && GetGroupFromID(gsid) == groupId)
  {
    // Don't remove user from local group if member of the same server group
    gUserManager.DropUser(u);
    return;
  }

  // Update user object
  u->SetInGroup(groupType, groupId, inGroup);
  string accountId = u->accountId();
  unsigned long ppid = u->ppid();
  gUserManager.DropUser(u);

  // Notify server
  if (updateServer && gLicqDaemon != NULL)
  {
    if (groupType == GROUPS_SYSTEM)
    {
      if (groupId == GROUP_VISIBLE_LIST)
        gLicqDaemon->visibleListSet(userId, inGroup);

      else if (groupId == GROUP_INVISIBLE_LIST)
        gLicqDaemon->invisibleListSet(userId, inGroup);

      else if (groupId == GROUP_IGNORE_LIST)
        gLicqDaemon->ignoreListSet(userId, inGroup);
    }
    else
    {
      // Server group currently only supported for ICQ protocol
      // Server group can only be changed, not removed
      if (ppid == LICQ_PPID && inGroup)
        gLicqDaemon->icqChangeGroup(accountId.c_str(), ppid, groupId, gsid,
            ICQ_ROSTxNORMAL, ICQ_ROSTxNORMAL);
    }
  }

  // Notify plugins
  if (gLicqDaemon != NULL)
    gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_GENERAL, userId));
}

void CUserManager::SetDefaultUserEncoding(const char* defaultEncoding)
{
  SetString(&m_szDefaultEncoding, defaultEncoding);
}


LicqGroup::LicqGroup(int id, const string& name)
  : myId(id),
    myName(name),
    mySortIndex(0),
    myIcqGroupId(0)
{
  char strId[8];
  snprintf(strId, 7, "%u", myId);
  strId[7] = '\0';

  myMutex.setName(strId);
}

LicqGroup::~LicqGroup()
{
}

bool compare_groups(const LicqGroup* first, const LicqGroup* second)
{
  return first->sortIndex() < second->sortIndex();
}

//=====CUser====================================================================


UserId LicqUser::makeUserId(const string& accountId, unsigned long ppid)
{
  // ppid is always four ascii charaters, use them plus the normalized account id as our user ids
  char ppidstr[5];
  ppidstr[0] = ((ppid & 0xFF000000) >> 24);
  ppidstr[1] = ((ppid & 0x00FF0000) >> 16);
  ppidstr[2] = ((ppid & 0x0000FF00) >> 8);
  ppidstr[3] = ((ppid & 0x000000FF));
  ppidstr[4] = '\0';
  return ppidstr + normalizeId(accountId, ppid);
}

string LicqUser::getUserAccountId(const UserId& userId)
{
  if (userId.size() < 4)
    return "";
  return userId.substr(4);
}

unsigned long LicqUser::getUserProtocolId(const UserId& userId)
{
  if (userId.size() < 4)
    return 0;
  return userId[0] << 24 | userId[1] << 16 | userId[2] << 8 | userId[3];
}


unsigned short ICQUser::s_nNumUserEvents = 0;
pthread_mutex_t ICQUser::mutex_nNumUserEvents = PTHREAD_MUTEX_INITIALIZER;

LicqUser::LicqUser(const string& accountId, unsigned long ppid, const string& filename)
  : myId(makeUserId(accountId, ppid)),
    myAccountId(accountId),
    myPpid(ppid)
{
  Init();
  m_fConf.SetFlags(INI_FxWARN);
  m_fConf.SetFileName(filename.c_str());
  if (!LoadInfo())
  {
    gLog.Error("%sUnable to load user info from '%s'.\n%sUsing default values.\n",
        L_ERRORxSTR, filename.c_str(), L_BLANKxSTR);
    SetDefaults();
  }
  m_fConf.CloseFile();
  m_fConf.SetFlags(INI_FxWARN | INI_FxALLOWxCREATE);
}

LicqUser::LicqUser(const string& accountId, unsigned long ppid, bool temporary)
  : myId(makeUserId(accountId, ppid)),
    myAccountId(accountId),
    myPpid(ppid)
{
  Init();
  SetDefaults();
  m_bNotInList = temporary;
  if (!m_bNotInList)
  {
    char szFilename[MAX_FILENAME_LEN];
    char* p = PPIDSTRING(ppid);
    snprintf(szFilename, MAX_FILENAME_LEN, "%s/%s/%s.%s", BASE_DIR, USER_DIR,
        myAccountId.c_str(), p);
    delete [] p;
    szFilename[MAX_FILENAME_LEN - 1] = '\0';
    m_fConf.SetFileName(szFilename);
    m_fConf.SetFlags(INI_FxWARN | INI_FxALLOWxCREATE);
  }
}

void ICQUser::AddToContactList()
{
  m_bOnContactList = m_bEnableSave = true;
  m_bNotInList = false;

  // Check for old history file
  if (access(m_fHistory.FileName(), F_OK) == -1)
  {
    char szFilename[MAX_FILENAME_LEN];
    char* p = PPIDSTRING(myPpid);
    snprintf(szFilename, MAX_FILENAME_LEN, "%s/%s/%s.%s.%s", BASE_DIR, HISTORY_DIR, myAccountId.c_str(),
             p, HISTORYxOLD_EXT);
    delete [] p;

    szFilename[MAX_FILENAME_LEN - 1] = '\0';
    if (access(szFilename, F_OK) == 0)
    {
      if (rename(szFilename, m_fHistory.FileName()) == -1)
      {
        gLog.Warn(tr("%sFailed to rename old history file (%s):\n%s%s\n"), L_WARNxSTR,
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

  loadUserInfo();
  LoadPhoneBookInfo();
  LoadPictureInfo();
  LoadLicqInfo();

  return true;
}

void ICQUser::loadUserInfo()
{
  // read in the fields, checking for errors each time
  m_fConf.SetSection("user");
  m_fConf.readString("Alias", myAlias, tr("Unknown"));
  m_fConf.ReadNum("Timezone", m_nTimezone, TIMEZONE_UNKNOWN);
  m_fConf.ReadBool("Authorization", m_bAuthorization, false);

  PropertyMap::iterator i;
  for (i = myUserInfo.begin(); i != myUserInfo.end(); ++i)
    m_fConf.readVar(i->first, i->second);

  loadCategory(myInterests, m_fConf, "Interests");
  loadCategory(myBackgrounds, m_fConf, "Backgrounds");
  loadCategory(myOrganizations, m_fConf, "Organizations");
}

//-----ICQUser::LoadPhoneBookInfo--------------------------------------------
void ICQUser::LoadPhoneBookInfo()
{
  m_PhoneBook->LoadFromDisk(m_fConf);
}

//-----ICQUser::LoadPictureInfo----------------------------------------------
void ICQUser::LoadPictureInfo()
{
  char szTemp[MAX_LINE_LEN];
  m_fConf.SetSection("user");
  m_fConf.ReadBool("PicturePresent", m_bPicturePresent, false);
  m_fConf.ReadNum("BuddyIconType", m_nBuddyIconType, 0);
  m_fConf.ReadNum("BuddyIconHashType", m_nBuddyIconHashType, 0);
  m_fConf.ReadStr("BuddyIconHash", szTemp, "");
  SetString(&m_szBuddyIconHash, szTemp );
  m_fConf.ReadStr("OurBuddyIconHash", szTemp, "");
  SetString(&m_szOurBuddyIconHash, szTemp );
}

//-----ICQUser::LoadLicqInfo-------------------------------------------------
void ICQUser::LoadLicqInfo()
{
  // read in the fields, checking for errors each time
  char szTemp[MAX_LINE_LEN];
  unsigned short nNewMessages;
  unsigned long nLast;
  unsigned short nPPFieldCount;
  m_fConf.SetSection("user");
  m_fConf.ReadNum("Groups.System", mySystemGroups, 0);
  m_fConf.ReadStr("Ip", szTemp, "0.0.0.0");
  struct in_addr in;
  m_nIp = inet_pton(AF_INET, szTemp, &in);
  if (m_nIp > 0)
    m_nIp = in.s_addr;
  m_fConf.ReadStr("IntIp", szTemp, "0.0.0.0");
  m_nIntIp = inet_pton(AF_INET, szTemp, &in);
  if (m_nIntIp > 0)
    m_nIntIp = in.s_addr;
  m_fConf.ReadNum("Port", m_nPort, 0);
  //m_fConf.ReadBool("NewUser", m_bNewUser, false);
  m_fConf.ReadNum("NewMessages", nNewMessages, 0);
  m_fConf.ReadNum("LastOnline", nLast, 0);
  m_nLastCounters[LAST_ONLINE] = nLast;
  m_fConf.ReadNum("LastSent", nLast, 0);
  m_nLastCounters[LAST_SENT_EVENT] = nLast;
  m_fConf.ReadNum("LastRecv", nLast, 0);
  m_nLastCounters[LAST_RECV_EVENT] = nLast;
  m_fConf.ReadNum("LastCheckedAR", nLast, 0);
  m_nLastCounters[LAST_CHECKED_AR] = nLast;
  m_fConf.ReadNum("RegisteredTime", nLast, 0);
  m_nRegisteredTime = nLast;
  m_fConf.ReadNum("AutoAccept", m_nAutoAccept, 0);
  m_fConf.ReadNum("StatusToUser", m_nStatusToUser, ICQ_STATUS_OFFLINE);
  if (User()) // Only allow to keep a modified alias for user uins
    m_fConf.ReadBool("KeepAliasOnUpdate", m_bKeepAliasOnUpdate, false);
  else
    m_bKeepAliasOnUpdate = false;
  m_fConf.ReadStr("CustomAutoRsp", szTemp, "");
  m_fConf.ReadBool("SendIntIp", m_bSendIntIp, false);
  SetCustomAutoResponse(szTemp);
  m_fConf.ReadStr( "UserEncoding", szTemp, "" );
  SetString( &m_szEncoding, szTemp );
  m_fConf.ReadStr("History", szTemp, "default");
  if (szTemp[0] == '\0') strcpy(szTemp, "default");
  SetHistoryFile(szTemp);
  m_fConf.ReadBool("AwaitingAuth", m_bAwaitingAuth, false);
  m_fConf.ReadNum("SID", m_nSID[NORMAL_SID], 0);
  m_fConf.ReadNum("InvisibleSID", m_nSID[INV_SID], 0);
  m_fConf.ReadNum("VisibleSID", m_nSID[VIS_SID], 0);
  m_fConf.ReadNum("GSID", m_nGSID, 0);
  m_fConf.ReadNum("ClientTimestamp", m_nClientTimestamp, 0);
  m_fConf.ReadNum("ClientInfoTimestamp", m_nClientInfoTimestamp, 0);
  m_fConf.ReadNum("ClientStatusTimestamp", m_nClientStatusTimestamp, 0);
  m_fConf.ReadNum("OurClientTimestamp", m_nOurClientTimestamp, 0);
  m_fConf.ReadNum("OurClientInfoTimestamp", m_nOurClientInfoTimestamp, 0);
  m_fConf.ReadNum("OurClientStatusTimestamp", m_nOurClientStatusTimestamp, 0);
  m_fConf.ReadNum("PhoneFollowMeStatus", m_nPhoneFollowMeStatus,
                  ICQ_PLUGIN_STATUSxINACTIVE);
  m_fConf.ReadNum("ICQphoneStatus", m_nICQphoneStatus,
                  ICQ_PLUGIN_STATUSxINACTIVE);
  m_fConf.ReadNum("SharedFilesStatus", m_nSharedFilesStatus,
                  ICQ_PLUGIN_STATUSxINACTIVE);
  m_fConf.ReadBool("UseGPG", m_bUseGPG, false );
  m_fConf.ReadStr("GPGKey", szTemp, "" );
  SetString( &m_szGPGKey, szTemp );
  m_fConf.ReadBool("SendServer", m_bSendServer, false);
  m_fConf.ReadNum("PPFieldCount", nPPFieldCount, 0);
  for (int i = 0; i < nPPFieldCount; i++)
  {
    char szBuf[15];
    char szTempName[MAX_LINE_LEN], szTempValue[MAX_LINE_LEN];
    sprintf(szBuf, "PPField%d.Name", i+1);
    m_fConf.ReadStr(szBuf, szTempName, "");
    if (strcmp(szTempName, "") != 0)
    {
      sprintf(szBuf, "PPField%d.Value", i+1);
      m_fConf.ReadStr(szBuf, szTempValue, "");
      if (strcmp(szTempValue, "") != 0)
	m_mPPFields[szTempName] = szTempValue;
    }
  }

  unsigned int userGroupCount;
  if (m_fConf.ReadNum("GroupCount", userGroupCount, 0))
  {
    for (unsigned int i = 1; i <= userGroupCount; ++i)
    {
      sprintf(szTemp, "Group%u", i);
      int groupId;
      m_fConf.ReadNum(szTemp, groupId, 0);
      if (groupId > 0)
        AddToGroup(GROUPS_USER, groupId);
    }
  }
  else
  {
    // Groupcount is missing in user config, try and read old group configuration
    unsigned int oldGroups;
    m_fConf.ReadNum("Groups.User", oldGroups, 0);
    for (int i = 0; i <= 31; ++i)
      if (oldGroups & (1L << i))
        AddToGroup(GROUPS_USER, i+1);
  }

  m_bSupportsUTF8 = false;
  
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
LicqUser::~LicqUser()
{
  unsigned long nId;
  while (m_vcMessages.size() > 0)
  {
    nId = m_vcMessages[m_vcMessages.size() - 1]->Id();
    delete m_vcMessages[m_vcMessages.size() - 1];
    m_vcMessages.pop_back();
    decNumUserEvents();
    
    if (gLicqDaemon != NULL)
      gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER,
          USER_EVENTS, myId, nId));
  }

  if ( m_szAutoResponse )
      free( m_szAutoResponse );
  if ( m_szEncoding )
      free( m_szEncoding );
  if ( m_szCustomAutoResponse )
      free( m_szCustomAutoResponse );
  if ( m_szClientInfo )
      free( m_szClientInfo );
  if ( m_szGPGKey )
      free( m_szGPGKey );
  if (m_szBuddyIconHash)
    free(m_szBuddyIconHash);
  if (m_szOurBuddyIconHash)
    free(m_szOurBuddyIconHash);

  delete m_PhoneBook;
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
    char *p = PPIDSTRING(myPpid);
    snprintf(szFilename, MAX_FILENAME_LEN, "%s/%s/%s.%s.%s", BASE_DIR, HISTORY_DIR,
        myAccountId.c_str(), p, HISTORYxOLD_EXT);
    delete [] p;

    szFilename[MAX_FILENAME_LEN - 1] = '\0';
    if (rename(m_fHistory.FileName(), szFilename) == -1)
    {
      gLog.Warn(tr("%sFailed to rename history file (%s):\n%s%s\n"), L_WARNxSTR,
          szFilename, L_BLANKxSTR, strerror(errno));
      remove(m_fHistory.FileName());
    }
  }
}

void LicqUser::Init()
{
  myRealAccountId = normalizeId(myAccountId, myPpid);

  //SetOnContactList(false);
  m_bOnContactList = m_bEnableSave = false;
  m_szAutoResponse = NULL;
  m_szEncoding = strdup("");
  m_bSecure = false;

  // TODO: Only user data fields valid for protocol should be populated

  // General Info
  myAlias = string();
  myUserInfo["FirstName"] = string();
  myUserInfo["LastName"] = string();
  myUserInfo["Email1"] = string(); // Primary email
  myUserInfo["Email2"] = string(); // Secondary email
  myUserInfo["Email0"] = string(); // Old email
  myUserInfo["City"] = string();
  myUserInfo["State"] = string();
  myUserInfo["PhoneNumber"] = string();
  myUserInfo["FaxNumber"] = string();
  myUserInfo["Address"] = string();
  myUserInfo["CellularNumber"] = string();
  myUserInfo["Zipcode"] = string();
  myUserInfo["Country"] = (unsigned int)COUNTRY_UNSPECIFIED;
  myUserInfo["HideEmail"] = false;
  m_nTimezone = TIMEZONE_UNKNOWN;
  m_bAuthorization = false;
  m_nTyping = ICQ_TYPING_INACTIVEx0;
  m_bNotInList = false;
  myOnEventsBlocked = false;

  // More Info
  myUserInfo["Age"] = (unsigned int)0xffff;
  myUserInfo["Gender"] = (unsigned int)0;
  myUserInfo["Homepage"] = string();
  myUserInfo["BirthYear"] = (unsigned int)0;
  myUserInfo["BirthMonth"] = (unsigned int)0;
  myUserInfo["BirthDay"] = (unsigned int)0;
  myUserInfo["Language0"] = (unsigned int)0;
  myUserInfo["Language1"] = (unsigned int)0;
  myUserInfo["Language2"] = (unsigned int)0;

  // Homepage Info
  myUserInfo["HomepageCatPresent"] = false;
  myUserInfo["HomepageCatCode"] = (unsigned int)0;
  myUserInfo["HomepageDesc"] = string();
  myUserInfo["ICQHomepagePresent"] = false;

  // More2
  myInterests.clear();
  myBackgrounds.clear();
  myOrganizations.clear();

  // Work Info
  myUserInfo["CompanyCity"] = string();
  myUserInfo["CompanyState"] = string();
  myUserInfo["CompanyPhoneNumber"] = string();
  myUserInfo["CompanyFaxNumber"] = string();
  myUserInfo["CompanyAddress"] = string();
  myUserInfo["CompanyZip"] = string();
  myUserInfo["CompanyCountry"] = (unsigned int)COUNTRY_UNSPECIFIED;
  myUserInfo["CompanyName"] = string();
  myUserInfo["CompanyDepartment"] = string();
  myUserInfo["CompanyPosition"] = string();
  myUserInfo["CompanyOccupation"] = (unsigned int)OCCUPATION_UNSPECIFIED;
  myUserInfo["CompanyHomepage"] = string();

  // About
  myUserInfo["About"] = string();

  // Phone Book
  m_PhoneBook = new ICQUserPhoneBook();

  // Picture
  m_bPicturePresent = false;
  m_nBuddyIconType = 0;
  m_nBuddyIconHashType = 0;
  m_szBuddyIconHash = strdup("");
  m_szOurBuddyIconHash = strdup("");

  // GPG key
  m_szGPGKey = strdup("");

  // gui plugin compat
  SetStatus(ICQ_STATUS_OFFLINE);
  SetAutoResponse("");
  SetSendServer(false);
  SetSendIntIp(false);
  SetShowAwayMsg(false);
  SetSequence(static_cast<unsigned short>(-1)); // set all bits 0xFFFF
  SetOfflineOnDisconnect(false);
  ClearSocketDesc();
  m_nIp = m_nPort = m_nIntIp = 0;
  m_nMode = MODE_DIRECT;
  m_nVersion = 0;
  m_nCookie = 0;
  m_nClientTimestamp = 0;
  m_nClientInfoTimestamp = 0;
  m_nClientStatusTimestamp = 0;
  m_nOurClientTimestamp = 0;
  m_nOurClientInfoTimestamp = 0;
  m_nOurClientStatusTimestamp = 0;
  m_bUserUpdated = false;
  m_nPhoneFollowMeStatus = ICQ_PLUGIN_STATUSxINACTIVE;
  m_nICQphoneStatus = ICQ_PLUGIN_STATUSxINACTIVE;
  m_nSharedFilesStatus = ICQ_PLUGIN_STATUSxINACTIVE;
  Touch();
  for (unsigned short i = 0; i < 4; i++)
    m_nLastCounters[i] = 0;
  m_nOnlineSince = 0;
  m_nIdleSince = 0;
  m_nRegisteredTime = 0;
  m_nStatusToUser = ICQ_STATUS_OFFLINE;
  m_bKeepAliasOnUpdate = false;
  m_nStatus = ICQ_STATUS_OFFLINE;
  m_nAutoAccept = 0;
  m_szCustomAutoResponse = NULL;
  m_bConnectionInProgress = false;
  m_bAwaitingAuth = false;
  m_nSID[0] = m_nSID[1] = m_nSID[2] = 0;
  m_nGSID = 0;
  m_szClientInfo = NULL;

  myMutex.setName(myAccountId);
}

void ICQUser::SetPermanent()
{
  // Set the flags and check for history file to recover
  AddToContactList();

  // Create the user file
  char szFilename[MAX_FILENAME_LEN];
  char* p = PPIDSTRING(myPpid);
  snprintf(szFilename, MAX_FILENAME_LEN, "%s/%s/%s.%s", BASE_DIR, USER_DIR,
      myAccountId.c_str(), p);
  delete [] p;
  szFilename[MAX_FILENAME_LEN - 1] = '\0';
  m_fConf.SetFileName(szFilename);
  m_fConf.SetFlags(INI_FxWARN | INI_FxALLOWxCREATE);

  // Save all the info now
  saveAll();

  // Notify the plugins of the change
  // Send a USER_BASIC, don't want a new signal just for this.
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER,
      USER_BASIC, myId, 0));
}

//-----ICQUser::SetDefaults-----------------------------------------------------
void ICQUser::SetDefaults()
{
  char szTemp[12];
  setAlias(myAccountId);
  SetHistoryFile("default");
  SetSystemGroups(0);
  myGroups.clear();
  SetNewUser(true);
  SetAuthorization(false);

  szTemp[0] = '\0';
  SetCustomAutoResponse(szTemp);
}

string ICQUser::getUserInfoString(const string& key) const
{
  try
  {
    PropertyMap::const_iterator i = myUserInfo.find(key);
    if (i != myUserInfo.end())
      // Try to cast value to a string
      return any_cast<string>(i->second);
  }
  catch(const bad_any_cast &)
  {
  }
  // Unknown property or not a string so just return an empty string object
  return string();
}

unsigned int ICQUser::getUserInfoUint(const string& key) const
{
  try
  {
    PropertyMap::const_iterator i = myUserInfo.find(key);
    if (i != myUserInfo.end())
      // Try to cast value to an unsigned int
      return any_cast<unsigned int>(i->second);
  }
  catch(const bad_any_cast &)
  {
  }
  // Unknown property or not an int so just return 0
  return 0;
}

bool ICQUser::getUserInfoBool(const string& key) const
{
  try
  {
    PropertyMap::const_iterator i = myUserInfo.find(key);
    if (i != myUserInfo.end())
      // Try to cast value to a bool
      return any_cast<bool>(i->second);
  }
  catch(const bad_any_cast &)
  {
  }
  // Unknown property or not an int so just return false
  return false;
}

void ICQUser::setUserInfoString(const string& key, const string& value)
{
  PropertyMap::iterator i = myUserInfo.find(key);
  if (i == myUserInfo.end() || i->second.type() != typeid(string))
{
    return;
}

  i->second = value;
  saveUserInfo();
}

void ICQUser::setUserInfoUint(const string& key, unsigned int value)
{
  PropertyMap::iterator i = myUserInfo.find(key);
  if (i == myUserInfo.end() || i->second.type() != typeid(unsigned int))
{
    return;
}

  i->second = value;
  saveUserInfo();
}

void ICQUser::setUserInfoBool(const string& key, bool value)
{
  PropertyMap::iterator i = myUserInfo.find(key);
  if (i == myUserInfo.end() || i->second.type() != typeid(bool))
{
    return;
}

  i->second = value;
  saveUserInfo();
}

std::string ICQUser::getFullName() const
{
  string name = getFirstName();
  string lastName = getLastName();
  if (!name.empty() && !lastName.empty())
    name += ' ';
  return name + lastName;
}

std::string ICQUser::getEmail() const
{
  string email = getUserInfoString("Email1");
  if (email.empty())
    email = getUserInfoString("Email2");
  if (email.empty())
    email = getUserInfoString("Email0");
  return email;
}

const char* ICQUser::UserEncoding() const
{
  if (m_szEncoding == NULL || m_szEncoding[0] == '\0')
    return gUserManager.DefaultUserEncoding();
  else
    return m_szEncoding;
}


unsigned short ICQUser::Status() const
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
    m_nLastCounters[LAST_ONLINE] = time(NULL);
    SaveLicqInfo();
  }
  
  SetTyping(ICQ_TYPING_INACTIVEx0);
  SetUserUpdated(false);
  SetStatus(ICQ_STATUS_OFFLINE);
}



/* Birthday: checks to see if the users birthday is within the next nRange
   days.  Returns -1 if not, or the number of days until their bday */
int ICQUser::Birthday(unsigned short nRange) const
{
  static const unsigned char nMonthDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  time_t t = time(NULL);
  struct tm *ts = localtime(&t);
  int nDays = -1;

  int birthDay = getUserInfoUint("BirthDay");
  int birthMonth = getUserInfoUint("BirthMonth");

  if (birthMonth == 0 || birthDay == 0)
  {
    if (StatusBirthday() && User()) return 0;
    return -1;
  }

  if (nRange == 0)
  {
    if (ts->tm_mon + 1 == birthMonth && ts->tm_mday == birthDay)
      nDays = 0;
  }
  else
  {
    unsigned char nMonth, nDayMin, nDayMax, nMonthNext, nDayMaxNext;

    nMonth = ts->tm_mon + 1;
    nMonthNext = nDayMaxNext = 0;
    nDayMin = ts->tm_mday;
    nDayMax = nDayMin + nRange;
    if (nDayMax > nMonthDays[nMonth])
    {
      nMonthNext = nMonth + 1;
      if (nMonthNext == 13) nMonth = 1;
      nDayMaxNext = nDayMax - nMonthDays[nMonth];
      nDayMax = nMonthDays[nMonth];
    }

    if (birthMonth == nMonth && birthDay >= nDayMin && birthDay <= nDayMax)
    {
      nDays = birthDay - nDayMin;
    }
    else if (nMonthNext != 0 && birthMonth == nMonthNext && birthDay <= nDayMaxNext)
    {
      nDays = birthDay + (nMonthDays[nMonth] - nDayMin);
    }

    /*struct tm tb = *ts;
    tm_mday = birthDay - 1;
    tm_mon = birthMonth - 1;
    mktime(&tb);
    nDays = tb.tm_yday - ts->tm_yday;*/
  }

  return nDays;
}


unsigned short ICQUser::Sequence(bool increment)
{
   if (increment)
      return (m_nSequence--);
   else
      return (m_nSequence);
}

void LicqUser::setAlias(const string& alias)
{
  if (alias.empty())
  {
    string firstName = getFirstName();
    if (!firstName.empty())
      myAlias = firstName;
    else
      myAlias = myAccountId;
  }
  else
    myAlias = alias;

  // If there is a valid alias, set the server side list alias as well.
  if (!myAlias.empty())
  {
    size_t aliasLen = myAlias.size();
    TLVPtr aliasTLV(new COscarTLV(0x131, aliasLen, myAlias.c_str()));
    AddTLV(aliasTLV);
  }

  saveUserInfo();
}


bool ICQUser::Away() const
{
   unsigned short n = Status();
   return (n == ICQ_STATUS_AWAY || n == ICQ_STATUS_NA ||
           n == ICQ_STATUS_DND || n == ICQ_STATUS_OCCUPIED);
}

void ICQUser::SetHistoryFile(const char *s)
{
  m_fHistory.SetFile(s, myAccountId.c_str(), myPpid);
  SaveLicqInfo();
}


void ICQUser::SetIpPort(unsigned long _nIp, unsigned short _nPort)
{
  if ((SocketDesc(ICQ_CHNxNONE) != -1 || SocketDesc(ICQ_CHNxINFO) != -1
       || SocketDesc(ICQ_CHNxSTATUS) != -1) &&
      ( (Ip() != 0 && Ip() != _nIp) || (Port() != 0 && Port() != _nPort)) )
  {
    // Close our socket, but don't let socket manager try and clear
    // our socket descriptor
    if (SocketDesc(ICQ_CHNxNONE) != -1)
      gSocketManager.CloseSocket(SocketDesc(ICQ_CHNxNONE), false);
    if (SocketDesc(ICQ_CHNxINFO) != -1)
      gSocketManager.CloseSocket(SocketDesc(ICQ_CHNxINFO), false);
    if (SocketDesc(ICQ_CHNxSTATUS) != -1)
      gSocketManager.CloseSocket(SocketDesc(ICQ_CHNxSTATUS), false);
    ClearSocketDesc();
  }
  m_nIp = _nIp;
  m_nPort = _nPort;
  SaveLicqInfo();
}

int ICQUser::SocketDesc(unsigned char nChannel) const
{
  switch (nChannel)
  {
  case ICQ_CHNxNONE:
    return m_nNormalSocketDesc;
  case ICQ_CHNxINFO:
    return m_nInfoSocketDesc;
  case ICQ_CHNxSTATUS:
    return m_nStatusSocketDesc;
  }
  gLog.Warn("%sUnknown channel type %u\n", L_WARNxSTR, nChannel);

  return 0;
}

void ICQUser::SetSocketDesc(TCPSocket *s)
{
  if (s->Channel() == ICQ_CHNxNONE)
    m_nNormalSocketDesc = s->Descriptor();
  else if (s->Channel() == ICQ_CHNxINFO)
    m_nInfoSocketDesc = s->Descriptor();
  else if (s->Channel() == ICQ_CHNxSTATUS)
    m_nStatusSocketDesc = s->Descriptor();
  m_nLocalPort = s->getLocalPort();
  m_nConnectionVersion = s->Version();
  if (m_bSecure != s->Secure())
  {
    m_bSecure = s->Secure();
    if (gLicqDaemon != NULL && m_bOnContactList)
      gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_SECURITY,
          myId, m_bSecure ? 1 : 0));
  }

  if (m_nIntIp == 0)
    m_nIntIp = s->getRemoteIpInt();
  if (m_nPort == 0)
    m_nPort = s->getRemotePort();
  SetSendServer(false);
}

void ICQUser::ClearSocketDesc(unsigned char nChannel)
{
  switch (nChannel)
  {
  case ICQ_CHNxNONE:
    m_nNormalSocketDesc = -1;
    break;
  case ICQ_CHNxINFO:
    m_nInfoSocketDesc = -1;
    break;
  case ICQ_CHNxSTATUS:
    m_nStatusSocketDesc = -1;
    break;
  case 0x00: // Used as default value to clear all socket descriptors
    m_nNormalSocketDesc = m_nInfoSocketDesc = m_nStatusSocketDesc = -1;
    break;
  default:
    gLog.Info("%sUnknown channel %u\n", L_WARNxSTR, nChannel);
    return;
  }

  if (m_nStatusSocketDesc == -1 &&
      m_nInfoSocketDesc == -1 &&
      m_nNormalSocketDesc == -1)
  {
    m_nLocalPort = 0;
    m_nConnectionVersion = 0;
    m_bSecure = false;
  }

  if (gLicqDaemon != NULL && m_bOnContactList)
    gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, USER_SECURITY, myId, 0));
}

unsigned short ICQUser::ConnectionVersion() const
{
  // If we are already connected, use that version
  if (m_nConnectionVersion != 0) return m_nConnectionVersion;
  // We aren't connected, see if we know their version
  return VersionToUse(m_nVersion);
}


int ICQUser::LocalTimeGMTOffset() const
{
  return GetTimezone() * 1800;
}


int ICQUser::SystemTimeGMTOffset()
{
  time_t t = time(NULL);
  struct tm *tzone = localtime(&t);
#ifdef USE_GMTOFF
  return -(tzone->tm_gmtoff) + (tzone->tm_isdst == 1 ? 3600 : 0); // seconds _east_ of UTC
#elif defined(USE_TIMEZONE)
  return timezone;  // seconds _west_ of UTC
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


int ICQUser::LocalTimeOffset() const
{
  return SystemTimeGMTOffset() - LocalTimeGMTOffset();
}


time_t ICQUser::LocalTime() const
{
  return time(NULL) + LocalTimeOffset();
}


SecureChannelSupport_et ICQUser::SecureChannelSupport() const
{
  if ((m_nClientTimestamp & 0xFFFF0000) == LICQ_WITHSSL)
    return SECURE_CHANNEL_SUPPORTED;
  else if ((m_nClientTimestamp & 0xFFFF0000) == LICQ_WITHOUTSSL)
    return SECURE_CHANNEL_NOTSUPPORTED;
  else
    return SECURE_CHANNEL_UNKNOWN;
}



unsigned short ICQUser::LicqVersion() const
{
  if ((m_nClientTimestamp & 0xFFFF0000) == LICQ_WITHSSL ||
       (m_nClientTimestamp & 0xFFFF0000) == LICQ_WITHOUTSSL)
    return m_nClientTimestamp & 0x0000FFFF;

  return LICQ_VERSION_UNKNOWN;
}


const char* ICQUser::StatusStr() const
{
  return StatusToStatusStr(m_nStatus, StatusInvisible());
}

const char* ICQUser::StatusStrShort() const
{
  return StatusToStatusStrShort(m_nStatus, StatusInvisible());
}


const char* ICQUser::StatusToStatusStr(unsigned short n, bool b)
{
  if (n == ICQ_STATUS_OFFLINE) return b ? tr("(Offline)") : tr("Offline");
  else if (n & ICQ_STATUS_DND) return b ? tr("(Do Not Disturb)") : tr("Do Not Disturb");
  else if (n & ICQ_STATUS_OCCUPIED) return b ? tr("(Occupied)") : tr("Occupied");
  else if (n & ICQ_STATUS_NA) return b ? tr("(Not Available)") : tr("Not Available");
  else if (n & ICQ_STATUS_AWAY) return b ? tr("(Away)") : tr("Away");
  else if (n & ICQ_STATUS_FREEFORCHAT) return b ? tr("(Free for Chat)") : tr("Free for Chat");
  else if (n << 24 == 0x00) return b ? tr("(Online)") : tr("Online");
  else return "Unknown";
}


const char* ICQUser::StatusToStatusStrShort(unsigned short n, bool b)
{
  if (n == ICQ_STATUS_OFFLINE) return b ? tr("(Off)") : tr("Off");
  else if (n & ICQ_STATUS_DND) return b ? tr("(DND)") : tr("DND");
  else if (n & ICQ_STATUS_OCCUPIED) return b ? tr("(Occ)") : tr("Occ");
  else if (n & ICQ_STATUS_NA) return b ? tr("(N/A)") : tr("N/A");
  else if (n & ICQ_STATUS_AWAY) return b ? tr("(Away)") : tr("Away");
  else if (n & ICQ_STATUS_FREEFORCHAT) return b ? tr("(FFC)") : tr("FFC");
  else if (n << 24 == 0x00) return b ? tr("(On)") : tr("On");
  else return "???";
}


char* ICQUser::IpStr(char* rbuf) const
{
  char ip[32], buf[32];

  if (Ip() > 0)     		// Default to the given ip
    strcpy(ip, ip_ntoa(m_nIp, buf));
  else				// Otherwise we don't know
    strcpy(ip, tr("Unknown"));

  if (StatusHideIp())
    sprintf(rbuf, "(%s)", ip);
  else
    sprintf(rbuf, "%s", ip);

  return rbuf;
}


char* ICQUser::PortStr(char* rbuf) const
{
  if (Port() > 0)     		// Default to the given port
    sprintf(rbuf, "%d", Port());
  else				// Otherwise we don't know
    rbuf[0] = '\0';

  return rbuf;
}


char* ICQUser::IntIpStr(char* rbuf) const
{
  char buf[32];
  int socket = SocketDesc(ICQ_CHNxNONE);
  if (socket < 0)
    socket = SocketDesc(ICQ_CHNxINFO);
  if (socket < 0)
    socket = SocketDesc(ICQ_CHNxSTATUS);

  if (socket > 0)		// First check if we are connected
  {
    INetSocket *s = gSocketManager.FetchSocket(socket);
    if (s != NULL)
    {
      strcpy(rbuf, s->getRemoteIpString().c_str());
      gSocketManager.DropSocket(s);
    }
    else
      strcpy(rbuf, tr("Invalid"));
  }
  else
  {
    if (IntIp() > 0)		// Default to the given ip
      strcpy(rbuf, ip_ntoa(m_nIntIp, buf));
    else			// Otherwise we don't know
      rbuf[0] = '\0';
  }

  return rbuf;
}


char* ICQUser::usprintf(const char* _szFormat, unsigned long nFlags) const
{
  bool bLeft = false;
  unsigned long i = 0, j, nField = 0, nPos = 0;
  char szTemp[128];

  // Our secure string for escaping stuff
  bool bSecure = (_szFormat[0] == '|' && (nFlags & USPRINTF_PIPEISCMD)) ||
   (nFlags & USPRINTF_LINEISCMD);

  unsigned long bufSize = strlen(_szFormat) + 512;
  char *_sz = (char *)malloc(bufSize);
#define CHECK_BUFFER                     \
  if (nPos >= bufSize - 1)               \
  {                                      \
    bufSize *= 2;                        \
    _sz = (char *)realloc(_sz, bufSize); \
  }

  while(_szFormat[i] != '\0')
  {
    if (_szFormat[i] == '`')
    {
        _sz[nPos++] = '`';
        CHECK_BUFFER;
        i++;
        while(_szFormat[i] != '`' && _szFormat[i] != '\0')
        {
            _sz[nPos++] = _szFormat[i++];
            CHECK_BUFFER;
        }
        if (_szFormat[i] != '\0')
        {
          _sz[nPos++] = _szFormat[i];
          CHECK_BUFFER;
          i++;
        }
    }
    else if (_szFormat[i] == '%')
    {
      i++;
      if (!(nFlags & USPRINTF_NOFW))
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
          CHECK_BUFFER;
          _sz[nPos++] = _szFormat[i++];
          CHECK_BUFFER;
          continue;
        }
      }

      const char *sz = 0;
      switch(_szFormat[i])
      {
        case 'i':
          char buf[32];
          strcpy(szTemp, ip_ntoa(m_nIp, buf));
          sz = szTemp;
          break;
        case 'p':
          sprintf(szTemp, "%d", Port());
          sz = szTemp;
          break;
        case 'P':
        {
          ProtoPluginsList pl;
          ProtoPluginsListIter it;
          gLicqDaemon->ProtoPluginList(pl);
          for (it = pl.begin(); it != pl.end(); it++)
          {
            if (myPpid == (*it)->PPID())
            {
              strcpy(szTemp, (*it)->Name());
              sz = szTemp;
              break;
            }
          }
          break;
        }
        case 'e':
          sz = getEmail().c_str();
          break;
        case 'n':
          sz = getFullName().c_str();
          break;
        case 'f':
          sz = getFirstName().c_str();
          break;
        case 'l':
          sz = getLastName().c_str();
          break;
        case 'a':
          sz = getAlias().c_str();
          break;
        case 'u':
          sz = accountId().c_str();
          break;
        case 'w':
          sz = getUserInfoString("Homepage").c_str();
          break;
        case 'h':
          sz = getUserInfoString("PhoneNumber").c_str();
          break;
        case 'c':
          sz = getUserInfoString("CellularNumber").c_str();
          break;
        case 'S':
          sz = StatusStrShort();
          break;
        case 's':
          sz = StatusStr();
          break;

        case 't':
        {
          time_t t = time(NULL);
          strftime(szTemp, 128, "%b %d %r", localtime(&t));
          sz = szTemp;
          break;
        }

        case 'T':
        {
          time_t t = time(NULL);
          strftime(szTemp, 128, "%b %d %R %Z", localtime(&t));
          sz = szTemp;
          break;
        }

        case 'z':
        {
          char zone = GetTimezone();
          if (zone == TIMEZONE_UNKNOWN)
            strcpy(szTemp, tr("Unknown"));
          else
            sprintf(szTemp, tr("GMT%c%i%c0"), (zone > 0 ? '-' : '+'), abs(zone / 2), (zone & 1 ? '3' : '0'));
          sz = szTemp;
          break;
        }

        case 'L':
        {
          char zone = GetTimezone();
          if (zone == TIMEZONE_UNKNOWN)
            strcpy(szTemp, tr("Unknown"));
          else
          {
            time_t t = time(NULL) - zone*30*60;
            struct tm ts;
            strftime(szTemp, 128, "%R", gmtime_r(&t, &ts));
          }

          sz = szTemp;
          break;
        }
        case 'F':
        {
          char zone = GetTimezone();
          if (zone == TIMEZONE_UNKNOWN)
            strcpy(szTemp, tr("Unknown"));
          else
          {
            time_t t = time(NULL) - zone*30*60;
            struct tm ts;
            strftime(szTemp, 128, "%c", gmtime_r(&t, &ts));
          }

          sz = szTemp;
          break;
        }

        case 'o':
          if(m_nLastCounters[LAST_ONLINE] == 0)
          {
            strcpy(szTemp, tr("Never"));
            sz = szTemp;
            break;
          }
          strftime(szTemp, 128, "%b %d %R", localtime(&m_nLastCounters[LAST_ONLINE]));
          sz = szTemp;
          break;
        case 'O':
          if (m_nStatus == ICQ_STATUS_OFFLINE || m_nOnlineSince == 0)
          {
            strcpy(szTemp, tr("Unknown"));
            sz = szTemp;
            break;
          }
          strftime(szTemp, 128, "%b %d %R", localtime(&m_nOnlineSince));
          sz = szTemp;
          break;

        case 'I':
        {
          if (m_nIdleSince)
          {
            unsigned short nDays, nHours, nMinutes;
            char szTime[128];
            time_t nIdleTime = (time(NULL) > m_nIdleSince ? time(NULL) - m_nIdleSince : 0);
            nDays = nIdleTime / ( 60 * 60 * 24);
            nHours = (nIdleTime % (60 * 60 * 24)) / (60 * 60);
            nMinutes = (nIdleTime % (60 * 60)) / 60;

            strcpy(szTemp, "");

            if (nDays)
            {
              if (nDays > 1)
                sprintf(szTime, tr("%d days "), nDays);
              else
                sprintf(szTime, tr("%d day "), nDays);
              strcat(szTemp, szTime);
            }

            if (nHours)
            {
              if (nHours > 1)
                sprintf(szTime, tr("%d hours "), nHours);
              else
                sprintf(szTime, tr("%d hour "), nHours);
              strcat(szTemp, szTime);
            }

            if (nMinutes)
            {
              if (nMinutes > 1)
                sprintf(szTime, tr("%d minutes"), nMinutes);
              else
                sprintf(szTime, tr("%d minute"), nMinutes);
              strcat(szTemp, szTime);
            }
          }
          else
            strcpy(szTemp, tr("Active"));

          sz = szTemp;

          break;
        }

        case 'm':
        case 'M':
          if (_szFormat[i] == 'm' || NewMessages())
            sprintf(szTemp, "%d", NewMessages());
          else
            szTemp[0] = '\0';
          sz = szTemp;
          break;
        case '%':
          strcpy(szTemp, "\%");
          sz = szTemp;
          break;
        default:
          gLog.Warn("%sWarning: Invalid qualifier in command: %%%c.\n",
                    L_WARNxSTR, _szFormat[i]);
          sprintf(szTemp, "%s%lu%%%c", (bLeft ? "-" : ""), nField, _szFormat[i]);
          sz = szTemp;
          bLeft = false;
          nField = 0;
          break;
      }

      if (!sz)
        continue;

      // If we need to be secure, then quote the % string
      if (bSecure)
      {
        _sz[nPos++] = '\'';
        CHECK_BUFFER;
      }

// The only way to escape a ' inside a ' is to do '\'' believe it or not
#define PACK_STRING(x)                          \
  while(x)                                      \
  {                                             \
    if (bSecure && sz[j] == '\'')               \
    {                                           \
      if (nPos >= bufSize - 5)                  \
      {                                         \
        bufSize *= 2;                           \
        _sz = (char *)realloc(_sz, bufSize);    \
      }                                         \
      nPos += sprintf(&_sz[nPos], "'\\''");     \
      j++;                                      \
    }                                           \
    else                                        \
    {                                           \
      _sz[nPos++] = sz[j++];                    \
      CHECK_BUFFER;                             \
    }                                           \
  }

      // Now append sz to the string using the given field width and alignment
      if (nField == 0)
      {
        j = 0;
        PACK_STRING(sz[j] != '\0');
      }
      else
      {
        if (bLeft)
        {
          j = 0;
          PACK_STRING(sz[j] != '\0');
          while(j++ < nField)
          {
            _sz[nPos++] = ' ';
            CHECK_BUFFER;
          }
        }
        else
        {
          int nLen = nField - strlen(sz);
          if (nLen < 0)
          {
            j = 0;
            //while(j < nField) _sz[nPos++] = sz[j++];
            PACK_STRING(j < nField);
          }
          else
          {
            for (j = 0; j < (unsigned long)nLen; j++)
            {
              _sz[nPos++] = ' ';
              CHECK_BUFFER;
            }
            j = 0;
            PACK_STRING(sz[j] != '\0');
          }
        }
      }

      // If we need to be secure, then quote the % string
      if (bSecure)
      {
        _sz[nPos++] = '\'';
        CHECK_BUFFER;
      }

      if (_szFormat[i] != '\0') i++;
    }
    else
    {
      if (_szFormat[i] == '\n')
      {
        if (nFlags & USPRINTF_NTORN)
        {
          _sz[nPos++] = '\r';
          CHECK_BUFFER;
        }
        if (nFlags & USPRINTF_PIPEISCMD)
          bSecure = (_szFormat[i + 1] == '|');
      }
      _sz[nPos++] = _szFormat[i++];
      CHECK_BUFFER;
    }
  }
  _sz[nPos] = '\0';

  return _sz;
}

//Return value must be delete []'d
char* LicqUser::MakeRealId(const string& accountId, unsigned long ppid,
                                char *&szRealId)
{
  if (accountId.empty())
  {
    szRealId = new char[1];
    szRealId[0] = '\0';
    return szRealId;
  }

  szRealId = new char[accountId.length() + 1];

  int j = 0;
  if (ppid == LICQ_PPID && !isdigit(accountId[0]))
  {
    for (unsigned int i = 0; i < accountId.length(); i++)
      if (accountId[i] != ' ')
        szRealId[j++] = tolower(accountId[i]);
    szRealId[j] = '\0';
  }
  else
    strcpy(szRealId, accountId.c_str());

  return szRealId;
}

string LicqUser::normalizeId(const string& accountId, unsigned long ppid)
{
  if (accountId.empty())
    return string();

  string realId = accountId;

  // TODO Make the protocol plugin normalize the accountId
  // For AIM, account id is case insensitive and spaces should be ignored
  if (ppid == LICQ_PPID && !isdigit(accountId[0]))
  {
    boost::erase_all(realId, " ");
    boost::to_lower(realId);
  }

  return realId;
}

void ICQUser::saveUserInfo()
{
  if (!EnableSave()) return;

  if (!m_fConf.ReloadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(),  L_BLANKxSTR);
     return;
  }
  m_fConf.SetSection("user");
  m_fConf.writeString("Alias", myAlias);
  m_fConf.WriteBool("KeepAliasOnUpdate", m_bKeepAliasOnUpdate);
  m_fConf.WriteNum("Timezone", m_nTimezone);
  m_fConf.WriteBool("Authorization", m_bAuthorization);

  PropertyMap::const_iterator i;
  for (i = myUserInfo.begin(); i != myUserInfo.end(); ++i)
    m_fConf.writeVar(i->first, i->second);

  saveCategory(myInterests, m_fConf, "Interests");
  saveCategory(myBackgrounds, m_fConf, "Backgrounds");
  saveCategory(myOrganizations, m_fConf, "Organizations");

  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
               L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
    return;
  }

  m_fConf.CloseFile();
}

void ICQUser::saveCategory(const UserCategoryMap& category, CIniFile& file, const string& key)
{
  file.WriteNum(key + 'N', category.size());

  UserCategoryMap::const_iterator i;
  unsigned int count = 0;
  for (i = category.begin(); i != category.end(); ++i)
  {
    char n[10];
    snprintf(n, sizeof(n), "%04X", count);
    file.WriteNum(key + "Cat" + n, i->first);
    file.writeString(key + "Desc" + n, i->second);
    ++count;
  }
}

void ICQUser::loadCategory(UserCategoryMap& category, CIniFile& file, const string& key)
{
  category.clear();
  unsigned int count;
  file.ReadNum(key + 'N', count, 0);

  if (count > MAX_CATEGORIES)
  {
    gLog.Warn("%sTrying to load more categories than the max limit. Truncating.\n", L_WARNxSTR);
    count = MAX_CATEGORIES;
  }

  for (unsigned int i = 0; i < count; ++i)
  {
    char n[10];
    snprintf(n, sizeof(n), "%04X", i);

    unsigned int cat;
    if (!file.ReadNum(key + "Cat" + n, cat))
      continue;

    string descr;
    if (!file.readString(key + "Desc" + n, descr))
      continue;

    category[cat] = descr;
  }
}

//-----ICQUser::SavePhoneBookInfo--------------------------------------------
void ICQUser::SavePhoneBookInfo()
{
  if (!EnableSave()) return;

  m_PhoneBook->SaveToDisk(m_fConf);
}

//-----ICQUser::SavePictureInfo----------------------------------------------
void ICQUser::SavePictureInfo()
{
  if (!EnableSave()) return;

  if (!m_fConf.ReloadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
     return;
  }
  m_fConf.SetSection("user");
  m_fConf.WriteBool("PicturePresent", m_bPicturePresent);
  m_fConf.WriteNum("BuddyIconType", m_nBuddyIconType);
  m_fConf.WriteNum("BuddyIconHashType", m_nBuddyIconHashType);
  m_fConf.WriteStr("BuddyIconHash", m_szBuddyIconHash);
  m_fConf.WriteStr("OurBuddyIconHash", m_szOurBuddyIconHash);
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
   m_fConf.WriteNum("Groups.System", GetSystemGroups());
   m_fConf.WriteStr("Ip", ip_ntoa(m_nIp, buf));
   m_fConf.WriteStr("IntIp", ip_ntoa(m_nIntIp, buf));
   m_fConf.WriteNum("Port", Port());
   m_fConf.WriteNum("NewMessages", NewMessages());
   m_fConf.WriteNum("LastOnline", (unsigned long)LastOnline());
   m_fConf.WriteNum("LastSent", (unsigned long)LastSentEvent());
   m_fConf.WriteNum("LastRecv", (unsigned long)LastReceivedEvent());
   m_fConf.WriteNum("LastCheckedAR", (unsigned long)LastCheckedAutoResponse());
   m_fConf.WriteNum("RegisteredTime", (unsigned long)RegisteredTime());
   m_fConf.WriteNum("AutoAccept", m_nAutoAccept);
   m_fConf.WriteNum("StatusToUser", m_nStatusToUser);
   m_fConf.WriteStr("CustomAutoRsp", CustomAutoResponse());
   m_fConf.WriteBool("SendIntIp", m_bSendIntIp);
   m_fConf.WriteStr("UserEncoding", m_szEncoding);
   m_fConf.WriteBool("AwaitingAuth", m_bAwaitingAuth);
   m_fConf.WriteNum("SID", m_nSID[NORMAL_SID]);
   m_fConf.WriteNum("InvisibleSID", m_nSID[INV_SID]);
   m_fConf.WriteNum("VisibleSID", m_nSID[VIS_SID]);
   m_fConf.WriteNum("GSID", m_nGSID);
   m_fConf.WriteNum("ClientTimestamp", m_nClientTimestamp);
   m_fConf.WriteNum("ClientInfoTimestamp", m_nClientInfoTimestamp);
   m_fConf.WriteNum("ClientStatusTimestamp", m_nClientStatusTimestamp);
   m_fConf.WriteNum("OurClientTimestamp", m_nOurClientTimestamp);
   m_fConf.WriteNum("OurClientInfoTimestamp", m_nOurClientInfoTimestamp);
   m_fConf.WriteNum("OurClientStatusTimestamp", m_nOurClientStatusTimestamp);
   m_fConf.WriteNum("PhoneFollowMeStatus", m_nPhoneFollowMeStatus);
   m_fConf.WriteNum("ICQphoneStatus", m_nICQphoneStatus);
   m_fConf.WriteNum("SharedFilesStatus", m_nSharedFilesStatus);
   m_fConf.WriteBool("UseGPG", m_bUseGPG );
   m_fConf.WriteStr("GPGKey", m_szGPGKey );
   m_fConf.WriteBool("SendServer", m_bSendServer);
   m_fConf.WriteNum("PPFieldCount", (unsigned short)m_mPPFields.size());
   
   std::map<string,string>::iterator iter;
   int i = 0;
   for (iter = m_mPPFields.begin(); iter != m_mPPFields.end(); ++iter)
   {
     char szBuf[25];
     sprintf(szBuf, "PPField%d.Name", ++i);
     m_fConf.WriteStr(szBuf, iter->first.c_str());
     sprintf(szBuf, "PPField%d.Value", i);
     m_fConf.WriteStr(szBuf, iter->second.c_str());
   }

  m_fConf.WriteNum("GroupCount", static_cast<unsigned int>(myGroups.size()));
  i = 1;
  for (UserGroupList::iterator g = myGroups.begin(); g != myGroups.end(); ++g)
  {
    sprintf(buf, "Group%u", i);
    m_fConf.WriteNum(buf, *g);
    ++i;
  }

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

void ICQUser::saveAll()
{
  SaveLicqInfo();
  saveUserInfo();
  SavePhoneBookInfo();
  SavePictureInfo();
}

//-----ICQUser::EventPush--------------------------------------------------------
void ICQUser::EventPush(CUserEvent *e)
{
  m_vcMessages.push_back(e);
  incNumUserEvents();
  SaveNewMessagesInfo();
  Touch();
  SetLastReceivedEvent();

  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER,
      USER_EVENTS, myId, e->Id(), e->ConvoId()));
}


void ICQUser::WriteToHistory(const char *_szText)
{
  m_fHistory.Append(_szText);
}



//-----ICQUser::GetEvent--------------------------------------------------------
void LicqUser::CancelEvent(unsigned short index)
{
  if (index < NewMessages())
    return;
  m_vcMessages[index]->Cancel();
}

const CUserEvent* ICQUser::EventPeek(unsigned short index) const
{
  if (index >= NewMessages()) return (NULL);
  return (m_vcMessages[index]);
}

const CUserEvent* ICQUser::EventPeekId(int id) const
{
  if (m_vcMessages.size() == 0) return NULL;
  CUserEvent *e = NULL;
  UserEventList::const_iterator iter;
  for (iter = m_vcMessages.begin(); iter != m_vcMessages.end(); ++iter)
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
const CUserEvent* ICQUser::EventPeekLast() const
{
  if (m_vcMessages.size() == 0) return (NULL);
  return (m_vcMessages[m_vcMessages.size() - 1]);
}

//-----ICQUser::EventPeekFirst----------------------------------------------------
const CUserEvent* ICQUser::EventPeekFirst() const
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

  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER,
      USER_EVENTS, myId, e->Id()));

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

  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER,
      USER_EVENTS, myId, -id));
}


void ICQUser::EventClearId(int id)
{
  UserEventList::iterator iter;
  for (iter = m_vcMessages.begin(); iter != m_vcMessages.end(); ++iter)
  {
    if ((*iter)->Id() == id)
    {
      delete *iter;
      m_vcMessages.erase(iter);
      decNumUserEvents();
      SaveNewMessagesInfo();

      gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER,
          USER_EVENTS, myId, -id));
      break;
    }
  }
}


bool ICQUser::GetInGroup(GroupType gtype, int groupId) const
{
  if (gtype == GROUPS_SYSTEM)
  {
    if (groupId < 0)
      return false;
    if (groupId == 0)
      return true;
    return (mySystemGroups & (1L << (groupId -1))) != 0;
  }
  else
    return myGroups.count(groupId) > 0;
}

void ICQUser::SetInGroup(GroupType g, int _nGroup, bool _bIn)
{
  if (_bIn)
    AddToGroup(g, _nGroup);
  else
    RemoveFromGroup(g, _nGroup);
}

void ICQUser::AddToGroup(GroupType gtype, int groupId)
{
  if (groupId <= 0)
    return;

  if (gtype == GROUPS_SYSTEM)
    mySystemGroups |= (1L << (groupId - 1));
  else
    myGroups.insert(groupId);
  SaveLicqInfo();
}

bool ICQUser::RemoveFromGroup(GroupType gtype, int groupId)
{
  if (groupId <= 0)
    return false;

  bool inGroup;
  if (gtype == GROUPS_SYSTEM)
  {
    unsigned long mask = 1L << (groupId - 1);
    inGroup = mySystemGroups & mask;
    mySystemGroups &= ~mask;
  }
  else
  {
    inGroup = myGroups.erase(groupId);
  }
  SaveLicqInfo();
  return inGroup;
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

bool ICQUser::SetPPField(const string &_sName, const string &_sValue)
{
  m_mPPFields[_sName] = _sValue;
  return true;
}

string ICQUser::GetPPField(const string &_sName)
{
  map<string,string>::iterator iter = m_mPPFields.find(_sName);
  if (iter != m_mPPFields.end())
    return iter->second;

  return string("");
}

void ICQUser::AddTLV(TLVPtr tlv)
{
  myTLVs[tlv->getType()] = tlv;
}

void ICQUser::RemoveTLV(unsigned long type)
{
  myTLVs.erase(type);
}

void ICQUser::SetTLVList(TLVList& tlvs)
{
  myTLVs.clear();

  for (TLVListIter it = tlvs.begin(); it != tlvs.end(); it++)
    myTLVs[it->first] = it->second;
}

//=====ICQOwner=================================================================

//-----ICQOwner::constructor----------------------------------------------------
LicqOwner::LicqOwner(const string& accountId, unsigned long ppid)
  : LicqUser(accountId, ppid, true)
{
  // Pretend to be temporary to LicqUser constructior so it doesn't setup m_fConf
  // Restore NotInList flag to proper value when we get here
  m_bNotInList = false;
  m_bOnContactList = true;

  char szTemp[MAX_LINE_LEN];
  char filename[MAX_FILENAME_LEN];
  m_bException = false;
  m_bSavePassword = true;
  m_szPassword = NULL;
  m_nPDINFO = 0;

  // Get data from the config file
  char* p = PPIDSTRING(ppid);
  snprintf(filename, MAX_FILENAME_LEN - 1, "%s/owner.%s", BASE_DIR, p);
  filename[MAX_FILENAME_LEN - 1] = '\0';

  // Make sure owner.Licq is mode 0600
  if (chmod(filename, S_IRUSR | S_IWUSR) == -1)
  {
    gLog.Warn(tr("%sUnable to set %s to mode 0600.  Your ICQ password is vulnerable.\n"),
                 L_WARNxSTR, filename);
  }

  m_fConf.SetFileName(filename);
  m_fConf.SetFlags(INI_FxWARN | INI_FxALLOWxCREATE);
  m_fConf.ReloadFile();
  m_fConf.SetFlags(0);

  // And finally our favorite function
  LoadInfo();
  // Owner encoding fixup to be UTF-8 by default
  if (strcmp(m_szEncoding, "") == 0)
    SetString(&m_szEncoding, "UTF-8");
  m_fConf.ReadStr("Password", szTemp, "", false);
  SetPassword(&szTemp[1]); // skip leading space since we didn't trim
  m_fConf.ReadBool("WebPresence", m_bWebAware, false);
  m_fConf.ReadBool("HideIP", m_bHideIp, false);
  m_fConf.ReadNum("RCG", m_nRandomChatGroup, ICQ_RANDOMxCHATxGROUP_NONE);
  m_fConf.ReadStr("AutoResponse", szTemp, "");
  m_fConf.ReadNum("SSTime", m_nSSTime, 0L);
  m_fConf.ReadNum("SSCount", m_nSSCount, 0);
  m_fConf.ReadNum("PDINFO", m_nPDINFO, 0);
  
  SetAutoResponse(szTemp);

  m_fConf.CloseFile();

  gLog.Info(tr("%sOwner configuration for %s.\n"), L_INITxSTR, myAccountId.c_str());

  snprintf(filename, MAX_FILENAME_LEN - 1, "%s/%s/owner.%s.%s.history", BASE_DIR, HISTORY_DIR,
      myAccountId.c_str(), p);
    SetHistoryFile(filename);

  if (m_nTimezone != SystemTimezone() && m_nTimezone != TIMEZONE_UNKNOWN)
  {
    gLog.Warn(tr("%sCurrent Licq GMT offset (%d) does not match system GMT offset (%d).\n"
              "%sUpdate general info on server to fix.\n"),
       L_WARNxSTR, m_nTimezone, SystemTimezone(), L_BLANKxSTR);
  }
  SetEnableSave(true);

  delete [] p;
}

LicqOwner::~LicqOwner()
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
  m_fConf.WriteNum("SSTime", (unsigned long)m_nSSTime);
  m_fConf.WriteNum("SSCount", m_nSSCount);
  m_fConf.WriteNum("PDINFO", m_nPDINFO);
  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
               L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
    return;
  }
  m_fConf.CloseFile();

  if ( m_szPassword )
    free( m_szPassword );
}

unsigned long ICQOwner::AddStatusFlags(unsigned long s) const
{
  s &= 0x0000FFFF;

  if (WebAware())
    s |= ICQ_STATUS_FxWEBxPRESENCE;
  if (HideIp())
    s |= ICQ_STATUS_FxHIDExIP;
  if (Birthday() == 0)
    s |= ICQ_STATUS_FxBIRTHDAY;
  if (PhoneFollowMeStatus() != ICQ_PLUGIN_STATUSxINACTIVE)
    s |= ICQ_STATUS_FxPFM;
  if (PhoneFollowMeStatus() == ICQ_PLUGIN_STATUSxACTIVE)
    s |= ICQ_STATUS_FxPFMxAVAILABLE;

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
  m_fConf.writeString("Uin", accountId());
  m_fConf.WriteBool("WebPresence", WebAware());
  m_fConf.WriteBool("HideIP", HideIp());
  m_fConf.WriteBool("Authorization", GetAuthorization());
  m_fConf.WriteNum("RCG", RandomChatGroup());
  m_fConf.WriteNum("SSTime", (unsigned long)m_nSSTime);
  m_fConf.WriteNum("SSCount", m_nSSCount);
  m_fConf.WriteNum("PDINFO", m_nPDINFO);
  
  if (m_bSavePassword)
    m_fConf.WriteStr("Password", Password());
  else
    m_fConf.WriteStr("Password", "");

  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
               L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
    return;
  }

  m_fConf.CloseFile();
}

void ICQOwner::SetStatusOffline()
{
  SetStatus(m_nStatus | ICQ_STATUS_OFFLINE);
}

//----ICQOwner::SetPicture---------------------------------------------------
void ICQOwner::SetPicture(const char *f)
{
  char szFilename[MAX_FILENAME_LEN];
  szFilename[MAX_FILENAME_LEN - 1] = '\0';
  snprintf(szFilename, MAX_FILENAME_LEN - 1, "%s/owner.pic", BASE_DIR);
  if (f == NULL)
  {
    SetPicturePresent(false);
    if (remove(szFilename) != 0 && errno != ENOENT)
    {
      gLog.Error("%sUnable to delete %s's picture file (%s):\n%s%s.\n",
          L_ERRORxSTR, myAlias.c_str(), szFilename, L_BLANKxSTR,
                         strerror(errno));
    }
  }
  else if (strcmp(f, szFilename) == 0)
  {
    SetPicturePresent(true);
    return;
  }
  else
  {
    int source = open(f, O_RDONLY);
    if (source == -1)
    {
      gLog.Error("%sUnable to open source picture file (%s):\n%s%s.\n",
                       L_ERRORxSTR, f, L_BLANKxSTR, strerror(errno));
      return;
    }

    int dest = open(szFilename, O_WRONLY | O_CREAT | O_TRUNC, 00664);
    if (dest == -1)
    {
      gLog.Error("%sUnable to open picture file (%s):\n%s%s.\n", L_ERRORxSTR,
                                     szFilename, L_BLANKxSTR, strerror(errno));
      return;
    }

    char buf[8192];
    ssize_t s;
    while (1)
    {
      s = read(source, buf, sizeof(buf));
      if (s == 0)
      {
        SetPicturePresent(true);
        break;
      }
      else if (s == -1)
      {
        gLog.Error("%sError reading from %s:\n%s%s.\n", L_ERRORxSTR, f,
                                         L_BLANKxSTR, strerror(errno));
        SetPicturePresent(false);
        break;
      }

      if (write(dest, buf, s) != s)
      {
        gLog.Error("%sError writing to %s:\n%s%s.\n", L_ERRORxSTR, f,
                                         L_BLANKxSTR, strerror(errno));
        SetPicturePresent(false);
        break;
      }
    }

    close(source);
    close(dest);
  }
}
 
