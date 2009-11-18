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

#ifndef USERSENDCOMMON_H
#define USERSENDCOMMON_H

#include "usereventcommon.h"

#include <licq_color.h>

class QGroupBox;
class QLabel;
class QPushButton;
class QSplitter;

class LicqEvent;

namespace LicqQtGui
{
class HistoryView;
class MLEdit;
class MMUserView;

class UserSendCommon : public UserEventCommon
{
  Q_OBJECT
public:
  /**
   * Constructor, create and open send event dialog
   *
   * @param type Type of event
   * @param userId User to open dialog for
   * @param parent Parent widget
   * @param name Object name of widget
   */
  UserSendCommon(int type, const UserId& userId, QWidget* parent = 0, const char* name = 0);
  virtual ~UserSendCommon();
  virtual bool eventFilter(QObject* watched, QEvent* e);

  void setText(const QString& text);

  /**
   * Someone joined the conversation
   *
   * @param userId User that joined conversation
   */
  void convoJoin(const UserId& userId);

  /**
   * Someone left the conversation
   *
   * @param userId User that left conversation
   */
  void convoLeave(const UserId& userId);

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
  void eventSent(const LicqEvent* event);

public slots:
  /**
   * Convert dialog to a specified event type
   *
   * @param type Type of event to switch to
   * @return Pointer to new window (if replaced), otherwise to this window
   */
  UserSendCommon* changeEventType(int type);

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
  std::list<unsigned long> myEventTag;

  void retrySend(const LicqEvent* e, bool online, unsigned short level);

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
  void updatePicture(const LicqUser* u = NULL);
  bool checkSecure();

  /**
   * Get icon for a message type
   *
   * @param type Message type
   * @return Message icon
   */
  const QPixmap& iconForType(int type) const;

  virtual void resetSettings() = 0;
  virtual bool sendDone(const LicqEvent* e) = 0;

  /**
   * Widget is about to be closed
   * Overloaded to make sure widget is always removed from tab dialog before it's destroyed
   *
   * @parame event Close event
   */
  virtual void closeEvent(QCloseEvent* event);

  /**
   * Overloaded resize event to save new dialog size
   *
   * @param event Resize event
   */
  virtual void resizeEvent(QResizeEvent* event);

protected slots:
  /**
   * Update iconset in menus and on buttons
   */
  virtual void updateIcons();

  /**
   * Update keyboard shortcuts
   */
  virtual void updateShortcuts();

  virtual void send();
  virtual void eventDoneReceived(const LicqEvent* e);

  /**
   * An event tag was generated
   * Used by protocols that cannot return event id from event send function
   *
   * @param userId User event belongs to
   * @param eventTag Id for event
   */
  void addEventTag(const UserId& userId, unsigned long eventTag);

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

  /**
   * A dragged object has entered this widget
   * Overloaded to accept files, URLs and users to be sent by dragging them
   *
   * @param event The drag event
   */
  void dragEnterEvent(QDragEnterEvent* event);

  /**
   * A dragged object was dropped on this widget
   * Overloaded to accept files, URLs and users to be sent by dragging them
   *
   * @param event The drop event
   */
  void dropEvent(QDropEvent* event);
};

} // namespace LicqQtGui
#endif
