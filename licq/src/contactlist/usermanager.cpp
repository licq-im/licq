#include "usermanager.h"

#include <boost/foreach.hpp>
#include <cstdio> // sprintf

#include "gettext.h"
#include "licq_icqd.h"
#include "licq_log.h"
#include "licq_events.h"

#include "../protocolmanager.h"

using std::list;
using std::string;
using Licq::Group;
using Licq::GroupMap;
using Licq::GroupType;
using Licq::Owner;
using Licq::OwnerMap;
using Licq::User;
using Licq::UserId;
using Licq::UserGroupList;
using Licq::UserMap;
using Licq::GROUP_IGNORE_LIST;
using Licq::GROUP_INVISIBLE_LIST;
using Licq::GROUP_VISIBLE_LIST;
using Licq::GROUPS_USER;
using Licq::GROUPS_SYSTEM;
using Licq::NUM_GROUPS_SYSTEM_ALL;
using namespace LicqDaemon;

// Declare global UserManager (internal for daemon)
LicqDaemon::UserManager LicqDaemon::gUserManager;

// Initialize global Licq::UserManager to refer to the internal UserManager
Licq::UserManager& Licq::gUserManager(LicqDaemon::gUserManager);


/* This array is provided for plugin writers' convenience only.
 * No translation is done here. Thus, if your plugin wishes to translate
 * these names, replicate the array into your plugin and do it there.
 */
const char* Licq::GroupsSystemNames[NUM_GROUPS_SYSTEM_ALL] = {
  "All Users",
  "Online Notify",
  "Visible List",
  "Invisible List",
  "Ignore List",
  "New Users"
};

UserManager::UserManager()
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

  LockOwnerList(LOCK_W);
  myOwners[userId.protocolId()] = o;
  UnlockOwnerList();

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
  LockOwnerList(LOCK_W);
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
  UnlockOwnerList();

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

    (*groups)[groupId] = newGroup;

    licqConf.SetFlags(INI_FxFATAL | INI_FxERROR | INI_FxWARN);
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
  User* u;
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

    UserId userId(szId, nPPID);
    u = new User(userId, string(filename));
    u->AddToContactList();
    myUsers[userId] = u;
  }
  UnlockUserList();

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

  LockUserList(LOCK_W);

  // Make sure user isn't already in the list
  UserMap::const_iterator iter = myUsers.find(uid);
  if (iter != myUsers.end())
  {
    UnlockUserList();
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

  UnlockUserList();

  // Notify plugins that user was added
  // Send this before adding user to server side as protocol code may generate updated signals
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_ADD, uid, groupId));

  // Add user to server side list
  if (permanent && addToServer)
    gProtocolManager.addUser(uid, groupId);

  // Set initial group membership, also sets server group for user
  if (groupId != 0)
    setUserInGroup(uid, GROUPS_USER, groupId, true, permanent && addToServer);

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
    setUserInGroup(userId, GROUPS_USER, groupId, true, addToServer);

  return true;
}

void UserManager::removeUser(const UserId& userId, bool removeFromServer)
{
  // Remove the user from the server side list first
  if (removeFromServer)
    gProtocolManager.removeUser(userId);

  // List should only be locked when not holding any user lock to avoid
  // deadlock, so we cannot call FetchUser here.
  LockUserList(LOCK_W);
  UserMap::iterator iter = myUsers.find(userId);
  if (iter == myUsers.end())
  {
    UnlockUserList();
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
  UnlockUserList();
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
  LockOwnerList(LOCK_W);
  OwnerMap::iterator iter = myOwners.find(ppid);
  if (iter == myOwners.end())
  {
    UnlockOwnerList();
    return;
  }

  Owner* o = iter->second;
  o->Lock(LOCK_W);
  myOwners.erase(iter);
  o->RemoveFiles();
  UserId id = o->id();
  UnlockOwnerList();
  o->Unlock();
  delete o;

  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_OWNERxLIST,
        LIST_OWNER_REMOVED, id));
}

User* UserManager::fetchUser(const UserId& userId,
    unsigned short lockType, bool addUser, bool* retWasAdded)
{
  if (retWasAdded != NULL)
    *retWasAdded = false;

  User* user = NULL;

  if (!userId.isValid())
    return NULL;

  // Check for an owner first
  LockOwnerList(LOCK_R);
  OwnerMap::iterator iter_o = myOwners.find(User::getUserProtocolId(userId));
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
    UnlockUserList();

  return user;
}

bool UserManager::userExists(const UserId& userId)
{
  bool exists = false;

  LockOwnerList(LOCK_R);
  OwnerMap::iterator iter_o = myOwners.find(User::getUserProtocolId(userId));
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

UserId UserManager::ownerUserId(unsigned long ppid)
{
  const Owner* owner = FetchOwner(ppid, LOCK_R);
  if (owner == NULL)
    return UserId();

  UserId ret = owner->id();
  DropOwner(owner);
  return ret;
}

string UserManager::OwnerId(unsigned long ppid)
{
  const Owner* owner = FetchOwner(ppid, LOCK_R);
  if (owner == NULL)
    return "";

  string ret = owner->accountId();
  DropOwner(owner);
  return ret;
}

bool UserManager::isOwner(const UserId& userId)
{
  bool exists = false;

  LockOwnerList(LOCK_R);
  OwnerMap::iterator iter_o = myOwners.find(User::getUserProtocolId(userId));
  if (iter_o != myOwners.end())
  {
    if (iter_o->second->id() == userId)
      exists = true;
  }
  UnlockOwnerList();

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

Group* UserManager::FetchGroup(int group, unsigned short lockType)
{
  GroupMap* groups = LockGroupList(LOCK_R);
  GroupMap::const_iterator iter = groups->find(group);
  Group* g = NULL;
  if (iter != groups->end())
  {
    g = iter->second;
    g->Lock(lockType);
  }
  UnlockGroupList();
  return g;
}

void UserManager::DropGroup(const Group* group)
{
  if (group != NULL)
    group->Unlock();
}

bool UserManager::groupExists(GroupType gtype, int groupId)
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

  GroupMap* groups = LockGroupList(LOCK_W);

  // Find first free group id
  int gid;
  for (gid = 1; groups->count(gid) != 0 ; ++gid)
    ;

  Group* newGroup = new Group(gid, name);
  newGroup->setServerId(LICQ_PPID, icqGroupId);
  newGroup->setSortIndex(groups->size());
  (*groups)[gid] = newGroup;

  SaveGroups();
  UnlockGroupList();

  bool icqOnline = false;
  Owner* icqOwner = FetchOwner(LICQ_PPID, LOCK_R);
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
  Group* group = FetchGroup(groupId, LOCK_R);
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
      notifyUserUpdated(pUser->id(), USER_GROUPS);
  }
  FOR_EACH_USER_END;

  SaveGroups();
  UnlockGroupList();

  // Send signal to let plugins know of the removed group
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_GROUP_REMOVED, UserId(), groupId));

  // Send signal to let plugins know that sorting indexes may have changed
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExLIST, LIST_GROUP_REORDERED));
}

void UserManager::ModifyGroupSorting(int groupId, int newIndex)
{
  Group* group = FetchGroup(groupId, LOCK_R);
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

  Group* group = FetchGroup(groupId, LOCK_W);
  if (group == NULL)
  {
    gLog.Warn(tr("%sRenaming request for invalid group %u.\n"), L_WARNxSTR, groupId);
    return false;
  }

  group->setName(name);
  unsigned short icqGroupId = group->serverId(LICQ_PPID);
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
  Group* group = gUserManager.FetchGroup(groupId, LOCK_R);
  if (group == NULL)
    return 0;

  unsigned short icqGroupId = group->serverId(LICQ_PPID);
  DropGroup(group);

  return icqGroupId;
}

int UserManager::GetGroupFromID(unsigned short icqGroupId)
{
  const GroupMap* groups = LockGroupList(LOCK_R);
  GroupMap::const_iterator iter;
  int groupId = 0;
  for (iter = groups->begin(); iter != groups->end(); ++iter)
  {
    iter->second->Lock(LOCK_R);
    if (iter->second->serverId(LICQ_PPID) == icqGroupId)
      groupId = iter->first;
    iter->second->Unlock();
  }
  UnlockGroupList();

  return groupId;
}

int UserManager::GetGroupFromName(const string& name)
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

std::string UserManager::GetGroupNameFromGroup(int groupId)
{
  const GroupMap* groups = LockGroupList(LOCK_R);
  GroupMap::const_iterator iter;
  std::string name;
  for (iter = groups->begin(); iter != groups->end(); ++iter)
  {
    iter->second->Lock(LOCK_R);
    if (iter->second->id() == groupId)
      name = iter->second->name();
    iter->second->Unlock();
  }
  UnlockGroupList();

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
  Group* group = FetchGroup(groupId, LOCK_W);
  if (group == NULL)
    return;

  group->setServerId(LICQ_PPID, icqGroupId);
  DropGroup(group);

  LockGroupList(LOCK_R);
  SaveGroups();
  UnlockGroupList();
}

unsigned short UserManager::GenerateSID()
{
  bool bCheckGroup, bDone;
  int nSID;
  unsigned short nOwnerPDINFO;

  const Owner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
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

void UserManager::DropUser(const User* u)
{
  if (u == NULL) return;
  u->Unlock();
}

Owner* UserManager::FetchOwner(unsigned long ppid, unsigned short lockType)
{
  Owner* o = NULL;

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

void UserManager::DropOwner(const Owner* owner)
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
        pUser->AddToGroup(GROUPS_USER, nInGroup);
        bDid = true;
      }
    }
  }
  FOR_EACH_PROTO_USER_END

  return bDid;
}

unsigned short UserManager::NumUsers()
{
  //LockUserList(LOCK_R);
  unsigned short n = myUsers.size();
  //UnlockUserList();
  return n;
}

unsigned short UserManager::NumOwners()
{
  unsigned short n = myOwners.size();
  return n;
}

unsigned int UserManager::NumGroups()
{
  //LockGroupList(LOCK_R);
  unsigned int n = myGroups.size();
  //UnlockGroupList();
  return n;
}

UserMap* UserManager::LockUserList(unsigned short _nLockType)
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

void UserManager::UnlockUserList()
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

GroupMap* UserManager::LockGroupList(unsigned short lockType)
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

void UserManager::UnlockGroupList()
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

OwnerMap* UserManager::LockOwnerList(unsigned short _nLockType)
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

void UserManager::UnlockOwnerList()
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

void UserManager::setUserInGroup(const UserId& userId,
    GroupType groupType, int groupId, bool inGroup, bool updateServer)
{
  // User group 0 is invalid and system group 0 is All Users
  if (groupId == 0)
    return;

  User* u = gUserManager.fetchUser(userId, LOCK_W);
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
    if (groupType == GROUPS_SYSTEM)
    {
      if (groupId == GROUP_VISIBLE_LIST)
        gProtocolManager.visibleListSet(userId, inGroup);

      else if (groupId == GROUP_INVISIBLE_LIST)
        gProtocolManager.invisibleListSet(userId, inGroup);

      else if (groupId == GROUP_IGNORE_LIST)
        gProtocolManager.ignoreListSet(userId, inGroup);
    }
    else
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
  }

  // Notify plugins
  if (gLicqDaemon != NULL)
    notifyUserUpdated(userId, (groupType == GROUPS_USER ? USER_GROUPS : USER_SETTINGS));
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
