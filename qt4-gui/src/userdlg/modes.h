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

#ifndef USERDLG_MODES_H
#define USERDLG_MODES_H

#include <config.h>

#include <QObject>

class QCheckBox;
class QGridLayout;
class QGroupBox;
class QRadioButton;
class QTableWidget;
class QVBoxLayout;
class QWidget;

class CICQSignal;
class ICQUser;


namespace LicqQtGui
{
class UserDlg;

namespace UserPages
{
class Modes : public QObject
{
  Q_OBJECT

public:
  Modes(bool isOwner, UserDlg* parent);
  virtual ~Modes() {}

  void load(const ICQUser* user);
  void apply(ICQUser* user);

  /**
   * Save user data for pages
   * Call this function without user locked
   *
   * @param id User id
   * @param ppid User protocol id
   */
  void apply2(const QString& id, unsigned long ppid);

  /**
   * User was updated
   *
   * @param sig Signal from daemon
   * @param user User locked for read access
   */
  void userUpdated(const CICQSignal* sig, const ICQUser* user);

private:
  /**
   * Setup the modes page.
   *
   * @return a widget with the modes settings
   */
  QWidget* createPageModes(QWidget* parent);

  /**
   * Setup the groups page
   *
   * @return a widget with the groups selection
   */
  QWidget* createPageGroups(QWidget* parent);

  bool myIsOwner;

  // Widgets for on modes page
  QGridLayout* myPageModesLayout;
  QGroupBox* myMiscModesBox;
  QGridLayout* myMiscModesLayout;
  QCheckBox* myAcceptInAwayCheck;
  QCheckBox* myAcceptInNaCheck;
  QCheckBox* myAcceptInOccupiedCheck;
  QCheckBox* myAcceptInDndCheck;
  QCheckBox* myAutoAcceptFileCheck;
  QCheckBox* myAutoAcceptChatCheck;
  QCheckBox* myAutoSecureCheck;
  QCheckBox* myUseGpgCheck;
  QCheckBox* myUseRealIpCheck;
  QGroupBox* myStatusBox;
  QVBoxLayout* myStatusLayout;
  QRadioButton* myStatusNoneRadio;
  QRadioButton* myStatusOnlineRadio;
  QRadioButton* myStatusAwayRadio;
  QRadioButton* myStatusNaRadio;
  QRadioButton* myStatusOccupiedRadio;
  QRadioButton* myStatusDndRadio;
  QGroupBox* mySysGroupBox;
  QVBoxLayout* mySysGroupLayout;
  QCheckBox* myOnlineNotifyCheck;
  QCheckBox* myVisibleListCheck;
  QCheckBox* myInvisibleListCheck;
  QCheckBox* myIgnoreListCheck;
  QCheckBox* myNewUsersCheck;

  // Widgets for groups page
  QVBoxLayout* myPageGroupsLayout;
  QGroupBox* myGroupsBox;
  QVBoxLayout* myGroupsLayout;
  QTableWidget* myGroupsTable;
};

} // namespace UserPages
} // namespace LicqQtGui

#endif
