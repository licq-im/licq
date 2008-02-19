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

#ifndef SETTINGS_STATUS_H
#define SETTINGS_STATUS_H

#include <config.h>

#include <QObject>

class QCheckBox;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QVBoxLayout;

namespace LicqQtGui
{
class MLEdit;
class SettingsDlg;

namespace Settings
{
class Status : public QObject
{
  Q_OBJECT

public:
  Status(SettingsDlg* parent);
  virtual ~Status() {}

  void load();
  void apply();

private slots:
  void slot_SARmsg_act(int);
  void slot_SARgroup_act(int);
  void slot_SARsave_act();
  void slot_SARhints();

private:
  /**
   * Setup the status page.
   *
   * @return a widget with the status settings
   */
  QWidget* createPageStatus(QWidget* parent);

  /**
   * Setup the response messages page.
   *
   * @return a widget with the response message settings
   */
  QWidget* createPageRespMsg(QWidget* parent);

  void buildAutoStatusCombos(bool);

  // Widgets for status settings
  QVBoxLayout* layPageStatus;
  QGroupBox* boxAutoLogon;
  QGroupBox* boxAutoAway;
  QVBoxLayout* layAutoLogon;
  QGridLayout* layAutoAway;
  QComboBox* cmbAutoLogon;
  QCheckBox* chkAutoLogonInvisible;
  QLabel* lblAutoAway;
  QLabel* lblAutoNa;
  QLabel* lblAutoOffline;
  QSpinBox* spnAutoAway;
  QSpinBox* spnAutoNa;
  QSpinBox* spnAutoOffline;
  QComboBox* cmbAutoAwayMess;
  QComboBox* cmbAutoNAMess;

  // Widgets for response message settings
  QVBoxLayout* layPageRespMsg;
  QGroupBox* boxDefRespMsg;
  QGridLayout* layDefRespMsg;
  QLabel* lblSARgroup;
  QLabel* lblSARmsg;
  QComboBox* cmbSARgroup;
  QComboBox* cmbSARmsg;
  MLEdit* edtSARtext;
  QPushButton* btnSARsave;
  QPushButton* btnSARhints;
};

} // namespace Settings
} // namespace LicqQtGui

#endif
