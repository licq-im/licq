// -*- c-basic-offset: 2 -*-
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

#include "network.h"

#include "config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>

#include <licq_constants.h>
#include <licq_icqd.h>

#include "settingsdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Settings::Network */

Settings::Network::Network(SettingsDlg* parent)
  : QObject(parent)
{
  parent->addPage(SettingsDlg::NetworkPage, createPageNetwork(parent),
      tr("Network"));
  parent->addPage(SettingsDlg::IcqPage, createPageIcq(parent),
      tr("ICQ"), SettingsDlg::NetworkPage);

  load();
}

QWidget* Settings::Network::createPageNetwork(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageNetworkLayout = new QVBoxLayout(w);
  myPageNetworkLayout->setContentsMargins(0, 0, 0, 0);

  myFirewallBox = new QGroupBox(tr("Firewall"));
  myFirewallLayout = new QGridLayout(myFirewallBox);

  myFirewallCheck = new QCheckBox(tr("I am behind a firewall"));
  connect(myFirewallCheck, SIGNAL(toggled(bool)), SLOT(useFirewallToggled(bool)));
  myFirewallLayout->addWidget(myFirewallCheck, 0, 0);

  myTcpEnabledCheck = new QCheckBox(tr("I can receive direct connections"));
  connect(myTcpEnabledCheck, SIGNAL(toggled(bool)), SLOT(usePortRangeToggled(bool)));
  myFirewallLayout->addWidget(myTcpEnabledCheck, 1, 0);

  QHBoxLayout* myPortsInLayout = new QHBoxLayout();
  myPortsInLayout->addStretch(1);
  myPortsInLabel = new QLabel(tr("Port range:"));
  myPortsInLabel->setToolTip(tr("TCP port range for incoming connections."));
  myPortsInLayout->addWidget(myPortsInLabel);
  myPortLowSpin = new QSpinBox();
  myPortLowSpin->setRange(0, 0xFFFF);
  myPortLowSpin->setSpecialValueText(tr("Auto"));
  myPortsInLabel->setBuddy(myPortLowSpin);
  myPortsInLayout->addWidget(myPortLowSpin);
  myPortsIn2Label = new QLabel(tr("to"));
  myPortsInLayout->addWidget(myPortsIn2Label);
  myPortHighSpin = new QSpinBox();
  myPortHighSpin->setRange(0, 0xFFFF);
  myPortHighSpin->setSpecialValueText(tr("Auto"));
  myPortsIn2Label->setBuddy(myPortHighSpin);
  myPortsInLayout->addWidget(myPortHighSpin);
  myFirewallLayout->addLayout(myPortsInLayout, 1, 1);


  myProxyBox = new QGroupBox(tr("Proxy"));
  myProxyLayout = new QGridLayout(myProxyBox);

  myProxyEnabledCheck = new QCheckBox(tr("Use proxy server"));
  connect(myProxyEnabledCheck, SIGNAL(toggled(bool)), SLOT(useProxyToggled(bool)));
  myProxyLayout->addWidget(myProxyEnabledCheck, 0, 0);

  QHBoxLayout* myProxyTypeLayout = new QHBoxLayout();
  myProxyTypeLayout->addStretch(1);
  myProxyTypeLabel = new QLabel(tr("Proxy type:"));
  myProxyTypeLayout->addWidget(myProxyTypeLabel);
  myProxyTypeCombo = new QComboBox();
  myProxyTypeCombo->setFixedWidth(80);
  myProxyTypeCombo->addItem(tr("HTTPS"));
  myProxyTypeLabel->setBuddy(myProxyTypeCombo);
  myProxyTypeLayout->addWidget(myProxyTypeCombo);
  myProxyLayout->addLayout(myProxyTypeLayout, 0, 1);

  myProxyHostLabel = new QLabel(tr("Proxy server:"));
  myProxyLayout->addWidget(myProxyHostLabel, 1, 0);

  myProxyHostEdit = new QLineEdit();
  myProxyHostLabel->setBuddy(myProxyHostEdit);
  myProxyLayout->addWidget(myProxyHostEdit, 1, 1);

  myProxyPortLabel = new QLabel(tr("Proxy server port:"));
  myProxyLayout->addWidget(myProxyPortLabel, 2, 0);

  myProxyPortSpin = new QSpinBox();
  myProxyPortSpin->setRange(0, 0xFFFF);
  myProxyPortLabel->setBuddy(myProxyPortSpin);
  myProxyLayout->addWidget(myProxyPortSpin, 2, 1);

  myProxyAuthEnabledCheck = new QCheckBox(tr("Use authorization"));
  myProxyLayout->addWidget(myProxyAuthEnabledCheck, 3, 0);

  myProxyLoginLabel = new QLabel(tr("Username:"));
  myProxyLayout->addWidget(myProxyLoginLabel, 4, 0);

  myProxyLoginEdit = new QLineEdit();
  myProxyLoginLabel->setBuddy(myProxyLoginEdit);
  myProxyLayout->addWidget(myProxyLoginEdit, 4, 1);

  myProxyPasswdLabel = new QLabel(tr("Password:"));
  myProxyLayout->addWidget(myProxyPasswdLabel, 5, 0);

  myProxyPasswdEdit = new QLineEdit();
  myProxyPasswdEdit->setEchoMode(QLineEdit::Password);
  myProxyPasswdLabel->setBuddy(myProxyPasswdEdit);
  myProxyLayout->addWidget(myProxyPasswdEdit, 5, 1);

  connect(myProxyAuthEnabledCheck, SIGNAL(toggled(bool)), myProxyLoginEdit, SLOT(setEnabled(bool)));
  connect(myProxyAuthEnabledCheck, SIGNAL(toggled(bool)), myProxyPasswdEdit, SLOT(setEnabled(bool)));


  myPageNetworkLayout->addWidget(myFirewallBox);
  myPageNetworkLayout->addWidget(myProxyBox);
  myPageNetworkLayout->addStretch(1);

  return w;
}

QWidget* Settings::Network::createPageIcq(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageIcqLayout = new QVBoxLayout(w);
  myPageIcqLayout->setContentsMargins(0, 0, 0, 0);

  myIcqServerBox = new QGroupBox(tr("Server Settings"));
  myIcqServerLayout = new QGridLayout(myIcqServerBox);


  myIcqServerLabel = new QLabel(tr("ICQ server:"));
  myIcqServerLayout->addWidget(myIcqServerLabel, 0, 0);

  myIcqServerEdit = new QLineEdit();
  myIcqServerLabel->setBuddy(myIcqServerEdit);
  myIcqServerLayout->addWidget(myIcqServerEdit, 0, 1);

  myIcqServerPortLabel = new QLabel(tr("ICQ server port:"));
  myIcqServerLayout->addWidget(myIcqServerPortLabel, 1, 0);

  myIcqServerPortSpin = new QSpinBox();
  myIcqServerPortSpin->setRange(0, 0xFFFF);
  myIcqServerPortLabel->setBuddy(myIcqServerPortSpin);
  myIcqServerLayout->addWidget(myIcqServerPortSpin, 1, 1);


  myIcqConnectionBox = new QGroupBox(tr("Connection"));
  myIcqConnectionLayout = new QVBoxLayout(myIcqConnectionBox);

  myReconnectAfterUinClashCheck = new QCheckBox(tr("Reconnect after Uin clash"));
  myReconnectAfterUinClashCheck->setToolTip(tr("Licq can reconnect you when you got "
        "disconnected because your Uin was used "
        "from another location. Check this if you "
        "want Licq to reconnect automatically."));

  myIcqConnectionLayout->addWidget(myReconnectAfterUinClashCheck);


  myPageIcqLayout->addWidget(myIcqServerBox);
  myPageIcqLayout->addWidget(myIcqConnectionBox);
  myPageIcqLayout->addStretch(1);

  return w;
}

void Settings::Network::useFirewallToggled(bool useFirewall)
{
  myTcpEnabledCheck->setEnabled(useFirewall);
  usePortRangeToggled(useFirewall && myTcpEnabledCheck->isChecked());
}

void Settings::Network::usePortRangeToggled(bool usePortRange)
{
  myPortLowSpin->setEnabled(usePortRange);
  myPortHighSpin->setEnabled(usePortRange);
}

void Settings::Network::useProxyToggled(bool useProxy)
{
  if (useProxy)
  {
    myProxyTypeCombo->setEnabled(true);
    myProxyHostEdit->setEnabled(true);
    myProxyPortSpin->setEnabled(true);
    myProxyAuthEnabledCheck->setEnabled(true);
    if (myProxyAuthEnabledCheck->isChecked())
    {
      myProxyLoginEdit->setEnabled(true);
      myProxyPasswdEdit->setEnabled(true);
    }
    myIcqServerPortSpin->setValue(DEFAULT_SSL_PORT);
  }
  else
  {
    myProxyTypeCombo->setEnabled(false);
    myProxyHostEdit->setEnabled(false);
    myProxyPortSpin->setEnabled(false);
    myProxyAuthEnabledCheck->setEnabled(false);
    myProxyLoginEdit->setEnabled(false);
    myProxyPasswdEdit->setEnabled(false);
    myIcqServerPortSpin->setValue(DEFAULT_SERVER_PORT);
  }
}

void Settings::Network::load()
{
  myIcqServerEdit->setText(QString(gLicqDaemon->ICQServer()));
  myFirewallCheck->setChecked(gLicqDaemon->Firewall());
  myTcpEnabledCheck->setChecked(gLicqDaemon->TCPEnabled());
  myPortLowSpin->setValue(gLicqDaemon->TCPPortsLow());
  myPortHighSpin->setValue(gLicqDaemon->TCPPortsHigh());

  if (!gLicqDaemon->Firewall())
  {
    myTcpEnabledCheck->setEnabled(false);
    myPortLowSpin->setEnabled(false);
    myPortHighSpin->setEnabled(false);
  }
  else
  {
    if (!gLicqDaemon->TCPEnabled())
    {
      myPortLowSpin->setEnabled(false);
      myPortHighSpin->setEnabled(false);
    }
  }

  myProxyEnabledCheck->setChecked(gLicqDaemon->ProxyEnabled());
  myProxyTypeCombo->setCurrentIndex(gLicqDaemon->ProxyType() - 1);
  myProxyHostEdit->setText(QString(gLicqDaemon->ProxyHost()));
  myProxyPortSpin->setValue(gLicqDaemon->ProxyPort());
  myProxyAuthEnabledCheck->setChecked(gLicqDaemon->ProxyAuthEnabled());
  myProxyLoginEdit->setText(QString(gLicqDaemon->ProxyLogin()));
  myProxyPasswdEdit->setText(QString(gLicqDaemon->ProxyPasswd()));

  // Set server port after myProxyEnabledCheck as it will trigger useProxyToggled
  // which overwrites server port field.
  myIcqServerPortSpin->setValue(gLicqDaemon->ICQServerPort());

  myReconnectAfterUinClashCheck->setChecked(gLicqDaemon->ReconnectAfterUinClash());

  if (!gLicqDaemon->ProxyEnabled())
  {
    myProxyTypeCombo->setEnabled(false);
    myProxyHostEdit->setEnabled(false);
    myProxyPortSpin->setEnabled(false);
    myProxyAuthEnabledCheck->setEnabled(false);
    myProxyLoginEdit->setEnabled(false);
    myProxyPasswdEdit->setEnabled(false);
  } else if (!gLicqDaemon->ProxyAuthEnabled())
  {
    myProxyLoginEdit->setEnabled(false);
    myProxyPasswdEdit->setEnabled(false);
  }
}

void Settings::Network::apply()
{
  gLicqDaemon->SetICQServer(myIcqServerEdit->text().toLocal8Bit());
  gLicqDaemon->SetICQServerPort(myIcqServerPortSpin->value());
  gLicqDaemon->SetTCPPorts(myPortLowSpin->value(), myPortHighSpin->value());
  gLicqDaemon->SetTCPEnabled(myTcpEnabledCheck->isChecked());
  gLicqDaemon->SetFirewall(myFirewallCheck->isChecked());
  gLicqDaemon->SetProxyEnabled(myProxyEnabledCheck->isChecked());
  gLicqDaemon->SetProxyType(myProxyTypeCombo->currentIndex() + 1);
  gLicqDaemon->SetProxyHost(myProxyHostEdit->text().toLocal8Bit());
  gLicqDaemon->SetProxyPort(myProxyPortSpin->value());
  gLicqDaemon->SetProxyAuthEnabled(myProxyAuthEnabledCheck->isChecked());
  gLicqDaemon->SetProxyLogin(myProxyLoginEdit->text().toLocal8Bit());
  gLicqDaemon->SetProxyPasswd(myProxyPasswdEdit->text().toLocal8Bit());

  gLicqDaemon->setReconnectAfterUinClash(myReconnectAfterUinClashCheck->isChecked());
}
