/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq Developers <licq-dev@googlegroups.com>
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

#ifndef JABBER_H
#define JABBER_H

namespace Licq
{
class ProtoAddUserSignal;
class ProtoChangeStatusSignal;
class ProtoChangeUserGroupsSignal;
class ProtoGrantAuthSignal;
class ProtoLogonSignal;
class ProtoRefuseAuthSignal;
class ProtoRemoveUserSignal;
class ProtoRenameUserSignal;
class ProtoRequestInfo;
class ProtoUpdateInfoSignal;
class ProtoSendMessageSignal;
class ProtocolSignal;
}

class Client;
class Handler;

class Jabber
{
public:
  Jabber();
  ~Jabber();

  int run(int pipe);

private:
  void processPipe(int pipe);
  void processSignal(Licq::ProtocolSignal* signal);

  void doLogon(Licq::ProtoLogonSignal* signal);
  void doChangeStatus(Licq::ProtoChangeStatusSignal* signal);
  void doLogoff();
  void doSendMessage(Licq::ProtoSendMessageSignal* signal);
  void doGetInfo(Licq::ProtoRequestInfo* signal);
  void doUpdateInfo(Licq::ProtoUpdateInfoSignal* signal);
  void doAddUser(Licq::ProtoAddUserSignal* signal);
  void doChangeUserGroups(Licq::ProtoChangeUserGroupsSignal* signal);
  void doRemoveUser(Licq::ProtoRemoveUserSignal* signal);
  void doRenameUser(Licq::ProtoRenameUserSignal* signal);
  void doGrantAuth(Licq::ProtoGrantAuthSignal* signal);
  void doRefuseAuth(Licq::ProtoRefuseAuthSignal* signal);

  Handler* myHandler;
  bool myDoRun;
  Client* myClient;
};

#endif
