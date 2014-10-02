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

#ifndef USERDLG_MODES_H
#define USERDLG_MODES_H

#include <config.h>

#include <QObject>

class QCheckBox;
class QGridLayout;
class QGroupBox;
class QHBoxLayout;
class QPushButton;
class QRadioButton;
class QTableWidget;
class QVBoxLayout;
class QWidget;

namespace Licq
{
class User;
class UserId;
}

namespace LicqQtGui
{
class MLEdit;
class OnEventBox;
class UserDlg;

namespace UserPages
{
class Settings : public QObject
{
  Q_OBJECT

public:
  Settings(UserDlg* parent);
  virtual ~Settings() {}

  void load(const Licq::User* user);
  void apply(Licq::User* user);

  /**
   * Save user data for pages
   * Call this function without user locked
   *
   * @param userId User id
   */
  void apply2(const Licq::UserId& userId);

  /**
   * User was updated
   *
   * @param user User locked for read access
   * @param subSignal Sub signal telling what the change was
   */
  void userUpdated(const Licq::User* user, unsigned long subSignal);

private slots:
  /**
   * Show hints for auto response editing
   */
  void showAutoRespHints();

private:
  /**
   * Setup the modes page.
   *
   * @return a widget with the modes settings
   */
  QWidget* createPageSettings(QWidget* parent);

  /**
   * Setup the status page.
   *
   * @return a widget with the status settings
   */
  QWidget* createPageStatus(QWidget* parent);

  /**
   * Setup the sounds page
   *
   * @return a widget with the sounds settings
   */
  QWidget* createPageOnEvent(QWidget* parent);

  /**
   * Setup the groups page
   *
   * @return a widget with the groups selection
   */
  QWidget* createPageGroups(QWidget* parent);

  // Widgets for on modes page
  QGridLayout* myPageModesLayout;
  QGroupBox* mySettingsBox;
  QGridLayout* mySettingsLayout;
  QCheckBox* myAcceptInAwayCheck;
  QCheckBox* myAcceptInNaCheck;
  QCheckBox* myAcceptInOccupiedCheck;
  QCheckBox* myAcceptInDndCheck;
  QCheckBox* myAutoAcceptFileCheck;
  QCheckBox* myAutoAcceptChatCheck;
  QCheckBox* myAutoSecureCheck;
  QCheckBox* myUseGpgCheck;
  QCheckBox* myUseRealIpCheck;

  // Status page
  QGridLayout* myPageStatusLayout;
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
  QCheckBox* myNewUserCheck;
  QGroupBox* myAutoRespBox;
  QHBoxLayout* myAutoRespLayout;
  MLEdit* myAutoRespEdit;
  QPushButton* myAutoRespClearButton;
  QPushButton* myAutoRespHintsButton;

  // Sounds page
  QVBoxLayout* myPageOnEventLayout;
  OnEventBox* myOnEventBox;

  // Widgets for groups page
  QVBoxLayout* myPageGroupsLayout;
  QGroupBox* myGroupsBox;
  QVBoxLayout* myGroupsLayout;
  QTableWidget* myGroupsTable;
};

} // namespace UserPages
} // namespace LicqQtGui

#endif
