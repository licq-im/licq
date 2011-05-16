/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2011 Licq developers
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

#include "filter.h"

#include <licq/contactlist/user.h>
#include <licq/inifile.h>
#include <licq/thread/mutexlocker.h>
#include <licq/userevents.h>

#include <boost/foreach.hpp>
#include <boost/regex.hpp>

using namespace std;
using namespace LicqDaemon;
using Licq::FilterRule;
using Licq::FilterRules;
using Licq::UserEvent;

// Declare global FilterManager (internal for daemon)
LicqDaemon::FilterManager LicqDaemon::gFilterManager;

// Declare global Licq::FilterManager to refer to the internal FilterManager
Licq::FilterManager& Licq::gFilterManager(LicqDaemon::gFilterManager);

FilterManager::FilterManager()
{
  // Empty
}

FilterManager::~FilterManager()
{
  myRules.clear();
}

void FilterManager::initialize()
{
  Licq::IniFile conf("filter.conf");
  if(!conf.loadFile())
  {
    // Failed to read configuration, setup defaults

    FilterRule rule;

    // Log but don't notify authorization requests containing URLs
    rule.isEnabled = true;
    rule.protocolId = 0;
    rule.eventMask = 1<<UserEvent::TypeAuthRequest;
    rule.expression = ".*http://.*";
    rule.action = FilterRule::ActionSilent;
    myRules.push_back(rule);

    // Ignore reoccuring spam messages that are sent as ICQ Authorization Requests
    rule.isEnabled = true;
    rule.protocolId = LICQ_PPID;
    rule.eventMask = 1<<UserEvent::TypeAuthRequest;
    rule.action = FilterRule::ActionIgnore;
    rule.expression = "\\xD0[\\xBF\\x9F]\\xD1\\x80\\xD0\\xB8\\xD0\\xB2\\xD0\\xB5\\xD1\\x82(\\xD0\\xB8\\xD0\\xBA)? =\\)";
    myRules.push_back(rule);
    rule.expression = "\\xD0\\x90 \\xD1\\x8F \\xD1\\x82\\xD0\\xB5\\xD0\\xB1\\xD1\\x8F \\xD0\\xB7\\xD0\\xBD\\xD0\\xB0\\xD1\\x8E =\\)";
    myRules.push_back(rule);
    rule.expression = "\\xD0\\x9F\\xD0\\xBE\\xD0\\xB6\\xD0\\xB5\\xD0\\xBB\\xD1\\x83\\xD0\\xB9\\xD1\\x81\\xD1\\x82\\xD0\\xB0, "
        "\\xD1\\x80\\xD0\\xB0\\xD0\\xB7\\xD1\\x80\\xD0\\xB5\\xD1\\x88\\xD0\\xB8\\xD1\\x82\\xD0\\xB5 \\xD0\\xB4\\xD0\\xBE"
        "\\xD0\\xB1\\xD0\\xB0\\xD0\\xB2\\xD0\\xB8\\xD1\\x82\\xD1\\x8C \\xD0\\x92\\xD0\\xB0\\xD1\\x81 \\xD0\\xB2 "
        "\\xD0\\xBC\\xD0\\xBE\\xD0\\xB9 \\xD1\\x81\\xD0\\xBF\\xD0\\xB8\\xD1\\x81\\xD0\\xBE\\xD0\\xBA "
        "\\xD0\\xBA\\xD0\\xBE\\xD0\\xBD\\xD1\\x82\\xD0\\xB0\\xD0\\xBA\\xD1\\x82\\xD0\\xBE\\xD0\\xB2";
    myRules.push_back(rule);

    saveRules();
    return;
  }

  conf.setSection("eventfilter");
  int numrules;
  conf.get("NumRules", numrules);

  for (int i = 0; i < numrules; ++i)
  {
    char key[20];
    FilterRule rule;

    sprintf(key, "Rule%i.enabled", i);
    conf.get(key, rule.isEnabled);
    sprintf(key, "Rule%i.protocol", i);
    conf.get(key, rule.protocolId);
    sprintf(key, "Rule%i.events", i);
    conf.get(key, rule.eventMask);
    sprintf(key, "Rule%i.expression", i);
    conf.get(key, rule.expression);
    sprintf(key, "Rule%i.action", i);
    conf.get(key, rule.action);

    myRules.push_back(rule);
  }
}

FilterRules FilterManager::getRules()
{
  Licq::MutexLocker lock(myDataMutex);
  return myRules;
}

void FilterManager::setRules(const FilterRules& newRules)
{
  Licq::MutexLocker lock(myDataMutex);
  myRules = newRules;

  saveRules();
}

void FilterManager::saveRules()
{
  Licq::IniFile conf("filter.conf");
  conf.loadFile();
  conf.setSection("eventfilter");
  conf.set("NumRules", myRules.size());

  int i = 0;
  BOOST_FOREACH(const FilterRule& rule, myRules)
  {
    char key[20];

    sprintf(key, "Rule%i.enabled", i);
    conf.set(key, rule.isEnabled);
    sprintf(key, "Rule%i.protocol", i);
    conf.set(key, rule.protocolId);
    sprintf(key, "Rule%i.events", i);
    conf.set(key, rule.eventMask);
    sprintf(key, "Rule%i.expression", i);
    conf.set(key, rule.expression);
    sprintf(key, "Rule%i.action", i);
    conf.set(key, rule.action);

    ++i;
  }
  conf.writeFile();
}

int FilterManager::filterEvent(const Licq::User* user, const Licq::UserEvent* event)
{
  // Get message and user id
  Licq::UserId userId = user->id();
  string msg;

  switch (event->eventType())
  {
    case UserEvent::TypeMessage:
      msg = (dynamic_cast<const Licq::EventMsg*>(event))->message();
      break;
    case UserEvent::TypeFile:
      msg = (dynamic_cast<const Licq::EventFile*>(event))->fileDescription();
      break;
    case UserEvent::TypeUrl:
      msg = (dynamic_cast<const Licq::EventUrl*>(event))->urlDescription();
      break;
    case UserEvent::TypeChat:
      msg = (dynamic_cast<const Licq::EventChat*>(event))->reason();
      break;
    case UserEvent::TypeAdded:
      // No message
      userId = (dynamic_cast<const Licq::EventAdded*>(event))->userId();
      break;
    case UserEvent::TypeAuthRequest:
      msg = (dynamic_cast<const Licq::EventAuthRequest*>(event))->reason();
      userId = (dynamic_cast<const Licq::EventAuthRequest*>(event))->userId();
      break;
    case UserEvent::TypeAuthGranted:
      msg = (dynamic_cast<const Licq::EventAuthGranted*>(event))->message();
      userId = (dynamic_cast<const Licq::EventAuthGranted*>(event))->userId();
      break;
    case UserEvent::TypeAuthRefused:
      msg = (dynamic_cast<const Licq::EventAuthRefused*>(event))->message();
      userId = (dynamic_cast<const Licq::EventAuthRefused*>(event))->userId();
      break;
    case UserEvent::TypeWebPanel:
      msg = (dynamic_cast<const Licq::EventWebPanel*>(event))->message();
      break;
    case UserEvent::TypeEmailPager:
      msg = (dynamic_cast<const Licq::EventEmailPager*>(event))->message();
      break;
    case UserEvent::TypeContactList:
      // No message
      break;
    case UserEvent::TypeSms:
      msg = (dynamic_cast<const Licq::EventSms*>(event))->message();
      break;
    case UserEvent::TypeMsgServer:
      msg = (dynamic_cast<const Licq::EventServerMessage*>(event))->message();
      break;
    case UserEvent::TypeEmailAlert:
      msg = (dynamic_cast<const Licq::EventEmailAlert*>(event))->subject();
      break;
  }

  // Check if user is in list
  bool userInList = false;
  if (userId == user->id())
  {
    userInList = !user->NotInList();
  }
  else
  {
    Licq::UserReadGuard u(userId);
    if (u.isLocked() && !user->NotInList())
      userInList = true;
  }

  // Always accept users that are already in list
  if (userInList)
    return FilterRule::ActionAccept;

  Licq::MutexLocker lock(myDataMutex);
  BOOST_FOREACH(const FilterRule& rule, myRules)
  {
    if (!rule.isEnabled)
      continue;

    if (rule.protocolId != 0 && rule.protocolId != userId.protocolId())
      continue;

    if ((rule.eventMask & (1<<event->eventType())) == 0)
      continue;

    if (!rule.expression.empty())
    {
      try
      {
        boost::regex re(rule.expression, boost::regex::nosubs);
        if (!boost::regex_match(msg, re))
          continue;
      }
      catch (boost::regex_error& e)
      {
        // Expression is invalid so ignore this rule
        continue;
      }
    }

    // This rule matches, return result
    return rule.action;
  }

  // No rule matched, use default
  return FilterRule::ActionAccept;
}
