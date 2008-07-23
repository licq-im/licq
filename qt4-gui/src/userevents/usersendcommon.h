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

#ifndef USERSENDCOMMON_H
#define USERSENDCOMMON_H

#include "usereventcommon.h"

#include <licq_color.h>

class QGroupBox;
class QLabel;
class QPushButton;
class QSplitter;

class CICQSignal;
class ICQEvent;
class ICQUser;

namespace LicqQtGui
{
class HistoryView;
class MLEdit;
class MMUserView;

class UserSendCommon : public UserEventCommon
{
  Q_OBJECT
public:

  UserSendCommon(int type, QString id, unsigned long ppid, QWidget* parent = 0, const char* name = 0);
  virtual ~UserSendCommon();
  virtual bool eventFilter(QObject* watched, QEvent* e);

  void setText(const QString& text);
  void convoJoin(QString id, unsigned long convoId);
  void convoLeave(QString id, unsigned long convoId);

  virtual void windowActivationChange(bool oldActive);
  int clearDelay;

signals:
  void autoCloseNotify();
  void msgTypeChanged(UserSendCommon* from, UserSendCommon* to);

  /**
   * Since daemon doesn't notify us when an event is sent we'll have to handle
   * it ourselfs. This signal is sent to notify other windows about the event
   * that was sent.
   *
   * @param event Event object that was sent
   */
  void eventSent(const ICQEvent* event);

public slots:
  void changeEventType(int type);

protected:
  CICQColor myIcqColor;
  HistoryView* myHistoryView;
  MLEdit* myMessageEdit;
  MMUserView* myMassMessageList;
  QAction* myMassMessageCheck;
  QAction* mySendServerCheck;
  QAction* myUrgentCheck;
  QAction* myEventTypeMenu;
  QActionGroup* myEventTypeGroup;
  QGroupBox* myMassMessageBox;
  QLabel* myPictureLabel;
  QPushButton* myCloseButton;
  QPushButton* mySendButton;
  QSplitter* myViewSplitter;
  QSplitter* myPictureSplitter;
  QString myTempMessage;
  QTimer* mySendTypingTimer;
  int myType;

  void retrySend(ICQEvent* e, bool online, unsigned short level);
  virtual void userUpdated(CICQSignal* sig, QString id = QString::null, unsigned long ppid = 0);
  void updatePicture(const ICQUser* u = NULL);
  bool checkSecure();

  /**
   * Get icon for a message type
   *
   * @param type Message type
   * @return Message icon
   */
  const QPixmap& iconForType(int type) const;

  virtual void resetSettings() = 0;
  virtual bool sendDone(ICQEvent* e) = 0;

protected slots:
  /**
   * Update iconset in menus and on buttons
   */
  virtual void updateIcons();

  virtual void send();
  virtual void eventDoneReceived(ICQEvent* e);

  void cancelSend();
  void changeEventType(QAction* action);
  void clearNewEvents();
  void closeDialog();
  void showEmoticonsMenu();
  void insertEmoticon(const QString& value);
  void massMessageToggled(bool on);
  void messageAdded();
  void resetTitle();
  void sendServerToggled(bool on);
  void setBackgroundICQColor();
  void setForegroundICQColor();
  void showSendTypeMenu();
  void messageTextChanged();
  void textChangedTimeout();
  void sendTrySecure();
};

} // namespace LicqQtGui
#endif
