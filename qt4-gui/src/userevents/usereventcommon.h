// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Licq developers
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
  UserEventCommon(int userId, QWidget* parent = 0, const char* name = 0);
  virtual ~UserEventCommon();

  /**
   * Get user for this dialog
   *
   * @return (First) user associated with with dialog
   */
  int userId() const { return myUsers.front(); }
  const QString& id() const { return myId; }
  unsigned long ppid() const { return myPpid; }
  unsigned long convoId() { return myConvoId; }
  const std::list<int>& convoUsers() const { return myUsers; }
  void setConvoId(unsigned long n) { myConvoId = n; }
  void addEventTag(unsigned long n) { if (n) myEventTag.push_back(n); }

  /**
   * Check if a user is part of this conversation
   *
   * @param userId Id of user to check
   * @return True if user is in conversation
   */
  bool isUserInConvo(int userId) const;
  void setTyping(unsigned short type);

protected:
  std::list<unsigned long> myEventTag;

  bool myIsOwner;
  bool myDeleteUser;
  unsigned long myPpid;
  unsigned long myConvoId;
  time_t myRemoteTimeOffset;
  std::list<int> myUsers;
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
  InfoField* myTimezone;
  QTextCodec* myCodec;
  QTimer* myTimeTimer;
  QTimer* myTypingTimer;

  void flashTaskbar();
  void updateWidgetInfo(const LicqUser* u);
  void pushToolTip(QAction* action, QString tooltip);

  /**
   * A user has been update, this virtual function allows subclasses to add additional handling
   * This function will only be called if user is in this conversation
   *
   * @param userId Updated user
   * @param subSignal Type of update
   * @param argument Signal specific argument
   * @param cid Conversation id
   */
  virtual void userUpdated(int userId, unsigned long subSignal, int argument, unsigned long cid) = 0;

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
  void updatedUser(int userId, unsigned long subSignal, int argument, unsigned long cid);

signals:
  /**
   * Dialog has finished
   *
   * @param userId User for this dialog
   */
  void finished(int userId);
  void encodingChanged();
};

} // namespace LicqQtGui

#endif
