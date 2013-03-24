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

#ifndef LICQEMAIL_H
#define LICQEMAIL_H

#include <licq/plugin/generalpluginhelper.h>

#include <string>

#include <licq/userid.h>

namespace Licq
{
class Event;
class PluginSignal;
class TCPSocket;
class User;
class UserEvent;
}

#define FORWARD_EMAIL 0
#define FORWARD_LICQ 1

class CLicqForwarder : public Licq::GeneralPluginHelper
{
public:
  CLicqForwarder();
  ~CLicqForwarder();

  // From Licq::PluginInterface
  bool init(int argc, char** argv);
  int run();

  // From Licq::GeneralPluginInterface
  bool isEnabled() const;
  
protected:
  int m_nPipe;
  bool m_bExit;
  bool myIsEnabled;
  bool myMarkAsRead;
  std::string myStartupStatus;

  unsigned short m_nSMTPPort;
  std::string mySmtpHost;
  std::string mySmtpTo;
  std::string mySmtpFrom;
  std::string mySmtpDomain;
  Licq::UserId myUserId;
  unsigned m_nForwardType;

  Licq::TCPSocket* tcp;

public:
  void ProcessPipe();
  void ProcessSignal(const Licq::PluginSignal* s);
  void ProcessEvent(const Licq::Event* e);

  void ProcessUserEvent(const Licq::UserId& userId, unsigned long nId);
  bool ForwardEvent(const Licq::User* u, const Licq::UserEvent* e);
  bool ForwardEvent_Licq(const Licq::User* u, const Licq::UserEvent* e);
  bool ForwardEvent_Email(const Licq::User* u, const Licq::UserEvent* e);

private:

};


#endif
