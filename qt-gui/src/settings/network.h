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
class SpecialSpinBox;

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
  void useProxyToggled(bool useProxy);
  void useFirewallToggled(bool useFirewall);
  void usePortRangeToggled(bool usePortRange);

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
  QVBoxLayout* myPageNetworkLayout;
  QGroupBox* myFirewallBox;
  QGroupBox* myProxyBox;
  QGridLayout* myFirewallLayout;
  QGridLayout* myProxyLayout;
  QCheckBox* myFirewallCheck;
  QCheckBox* myTcpEnabledCheck;
  QLabel* myPortsInLabel;
  QLabel* myPortsIn2Label;
  SpecialSpinBox* myPortLowSpin;
  SpecialSpinBox* myPortHighSpin;
  QLabel* myProxyTypeLabel;
  QLabel* myProxyHostLabel;
  QLabel* myProxyPortLabel;
  QLabel* myProxyLoginLabel;
  QLabel* myProxyPasswdLabel;
  QCheckBox* myProxyEnabledCheck;
  QCheckBox* myProxyAuthEnabledCheck;
  QComboBox* myProxyTypeCombo;
  QLineEdit* myProxyHostEdit;
  QLineEdit* myProxyLoginEdit;
  QLineEdit* myProxyPasswdEdit;
  QSpinBox* myProxyPortSpin;
};

} // namespace Settings
} // namespace LicqQtGui

#endif
