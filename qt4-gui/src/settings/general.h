/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2009 Licq developers
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

#ifndef SETTINGS_GENERAL_H
#define SETTINGS_GENERAL_H

#include <config.h>

#include <QObject>

class QCheckBox;
class QComboBox;
class QFont;
class QGridLayout;
class QGroupBox;
class QLabel;
class QRadioButton;
class QVBoxLayout;
class QWidget;

namespace LicqQtGui
{
class FontEdit;
class SettingsDlg;

namespace Settings
{

class General : public QObject
{
  Q_OBJECT

public:
  General(SettingsDlg* parent);
  virtual ~General() {}

  void load();
  void apply();

private slots:
  void useDockToggled(bool useDock);
  void normalFontChanged(const QFont& font);

private:
  /**
   * Setup the docking settings page.
   *
   * @return a widget with the docking settings
   */
  QWidget* createPageDocking(QWidget* parent);

  /**
   * Setup the fonts and localization settings page.
   *
   * @return a widget with the fonts and localization settings
   */
  QWidget* createPageFonts(QWidget* parent);

  // Widgets for docking settings
  QVBoxLayout* myPageDockingLayout;
  QGroupBox* myDockingBox;
  QGridLayout* myDockingLayout;
  QCheckBox* myUseDockCheck;
  QCheckBox* myHiddenCheck;
  QCheckBox* myDockFortyEightCheck;
  QCheckBox* myDockTrayBlinkCheck;
  QCheckBox* myTrayMsgOnlineNotify;
  QRadioButton* myDockDefaultRadio;
  QRadioButton* myDockThemedRadio;
  QRadioButton* myDockTrayRadio;
  QComboBox* myDockThemeCombo;

  // Widgets for fonts and localization settings
  QVBoxLayout* myPageFontsLayout;
  QGroupBox* myFontBox;
  QGroupBox* myLocaleBox;
  QGridLayout* myFontLayout;
  QVBoxLayout* myLocaleLayout;
  QLabel* myFontLabel;
  QLabel* myEditFontLabel;
  QLabel* myHistoryFontLabel;
  QLabel* myFixedFontLabel;
  FontEdit* myNormalFontEdit;
  FontEdit* myEditFontEdit;
  FontEdit* myHistoryFontEdit;
  FontEdit* myFixedFontEdit;
  QLabel* myDefaultEncodingLabel;
  QCheckBox* myShowAllEncodingsCheck;
  QComboBox* myDefaultEncodingCombo;
};

} // namespace Settings
} // namespace LicqQtGui

#endif
