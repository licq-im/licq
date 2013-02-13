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
  : User(id, false)
{
  m_bOnContactList = true;

  m_bSavePassword = true;
  myPassword = "";

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
  conf.get("ServerHost", myServerHost);
  conf.get("ServerPort", myServerPort);

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
    deletePictureData();
  }
  else if (filename == f)
  {
    SetPicturePresent(true);
  }
  else
  {
    int source = open(f, O_RDONLY);
    if (source == -1)
    {
      gLog.error(tr("Unable to open source picture file (%s): %s"),
          f, strerror(errno));
      return;
    }

    int dest = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 00664);
    if (dest == -1)
    {
      gLog.error(tr("Unable to open picture file (%s): %s"),
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
        gLog.error(tr("Error reading from %s: %s"), f, strerror(errno));
        SetPicturePresent(false);
        break;
      }

      if (write(dest, buf, s) != s)
      {
        gLog.error(tr("Error writing to %s: %s"), f, strerror(errno));
        SetPicturePresent(false);
        break;
      }
    }

    close(source);
    close(dest);
  }
}
