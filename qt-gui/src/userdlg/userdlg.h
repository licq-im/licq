/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2008-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef USERDLG_H
#define USERDLG_H

#include <config.h>

#include <licq/userid.h>

#include <QDialog>
#include <QMap>
#include <QPushButton>

namespace Licq
{
class Event;
class User;
}

namespace LicqQtGui
{
class TreePager;

namespace UserPages
{
  class Info;
  class Owner;
  class Settings;
}

/**
 * Dialog for user information and per user settings.
 */
class UserDlg : public QDialog
{
  Q_OBJECT

public:
  enum UserPage
  {
    UnknownPage = -1,
    GeneralPage,
    MorePage,
    More2Page,
    WorkPage,
    AboutPage,
    PhonePage,
    PicturePage,
    CountersPage,
    SettingsPage,
    StatusPage,
    OnEventPage,
    GroupsPage,
    OwnerPage,
    OwnerSecurityPage,
    OwnerChatGroupPage,
  };

  /**
   * Open user dialog or bring up existing dialog for a user
   *
   * @param userId User to open dialog for
   * @param page Page to open or UnknownPage to not set
   * @param updateNow True to fetch info from server for current page
   */
  static void showDialog(const Licq::UserId& userId,
      UserPage page = UnknownPage, bool updateNow = false);

  /**
   * Add page to user dialog
   *
   * @param page Page id
   * @param widget Widget containg the actual page
   * @param title Page title
   * @param parent Parent page if not a top level page
   */
  void addPage(UserPage page, QWidget* widget, const QString& title, UserPage parent = UnknownPage);

private slots:
  /**
   * Save user settings and close dialog
   */
  void ok();

  /**
   * Save user settings
   * Does not close dialog
   */
  void apply();

  /**
   * Update user info from server
   */
  void retrieve();

  /**
   * Update server side user info
   */
  void send();

  /**
   * Current page has changed
   *
   * @param widget Main widget of new current page
   */
  void pageChanged(QWidget* widget);

  /**
   * User has updated
   *
   * @param userId Id for affected user
   * @param subSignal Sub signal telling what the change was
   */
  void userUpdated(const Licq::UserId& userId, unsigned long subSignal);

  /// Contact list has changed
  void listUpdated(unsigned long subSignal, int argument,
      const Licq::UserId& userId);

  /**
   * Server request has finished
   *
   * @param event Event from server with result
   */
  void doneFunction(const Licq::Event* event);

  /**
   * Show user menu
   */
  void showUserMenu();

  /**
   * Restore window title to default
   */
  void resetCaption();

private:
  static QMap<Licq::UserId, UserDlg*> myDialogs;

  /**
   * Constructor
   *
   * @param userId User id
   * @param parent Parent widget
   */
  UserDlg(const Licq::UserId& userId, QWidget* parent = 0);

  /**
   * Destructor
   */
  virtual ~UserDlg();

  /**
   * Get currenty visible page
   *
   * @return Current page
   */
  UserPage currentPage() const;

  /**
   * Select page to show
   *
   * @param page Page to show
   */
  void showPage(UserPage page);

  /**
   * Update base part of window title
   *
   * @param @user The user object
   */
  void setBasicTitle(const Licq::User* user);

  Licq::UserId myUserId;
  bool myIsOwner;
  unsigned long myIcqEventTag;
  QString myBasicTitle;
  QString myProgressMsg;

  TreePager* myPager;
  QMap<UserPage, QWidget*> myPages;
  QPushButton* myUserMenuButton;
  QPushButton* myRetrieveButton;
  QPushButton* mySendButton;

  UserPages::Info* myUserInfo;
  UserPages::Settings* myUserSettings;
  UserPages::Owner* myOwnerSettings;
};

} // namespace LicqQtGui

#endif
