#include "licq/contactlist/user.h"

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "gettext.h"
#include "licq_constants.h"
#include "licq_events.h"
#include "licq_file.h"
#include "licq_icqd.h"
#include "licq_log.h"
#include "licq_socket.h"
#include <licq/icqcodes.h>
#include "licq/contactlist/usermanager.h"
#include "licq/pluginmanager.h"

using boost::any_cast;
using boost::bad_any_cast;
using std::map;
using std::string;
using std::vector;
using Licq::ICQUserPhoneBook;
using Licq::SecureChannelSupport_et;
using Licq::User;
using Licq::UserId;
using Licq::gPluginManager;
using Licq::gUserManager;


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

unsigned short User::s_nNumUserEvents = 0;
pthread_mutex_t User::mutex_nNumUserEvents = PTHREAD_MUTEX_INITIALIZER;

User::User(const UserId& id, const string& filename)
  : myId(id)
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

User::User(const UserId& id, bool temporary)
  : myId(id)
{
  Init();
  SetDefaults();
  m_bNotInList = temporary;
  if (!m_bNotInList)
  {
    char szFilename[MAX_FILENAME_LEN];
    char p[5];
    protocolId_toStr(p, myId.protocolId());
    snprintf(szFilename, MAX_FILENAME_LEN, "%s/%s/%s.%s", BASE_DIR, USER_DIR,
        myId.accountId().c_str(), p);
    szFilename[MAX_FILENAME_LEN - 1] = '\0';
    m_fConf.SetFileName(szFilename);
    m_fConf.SetFlags(INI_FxWARN | INI_FxALLOWxCREATE);
  }
}

void User::AddToContactList()
{
  m_bOnContactList = m_bEnableSave = true;
  m_bNotInList = false;

  // Check for old history file
  if (access(m_fHistory.FileName(), F_OK) == -1)
  {
    char szFilename[MAX_FILENAME_LEN];
    char p[5];
    protocolId_toStr(p, myId.protocolId());
    snprintf(szFilename, MAX_FILENAME_LEN, "%s/%s/%s.%s.%s", BASE_DIR, HISTORY_DIR, myId.accountId().c_str(),
             p, HISTORYxOLD_EXT);

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

bool User::LoadInfo()
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

void User::loadUserInfo()
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

void User::LoadPhoneBookInfo()
{
  m_PhoneBook->LoadFromDisk(m_fConf);
}

void User::LoadPictureInfo()
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

void User::LoadLicqInfo()
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
  if (isUser()) // Only allow to keep a modified alias for user uins
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


User::~User()
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

void User::RemoveFiles()
{
  remove(m_fConf.FileName());

  // Check for old history file and back up
  struct stat buf;
  if (stat(m_fHistory.FileName(), &buf) == 0 && buf.st_size > 0)
  {
    char szFilename[MAX_FILENAME_LEN];
    char p[5];
    protocolId_toStr(p, myId.protocolId());
    snprintf(szFilename, MAX_FILENAME_LEN, "%s/%s/%s.%s.%s", BASE_DIR, HISTORY_DIR,
        myId.accountId().c_str(), p, HISTORYxOLD_EXT);

    szFilename[MAX_FILENAME_LEN - 1] = '\0';
    if (rename(m_fHistory.FileName(), szFilename) == -1)
    {
      gLog.Warn(tr("%sFailed to rename history file (%s):\n%s%s\n"), L_WARNxSTR,
          szFilename, L_BLANKxSTR, strerror(errno));
      remove(m_fHistory.FileName());
    }
  }
}

void User::Init()
{
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

  myMutex.setName(myId.toString());
}

void User::SetPermanent()
{
  // Set the flags and check for history file to recover
  AddToContactList();

  // Create the user file
  char szFilename[MAX_FILENAME_LEN];
  char p[5];
  protocolId_toStr(p, myId.protocolId());
  snprintf(szFilename, MAX_FILENAME_LEN, "%s/%s/%s.%s", BASE_DIR, USER_DIR,
      myId.accountId().c_str(), p);
  szFilename[MAX_FILENAME_LEN - 1] = '\0';
  m_fConf.SetFileName(szFilename);
  m_fConf.SetFlags(INI_FxWARN | INI_FxALLOWxCREATE);

  // Save all the info now
  saveAll();

  // Notify the plugins of the change
  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER,
      USER_SETTINGS, myId, 0));
}

void User::SetDefaults()
{
  char szTemp[12];
  setAlias(myId.accountId());
  SetHistoryFile("default");
  SetSystemGroups(0);
  myGroups.clear();
  SetNewUser(true);
  SetAuthorization(false);

  szTemp[0] = '\0';
  SetCustomAutoResponse(szTemp);
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

std::string User::getFullName() const
{
  string name = getFirstName();
  string lastName = getLastName();
  if (!name.empty() && !lastName.empty())
    name += ' ';
  return name + lastName;
}

std::string User::getEmail() const
{
  string email = getUserInfoString("Email1");
  if (email.empty())
    email = getUserInfoString("Email2");
  if (email.empty())
    email = getUserInfoString("Email0");
  return email;
}

const char* User::UserEncoding() const
{
  if (m_szEncoding == NULL || m_szEncoding[0] == '\0')
    return gUserManager.DefaultUserEncoding();
  else
    return m_szEncoding;
}

void User::setStatus(unsigned status)
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
    m_nStatus |= ICQ_STATUS_NA;
  if (status & OccupiedStatus)
    m_nStatus |= ICQ_STATUS_OCCUPIED;
  if (status & DoNotDisturbStatus)
    m_nStatus |= ICQ_STATUS_DND;
  if (status & FreeForChatStatus)
    m_nStatus |= ICQ_STATUS_FREEFORCHAT;
}

void User::SetStatus(unsigned long n)
{
  m_nStatus = n;

  // Build status from ICQ flags
  myStatus = statusFromIcqStatus(m_nStatus & 0xFFFF);
  if (myStatus != OfflineStatus && m_nIdleSince != 0)
    myStatus |= IdleStatus;
}

unsigned short User::icqStatusFromStatus(unsigned status)
{
  if (status == OfflineStatus)
    return ICQ_STATUS_OFFLINE;
  if (status & DoNotDisturbStatus)
    return ICQ_STATUS_DND;
  if (status & OccupiedStatus)
    return ICQ_STATUS_OCCUPIED;
  if (status & NotAvailableStatus)
    return ICQ_STATUS_NA;
  if (status & AwayStatus)
    return ICQ_STATUS_AWAY;
  if (status & FreeForChatStatus)
    return ICQ_STATUS_FREEFORCHAT;
  if (status & InvisibleStatus)
   return ICQ_STATUS_FxPRIVATE;
  return ICQ_STATUS_ONLINE;
}

unsigned User::statusFromIcqStatus(unsigned short icqStatus)
{
  // Build status from ICQ flags
  if (icqStatus == ICQ_STATUS_OFFLINE)
    return OfflineStatus;

  unsigned status = OnlineStatus;
  if (icqStatus & ICQ_STATUS_FxPRIVATE)
    status |= InvisibleStatus;
  if (icqStatus & ICQ_STATUS_AWAY)
    status |= AwayStatus;
  if (icqStatus & ICQ_STATUS_NA)
    status |= NotAvailableStatus;
  if (icqStatus & ICQ_STATUS_OCCUPIED)
    status |= OccupiedStatus;
  if (icqStatus & ICQ_STATUS_DND)
    status |= DoNotDisturbStatus;
  if (icqStatus & ICQ_STATUS_FREEFORCHAT)
    status |= FreeForChatStatus;

  return status;
}

unsigned short User::Status() const
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

void User::SetStatusOffline()
{
  if (isOnline())
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
int User::Birthday(unsigned short nRange) const
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

unsigned short User::Sequence(bool increment)
{
   if (increment)
      return (m_nSequence--);
   else
      return (m_nSequence);
}

void User::setAlias(const string& alias)
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
    TLVPtr aliasTLV(new COscarTLV(0x131, aliasLen, myAlias.c_str()));
    AddTLV(aliasTLV);
  }

  saveUserInfo();
}

bool User::Away() const
{
   unsigned short n = Status();
   return (n == ICQ_STATUS_AWAY || n == ICQ_STATUS_NA ||
           n == ICQ_STATUS_DND || n == ICQ_STATUS_OCCUPIED);
}

void User::SetHistoryFile(const char *s)
{
  m_fHistory.SetFile(s, myId.accountId().c_str(), myId.protocolId());
  SaveLicqInfo();
}

void User::SetIpPort(unsigned long _nIp, unsigned short _nPort)
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

int User::SocketDesc(unsigned char nChannel) const
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

void User::SetSocketDesc(TCPSocket *s)
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

void User::ClearSocketDesc(unsigned char nChannel)
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

unsigned short User::ConnectionVersion() const
{
  // If we are already connected, use that version
  if (m_nConnectionVersion != 0) return m_nConnectionVersion;
  // We aren't connected, see if we know their version
  return VersionToUse(m_nVersion);
}

int User::LocalTimeGMTOffset() const
{
  return GetTimezone() * 1800;
}

int User::SystemTimeGMTOffset()
{
  time_t t = time(NULL);
  struct tm *tzone = localtime(&t);
  return -(tzone->tm_gmtoff) + (tzone->tm_isdst == 1 ? 3600 : 0); // seconds _east_ of UTC
}

char User::SystemTimezone()
{
  char nTimezone = SystemTimeGMTOffset() / 1800;
  if (nTimezone > 23)
    return 23 - nTimezone;
  return nTimezone;
}

int User::LocalTimeOffset() const
{
  return SystemTimeGMTOffset() - LocalTimeGMTOffset();
}

time_t User::LocalTime() const
{
  return time(NULL) + LocalTimeOffset();
}

SecureChannelSupport_et User::SecureChannelSupport() const
{
  if ((m_nClientTimestamp & 0xFFFF0000) == LICQ_WITHSSL)
    return SECURE_CHANNEL_SUPPORTED;
  else if ((m_nClientTimestamp & 0xFFFF0000) == LICQ_WITHOUTSSL)
    return SECURE_CHANNEL_NOTSUPPORTED;
  else
    return SECURE_CHANNEL_UNKNOWN;
}

unsigned short User::LicqVersion() const
{
  if ((m_nClientTimestamp & 0xFFFF0000) == LICQ_WITHSSL ||
       (m_nClientTimestamp & 0xFFFF0000) == LICQ_WITHOUTSSL)
    return m_nClientTimestamp & 0x0000FFFF;

  return LICQ_VERSION_UNKNOWN;
}

const char* User::StatusStr() const
{
  return StatusToStatusStr(m_nStatus, isInvisible());
}

const char* User::StatusStrShort() const
{
  return StatusToStatusStrShort(m_nStatus, isInvisible());
}

const char* User::StatusToStatusStr(unsigned short n, bool b)
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

const char* User::StatusToStatusStrShort(unsigned short n, bool b)
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

unsigned User::singleStatus(unsigned status)
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

string User::statusToString(unsigned status, bool full, bool markInvisible)
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

char* User::IpStr(char* rbuf) const
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


char* User::PortStr(char* rbuf) const
{
  if (Port() > 0)               // Default to the given port
    sprintf(rbuf, "%d", Port());
  else                          // Otherwise we don't know
    rbuf[0] = '\0';

  return rbuf;
}

char* User::IntIpStr(char* rbuf) const
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


char* User::usprintf(const char* _szFormat, unsigned long nFlags) const
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

void User::saveCategory(const UserCategoryMap& category, CIniFile& file, const string& key)
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

void User::loadCategory(UserCategoryMap& category, CIniFile& file, const string& key)
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

void User::SavePhoneBookInfo()
{
  if (!EnableSave()) return;

  m_PhoneBook->SaveToDisk(m_fConf);
}

void User::SavePictureInfo()
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

void User::SaveLicqInfo()
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

   map<string,string>::iterator iter;
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

void User::SaveNewMessagesInfo()
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

void User::saveAll()
{
  SaveLicqInfo();
  saveUserInfo();
  SavePhoneBookInfo();
  SavePictureInfo();
}

void User::EventPush(CUserEvent *e)
{
  m_vcMessages.push_back(e);
  incNumUserEvents();
  SaveNewMessagesInfo();
  Touch();
  SetLastReceivedEvent();

  gLicqDaemon->pushPluginSignal(new LicqSignal(SIGNAL_UPDATExUSER,
      USER_EVENTS, myId, e->Id(), e->ConvoId()));
}

void User::WriteToHistory(const char *_szText)
{
  m_fHistory.Append(_szText);
}

void User::CancelEvent(unsigned short index)
{
  if (index < NewMessages())
    return;
  m_vcMessages[index]->Cancel();
}

const CUserEvent* User::EventPeek(unsigned short index) const
{
  if (index >= NewMessages()) return (NULL);
  return (m_vcMessages[index]);
}

const CUserEvent* User::EventPeekId(int id) const
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

const CUserEvent* User::EventPeekLast() const
{
  if (m_vcMessages.size() == 0) return (NULL);
  return (m_vcMessages[m_vcMessages.size() - 1]);
}

const CUserEvent* User::EventPeekFirst() const
{
  if (m_vcMessages.size() == 0) return (NULL);
  return (m_vcMessages[0]);
}

CUserEvent *User::EventPop()
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

void User::EventClear(unsigned short index)
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

void User::EventClearId(int id)
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

bool User::GetInGroup(GroupType gtype, int groupId) const
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

void User::SetInGroup(GroupType g, int _nGroup, bool _bIn)
{
  if (_bIn)
    AddToGroup(g, _nGroup);
  else
    RemoveFromGroup(g, _nGroup);
}

void User::AddToGroup(GroupType gtype, int groupId)
{
  if (groupId <= 0)
    return;

  if (gtype == GROUPS_SYSTEM)
    mySystemGroups |= (1L << (groupId - 1));
  else
    myGroups.insert(groupId);
  SaveLicqInfo();
}

bool User::RemoveFromGroup(GroupType gtype, int groupId)
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

unsigned short User::getNumUserEvents()
{
  pthread_mutex_lock(&mutex_nNumUserEvents);
  unsigned short n = s_nNumUserEvents;
  pthread_mutex_unlock(&mutex_nNumUserEvents);
  return n;
}

void User::incNumUserEvents()
{
  pthread_mutex_lock(&mutex_nNumUserEvents);
  s_nNumUserEvents++;
  pthread_mutex_unlock(&mutex_nNumUserEvents);
}

void User::decNumUserEvents()
{
  pthread_mutex_lock(&mutex_nNumUserEvents);
  s_nNumUserEvents--;
  pthread_mutex_unlock(&mutex_nNumUserEvents);
}

bool User::SetPPField(const string &_sName, const string &_sValue)
{
  m_mPPFields[_sName] = _sValue;
  return true;
}

string User::GetPPField(const string &_sName)
{
  map<string,string>::iterator iter = m_mPPFields.find(_sName);
  if (iter != m_mPPFields.end())
    return iter->second;

  return string("");
}

void User::AddTLV(TLVPtr tlv)
{
  myTLVs[tlv->getType()] = tlv;
}

void User::RemoveTLV(unsigned long type)
{
  myTLVs.erase(type);
}

void User::SetTLVList(TLVList& tlvs)
{
  myTLVs.clear();

  for (TLVListIter it = tlvs.begin(); it != tlvs.end(); it++)
    myTLVs[it->first] = it->second;
}

