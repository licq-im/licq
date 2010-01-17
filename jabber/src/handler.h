/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Erik Johansson <erijo@licq.org>
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

#ifndef HANDLER_H
#define HANDLER_H

#include <list>
#include <set>
#include <string>

class CICQDaemon;

class Handler
{
public:
  Handler(CICQDaemon* daemon);
  ~Handler();

  void setStatus(unsigned long status);

  void onConnect();
  void onChangeStatus(unsigned long status);
  void onDisconnect();

  void onUserAdded(const std::string& id, const std::string& name,
                   const std::list<std::string>& groups);
  void onUserRemoved(const std::string& id);
  void onUserStatusChange(const std::string& id, const unsigned long newStatus);
  void onRosterReceived(const std::set<std::string>& ids);

  void onMessage(const std::string& from, const std::string& message);

  std::string getStatusMessage(unsigned long status);

private:
  CICQDaemon* myDaemon;
  unsigned long myStatus;
};

inline void Handler::setStatus(unsigned long status)
{
  myStatus = status;
}

#endif
