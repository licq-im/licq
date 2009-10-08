// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2009 Licq developers
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
class QGroupBox;
class QPushButton;
class QSplitter;
class QTreeWidgetItem;

class CUserEvent;
class LicqEvent;


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
  /**
   * Constructor, create and open dialog to view user events
   *
   * @param userId User to open dialog for
   * @param parent Parent widget
   */
  UserViewEvent(const UserId& userId, QWidget* parent = 0);
  virtual ~UserViewEvent();

protected:
  /**
   * Overloaded resize event to save new dialog size
   *
   * @param event Resize event
   */
  virtual void resizeEvent(QResizeEvent* event);

private:
  QSplitter* myReadSplitter;
  MLView* myMessageView;
  MessageList* myMessageList;
  CUserEvent* myCurrentEvent;
  QCheckBox* myAutoCloseCheck;
  QGroupBox* myActionsBox;
  QPushButton* myRead1Button;
  QPushButton* myRead2Button;
  QPushButton* myRead3Button;
  QPushButton* myRead4Button;
  QPushButton* myReadNextButton;
  SkinnableButton* myCloseButton;

  // The currently displayed message in decoded (Unicode) form.
  QString myMessageText;

  void generateReply();
  void sendMsg(QString text);
  void updateNextButton();

  /**
   * A user has been update, this virtual function allows subclasses to add additional handling
   * This function will only be called if user is in this conversation
   *
   * @param userId Updated user
   * @param subSignal Type of update
   * @param argument Signal specific argument
   * @param cid Conversation id
   */
  virtual void userUpdated(const UserId& userId, unsigned long subSignal, int argument, unsigned long cid);

private slots:
  void autoClose();
  void read1();
  void read2();
  void read3();
  void read4();
  void readNext();
  void clearEvent();
  void closeDialog();
  void msgTypeChanged(UserSendCommon* from, UserSendCommon* to);
  void printMessage(QTreeWidgetItem* item);
  void sentEvent(const LicqEvent* e);
  void setEncoding();
};

} // namespace LicqQtGui

#endif
