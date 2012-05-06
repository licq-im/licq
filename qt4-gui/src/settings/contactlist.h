/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2012 Licq developers <licq-dev@googlegroups.com>
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
class ShortcutEdit;

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
  void numColumnsChanged();

private:
  /**
   * Setup the contact list page.
   *
   * @return a widget with the contact list settings
   */
  QWidget* createPageContactList(QWidget* parent);

  /**
   * Setup the contact info page.
   *
   * @return a widget with the contact info settings
   */
  QWidget* createPageContactInfo(QWidget* parent);

  // Widget for general contact list settings
  QVBoxLayout* myPageContactListLayout;
  QGroupBox* myAppearanceBox;
  QGroupBox* myBehaviourBox;
  QGridLayout* myAppearanceLayout;
  QGridLayout* myBehaviourLayout;
  QLabel* myFrameStyleLabel;
  QLabel* mySortByLabel;
  QLineEdit* myFrameStyleEdit;
  QComboBox* mySortByCombo;
  QCheckBox* myGridLinesCheck;
  QCheckBox* myHeaderCheck;
  QCheckBox* myShowDividersCheck;
  QCheckBox* myMode2ViewCheck;
  QCheckBox* myFontStylesCheck;
  QCheckBox* myAlwaysShowONUCheck;
  QCheckBox* myScrollBarCheck;
  QCheckBox* myShowExtIconsCheck;
  QCheckBox* myShowPhoneIconsCheck;
  QCheckBox* mySysBackCheck;
  QCheckBox* myShowUserIconsCheck;
  QCheckBox* myManualNewUserCheck;
  QCheckBox* myShowGroupIfNoMsgCheck;
  QCheckBox* myEnableMainwinMouseMovementCheck;
  QCheckBox* myMainWinStickyCheck;
  QCheckBox* myTransparentCheck;
  QCheckBox* myDragMovesUserCheck;
  QLabel* myGuiStyleLabel;
  QComboBox* myGuiStyleCombo;
#ifdef Q_WS_X11
  QLabel* myHotKeyLabel;
  ShortcutEdit* myHotKeyEdit;
#endif

  // Widgets for contact list column settings
  QGroupBox* myColumnsBox;
  QGridLayout* myColumnsLayout;
  QLabel* myColWidthLabel;
  QLabel* myColAlignLabel;
  QLabel* myColTitleLabel;
  QLabel* myColFormatLabel;
  QRadioButton* myColNumberRadio[MAX_COLUMNCOUNT];
  QSpinBox* myColWidthSpin[MAX_COLUMNCOUNT];
  QComboBox* myColAlignCombo[MAX_COLUMNCOUNT];
  QLineEdit* myColTitleEdit[MAX_COLUMNCOUNT];
  QLineEdit* myColFormatEdit[MAX_COLUMNCOUNT];

  // Widgets for contact info settings
  QGroupBox* myPopupBox;
  QGridLayout* myPopupLayout;
  QCheckBox* myPopupPictureCheck;
  QCheckBox* myPopupAliasCheck;
  QCheckBox* myPopupAuthCheck;
  QCheckBox* myPopupNameCheck;
  QCheckBox* myPopupEmailCheck;
  QCheckBox* myPopupPhoneCheck;
  QCheckBox* myPopupFaxCheck;
  QCheckBox* myPopupCellularCheck;
  QCheckBox* myPopupIpCheck;
  QCheckBox* myPopupLastOnlineCheck;
  QCheckBox* myPopupOnlineSinceCheck;
  QCheckBox* myPopupAwayTimeCheck;
  QCheckBox* myPopupIdleTimeCheck;
  QCheckBox* myPopupLocalTimeCheck;
  QCheckBox* myPopupIdCheck;
};

} // namespace Settings
} // namespace LicqQtGui

#endif
