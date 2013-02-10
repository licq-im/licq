/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq developers <licq-dev@googlegroups.com>
 *
 * Please refer to the COPYRIGHT file distributed with this source
 * distribution for the names of the individual contributors.
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

#ifndef LICQJABBER_HANDLER_H
#define LICQJABBER_HANDLER_H

#include <boost/noncopyable.hpp>
#include <list>
#include <map>
#include <set>
#include <string>

#include <licq/userid.h>

namespace LicqJabber
{

class VCardToUser;

class Handler : private boost::noncopyable
{
public:
  Handler(const Licq::UserId& ownerId);

  void onConnect(const std::string& ip, int port, unsigned status);
  void onChangeStatus(unsigned status);
  void onDisconnect(bool authError);

  void onUserAdded(const std::string& id, const std::string& name,
                   const std::list<std::string>& groups,
                   bool awaitingAuthorization);
  void onUserRemoved(const std::string& id);
  void onUserStatusChange(const std::string& id, unsigned status,
                          const std::string& msg,
                          const std::string& photoHash);
  void onUserInfo(const std::string& id, const VCardToUser& wrapper);
  void onRosterReceived(const std::set<std::string>& ids);
  void onUserAuthorizationRequest(const std::string& id,
                                  const std::string& message);

  void onMessage(const std::string& from, const std::string& message,
                 time_t sent, bool urgent);
  void onNotifyTyping(const std::string& from, bool active);

  std::string getStatusMessage(unsigned status);

private:
  Licq::UserId myOwnerId;
};

} // namespace LicqJabber

#endif
