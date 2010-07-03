#include "user.h"

#include <arpa/inet.h> // inet_pton
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "gettext.h"
#include "licq_log.h"
#include <licq/icq.h> // For VersionToUse()
#include <licq/icqcodes.h>
#include <licq/icqdefines.h>
#include <licq/inifile.h>
#include "licq/contactlist/usermanager.h"
#include <licq/daemon.h>
#include "licq/pluginmanager.h"
#include "licq/pluginsignal.h"
#include <licq/socket.h>
#include <licq/userevents.h>

#include "../icq/icq.h" // For gSocketManager

using boost::any_cast;
using boost::bad_any_cast;
using std::map;
using std::string;
using std::vector;
using Licq::ICQUserPhoneBook;
using Licq::IniFile;
using Licq::PluginSignal;
using Licq::SecureChannelSupport_et;
using Licq::UserId;
using Licq::gDaemon;
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

  if (!conf.loadFile())
  {
    gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
        L_ERRORxSTR, conf.filename().c_str(), L_BLANKxSTR);
    return false;
  }

  conf.setSection("user");

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

  if (!conf.writeFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
        L_ERRORxSTR, conf.filename().c_str(), L_BLANKxSTR);
    return false;
  }

  return true;
}

bool ICQUserPhoneBook::LoadFromDisk(IniFile& conf)
{
  char buff[40];
  struct PhoneBookEntry entry;

  Clean();
  conf.setSection("user");

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

User::User(const UserId& id, const string& filename)
{
  myId = id;

  Init();
  myConf.setFilename(filename);
  if (!LoadInfo())
  {
    gLog.Error("%sUnable to load user info from '%s'.\n%sUsing default values.\n",
        L_ERRORxSTR, filename.c_str(), L_BLANKxSTR);
    SetDefaults();
  }
}

User::User(const UserId& id, bool temporary)
{
  myId = id;

  Init();
  SetDefaults();
  m_bNotInList = temporary;
  if (!m_bNotInList)
  {
    char p[5];
    Licq::protocolId_toStr(p, myId.protocolId());

    string filename = ConfigDir;
    filename += myId.accountId();
    filename += ".";
    filename += p;
    myConf.setFilename(filename);

    // Create file so load won't fail later
    if (!myConf.loadFile())
    {
      myConf.setSection("user");
      if (!myConf.writeFile())
        gLog.Error("%sError opening '%s' for writing.\n", L_ERRORxSTR, myConf.filename().c_str());
    }
  }
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
        gLog.Warn(tr("%sFailed to rename old history file (%s):\n%s%s\n"), L_WARNxSTR,
            oldHistory.c_str(), L_BLANKxSTR, strerror(errno));
      }
    }
  }
}

bool User::LoadInfo()
{
  if (!myConf.loadFile())
    return false;
  myConf.setSection("user");

  loadUserInfo();
  LoadPhoneBookInfo();
  LoadPictureInfo();
  LoadLicqInfo();

  return true;
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

  loadCategory(myInterests, myConf, "Interests");
  loadCategory(myBackgrounds, myConf, "Backgrounds");
  loadCategory(myOrganizations, myConf, "Organizations");
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
  myConf.get("BuddyIconHash", myBuddyIconHash, "");
  myConf.get("OurBuddyIconHash", myOurBuddyIconHash, "");
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
  myConf.get("AutoAccept", myAutoAccept, 0);
  unsigned icqStatusToUser;
  myConf.get("StatusToUser", icqStatusToUser, ICQ_STATUS_OFFLINE);
  m_nStatusToUser = icqStatusToUser;
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
  myConf.get("PhoneFollowMeStatus", m_nPhoneFollowMeStatus, ICQ_PLUGIN_STATUSxINACTIVE);
  myConf.get("ICQphoneStatus", m_nICQphoneStatus, ICQ_PLUGIN_STATUSxINACTIVE);
  myConf.get("SharedFilesStatus", m_nSharedFilesStatus, ICQ_PLUGIN_STATUSxINACTIVE);
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

    gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
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
      gLog.Warn(tr("%sFailed to rename history file (%s):\n%s%s\n"), L_WARNxSTR,
          oldHistory.c_str(), L_BLANKxSTR, strerror(errno));
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
  myBuddyIconHash = "";
  myOurBuddyIconHash = "";

  myPictureFileName = gDaemon.baseDir() + ConfigDir + myId.accountId() + ".pic";

  // GPG key
  myGpgKey = "";

  // gui plugin compat
  SetStatus(ICQ_STATUS_OFFLINE);
  myAutoResponse = "";
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
  myAutoAccept = 0;
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

  // Create the user file
  char p[5];
  Licq::protocolId_toStr(p, myId.protocolId());

  string filename = ConfigDir;
  filename += myId.accountId();
  filename += ".";
  filename += p;
  myConf.setFilename(filename);

  // Create file so save will have something to write in
  if (!myConf.loadFile())
  {
    myConf.setSection("user");
    if (!myConf.writeFile())
      gLog.Error("%sError opening '%s' for writing.\n", L_ERRORxSTR, myConf.filename().c_str());
  }


  // Save all the info now
  saveAll();

  // Notify the plugins of the change
  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
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
  saveUserInfo();
}

void User::setUserInfoUint(const string& key, unsigned int value)
{
  PropertyMap::iterator i = myUserInfo.find(key);
  if (i == myUserInfo.end() || i->second.type() != typeid(unsigned int))
{
    return;
}

  i->second = value;
  saveUserInfo();
}

void User::setUserInfoBool(const string& key, bool value)
{
  PropertyMap::iterator i = myUserInfo.find(key);
  if (i == myUserInfo.end() || i->second.type() != typeid(bool))
{
    return;
}

  i->second = value;
  saveUserInfo();
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

void Licq::User::statusChanged(unsigned newStatus, unsigned long s)
{
  unsigned oldStatus = status();
  int arg = 0;

  if (myStatus == User::OfflineStatus)
    SetUserUpdated(false);

  if (newStatus == User::OfflineStatus)
  {
    if (isOnline())
      arg = -1;
    SetStatusOffline();
  }
  else
  {
    if (!isOnline())
      arg = 1;
    if (s != 0)
      SetStatus(s);
    else
      setStatus(newStatus);

    //This is the v6 way of telling us phone follow me status
    if (s & ICQ_STATUS_FxPFM)
    {
      if (s & ICQ_STATUS_FxPFMxAVAILABLE)
        SetPhoneFollowMeStatus(ICQ_PLUGIN_STATUSxACTIVE);
      else
        SetPhoneFollowMeStatus(ICQ_PLUGIN_STATUSxBUSY);
    }
    else if (Version() < 7)
      SetPhoneFollowMeStatus(ICQ_PLUGIN_STATUSxINACTIVE);
  }

  // Say that we know their status for sure
  SetOfflineOnDisconnect(false);

  if (oldStatus != newStatus)
  {
    Touch();
    gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
        PluginSignal::UserStatus, myId, arg));
  }
}

void Licq::User::setStatus(unsigned status)
{
  myStatus = status;

  // Build ICQ status equivalent for compatibility with old code
  m_nStatus &= (ICQ_STATUS_FxFLAGS & ~ICQ_STATUS_FxPFMxAVAILABLE);
  if (status == OfflineStatus)
    m_nStatus = ICQ_STATUS_OFFLINE;
  if (status & InvisibleStatus)
    m_nStatus |= ICQ_STATUS_FxPRIVATE;
  if (status & AwayStatus)
    m_nStatus |= ICQ_STATUS_AWAY;
  if (status & NotAvailableStatus)
    m_nStatus |= ICQ_STATUS_NA | ICQ_STATUS_AWAY;
  if (status & OccupiedStatus)
    m_nStatus |= ICQ_STATUS_OCCUPIED | ICQ_STATUS_AWAY;
  if (status & DoNotDisturbStatus)
    m_nStatus |= ICQ_STATUS_DND | ICQ_STATUS_AWAY | ICQ_STATUS_OCCUPIED;
  if (status & FreeForChatStatus)
    m_nStatus |= ICQ_STATUS_FREEFORCHAT;
}

void Licq::User::SetStatus(unsigned long n)
{
  m_nStatus = n;

  // Build status from ICQ flags
  myStatus = statusFromIcqStatus(m_nStatus & 0xFFFF);
  if (myStatus != OfflineStatus && m_nIdleSince != 0)
    myStatus |= IdleStatus;
}

unsigned short Licq::User::icqStatusFromStatus(unsigned status)
{
  if (status == OfflineStatus)
    return ICQ_STATUS_OFFLINE;
  if (status & DoNotDisturbStatus)
    return ICQ_STATUS_DND | ICQ_STATUS_AWAY | ICQ_STATUS_OCCUPIED;
  if (status & OccupiedStatus)
    return ICQ_STATUS_OCCUPIED | ICQ_STATUS_AWAY;
  if (status & NotAvailableStatus)
    return ICQ_STATUS_NA | ICQ_STATUS_AWAY;
  if (status & AwayStatus)
    return ICQ_STATUS_AWAY;
  if (status & FreeForChatStatus)
    return ICQ_STATUS_FREEFORCHAT;
  if (status & InvisibleStatus)
   return ICQ_STATUS_FxPRIVATE;
  return ICQ_STATUS_ONLINE;
}

unsigned Licq::User::statusFromIcqStatus(unsigned short icqStatus)
{
  // Build status from ICQ flags
  if (icqStatus == ICQ_STATUS_OFFLINE)
    return OfflineStatus;

  unsigned status = OnlineStatus;
  if (icqStatus & ICQ_STATUS_FxPRIVATE)
    status |= InvisibleStatus;
  if (icqStatus & ICQ_STATUS_DND)
    status |= DoNotDisturbStatus;
  else if (icqStatus & ICQ_STATUS_OCCUPIED)
    status |= OccupiedStatus;
  else if (icqStatus & ICQ_STATUS_NA)
    status |= NotAvailableStatus;
  else if (icqStatus & ICQ_STATUS_AWAY)
    status |= AwayStatus;
  if (icqStatus & ICQ_STATUS_FREEFORCHAT)
    status |= FreeForChatStatus;

  return status;
}

unsigned short Licq::User::Status() const
// guarantees to return a unique status that switch can be run on
{
   if (!isOnline()) return ICQ_STATUS_OFFLINE;
   else if (m_nStatus & ICQ_STATUS_DND) return ICQ_STATUS_DND;
   else if (m_nStatus & ICQ_STATUS_OCCUPIED) return ICQ_STATUS_OCCUPIED;
   else if (m_nStatus & ICQ_STATUS_NA) return ICQ_STATUS_NA;
   else if (m_nStatus & ICQ_STATUS_AWAY) return ICQ_STATUS_AWAY;
   else if (m_nStatus & ICQ_STATUS_FREEFORCHAT) return ICQ_STATUS_FREEFORCHAT;
   else if ((m_nStatus & 0xFF) == 0x00) return ICQ_STATUS_ONLINE;
   else return (ICQ_STATUS_OFFLINE - 1);
}

bool Licq::User::StatusWebPresence() const
{
  return m_nStatus & ICQ_STATUS_FxWEBxPRESENCE;
}

bool Licq::User::StatusHideIp() const
{
  return m_nStatus & ICQ_STATUS_FxHIDExIP;
}

bool Licq::User::StatusBirthday() const
{
  return m_nStatus & ICQ_STATUS_FxBIRTHDAY;
}

void Licq::User::SetStatusOffline()
{
  if (isOnline())
  {
    m_nLastCounters[LAST_ONLINE] = time(NULL);
    SaveLicqInfo();
  }

  setIsTyping(false);
  SetUserUpdated(false);
  SetStatus(ICQ_STATUS_OFFLINE);
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
    if (StatusBirthday() && isUser()) return 0;
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

  saveUserInfo();
}

bool Licq::User::Away() const
{
   unsigned short n = Status();
   return (n == ICQ_STATUS_AWAY || n == ICQ_STATUS_NA ||
           n == ICQ_STATUS_DND || n == ICQ_STATUS_OCCUPIED);
}

void User::setHistoryFile(const std::string& file)
{
  string realFile;
  if (file == "default")
  {
    char p[5];
    Licq::protocolId_toStr(p, myId.protocolId());
    realFile = gDaemon.baseDir() + HistoryDir + myId.accountId() + '.' + p + HistoryExt;
  }
  else if (file != "none")
  {
    // use given name
    realFile = file;
  }

  myHistory.setFile(realFile, file);
  SaveLicqInfo();
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

int Licq::User::SocketDesc(unsigned char nChannel) const
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

void Licq::User::SetSocketDesc(Licq::TCPSocket* s)
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
    if (m_bOnContactList)
      gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
          PluginSignal::UserSecurity, myId, m_bSecure ? 1 : 0));
  }

  if (m_nIntIp == 0)
    m_nIntIp = s->getRemoteIpInt();
  if (m_nPort == 0)
    m_nPort = s->getRemotePort();
  SetSendServer(false);
}

void Licq::User::ClearSocketDesc(unsigned char nChannel)
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

  if (m_bOnContactList)
    gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
        PluginSignal::UserSecurity, myId, 0));
}

void Licq::User::clearNormalSocketDesc()
{
  ClearSocketDesc(ICQ_CHNxNONE);
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
  time_t t = time(NULL);
  struct tm *tzone = localtime(&t);
  return -(tzone->tm_gmtoff) + (tzone->tm_isdst == 1 ? 3600 : 0); // seconds _east_ of UTC
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

SecureChannelSupport_et Licq::User::SecureChannelSupport() const
{
  if ((m_nClientTimestamp & 0xFFFF0000) == LICQ_WITHSSL)
    return SECURE_CHANNEL_SUPPORTED;
  else if ((m_nClientTimestamp & 0xFFFF0000) == LICQ_WITHOUTSSL)
    return SECURE_CHANNEL_NOTSUPPORTED;
  else
    return SECURE_CHANNEL_UNKNOWN;
}

unsigned short Licq::User::LicqVersion() const
{
  if ((m_nClientTimestamp & 0xFFFF0000) == LICQ_WITHSSL ||
       (m_nClientTimestamp & 0xFFFF0000) == LICQ_WITHOUTSSL)
    return m_nClientTimestamp & 0x0000FFFF;

  return LicqVersionUnknown;
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

char* Licq::User::IpStr(char* rbuf) const
{
  char ip[32], buf[32];

  if (Ip() > 0)                 // Default to the given ip
    strcpy(ip, ip_ntoa(m_nIp, buf));
  else                          // Otherwise we don't know
    strcpy(ip, tr("Unknown"));

  if (StatusHideIp())
    sprintf(rbuf, "(%s)", ip);
  else
    sprintf(rbuf, "%s", ip);

  return rbuf;
}


char* Licq::User::PortStr(char* rbuf) const
{
  if (Port() > 0)               // Default to the given port
    sprintf(rbuf, "%d", Port());
  else                          // Otherwise we don't know
    rbuf[0] = '\0';

  return rbuf;
}

char* Licq::User::IntIpStr(char* rbuf) const
{
  char buf[32];
  int socket = SocketDesc(ICQ_CHNxNONE);
  if (socket < 0)
    socket = SocketDesc(ICQ_CHNxINFO);
  if (socket < 0)
    socket = SocketDesc(ICQ_CHNxSTATUS);

  if (socket > 0)		// First check if we are connected
  {
    Licq::INetSocket *s = gSocketManager.FetchSocket(socket);
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


char* Licq::User::usprintf(const char* _szFormat, unsigned long nFlags) const
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
          Licq::ProtocolPluginsList plugins;
          gPluginManager.getProtocolPluginsList(plugins);
          BOOST_FOREACH(Licq::ProtocolPlugin::Ptr plugin, plugins)
          {
            if (myId.protocolId() == plugin->getProtocolId())
            {
              strcpy(szTemp, plugin->getName());
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
          sz = statusString(false).c_str();
          break;
        case 's':
          sz = statusString(true).c_str();
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
          if (zone == TimezoneUnknown)
            strcpy(szTemp, tr("Unknown"));
          else
            sprintf(szTemp, tr("GMT%c%i%c0"), (zone > 0 ? '-' : '+'), abs(zone / 2), (zone & 1 ? '3' : '0'));
          sz = szTemp;
          break;
        }

        case 'L':
        {
          char zone = GetTimezone();
          if (zone == TimezoneUnknown)
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
          if (zone == TimezoneUnknown)
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
  if (!EnableSave()) return;

  if (!myConf.loadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
        L_ERRORxSTR, myConf.filename().c_str(), L_BLANKxSTR);
     return;
  }
  myConf.setSection("user");
  myConf.set("Alias", myAlias);
  myConf.set("KeepAliasOnUpdate", m_bKeepAliasOnUpdate);
  myConf.set("Timezone", m_nTimezone);
  myConf.set("Authorization", m_bAuthorization);

  PropertyMap::const_iterator i;
  for (i = myUserInfo.begin(); i != myUserInfo.end(); ++i)
    myConf.set(i->first, i->second);

  saveCategory(myInterests, myConf, "Interests");
  saveCategory(myBackgrounds, myConf, "Backgrounds");
  saveCategory(myOrganizations, myConf, "Organizations");

  if (!myConf.writeFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
        L_ERRORxSTR, myConf.filename().c_str(), L_BLANKxSTR);
    return;
  }
}

void Licq::User::saveCategory(const UserCategoryMap& category, IniFile& file, const string& key)
{
  file.set(key + 'N', category.size());

  UserCategoryMap::const_iterator i;
  unsigned int count = 0;
  for (i = category.begin(); i != category.end(); ++i)
  {
    char n[10];
    snprintf(n, sizeof(n), "%04X", count);
    file.set(key + "Cat" + n, i->first);
    file.set(key + "Desc" + n, i->second);
    ++count;
  }
}

void Licq::User::loadCategory(UserCategoryMap& category, IniFile& file, const string& key)
{
  category.clear();
  unsigned int count;
  file.get(key + 'N', count, 0);

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
    if (!file.get(key + "Cat" + n, cat))
      continue;

    string descr;
    if (!file.get(key + "Desc" + n, descr))
      continue;

    category[cat] = descr;
  }
}

void User::SavePhoneBookInfo()
{
  if (!EnableSave()) return;

  m_PhoneBook->SaveToDisk(myConf);
}

void User::SavePictureInfo()
{
  if (!EnableSave()) return;

  if (!myConf.loadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
        L_ERRORxSTR, myConf.filename().c_str(), L_BLANKxSTR);
     return;
  }
  myConf.setSection("user");
  myConf.set("PicturePresent", m_bPicturePresent);
  myConf.set("BuddyIconType", myBuddyIconType);
  myConf.set("BuddyIconHashType", myBuddyIconHashType);
  myConf.set("BuddyIconHash", myBuddyIconHash);
  myConf.set("OurBuddyIconHash", myOurBuddyIconHash);
  if (!myConf.writeFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
        L_ERRORxSTR, myConf.filename().c_str(), L_BLANKxSTR);
    return;
  }
}

void User::SaveLicqInfo()
{
   if (!EnableSave()) return;

  if (!myConf.loadFile())
  {
      gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
        L_ERRORxSTR, myConf.filename().c_str(), L_BLANKxSTR);
      return;
   }
   char buf[64];
  myConf.setSection("user");
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
  myConf.set("AutoAccept", myAutoAccept);
  myConf.set("StatusToUser", m_nStatusToUser);
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
  myConf.set("PhoneFollowMeStatus", m_nPhoneFollowMeStatus);
  myConf.set("ICQphoneStatus", m_nICQphoneStatus);
  myConf.set("SharedFilesStatus", m_nSharedFilesStatus);
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

  if (!myConf.writeFile())
  {
     gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
        L_ERRORxSTR, myConf.filename().c_str(), L_BLANKxSTR);
     return;
   }
}

void User::SaveNewMessagesInfo()
{
   if (!EnableSave()) return;

  if (!myConf.loadFile())
  {
      gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
        L_ERRORxSTR, myConf.filename().c_str(), L_BLANKxSTR);
      return;
   }
  myConf.setSection("user");
  myConf.set("NewMessages", NewMessages());
  if (!myConf.writeFile())
  {
     gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
        L_ERRORxSTR, myConf.filename().c_str(), L_BLANKxSTR);
     return;
   }
}

void Licq::User::saveAll()
{
  SaveLicqInfo();
  saveUserInfo();
  SavePhoneBookInfo();
  SavePictureInfo();
}

void Licq::User::EventPush(Licq::UserEvent *e)
{
  m_vcMessages.push_back(e);
  incNumUserEvents();
  SaveNewMessagesInfo();
  Touch();
  SetLastReceivedEvent();

  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
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
  if (m_vcMessages.size() == 0) return NULL;
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
  if (m_vcMessages.size() == 0) return (NULL);
  return (m_vcMessages[m_vcMessages.size() - 1]);
}

const Licq::UserEvent* Licq::User::EventPeekFirst() const
{
  if (m_vcMessages.size() == 0) return (NULL);
  return (m_vcMessages[0]);
}

Licq::UserEvent *Licq::User::EventPop()
{
  if (m_vcMessages.size() == 0) return NULL;
  Licq::UserEvent* e = m_vcMessages[0];
  for (unsigned short i = 0; i < m_vcMessages.size() - 1; i++)
    m_vcMessages[i] = m_vcMessages[i + 1];
  m_vcMessages.pop_back();
  decNumUserEvents();
  SaveNewMessagesInfo();

  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
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
  SaveNewMessagesInfo();

  gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
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
      SaveNewMessagesInfo();

      gDaemon.pushPluginSignal(new PluginSignal(PluginSignal::SignalUser,
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
  SaveLicqInfo();
}

bool Licq::User::removeFromGroup(int groupId)
{
  bool inGroup = myGroups.erase(groupId);
  SaveLicqInfo();
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

