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

class CICQSignal;
class ICQUser;


namespace LicqQtGui
{
class InfoField;

class UserEventCommon : public QWidget
{
  Q_OBJECT

public:
  UserEventCommon(QString id, unsigned long ppid, QWidget* parent = 0, const char* name = 0);
  virtual ~UserEventCommon();

  QString id() { return QString::fromAscii(myUsers.front().c_str()); }
  unsigned long ppid() { return myPpid; }
  unsigned long convoId() { return myConvoId; }
  std::list<std::string>& convoUsers() { return myUsers; }
  void setConvoId(unsigned long n) { myConvoId = n; }
  void addEventTag(unsigned long n) { if (n) myEventTag.push_back(n); }

  bool isUserInConvo(QString id);
  void setTyping(unsigned short type);

protected:
  std::list<unsigned long> myEventTag;

  bool myIsOwner;
  bool myDeleteUser;
  unsigned long myPpid;
  unsigned long myConvoId;
  time_t myRemoteTimeOffset;
  std::list<std::string> myUsers;
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
  void updateWidgetInfo(const ICQUser* u);
  void pushToolTip(QAction* action, QString tooltip);

  virtual void userUpdated(CICQSignal* sig, QString id = QString::null, unsigned long ppid = 0) = 0;

protected slots:
  /**
   * Update iconset in menus and on buttons
   */
  virtual void updateIcons();

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
  void updatedUser(CICQSignal* sig);

signals:
  void finished(QString id, unsigned long ppid);
  void encodingChanged();
};

} // namespace LicqQtGui

#endif
