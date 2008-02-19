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
  void slot_chkOnEventsToggled(bool);

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
  QVBoxLayout* layPageOnEvent;
  QGroupBox* boxNewMsgActions;
  QGroupBox* boxParanoia;
  QVBoxLayout* layParanoia;
  QGridLayout* myMsgActionsLayout;
  QCheckBox* chkOEAway;
  QCheckBox* chkOENA;
  QCheckBox* chkOEOccupied;
  QCheckBox* chkOEDND;
  QCheckBox* chkAlwaysOnlineNotify;
  QCheckBox* chkBoldOnMsg;
  QCheckBox* chkAutoPopup;
  QCheckBox* chkAutoRaise;
  QCheckBox* chkAutoFocus;
  QCheckBox* chkFlashTaskbar;
  QCheckBox* chkFlashAll;
  QCheckBox* chkFlashUrgent;
  QCheckBox* chkIgnoreNewUsers;
  QCheckBox* chkIgnoreWebPanel;
  QCheckBox* chkIgnoreMassMsg;
  QCheckBox* chkIgnoreEmailPager;
  QLabel* myHotKeyLabel;
  QLineEdit* myHotKeyField;

  // Widget for sounds settings
  QVBoxLayout* layPageSounds;
  QGroupBox* boxEventParams;
  QGroupBox* boxAcceptEvents;
  QGridLayout* layEventParams;
  QGridLayout* layAcceptEvents;
  QCheckBox* chkOnEvents;
  QLabel* lblSndPlayer;
  QLabel* lblSndMsg;
  QLabel* lblSndChat;
  QLabel* lblSndUrl;
  QLabel* lblSndFile;
  QLabel* lblSndNotify;
  QLabel* lblSndSysMsg;
  QLabel* lblSndMsgSent;
  FileNameEdit* edtSndPlayer;
  FileNameEdit* edtSndMsg;
  FileNameEdit* edtSndChat;
  FileNameEdit* edtSndUrl;
  FileNameEdit* edtSndFile;
  FileNameEdit* edtSndNotify;
  FileNameEdit* edtSndSysMsg;
  FileNameEdit* edtSndMsgSent;
};

} // namespace Settings
} // namespace LicqQtGui

#endif
