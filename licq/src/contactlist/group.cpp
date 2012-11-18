/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq developers <licq-dev@googlegroups.com>
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

#include "group.h"

#include <cstdio> // snprintf

#include <licq/inifile.h>
#include <licq/userid.h>

using std::string;
using namespace LicqDaemon;

Group::Group(int id, const string& name)
{
  myId = id;
  myName = name;
  mySortIndex = 0;

  char strId[8];
  snprintf(strId, 7, "%u", myId);
  strId[7] = '\0';

  myMutex.setName(strId);
}

Group::~Group()
{
}

void Group::save(Licq::IniFile& file, int num) const
{
  char key[40];

  sprintf(key, "Group%d.id", num);
  file.set(key, myId);

  sprintf(key, "Group%d.name", num);
  file.set(key, myName);

  sprintf(key, "Group%d.Sorting", num);
  file.set(key, mySortIndex);

  ServerIdMap::const_iterator i;
  for (i = myServerIds.begin(); i != myServerIds.end(); ++i)
  {
    sprintf(key, "Group%d.ServerId.", num);
    file.set(key + i->first.toString(), i->second);
  }
}

unsigned long Group::serverId(const Licq::UserId& ownerId) const
{
  ServerIdMap::const_iterator iter;
  iter = myServerIds.find(ownerId);
  if (iter == myServerIds.end())
    return 0;
  return iter->second;
}

void Group::setServerId(const Licq::UserId& ownerId, unsigned long serverId)
{
  myServerIds[ownerId] = serverId;
}

void Group::unsetServerId(const Licq::UserId& ownerId)
{
  myServerIds.erase(ownerId);
}


bool LicqDaemon::compare_groups(const Licq::Group* first, const Licq::Group* second)
{
  return first->sortIndex() < second->sortIndex();
}
