#include "usermanager.h"

#include <boost/foreach.hpp>
#include <cstdio> // sprintf

#include "licq_log.h"
#include <licq/daemon.h>
#include <licq/icqdefines.h>
#include <licq/inifile.h>
#include <licq/pluginsignal.h>
#include <licq/protocolsignal.h>

#include "../gettext.h"
#include "../icq/icq.h"
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
using Licq::PluginSignal;
using Licq::UserListGuard;
using Licq::UserId;
using Licq::UserGroupList;
using Licq::UserReadGuard;
using Licq::UserWriteGuard;
using Licq::gDaemon;
using namespace LicqDaemon;

// Declare global UserManager (internal for daemon)
LicqDaemon::UserManager LicqDaemon::gUserManager;

// Initialize global Licq::UserManager to refer to the internal UserManager
Licq::UserManager& Licq::gUserManager(LicqDaemon::gUserManager);


UserManager::UserManager()
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
}

void UserManager::addOwner(const UserId& userId)
{
  Owner* o = new Owner(userId);

  myOwnerListMutex.lockWrite();
  myOwners[userId.protocolId()] = o;
  myOwnerListMutex.unlockWrite();

  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListOwnerAdded, userId));
}

/*---------------------------------------------------------------------------
 * UserManager::Load
 *-------------------------------------------------------------------------*/
bool UserManager::Load()
{
  gLog.Info(tr("%sUser configuration.\n"), L_INITxSTR);

  // Load the group info from licq.conf
  Licq::IniFile licqConf("licq.conf");
  licqConf.loadFile();

  unsigned nOwners;
  licqConf.setSection("owners");
  licqConf.get("NumOfOwners", nOwners, 0);

  m_bAllowSave = false;

  //TODO Check for loaded plugins before the owner, so we can see
  //which owner(s) to load
  myOwnerListMutex.lockWrite();
  for (unsigned short i = 1; i <= nOwners; i++)
  {
    char sOwnerIDKey[20], sOwnerPPIDKey[20];
    string accountId, ppidStr;
    sprintf(sOwnerIDKey, "Owner%d.Id", i);
    licqConf.get(sOwnerIDKey, accountId);
    sprintf(sOwnerPPIDKey, "Owner%d.PPID", i);
    licqConf.get(sOwnerPPIDKey, ppidStr);
    unsigned long protocolId = (ppidStr[0] << 24) | (ppidStr[1] << 16) | (ppidStr[2] << 8) | (ppidStr[3]);

    UserId ownerId(accountId, protocolId);
    Owner* o = new Owner(ownerId);

    myOwners[protocolId] = o;
  }
  myOwnerListMutex.unlockWrite();

  unsigned int nGroups;
  licqConf.setSection("groups");
  licqConf.get("NumOfGroups", nGroups);

  myGroupListMutex.lockWrite();
  m_bAllowSave = false;
  for (unsigned int i = 1; i <= nGroups; i++)
  {
    char key[40];
    string groupName;
    int groupId, sortIndex;
    unsigned icqGroupId = 0;
    sprintf(key, "Group%d.name", i);
    licqConf.get(key, groupName);

    sprintf(key, "Group%d.id", i);
    licqConf.get(key, groupId, 0);

    sprintf(key, "Group%d.Sorting", i);
    bool newConfig = licqConf.get(key, sortIndex, i-1);

    sprintf(key, "Group%d.IcqServerId", i);
    licqConf.get(key, icqGroupId, 0);

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
      licqConf.get(serverIdKey, serverId, 0);
      newGroup->setServerId(protocolId, serverId);
    }

    // ServerId per protocol didn't exist in 1.3.x and older.
    // This will preserve ICQ group ids when reading old config.
    if (serverIdKeys.size() == 0 && icqGroupId != 0)
      newGroup->setServerId(LICQ_PPID, icqGroupId);

    myGroups[groupId] = newGroup;
  }
  m_bAllowSave = true;
  myGroupListMutex.unlockWrite();

  licqConf.setSection("network");
  licqConf.get("DefaultUserEncoding", myDefaultEncoding, "");

  // Load users from users.conf
  Licq::IniFile usersConf("users.conf");
  usersConf.loadFile();

  unsigned nUsers;
  usersConf.setSection("users");
  usersConf.get("NumOfUsers", nUsers);
  gLog.Info(tr("%sLoading %d users.\n"), L_INITxSTR, nUsers);

  // TODO: We need to only load users of protocol plugins that are loaded!
  myUserListMutex.lockWrite();
  for (unsigned short i = 1; i<= nUsers; i++)
  {
    string userFile;
    char sUserKey[20];
    sprintf(sUserKey, "User%d", i);
    if (!usersConf.get(sUserKey, userFile, ""))
    {
      gLog.Warn(tr("%sSkipping user %i, empty key.\n"), L_WARNxSTR, i);
      continue;
    }
    string filename = User::ConfigDir;
    filename += '/';
    filename += userFile;
    size_t sz = userFile.rfind('.');
    if (sz == string::npos)
    {
      gLog.Error(tr("%sFatal error reading protocol information for User%d with ID '%s'.\n"
          "%sPlease check \"%s/users.conf\".\n"), L_ERRORxSTR, i,
          userFile.c_str(), L_BLANKxSTR, gDaemon.baseDir().c_str());
      exit(1);
    }
    string accountId = userFile.substr(0, sz);
    unsigned long protocolId = (userFile[sz+1] << 24) | (userFile[sz+2] << 16) | (userFile[sz+3] << 8) | userFile[sz+4];

    UserId userId(accountId, protocolId);
    User* u = new User(userId, filename);
    u->AddToContactList();
    myUsers[userId] = u;
  }
  myUserListMutex.unlockWrite();

  return true;
}

void UserManager::saveUserList() const
{
  Licq::IniFile usersConf("users.conf");
  usersConf.loadFile();
  usersConf.setSection("users");

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

    char key[20];
    sprintf(key, "User%i", count);

    char ps[5];
    Licq::protocolId_toStr(ps, ppid);
    usersConf.set(key, accountId + "." + ps);
  }
  usersConf.set("NumOfUsers", count);
  usersConf.writeFile();
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
  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListUserAdded, uid, groupId));

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
  // deadlock, so we cannot call fetchUser here.
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
  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListUserRemoved, userId));
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

  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListOwnerRemoved, id));
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
      gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
          PluginSignal::ListUserAdded, userId));

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
  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser, subSignal, userId));
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

  if (icqGroupId == 0 && icqOnline)
    gIcqProtocol.icqAddGroup(name.c_str());
  else
    gLog.Info(tr("%sAdded group %s (%u) to list from server.\n"),
        L_SRVxSTR, name.c_str(), icqGroupId);

  // Send signal to let plugins know of the new group
  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupAdded, UserId(), gid));

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
  gIcqProtocol.icqRemoveGroup(name.c_str());

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
      notifyUserUpdated(pUser->id(), PluginSignal::UserGroups);
  }
  FOR_EACH_USER_END;

  SaveGroups();
  myGroupListMutex.unlockWrite();

  // Send signal to let plugins know of the removed group
  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupRemoved, UserId(), groupId));

  // Send signal to let plugins know that sorting indexes may have changed
  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupsReordered));
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
  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupsReordered));
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

  // If we rename a group on logon, don't send the rename packet
  if (sendUpdate)
    gIcqProtocol.icqRenameGroup(name.c_str(), icqGroupId);

  // Send signal to let plugins know the group has changed
  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalList,
      PluginSignal::ListGroupChanged, UserId(), groupId));

  return true;
}

void UserManager::SaveGroups()
{
  if (!m_bAllowSave) return;

  // Load the group info from licq.conf
  Licq::IniFile licqConf("licq.conf");
  licqConf.loadFile();

  licqConf.setSection("groups");
  GroupMap::size_type count = myGroups.size();
  licqConf.set("NumOfGroups", static_cast<unsigned int>(count));

  int i = 1;
  for (GroupMap::iterator group = myGroups.begin(); group != myGroups.end(); ++group)
  {
    group->second->Lock(LOCK_R);
    group->second->save(licqConf, i);
    group->second->Unlock();
    ++i;
  }

  licqConf.writeFile();
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

void UserManager::dropUser(const Licq::User* u)
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

  int gsid;

  {
    Licq::UserWriteGuard u(userId);
    if (!u.isLocked())
      return;

    gsid = u->GetGSID();

    // Don't remove user from local group if member of the same server group
    if (!inGroup && u->GetSID() != 0 && GetGroupFromID(gsid) == groupId)
      return;

    // Update user object
    u->setInGroup(groupId, inGroup);
  }

  // Notify server
  if (updateServer)
  {
    if (userId.protocolId() == LICQ_PPID)
    {
      if (inGroup) // Server group can only be changed, not removed
        gIcqProtocol.icqChangeGroup(userId.accountId().c_str(), userId.protocolId(), groupId, gsid,
            ICQ_ROSTxNORMAL, ICQ_ROSTxNORMAL);
    }
    else
      gDaemon.PushProtoSignal(new Licq::ProtoChangeUserGroupsSignal(userId), userId.protocolId());
  }

  // Notify plugins
  notifyUserUpdated(userId, PluginSignal::UserGroups);
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

const string& UserManager::defaultUserEncoding()
{
  return myDefaultEncoding;
}

void UserManager::setDefaultUserEncoding(const string& defaultEncoding)
{
  myDefaultEncoding = defaultEncoding;
}


UserReadGuard::UserReadGuard(const UserId& userId, bool addUser, bool* retWasAdded)
  : ReadMutexGuard<User>(LicqDaemon::gUserManager.fetchUser(userId, LOCK_R, addUser, retWasAdded), true)
{
  // Empty
}

UserWriteGuard::UserWriteGuard(const UserId& userId, bool addUser, bool* retWasAdded)
  : WriteMutexGuard<User>(LicqDaemon::gUserManager.fetchUser(userId, LOCK_W, addUser, retWasAdded), true)
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
