/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2013 Licq developers <licq-dev@googlegroups.com>
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
#include <fcntl.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "gettext.h"
#include <licq/logging/log.h>
#include <licq/inifile.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/oneventmanager.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/socket.h>
#include <licq/userevents.h>

using boost::any_cast;
using boost::bad_any_cast;
using std::map;
using std::string;
using std::stringstream;
using std::vector;
using LicqDaemon::UserHistory;
using namespace Licq;


unsigned short Licq::User::s_nNumUserEvents = 0;
pthread_mutex_t Licq::User::mutex_nNumUserEvents = PTHREAD_MUTEX_INITIALIZER;


User::Private::Private(User* user, const UserId& id)
  : myUser(user),
    myId(id),
    myHistory(myId)
{
  // Empty
}

User::Private::~Private()
{
  // Empty
}

User::User(const UserId& id, bool temporary)
  : myId(id),
    m_bNotInList(temporary),
    myPrivate(new Private(this, id))
{
  LICQ_D();

  // Cache protocol capabilities as a convenience
  Licq::ProtocolPlugin::Ptr protocol = Licq::gPluginManager.getProtocolPlugin(myId.protocolId());
  myProtocolCapabilities = (protocol.get() != NULL ? protocol->capabilities() : 0);

  myServerGroup = 0;
  if ((myProtocolCapabilities & ProtocolPlugin::CanSingleGroup) == 0)
    // Protocol handles multiple groups or no groups at all
    myServerGroup = -1;

  d->Init();

  // Start building filename for user files
  string filename = "users/" + myId.ownerId().accountId()
      + "." + Licq::protocolId_toString(myId.protocolId());

  // Create owner specific dir if needed
  string dirname = gDaemon.baseDir() + filename.c_str();
  if (mkdir(dirname.c_str(), 0700) < 0 && errno != EEXIST)
    gLog.error(tr("Failed to create directory %s: %s"), dirname.c_str(), strerror(errno));

  // Add final parts to get filenames
  filename += "/" + myId.accountId();
  d->myConf.setFilename(filename + ".conf");
  d->myHistory.setFile(gDaemon.baseDir() + filename + ".history");
  myPictureFileName = gDaemon.baseDir() + filename + ".picture";

  if (m_bNotInList)
  {
    d->setDefaults();
    return;
  }


  // Make sure we have a file so load won't fail
  if (!d->myConf.loadFile())
  {
    d->myConf.setSection("user");
    if (!d->myConf.writeFile())
      gLog.error(tr("Error opening '%s' for writing."), d->myConf.filename().c_str());
    d->setDefaults();
  }
  else
  {
    d->myConf.setSection("user");
  }

  d->loadUserInfo();
  d->loadPictureInfo();
  d->loadLicqInfo();
}

IniFile& User::userConf()
{
  LICQ_D();
  return d->myConf;
}

void User::Private::addToContactList()
{
  myUser->m_bOnContactList = true;
  myUser->m_bEnableSave = true;
  myUser->m_bNotInList = false;
}

void User::Private::loadUserInfo()
{
  // read in the fields, checking for errors each time
  myConf.setSection("user");
  myConf.get("Alias", myUser->myAlias, tr("Unknown"));
  myConf.get("Timezone", myUser->myTimezone, TimezoneUnknown);
  if (myUser->myTimezone == -100) // Old value used for unknown
    myUser->myTimezone = TimezoneUnknown;
  if (myUser->myTimezone > -24 && myUser->myTimezone <= 24) // Old ICQ style values
    myUser->myTimezone *= -1800;
  myConf.get("Authorization", myUser->m_bAuthorization, false);

  PropertyMap::iterator i;
  for (i = myUserInfo.begin(); i != myUserInfo.end(); ++i)
    myConf.get(i->first, i->second);
}

void User::Private::loadPictureInfo()
{
  myConf.setSection("user");
  myConf.get("PicturePresent", myUser->m_bPicturePresent, false);
}

void User::Private::loadLicqInfo()
{
  // read in the fields, checking for errors each time
  string temp;
  unsigned nNewMessages;
  unsigned long nLast;
  myConf.setSection("user");

  // Get deprecated parameter and use as default if new values aren't available
  unsigned long oldSystemGroups;
  myConf.get("Groups.System", oldSystemGroups, 0);

  myConf.get("OnVisibleList", myUser->myOnVisibleList, oldSystemGroups & 1<<1);
  myConf.get("OnInvisibleList", myUser->myOnInvisibleList, oldSystemGroups & 1<<2);
  myConf.get("OnIgnoreList", myUser->myOnIgnoreList, oldSystemGroups & 1<<3);
  myConf.get("OnlineNotify", myUser->myOnlineNotify, oldSystemGroups & 1<<0);
  myConf.get("NewUser", myUser->myNewUser, oldSystemGroups & 1<<4);
  myConf.get("Ip", temp, "0.0.0.0");
  struct in_addr in;
  myUser->m_nIp = inet_pton(AF_INET, temp.c_str(), &in);
  if (myUser->m_nIp > 0)
    myUser->m_nIp = in.s_addr;
  myConf.get("IntIp", temp, "0.0.0.0");
  myUser->m_nIntIp = inet_pton(AF_INET, temp.c_str(), &in);
  if (myUser->m_nIntIp > 0)
    myUser->m_nIntIp = in.s_addr;
  myConf.get("Port", myUser->m_nPort, 0);
  myConf.get("NewMessages", nNewMessages, 0);
  myConf.get("LastOnline", nLast, 0);
  myUser->m_nLastCounters[Licq::LAST_ONLINE] = nLast;
  myConf.get("LastSent", nLast, 0);
  myUser->m_nLastCounters[Licq::LAST_SENT_EVENT] = nLast;
  myConf.get("LastRecv", nLast, 0);
  myUser->m_nLastCounters[Licq::LAST_RECV_EVENT] = nLast;
  myConf.get("LastCheckedAR", nLast, 0);
  myUser->m_nLastCounters[Licq::LAST_CHECKED_AR] = nLast;
  myConf.get("RegisteredTime", nLast, 0);
  myUser->m_nRegisteredTime = nLast;

  unsigned autoAcceptFlags;
  myConf.get("AutoAccept", autoAcceptFlags, 0);
  myUser->myAcceptInAway                = (autoAcceptFlags & 0x0001);
  myUser->myAcceptInNotAvailable        = (autoAcceptFlags & 0x0002);
  myUser->myAcceptInOccupied            = (autoAcceptFlags & 0x0004);
  myUser->myAcceptInDoNotDisturb        = (autoAcceptFlags & 0x0008);
  myUser->myAutoAcceptChat              = (autoAcceptFlags & 0x0100);
  myUser->myAutoAcceptFile              = (autoAcceptFlags & 0x0200);
  myUser->myAutoSecure                  = (autoAcceptFlags & 0x0400);

  unsigned icqStatusToUser;
  myConf.get("StatusToUser", icqStatusToUser, 0xFFFF);
  if (icqStatusToUser == 0xFFFF)
    myUser->myStatusToUser = OfflineStatus;
  else
  {
    myUser->myStatusToUser = OnlineStatus;
    if (icqStatusToUser & 0x0002)       myUser->myStatusToUser |= DoNotDisturbStatus;
    else if (icqStatusToUser & 0x0010)  myUser->myStatusToUser |= OccupiedStatus;
    else if (icqStatusToUser & 0x0004)  myUser->myStatusToUser |= NotAvailableStatus;
    else if (icqStatusToUser & 0x0001)  myUser->myStatusToUser |= AwayStatus;
    else if (icqStatusToUser & 0x0020)  myUser->myStatusToUser |= FreeForChatStatus;
    if (icqStatusToUser & 0x0100)       myUser->myStatusToUser |= InvisibleStatus;
  }

  if (myUser->isUser()) // Only allow to keep a modified alias for user uins
    myConf.get("KeepAliasOnUpdate", myUser->m_bKeepAliasOnUpdate, false);
  else
    myUser->m_bKeepAliasOnUpdate = false;
  myConf.get("CustomAutoRsp", myUser->myCustomAutoResponse, "");
  myConf.get("SendIntIp", myUser->m_bSendIntIp, false);
  myConf.get("UserEncoding", myUser->myEncoding, "");
  myConf.get("AwaitingAuth", myUser->m_bAwaitingAuth, false);
  myConf.get("UseGPG", myUser->m_bUseGPG, false );
  myConf.get("GPGKey", myUser->myGpgKey, "");
  myConf.get("SendServer", myUser->m_bSendServer, false);

  if (myUser->myServerGroup > -1)
  {
    if (!myConf.get("ServerGroup", myUser->myServerGroup, 0) && myId.protocolId() == ICQ_PPID)
    {
      unsigned gsid;
      myConf.get("GSID", gsid, 0);
      myUser->myServerGroup = gUserManager.getGroupFromServerId(myId.ownerId(), gsid);
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
        myUser->addToGroup(groupId);
    }
  }
  else
  {
    // Groupcount is missing in user config, try and read old group configuration
    unsigned int oldGroups;
    myConf.get("Groups.User", oldGroups, 0);
    for (int i = 0; i <= 31; ++i)
      if (oldGroups & (1L << i))
        myUser->addToGroup(i+1);
  }

  if (nNewMessages > 0)
  {
    Licq::HistoryList hist;
    if (myUser->GetHistory(hist))
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
        myUser->m_vcMessages.push_back( (*it)->Copy() );
        myUser->incNumUserEvents();
        it++;
      }
    }
    myUser->ClearHistory(hist);
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

  delete myPrivate;
}

void User::Private::removeFiles()
{
  remove(myConf.filename().c_str());
}

void User::Private::Init()
{
  //myUser->SetOnContactList(false);
  myUser->m_bOnContactList = false;
  myUser->m_bEnableSave = false;
  myUser->myAutoResponse = "";
  myUser->myEncoding = "";
  myUser->m_bSecure = false;

  // TODO: Only user data fields valid for protocol should be populated

  // General Info
  myUser->myAlias = string();
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
  myUserInfo["Country"] = (unsigned int)0; // COUNTRY_UNSPECIFIED
  myUserInfo["HideEmail"] = false;
  myUser->myTimezone = TimezoneUnknown;
  myUser->m_bAuthorization = false;
  myUser->myIsTyping = false;
  myUser->m_bNotInList = false;
  myUser->myOnEventsBlocked = false;

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

  // Work Info
  myUserInfo["CompanyCity"] = string();
  myUserInfo["CompanyState"] = string();
  myUserInfo["CompanyPhoneNumber"] = string();
  myUserInfo["CompanyFaxNumber"] = string();
  myUserInfo["CompanyAddress"] = string();
  myUserInfo["CompanyZip"] = string();
  myUserInfo["CompanyCountry"] = (unsigned int)0; // COUNTRY_UNSPECIFIED
  myUserInfo["CompanyName"] = string();
  myUserInfo["CompanyDepartment"] = string();
  myUserInfo["CompanyPosition"] = string();
  myUserInfo["CompanyOccupation"] = (unsigned int)0; // OCCUPATION_UNSPECIFIED
  myUserInfo["CompanyHomepage"] = string();

  // About
  myUserInfo["About"] = string();

  // Picture
  myUser->m_bPicturePresent = false;

  // GPG key
  myUser->myGpgKey = "";

  // gui plugin compat
  myUser->myStatus = OfflineStatus;
  myUser->myAutoResponse = "";
  myUser->SetSendServer(false);
  myUser->SetSendIntIp(false);
  myUser->SetShowAwayMsg(false);
  myUser->SetOfflineOnDisconnect(false);
  myUser->m_nIp = 0;
  myUser->m_nPort = 0;
  myUser->m_nIntIp = 0;
  myUser->m_bUserUpdated = false;
  myUser->myWebPresence = false;
  myUser->myHideIp = false;
  myUser->myBirthdayFlag = false;
  myUser->myHomepageFlag = false;
  myUser->mySecureChannelSupport = SecureChannelUnknown;
  myUser->Touch();
  for (unsigned short i = 0; i < 4; i++)
    myUser->m_nLastCounters[i] = 0;
  myUser->m_nOnlineSince = 0;
  myUser->m_nIdleSince = 0;
  myUser->myAwaySince = 0;
  myUser->m_nRegisteredTime = 0;
  myUser->myStatusToUser = OfflineStatus;
  myUser->m_bKeepAliasOnUpdate = false;
  myUser->myAutoAcceptChat = false;
  myUser->myAutoAcceptFile = false;
  myUser->myAutoSecure = false;
  myUser->myAcceptInAway = false;
  myUser->myAcceptInNotAvailable = false;
  myUser->myAcceptInOccupied = false;
  myUser->myAcceptInDoNotDisturb = false;
  myUser->myCustomAutoResponse = "";
  myUser->m_bConnectionInProgress = false;
  myUser->m_bAwaitingAuth = false;
  myUser->myClientInfo = "";

  myUser->myMutex.setName(myId.toString());
}

void User::Private::setPermanent()
{
  // Set the flags and check for history file to recover
  addToContactList();

  // Create file so save will have something to write in
  if (!myConf.loadFile())
  {
    myConf.setSection("user");
    if (!myConf.writeFile())
      gLog.error(tr("Error opening '%s' for writing."), myConf.filename().c_str());
  }

  // Save all the info now
  myUser->save(SaveAll);
}

void User::Private::setDefaults()
{
  myUser->setAlias(myId.accountId());
  myUser->myOnVisibleList = false;
  myUser->myOnInvisibleList = false;
  myUser->myOnIgnoreList = false;
  myUser->myGroups.clear();
  myUser->SetNewUser(true);
  myUser->SetAuthorization(false);
  myUser->myOnlineNotify = false;

  myUser->clearCustomAutoResponse();
}

string User::getUserInfoString(const string& key) const
{
  LICQ_D();

  try
  {
    PropertyMap::const_iterator i = d->myUserInfo.find(key);
    if (i != d->myUserInfo.end())
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
  LICQ_D();

  try
  {
    PropertyMap::const_iterator i = d->myUserInfo.find(key);
    if (i != d->myUserInfo.end())
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
  LICQ_D();

  try
  {
    PropertyMap::const_iterator i = d->myUserInfo.find(key);
    if (i != d->myUserInfo.end())
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
  LICQ_D();

  PropertyMap::iterator i = d->myUserInfo.find(key);
  if (i == d->myUserInfo.end() || i->second.type() != typeid(string))
    return;

  i->second = value;
  save(SaveUserInfo);
}

void User::setUserInfoUint(const string& key, unsigned int value)
{
  LICQ_D();

  PropertyMap::iterator i = d->myUserInfo.find(key);
  if (i == d->myUserInfo.end() || i->second.type() != typeid(unsigned int))
    return;

  i->second = value;
  save(SaveUserInfo);
}

void User::setUserInfoBool(const string& key, bool value)
{
  LICQ_D();

  PropertyMap::iterator i = d->myUserInfo.find(key);
  if (i == d->myUserInfo.end() || i->second.type() != typeid(bool))
    return;

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

    if (myIsTyping)
    {
      setIsTyping(false);
      gPluginManager.pushPluginSignal(new Licq::PluginSignal(
          Licq::PluginSignal::SignalUser, Licq::PluginSignal::UserTyping, myId, 0));
    }
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

  save(SaveUserInfo);
}

int User::GetHistory(Licq::HistoryList& history) const
{
  LICQ_D();

  return d->myHistory.load(history, userEncoding());
}

void Licq::User::ClearHistory(HistoryList& h)
{
  UserHistory::clear(h);
}

bool User::canSendDirect() const
{
  if ((myProtocolCapabilities & Licq::ProtocolPlugin::CanSendDirect) == 0)
    // Protocol can't send direct at all
    return false;

  if (!isOnline() || InvisibleList())
    // Don't connect to offline user and don't reveal invisible list
    return false;

  // Even if ip/port are missing a reverse connect via server might be possible
  return true;
}

void Licq::User::SetIpPort(unsigned long _nIp, unsigned short _nPort)
{
  m_nIp = _nIp;
  m_nPort = _nPort;
  save(SaveLicqInfo);
}

void Licq::User::clearSocketDesc(INetSocket* /* s */)
{
  /* Empty, reimplemented in subclasses as needed */
}

int Licq::User::systemTimezone()
{
  // Get local time
  time_t lt = time(NULL);

  // Get GMT time
  struct tm gtm;
  gmtime_r(&lt, &gtm);
  time_t gt = mktime(&gtm);

  // Diff is seconds east of UTC
  return lt - gt;
}

int Licq::User::LocalTimeOffset() const
{
  return myTimezone - systemTimezone();
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
  if (IntIp() > 0)		// Default to the given ip
  {
    char buf[32];
    return ip_ntoa(m_nIntIp, buf);
  }
  else			// Otherwise we don't know
    return "";
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
            int zone = timezone();
            if (zone == TimezoneUnknown)
              sz = tr("Unknown");
            else
            {
              char buf[128];
              snprintf(buf, 128, tr("GMT%c%i:%02i"), (zone >= 0 ? '+' : '-'), abs(zone/3600), abs(zone/60)%60);
              sz = buf;
            }
            break;
          }

          case 'L':
          case 'F':
          {
            int zone = timezone();
            if (zone == TimezoneUnknown)
              sz = tr("Unknown");
            else
            {
              time_t t = time(NULL) + zone;
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
  if (ppid == ICQ_PPID && !accountId.empty() && !isdigit(accountId[0]))
  {
    boost::erase_all(realId, " ");
    boost::to_lower(realId);
  }

  return realId;
}

void User::saveUserInfo()
{
  LICQ_D();

  d->myConf.set("Alias", myAlias);
  d->myConf.set("KeepAliasOnUpdate", m_bKeepAliasOnUpdate);
  d->myConf.set("Timezone", myTimezone);
  d->myConf.set("Authorization", m_bAuthorization);

  PropertyMap::const_iterator i;
  for (i = d->myUserInfo.begin(); i != d->myUserInfo.end(); ++i)
    d->myConf.set(i->first, i->second);
}

void User::savePictureInfo()
{
  LICQ_D();

  d->myConf.set("PicturePresent", m_bPicturePresent);
}

void User::saveLicqInfo()
{
  LICQ_D();

   char buf[64];
  d->myConf.unset("History");
  d->myConf.unset("Groups.System");
  d->myConf.set("OnVisibleList", myOnVisibleList);
  d->myConf.set("OnInvisibleList", myOnInvisibleList);
  d->myConf.set("OnIgnoreList", myOnIgnoreList);
  d->myConf.set("OnlineNotify", myOnlineNotify);
  d->myConf.set("NewUser", myNewUser);
  d->myConf.set("Ip", Licq::ip_ntoa(m_nIp, buf));
  d->myConf.set("IntIp", Licq::ip_ntoa(m_nIntIp, buf));
  d->myConf.set("Port", Port());
  d->myConf.set("NewMessages", NewMessages());
  d->myConf.set("LastOnline", (unsigned long)LastOnline());
  d->myConf.set("LastSent", (unsigned long)LastSentEvent());
  d->myConf.set("LastRecv", (unsigned long)LastReceivedEvent());
  d->myConf.set("LastCheckedAR", (unsigned long)LastCheckedAutoResponse());
  d->myConf.set("RegisteredTime", (unsigned long)RegisteredTime());

  unsigned autoAcceptFlags = 0;
  if (myAcceptInAway)           autoAcceptFlags |= 0x0001;
  if (myAcceptInNotAvailable)   autoAcceptFlags |= 0x0002;
  if (myAcceptInOccupied)       autoAcceptFlags |= 0x0004;
  if (myAcceptInDoNotDisturb)   autoAcceptFlags |= 0x0008;
  if (myAutoAcceptChat)         autoAcceptFlags |= 0x0100;
  if (myAutoAcceptFile)         autoAcceptFlags |= 0x0200;
  if (myAutoSecure)             autoAcceptFlags |= 0x0400;
  d->myConf.set("AutoAccept", autoAcceptFlags);

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
  d->myConf.set("StatusToUser", icqStatusToUser);

  d->myConf.set("CustomAutoRsp", customAutoResponse());
  d->myConf.set("SendIntIp", m_bSendIntIp);
  d->myConf.set("UserEncoding", myEncoding);
  d->myConf.set("AwaitingAuth", m_bAwaitingAuth);
  d->myConf.set("UseGPG", m_bUseGPG );
  d->myConf.set("GPGKey", myGpgKey );
  d->myConf.set("SendServer", m_bSendServer);

  if (myServerGroup > -1)
    d->myConf.set("ServerGroup", myServerGroup);
  d->myConf.unset("GSID");
  d->myConf.unset("Groups.User");
  d->myConf.set("GroupCount", static_cast<unsigned int>(myGroups.size()));
  int i = 1;
  for (Licq::UserGroupList::iterator g = myGroups.begin(); g != myGroups.end(); ++g)
  {
    sprintf(buf, "Group%u", i);
    d->myConf.set(buf, *g);
    ++i;
  }
}

void User::saveNewMessagesInfo()
{
  LICQ_D();

  d->myConf.set("NewMessages", NewMessages());
}

void User::saveOwnerInfo()
{
  // Empty here, overloaded by owner object
}

void User::save(unsigned group)
{
  if (!EnableSave())
    return;

  LICQ_D();

  if (!d->myConf.loadFile())
  {
    gLog.error(tr("Error opening '%s' for reading. See log for details."),
        d->myConf.filename().c_str());
    return;
  }

  d->myConf.setSection("user");

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

  if (!d->myConf.writeFile())
    gLog.error(tr("Error opening '%s' for writing. See log for details."),
        d->myConf.filename().c_str());
}

bool Licq::User::readPictureData(std::string& pictureData) const
{
  int fd = ::open(pictureFileName().c_str(), O_RDONLY);
  if (fd == -1)
  {
    gLog.error(tr("Could not open picture file '%s' for reading (%s)"),
               pictureFileName().c_str(), ::strerror(errno));
    return false;
  }

  uint8_t buffer[1024];
  ssize_t count;
  while ((count = ::read(fd, &buffer, sizeof(buffer))) > 0)
    pictureData.append(buffer, buffer + count);

  if (count < 0)
    gLog.error(tr("Could not read picture file '%s' (%s)"),
               pictureFileName().c_str(), ::strerror(errno));

  ::close(fd);
  return count == 0;
}

bool Licq::User::writePictureData(const std::string& pictureData) const
{
  int fd = ::open(pictureFileName().c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR);
  if (fd == -1)
  {
    gLog.error(tr("Could not open picture file '%s' for writing (%s)"),
               pictureFileName().c_str(), ::strerror(errno));
    return false;
  }

  const ssize_t count = ::write(fd, pictureData.data(), pictureData.size());
  if (count != static_cast<ssize_t>(pictureData.size()))
    gLog.error(tr("Could not write picture file '%s' (%s)"),
               pictureFileName().c_str(), ::strerror(errno));

  ::close(fd);
  return count == static_cast<ssize_t>(pictureData.size());
}

bool Licq::User::deletePictureData() const
{
  if (::unlink(pictureFileName().c_str()) == -1 && errno != ENOENT)
  {
    gLog.error(tr("Could not delete the picture file '%s' (%s)"),
               pictureFileName().c_str(), ::strerror(errno));
    return false;
  }
  return true;
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

void User::Private::writeToHistory(const string& text)
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

unsigned long Licq::protocolId_fromString(const std::string& s)
{
  // Known names (case insensitive compare) and raw string versions of protocol id
  if (boost::iequals(s, "ICQ") || s == "ICQ_" || s == "Licq")
    return ICQ_PPID;
  if (boost::iequals(s, "MSN") || s == "MSN_")
    return MSN_PPID;
  if (boost::iequals(s, "Jabber") || s == "XMPP")
    return JABBER_PPID;

  // Try the names of all loaded plugins
  Licq::ProtocolPluginsList plugins;
  gPluginManager.getProtocolPluginsList(plugins);
  BOOST_FOREACH(Licq::ProtocolPlugin::Ptr plugin, plugins)
  {
    if (boost::iequals(s, plugin->name()))
      return plugin->protocolId();
  }

  // Unknown
  return 0;
}
