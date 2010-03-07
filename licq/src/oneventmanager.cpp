/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#include "oneventmanager.h"

#include <licq_constants.h>
#include <licq_icq.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>

#include <cstdlib> // system, free
#include <ctime> // time

using namespace std;
using namespace LicqDaemon;
using Licq::gUserManager;


// Declare global OnEventManager (internal for daemon)
LicqDaemon::OnEventManager LicqDaemon::gOnEventManager;

// Declare global Licq::OnEventManager to refer to the internal OnEventManager
Licq::OnEventManager& Licq::gOnEventManager(LicqDaemon::gOnEventManager);


OnEventManager::OnEventManager()
  : myEnabled(true)
{
  // Empty
}

OnEventManager::~OnEventManager()
{
  // Empty
}

void OnEventManager::initialize()
{
  string filename = BASE_DIR;
  filename += "licq.conf";
  CIniFile licqConf(INI_FxERROR | INI_FxFATAL);
  licqConf.LoadFile(filename.c_str());
  licqConf.SetFlags(0);

  string soundDir = SHARE_DIR;
  soundDir += "sounds/icq/";

  licqConf.SetSection("onevent");
  licqConf.ReadBool("Enable", myEnabled, true);
  licqConf.ReadBool("AlwaysOnlineNotify", myAlwaysOnlineNotify, false);
  licqConf.readString("Command", myCommand, "play");
  licqConf.readString("Message", myParameters[OnEventMessage], soundDir + "Message.wav");
  licqConf.readString("Url", myParameters[OnEventUrl], soundDir + "URL.wav");
  licqConf.readString("Chat", myParameters[OnEventChat], soundDir + "Chat.wav");
  licqConf.readString("File", myParameters[OnEventFile], soundDir + "File.wav");
  licqConf.readString("OnlineNotify", myParameters[OnEventOnline], soundDir + "Online.wav");
  licqConf.readString("SysMsg", myParameters[OnEventSysMsg], soundDir + "System.wav");
  licqConf.readString("MsgSent", myParameters[OnEventMsgSent], soundDir + "Message.wav");
  licqConf.readString("Sms", myParameters[OnEventSms], soundDir + "Message.wav");
}

void OnEventManager::lock()
{
  myMutex.lock();
}

void OnEventManager::unlock(bool save)
{
  if (save)
  {
    string filename = BASE_DIR;
    filename += "licq.conf";
    CIniFile licqConf(INI_FxERROR | INI_FxALLOWxCREATE);
    if (!licqConf.LoadFile(filename.c_str()))
      return;

    licqConf.SetSection("onevent");
    licqConf.WriteBool("Enable", myEnabled);
    licqConf.WriteBool("AlwaysOnlineNotify", myAlwaysOnlineNotify);
    licqConf.writeString("Command", myCommand);
    licqConf.writeString("Message", myParameters[OnEventMessage]);
    licqConf.writeString("Url", myParameters[OnEventUrl]);
    licqConf.writeString("Chat", myParameters[OnEventChat]);
    licqConf.writeString("File", myParameters[OnEventFile]);
    licqConf.writeString("OnlineNotify", myParameters[OnEventOnline]);
    licqConf.writeString("SysMsg", myParameters[OnEventSysMsg]);
    licqConf.writeString("MsgSent", myParameters[OnEventMsgSent]);
    licqConf.writeString("Sms", myParameters[OnEventSms]);
    licqConf.FlushFile();
  }

  myMutex.unlock();
}

bool OnEventManager::enabled() const
{
  return myEnabled;
}

void OnEventManager::setEnabled(bool enabled)
{
  myEnabled = enabled;
}

bool OnEventManager::alwaysOnlineNotify() const
{
  return myAlwaysOnlineNotify;
}

void OnEventManager::setAlwaysOnlineNotify(bool alwaysOnlineNotify)
{
  myAlwaysOnlineNotify = alwaysOnlineNotify;
}

string OnEventManager::command() const
{
  return myCommand;
}

void OnEventManager::setCommand(const string& command)
{
  myCommand = command;
}

string OnEventManager::parameter(OnEventType event) const
{
  return myParameters[event];
}

void OnEventManager::setParameter(OnEventType event, const string& parameter)
{
  myParameters[event] = parameter;
}

void OnEventManager::performOnEvent(OnEventType event, const Licq::User* user)
{
  if (!myEnabled)
    return;

  if (user != NULL)
  {
    if (user->onEventsBlocked())
      return;

    if (event == OnEventOnline)
    {
      if (!user->OnlineNotify())
        return;

      // We cannot always differentiate from users going online and users
      //   already online when we sign on. Make a guess based on online since.
      if (!myAlwaysOnlineNotify && user->OnlineSince()+60 < time(NULL))
        return;
    }

    // Get owner for user
    const Licq::Owner* owner;
    if (user->isUser())
      owner = gUserManager.FetchOwner(user->ppid(), LOCK_R);
    else
      owner = dynamic_cast<const Licq::Owner*>(user);
    if (owner == NULL)
      return;

    // Check if current status is reason to block on event action
    bool blocked = false;
    switch (owner->Status())
    {
      case ICQ_STATUS_AWAY:
        blocked = (!owner->AcceptInAway() && !user->AcceptInAway());
        break;
      case ICQ_STATUS_NA:
        blocked = (!owner->AcceptInNA() && !user->AcceptInNA());
        break;
      case ICQ_STATUS_OCCUPIED:
        blocked = (!owner->AcceptInOccupied() && !user->AcceptInOccupied());
        break;
      case ICQ_STATUS_DND:
        blocked = (!owner->AcceptInDND() && !user->AcceptInDND());
        break;
    }

    if (user->isUser())
      gUserManager.DropOwner(owner);

    if (blocked)
      return;
  }

  myMutex.lock();

  string param = myParameters[event];
  if (user != NULL)
  {
    char* newParam = user->usprintf(param.c_str(), Licq::USPRINTF_LINEISCMD);
    param = newParam;
    free(newParam);
  }
  if (!param.empty())
  {
    string fullCmd = myCommand + " " + param + " &";
    system(fullCmd.c_str());
  }

  myMutex.unlock();
}
