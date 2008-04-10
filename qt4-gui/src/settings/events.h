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

#ifndef SETTINGS_EVENTS_H
#define SETTINGS_EVENTS_H

#include <config.h>

#include <QObject>

class QCheckBox;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QVBoxLayout;
class QWidget;


namespace LicqQtGui
{
class FileNameEdit;
class SettingsDlg;

namespace Settings
{
class Events : public QObject
{
  Q_OBJECT

public:
  Events(SettingsDlg* parent);
  virtual ~Events() {}

  void load();
  void apply();

private slots:
  void setOnEventsEnabled(bool enable);

private:
  /**
   * Setup the events page.
   *
   * @return a widget with the event settings
   */
  QWidget* createPageOnEvent(QWidget* parent);

  /**
   * Setup the sound page.
   *
   * @return a widget with the sound settings
   */
  QWidget* createPageSounds(QWidget* parent);

  // Widgets for on event settings
  QVBoxLayout* myPageOnEventLayout;
  QGroupBox* myNewMsgActionsBox;
  QGroupBox* myParanoiaBox;
  QVBoxLayout* myParanoiaLayout;
  QGridLayout* myMsgActionsLayout;
  QCheckBox* myOnEventAwayCheck;
  QCheckBox* myOnEventNaCheck;
  QCheckBox* myOnEventOccupiedCheck;
  QCheckBox* myOnEventDndCheck;
  QCheckBox* myAlwaysOnlineNotifyCheck;
  QCheckBox* myBoldOnMsgCheck;
  QComboBox* myAutoPopupCombo;
  QCheckBox* myAutoRaiseCheck;
  QCheckBox* myAutoFocusCheck;
  QCheckBox* myFlashTaskbarCheck;
  QCheckBox* myFlashAllCheck;
  QCheckBox* myFlashUrgentCheck;
  QCheckBox* myIgnoreNewUsersCheck;
  QCheckBox* myIgnoreWebPanelCheck;
  QCheckBox* myIgnoreMassMsgCheck;
  QCheckBox* myIgnoreEmailPagerCheck;
  QLabel* myHotKeyLabel;
  QLineEdit* myHotKeyField;

  // Widget for sounds settings
  QVBoxLayout* myPageSoundsLayout;
  QGroupBox* myEventParamsBox;
  QGroupBox* myAcceptEventsBox;
  QGridLayout* myEventParamsLayout;
  QGridLayout* myAcceptEventsLayout;
  QCheckBox* myOnEventsCheck;
  QLabel* mySndPlayerLabel;
  QLabel* mySndMsgLabel;
  QLabel* mySndChatLabel;
  QLabel* mySndUrlLabel;
  QLabel* mySndFileLabel;
  QLabel* mySndNotifyLabel;
  QLabel* mySndSysMsgLabel;
  QLabel* mySndMsgSentLabel;
  FileNameEdit* mySndPlayerEdit;
  FileNameEdit* mySndMsgEdit;
  FileNameEdit* mySndChatEdit;
  FileNameEdit* mySndUrlEdit;
  FileNameEdit* mySndFileEdit;
  FileNameEdit* mySndNotifyEdit;
  FileNameEdit* mySndSysMsgEdit;
  FileNameEdit* mySndMsgSentEdit;
};

} // namespace Settings
} // namespace LicqQtGui

#endif
