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

#ifndef LICQJABBER_PLUGIN_H
#define LICQJABBER_PLUGIN_H

#include <licq/plugin/protocolpluginhelper.h>
#include <licq/mainloop.h>

#include <gloox/gloox.h>

namespace Licq
{
class ProtoAddUserSignal;
class ProtoChangeStatusSignal;
class ProtoChangeUserGroupsSignal;
class ProtoGrantAuthSignal;
class ProtoLogonSignal;
class ProtoRefuseAuthSignal;
class ProtoRemoveUserSignal;
class ProtoRenameGroupSignal;
class ProtoRenameUserSignal;
class ProtoRequestAuthSignal;
class ProtoRequestInfo;
class ProtoRequestPicture;
class ProtoTypingNotificationSignal;
class ProtoUpdateInfoSignal;
class ProtoSendMessageSignal;
class ProtocolSignal;
class UserId;
}

namespace LicqJabber
{

class Client;

class Plugin : public Licq::ProtocolPluginHelper, public Licq::MainLoopCallback
{
public:
  Plugin();
  ~Plugin();

  // From Licq::PluginInterface
  int run();
  
  // From Licq::MainLoopCallback
  void rawFileEvent(int fd, int revents);

private:
  void processSignal(const Licq::ProtocolSignal* signal);

  void doLogon(const Licq::ProtoLogonSignal* signal);
  void doChangeStatus(const Licq::ProtoChangeStatusSignal* signal);
  void doLogoff();
  void doSendMessage(const Licq::ProtoSendMessageSignal* signal);
  void doNotifyTyping(const Licq::ProtoTypingNotificationSignal* signal);
  void doGetInfo(const Licq::ProtoRequestInfo* signal);
  void doUpdateInfo(const Licq::ProtoUpdateInfoSignal* signal);
  void doGetPicture(const Licq::ProtoRequestPicture* signal);
  void doAddUser(const Licq::ProtoAddUserSignal* signal);
  void doChangeUserGroups(const Licq::ProtoChangeUserGroupsSignal* signal);
  void doRemoveUser(const Licq::ProtoRemoveUserSignal* signal);
  void doRenameUser(const Licq::ProtoRenameUserSignal* signal);
  void doGrantAuth(const Licq::ProtoGrantAuthSignal* signal);
  void doRefuseAuth(const Licq::ProtoRefuseAuthSignal* signal);
  void doRequestAuth(const Licq::ProtoRequestAuthSignal* signal);
  void doRenameGroup(const Licq::ProtoRenameGroupSignal* signal);

  void getUserGroups(const Licq::UserId& userId,
                     gloox::StringList& retGroupNames);

  Client* myClient;
  Licq::MainLoop myMainLoop;
};

} // namespace LicqJabber

#endif
