/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#ifndef LICQGUI_H
#define LICQGUI_H

#include "config.h"

#ifdef USE_KDE
#include <KDE/KApplication>
#else
#include <QApplication>
#endif

#include <QList>
#include <QStringList>
#include <QTimer>

#include <licq_user.h>

class CICQDaemon;
class CICQSignal;
class ICQEvent;

namespace LicqQtGui
{
class ContactListModel;
class DockIcon;
class GroupMenu;
class LogWindow;
class MainWindow;
class SignalManager;
class UserEventCommon;
class UserEventTabDlg;
class UserDlg;
class UserMenu;
class UserSendCommon;
class UserViewEvent;

typedef QList<UserViewEvent*> UserViewEventList;
typedef QList<UserDlg*> UserDlgList;
typedef QList<UserSendCommon*> UserSendEventList;

#ifdef USE_KDE
class LicqGui : public KApplication
#else
class LicqGui : public QApplication
#endif
{
  Q_OBJECT

public:
  /**
   * Get the singleton instance
   *
   * @return The instance
   */
  static LicqGui* instance()
  { return myInstance; }

  LicqGui(int& argc, char** argv);
  ~LicqGui();

  int Run(CICQDaemon* daemon);

  // Functions to get daemon and gui resources
  CICQDaemon* licqDaemon() { return myLicqDaemon; }
  MainWindow* mainWindow() { return myMainWindow; }
  SignalManager* signalManager() { return mySignalManager; }
  LogWindow* logWindow() { return myLogWindow; }
  ContactListModel* contactList() { return myContactList; }
  UserMenu* userMenu() { return myUserMenu; }
  GroupMenu* groupMenu() { return myGroupMenu; }
  DockIcon* dockIcon() { return myDockIcon; }
  UserEventTabDlg* userEventTabDlg() { return myUserEventTabDlg; }

  virtual void commitData(QSessionManager& sm);
  virtual void saveState(QSessionManager& sm);

#if defined(Q_WS_X11)
  virtual bool x11EventFilter(XEvent* event);
#endif /* defined(Q_WS_X11) */

  /**
   * Remove a contact from the list
   *
   * @param id Contact id
   * @param ppid Contact protocol id
   * @param parent Parent window to use for confirmation box or NULL to use mainwin
   * @return true if contact was removed
   */
  bool removeUserFromList(QString id, unsigned long ppid, QWidget* parent = NULL);

  /**
   * Show contact info dialog
   *
   * @param fcn Tab to show
   * @param id Contact id
   * @param ppid Contact protocol id
   * @param toggle True to close dialog if already open
   * @param updateNow True to make the dialog contents update
   */
  void showInfoDialog(int fcn, QString id, unsigned long ppid, bool toggle = false, bool updateNow = false);

  /**
   * Show contact view event dialog (used when chat mode is disabled)
   *
   * @param id Contact id
   * @param ppid Contact protocol id
   */
  UserViewEvent* showViewEventDialog(QString id, unsigned long ppid);

  /**
   * Show contact event dialog
   *
   * @param fcn Type of event to open
   * @param id Contact id
   * @param ppid Contact protocol id
   * @param convoId Conversation id
   * @param autoPopup True if the dialog was triggered automatically, false if triggered by the user
   */
  UserEventCommon* showEventDialog(int fcn, QString id, unsigned long ppid, int convoId = -1, bool autoPopup = false);

  /**
   * Replace event dialog
   * Called when event type is changed and dialog is replaced
   *
   * @param oldDialog Old (current) event dialog
   * @param newDialog New event dialog
   * @param id Contact id
   * @param ppid Contact protocol id
   */
  void replaceEventDialog(UserSendCommon* oldDialog, UserSendCommon* newDialog, QString id, unsigned long ppid);

  /**
   * Toggle floaty for a contact
   *
   * @param id Contact id
   * @param ppid Contact protocol id
   */
  void toggleFloaty(QString id, unsigned long ppid);

  /**
   * Trigger contact data to be reread from daemon
   * Needed since daemon won't generate signals for some changes.
   *
   * @param id Contact id
   * @param ppid Contact protocol id
   */
  void updateUserData(QString id, unsigned long ppid);

  /**
   * Set new status for all owners
   *
   * @param status New status
   * @param invisible True to set status with invisible mode active
   */
  void changeStatus(unsigned long status, bool invisible = false);

  /**
   * Set new status for an owner
   *
   * @param status New status
   * @param ppid Protocol id
   * @param invisible True to set status with invisible mode active
   */
  void changeStatus(unsigned long status, unsigned long ppid, bool invisible = false);

  /**
   * Invoke the desktop-aware URL viewer
   *
   * @param url The URL to open
   */
  void viewUrl(QString url);

public slots:
  /**
   * Write current configuration to file
   * Also makes daemon save it's configuration
   */
  void saveConfig();

  /**
   * Show next available event
   *
   * @param id Contact id or "0" for any contact
   */
  void showNextEvent(QString id = "0");

  /**
   * Open dialogs for all owner events
   */
  void showAllOwnerEvents();

  /**
   * Open dialogs for all events
   */
  void showAllEvents();

  void showDefaultEventDialog(QString id, unsigned long ppid);
  void sendMsg(QString id, unsigned long ppid, const QString& message);
  void sendFileTransfer(QString id, unsigned long ppid, const QString& filename, const QString& description);
  void sendChatRequest(QString id, unsigned long ppid);

signals:
  /**
   * Since daemon doesn't notify us when an event is sent we'll have to handle
   * it ourselfs. This is used by event dialog to notify other dialogs when a
   * message has been sent.
   *
   * @param event Event object that was sent
   */
  void eventSent(const ICQEvent* event);

private slots:
#ifdef Q_WS_X11
  void grabKey(QString key);
#endif

  void userDlgFinished(UserDlg* dialog);
  void userEventTabDlgDone();
  void userEventFinished(QString id, unsigned long ppid);
  void sendEventFinished(QString id, unsigned long ppid);
  void showMessageDialog(QString id, unsigned long ppid);
  void addEventTag(QString id, unsigned long ppid, unsigned long eventTag);

  /**
   * Act on changes to the contact list
   *
   * @param sig Signal from daemon
   */
  void listUpdated(CICQSignal* sig);

  /**
   * Act on changes to a contact
   *
   * @param sig Signal from daemon
   */
  void userUpdated(CICQSignal* sig);

  void convoSet(QString id, unsigned long ppid, unsigned long convoId);
  void convoJoin(QString id, unsigned long ppid, unsigned long convoId);
  void convoLeave(QString id, unsigned long ppid, unsigned long convoId);
  void autoAway();
  void updateDockIcon();

private:
  static LicqGui* myInstance;

  void loadGuiConfig();
  void loadFloatiesConfig();

  void createFloaty(QString id, unsigned long ppid,
    unsigned short x = 0, unsigned short y = 0, unsigned short w = 0);

  CICQDaemon* myLicqDaemon;

  QString mySkin;
  QString myIcons;
  QString myExtendedIcons;

  bool myStartHidden;
  bool myDisableDockIcon;

  // principal GUI objects
  MainWindow* myMainWindow;
  SignalManager* mySignalManager;
  LogWindow* myLogWindow;
  ContactListModel* myContactList;
  UserMenu* myUserMenu;
  GroupMenu* myGroupMenu;
  DockIcon* myDockIcon;

  // User dialogs
  UserEventTabDlg* myUserEventTabDlg;
  UserViewEventList myUserViewList;
  UserDlgList myUserDlgList;
  UserSendEventList myUserSendList;

  QStringList myCmdLineParams;
  int grabKeysym;
  QTimer myAutoAwayTimer;

#ifndef USE_KDE
  QStyle* createStyle(const char* name) const;
#endif
};

} // namespace LicqQtGui

#endif
