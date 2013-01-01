/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "licq.h"

#include <boost/foreach.hpp>
#include <boost/scoped_array.hpp>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <list>
#include <map>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <licq/daemon.h>
#include <licq/inifile.h>
#include <licq/logging/log.h>

#include "gettext.h"

using namespace std;
using Licq::IniFile;
using Licq::gDaemon;
using Licq::gLog;

typedef map<string, string> StringMap;


// Helper function to rename a file
static void file_rename(const string& from, const string& to, bool missingok = false)
{
  if (rename(from.c_str(), to.c_str()) == 0)
    return;

  if (missingok && errno == ENOENT)
    return;

  gLog.error(tr("Failed to move file from '%s' to '%s': %s"),
      from.c_str(), to.c_str(), strerror(errno));
  throw exception();
}

// Helper function to copy a file
// Currently hardcoded for how Picture files are copied, extend only if needed
static void file_copy(const string& from, const string& to) throw()
{
  int fromFd = open(from.c_str(), O_RDONLY);
  if (fromFd < 0)
    return; // No fault if owner.pic is missing

  // Same mode bits as in contactlist/owner.cpp
  int toFd = open(to.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 00664);
  if (toFd < 0)
  {
    gLog.error(tr("Failed to create '%s': %s"), to.c_str(), strerror(errno));
    throw exception();
  }

  while (1)
  {
    char buf[8192];
    ssize_t sr = read(fromFd, buf, sizeof(buf));
    if (sr == 0) // End of file
      break;
    else if (sr > 0) // Read ok
    {
      ssize_t sw = write(toFd, buf, sr);
      if (sw  == sr) // Write ok
        continue;
      else if (sw < 0) // Write failed
        gLog.error(tr("Failed to write to '%s': %s"), to.c_str(), strerror(errno));
      else // Write too short
        gLog.error(tr("Failed to write %zi bytes to '%s'"), sr, to.c_str());
    }
    else if (sr < 0) // Read failed
      gLog.error(tr("Failed to read from '%s': %s"), from.c_str(), strerror(errno));

    // Copy failed
    close(toFd);
    close(fromFd);
    throw exception();
  }

  // Copy complete
  close(toFd);
  close(fromFd);
}


/*-----------------------------------------------------------------------------
 * Upgrade for Licq 1.2.8
 *
 * - Add support for protocol plugins and multiple owners
 * - Rename user, owner and history files to support multiple protocols
 *---------------------------------------------------------------------------*/
void CLicq::upgradeLicq128(IniFile& licqConf)
{
  gLog.info(tr("Upgrading config file formats for multiple protocols"));

  string strBaseDir = gDaemon.baseDir();
  IniFile ownerFile("owner.uin");
  if (!ownerFile.loadFile())
    throw exception();

  // Get the UIN
  unsigned long nUin;
  ownerFile.setSection("user");
  ownerFile.get("Uin", nUin, 0);

  // Create the owner section and fill it
  licqConf.setSection("owners");
  licqConf.set("NumOfOwners", 1);
  licqConf.set("Owner1.Id", nUin);
  licqConf.set("Owner1.PPID", "Licq");

  // Add the protocol plugins info
  licqConf.setSection("plugins");
  licqConf.set(string("NumProtoPlugins"), 0);

  // Rename owner.uin to owner.Licq
  file_rename(strBaseDir + "owner.uin", strBaseDir + "owner.Licq");

  // Update all the user files and update users.conf
  string strUserDir = strBaseDir + "users";
  DIR* userDir = opendir(strUserDir.c_str());
  if (userDir != NULL)
  {
    IniFile userConf("users.conf");
    if (!userConf.loadFile())
      throw exception();
    userConf.setSection("users");
    int n = 0;

    boost::scoped_array<char> ent(new char[offsetof(struct dirent, d_name) +
        pathconf(strUserDir.c_str(), _PC_NAME_MAX) + 1]);
    struct dirent* res;

    while (readdir_r(userDir, (struct dirent*)ent.get(), &res) == 0 && res != NULL)
    {
      const char* dot = strrchr(res->d_name, '.');
      if (dot == NULL || strcmp(dot, ".uin") != 0)
        continue;

      char szKey[20];
      snprintf(szKey, sizeof(szKey), "User%d", n+1);
      string strFileName = strUserDir + "/" + res->d_name;
      string strNewName = res->d_name;
      strNewName.replace(strNewName.find(".uin", 0), 4, ".Licq");
      string strNewFile = strUserDir + "/" + strNewName;
      file_rename(strFileName, strNewFile);
      userConf.set(szKey, strNewName);
      ++n;
    }
    userConf.set("NumOfUsers", n);
    userConf.writeFile();
    closedir(userDir);
  }

  // Rename the history files
  string strHistoryDir = strBaseDir + "history";
  DIR* historyDir = opendir(strHistoryDir.c_str());
  if (historyDir != NULL)
  {
    boost::scoped_array<char> ent(new char[offsetof(struct dirent, d_name) +
        pathconf(strHistoryDir.c_str(), _PC_NAME_MAX) + 1]);
    struct dirent* res;

    while (readdir_r(historyDir, (struct dirent*)ent.get(), &res) == 0 && res != NULL)
    {
      const char* dot = strrchr(res->d_name, '.');
      if (dot == NULL || (strcmp(dot, ".history") != 0 && strcmp(dot, ".history.removed") != 0))
        continue;

      string strFileName = strHistoryDir + "/" + res->d_name;
      string strNewFile = strHistoryDir + "/" + res->d_name;
      strNewFile.replace(strNewFile.find(".history", 0), 8, ".Licq.history");
      file_rename(strFileName, strNewFile);
    }
    closedir(historyDir);
  }
  gLog.info(tr("Upgrade completed"));
}

/******************************************************************************/
/* Functions used for upgrade to Licq 1.8.0                                   */
/******************************************************************************/

/*-----------------------------------------------------------------------------
 * Move user data files into owner specific directories
 *---------------------------------------------------------------------------*/
static void upgradeLicq18_moveUserData(const string& baseDir, StringMap& owners, StringMap& userDirs)
{
  string userDir = baseDir + "users";
  DIR* userDirList = opendir(userDir.c_str());
  if (userDirList == NULL)
    throw std::exception(); // Should never happen
  boost::scoped_array<char> ent(new char[offsetof(struct dirent, d_name) +
      pathconf(userDir.c_str(), _PC_NAME_MAX) + 1]);
  struct dirent* res;
  while (readdir_r(userDirList, (struct dirent*)ent.get(), &res) == 0 && res != NULL)
  {
    // Ignore files that doesn't match the naming standard for user files
    size_t namelen = strlen(res->d_name);
    if (namelen < 6 || res->d_name[namelen-5] != '.')
      continue;

    string accountId(res->d_name, namelen-5);
    string ppidStr(res->d_name + namelen-4);

    // Skip if the file doesn't belong to any known owner
    if (owners.count(ppidStr) == 0)
      continue;

    // Don't try and move the owner directory into itself
    if (accountId == owners[ppidStr])
      continue;

    string oldUserFile = userDir + "/" + res->d_name;
    string newUserFile = userDirs[ppidStr] + "/" + accountId + ".conf";
    file_rename(oldUserFile, newUserFile);

    // If there is a user picture file, move that too
    string oldPicFile = userDir + "/" + accountId + ".pic";
    string newPicFile = userDirs[ppidStr] + "/" + accountId + ".pic";
    file_rename(oldPicFile, newPicFile, true);
  }
  closedir(userDirList);
}

/*-----------------------------------------------------------------------------
 * Move owner data files into directories
 *---------------------------------------------------------------------------*/
static void upgradeLicq18_moveOwnerData(const string& baseDir, StringMap& owners, StringMap& userDirs)
{
  string oldOwnerPicFile = baseDir + "owner.pic";
  for (StringMap::const_iterator iter = owners.begin(); iter != owners.end(); ++iter)
  {
    const string& ppidStr = iter->first;
    string oldOwnerFile = baseDir + "owner." + ppidStr;
    string newOwnerFile = userDirs[ppidStr] + "/" + iter->second + ".conf";
    file_rename(oldOwnerFile, newOwnerFile);

    // If there is an owner picture file, copy that too (old file is shared for all owners)
    string newPicFile = userDirs[ppidStr] + "/" + iter->second + ".pic";
    file_copy(oldOwnerPicFile, newPicFile);
  }

  // Remove the old picture file (if it exists)
  unlink(oldOwnerPicFile.c_str());
}

/*-----------------------------------------------------------------------------
 * Move history files into users directory
 *---------------------------------------------------------------------------*/
static void upgradeLicq18_moveHistoryFiles(const string& baseDir, StringMap& owners, StringMap& userDirs)
{
  string historyDir = baseDir + "history";
  DIR* historyDirList = opendir(historyDir.c_str());
  if (historyDirList == NULL)
    // History dir is created when first entry is logged so it's not an error if it's missing
    return;

  boost::scoped_array<char> ent(new char[offsetof(struct dirent, d_name) +
      pathconf(historyDir.c_str(), _PC_NAME_MAX) + 1]);
  struct dirent* res;
  while (readdir_r(historyDirList, (struct dirent*)ent.get(), &res) == 0 && res != NULL)
  {
    size_t namelen = strlen(res->d_name);

    string accountId;
    string ppidStr;
    if (namelen > 13 && strcmp(res->d_name + namelen-8, ".history") == 0)
    {
      // Normal history file
      accountId = string(res->d_name, namelen-13);
      ppidStr = string(res->d_name + namelen-12, 4);
    }
    else if (namelen > 21 && strcmp(res->d_name + namelen-16, ".history.removed") == 0)
    {
      // Removed history file, bring those along as well but drop suffix
      accountId = string(res->d_name, namelen-21);
      ppidStr = string(res->d_name + namelen-20, 4);
    }
    else
      continue;

    // Skip if the file doesn't belong to any known owner
    if (owners.count(ppidStr) == 0)
      continue;

    // Skip if file is named exactly as owner (Owner history is prefixed with "owner.")
    if (accountId == owners[ppidStr])
      continue;

    // Drop prefix from owner history
    if (accountId == "owner." + owners[ppidStr])
      accountId = owners[ppidStr];

    string oldHistoryFile = historyDir + "/" + res->d_name;
    string newHistoryFile = userDirs[ppidStr] + "/" + accountId + ".history";
    file_rename(oldHistoryFile, newHistoryFile);
  }
  closedir(historyDirList);

  // If history dir is empty, remove it
  rmdir(historyDir.c_str());
}

/*-----------------------------------------------------------------------------
 * Migrate protocol config to owner data
 *---------------------------------------------------------------------------*/
static void upgradeLicq18_migrateOwnerConfig(const string& baseDir, StringMap& owners, StringMap& userDirs, IniFile& licqConf)
{
  string oldMsnConfFile = baseDir + "licq_msn.conf";
  string oldJabberConfFile = baseDir + "licq_jabber.conf";

  for (StringMap::const_iterator iter = owners.begin(); iter != owners.end(); ++iter)
  {
    // Migrate old protocol config into owner data
    const string& ppidStr = iter->first;
    string newOwnerFile = userDirs[ppidStr] + "/" + iter->second + ".conf";
    IniFile ownerConf(newOwnerFile);
    ownerConf.loadFile();
    ownerConf.setSection("user");
    if (ppidStr == "Licq")
    {
      // Old ICQ config is in licq.conf
      licqConf.setSection("network");

      // Login server (moved by Licq 1.6.0), clear if set to defaults
      string serverHost;
      if (!ownerConf.get("ServerHost", serverHost))
        licqConf.get("ICQServer", serverHost);
      ownerConf.set("ServerHost", (serverHost == "login.icq.com" ? "" : serverHost));
      licqConf.unset("ICQServer");
      int serverPort;
      if (!ownerConf.get("ServerPort", serverPort))
        licqConf.get("ICQServerPort", serverPort);
      ownerConf.set("ServerPort", (serverPort == 5190 ? 0 : serverPort));
      licqConf.unset("ICQServerPort");

      // ICQ specific parameters (moved by Licq 1.7.0)
      bool b;
      if (!ownerConf.get("AutoUpdateInfo", b))
        licqConf.get("AutoUpdateInfo", b, true);
      ownerConf.set("AutoUpdateInfo", b);
      licqConf.unset("AutoUpdateInfo");
      if (!ownerConf.get("AutoUpdateInfoPlugins", b))
        licqConf.get("AutoUpdateInfoPlugins", b, true);
      ownerConf.set("AutoUpdateInfoPlugins", b);
      licqConf.unset("AutoUpdateInfoPlugins");
      if (!ownerConf.get("AutoUpdateStatusPlugins", b))
        licqConf.get("AutoUpdateStatusPlugins", b, true);
      ownerConf.set("AutoUpdateStatusPlugins", b);
      licqConf.unset("AutoUpdateStatusPlugins");
      if (!ownerConf.get("UseSS", b))
        licqConf.get("UseSS", b, true);
      ownerConf.set("UseSS", b);
      licqConf.unset("UseSS");
      if (!ownerConf.get("UseBART", b))
        licqConf.get("UseBART", b, true);
      ownerConf.set("UseBART", b);
      licqConf.unset("UseBART");
      if (!ownerConf.get("ReconnectAfterUinClash", b))
        licqConf.get("ReconnectAfterUinClash", b, true);
      ownerConf.set("ReconnectAfterUinClash", b);
      licqConf.unset("ReconnectAfterUinClash");
    }
    else if (ppidStr == "MSN_")
    {
      // Old MSN config is in separate file
      IniFile msnConf(oldMsnConfFile);
      msnConf.loadFile();
      msnConf.setSection("network");

      // Login server (moved by Licq 1.6.0), clear if set to defaults
      string serverHost;
      if (!ownerConf.get("ServerHost", serverHost))
        msnConf.get("MsnServerAddress", serverHost);
      ownerConf.set("ServerHost", (serverHost == "messenger.hotmail.com" ? "" : serverHost));
      int serverPort;
      if (!ownerConf.get("ServerPort", serverPort))
        msnConf.get("MsnServerPort", serverPort);
      ownerConf.set("ServerPort", (serverPort == 1863 ? 0 : serverPort));

      // MSN specific parameters (moved by Licq 1.7.0)
      unsigned long listVersion;
      if (!ownerConf.get("ListVersion", listVersion))
        msnConf.get("ListVersion", listVersion, 0);
      ownerConf.set("ListVersion", listVersion);
    }
    else if (ppidStr == "XMPP")
    {
      // Old Jabber config is in separate file
      IniFile jabberConf(oldJabberConfFile);
      jabberConf.loadFile();
      jabberConf.setSection("network");

      // Login server (moved by Licq 1.6.0)
      string serverHost;
      if (!ownerConf.get("ServerHost", serverHost))
        jabberConf.get("Server", serverHost);
      ownerConf.set("ServerHost", serverHost);
      int serverPort;
      if (!ownerConf.get("ServerPort", serverPort))
        jabberConf.get("Port", serverPort);
      ownerConf.set("ServerPort", serverPort);

      // Jabber specific parameters (moved by Licq 1.7.0)
      string str;
      if (!ownerConf.get("JabberTlsPolicy", str))
        jabberConf.get("TlsPolicy", str, "optional");
      ownerConf.set("JabberTlsPolicy", str);
      if (!ownerConf.get("JabberResource", str))
        jabberConf.get("Resource", str, "Licq");
      ownerConf.set("JabberResource", str);
    }
    ownerConf.writeFile();
  }

  // Remove the replaced files (if they exist)
  unlink(oldMsnConfFile.c_str());
  unlink(oldJabberConfFile.c_str());
}

/*-----------------------------------------------------------------------------
 * Separate users.conf into sections per owner
 *---------------------------------------------------------------------------*/
static void upgradeLicq18_updateUsersList(StringMap& owners)
{
  IniFile usersConf("users.conf");
  usersConf.loadFile();
  for (StringMap::const_iterator iter = owners.begin(); iter != owners.end(); ++iter)
  {
    // Check if owner specific section already exists (introduced by Licq 1.7.0)
    string newSection = iter->second + "." + iter->first;
    if (usersConf.setSection(newSection, false))
      continue;

    list<string> users;
    {
      // Migrate from old mixed section
      usersConf.setSection("users");
      int numUsers;
      usersConf.get("NumOfUsers", numUsers, 0);
      for (int i = 1; i <= numUsers; ++i)
      {
        char key[20];
        sprintf(key, "User%i", i);
        string userIdStr;
        usersConf.get(key, userIdStr);
        if (userIdStr.size() < 6 || userIdStr[userIdStr.size()-5] != '.')
          continue;
        string ppidStr = userIdStr.substr(userIdStr.size()-4);
        if (ppidStr == iter->first)
          users.push_back(userIdStr.substr(0, userIdStr.size()-5));
      }
    }

    // Write users to new section
    usersConf.setSection(newSection);
    usersConf.set("NumUsers", users.size());
    int i = 0;
    BOOST_FOREACH(const string& userIdStr, users)
    {
      char key[20];
      sprintf(key, "User%i", ++i);
      usersConf.set(key, userIdStr);
    }
  }
  usersConf.removeSection("users");
  usersConf.writeFile();
}

/*-----------------------------------------------------------------------------
 * Migrate onevent configuration
 *---------------------------------------------------------------------------*/
static void upgradeLicq18_updateOnevent(StringMap& owners, IniFile& licqConf)
{
  IniFile onEventConf("onevent.conf");
  if (onEventConf.loadFile())
  {
    // File exists, find all User sections and add owner and protocol to them
    list<string> sections;
    onEventConf.getSections(sections, "User.");
    BOOST_FOREACH(const string& section, sections)
    {
      // Migrate user section
      // Note: Section name isn't updated, but it only needs to be unique and is never parsed
      onEventConf.setSection(section);
      string userIdStr;
      onEventConf.get("User", userIdStr);
      if (userIdStr.size() < 5)
        continue;
      string accountId = userIdStr.substr(4);
      string ppidStr = userIdStr.substr(0, 4);
      if (owners.count(ppidStr) == 0)
        continue;
      onEventConf.set("Protocol", ppidStr);
      onEventConf.set("Owner", owners[ppidStr]);
      onEventConf.set("User", accountId);
    }
  }
  else
  {
    // onevent.conf doesn't exist (introduced by Licq 1.5.0), migrate from licq.conf
    licqConf.setSection("onevent");
    onEventConf.setSection("global");
    string str;
    if (licqConf.get("Enable", str))             onEventConf.set("Enable", str);
    if (licqConf.get("AlwaysOnlineNotify", str)) onEventConf.set("AlwaysOnlineNotify", str);
    if (licqConf.get("Command", str))            onEventConf.set("Command", str);
    if (licqConf.get("Message", str))            onEventConf.set("Message", str);
    if (licqConf.get("Url", str))                onEventConf.set("Url", str);
    if (licqConf.get("Chat", str))               onEventConf.set("Chat", str);
    if (licqConf.get("File", str))               onEventConf.set("File", str);
    if (licqConf.get("Sms", str))                onEventConf.set("Sms", str);
    if (licqConf.get("OnlineNotify", str))       onEventConf.set("OnlineNotify", str);
    if (licqConf.get("SysMsg", str))             onEventConf.set("SysMsg", str);
    if (licqConf.get("MsgSent", str))            onEventConf.set("MsgSent", str);
    licqConf.removeSection("onevent");
  }
  onEventConf.writeFile();
}


/*-----------------------------------------------------------------------------
 * Update file structure for Licq 1.8.0
 *
 * - Add owner as directory level to allow multiple owners per protocol
 * - Move history files together with user data files
 * - Move parameters from protocol configurations to owner data
 * - Add owner account id everywhere user id is saved
 *---------------------------------------------------------------------------*/
void CLicq::upgradeLicq18(IniFile& licqConf)
{
  gLog.info(tr("Upgrading config file formats for multiple accounts"));

  const string& baseDir(gDaemon.baseDir());

  // Prepare a list of configured owners and create directories for user data
  StringMap owners;
  StringMap userDirs;
  licqConf.setSection("owners");
  int numOwners;
  licqConf.get("NumOfOwners", numOwners, 0);
  for (int i = 1; i <= numOwners; ++i)
  {
    char key[20];
    string accountId, ppidStr;
    sprintf(key, "Owner%d.Id", i);
    licqConf.get(key, accountId);
    sprintf(key, "Owner%d.PPID", i);
    licqConf.get(key, ppidStr);
    if (accountId.empty() || ppidStr.empty())
    {
      gLog.error(tr("Missing data for owner %i"), i);
      throw exception();
    }

    licqConf.unset(key);
    sprintf(key, "Owner%d.Protocol", i);
    licqConf.set(key, ppidStr);

    // Create a users directory for each owner
    string dirname = baseDir + "users/" + accountId + "." + ppidStr;
    if (mkdir(dirname.c_str(), 0700) < 0)
    {
      gLog.error(tr("Failed to create directory %s: %s"), dirname.c_str(), strerror(errno));
      throw exception();
    }

    // Save owners and user directories in map for use during the migration
    owners[ppidStr] = accountId;
    userDirs[ppidStr] = dirname;
  }

  // Clean out old owner entries
  for (int i = numOwners+1; ; ++i)
  {
    char key[14];
    sprintf(key, "Owner%d.", i);
    if (!licqConf.unset(string(key) + "Id") & !licqConf.unset(string(key) + "PPID"))
      break;
  }

  // Call each upgrade function
  upgradeLicq18_moveUserData(baseDir, owners, userDirs);
  upgradeLicq18_moveOwnerData(baseDir, owners, userDirs);
  upgradeLicq18_moveHistoryFiles(baseDir, owners, userDirs);
  upgradeLicq18_migrateOwnerConfig(baseDir, owners, userDirs, licqConf);
  upgradeLicq18_updateUsersList(owners);
  upgradeLicq18_updateOnevent(owners, licqConf);

  gLog.info(tr("Upgrade completed"));
}
