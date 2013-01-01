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

#include <boost/scoped_array.hpp>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <errno.h>
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
