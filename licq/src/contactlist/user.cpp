/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2011 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "user.h"

#include <arpa/inet.h> // inet_pton
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <cerrno>
#include <cstdio>
#include <ctime>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "gettext.h"
#include <licq/logging/log.h>
#include <licq/icq/icq.h> // For VersionToUse()
#include <licq/icq/codes.h>
#include <licq/inifile.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/oneventmanager.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/socket.h>
#include <licq/userevents.h>

#include "../icq/icq.h" // For gSocketManager

using boost::any_cast;
using boost::bad_any_cast;
using std::map;
using std::string;
using std::stringstream;
using std::vector;
using Licq::ICQUserPhoneBook;
using Licq::IniFile;
using Licq::OnEventData;
using Licq::PluginSignal;
using Licq::UserId;
using Licq::gDaemon;
using Licq::gLog;
using Licq::gOnEventManager;
using Licq::gPluginManager;
using Licq::gUserManager;
using namespace LicqDaemon;

const char* const User::ConfigDir = "users/";
const char* const User::HistoryDir = "history/";
const char* const User::HistoryExt = ".history";
const char* const User::HistoryOldExt = ".removed";


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

  PhoneBookVector[nEntry] = *entry;
}

void ICQUserPhoneBook::ClearEntry(unsigned long nEntry)
{
  if (nEntry >= PhoneBookVector.size())
    return;

  vector<struct PhoneBookEntry>::iterator i = PhoneBookVector.begin();
  for (;nEntry > 0; nEntry--, ++i)
    ;

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

bool ICQUserPhoneBook::SaveToDisk(IniFile& conf)
{
  char buff[40];

  conf.set("PhoneEntries", (unsigned long)PhoneBookVector.size());

  for (unsigned long i = 0 ; i < PhoneBookVector.size(); i++)
  {
    snprintf(buff, sizeof(buff), "PhoneDescription%lu", i);
    conf.set(buff, PhoneBookVector[i].description);

    snprintf(buff, sizeof(buff), "PhoneAreaCode%lu", i);
    conf.set(buff, PhoneBookVector[i].areaCode);

    snprintf(buff, sizeof(buff), "PhoneNumber%lu", i);
    conf.set(buff, PhoneBookVector[i].phoneNumber);

    snprintf(buff, sizeof(buff), "PhoneExtension%lu", i);
    conf.set(buff, PhoneBookVector[i].extension);

    snprintf(buff, sizeof(buff), "PhoneCountry%lu", i);
    conf.set(buff, PhoneBookVector[i].country);

    snprintf(buff, sizeof(buff), "PhoneActive%lu", i);
    conf.set(buff, PhoneBookVector[i].nActive);

    snprintf(buff, sizeof(buff), "PhoneType%lu", i);
    conf.set(buff, PhoneBookVector[i].nType);

    snprintf(buff, sizeof(buff), "PhoneGateway%lu", i);
    conf.set(buff, PhoneBookVector[i].gateway);

    snprintf(buff, sizeof(buff), "PhoneGatewayType%lu", i);
    conf.set(buff, PhoneBookVector[i].nGatewayType);

    snprintf(buff, sizeof(buff), "PhoneSmsAvailable%lu", i);
    conf.set(buff, PhoneBookVector[i].nSmsAvailable);

    snprintf(buff, sizeof(buff), "PhoneRemoveLeading0s%lu", i);
    conf.set(buff, PhoneBookVector[i].nRemoveLeading0s);

    snprintf(buff, sizeof(buff), "PhonePublish%lu", i);
    conf.set(buff, PhoneBookVector[i].nPublish);
  }

  return true;
}

bool ICQUserPhoneBook::LoadFromDisk(IniFile& conf)
{
  char buff[40];
  struct PhoneBookEntry entry;

  Clean();

  unsigned long nNumEntries;
  conf.get("PhoneEntries", nNumEntries);
  for (unsigned long i = 0; i < nNumEntries; i++)
  {
    snprintf(buff, sizeof(buff), "PhoneDescription%lu", i);
    conf.get(buff, entry.description, "");

    snprintf(buff, sizeof(buff), "PhoneAreaCode%lu", i);
    conf.get(buff, entry.areaCode, "");

    snprintf(buff, sizeof(buff), "PhoneNumber%lu", i);
    conf.get(buff, entry.phoneNumber, "");

    snprintf(buff, sizeof(buff), "PhoneExtension%lu", i);
    conf.get(buff, entry.extension, "");

    snprintf(buff, sizeof(buff), "PhoneCountry%lu", i);
    conf.get(buff, entry.country, "");

    snprintf(buff, sizeof(buff), "PhoneActive%lu", i);
    conf.get(buff, entry.nActive, 0);

    snprintf(buff, sizeof(buff), "PhoneType%lu", i);
    conf.get(buff, entry.nType, 0);

    snprintf(buff, sizeof(buff), "PhoneGateway%lu", i);
    conf.get(buff, entry.gateway, "");

    snprintf(buff, sizeof(buff), "PhoneGatewayType%lu", i);
    conf.get(buff, entry.nGatewayType, 1);

    snprintf(buff, sizeof(buff), "PhoneSmsAvailable%lu", i);
    conf.get(buff, entry.nSmsAvailable, 0);

    snprintf(buff, sizeof(buff), "PhoneRemoveLeading0s%lu", i);
    conf.get(buff, entry.nRemoveLeading0s, 1);

    snprintf(buff, sizeof(buff), "PhonePublish%lu", i);
    conf.get(buff, entry.nPublish, 2);

    AddEntry(&entry);
  }

  return true;
}

unsigned short Licq::User::s_nNumUserEvents = 0;
pthread_mutex_t Licq::User::mutex_nNumUserEvents = PTHREAD_MUTEX_INITIALIZER;


User::User(const UserId& id, bool temporary, bool isOwner)
  : myHistory(id.protocolId())
{
  myId = id;

  Init();
  m_bNotInList = temporary;

  // Build filename for user properties
  string filename;
  if (isOwner)
  {
    filename = "owner.";
    filename += Licq::protocolId_toString(myId.protocolId());
  }
  else
  {
    filename = ConfigDir;
    filename += myId.accountId();
    filename += ".";
    filename += Licq::protocolId_toString(myId.protocolId());
  }
  myConf.setFilename(filename);

  if (m_bNotInList)
  {
    SetDefaults();
    return;
  }


  // Make sure we have a file so load won't fail
  if (!myConf.loadFile())
  {
    myConf.setSection("user");
    if (!myConf.writeFile())
      gLog.error(tr("Error opening '%s' for writing."), myConf.filename().c_str());
    SetDefaults();
  }
  else
  {
    myConf.setSection("user");
  }

  loadUserInfo();
  LoadPhoneBookInfo();
  LoadPictureInfo();
  LoadLicqInfo();
}

void User::AddToContactList()
{
  m_bOnContactList = m_bEnableSave = true;
  m_bNotInList = false;

  // Check for old history file
  if (access(myHistory.filename().c_str(), F_OK) == -1)
  {
    string oldHistory = myHistory.filename() + HistoryOldExt;
    if (access(oldHistory.c_str(), F_OK) == 0)
    {
      if (rename(oldHistory.c_str(), myHistory.filename().c_str()) == -1)
      {
        gLog.warning(tr("Failed to rename old history file (%s): %s"),
            oldHistory.c_str(), strerror(errno));
      }
    }
  }
}

void User::loadUserInfo()
{
  // read in the fields, checking for errors each time
  myConf.setSection("user");
  myConf.get("Alias", myAlias, tr("Unknown"));
  int timezone;
  myConf.get("Timezone", timezone, TimezoneUnknown);
  m_nTimezone = timezone;
  myConf.get("Authorization", m_bAuthorization, false);

  PropertyMap::iterator i;
  for (i = myUserInfo.begin(); i != myUserInfo.end(); ++i)
    myConf.get(i->first, i->second);

  loadCategory(myInterests, "Interests");
  loadCategory(myBackgrounds, "Backgrounds");
  loadCategory(myOrganizations, "Organizations");
}

void User::LoadPhoneBookInfo()
{
  m_PhoneBook->LoadFromDisk(myConf);
}

void User::LoadPictureInfo()
{
  myConf.setSection("user");
  myConf.get("PicturePresent", m_bPicturePresent, false);
  myConf.get("BuddyIconType", myBuddyIconType, 0);
  myConf.get("BuddyIconHashType", myBuddyIconHashType, 0);
  myConf.getHex("BuddyIconHash", myBuddyIconHash, "");
  myConf.getHex("OurBuddyIconHash", myOurBuddyIconHash, "");
}

void User::LoadLicqInfo()
{
  // read in the fields, checking for errors each time
  string temp;
  unsigned nNewMessages;
  unsigned long nLast;
  unsigned nPPFieldCount;
  myConf.setSection("user");

  // Get deprecated parameter and use as default if new values aren't available
  unsigned long oldSystemGroups;
  myConf.get("Groups.System", oldSystemGroups, 0);

  myConf.get("OnVisibleList", myOnVisibleList, oldSystemGroups & 1<<1);
  myConf.get("OnInvisibleList", myOnInvisibleList, oldSystemGroups & 1<<2);
  myConf.get("OnIgnoreList", myOnIgnoreList, oldSystemGroups & 1<<3);
  myConf.get("OnlineNotify", myOnlineNotify, oldSystemGroups & 1<<0);
  myConf.get("NewUser", myNewUser, oldSystemGroups & 1<<4);
  myConf.get("Ip", temp, "0.0.0.0");
  struct in_addr in;
  m_nIp = inet_pton(AF_INET, temp.c_str(), &in);
  if (m_nIp > 0)
    m_nIp = in.s_addr;
  myConf.get("IntIp", temp, "0.0.0.0");
  m_nIntIp = inet_pton(AF_INET, temp.c_str(), &in);
  if (m_nIntIp > 0)
    m_nIntIp = in.s_addr;
  myConf.get("Port", m_nPort, 0);
  myConf.get("NewMessages", nNewMessages, 0);
  myConf.get("LastOnline", nLast, 0);
  m_nLastCounters[Licq::LAST_ONLINE] = nLast;
  myConf.get("LastSent", nLast, 0);
  m_nLastCounters[Licq::LAST_SENT_EVENT] = nLast;
  myConf.get("LastRecv", nLast, 0);
  m_nLastCounters[Licq::LAST_RECV_EVENT] = nLast;
  myConf.get("LastCheckedAR", nLast, 0);
  m_nLastCounters[Licq::LAST_CHECKED_AR] = nLast;
  myConf.get("RegisteredTime", nLast, 0);
  m_nRegisteredTime = nLast;

  unsigned autoAcceptFlags;
  myConf.get("AutoAccept", autoAcceptFlags, 0);
  myAcceptInAway                = (autoAcceptFlags & 0x0001);
  myAcceptInNotAvailable        = (autoAcceptFlags & 0x0002);
  myAcceptInOccupied            = (autoAcceptFlags & 0x0004);
  myAcceptInDoNotDisturb        = (autoAcceptFlags & 0x0008);
  myAutoAcceptChat              = (autoAcceptFlags & 0x0100);
  myAutoAcceptFile              = (autoAcceptFlags & 0x0200);
  myAutoSecure                  = (autoAcceptFlags & 0x0400);

  unsigned icqStatusToUser;
  myConf.get("StatusToUser", icqStatusToUser, 0xFFFF);
  if (icqStatusToUser == 0xFFFF)
    myStatusToUser = OfflineStatus;
  else
  {
    myStatusToUser = OnlineStatus;
    if (icqStatusToUser & 0x0002)       myStatusToUser |= DoNotDisturbStatus;
    else if (icqStatusToUser & 0x0010)  myStatusToUser |= OccupiedStatus;
    else if (icqStatusToUser & 0x0004)  myStatusToUser |= NotAvailableStatus;
    else if (icqStatusToUser & 0x0001)  myStatusToUser |= AwayStatus;
    else if (icqStatusToUser & 0x0020)  myStatusToUser |= FreeForChatStatus;
    if (icqStatusToUser & 0x0100)       myStatusToUser |= InvisibleStatus;
  }

  if (isUser()) // Only allow to keep a modified alias for user uins
    myConf.get("KeepAliasOnUpdate", m_bKeepAliasOnUpdate, false);
  else
    m_bKeepAliasOnUpdate = false;
  myConf.get("CustomAutoRsp", myCustomAutoResponse, "");
  myConf.get("SendIntIp", m_bSendIntIp, false);
  myConf.get( "UserEncoding", myEncoding, "");
  myConf.get("History", temp, "default");
  if (temp.empty())
    temp = "default";
  setHistoryFile(temp);
  myConf.get("AwaitingAuth", m_bAwaitingAuth, false);
  myConf.get("SID", m_nSID[Licq::NORMAL_SID], 0);
  myConf.get("InvisibleSID", m_nSID[Licq::INV_SID], 0);
  myConf.get("VisibleSID", m_nSID[Licq::VIS_SID], 0);
  myConf.get("GSID", m_nGSID, 0);
  myConf.get("ClientTimestamp", m_nClientTimestamp, 0);
  myConf.get("ClientInfoTimestamp", m_nClientInfoTimestamp, 0);
  myConf.get("ClientStatusTimestamp", m_nClientStatusTimestamp, 0);
  myConf.get("OurClientTimestamp", m_nOurClientTimestamp, 0);
  myConf.get("OurClientInfoTimestamp", m_nOurClientInfoTimestamp, 0);
  myConf.get("OurClientStatusTimestamp", m_nOurClientStatusTimestamp, 0);
  myConf.get("PhoneFollowMeStatus", myPhoneFollowMeStatus, CICQDaemon::IcqPluginInactive);
  myConf.get("ICQphoneStatus", myIcqPhoneStatus, CICQDaemon::IcqPluginInactive);
  myConf.get("SharedFilesStatus", mySharedFilesStatus, CICQDaemon::IcqPluginInactive);
  myConf.get("UseGPG", m_bUseGPG, false );
  myConf.get("GPGKey", myGpgKey, "");
  myConf.get("SendServer", m_bSendServer, false);
  myConf.get("PPFieldCount", nPPFieldCount, 0);
  for (unsigned i = 0; i < nPPFieldCount; i++)
  {
    char szBuf[15];
    string tempName, tempValue;
    sprintf(szBuf, "PPField%d.Name", i+1);
    myConf.get(szBuf, tempName, "");
    if (!tempName.empty())
    {
      sprintf(szBuf, "PPField%d.Value", i+1);
      myConf.get(szBuf, tempValue, "");
      if (!tempValue.empty())
        m_mPPFields[tempName] = tempValue;
    }
  }

  unsigned int userGroupCount;
  if (myConf.get("GroupCount", userGroupCount, 0))
  {
    for (unsigned int i = 1; i <= userGroupCount; ++i)
    {
      char szTemp[20];
      sprintf(szTemp, "Group%u", i);
      int groupId;
      myConf.get(szTemp, groupId, 0);
      if (groupId > 0)
        addToGroup(groupId);
    }
  }
  else
  {
    // Groupcount is missing in user config, try and read old group configuration
    unsigned int oldGroups;
    myConf.get("Groups.User", oldGroups, 0);
    for (int i = 0; i <= 31; ++i)
      if (oldGroups & (1L << i))
        addToGroup(i+1);
  }

  m_bSupportsUTF8 = false;

  if (nNewMessages > 0)
  {
    Licq::HistoryList hist;
    if (GetHistory(hist))
    {
      Licq::HistoryList::iterator it;
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


User::~User()
{
  unsigned long nId;
  while (m_vcMessages.size() > 0)
  {
    nId = m_vcMessages[m_vcMessages.size() - 1]->Id();
    delete m_vcMessages[m_vcMessages.size() - 1];
    m_vcMessages.pop_back();
    decNumUserEvents();

    gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
        PluginSignal::UserEvents, myId, nId));
  }

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

void User::RemoveFiles()
{
  remove(myConf.filename().c_str());

  // Check for old history file and back up
  struct stat buf;
  if (stat(myHistory.filename().c_str(), &buf) == 0 && buf.st_size > 0)
  {
    string oldHistory = myHistory.filename() + HistoryOldExt;
    if (rename(myHistory.filename().c_str(), oldHistory.c_str()) == -1)
    {
      gLog.warning(tr("Failed to rename history file (%s): %s"),
          oldHistory.c_str(), strerror(errno));
      remove(myHistory.filename().c_str());
    }
  }
}

void User::Init()
{
  //SetOnContactList(false);
  m_bOnContactList = m_bEnableSave = false;
  myAutoResponse = "";
  myEncoding = "";
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
  m_nTimezone = TimezoneUnknown;
  m_bAuthorization = false;
  myIsTyping = false;
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
  myBuddyIconType = 0;
  myBuddyIconHashType = 0;
  myBuddyIconHash.clear();
  myOurBuddyIconHash.clear();

  myPictureFileName = gDaemon.baseDir() + ConfigDir + myId.accountId() + ".pic";

  // GPG key
  myGpgKey = "";

  // gui plugin compat
  myStatus = OfflineStatus;
  myAutoResponse = "";
  SetSendServer(false);
  SetSendIntIp(false);
  SetShowAwayMsg(false);
  SetSequence(static_cast<unsigned short>(-1)); // set all bits 0xFFFF
  SetOfflineOnDisconnect(false);
  clearAllSocketDesc();
  m_nIp = m_nPort = m_nIntIp = 0;
  myDirectMode = true;
  m_nVersion = 0;
  m_nCookie = 0;
  m_nClientTimestamp = 0;
  m_nClientInfoTimestamp = 0;
  m_nClientStatusTimestamp = 0;
  m_nOurClientTimestamp = 0;
  m_nOurClientInfoTimestamp = 0;
  m_nOurClientStatusTimestamp = 0;
  m_bUserUpdated = false;
  myPhoneFollowMeStatus = CICQDaemon::IcqPluginInactive;
  myIcqPhoneStatus = CICQDaemon::IcqPluginInactive;
  mySharedFilesStatus = CICQDaemon::IcqPluginInactive;
  myWebPresence = false;
  myHideIp = false;
  myBirthdayFlag = false;
  myHomepageFlag = false;
  myDirectFlag = DirectAnyone;
  mySecureChannelSupport = SecureChannelUnknown;
  Touch();
  for (unsigned short i = 0; i < 4; i++)
    m_nLastCounters[i] = 0;
  m_nOnlineSince = 0;
  m_nIdleSince = 0;
  myAwaySince = 0;
  m_nRegisteredTime = 0;
  myStatusToUser = OfflineStatus;
  m_bKeepAliasOnUpdate = false;
  myAutoAcceptChat = false;
  myAutoAcceptFile = false;
  myAutoSecure = false;
  myAcceptInAway = false;
  myAcceptInNotAvailable = false;
  myAcceptInOccupied = false;
  myAcceptInDoNotDisturb = false;
  myCustomAutoResponse = "";
  m_bConnectionInProgress = false;
  m_bAwaitingAuth = false;
  m_nSID[0] = m_nSID[1] = m_nSID[2] = 0;
  m_nGSID = 0;
  myClientInfo = "";

  myMutex.setName(myId.toString());
}

void User::SetPermanent()
{
  // Set the flags and check for history file to recover
  AddToContactList();

  // Create file so save will have something to write in
  if (!myConf.loadFile())
  {
    myConf.setSection("user");
    if (!myConf.writeFile())
      gLog.error(tr("Error opening '%s' for writing."), myConf.filename().c_str());
  }


  // Save all the info now
  save(SaveAll);

  // Notify the plugins of the change
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
      PluginSignal::UserSettings, myId, 0));
}

void User::SetDefaults()
{
  setAlias(myId.accountId());
  setHistoryFile("default");
  myOnVisibleList = false;
  myOnInvisibleList = false;
  myOnIgnoreList = false;
  myGroups.clear();
  SetNewUser(true);
  SetAuthorization(false);
  myOnlineNotify = false;

  clearCustomAutoResponse();
}

string User::getUserInfoString(const string& key) const
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

unsigned int User::getUserInfoUint(const string& key) const
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

bool User::getUserInfoBool(const string& key) const
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

void User::setUserInfoString(const string& key, const string& value)
{
  PropertyMap::iterator i = myUserInfo.find(key);
  if (i == myUserInfo.end() || i->second.type() != typeid(string))
{
    return;
}

  i->second = value;
  save(SaveUserInfo);
}

void User::setUserInfoUint(const string& key, unsigned int value)
{
  PropertyMap::iterator i = myUserInfo.find(key);
  if (i == myUserInfo.end() || i->second.type() != typeid(unsigned int))
{
    return;
}

  i->second = value;
  save(SaveUserInfo);
}

void User::setUserInfoBool(const string& key, bool value)
{
  PropertyMap::iterator i = myUserInfo.find(key);
  if (i == myUserInfo.end() || i->second.type() != typeid(bool))
{
    return;
}

  i->second = value;
  save(SaveUserInfo);
}

std::string Licq::User::getFullName() const
{
  string name = getFirstName();
  string lastName = getLastName();
  if (!name.empty() && !lastName.empty())
    name += ' ';
  return name + lastName;
}

std::string Licq::User::getEmail() const
{
  string email = getUserInfoString("Email1");
  if (email.empty())
    email = getUserInfoString("Email2");
  if (email.empty())
    email = getUserInfoString("Email0");
  return email;
}

const string& Licq::User::userEncoding() const
{
  if (myEncoding.empty())
    return gUserManager.defaultUserEncoding();
  else
    return myEncoding;
}

void Licq::User::statusChanged(unsigned newStatus, time_t onlineSince)
{
  unsigned oldStatus = status();
  int arg = 0;

  if (myStatus == User::OfflineStatus)
    SetUserUpdated(false);

  if (newStatus == User::OfflineStatus)
  {
    if (isOnline())
    {
      arg = -1;
      m_nLastCounters[LAST_ONLINE] = time(NULL);
      save(SaveLicqInfo);
    }

    setIsTyping(false);
    SetUserUpdated(false);
  }
  else
  {
    if (oldStatus == User::OfflineStatus)
    {
      // We going from offline to something else
      m_nOnlineSince = (onlineSince != 0 ? onlineSince : time(NULL));
      arg = 1;
    }
  }

  myStatus = newStatus;

  // Say that we know their status for sure
  SetOfflineOnDisconnect(false);

  if (oldStatus != newStatus)
  {
    Touch();
    gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
        PluginSignal::UserStatus, myId, arg));

    if (isUser() && oldStatus == OfflineStatus)
      gOnEventManager.performOnEvent(OnEventData::OnEventOnline, this);
  }
}

/* Birthday: checks to see if the users birthday is within the next nRange
   days.  Returns -1 if not, or the number of days until their bday */
int Licq::User::Birthday(unsigned short nRange) const
{
  static const unsigned char nMonthDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  time_t t = time(NULL);
  struct tm *ts = localtime(&t);
  int nDays = -1;

  int birthDay = getUserInfoUint("BirthDay");
  int birthMonth = getUserInfoUint("BirthMonth");

  if (birthMonth == 0 || birthDay == 0)
  {
    if (birthdayFlag() && isUser()) return 0;
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

unsigned short Licq::User::Sequence(bool increment)
{
   if (increment)
      return (m_nSequence--);
   else
      return (m_nSequence);
}

void Licq::User::setAlias(const string& alias)
{
  if (alias.empty())
  {
    string firstName = getFirstName();
    if (!firstName.empty())
      myAlias = firstName;
    else
      myAlias = myId.accountId();
  }
  else
    myAlias = alias;

  // If there is a valid alias, set the server side list alias as well.
  if (!myAlias.empty())
  {
    size_t aliasLen = myAlias.size();
    Licq::TlvPtr aliasTLV(new Licq::OscarTlv(0x131, aliasLen, myAlias.c_str()));
    AddTLV(aliasTLV);
  }

  save(SaveUserInfo);
}

void User::setHistoryFile(const std::string& file)
{
  string realFile;
  if (file == "default")
  {
    realFile = gDaemon.baseDir() + HistoryDir + myId.accountId() + '.' +
        Licq::protocolId_toString(myId.protocolId()) + HistoryExt;
  }
  else if (file != "none")
  {
    // use given name
    realFile = file;
  }

  myHistory.setFile(realFile, file);
  save(SaveLicqInfo);
}

int User::GetHistory(Licq::HistoryList& history) const
{
  return myHistory.load(history);
}

void Licq::User::ClearHistory(HistoryList& h)
{
  UserHistory::clear(h);
}

const string& User::historyName() const
{
  return myHistory.description();
}

const string& User::historyFile() const
{
  return myHistory.filename();
}

void Licq::User::SetIpPort(unsigned long _nIp, unsigned short _nPort)
{
  if ((myNormalSocketDesc != -1 || myInfoSocketDesc != -1 || myStatusSocketDesc != -1) &&
      ( (Ip() != 0 && Ip() != _nIp) || (Port() != 0 && Port() != _nPort)) )
  {
    // Close our socket, but don't let socket manager try and clear
    // our socket descriptor
    if (myNormalSocketDesc != -1)
      gSocketManager.CloseSocket(myNormalSocketDesc, false);
    if (myInfoSocketDesc != -1)
      gSocketManager.CloseSocket(myInfoSocketDesc, false);
    if (myStatusSocketDesc != -1)
      gSocketManager.CloseSocket(myStatusSocketDesc, false);
    clearAllSocketDesc();
  }
  m_nIp = _nIp;
  m_nPort = _nPort;
  save(SaveLicqInfo);
}

int Licq::User::socketDesc(int channel) const
{
  switch (channel)
  {
    case TCPSocket::ChannelNormal:
      return myNormalSocketDesc;
    case TCPSocket::ChannelInfo:
      return myInfoSocketDesc;
    case TCPSocket::ChannelStatus:
      return myStatusSocketDesc;
  }
  gLog.warning(tr("Unknown channel type %u."), channel);

  return 0;
}

void Licq::User::setSocketDesc(Licq::TCPSocket* s)
{
  if (s->channel() == TCPSocket::ChannelNormal)
    myNormalSocketDesc = s->Descriptor();
  else if (s->channel() == TCPSocket::ChannelInfo)
    myInfoSocketDesc = s->Descriptor();
  else if (s->channel() == TCPSocket::ChannelStatus)
    myStatusSocketDesc = s->Descriptor();
  m_nLocalPort = s->getLocalPort();
  m_nConnectionVersion = s->Version();
  if (m_bSecure != s->Secure())
  {
    m_bSecure = s->Secure();
    if (m_bOnContactList)
      gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
          PluginSignal::UserSecurity, myId, m_bSecure ? 1 : 0));
  }

  if (m_nIntIp == 0)
    m_nIntIp = s->getRemoteIpInt();
  if (m_nPort == 0)
    m_nPort = s->getRemotePort();
  SetSendServer(false);
}

void Licq::User::clearSocketDesc(int channel)
{
  switch (channel)
  {
    case TCPSocket::ChannelNormal:
      myNormalSocketDesc = -1;
      break;
    case TCPSocket::ChannelInfo:
      myInfoSocketDesc = -1;
      break;
    case TCPSocket::ChannelStatus:
      myStatusSocketDesc = -1;
      break;
    case -1: // Used as default value to clear all socket descriptors
      myNormalSocketDesc = -1;
      myInfoSocketDesc = -1;
      myStatusSocketDesc = -1;
      break;
    default:
      gLog.info(tr("Unknown channel %u."), channel);
      return;
  }

  if (myStatusSocketDesc == -1 && myInfoSocketDesc == -1 && myNormalSocketDesc == -1)
  {
    m_nLocalPort = 0;
    m_nConnectionVersion = 0;
    m_bSecure = false;
  }

  if (m_bOnContactList)
    gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
        PluginSignal::UserSecurity, myId, 0));
}

void Licq::User::clearAllSocketDesc()
{
  clearSocketDesc(-1);
}

void Licq::User::clearNormalSocketDesc()
{
  clearSocketDesc(TCPSocket::ChannelNormal);
}

unsigned short Licq::User::ConnectionVersion() const
{
  // If we are already connected, use that version
  if (m_nConnectionVersion != 0) return m_nConnectionVersion;
  // We aren't connected, see if we know their version
  return VersionToUse(m_nVersion);
}

int Licq::User::LocalTimeGMTOffset() const
{
  return GetTimezone() * 1800;
}

int Licq::User::SystemTimeGMTOffset()
{
  // Get local time
  time_t lt = time(NULL);

  // Get GMT time
  struct tm gtm;
  gmtime_r(&lt, &gtm);
  time_t gt = mktime(&gtm);

  // Diff is seconds east of UTC
  return gt - lt;
}

char Licq::User::SystemTimezone()
{
  char nTimezone = SystemTimeGMTOffset() / 1800;
  if (nTimezone > 23)
    return 23 - nTimezone;
  return nTimezone;
}

int Licq::User::LocalTimeOffset() const
{
  return SystemTimeGMTOffset() - LocalTimeGMTOffset();
}

time_t Licq::User::LocalTime() const
{
  return time(NULL) + LocalTimeOffset();
}

unsigned Licq::User::singleStatus(unsigned status)
{
  if (status == OfflineStatus)
    return OfflineStatus;
  if (status & InvisibleStatus)
    return InvisibleStatus;
  if (status & DoNotDisturbStatus)
    return DoNotDisturbStatus;
  if (status & OccupiedStatus)
    return OccupiedStatus;
  if (status & NotAvailableStatus)
    return NotAvailableStatus;
  if (status & AwayStatus)
    return AwayStatus;
  if (status & FreeForChatStatus)
    return FreeForChatStatus;
  if (status & IdleStatus)
    return IdleStatus;
  return OnlineStatus;
}

string Licq::User::statusToString(unsigned status, bool full, bool markInvisible)
{
  string str;
  if (status == OfflineStatus)
    str = (full ? tr("Offline") : tr("Off"));
  else if (!markInvisible && status & InvisibleStatus)
    str = (full ? tr("Invisible") : tr("Inv"));
  else if (status & DoNotDisturbStatus)
    str = (full ? tr("Do Not Disturb") : tr("DND"));
  else if (status & OccupiedStatus)
    str = (full ? tr("Occupied") : tr("Occ"));
  else if (status & NotAvailableStatus)
    str = (full ? tr("Not Available") : tr("N/A"));
  else if (status & AwayStatus)
    str = (full ? tr("Away") : tr("Away"));
  else if (status & FreeForChatStatus)
    str = (full ? tr("Free For Chat") : tr("FFC"));
  else if (status & IdleStatus)
    str = (full ? tr("Idle") : tr("Idle"));
  else
    str = (full ? tr("Online") : tr("On"));

  if (markInvisible && status & InvisibleStatus)
    return '(' + str + ')';
  return str;
}

bool Licq::User::stringToStatus(const string& strStatus, unsigned& retStatus)
{
  string str2;
  bool invisible;
  if (strStatus[0] == '*')
  {
    str2 = strStatus.substr(1);
    invisible = true;
  }
  else if (strStatus[0] == '(' && strStatus[strStatus.size()-1] == ')')
  {
    str2 = strStatus.substr(1, strStatus.size()-2);
    invisible = true;
  }
  else
  {
    str2 = strStatus;
    invisible = false;
  }

  if (strcasecmp(str2.c_str(), "offline") == 0 || strcasecmp(str2.c_str(), "off") == 0)
    retStatus = OfflineStatus;
  else if (strcasecmp(str2.c_str(), "online") == 0 || strcasecmp(str2.c_str(), "on") == 0)
    retStatus = OnlineStatus;
  else if (strcasecmp(str2.c_str(), "away") == 0)
    retStatus = OnlineStatus | AwayStatus;
  else if (strcasecmp(str2.c_str(), "not available") == 0 || strcasecmp(str2.c_str(), "na") == 0 || strcasecmp(str2.c_str(), "n/a") == 0)
    retStatus = OnlineStatus | NotAvailableStatus;
  else if (strcasecmp(str2.c_str(), "do not disturb") == 0 || strcasecmp(str2.c_str(), "dnd") == 0)
    retStatus = OnlineStatus | DoNotDisturbStatus;
  else if (strcasecmp(str2.c_str(), "occupied") == 0 || strcasecmp(str2.c_str(), "occ") == 0)
    retStatus = OnlineStatus | OccupiedStatus;
  else if (strcasecmp(str2.c_str(), "free for chat") == 0 || strcasecmp(str2.c_str(), "ffc") == 0)
    retStatus = OnlineStatus | FreeForChatStatus;
  else if (strcasecmp(str2.c_str(), "idle") == 0)
    retStatus = OnlineStatus | IdleStatus;
  else if (strcasecmp(str2.c_str(), "invisible") == 0 || strcasecmp(str2.c_str(), "inv") == 0)
    retStatus = OnlineStatus | InvisibleStatus;
  else
    return false;

  if (invisible)
    retStatus |= InvisibleStatus;
  return true;
}

string Licq::User::ipToString() const
{
  string ip;

  if (Ip() > 0)                 // Default to the given ip
  {
    char buf[32];
    ip = ip_ntoa(m_nIp, buf);
  }
  else                          // Otherwise we don't know
    ip = tr("Unknown");

  if (hideIp())
    return "(" + ip + ")";
  else
    return ip;
}


string Licq::User::portToString() const
{
  if (Port() > 0)               // Default to the given port
  {
    char buf[10];
    snprintf(buf, 10, "%d", Port());
    return buf;
  }

  return "";                     // Otherwise we don't know
}

string Licq::User::internalIpToString() const
{
  int socket = myNormalSocketDesc;
  if (socket < 0)
    socket = myInfoSocketDesc;
  if (socket < 0)
    socket = myStatusSocketDesc;

  if (socket > 0)		// First check if we are connected
  {
    Licq::INetSocket *s = gSocketManager.FetchSocket(socket);
    if (s != NULL)
    {
      string ret = s->getRemoteIpString();
      gSocketManager.DropSocket(s);
      return ret;
    }
    else
      return tr("Invalid");
  }
  else
  {
    if (IntIp() > 0)		// Default to the given ip
    {
      char buf[32];
      return ip_ntoa(m_nIntIp, buf);
    }
    else			// Otherwise we don't know
      return "";
  }
}

string Licq::User::usprintf(const string& format, int quotes, bool toDos, bool allowFieldWidth) const
{
  // Our secure string for escaping stuff
  bool addQuotes = (quotes == usprintf_quoteall || (quotes == usprintf_quotepipe && format.size() > 0 && format[0] == '|'));

  string s = format;
  size_t pos = 0;
  while (pos < s.size())
  {
    switch (s[pos])
    {
      case '`':
        // Don't do any processing on data between back ticks
        pos = s.find('`', pos+1);
        if (pos != string::npos)
          ++pos;
        break;

      case '%':
      {
        bool alignLeft = false;
        int fieldWidth = 0;
        size_t pos2 = pos + 1;
        if (!allowFieldWidth)
        {
          if (isdigit(s[pos2]))
          {
            // Digit after % but we don't allow field length, just skip this and leave as is
            pos += 2;
            continue;
          }
        }
        else
        {
          if (s[pos2] == '-')
          {
            ++pos2;
            alignLeft = true;
          }
          fieldWidth = 0;
          while (isdigit(s[pos2]))
          {
            fieldWidth = fieldWidth*10 + (s[pos2] - '0');
            ++pos2;
          }
        }

        string sz;
        bool szok = true;
        char c = s[pos2];
        switch (c)
        {
          case 'i':
          {
            char buf[32];
            sz = ip_ntoa(Ip(), buf);
            break;
          }
          case 'p':
          {
            char buf[10];
            snprintf(buf, 10, "%d", Port());
            sz = buf;
            break;
          }
          case 'P':
          {
            Licq::ProtocolPluginsList plugins;
            gPluginManager.getProtocolPluginsList(plugins);
            szok = false;
            BOOST_FOREACH(Licq::ProtocolPlugin::Ptr plugin, plugins)
            {
              if (myId.protocolId() == plugin->protocolId())
              {
                sz = plugin->name();
                szok = true;
                break;
              }
            }
            break;
          }
          case 'e':
            sz = getEmail();
            break;
          case 'n':
            sz = getFullName();
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
            sz = accountId();
            break;
          case 'w':
            sz = getUserInfoString("Homepage");
            break;
          case 'h':
            sz = getUserInfoString("PhoneNumber");
            break;
          case 'c':
            sz = getUserInfoString("CellularNumber");
            break;
          case 'S':
            sz = statusString(false);
            break;
          case 's':
            sz = statusString(true);
            break;

          case 't':
          case 'T':
          {
            time_t t = time(NULL);
            char buf[128];
            strftime(buf, 128, (c == 't' ? "%b %d %r" : "%b %d %R %Z"), localtime(&t));
            sz = buf;
            break;
          }

          case 'z':
          {
            char zone = GetTimezone();
            if (zone == TimezoneUnknown)
              sz = tr("Unknown");
            else
            {
              char buf[128];
              snprintf(buf, 128, tr("GMT%c%i%c0"), (zone > 0 ? '-' : '+'), abs(zone / 2), (zone & 1 ? '3' : '0'));
              sz = buf;
            }
            break;
          }

          case 'L':
          case 'F':
          {
            char zone = GetTimezone();
            if (zone == TimezoneUnknown)
              sz = tr("Unknown");
            else
            {
              time_t t = time(NULL) - zone*30*60;
              struct tm ts;
              char buf[128];
              strftime(buf, 128, (c == 'L' ? "%R" : "%c"), gmtime_r(&t, &ts));
              sz = buf;
            }
            break;
          }

          case 'o':
            if (m_nLastCounters[LAST_ONLINE] == 0)
              sz  = tr("Never");
            else
            {
              char buf[128];
              strftime(buf, 128, "%b %d %R", localtime(&m_nLastCounters[LAST_ONLINE]));
              sz = buf;
            }
            break;
          case 'O':
            if (myStatus == OfflineStatus || m_nOnlineSince == 0)
              sz = tr("Unknown");
            else
            {
              char buf[128];
              strftime(buf, 128, "%b %d %R", localtime(&m_nOnlineSince));
              sz = buf;
            }
            break;

          case 'I':
          {
            if (m_nIdleSince == 0)
              sz = tr("Active");
            else
              sz = RelativeStrTime(m_nIdleSince);
            break;
          }

          case 'm':
          case 'M':
            if (c == 'm' || NewMessages())
            {
              char buf[128];
              snprintf(buf, 128, "%d", NewMessages());
              sz = buf;
            }
            break;
          case '%':
            sz = "%";
            break;
          default:
            gLog.warning(tr("Warning: Invalid qualifier in command: %%%c."), c);
            szok = false;
            break;
        }

        if (!szok)
        {
          // No proper replace, leave original characters and move on
          pos += 2;
          continue;
        }

        // Add width and allignment
        if (fieldWidth > 0)
        {
          int len = fieldWidth - sz.size();
          if (alignLeft)
          {
            if (len > 0)
              sz.append(len, ' ');
          }
          else
          {
            if (len < 0)
              sz.erase(len);
            else if (len > 0)
              sz.insert(0, len, ' ');
          }
        }

        // If we need to be secure, then quote the % string
        if (addQuotes)
        {
          size_t pos3 = 0;
          while ((pos3 = sz.find('\'', pos3)) != string::npos)
          {
            // Single quotes in the string needs extra handling
            sz.replace(pos3, 1, "'\\''");
            pos3 += 4;
          }
          sz = '\'' + sz + '\'';
        }

        s.replace(pos, pos2-pos+1, sz);
        pos += sz.size();
        break;
      }
      case '\n':
        // If we're converting newlines, insert \r before the \n we just found
        if (toDos)
        {
          s.insert(pos, "\r");
          ++pos;
        }
        // Check if next line starts with a pipe
        if (quotes == usprintf_quotepipe && pos+1 < s.size())
          addQuotes = (s[pos + 1] == '|');

        // Fall through to let new line character be handled normally
      default:
        // Nothing special here, check next character
        ++pos;
    }
  }
  return s;
}

string Licq::User::RelativeStrTime(time_t t)
{
  time_t diff = time(NULL) - t;

  if (diff < 30)
    return tr("now");

  // Round to nearest minute
  diff += 30;

  stringstream buf;

  int days = diff / 86400;
  if (days > 0)
    buf << days << (days == 1 ? tr(" day ") : tr(" days "));

  int hours = (diff % 86400) / 3600;
  if (hours > 0)
    buf << hours << (hours == 1 ? tr(" hour ") : tr(" hours "));

  int minutes = (diff % 3600) / 60;
  if (minutes > 0)
    buf << minutes << (minutes == 1 ? tr(" minute ") : tr(" minutes "));

  // Remove the trailing space
  string ret = buf.str();
  ret.erase(ret.size() - 1);
  return ret;
}

string UserId::normalizeId(const string& accountId, unsigned long ppid)
{
  if (accountId.empty())
    return string();

  string realId = accountId;

  // TODO Make the protocol plugin normalize the accountId
  // For AIM, account id is case insensitive and spaces should be ignored
  if (ppid == LICQ_PPID && !accountId.empty() && !isdigit(accountId[0]))
  {
    boost::erase_all(realId, " ");
    boost::to_lower(realId);
  }

  return realId;
}

void User::saveUserInfo()
{
  myConf.set("Alias", myAlias);
  myConf.set("KeepAliasOnUpdate", m_bKeepAliasOnUpdate);
  myConf.set("Timezone", m_nTimezone);
  myConf.set("Authorization", m_bAuthorization);

  PropertyMap::const_iterator i;
  for (i = myUserInfo.begin(); i != myUserInfo.end(); ++i)
    myConf.set(i->first, i->second);

  saveCategory(myInterests, "Interests");
  saveCategory(myBackgrounds, "Backgrounds");
  saveCategory(myOrganizations, "Organizations");
}

void User::saveCategory(const Licq::UserCategoryMap& category, const string& key)
{
  myConf.set(key + 'N', category.size());

  Licq::UserCategoryMap::const_iterator i;
  unsigned int count = 0;
  for (i = category.begin(); i != category.end(); ++i)
  {
    char n[10];
    snprintf(n, sizeof(n), "%04X", count);
    myConf.set(key + "Cat" + n, i->first);
    myConf.set(key + "Desc" + n, i->second);
    ++count;
  }
}

void User::loadCategory(Licq::UserCategoryMap& category, const string& key)
{
  category.clear();
  unsigned int count;
  myConf.get(key + 'N', count, 0);

  if (count > Licq::MAX_CATEGORIES)
  {
    gLog.warning(tr("Trying to load more categories than the max limit. Truncating."));
    count = Licq::MAX_CATEGORIES;
  }

  for (unsigned int i = 0; i < count; ++i)
  {
    char n[10];
    snprintf(n, sizeof(n), "%04X", i);

    unsigned int cat;
    if (!myConf.get(key + "Cat" + n, cat))
      continue;

    string descr;
    if (!myConf.get(key + "Desc" + n, descr))
      continue;

    category[cat] = descr;
  }
}

void User::savePictureInfo()
{
  myConf.set("PicturePresent", m_bPicturePresent);
  myConf.set("BuddyIconType", myBuddyIconType);
  myConf.set("BuddyIconHashType", myBuddyIconHashType);
  myConf.setHex("BuddyIconHash", myBuddyIconHash);
  myConf.setHex("OurBuddyIconHash", myOurBuddyIconHash);
}

void User::saveLicqInfo()
{
   char buf[64];
  myConf.set("History", historyName());
  myConf.set("OnVisibleList", myOnVisibleList);
  myConf.set("OnInvisibleList", myOnInvisibleList);
  myConf.set("OnIgnoreList", myOnIgnoreList);
  myConf.set("OnlineNotify", myOnlineNotify);
  myConf.set("NewUser", myNewUser);
  myConf.set("Ip", Licq::ip_ntoa(m_nIp, buf));
  myConf.set("IntIp", Licq::ip_ntoa(m_nIntIp, buf));
  myConf.set("Port", Port());
  myConf.set("NewMessages", NewMessages());
  myConf.set("LastOnline", (unsigned long)LastOnline());
  myConf.set("LastSent", (unsigned long)LastSentEvent());
  myConf.set("LastRecv", (unsigned long)LastReceivedEvent());
  myConf.set("LastCheckedAR", (unsigned long)LastCheckedAutoResponse());
  myConf.set("RegisteredTime", (unsigned long)RegisteredTime());

  unsigned autoAcceptFlags = 0;
  if (myAcceptInAway)           autoAcceptFlags |= 0x0001;
  if (myAcceptInNotAvailable)   autoAcceptFlags |= 0x0002;
  if (myAcceptInOccupied)       autoAcceptFlags |= 0x0004;
  if (myAcceptInDoNotDisturb)   autoAcceptFlags |= 0x0008;
  if (myAutoAcceptChat)         autoAcceptFlags |= 0x0100;
  if (myAutoAcceptFile)         autoAcceptFlags |= 0x0200;
  if (myAutoSecure)             autoAcceptFlags |= 0x0400;
  myConf.set("AutoAccept", autoAcceptFlags);

  unsigned icqStatusToUser = 0xFFFF;
  if (myStatusToUser & OnlineStatus)
  {
    if (myStatusToUser & DoNotDisturbStatus)      icqStatusToUser = 0x0013;
    else if (myStatusToUser & OccupiedStatus)     icqStatusToUser = 0x0011;
    else if (myStatusToUser & NotAvailableStatus) icqStatusToUser = 0x0005;
    else if (myStatusToUser & AwayStatus)         icqStatusToUser = 0x0001;
    else if (myStatusToUser & FreeForChatStatus)  icqStatusToUser = 0x0020;
    else                                          icqStatusToUser = 0;
    if (myStatusToUser & InvisibleStatus)         icqStatusToUser |= 0x0100;
  }
  myConf.set("StatusToUser", icqStatusToUser);

  myConf.set("CustomAutoRsp", customAutoResponse());
  myConf.set("SendIntIp", m_bSendIntIp);
  myConf.set("UserEncoding", myEncoding);
  myConf.set("AwaitingAuth", m_bAwaitingAuth);
  myConf.set("SID", m_nSID[Licq::NORMAL_SID]);
  myConf.set("InvisibleSID", m_nSID[Licq::INV_SID]);
  myConf.set("VisibleSID", m_nSID[Licq::VIS_SID]);
  myConf.set("GSID", m_nGSID);
  myConf.set("ClientTimestamp", m_nClientTimestamp);
  myConf.set("ClientInfoTimestamp", m_nClientInfoTimestamp);
  myConf.set("ClientStatusTimestamp", m_nClientStatusTimestamp);
  myConf.set("OurClientTimestamp", m_nOurClientTimestamp);
  myConf.set("OurClientInfoTimestamp", m_nOurClientInfoTimestamp);
  myConf.set("OurClientStatusTimestamp", m_nOurClientStatusTimestamp);
  myConf.set("PhoneFollowMeStatus", myPhoneFollowMeStatus);
  myConf.set("ICQphoneStatus", myIcqPhoneStatus);
  myConf.set("SharedFilesStatus", mySharedFilesStatus);
  myConf.set("UseGPG", m_bUseGPG );
  myConf.set("GPGKey", myGpgKey );
  myConf.set("SendServer", m_bSendServer);
  myConf.set("PPFieldCount", (unsigned short)m_mPPFields.size());

   map<string,string>::iterator iter;
   int i = 0;
   for (iter = m_mPPFields.begin(); iter != m_mPPFields.end(); ++iter)
   {
     char szBuf[25];
     sprintf(szBuf, "PPField%d.Name", ++i);
      myConf.set(szBuf, iter->first);
     sprintf(szBuf, "PPField%d.Value", i);
      myConf.set(szBuf, iter->second);
   }

  myConf.set("GroupCount", static_cast<unsigned int>(myGroups.size()));
  i = 1;
  for (Licq::UserGroupList::iterator g = myGroups.begin(); g != myGroups.end(); ++g)
  {
    sprintf(buf, "Group%u", i);
    myConf.set(buf, *g);
    ++i;
  }
}

void User::saveNewMessagesInfo()
{
  myConf.set("NewMessages", NewMessages());
}

void User::saveOwnerInfo()
{
  // Empty here, overloaded by owner object
}

void User::save(unsigned group)
{
  if (!EnableSave())
    return;

  if (!myConf.loadFile())
  {
    gLog.error(tr("Error opening '%s' for reading. See log for details."),
        myConf.filename().c_str());
    return;
  }

  myConf.setSection("user");

  if (group & SaveUserInfo)
    saveUserInfo();
  if (group & SaveLicqInfo)
    saveLicqInfo();
  if (group & SaveOwnerInfo)
    saveOwnerInfo();
  if (group & SaveNewMessagesInfo)
    saveNewMessagesInfo();
  if (group & SavePictureInfo)
    savePictureInfo();
  if (group & SavePhoneBook)
    m_PhoneBook->SaveToDisk(myConf);

  if (!myConf.writeFile())
    gLog.error(tr("Error opening '%s' for writing. See log for details."),
        myConf.filename().c_str());
}

void Licq::User::EventPush(Licq::UserEvent *e)
{
  m_vcMessages.push_back(e);
  incNumUserEvents();
  save(SaveNewMessagesInfo);
  Touch();
  SetLastReceivedEvent();

  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
      PluginSignal::UserEvents, myId, e->Id(), e->ConvoId()));
}

void User::writeToHistory(const string& text)
{
  myHistory.append(text);
}

void Licq::User::CancelEvent(unsigned short index)
{
  if (index < NewMessages())
    return;
  m_vcMessages[index]->Cancel();
}

const Licq::UserEvent* Licq::User::EventPeek(unsigned short index) const
{
  if (index >= NewMessages()) return (NULL);
  return (m_vcMessages[index]);
}

const Licq::UserEvent* Licq::User::EventPeekId(int id) const
{
  if (m_vcMessages.empty()) return NULL;
  Licq::UserEvent* e = NULL;
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

const Licq::UserEvent* Licq::User::EventPeekLast() const
{
  if (m_vcMessages.empty()) return (NULL);
  return (m_vcMessages[m_vcMessages.size() - 1]);
}

const Licq::UserEvent* Licq::User::EventPeekFirst() const
{
  if (m_vcMessages.empty()) return (NULL);
  return (m_vcMessages[0]);
}

Licq::UserEvent *Licq::User::EventPop()
{
  if (m_vcMessages.empty()) return NULL;
  Licq::UserEvent* e = m_vcMessages[0];
  for (unsigned short i = 0; i < m_vcMessages.size() - 1; i++)
    m_vcMessages[i] = m_vcMessages[i + 1];
  m_vcMessages.pop_back();
  decNumUserEvents();
  save(SaveNewMessagesInfo);

  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
      PluginSignal::UserEvents, myId, e->Id()));

  return e;
}

void Licq::User::EventClear(unsigned short index)
{
  if (index >= m_vcMessages.size()) return;

  unsigned long id = m_vcMessages[index]->Id();

  delete m_vcMessages[index];
  for (unsigned short i = index; i < m_vcMessages.size() - 1; i++)
    m_vcMessages[i] = m_vcMessages[i + 1];
  m_vcMessages.pop_back();
  decNumUserEvents();
  save(SaveNewMessagesInfo);

  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
      PluginSignal::UserEvents, myId, -id));
}

void Licq::User::EventClearId(int id)
{
  UserEventList::iterator iter;
  for (iter = m_vcMessages.begin(); iter != m_vcMessages.end(); ++iter)
  {
    if ((*iter)->Id() == id)
    {
      delete *iter;
      m_vcMessages.erase(iter);
      decNumUserEvents();
      save(SaveNewMessagesInfo);

      gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
          PluginSignal::UserEvents, myId, -id));
      break;
    }
  }
}

bool Licq::User::isInGroup(int groupId) const
{
  return myGroups.count(groupId) > 0;
}

void Licq::User::setInGroup(int groupId, bool member)
{
  if (member)
    addToGroup(groupId);
  else
    removeFromGroup(groupId);
}

void Licq::User::addToGroup(int groupId)
{
  if (groupId <= 0)
    return;

  myGroups.insert(groupId);
  save(SaveLicqInfo);
}

bool Licq::User::removeFromGroup(int groupId)
{
  bool inGroup = myGroups.erase(groupId);
  save(SaveLicqInfo);
  return inGroup;
}

unsigned short Licq::User::getNumUserEvents()
{
  pthread_mutex_lock(&mutex_nNumUserEvents);
  unsigned short n = s_nNumUserEvents;
  pthread_mutex_unlock(&mutex_nNumUserEvents);
  return n;
}

void Licq::User::incNumUserEvents()
{
  pthread_mutex_lock(&mutex_nNumUserEvents);
  s_nNumUserEvents++;
  pthread_mutex_unlock(&mutex_nNumUserEvents);
}

void Licq::User::decNumUserEvents()
{
  pthread_mutex_lock(&mutex_nNumUserEvents);
  s_nNumUserEvents--;
  pthread_mutex_unlock(&mutex_nNumUserEvents);
}

bool Licq::User::SetPPField(const string &_sName, const string &_sValue)
{
  m_mPPFields[_sName] = _sValue;
  return true;
}

string Licq::User::GetPPField(const string &_sName)
{
  map<string,string>::iterator iter = m_mPPFields.find(_sName);
  if (iter != m_mPPFields.end())
    return iter->second;

  return string("");
}

void Licq::User::AddTLV(Licq::TlvPtr tlv)
{
  myTLVs[tlv->getType()] = tlv;
}

void Licq::User::RemoveTLV(unsigned long type)
{
  myTLVs.erase(type);
}

void Licq::User::SetTLVList(Licq::TlvList& tlvs)
{
  myTLVs.clear();

  for (Licq::TlvList::iterator it = tlvs.begin(); it != tlvs.end(); it++)
    myTLVs[it->first] = it->second;
}

