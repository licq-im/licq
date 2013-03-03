/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2011, 2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQAUTOREPLY_H
#define LICQAUTOREPLY_H

#include <licq/plugin/generalpluginhelper.h>

#include <string>


namespace Licq
{
class Event;
class PluginSignal;
class UserEvent;
class UserId;
}

class CLicqAutoReply : public Licq::GeneralPluginHelper
{
public:
  CLicqAutoReply();
  ~CLicqAutoReply();

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
  std::string myProgram;
  std::string myArguments;
  bool m_bPassMessage, m_bFailOnExitCode, m_bAbortDeleteOnExitCode,
       m_bSendThroughServer;

  void ProcessPipe();
  void ProcessSignal(const Licq::PluginSignal* s);
  void ProcessEvent(const Licq::Event* e);

  /**
   * A new event arrived for a user
   *
   * @param userId Affected user
   * @param eventId Id of event
   */
  void processUserEvent(const Licq::UserId& userId, unsigned long eventId);

  /**
   * Make auto reply for an event
   *
   * @param userId Affected user
   * @Param event Event to reply to
   * @return True if a reply was sent
   */
  bool autoReplyEvent(const Licq::UserId& userId, const Licq::UserEvent* event);

  bool POpen(const char *cmd);
  int PClose();

protected:
  int pid;
  FILE *fStdOut;
  FILE *fStdIn;

};


#endif
