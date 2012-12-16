/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef USERDLG_OWNER_H
#define USERDLG_OWNER_H

#include <QObject>

#include "userdlg.h"

class QCheckBox;
class QComboBox;
class QLineEdit;
class QListWidget;
class QSpinBox;
class QWidget;

namespace Licq
{
class User;
class UserId;
}

namespace LicqQtGui
{
class SpecialSpinBox;

namespace UserPages
{
class Owner : public QObject
{
  Q_OBJECT

public:
  Owner(unsigned long protocolId, UserDlg* parent);
  virtual ~Owner() {}

  void load(const Licq::User* user);
  void apply(Licq::User* user);

  /**
   * User was updated
   *
   * @param user User locked for read access
   * @param subSignal Sub signal telling what the change was
   */
  void userUpdated(const Licq::User* user, unsigned long subSignal);

  /**
   * Send settings to server
   *
   * @param page to send settings for
   * @return Event tag for sent request
   */
  unsigned long send(UserDlg::UserPage page);

private:
  /**
   * Create the settings page
   *
   * @return a widget with the settings
   */
  QWidget* createPageSettings(QWidget* parent);

  /// Create ICQ security settings page
  QWidget* createPageIcqSecurity(QWidget* parent);

  /// Create ICQ random chat group setting page
  QWidget* createPageIcqChatGroup(QWidget* parent);

  unsigned long myProtocolId;
  Licq::UserId myUserId;

  // Account Settings
  QLineEdit* myAccountEdit;
  QLineEdit* myPasswordEdit;
  QCheckBox* mySavePwdCheck;
  QLineEdit* myServerHostEdit;
  QSpinBox* myServerPortSpin;
  QComboBox* myAutoLogonCombo;
  QCheckBox* myAutoLogonInvisibleCheck;

  // ICQ Settings
  QCheckBox* myReconnectAfterUinClashCheck;
  QCheckBox* mySSListCheck;
  QCheckBox* myAutoUpdateInfoCheck;
  QCheckBox* myAutoUpdateInfoPluginsCheck;
  QCheckBox* myAutoUpdateStatusPluginsCheck;

  // ICQ Security Settings
  QCheckBox* myIcqRequireAuthCheck;
  QCheckBox* myIcqWebAwareCheck;

  // ICQ Random Chat Group
  QListWidget* myIcqChatGroupList;
};

} // namespace UserPages
} // namespace LicqQtGui

#endif
