/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#include <cstdio>

#include <licq/contactlist/usermanager.h>
#include <licq/icq/icq.h>
#include <licq/inifile.h>
#include <licq/logging/log.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>

#include "../gettext.h"

#include "icq.h"
#include "socket.h"

using std::string;
using std::vector;
using namespace LicqIcq;
using Licq::ICQUserPhoneBook;
using Licq::IcqUser;
using Licq::IniFile;
using Licq::PhoneBookEntry;
using Licq::UserCategoryMap;
using Licq::gLog;


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

IcqUser::IcqUser(const Licq::UserId& id, bool temporary, bool isOwner)
  : Licq::User(id, temporary, isOwner)
{
  // Empty
}

IcqUser::~IcqUser()
{
  // Empty
}

User::User(const Licq::UserId& id, bool temporary, bool isOwner)
  : Licq::User(id, temporary, isOwner), IcqUser(id, temporary, isOwner)
{
  mySupportsUtf8 = false;
  mySequence = static_cast<unsigned short>(-1); // set all bits 0xFFFF
  myDirectMode = true;
  myDirectFlag = DirectAnyone;
  myCookie = 0;
  myVersion = 0;
  clearAllSocketDesc();

  if (temporary)
  {
    myNormalSid = 0;
    myInvisibleSid = 0;
    myVisibleSid = 0;
    myGroupSid = 0;
    myBuddyIconType = 0;
    myBuddyIconHashType = 0;
    myClientTimestamp = 0;
    myClientInfoTimestamp = 0;
    myClientStatusTimestamp = 0;
    myOurClientTimestamp = 0;
    myOurClientInfoTimestamp = 0;
    myOurClientStatusTimestamp = 0;
    myPhoneFollowMeStatus = Licq::IcqPluginInactive;
    myIcqPhoneStatus = Licq::IcqPluginInactive;
    mySharedFilesStatus = Licq::IcqPluginInactive;
    return;
  }

  IniFile& conf(userConf());
  conf.setSection("user");

  conf.get("SID", myNormalSid, 0);
  conf.get("InvisibleSID", myInvisibleSid, 0);
  conf.get("VisibleSID", myVisibleSid, 0);
  conf.get("GSID", myGroupSid, 0);
  conf.get("ClientTimestamp", myClientTimestamp, 0);
  conf.get("ClientInfoTimestamp", myClientInfoTimestamp, 0);
  conf.get("ClientStatusTimestamp", myClientStatusTimestamp, 0);
  conf.get("OurClientTimestamp", myOurClientTimestamp, 0);
  conf.get("OurClientInfoTimestamp", myOurClientInfoTimestamp, 0);
  conf.get("OurClientStatusTimestamp", myOurClientStatusTimestamp, 0);
  conf.get("PhoneFollowMeStatus", myPhoneFollowMeStatus, Licq::IcqPluginInactive);
  conf.get("ICQphoneStatus", myIcqPhoneStatus, Licq::IcqPluginInactive);
  conf.get("SharedFilesStatus", mySharedFilesStatus, Licq::IcqPluginInactive);
  conf.get("BuddyIconType", myBuddyIconType, 0);
  conf.get("BuddyIconHashType", myBuddyIconHashType, 0);
  conf.getHex("BuddyIconHash", myBuddyIconHash, "");
  conf.getHex("OurBuddyIconHash", myOurBuddyIconHash, "");

  loadCategory(myInterests, "Interests");
  loadCategory(myBackgrounds, "Backgrounds");
  loadCategory(myOrganizations, "Organizations");

  myPhoneBook.LoadFromDisk(conf);
}

User::~User()
{
  // Empty
}

void User::saveLicqInfo()
{
  IcqUser::saveLicqInfo();

  IniFile& conf(userConf());
  conf.setSection("user");

  conf.set("SID", myNormalSid);
  conf.set("InvisibleSID", myInvisibleSid);
  conf.set("VisibleSID", myVisibleSid);
  conf.set("GSID", myGroupSid);
  conf.set("ClientTimestamp", myClientTimestamp);
  conf.set("ClientInfoTimestamp", myClientInfoTimestamp);
  conf.set("ClientStatusTimestamp", myClientStatusTimestamp);
  conf.set("OurClientTimestamp", myOurClientTimestamp);
  conf.set("OurClientInfoTimestamp", myOurClientInfoTimestamp);
  conf.set("OurClientStatusTimestamp", myOurClientStatusTimestamp);
  conf.set("PhoneFollowMeStatus", myPhoneFollowMeStatus);
  conf.set("ICQphoneStatus", myIcqPhoneStatus);
  conf.set("SharedFilesStatus", mySharedFilesStatus);
}

void User::saveUserInfo()
{
  IcqUser::saveUserInfo();

  IniFile& conf(userConf());
  conf.setSection("user");

  saveCategory(myInterests, "Interests");
  saveCategory(myBackgrounds, "Backgrounds");
  saveCategory(myOrganizations, "Organizations");
  myPhoneBook.SaveToDisk(conf);
}

void User::savePictureInfo()
{
  IcqUser::savePictureInfo();

  IniFile& conf(userConf());
  conf.setSection("user");

  conf.set("BuddyIconType", myBuddyIconType);
  conf.set("BuddyIconHashType", myBuddyIconHashType);
  conf.setHex("BuddyIconHash", myBuddyIconHash);
  conf.setHex("OurBuddyIconHash", myOurBuddyIconHash);
}

void User::saveCategory(const UserCategoryMap& category, const string& key)
{
  IniFile& conf(userConf());
  conf.setSection("user");

  conf.set(key + 'N', category.size());

  Licq::UserCategoryMap::const_iterator i;
  unsigned int count = 0;
  for (i = category.begin(); i != category.end(); ++i)
  {
    char n[10];
    snprintf(n, sizeof(n), "%04X", count);
    conf.set(key + "Cat" + n, i->first);
    conf.set(key + "Desc" + n, i->second);
    ++count;
  }
}

void User::loadCategory(UserCategoryMap& category, const string& key)
{
  IniFile& conf(userConf());
  conf.setSection("user");

  category.clear();
  unsigned int count;
  conf.get(key + 'N', count, 0);

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
    if (!conf.get(key + "Cat" + n, cat))
      continue;

    string descr;
    if (!conf.get(key + "Desc" + n, descr))
      continue;

    category[cat] = descr;
  }
}

void User::SetGSID(unsigned short s)
{
  myGroupSid = s;
  setServerGroup(Licq::gUserManager.getGroupFromServerId(protocolId(), s));
}

unsigned short User::Sequence(bool increment)
{
  if (increment)
    return (mySequence--);
  else
    return mySequence;
}

void User::AddTLV(TlvPtr tlv)
{
  myTLVs[tlv->getType()] = tlv;
}

void User::RemoveTLV(unsigned long type)
{
  myTLVs.erase(type);
}

void User::SetTLVList(TlvList& tlvs)
{
  myTLVs.clear();

  for (TlvList::iterator it = tlvs.begin(); it != tlvs.end(); it++)
    myTLVs[it->first] = it->second;
}

void User::setAlias(const string& alias)
{
  // If there is a valid alias, set the server side list alias as well.
  if (!alias.empty())
  {
    size_t aliasLen = alias.size();
    TlvPtr aliasTLV(new OscarTlv(0x131, aliasLen, alias.c_str()));
    AddTLV(aliasTLV);
  }

  IcqUser::setAlias(alias);
}

bool User::canSendDirect() const
{
  if (myNormalSocketDesc != -1)
    // We already have a connection
    return true;

  if (!isOnline() || InvisibleList())
    // Don't connect to offline user and don't reveal invisible list
    return false;

  return true;
}

string User::internalIpToString() const
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
    return Licq::User::internalIpToString();
}

void User::SetIpPort(unsigned long _nIp, unsigned short _nPort)
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

  Licq::User::SetIpPort(_nIp, _nPort);
}

int User::socketDesc(int channel) const
{
  switch (channel)
  {
    case DcSocket::ChannelNormal:
      return myNormalSocketDesc;
    case DcSocket::ChannelInfo:
      return myInfoSocketDesc;
    case DcSocket::ChannelStatus:
      return myStatusSocketDesc;
  }
  gLog.warning(tr("Unknown channel type %u."), channel);

  return -1;
}

void User::setSocketDesc(DcSocket* s)
{
  if (s->channel() == DcSocket::ChannelNormal)
    myNormalSocketDesc = s->Descriptor();
  else if (s->channel() == DcSocket::ChannelInfo)
    myInfoSocketDesc = s->Descriptor();
  else if (s->channel() == DcSocket::ChannelStatus)
    myStatusSocketDesc = s->Descriptor();
  m_nLocalPort = s->getLocalPort();
  myConnectionVersion = s->Version();
  if (m_bSecure != s->Secure())
  {
    m_bSecure = s->Secure();
    if (m_bOnContactList)
      Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
          Licq::PluginSignal::UserSecurity, myId, m_bSecure ? 1 : 0));
  }

  if (m_nIntIp == 0)
    m_nIntIp = s->getRemoteIpInt();
  if (m_nPort == 0)
    m_nPort = s->getRemotePort();
  SetSendServer(false);
}

void User::clearSocketDesc(Licq::INetSocket* s)
{
  if (s == NULL || s->Descriptor() == myNormalSocketDesc)
    myNormalSocketDesc = -1;
  if (s == NULL || s->Descriptor() == myInfoSocketDesc)
    myInfoSocketDesc = -1;
  if (s == NULL || s->Descriptor() == myStatusSocketDesc)
    myStatusSocketDesc = -1;

  if (myStatusSocketDesc == -1 && myInfoSocketDesc == -1 && myNormalSocketDesc == -1)
  {
    m_nLocalPort = 0;
    myConnectionVersion = 0;

    if (m_bSecure)
    {
      m_bSecure = false;
      if (m_bOnContactList)
        Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
            Licq::PluginSignal::UserSecurity, myId, 0));
    }
  }
}

unsigned short User::ConnectionVersion() const
{
  // If we are already connected, use that version
  if (myConnectionVersion != 0)
    return myConnectionVersion;

  // We aren't connected, see if we know their version
  return IcqProtocol::dcVersionToUse(myVersion);
}
