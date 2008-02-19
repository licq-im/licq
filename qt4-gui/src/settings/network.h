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

#ifndef SETTINGS_NETWORK_H
#define SETTINGS_NETWORK_H

#include <config.h>

#include <QObject>

class QCheckBox;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QVBoxLayout;
class QWidget;

namespace LicqQtGui
{
class SettingsDlg;

namespace Settings
{
class Network : public QObject
{
  Q_OBJECT

public:
  Network(SettingsDlg* parent);

  void load();
  void apply();

private slots:
  void slot_useProxy(bool);
  void slot_useFirewall(bool);
  void slot_usePortRange(bool b);

private:
  /**
   * Setup the network page.
   *
   * @return a widget with the network settings
   */
  QWidget* createPageNetwork(QWidget* parent);

  /**
   * Setup the ICQ configuration page.
   *
   * @return a widget with the icq network settings
   */
  QWidget* createPageIcq(QWidget* parent);

  // Widgets for network settings
  QVBoxLayout* layPageNetwork;
  QGroupBox* boxFirewall;
  QGroupBox* boxProxy;
  QGridLayout* layFirewall;
  QGridLayout* layProxy;
  QCheckBox* chkFirewall;
  QCheckBox* chkTCPEnabled;
  QLabel* lblPortsIn;
  QLabel* lblPortsIn2;
  QSpinBox* spnPortLow;
  QSpinBox* spnPortHigh;
  QLabel* lblProxyType;
  QLabel* lblProxyHost;
  QLabel* lblProxyPort;
  QLabel* lblProxyLogin;
  QLabel* lblProxyPasswd;
  QCheckBox* chkProxyEnabled;
  QCheckBox* chkProxyAuthEnabled;
  QCheckBox* chkReconnectAfterUinClash;
  QComboBox* cmbProxyType;
  QLineEdit* edtProxyHost;
  QLineEdit* edtProxyLogin;
  QLineEdit* edtProxyPasswd;
  QSpinBox* spnProxyPort;

  // Widgets for icq network settings
  QVBoxLayout* layPageIcq;
  QGroupBox* boxIcqServer;
  QGroupBox* boxIcqConnection;
  QGridLayout* layIcqServer;
  QVBoxLayout* layIcqConnection;
  QLabel* lblICQServer;
  QLabel* lblICQServerPort;
  QLineEdit* edtICQServer;
  QSpinBox* spnICQServerPort;
};

} // namespace Settings
} // namespace LicqQtGui

#endif
