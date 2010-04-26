/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#include <licq_types.h>

class QMimeData;

class LicqEvent;

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

  int Run();

  // Functions to get daemon and gui resources
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
   * @param userId Contact id
   * @param parent Parent window to use for confirmation box or NULL to use mainwin
   * @return true if contact was removed
   */
  bool removeUserFromList(const UserId& userId, QWidget* parent = NULL);

  /**
   * Show contact info dialog
   *
   * @param fcn Tab to show
   * @param userId Contact id
   * @param toggle True to close dialog if already open
   * @param updateNow True to make the dialog contents update
   */
  void showInfoDialog(int fcn, const UserId& userId, bool toggle = false, bool updateNow = false);

  /**
   * Show contact view event dialog (used when chat mode is disabled)
   *
   * @param userId Contact id
   */
  UserViewEvent* showViewEventDialog(const UserId& userid);

  /**
   * Show contact event dialog
   *
   * @param fcn Type of event to open
   * @param userId Contact id
   * @param convoId Conversation id
   * @param autoPopup True if the dialog was triggered automatically, false if triggered by the user
   */
  UserEventCommon* showEventDialog(int fcn, const UserId& userId, int convoId = -1, bool autoPopup = false);

  /**
   * Replace event dialog
   * Called when event type is changed and dialog is replaced
   *
   * @param oldDialog Old (current) event dialog
   * @param newDialog New event dialog
   * @param userId Contact id
   */
  void replaceEventDialog(UserSendCommon* oldDialog, UserSendCommon* newDialog, const UserId& userId);

  /**
   * Toggle floaty for a contact
   *
   * @param userId Contact id
   */
  void toggleFloaty(const UserId& userId);

  /**
   * Set new status for all owners
   *
   * @param status New status
   * @param invisible True to set status with invisible mode active
   * @param autoMessage Auto response message to set
   */
  void changeStatus(unsigned status, bool invisible = false, const QString& autoMessage = QString());

  /**
   * Set new status for an owner
   *
   * @param status New status
   * @param userId Owner to change status for
   * @param invisible True to set status with invisible mode active
   * @param autoMessage Auto response message to set
   */
  void changeStatus(unsigned status, const Licq::UserId& userId, bool invisible = false, const QString& autoMessage = QString());

  /**
   * Invoke the desktop-aware URL viewer
   *
   * @param url The URL to open
   */
  void viewUrl(const QString& url);

public slots:
  /**
   * Write current configuration to file
   * Also makes daemon save it's configuration
   */
  void saveConfig();

  /**
   * Show next available event
   *
   * @param userId Contact id or negative for any contact
   */
  void showNextEvent(const Licq::UserId& userId = Licq::UserId());

  /**
   * Open dialogs for all owner events
   */
  void showAllOwnerEvents();

  /**
   * Open dialogs for all events
   */
  void showAllEvents();

  void showDefaultEventDialog(const Licq::UserId& userId);

  /**
   * Open a send message dialog and set message text
   *
   * @param userId User to send message to
   * @param message Text to put in input area
   */
  void sendMsg(const Licq::UserId& userId, const QString& message);

  /**
   * Open a file transfer dialog for a specified file
   *
   * @param userId User to send file to
   * @param filename Path to file to sendof
   * @param description Text to put in description area
   */
  void sendFileTransfer(const Licq::UserId& userId, const QString& filename, const QString& description);

  /**
   * Open a chat request dialog
   *
   * @param userId User to open chat request dialog for
   */
  void sendChatRequest(const Licq::UserId& userId);

  /**
   * Act on object being dropped on a user
   * This is a common function to handle drops both in contact list and in dialogs
   *
   * @param userId User data was dropped on
   * @param mimeData Dropped data
   * @return true if data was accepted
   */
  bool userDropEvent(const Licq::UserId& userId, const QMimeData& mimeData);

signals:
  /**
   * Since daemon doesn't notify us when an event is sent we'll have to handle
   * it ourselfs. This is used by event dialog to notify other dialogs when a
   * message has been sent.
   *
   * @param event Event object that was sent
   */
  void eventSent(const LicqEvent* event);

private slots:
#ifdef Q_WS_X11
  void grabKey(const QString& key);
#endif

  void userDlgFinished(UserDlg* dialog);
  void userEventTabDlgDone();

  /**
   * A view user event dialog has finished
   *
   * @param userId User dialog was opened for
   */
  void userEventFinished(const Licq::UserId& userId);

  /**
   * A send user event dialog has finished
   *
   * @param userId User dialog was opened for
   */
  void sendEventFinished(const Licq::UserId& userId);

  /**
   * Open a message dialog
   *
   * @param userId User to open dialog for
   */
  void showMessageDialog(const Licq::UserId& userId);

  /**
   * Act on changes to the contact list
   *
   * @param subSignal Sub signal telling what the change was
   * @param argument Additional data, usage depend on sub signal type
   * @param userId Id for affected user, if applicable
   */
  void listUpdated(unsigned long subSignal, int argument, const Licq::UserId& userId);

  /**
   * Act on changes to a contact
   *
   * @param userId Id for affected user
   * @param subSignal Sub signal telling what the change was
   * @param argument Additional data, usage depend on sub signal type
   * @param cid Conversation id
   */
  void userUpdated(const Licq::UserId& userId, unsigned long subSignal, int argument, unsigned long cid);

  /**
   * Set conversation id for user event dialog
   *
   * @param userId User to find dialog for
   * @param convoId Conversation id to set
   */
  void convoSet(const Licq::UserId& userId, unsigned long convoId);

  /**
   * Someone joined an ongoing conversation
   *
   * @param userId User that joined conversation
   * @param ppid Protocol of conversation
   * @param convoId Id of conversation
   */
  void convoJoin(const Licq::UserId& userId, unsigned long ppid, unsigned long convoId);

  /**
   * Someone left an ongoing conversation
   *
   * @param userId User that left conversation
   * @param ppid Protocol of conversation
   * @param convoId Id of conversation
   */
  void convoLeave(const Licq::UserId& userId, unsigned long ppid, unsigned long convoId);
  void autoAway();
  void updateDockIcon();

private:
  static LicqGui* myInstance;

  void loadGuiConfig();
  void loadFloatiesConfig();

  void createFloaty(const UserId& userId, int x = 0, int y = 0, int w = 0);

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
};

} // namespace LicqQtGui

#endif
