// -*- c-basic-offset: 2 -*-
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
  layPageNetwork = new QVBoxLayout(w);
  layPageNetwork->setContentsMargins(0, 0, 0, 0);

  boxFirewall = new QGroupBox(tr("Firewall"));
  layFirewall = new QGridLayout(boxFirewall);

  chkFirewall = new QCheckBox(tr("I am behind a firewall"));
  connect(chkFirewall, SIGNAL(toggled(bool)), SLOT(slot_useFirewall(bool)));
  layFirewall->addWidget(chkFirewall, 0, 0);

  chkTCPEnabled = new QCheckBox(tr("I can receive direct connections"));
  connect(chkTCPEnabled, SIGNAL(toggled(bool)), SLOT(slot_usePortRange(bool)));
  layFirewall->addWidget(chkTCPEnabled, 1, 0);

  QHBoxLayout* layPortsIn = new QHBoxLayout();
  layPortsIn->addStretch(1);
  lblPortsIn = new QLabel(tr("Port range:"));
  lblPortsIn->setToolTip(tr("TCP port range for incoming connections."));
  layPortsIn->addWidget(lblPortsIn);
  spnPortLow = new QSpinBox();
  spnPortLow->setRange(0, 0xFFFF);
  spnPortLow->setSpecialValueText(tr("Auto"));
  lblPortsIn->setBuddy(spnPortLow);
  layPortsIn->addWidget(spnPortLow);
  lblPortsIn2 = new QLabel(tr("to"));
  layPortsIn->addWidget(lblPortsIn2);
  spnPortHigh = new QSpinBox();
  spnPortHigh->setRange(0, 0xFFFF);
  spnPortHigh->setSpecialValueText(tr("Auto"));
  lblPortsIn2->setBuddy(spnPortHigh);
  layPortsIn->addWidget(spnPortHigh);
  layFirewall->addLayout(layPortsIn, 1, 1);


  boxProxy = new QGroupBox(tr("Proxy"));
  layProxy = new QGridLayout(boxProxy);

  chkProxyEnabled = new QCheckBox(tr("Use proxy server"));
  connect(chkProxyEnabled, SIGNAL(toggled(bool)), SLOT(slot_useProxy(bool)));
  layProxy->addWidget(chkProxyEnabled, 0, 0);

  QHBoxLayout* layProxyType = new QHBoxLayout();
  layProxyType->addStretch(1);
  lblProxyType = new QLabel(tr("Proxy type:"));
  layProxyType->addWidget(lblProxyType);
  cmbProxyType = new QComboBox();
  cmbProxyType->setFixedWidth(80);
  cmbProxyType->addItem(tr("HTTPS"));
  lblProxyType->setBuddy(cmbProxyType);
  layProxyType->addWidget(cmbProxyType);
  layProxy->addLayout(layProxyType, 0, 1);

  lblProxyHost = new QLabel(tr("Proxy server:"));
  layProxy->addWidget(lblProxyHost, 1, 0);

  edtProxyHost = new QLineEdit();
  lblProxyHost->setBuddy(edtProxyHost);
  layProxy->addWidget(edtProxyHost, 1, 1);

  lblProxyPort = new QLabel(tr("Proxy server port:"));
  layProxy->addWidget(lblProxyPort, 2, 0);

  spnProxyPort = new QSpinBox();
  spnProxyPort->setRange(0, 0xFFFF);
  lblProxyPort->setBuddy(spnProxyPort);
  layProxy->addWidget(spnProxyPort, 2, 1);

  chkProxyAuthEnabled = new QCheckBox(tr("Use authorization"));
  layProxy->addWidget(chkProxyAuthEnabled, 3, 0);

  lblProxyLogin = new QLabel(tr("Username:"));
  layProxy->addWidget(lblProxyLogin, 4, 0);

  edtProxyLogin = new QLineEdit();
  lblProxyLogin->setBuddy(edtProxyLogin);
  layProxy->addWidget(edtProxyLogin, 4, 1);

  lblProxyPasswd = new QLabel(tr("Password:"));
  layProxy->addWidget(lblProxyPasswd, 5, 0);

  edtProxyPasswd = new QLineEdit();
  edtProxyPasswd->setEchoMode(QLineEdit::Password);
  lblProxyPasswd->setBuddy(edtProxyPasswd);
  layProxy->addWidget(edtProxyPasswd, 5, 1);

  connect(chkProxyAuthEnabled, SIGNAL(toggled(bool)), edtProxyLogin, SLOT(setEnabled(bool)));
  connect(chkProxyAuthEnabled, SIGNAL(toggled(bool)), edtProxyPasswd, SLOT(setEnabled(bool)));


  layPageNetwork->addWidget(boxFirewall);
  layPageNetwork->addWidget(boxProxy);
  layPageNetwork->addStretch(1);

  return w;
}

QWidget* Settings::Network::createPageIcq(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  layPageIcq = new QVBoxLayout(w);
  layPageIcq->setContentsMargins(0, 0, 0, 0);

  boxIcqServer = new QGroupBox(tr("Server Settings"));
  layIcqServer = new QGridLayout(boxIcqServer);


  lblICQServer = new QLabel(tr("ICQ server:"));
  layIcqServer->addWidget(lblICQServer, 0, 0);

  edtICQServer = new QLineEdit();
  lblICQServer->setBuddy(edtICQServer);
  layIcqServer->addWidget(edtICQServer, 0, 1);

  lblICQServerPort = new QLabel(tr("ICQ server port:"));
  layIcqServer->addWidget(lblICQServerPort, 1, 0);

  spnICQServerPort = new QSpinBox();
  spnICQServerPort->setRange(0, 0xFFFF);
  lblICQServerPort->setBuddy(spnICQServerPort);
  layIcqServer->addWidget(spnICQServerPort, 1, 1);


  boxIcqConnection = new QGroupBox(tr("Connection"));
  layIcqConnection = new QVBoxLayout(boxIcqConnection);

  chkReconnectAfterUinClash = new QCheckBox(tr("Reconnect after Uin clash"));
  chkReconnectAfterUinClash->setToolTip(tr("Licq can reconnect you when you got "
                                                "disconnected because your Uin was used "
                                                "from another location. Check this if you "
                                                "want Licq to reconnect automatically."));

  layIcqConnection->addWidget(chkReconnectAfterUinClash);


  layPageIcq->addWidget(boxIcqServer);
  layPageIcq->addWidget(boxIcqConnection);
  layPageIcq->addStretch(1);

  return w;
}

void Settings::Network::slot_useFirewall(bool b)
{
  chkTCPEnabled->setEnabled(b);
  slot_usePortRange(b && chkTCPEnabled->isChecked());
}

void Settings::Network::slot_usePortRange(bool b)
{
  spnPortLow->setEnabled(b);
  spnPortHigh->setEnabled(b);
}


void Settings::Network::slot_useProxy(bool b)
{
  if (b)
  {
    cmbProxyType->setEnabled(true);
    edtProxyHost->setEnabled(true);
    spnProxyPort->setEnabled(true);
    chkProxyAuthEnabled->setEnabled(true);
    if (chkProxyAuthEnabled->isChecked())
    {
      edtProxyLogin->setEnabled(true);
      edtProxyPasswd->setEnabled(true);
    }
    spnICQServerPort->setValue(DEFAULT_SSL_PORT);
  } else
  {
    cmbProxyType->setEnabled(false);
    edtProxyHost->setEnabled(false);
    spnProxyPort->setEnabled(false);
    chkProxyAuthEnabled->setEnabled(false);
    edtProxyLogin->setEnabled(false);
    edtProxyPasswd->setEnabled(false);
    spnICQServerPort->setValue(DEFAULT_SERVER_PORT);
  }
}

void Settings::Network::load()
{
  edtICQServer->setText(QString(gLicqDaemon->ICQServer()));
  spnICQServerPort->setValue(gLicqDaemon->ICQServerPort());
  chkFirewall->setChecked(gLicqDaemon->Firewall());
  chkTCPEnabled->setChecked(gLicqDaemon->TCPEnabled());
  spnPortLow->setValue(gLicqDaemon->TCPPortsLow());
  spnPortHigh->setValue(gLicqDaemon->TCPPortsHigh());

  if (!gLicqDaemon->Firewall())
  {
    chkTCPEnabled->setEnabled(false);
    spnPortLow->setEnabled(false);
    spnPortHigh->setEnabled(false);
  }
  else
  {
    if (!gLicqDaemon->TCPEnabled())
    {
      spnPortLow->setEnabled(false);
      spnPortHigh->setEnabled(false);
    }
  }

  chkProxyEnabled->setChecked(gLicqDaemon->ProxyEnabled());
  cmbProxyType->setCurrentIndex(gLicqDaemon->ProxyType() - 1);
  edtProxyHost->setText(QString(gLicqDaemon->ProxyHost()));
  spnProxyPort->setValue(gLicqDaemon->ProxyPort());
  chkProxyAuthEnabled->setChecked(gLicqDaemon->ProxyAuthEnabled());
  edtProxyLogin->setText(QString(gLicqDaemon->ProxyLogin()));
  edtProxyPasswd->setText(QString(gLicqDaemon->ProxyPasswd()));

  chkReconnectAfterUinClash->setChecked(gLicqDaemon->ReconnectAfterUinClash());

  if (!gLicqDaemon->ProxyEnabled())
  {
    cmbProxyType->setEnabled(false);
    edtProxyHost->setEnabled(false);
    spnProxyPort->setEnabled(false);
    chkProxyAuthEnabled->setEnabled(false);
    edtProxyLogin->setEnabled(false);
    edtProxyPasswd->setEnabled(false);
  } else if (!gLicqDaemon->ProxyAuthEnabled())
  {
    edtProxyLogin->setEnabled(false);
    edtProxyPasswd->setEnabled(false);
  }
}

void Settings::Network::apply()
{
  gLicqDaemon->SetICQServer(edtICQServer->text().toLocal8Bit());
  gLicqDaemon->SetICQServerPort(spnICQServerPort->value());
  gLicqDaemon->SetTCPPorts(spnPortLow->value(), spnPortHigh->value());
  gLicqDaemon->SetTCPEnabled(chkTCPEnabled->isChecked());
  gLicqDaemon->SetFirewall(chkFirewall->isChecked());
  gLicqDaemon->SetProxyEnabled(chkProxyEnabled->isChecked());
  gLicqDaemon->SetProxyType(cmbProxyType->currentIndex() + 1);
  gLicqDaemon->SetProxyHost(edtProxyHost->text().toLocal8Bit());
  gLicqDaemon->SetProxyPort(spnProxyPort->value());
  gLicqDaemon->SetProxyAuthEnabled(chkProxyAuthEnabled->isChecked());
  gLicqDaemon->SetProxyLogin(edtProxyLogin->text().toLocal8Bit());
  gLicqDaemon->SetProxyPasswd(edtProxyPasswd->text().toLocal8Bit());

  gLicqDaemon->setReconnectAfterUinClash(chkReconnectAfterUinClash->isChecked());
}
