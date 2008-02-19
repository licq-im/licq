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

using std::list;
using std::string;


namespace LicqQtGui
{
class InfoField;

enum EventType
{
  ET_MESSAGE,
  ET_URL,
  ET_CHAT,
  ET_FILE,
  ET_CONTACT,
  ET_SMS
};

class UserEventCommon : public QWidget
{
  Q_OBJECT

public:
  UserEventCommon(QString id, unsigned long ppid, QWidget* parent = 0, const char* name = 0);
  virtual ~UserEventCommon();

  QString id() { return QString::fromAscii(myUsers.front().c_str()); }
  unsigned long ppid() { return myPpid; }
  unsigned long convoId() { return myConvoId; }
  list<string>& convoUsers() { return myUsers; }
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
  list<string> myUsers;

  // ID of the higest event we've processed. Helps determine
  // which events we already processed in the ctor.
  int myHighestEventId;

  QString myId;
  QString myBaseTitle;
  QString myProgressMsg;

  QHBoxLayout* top_hlay;
  QVBoxLayout* top_lay;
  QVBoxLayout* mainWidget;
  QToolBar* myToolBar;
  QMenu* popupEncoding;
  QActionGroup* encodingsGroup;
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
  QTimer* tmrTime;
  QTimer* tmrTyping;

  void flashTaskbar();
  void updateWidgetInfo(ICQUser* u);
  void pushToolTip(QAction* action, QString tooltip);

  virtual void userUpdated(CICQSignal* sig, QString id = QString::null, unsigned long ppid = 0) = 0;

protected slots:
  /**
   * Update iconset in menus and on buttons
   */
  virtual void updateIcons();

  void slotConnectSignal();
  void slotSetEncoding(QAction* action);
  void slotSetMsgWinSticky(bool sticky = true);
  void slotShowHistory();
  void slotShowUserInfo();
  void slotSwitchSecurity();
  void slotUpdateTime();
  void slotUpdateTyping();
  void slotUserMenu();
  void showEncodingsMenu();
  void slotUserUpdated(CICQSignal* sig);

signals:
  void finished(QString id, unsigned long ppid);
  void encodingChanged();
  void viewUrl(QWidget*, QString);
};

} // namespace LicqQtGui

#endif
