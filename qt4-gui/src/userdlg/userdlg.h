// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2008 Licq developers
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

#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H

#include <config.h>

#include <QDialog>
#include <QMap>
#include <QPushButton>

class CICQSignal;
class ICQEvent;

namespace LicqQtGui
{
class TreePager;

namespace UserPages
{
  class Info;
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
    KabcPage,
    SettingsPage,
    StatusPage,
    GroupsPage,
  };

  /**
   * Constructor
   *
   * @param id User id
   * @param ppid User protocol id
   * @param parent Parent widget
   */
  UserDlg(const QString& id, unsigned long ppid, QWidget* parent = 0);

  /**
   * Destructor
   */
  virtual ~UserDlg();

  /**
   * Select page to show
   *
   * @param page Page to show
   */
  void showPage(UserPage page);

  /**
   * Get currenty visible page
   *
   * @return Current page
   */
  UserPage currentPage() const;

  /**
   * Retrive info for current page from server
   */
  void retrieveSettings() { retrieve(); }

  /**
   * Add page to user dialog
   *
   * @param page Page id
   * @param widget Widget containg the actual page
   * @param title Page title
   * @param parent Parent page if not a top level page
   */
  void addPage(UserPage page, QWidget* widget, QString title, UserPage parent = UnknownPage);

  /**
   * Get user id for dialog
   *
   * @return User id
   */
  QString id() const { return myId; }

  /**
   * Get user protocol id for dialog
   *
   * @return User protocol id
   */
  unsigned long ppid() const { return myPpid; }

signals:
  void finished(UserDlg* userDlg);

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
   * @prama sig Signal from daemon
   */
  void userUpdated(CICQSignal* sig);

  /**
   * Server request has finished
   *
   * @param event Event from server with result
   */
  void doneFunction(ICQEvent* event);

  /**
   * Show user menu
   */
  void showUserMenu();

  /**
   * Restore window title to default
   */
  void resetCaption();

private:
  QString myId;
  unsigned long myPpid;
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
};

} // namespace LicqQtGui

#endif
