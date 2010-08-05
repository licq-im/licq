/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#include "owner.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <licq/daemon.h>
#include <licq/icqdefines.h>

#include "gettext.h"
#include <licq/logging/log.h>

using std::string;
using Licq::UserId;
using Licq::gDaemon;
using Licq::gLog;
using namespace LicqDaemon;


Owner::Owner(const UserId& id)
  : User(id, true)
{
  // Pretend to be temporary to LicqUser constructior so it doesn't setup myConf
  // Restore NotInList flag to proper value when we get here
  m_bNotInList = false;
  m_bOnContactList = true;

  m_bException = false;
  m_bSavePassword = true;
  myPassword = "";
  myPDINFO = 0;

  myPictureFileName = gDaemon.baseDir() + "owner.pic";

  // Get data from the config file
  char p[5];
  Licq::protocolId_toStr(p, myId.protocolId());
  string filename = "owner.";
  filename += p;
  myConf.setFilename(filename);
  myConf.loadFile();
  myConf.setSection("user");
  myConf.writeFile();

  // Make sure config file is mode 0600
  filename = gDaemon.baseDir() + filename;
  if (chmod(filename.c_str(), S_IRUSR | S_IWUSR) == -1)
  {
    gLog.warning(tr("Unable to set %s to mode 0600. Your password is vulnerable if stored locally."),
        filename.c_str());
  }

  // And finally our favorite function
  LoadInfo();
  // Owner encoding fixup to be UTF-8 by default
  if (myEncoding.empty())
    myEncoding = "UTF-8";
  myConf.get("Password", myPassword, "");
  myConf.get("WebPresence", m_bWebAware, false);
  myConf.get("HideIP", m_bHideIp, false);
  myConf.get("RCG", m_nRandomChatGroup, ICQ_RANDOMxCHATxGROUP_NONE);
  myConf.get("AutoResponse", myAutoResponse, "");
  string statusStr;
  myConf.get("StartupStatus", statusStr, "");
  if (!User::stringToStatus(statusStr, myStartupStatus))
    myStartupStatus = User::OfflineStatus;
  unsigned long sstime;
  myConf.get("SSTime", sstime, 0);
  m_nSSTime = sstime;
  myConf.get("SSCount", mySsCount, 0);
  myConf.get("PDINFO", myPDINFO, 0);

  gLog.info(tr("Loading owner configuration for %s"), myId.toString().c_str());

  setHistoryFile(gDaemon.baseDir() + HistoryDir + "owner." + myId.accountId() +
      "." + p + HistoryExt);

  if (m_nTimezone != SystemTimezone() && m_nTimezone != TimezoneUnknown)
  {
    gLog.warning(tr("Current Licq GMT offset (%d) does not match system GMT offset (%d).\n"
                    "Update general info on server to fix."),
                 m_nTimezone, SystemTimezone());
  }
  SetEnableSave(true);
}

Owner::~Owner()
{
  // Save the current auto response
  if (!myConf.loadFile())
  {
     gLog.error("Error opening '%s' for reading. See log for details.",
         myConf.filename().c_str());
     return;
  }
  myConf.setSection("user");
  myConf.set("AutoResponse", myAutoResponse);
  myConf.set("SSTime", (unsigned long)m_nSSTime);
  myConf.set("SSCount", mySsCount);
  myConf.set("PDINFO", myPDINFO);
  if (!myConf.writeFile())
  {
    gLog.error("Error opening '%s' for writing. See log for details.",
        myConf.filename().c_str());
    return;
  }
}

unsigned long Licq::Owner::AddStatusFlags(unsigned long s) const
{
  s &= 0x0000FFFF;

  if (WebAware())
    s |= ICQ_STATUS_FxWEBxPRESENCE;
  if (HideIp())
    s |= ICQ_STATUS_FxHIDExIP;
  if (Birthday() == 0)
    s |= ICQ_STATUS_FxBIRTHDAY;
  if (PhoneFollowMeStatus() != ICQ_PLUGIN_STATUSxINACTIVE)
    s |= ICQ_STATUS_FxPFM;
  if (PhoneFollowMeStatus() == ICQ_PLUGIN_STATUSxACTIVE)
    s |= ICQ_STATUS_FxPFMxAVAILABLE;

  return s;
}

void Owner::SaveLicqInfo()
{
  if (!EnableSave()) return;

  User::SaveLicqInfo();

  if (!myConf.loadFile())
  {
     gLog.error("Error opening '%s' for reading. See log for details.",
         myConf.filename().c_str());
     return;
  }
  myConf.setSection("user");
  myConf.set("Uin", accountId());
  myConf.set("WebPresence", WebAware());
  myConf.set("HideIP", HideIp());
  myConf.set("Authorization", GetAuthorization());
  myConf.set("StartupStatus", User::statusToString(myStartupStatus));
  myConf.set("RCG", RandomChatGroup());
  myConf.set("SSTime", (unsigned long)m_nSSTime);
  myConf.set("SSCount", mySsCount);
  myConf.set("PDINFO", myPDINFO);

  if (m_bSavePassword)
    myConf.set("Password", myPassword);
  else
    myConf.set("Password", "");

  if (!myConf.writeFile())
  {
    gLog.error("Error opening '%s' for writing. See log for details.",
        myConf.filename().c_str());
    return;
  }
}

void Licq::Owner::SetStatusOffline()
{
  SetStatus(m_nStatus | ICQ_STATUS_OFFLINE);
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
