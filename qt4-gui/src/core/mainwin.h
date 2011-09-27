/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2011 Licq developers
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

#ifndef MAINWIN_H
#define MAINWIN_H

#include "config.h"

#ifdef USE_KDE
# include <KDE/KMenuBar>
#else
# include <QMenuBar>
#endif

#include <QBitmap>
#include <QByteArray>
#include <QCloseEvent>
#include <QDialog>
#include <QKeyEvent>
#include <QList>
#include <QMouseEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QWidget>

class QAction;
class QMenu;
class QPixmap;
class QPushButton;
class QStyle;
class QTextEdit;

namespace Licq
{
class Event;
class UserId;
}

namespace LicqQtGui
{
class RegisterUserDlg;
class SkinnableButton;
class SkinnableComboBox;
class SkinnableLabel;
class SystemMenu;
class UserEventCommon;
class UserView;

#ifdef USE_KDE
class LicqKIMIface;
#endif

//=====MainWindow===============================================================
class MainWindow : public QWidget
{
  Q_OBJECT
  friend class LicqGui;

public:
  MainWindow(bool bStartHidden, QWidget* parent = 0);
  virtual ~MainWindow();

  UserView* getUserView() { return myUserView; }
  SystemMenu* systemMenu() { return mySystemMenu; }

  QString usprintfHelp;

#ifdef USE_KDE
    LicqKIMIface* kdeIMInterface;
#endif

  // Functions
  void CreateUserView();

  virtual void resizeEvent(QResizeEvent*);
  virtual void moveEvent(QMoveEvent*);
  virtual void mouseMoveEvent(QMouseEvent*);
  virtual void mousePressEvent(QMouseEvent*);
  virtual void closeEvent(QCloseEvent*);

public slots:
  void slot_shutdown();

  /**
   * Contact list has changed
   *
   * @param subSignal Sub signal telling what the change was
   */
  void slot_updatedList(unsigned long subSignal);

  void slot_pluginUnloaded(unsigned long ppid);
  void updateGroups(bool initial = false);

  void showHints();
  void updateCurrentGroup();
  void showStats();
  void showAwayMsgDlg();
  void showAboutBox();
  void showAutoResponseHints(QWidget* parent = 0);
  void hide();

private:
  QString myCaption;
  bool myInMiniMode;

  int myMouseX;
  int myMouseY;

  SystemMenu* mySystemMenu;

  QAction* myViewEventAction;
  QAction* mySendMessageAction;
  QAction* mySendUrlAction;
  QAction* mySendFileAction;
  QAction* mySendChatRequestAction;
  QAction* myCheckUserArAction;
  QAction* myViewHistoryAction;

  // Widgets
  UserView* myUserView;
#ifdef USE_KDE
  KMenuBar* myMenuBar;
#else
  QMenuBar* myMenuBar;
#endif
  SkinnableLabel* myStatusField;
  SkinnableLabel* myMessageField;
  SkinnableButton* mySystemButton;
  SkinnableComboBox* myUserGroupsBox;

  void saveGeometry();

private slots:
  void updateConfig();
  void updateSkin();
  void updateEvents();

  /**
   * Our status has changed
   */
  void updateStatus();

  /**
   * Update shortcuts
   */
  void updateShortcuts();

  /**
   * Groups combo box was changed to show a new group
   *
   * @param index Combo box index of selected item
   */
  void setCurrentGroup(int index);

  /**
   * Switch to next group in group list
   */
  void nextGroup();

  /**
   * Switch to previous group in group list
   */
  void prevGroup();

  void slot_logon();
  void slot_protocolPlugin(unsigned long);
  void slot_updateContactList();
  void slot_updatedUser(const Licq::UserId& userId, unsigned long subSignal, int argument);

  /**
   * Open add user dialog
   *
   * @param userId User to add
   */
  void addUser(const Licq::UserId& userId);

  void setMiniMode(bool miniMode);
  void setMainwinSticky(bool sticky = true);
  void trayIconClicked();
  void removeUserFromList();
  void removeUserFromGroup();
  void callUserFunction(QAction* action);
  void checkUserAutoResponse();

  /**
   * Show history dialog for selected user
   */
  void showUserHistory();
};

// -----------------------------------------------------------------------------

extern MainWindow* gMainWindow;

} // namespace LicqQtGui

#endif
