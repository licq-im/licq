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

#ifndef USERSENDURLEVENT_H
#define USERSENDURLEVENT_H

#include "usersendcommon.h"

class QLabel;

namespace LicqQtGui
{

class UserSendUrlEvent : public UserSendCommon
{
  Q_OBJECT

public:
  /**
   * Constructor, create and open send URL dialog
   *
   * @param userId User to open dialog for
   * @param parent Parent widget
   */
  UserSendUrlEvent(const Licq::UserId& userId, QWidget* parent = 0);
  virtual ~UserSendUrlEvent();

  virtual bool eventFilter(QObject* watched, QEvent* e);

  void setUrl(const QString& url, const QString& description);

private:
  QLabel* myUrlLabel;
  InfoField* myUrlEdit;
  virtual bool sendDone(const Licq::Event* e);
  virtual void resetSettings();

private slots:
  virtual void send();
};

} //namespace LicqQtGui

#endif
