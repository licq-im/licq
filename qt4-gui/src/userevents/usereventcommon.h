// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2009 Licq developers
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

#ifndef USEREVENTCOMMON_H
#define USEREVENTCOMMON_H

#include <QWidget>

#include <list>
#include <string>

#include <licq_types.h>

class QActionGroup;
class QHBoxLayout;
class QMenu;
class QToolBar;
class QVBoxLayout;

class LicqUser;


namespace LicqQtGui
{
class InfoField;

class UserEventCommon : public QWidget
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param userId User to open event dialog for
   * @param parent Parent widget
   * @param name Object name to set for widget
   */
  UserEventCommon(const UserId& userId, QWidget* parent = 0, const char* name = 0);
  virtual ~UserEventCommon();

  /**
   * Get user for this dialog
   *
   * @return (First) user associated with with dialog
   */
  const UserId& userId() const { return myUsers.front(); }
  const QString& id() const { return myId; }
  unsigned long ppid() const { return myPpid; }
  unsigned long convoId() { return myConvoId; }
  const std::list<UserId>& convoUsers() const { return myUsers; }
  void setConvoId(unsigned long n) { myConvoId = n; }

  /**
   * Check if a user is part of this conversation
   *
   * @param userId Id of user to check
   * @return True if user is in conversation
   */
  bool isUserInConvo(const UserId& userId) const;
  void setTyping(unsigned short type);

public slots:
  /**
   * This window got or lost focus
   * Called from tab dialog when tab is switched
   *
   * @param gotFocus True if this window/tab got focus
   */
  void focusChanged(bool gotFocus);

protected:
  bool myIsOwner;
  bool myDeleteUser;
  unsigned long myPpid;
  unsigned long myConvoId;
  time_t myRemoteTimeOffset;
  std::list<UserId> myUsers;
  unsigned long mySendFuncs;

  // ID of the higest event we've processed. Helps determine
  // which events we already processed in the ctor.
  int myHighestEventId;

  QString myId;
  QString myBaseTitle;
  QString myProgressMsg;

  QHBoxLayout* myTophLayout;
  QVBoxLayout* myTopLayout;
  QVBoxLayout* myMainWidget;
  QToolBar* myToolBar;
  QMenu* myEncodingsMenu;
  QActionGroup* myEncodingsGroup;
  QAction* myMenu;
  QAction* myHistory;
  QAction* myInfo;
  QAction* myEncoding;
  QAction* mySecure;
  QAction* myEmoticon;
  QAction* myForeColor;
  QAction* myBackColor;
  QAction* myPopupNextMessage;
  InfoField* myTimezone;
  const QTextCodec* myCodec;
  QTimer* myTimeTimer;
  QTimer* myTypingTimer;

  void flashTaskbar();
  void updateWidgetInfo(const LicqUser* u);
  void pushToolTip(QAction* action, const QString& tooltip);

  /**
   * A user has been update, this virtual function allows subclasses to add additional handling
   * This function will only be called if user is in this conversation
   *
   * @param userId Updated user
   * @param subSignal Type of update
   * @param argument Signal specific argument
   * @param cid Conversation id
   */
  virtual void userUpdated(const UserId& userId, unsigned long subSignal, int argument, unsigned long cid) = 0;

  /**
   * Overloaded to get events when this window/tab looses and gains focus
   */
  virtual bool event(QEvent* event);

protected slots:
  /**
   * Update iconset in menus and on buttons
   */
  virtual void updateIcons();

  /**
   * Update keyboard shortcuts
   */
  virtual void updateShortcuts();

  void connectSignal();
  void setEncoding(QAction* action);
  void setMsgWinSticky(bool sticky = true);
  void showHistory();
  void showUserInfo();
  void switchSecurity();
  void updateTime();
  void updateTyping();
  void showUserMenu();
  void showEncodingsMenu();
  void updatedUser(const UserId& userId, unsigned long subSignal, int argument, unsigned long cid);

signals:
  /**
   * Dialog has finished
   *
   * @param userId User for this dialog
   */
  void finished(const UserId& userId);
  void encodingChanged();
};

} // namespace LicqQtGui

#endif
