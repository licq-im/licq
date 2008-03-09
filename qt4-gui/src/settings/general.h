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
  void slot_useDockToggled(bool);
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
  QVBoxLayout* layPageDocking;
  QGroupBox* boxDocking;
  QGridLayout* layDocking;
  QCheckBox* chkUseDock;
  QCheckBox* chkHidden;
  QCheckBox* chkDockFortyEight;
  QCheckBox* chkDockTrayBlink;
  QCheckBox* chkDockTrayMessage;
  QRadioButton* rdbDockDefault;
  QRadioButton* rdbDockThemed;
  QRadioButton* rdbDockTray;
  QComboBox* cmbDockTheme;

  // Widgets for fonts and localization settings
  QVBoxLayout* layPageFonts;
  QGroupBox* boxFont;
  QGroupBox* boxLocale;
  QGridLayout* layFont;
  QVBoxLayout* layLocale;
  QLabel* lblFont;
  QLabel* lblEditFont;
  FontEdit* myNormalFontEdit;
  FontEdit* myEditFontEdit;
  QLabel* lblDefaultEncoding;
  QCheckBox* chkShowAllEncodings;
  QComboBox* cmbDefaultEncoding;
};

} // namespace Settings
} // namespace LicqQtGui

#endif
