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

#include "owner.h"

#include <licq/inifile.h>
#include <licq/logging/log.h>

using namespace LicqIcq;
using Licq::IcqOwner;
using Licq::IniFile;
using Licq::gLog;

IcqOwner::IcqOwner(const Licq::UserId& id)
  : Licq::User(id, false, true), Licq::Owner(id)
{
  // Empty
}

IcqOwner::~IcqOwner()
{
  // Empty
}

Owner::Owner(const Licq::UserId& id)
  : Licq::User(id, false, true), Licq::IcqOwner(id)
{
  IniFile& conf(userConf());
  conf.setSection("user");

  conf.get("WebPresence", m_bWebAware, false);
  conf.get("HideIP", m_bHideIp, false);
  conf.get("RCG", myRandomChatGroup, 0);

  unsigned long sstime;
  conf.get("SSTime", sstime, 0);
  m_nSSTime = sstime;
  conf.get("SSCount", mySsCount, 0);
  conf.get("PDINFO", myPDINFO, 0);
}

Owner::~Owner()
{
  IniFile& conf(userConf());

  // Save the current auto response
  if (!conf.loadFile())
  {
     gLog.error("Error opening '%s' for reading. See log for details.",
         conf.filename().c_str());
     return;
  }
  conf.setSection("user");
  conf.set("SSTime", (unsigned long)m_nSSTime);
  conf.set("SSCount", mySsCount);
  conf.set("PDINFO", myPDINFO);
  if (!conf.writeFile())
  {
    gLog.error("Error opening '%s' for writing. See log for details.",
        conf.filename().c_str());
    return;
  }
}

void Owner::saveOwnerInfo()
{
  IcqOwner::saveOwnerInfo();

  IniFile& conf(userConf());

  conf.set("WebPresence", WebAware());
  conf.set("HideIP", HideIp());
  conf.set("RCG", myRandomChatGroup);
  conf.set("SSTime", (unsigned long)m_nSSTime);
  conf.set("SSCount", mySsCount);
  conf.set("PDINFO", myPDINFO);
}
