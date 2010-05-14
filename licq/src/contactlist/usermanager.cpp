#include "usermanager.h"

#include <boost/foreach.hpp>
#include <cstdio> // sprintf

#include "gettext.h"
#include <licq_icq.h>
#include "licq_icqd.h"
#include "licq_log.h"
#include "licq_events.h"

#include "../protocolmanager.h"
#include "group.h"
#include "owner.h"
#include "user.h"

using std::list;
using std::string;
using Licq::GroupListGuard;
using Licq::GroupReadGuard;
using Licq::GroupWriteGuard;
using Licq::OwnerListGuard;
using Licq::OwnerReadGuard;
using Licq::OwnerWriteGuard;
using Licq::UserListGuard;
using Licq::UserId;
using Licq::UserGroupList;
using Licq::UserReadGuard;
using Licq::UserWriteGuard;
using namespace LicqDaemon;

// Declare global UserManager (internal for daemon)
LicqDaemon::UserManager LicqDaemon::gUserManager;

// Initialize global Licq::UserManager to refer to the internal UserManager
Licq::UserManager& Licq::gUserManager(LicqDaemon::gUserManager);


UserManager::UserManager()
  : m_szDefaultEncoding(NULL)
{
  // Set up the basic all users and new users group
  myGroupListMutex.setName("grouplist");
  myUserListMutex.setName("userlist");
  myOwnerListMutex.setName("ownerlist");
}


UserManager::~UserManager()
{
  // Empty
}

void UserManager::shutdown()
{
  UserMap::iterator iter;
  for (iter = myUsers.begin(); iter != myUsers.end(); ++iter)
    delete iter->second;
  myUsers.clear();

  GroupMap::iterator g_iter;
  for (g_iter = myGroups.begin(); g_iter != myGroups.end(); ++g_iter)
    delete g_iter->second;
  myGroups.clear();

  OwnerMap::iterator o_iter;
  for (o_iter = myOwners.begin(); o_iter != myOwners.end(); ++o_iter)
    delete o_iter->second;
  myOwners.clear();

  if (m_szDefaultEncoding != NULL)
    free(m_szDefaultEncoding);
}

void UserManager::addOwner(const UserId& userId)
{
  Owner* o = new Owner(userId);

  myOwnerListMutex.lockWrite();
  myOwners[userId.protocolId()] = o;
  myOwnerListMutex.unlockWrite();

  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_OWNERxLIST,
      LIST_OWNER_ADDED, userId));
}

/*---------------------------------------------------------------------------
 * UserManager::Load
 *-------------------------------------------------------------------------*/
bool UserManager::Load()
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
  myOwnerListMutex.lockWrite();
  for (unsigned short i = 1; i <= nOwners; i++)
  {
    sprintf(sOwnerIDKey, "Owner%d.Id", i);
    licqConf.ReadStr(sOwnerIDKey, sOwnerID);
    sprintf(sOwnerPPIDKey, "Owner%d.PPID", i);
    licqConf.ReadStr(sOwnerPPIDKey, sOwnerPPID);
    nPPID = (sOwnerPPID[0] << 24) | (sOwnerPPID[1] << 16) |
            (sOwnerPPID[2] << 8) | (sOwnerPPID[3]);

    UserId ownerId(sOwnerID, nPPID);
    Owner* o = new Owner(ownerId);

    myOwners[nPPID] = o;
  }
  myOwnerListMutex.unlockWrite();

  unsigned int nGroups;
  licqConf.SetSection("groups");
  licqConf.ReadNum("NumOfGroups", nGroups);

  myGroupListMutex.lockWrite();
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

    Group* newGroup = new Group(groupId, groupName);
    newGroup->setSortIndex(sortIndex);

    list<string> serverIdKeys;
    sprintf(key, "Group%d.ServerId.", i);
    licqConf.getKeyList(serverIdKeys, key);
    BOOST_FOREACH(const string& serverIdKey, serverIdKeys)
    {
      size_t keylen = serverIdKey.size();
      unsigned long protocolId = serverIdKey[keylen-4] << 24 |
          serverIdKey[keylen-3] << 16 | serverIdKey[keylen-2] << 8 |
          serverIdKey[keylen-1];
      unsigned long serverId;
      licqConf.ReadNum(serverIdKey, serverId, 0);
      newGroup->setServerId(protocolId, serverId);
    }

    // ServerId per protocol didn't exist in 1.3.x and older.
    // This will preserve ICQ group ids when reading old config.
    if (serverIdKeys.size() == 0 && icqGroupId != 0)
      newGroup->setServerId(LICQ_PPID, icqGroupId);

    myGroups[groupId] = newGroup;

    licqConf.SetFlags(INI_FxFATAL | INI_FxERROR | INI_FxWARN);
  }
  m_bAllowSave = true;
  myGroupListMutex.unlockWrite();


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
  User* u;
  usersConf.SetFlags(INI_FxWARN);
  myUserListMutex.lockWrite();
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

    UserId userId(szId, nPPID);
    u = new User(userId, string(filename));
    u->AddToContactList();
    myUsers[userId] = u;
  }
  myUserListMutex.unlockWrite();

  return true;
}

void UserManager::saveUserList() const
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

    char ps[5];
    Licq::protocolId_toStr(ps, ppid);
    usersConf.writeString(key, accountId + "." + ps);
  }
  usersConf.WriteNum("NumOfUsers", count);
  usersConf.FlushFile();
  usersConf.CloseFile();
}

bool UserManager::addUser(const UserId& uid,
    bool permanent, bool addToServer, unsigned short groupId)
{
  if (!uid.isValid())
    return false;

  if (isOwner(uid))
    return false;

  myUserListMutex.lockWrite();

  // Make sure user isn't already in the list
  UserMap::const_iterator iter = myUsers.find(uid);
  if (iter != myUsers.end())
  {
    myUserListMutex.unlockWrite();
    return false;
  }

  User* pUser = new User(uid, !permanent);
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

  myUserListMutex.unlockWrite();

  // Notify plugins that user was added
  // Send this before adding user to server side as protocol code may generate updated signals
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_ADD, uid, groupId));

  // Add user to server side list
  if (permanent && addToServer)
    gProtocolManager.addUser(uid, groupId);

  // Set initial group membership, also sets server group for user
  if (groupId != 0)
    setUserInGroup(uid, groupId, true, permanent && addToServer);

  return true;
}

bool UserManager::makeUserPermanent(const UserId& userId, bool addToServer,
    int groupId)
{
  if (!userId.isValid())
    return false;

  if (isOwner(userId))
    return false;

  {
    Licq::UserWriteGuard user(userId);
    if (!user.isLocked())
      return false;

    // Check if user is already permanent
    if (!user->NotInList())
      return false;

    user->SetPermanent();
  }

  // Add user to server side list
  if (addToServer)
    gProtocolManager.addUser(userId, groupId);

  // Set initial group membership, also sets server group for user
  if (groupId != 0)
    setUserInGroup(userId, groupId, true, addToServer);

  return true;
}

void UserManager::removeUser(const UserId& userId, bool removeFromServer)
{
  // Remove the user from the server side list first
  if (removeFromServer)
    gProtocolManager.removeUser(userId);

  // List should only be locked when not holding any user lock to avoid
  // deadlock, so we cannot call FetchUser here.
  myUserListMutex.lockWrite();
  UserMap::iterator iter = myUsers.find(userId);
  if (iter == myUsers.end())
  {
    myUserListMutex.unlockWrite();
    return;
  }

  User* u = iter->second;
  u->Lock(LOCK_W);
  myUsers.erase(iter);
  if (!u->NotInList())
  {
    u->RemoveFiles();
    saveUserList();
  }
  myUserListMutex.unlockWrite();
  u->Unlock();
  delete u;

  // Notify plugins about the removed user
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_REMOVE, userId));
}

// Need to call CICQDaemon::SaveConf() after this
void UserManager::RemoveOwner(unsigned long ppid)
{
  // List should only be locked when not holding any user lock to avoid
  // deadlock, so we cannot call FetchOwner here.
  myOwnerListMutex.lockWrite();
  OwnerMap::iterator iter = myOwners.find(ppid);
  if (iter == myOwners.end())
  {
    myOwnerListMutex.unlockWrite();
    return;
  }

  Owner* o = iter->second;
  o->Lock(LOCK_W);
  myOwners.erase(iter);
  o->RemoveFiles();
  UserId id = o->id();
  myOwnerListMutex.unlockWrite();
  o->Unlock();
  delete o;

  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_OWNERxLIST,
        LIST_OWNER_REMOVED, id));
}

Licq::User* UserManager::fetchUser(const UserId& userId,
    unsigned short lockType, bool addUser, bool* retWasAdded)
{
  if (retWasAdded != NULL)
    *retWasAdded = false;

  User* user = NULL;

  if (!userId.isValid())
    return NULL;

  // Check for an owner first
  myOwnerListMutex.lockRead();
  OwnerMap::iterator iter_o = myOwners.find(User::getUserProtocolId(userId));
  if (iter_o != myOwners.end())
  {
    if (iter_o->second->id() == userId)
    {
      user = iter_o->second;
      user->Lock(lockType);
    }
  }
  myOwnerListMutex.unlockRead();

  if (user != NULL)
    return user;

  if (addUser)
    myUserListMutex.lockWrite();
  else
    myUserListMutex.lockRead();

  UserMap::const_iterator iter = myUsers.find(userId);
  if (iter != myUsers.end())
    user = iter->second;

    // If allowed by caller, add user if it wasn't found in list
    if (user == NULL && addUser)
    {
      // Create a temporary user
      user = new User(userId, true);

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

  if (addUser)
    myUserListMutex.unlockWrite();
  else
    myUserListMutex.unlockRead();

  return user;
}

bool UserManager::userExists(const UserId& userId)
{
  bool exists = false;

  myOwnerListMutex.lockRead();
  OwnerMap::iterator iter_o = myOwners.find(User::getUserProtocolId(userId));
  if (iter_o != myOwners.end())
  {
    if (iter_o->second->id() == userId)
      exists = true;
  }
  myOwnerListMutex.unlockRead();
  if (exists)
    return true;

  myUserListMutex.lockRead();
  UserMap::const_iterator iter = myUsers.find(userId);
  if (iter != myUsers.end())
    exists = true;
  myUserListMutex.unlockRead();
  return exists;
}

UserId UserManager::ownerUserId(unsigned long ppid)
{
  const Licq::Owner* owner = FetchOwner(ppid, LOCK_R);
  if (owner == NULL)
    return UserId();

  UserId ret = owner->id();
  DropOwner(owner);
  return ret;
}

string UserManager::OwnerId(unsigned long ppid)
{
  const Licq::Owner* owner = FetchOwner(ppid, LOCK_R);
  if (owner == NULL)
    return "";

  string ret = owner->accountId();
  DropOwner(owner);
  return ret;
}

bool UserManager::isOwner(const UserId& userId)
{
  bool exists = false;

  myOwnerListMutex.lockRead();
  OwnerMap::iterator iter_o = myOwners.find(User::getUserProtocolId(userId));
  if (iter_o != myOwners.end())
  {
    if (iter_o->second->id() == userId)
      exists = true;
  }
  myOwnerListMutex.unlockRead();

  return exists;
}

unsigned long UserManager::icqOwnerUin()
{
  return strtoul(OwnerId(LICQ_PPID).c_str(), (char**)NULL, 10);
}

void UserManager::notifyUserUpdated(const UserId& userId, unsigned long subSignal)
{
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER, subSignal, userId));
}

Group* UserManager::fetchGroup(int group, bool writeLock)
{
  myGroupListMutex.lockRead();
  GroupMap::const_iterator iter = myGroups.find(group);
  Group* g = NULL;
  if (iter != myGroups.end())
  {
    g = iter->second;
    if (writeLock)
      g->lockWrite();
    else
      g->lockRead();
  }
  myGroupListMutex.unlockRead();
  return g;
}

bool UserManager::groupExists(int groupId)
{
  myGroupListMutex.lockRead();
  GroupMap::const_iterator iter = myGroups.find(groupId);
  bool found = (iter != myGroups.end());
  myGroupListMutex.unlockRead();
  return found;
}

int UserManager::AddGroup(const string& name, unsigned short icqGroupId)
{
  if (name.empty())
    return 0;

  if (GetGroupFromName(name) != 0)
  {
    // Don't allow a duplicate name
    gLog.Warn(tr("%sGroup %s is already in list.\n"), L_WARNxSTR, name.c_str());
    return 0;
  }

  myGroupListMutex.lockWrite();

  // Find first free group id
  int gid;
  for (gid = 1; myGroups.count(gid) != 0 ; ++gid)
    ;

  Group* newGroup = new Group(gid, name);
  newGroup->setServerId(LICQ_PPID, icqGroupId);
  newGroup->setSortIndex(myGroups.size());
  myGroups[gid] = newGroup;

  SaveGroups();
  myGroupListMutex.unlockWrite();

  bool icqOnline = false;
  Licq::Owner* icqOwner = FetchOwner(LICQ_PPID, LOCK_R);
  if (icqOwner != NULL)
  {
    icqOnline = icqOwner->isOnline();
    DropOwner(icqOwner);
  }

  if (gLicqDaemon != NULL)
  {
    if (icqGroupId == 0 && icqOnline)
      gLicqDaemon->icqAddGroup(name.c_str());
    else
      gLog.Info(tr("%sAdded group %s (%u) to list from server.\n"),
          L_SRVxSTR, name.c_str(), icqGroupId);

    // Send signal to let plugins know of the new group
    gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_GROUP_ADDED, UserId(), gid));
  }

  return gid;
}

void UserManager::RemoveGroup(int groupId)
{
  Group* group = fetchGroup(groupId);
  if (group == NULL)
    return;

  string name = group->name();
  int sortIndex = group->sortIndex();
  group->unlockRead();

  // Must be called when there are no locks on GroupID and Group lists
  gLicqDaemon->icqRemoveGroup(name.c_str());

  // Lock it back up
  myGroupListMutex.lockWrite();
  group->lockWrite();

  // Erase the group
  myGroups.erase(groupId);
  group->unlockWrite();
  delete group;


  // Decrease sorting index for higher groups so we don't leave a gap
  GroupMap::const_iterator iter;
  for (iter = myGroups.begin(); iter != myGroups.end(); ++iter)
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
    if (pUser->removeFromGroup(groupId))
      notifyUserUpdated(pUser->id(), USER_GROUPS);
  }
  FOR_EACH_USER_END;

  SaveGroups();
  myGroupListMutex.unlockWrite();

  // Send signal to let plugins know of the removed group
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_GROUP_REMOVED, UserId(), groupId));

  // Send signal to let plugins know that sorting indexes may have changed
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_GROUP_REORDERED));
}

void UserManager::ModifyGroupSorting(int groupId, int newIndex)
{
  Group* group = fetchGroup(groupId);
  if (group == NULL)
    return;

  if (newIndex < 0)
    newIndex = 0;
  if (static_cast<unsigned int>(newIndex) >= NumGroups())
    newIndex = NumGroups() - 1;

  int oldIndex = group->sortIndex();
  group->unlockRead();

  myGroupListMutex.lockRead();

  // Move all groups between new and old position one step
  for (GroupMap::iterator i = myGroups.begin(); i != myGroups.end(); ++i)
  {
    i->second->lockWrite();
    int si = i->second->sortIndex();
    if (newIndex < oldIndex && si >= newIndex && si < oldIndex)
      i->second->setSortIndex(si + 1);
    else if (newIndex > oldIndex && si > oldIndex && si <= newIndex)
      i->second->setSortIndex(si - 1);
    i->second->unlockWrite();
  }

  group->lockWrite();
  group->setSortIndex(newIndex);
  group->unlockWrite();

  SaveGroups();
  myGroupListMutex.unlockRead();

  // Send signal to let plugins know that sorting indexes have changed
  if (gLicqDaemon != NULL)
    gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_GROUP_REORDERED));
}

bool UserManager::RenameGroup(int groupId, const string& name, bool sendUpdate)
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

  Group* group = fetchGroup(groupId, true);
  if (group == NULL)
  {
    gLog.Warn(tr("%sRenaming request for invalid group %u.\n"), L_WARNxSTR, groupId);
    return false;
  }

  group->setName(name);
  unsigned short icqGroupId = group->serverId(LICQ_PPID);
  group->unlockWrite();

  myGroupListMutex.lockRead();
  SaveGroups();
  myGroupListMutex.unlockRead();

  if (gLicqDaemon != NULL)
  {
    // If we rename a group on logon, don't send the rename packet
    if (sendUpdate)
      gLicqDaemon->icqRenameGroup(name.c_str(), icqGroupId);

    // Send signal to let plugins know the group has changed
    gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_GROUP_CHANGED, UserId(), groupId));
  }

  return true;
}

void UserManager::SaveGroups()
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

  int i = 1;
  for (GroupMap::iterator group = myGroups.begin(); group != myGroups.end(); ++group)
  {
    group->second->Lock(LOCK_R);
    group->second->save(licqConf, i);
    group->second->Unlock();
    ++i;
  }

  licqConf.FlushFile();
  licqConf.CloseFile();
}

unsigned short UserManager::GetIDFromGroup(const string& name)
{
  int groupId = GetGroupFromName(name);
  if (groupId == 0)
    return 0;

  return GetIDFromGroup(groupId);
}

unsigned short UserManager::GetIDFromGroup(int groupId)
{
  Group* group = fetchGroup(groupId);
  if (group == NULL)
    return 0;

  unsigned short icqGroupId = group->serverId(LICQ_PPID);
  group->unlockRead();

  return icqGroupId;
}

int UserManager::GetGroupFromID(unsigned short icqGroupId)
{
  myGroupListMutex.lockRead();
  GroupMap::const_iterator iter;
  int groupId = 0;
  for (iter = myGroups.begin(); iter != myGroups.end(); ++iter)
  {
    iter->second->Lock(LOCK_R);
    if (iter->second->serverId(LICQ_PPID) == icqGroupId)
      groupId = iter->first;
    iter->second->Unlock();
  }
  myGroupListMutex.unlockRead();

  return groupId;
}

int UserManager::GetGroupFromName(const string& name)
{
  myGroupListMutex.lockRead();
  GroupMap::const_iterator iter;
  int id = 0;
  for (iter = myGroups.begin(); iter != myGroups.end(); ++iter)
  {
    iter->second->Lock(LOCK_R);
    if (iter->second->name() == name)
      id = iter->first;
    iter->second->Unlock();
  }
  myGroupListMutex.unlockRead();

  return id;
}

std::string UserManager::GetGroupNameFromGroup(int groupId)
{
  myGroupListMutex.lockRead();
  GroupMap::const_iterator iter;
  std::string name;
  for (iter = myGroups.begin(); iter != myGroups.end(); ++iter)
  {
    iter->second->Lock(LOCK_R);
    if (iter->second->id() == groupId)
      name = iter->second->name();
    iter->second->Unlock();
  }
  myGroupListMutex.unlockRead();

  return name;
}

void UserManager::ModifyGroupID(const string& name, unsigned short icqGroupId)
{
  int id = GetGroupFromName(name);
  if (id != 0)
    ModifyGroupID(id, icqGroupId);
}

void UserManager::ModifyGroupID(int groupId, unsigned short icqGroupId)
{
  Group* group = fetchGroup(groupId, true);
  if (group == NULL)
    return;

  group->setServerId(LICQ_PPID, icqGroupId);
  group->unlockWrite();

  myGroupListMutex.lockRead();
  SaveGroups();
  myGroupListMutex.unlockRead();
}

unsigned short UserManager::GenerateSID()
{
  bool bCheckGroup, bDone;
  int nSID;
  unsigned short nOwnerPDINFO;

  const Licq::Owner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
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
        unsigned short icqGroupId = pGroup->serverId(LICQ_PPID);
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

void UserManager::DropUser(const Licq::User* u)
{
  if (u == NULL) return;
  u->Unlock();
}

Licq::Owner* UserManager::FetchOwner(unsigned long ppid, unsigned short lockType)
{
  Owner* o = NULL;

  myOwnerListMutex.lockRead();
  OwnerMap::iterator iter = myOwners.find(ppid);
  if (iter != myOwners.end())
  {
    o = iter->second;
    o->Lock(lockType);
  }
  myOwnerListMutex.unlockRead();

  return o;
}

void UserManager::DropOwner(const Licq::Owner* owner)
{
  if (owner == NULL)
    return;
  owner->Unlock();
}

void UserManager::SaveAllUsers()
{
  FOR_EACH_USER_START(LOCK_R)
  {
    if (!pUser->NotInList())
      pUser->saveAll();
  }
  FOR_EACH_USER_END
}

bool UserManager::UpdateUsersInGroups()
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
        pUser->addToGroup(nInGroup);
        bDid = true;
      }
    }
  }
  FOR_EACH_PROTO_USER_END

  return bDid;
}

unsigned short UserManager::NumUsers()
{
  //myUserListMutex.lockRead();
  unsigned short n = myUsers.size();
  //myUserListMutex.unlockRead();
  return n;
}

unsigned short UserManager::NumOwners()
{
  unsigned short n = myOwners.size();
  return n;
}

unsigned int UserManager::NumGroups()
{
  //myGroupListMutex.lockRead();
  unsigned int n = myGroups.size();
  //myGroupListMutex.unlockRead();
  return n;
}

const UserMap& UserManager::lockUserList()
{
  myUserListMutex.lockRead();
  return myUsers;
}

void UserManager::unlockUserList()
{
  myUserListMutex.unlockRead();
}

const GroupMap& UserManager::lockGroupList()
{
  myGroupListMutex.lockRead();
  return myGroups;
}

void UserManager::unlockGroupList()
{
  myGroupListMutex.unlockRead();
}

const OwnerMap& UserManager::lockOwnerList()
{
  myOwnerListMutex.lockRead();
  return myOwners;
}

void UserManager::unlockOwnerList()
{
  myOwnerListMutex.unlockRead();
}

void UserManager::setUserInGroup(const UserId& userId, int groupId,
      bool inGroup, bool updateServer)
{
  // User group 0 is invalid
  if (groupId == 0)
    return;

  Licq::User* u = gUserManager.fetchUser(userId, LOCK_W);
  if (u == NULL) 
    return;

  int gsid = u->GetGSID();

  if (!inGroup && u->GetSID() != 0 && GetGroupFromID(gsid) == groupId)
  {
    // Don't remove user from local group if member of the same server group
    gUserManager.DropUser(u);
    return;
  }

  // Update user object
  u->setInGroup(groupId, inGroup);
  Licq::StringList groupNames;
  std::string groupName;
  UserGroupList groups = u->GetGroups();
  UserGroupList::const_iterator it;
  for (it = groups.begin(); it != groups.end(); it++)
  {
    groupName = GetGroupNameFromGroup(*it);
    if (!groupName.empty())
      groupNames.push_back(groupName);
  }
  string accountId = u->accountId();
  unsigned long ppid = u->ppid();
  gUserManager.DropUser(u);

  // Notify server
  if (updateServer && gLicqDaemon != NULL)
  {
    if (ppid == LICQ_PPID)
    {
      if (inGroup) // Server group can only be changed, not removed
        gLicqDaemon->icqChangeGroup(accountId.c_str(), ppid, groupId, gsid,
            ICQ_ROSTxNORMAL, ICQ_ROSTxNORMAL);
    }
    else
      gLicqDaemon->PushProtoSignal(
          new LicqProtoChangeUserGroupsSignal(userId, groupNames), ppid);
  }

  // Notify plugins
  if (gLicqDaemon != NULL)
    notifyUserUpdated(userId, USER_GROUPS);
}

void UserManager::userStatusChanged(const UserId& userId, unsigned newStatus)
{
  Licq::UserWriteGuard u(userId);
  if (u.isLocked())
    u->statusChanged(newStatus);
}

void UserManager::ownerStatusChanged(unsigned long protocolId, unsigned newStatus)
{
  Licq::OwnerWriteGuard o(protocolId);
  if (o.isLocked())
    o->statusChanged(newStatus);
}

const char* UserManager::DefaultUserEncoding()
{
  return m_szDefaultEncoding;
}

void UserManager::SetDefaultUserEncoding(const char* defaultEncoding)
{
  SetString(&m_szDefaultEncoding, defaultEncoding);
}

char* PPIDSTRING(unsigned long ppid)
{
  char* ret = new char[5];
  return Licq::protocolId_toStr(ret, ppid);
}


UserReadGuard::UserReadGuard(const UserId& userId, bool addUser, bool* retWasAdded)
  : ReadMutexGuard<User>(gUserManager.fetchUser(userId, LOCK_R, addUser, retWasAdded), true)
{
  // Empty
}

UserWriteGuard::UserWriteGuard(const UserId& userId, bool addUser, bool* retWasAdded)
  : WriteMutexGuard<User>(gUserManager.fetchUser(userId, LOCK_W, addUser, retWasAdded), true)
{
  // Empty
}

OwnerReadGuard::OwnerReadGuard(const UserId& userId)
  : ReadMutexGuard<Owner>(gUserManager.fetchOwner(userId, LOCK_R), true)
{
  // Empty
}

OwnerReadGuard::OwnerReadGuard(unsigned long protocolId)
  : ReadMutexGuard<Owner>(gUserManager.FetchOwner(protocolId, LOCK_R), true)
{
  // Empty
}

OwnerWriteGuard::OwnerWriteGuard(const UserId& userId)
  : WriteMutexGuard<Owner>(gUserManager.fetchOwner(userId, LOCK_W), true)
{
  // Empty
}

OwnerWriteGuard::OwnerWriteGuard(unsigned long protocolId)
  : WriteMutexGuard<Owner>(gUserManager.FetchOwner(protocolId, LOCK_W), true)
{
  // Empty
}

GroupReadGuard::GroupReadGuard(int groupId)
  : ReadMutexGuard<Group>(LicqDaemon::gUserManager.fetchGroup(groupId), true)
{
  // Empty
}

GroupWriteGuard::GroupWriteGuard(int groupId)
  : WriteMutexGuard<Group>(LicqDaemon::gUserManager.fetchGroup(groupId, true), true)
{
  // Empty
}


UserListGuard::UserListGuard(unsigned long protocolId)
{
  const UserMap& userMap = LicqDaemon::gUserManager.lockUserList();

  for (UserMap::const_iterator i = userMap.begin(); i != userMap.end(); ++i)
    if (protocolId == 0 || i->first.protocolId() == protocolId)
      myUserList.push_back(i->second);
}

UserListGuard::~UserListGuard()
{
  LicqDaemon::gUserManager.unlockUserList();
}

OwnerListGuard::OwnerListGuard()
{
  const OwnerMap& ownerMap = LicqDaemon::gUserManager.lockOwnerList();

  for (OwnerMap::const_iterator i = ownerMap.begin(); i != ownerMap.end(); ++i)
    myOwnerList.push_back(i->second);
}

OwnerListGuard::~OwnerListGuard()
{
  LicqDaemon::gUserManager.unlockOwnerList();
}

GroupListGuard::GroupListGuard(bool sorted)
{
  const GroupMap& groupMap = LicqDaemon::gUserManager.lockGroupList();

  for (GroupMap::const_iterator i = groupMap.begin(); i != groupMap.end(); ++i)
    myGroupList.push_back(i->second);

  if (sorted)
    myGroupList.sort(compare_groups);
}

GroupListGuard::~GroupListGuard()
{
  LicqDaemon::gUserManager.unlockGroupList();
}
