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

#ifndef JABBER_H
#define JABBER_H

class Client;
class CICQDaemon;
class Handler;

class LicqProtoChangeStatusSignal;
class LicqProtoLogonSignal;
class LicqProtoRequestInfo;
class LicqProtoSendMessageSignal;
class LicqProtoSignal;

#define JABBER_PPID 0x584D5050
#define L_JABBERxSTR "[JAB] "

class Jabber
{
public:
  Jabber(CICQDaemon* daemon);
  ~Jabber();

  int run(int pipe);

private:
  void processPipe(int pipe);
  void processSignal(LicqProtoSignal* signal);

  void doLogon(LicqProtoLogonSignal* signal);
  void doChangeStatus(LicqProtoChangeStatusSignal* signal);
  void doLogoff();
  void doSendMessage(LicqProtoSendMessageSignal* signal);
  void doGetInfo(LicqProtoRequestInfo* signal);

  CICQDaemon* myDaemon;
  Handler* myHandler;
  bool myDoRun;
  Client* myClient;
};

#endif
