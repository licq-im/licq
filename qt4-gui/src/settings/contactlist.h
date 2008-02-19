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

#ifndef SETTINGS_CONTACTLIST_H
#define SETTINGS_CONTACTLIST_H

#include <config.h>

#include <QObject>

#include "core/gui-defines.h"

class QCheckBox;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSpinBox;
class QVBoxLayout;
class QWidget;

namespace LicqQtGui
{
class SettingsDlg;

namespace Settings
{
class ContactList : public QObject
{
  Q_OBJECT

public:
  ContactList(SettingsDlg* parent);
  virtual ~ContactList() {}

  void load();
  void apply();

private slots:
  void colEnable(bool);

private:
  /**
   * Setup the contact list page.
   *
   * @return a widget with the contact list settings
   */
  QWidget* createPageContactList(QWidget* parent);

  /**
   * Setup the contact list columns page.
   *
   * @return a widget with the columns settings
   */
  QWidget* createPageColumns(QWidget* parent);

  /**
   * Setup the contact info page.
   *
   * @return a widget with the contact info settings
   */
  QWidget* createPageContactInfo(QWidget* parent);

  // Widget for general contact list settings
  QVBoxLayout* layPageContactList;
  QGroupBox* appearanceBox;
  QGroupBox* behaviourBox;
  QGridLayout* appearanceLayout;
  QGridLayout* behaviourLayout;
  QLabel* lblFrameStyle;
  QLabel* lblSortBy;
  QLineEdit* edtFrameStyle;
  QComboBox* cmbSortBy;
  QCheckBox* chkSSList;
  QCheckBox* chkGridLines;
  QCheckBox* chkHeader;
  QCheckBox* chkShowDividers;
  QCheckBox* chkFontStyles;
  QCheckBox* chkAlwaysShowONU;
  QCheckBox* chkScrollBar;
  QCheckBox* chkShowExtIcons;
  QCheckBox* chkSysBack;
  QCheckBox* chkShowUserIcons;
  QCheckBox* chkManualNewUser;
  QCheckBox* chkShowGroupIfNoMsg;
  QCheckBox* chkEnableMainwinMouseMovement;
  QCheckBox* chkMainWinSticky;
  QCheckBox* chkTransparent;

  // Widgets for contact list column settings
  QVBoxLayout* layPageColumns;
  QGroupBox* boxColumns;
  QGridLayout* layColumns;
  QLabel* lblColWidth;
  QLabel* lblColAlign;
  QLabel* lblColTitle;
  QLabel* lblColFormat;
  QRadioButton* rdbColNumber[MAX_COLUMNCOUNT];
  QSpinBox* spnColWidth[MAX_COLUMNCOUNT];
  QComboBox* cmbColAlign[MAX_COLUMNCOUNT];
  QLineEdit* edtColTitle[MAX_COLUMNCOUNT];
  QLineEdit* edtColFormat[MAX_COLUMNCOUNT];

  // Widgets for contact info settings
  QVBoxLayout* layPageContactInfo;
  QGroupBox* boxPopup;
  QGroupBox* boxAutoUpdate;
  QGridLayout* layPopup;
  QVBoxLayout* layAutoUpdate;
  QCheckBox* popPicture;
  QCheckBox* popAlias;
  QCheckBox* popAuth;
  QCheckBox* popName;
  QCheckBox* popEmail;
  QCheckBox* popPhone;
  QCheckBox* popFax;
  QCheckBox* popCellular;
  QCheckBox* popIP;
  QCheckBox* popLastOnline;
  QCheckBox* popOnlineSince;
  QCheckBox* popIdleTime;
  QCheckBox* popLocalTime;
  QCheckBox* popID;
  QCheckBox* chkAutoUpdateInfo;
  QCheckBox* chkAutoUpdateInfoPlugins;
  QCheckBox* chkAutoUpdateStatusPlugins;
};

} // namespace Settings
} // namespace LicqQtGui

#endif
