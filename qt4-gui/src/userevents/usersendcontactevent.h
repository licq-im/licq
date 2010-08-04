// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#ifndef USERSENDCONTACTEVENT_H
#define USERSENDCONTACTEVENT_H

#include "usersendcommon.h"

namespace LicqQtGui
{

class UserSendContactEvent : public UserSendCommon
{
  Q_OBJECT

public:
  /**
   * Constructor, create and open send contact dialog
   *
   * @param userId User to open dialog for
   * @param parent Parent widget
   */
  UserSendContactEvent(const Licq::UserId& userId, QWidget* parent = 0);
  virtual ~UserSendContactEvent();

  /**
   * Add contact to send
   *
   * @param userId Id of user to add
   */
  void setContact(const Licq::UserId& userId);

private:
  MMUserView* myContactsList;

  virtual bool sendDone(const Licq::Event* e);
  virtual void resetSettings();

private slots:
  virtual void send();
};

} // namespace LicqQtGui

#endif
