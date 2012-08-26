/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2012 Licq developers <licq-dev@googlegroups.com>
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

#include <licq/contactlist/owner.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <licq/inifile.h>

#include "../daemon.h"
#include "gettext.h"
#include <licq/logging/log.h>

using std::string;
using namespace Licq;


Owner::Owner(const UserId& id)
  : User(id, false, true)
{
  m_bOnContactList = true;

  m_bSavePassword = true;
  myPassword = "";

  myPictureFileName = gDaemon.baseDir() + "owner.pic";

  IniFile& conf(userConf());

  // Make sure config file is mode 0600
  string filename = gDaemon.baseDir() + conf.filename();
  if (chmod(filename.c_str(), S_IRUSR | S_IWUSR) == -1)
  {
    gLog.warning(tr("Unable to set %s to mode 0600. Your password is vulnerable if stored locally."),
        filename.c_str());
  }

  // Owner encoding fixup to be UTF-8 by default
  if (myEncoding.empty())
    myEncoding = "UTF-8";
  conf.get("Password", myPassword, "");
  conf.get("AutoResponse", myAutoResponse, "");
  string statusStr;
  conf.get("StartupStatus", statusStr, "");
  if (!User::stringToStatus(statusStr, myStartupStatus))
    myStartupStatus = User::OfflineStatus;

  bool gotserver = false;
  if (conf.get("ServerHost", myServerHost))
    gotserver = true;
  if (conf.get("ServerPort", myServerPort))
    gotserver = true;

  if (!gotserver)
  {
    // Server parameters are missing, this could be due to upgrade from Licq 1.5.x or older
    // Try to migrate from protocol specific config file
    switch (myId.protocolId())
    {
      case LICQ_PPID:
      {
        Licq::IniFile& oldConf(LicqDaemon::gDaemon.getLicqConf());
        oldConf.setSection("network");
        oldConf.get("ICQServer", myServerHost);
        oldConf.get("ICQServerPort", myServerPort);
        LicqDaemon::gDaemon.releaseLicqConf();
        break;
      }
      case MSN_PPID:
      {
        Licq::IniFile oldConf("licq_msn.conf");
        oldConf.loadFile();
        oldConf.setSection("network");
        oldConf.get("MsnServerAddress", myServerHost);
        oldConf.get("MsnServerPort", myServerPort);
        break;
      }
      case JABBER_PPID:
      {
        Licq::IniFile oldConf("licq_jabber.conf");
        oldConf.loadFile();
        oldConf.setSection("network");
        oldConf.get("Server", myServerHost);
        oldConf.get("Port", myServerPort);
        break;
      }
    }
  }

  gLog.info(tr("Loading owner configuration for %s"), myId.toString().c_str());

  int systz = systemTimezone();
  int mytz = timezone();
  if (mytz != TimezoneUnknown && mytz != systz)
  {
    gLog.warning(tr("Current Licq GMT offset (%c%i:%02i) does not match system GMT offset (%c%i:%02i).\n"
                    "Update general info on server to fix."),
        (mytz >= 0 ? '+' : '-'), abs(mytz / 3600), abs(mytz / 60) % 60,
        (systz >= 0 ? '+' : '-'), abs(systz / 3600), abs(systz / 60) % 60);
  }
  SetEnableSave(true);
}

Owner::~Owner()
{
  // Empty
}

void Owner::saveOwnerInfo()
{
  IniFile& conf(userConf());

  conf.set("Authorization", GetAuthorization());
  conf.set("StartupStatus", User::statusToString(myStartupStatus));
  conf.set("ServerHost", myServerHost);
  conf.set("ServerPort", myServerPort);
  conf.set("AutoResponse", myAutoResponse);

  if (m_bSavePassword)
    conf.set("Password", myPassword);
  else
    conf.set("Password", "");
}

void Licq::Owner::SetPicture(const char *f)
{
  string filename = pictureFileName();
  if (f == NULL)
  {
    SetPicturePresent(false);
    if (remove(filename.c_str()) != 0 && errno != ENOENT)
    {
      gLog.error("Unable to delete %s's picture file (%s): %s",
          myAlias.c_str(), filename.c_str(), strerror(errno));
    }
  }
  else if (strcmp(f, filename.c_str()) == 0)
  {
    SetPicturePresent(true);
    return;
  }
  else
  {
    int source = open(f, O_RDONLY);
    if (source == -1)
    {
      gLog.error("Unable to open source picture file (%s): %s",
          f, strerror(errno));
      return;
    }

    int dest = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 00664);
    if (dest == -1)
    {
      gLog.error("Unable to open picture file (%s): %s",
          filename.c_str(), strerror(errno));
      close(source);
      return;
    }

    char buf[8192];
    ssize_t s;
    while (1)
    {
      s = read(source, buf, sizeof(buf));
      if (s == 0)
      {
        SetPicturePresent(true);
        break;
      }
      else if (s == -1)
      {
        gLog.error("Error reading from %s: %s", f, strerror(errno));
        SetPicturePresent(false);
        break;
      }

      if (write(dest, buf, s) != s)
      {
        gLog.error("Error writing to %s: %s", f, strerror(errno));
        SetPicturePresent(false);
        break;
      }
    }

    close(source);
    close(dest);
  }
}
