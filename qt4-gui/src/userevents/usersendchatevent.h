// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2006 Licq developers
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

#ifndef USERSENDCHATEVENT_H
#define USERSENDCHATEVENT_H

#include "usersendcommon.h"

class QLabel;

namespace LicqQtGui
{

class UserSendChatEvent : public UserSendCommon
{
  Q_OBJECT

public:
  UserSendChatEvent(QString id, unsigned long ppid, QWidget* parent = 0);
  virtual ~UserSendChatEvent();

private:
  QLabel* myItemLabel;
  InfoField* myItemEdit;
  QPushButton* myBrowseButton;
  QString myChatClients;
  unsigned short myChatPort;

  virtual bool sendDone(ICQEvent* e);
  virtual void resetSettings();

private slots:
  void inviteUser();
  virtual void send();
};

} //namespace LicqQtGui

#endif
