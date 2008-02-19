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

#ifndef USERVIEWEVENT_H
#define USERVIEWEVENT_H

#include "usereventcommon.h"

class QCheckBox;
class QPushButton;
class QSplitter;
class QTreeWidgetItem;

class CUserEvent;
class ICQEvent;


namespace LicqQtGui
{
class MLView;
class MessageList;
class SkinnableButton;
class UserSendCommon;

class UserViewEvent : public UserEventCommon
{
  Q_OBJECT

public:
  UserViewEvent(QString id, unsigned long ppid, QWidget* parent = 0);
  virtual ~UserViewEvent();

private:
  QSplitter* splRead;
  MLView* mlvRead;
  MessageList* msgView;
  CUserEvent* currentEvent;
  QCheckBox* chkAutoClose;
  QPushButton* btnRead1;
  QPushButton* btnRead2;
  QPushButton* btnRead3;
  QPushButton* btnRead4;
  QPushButton* btnReadNext;
  SkinnableButton* btnClose;

  // The currently displayed message in decoded (Unicode) form.
  QString myMessageText;

  void generateReply();
  void sendMsg(QString text);
  void updateNextButton();
  virtual void userUpdated(CICQSignal* sig, QString id = QString::null, unsigned long ppid = 0);

private slots:
  void slotAutoClose();
  void slotBtnRead1();
  void slotBtnRead2();
  void slotBtnRead3();
  void slotBtnRead4();
  void slotBtnReadNext();
  void slotClearEvent();
  void slotClose();
  void slotMsgTypeChanged(UserSendCommon* from, UserSendCommon* to);
  void slotPrintMessage(QTreeWidgetItem* item);
  void slotSentEvent(ICQEvent* e);
  void slotSetEncoding();
};

} // namespace LicqQtGui

#endif
