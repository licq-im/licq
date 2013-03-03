/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQRMS_H
#define LICQRMS_H

#include <licq/plugin/generalpluginhelper.h>

#include <list>

#include <licq/logging/pluginlogsink.h>
#include <licq/mainloop.h>
#include <licq/socket.h>
#include <licq/userid.h>

namespace Licq
{
class UserEvent;
}

const unsigned short MAX_LINE_LENGTH = 1024 * 1;
const unsigned short MAX_TEXT_LENGTH = 1024 * 8;

typedef std::list<class CRMSClient*> ClientList;
typedef std::list<unsigned long> TagList;

class CLicqRMS : public Licq::GeneralPluginHelper, public Licq::MainLoopCallback
{
public:
  CLicqRMS(const std::string& configFile);
  ~CLicqRMS();
  void Shutdown();

  // From Licq::PluginInterface
  bool init(int argc, char** argv);
  int run();

  // From Licq::GeneralPluginInterface
  bool isEnabled() const;

protected:
  // From Licq::MainLoopCallback
  void rawFileEvent(int fd, int revents);
  void socketEvent(Licq::INetSocket* inetSocket, int revents);

  void deleteClient(CRMSClient* client);
  void setupLogSink();

  bool m_bEnabled;

  unsigned int myPort;
  Licq::UserId myAuthOwnerId;
  std::string myAuthUser;
  std::string myAuthPassword;

  Licq::TCPSocket* server;
  ClientList clients;
  Licq::PluginLogSink::Ptr myLogSink;
  Licq::MainLoop myMainLoop;

private:
  const std::string myConfigFile;

public:
  void ProcessPipe();
  void ProcessSignal(const Licq::PluginSignal* s);
  void ProcessEvent(const Licq::Event* e);
  void ProcessServer();
  void ProcessLog();

friend class CRMSClient;

};


class CRMSClient : public Licq::MainLoopCallback
{
public:
  CRMSClient(Licq::TCPSocket*);
  ~CRMSClient();

  int Activity();

  int Process_QUIT();
  int Process_TERM();
  int Process_INFO();
  int Process_STATUS();
  int Process_HELP();
  int Process_GROUPS();
  int Process_HISTORY();
  int Process_LIST();
  int Process_MESSAGE();
  int Process_URL();
  int Process_SMS();
  int Process_LOG();
  int Process_VIEW();
  int Process_AR();
  int Process_ADDUSER();
  int Process_REMUSER();
  int Process_SECURE();
  int Process_NOTIFY();

protected:
  // From Licq::MainLoopCallback
  void socketEvent(Licq::INetSocket* inetSocket, int revents);

  Licq::TCPSocket sock;
  FILE *fs;
  TagList tags;
  unsigned short m_nState;
  char data_line[MAX_LINE_LENGTH + 1];
  char *data_arg;
  unsigned short data_line_pos;
  std::string myLoginUser;
  char *m_szCheckId;
  unsigned int myLogLevelsBitmask;
  bool m_bNotify;

  Licq::UserId myUserId;
  std::string myText;
  std::string myLine;

  int StateMachine();
  int ProcessCommand();
  bool ProcessEvent(const Licq::Event* e);
  bool AddLineToText();
  void ParseUser(const std::string& data);
  int changeStatus(const Licq::UserId& ownerId, const std::string& strStatus);

  int Process_MESSAGE_text();
  int Process_URL_url();
  int Process_URL_text();
  int Process_SMS_number();
  int Process_SMS_message();
  int Process_AR_text();

  /**
   * Output a user event
   *
   * @param e User event
   * @param alias Alias of sender
   */
  void printUserEvent(const Licq::UserEvent* e, const std::string& alias);

friend class CLicqRMS;
};

extern CLicqRMS *licqRMS;


#endif
