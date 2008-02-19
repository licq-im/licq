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
class QPushButton;
class QSplitter;

class CICQSignal;
class ICQEvent;

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
  void updateUser(CICQSignal* sig);
  void msgTypeChanged(UserSendCommon* from, UserSendCommon* to);

public slots:
  void changeEventType(int type);

protected:
  CICQColor icqColor;
  HistoryView* mleHistory;
  MLEdit* mleSend;
  MMUserView* lstMultipleRecipients;
  QAction* chkMass;
  QAction* chkSendServer;
  QAction* chkUrgent;
  QAction* cmbSendType;
  QActionGroup* grpSendType;
  QGroupBox* grpMR;
  QPushButton* btnClose;
  QPushButton* btnSend;
  QSplitter* splView;
  QString strTempMsg;
  QTimer* tmrSendTyping;
  int myType;

  void retrySend(ICQEvent* e, bool online, unsigned short level);
  virtual void userUpdated(CICQSignal* sig, QString id = QString::null, unsigned long ppid = 0);
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
  virtual void slotSendDone(ICQEvent* e);

  void slotCancelSend();
  void slotChangeEventType(QAction* action);
  void slotClearNewEvents();
  void slotClose();
  void slotEmoticon();
  void slotInsertEmoticon(const QString& value);
  void slotMassMessageToggled(bool on);
  void slotMessageAdded();
  void slotResetTitle();
  void slotSendServerToggled(bool on);
  void slotSetBackgroundICQColor();
  void slotSetForegroundICQColor();
  void slotShowSendTypeMenu();
  void messageTextChanged();
  void slotTextChangedTimeout();
  void sendTrySecure();
};

} // namespace LicqQtGui
#endif
