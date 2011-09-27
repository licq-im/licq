/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2011 Licq developers
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

#include "plugindlg.h"

#include "config.h"

#include <boost/foreach.hpp>
#include <algorithm>
#include <list>
#include <string>

#include <QDialogButtonBox>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include <licq/daemon.h>
#include <licq/plugin/pluginmanager.h>

#include "core/mainwin.h"
#include "core/messagebox.h"

#include "helpers/support.h"

#include "editfiledlg.h"

using std::string;
using std::list;
using std::find;
using Licq::gPluginManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::PluginDlg */

PluginDlg* PluginDlg::myInstance = NULL;

void PluginDlg::showPluginDlg()
{
  if (myInstance == NULL)
    myInstance = new PluginDlg();
  else
    myInstance->raise();
}

PluginDlg::PluginDlg()
{
  Support::setWidgetProps(this, "PluginDialog");
  setWindowTitle(tr("Licq - Plugin Manager"));
  setAttribute(Qt::WA_DeleteOnClose, true);

  QGroupBox* boxStandard = new QGroupBox(tr("Standard Plugins"));
  QVBoxLayout* layStandard = new QVBoxLayout(boxStandard);

  tblStandard = new QTableWidget(0, 6);
  tblStandard->setSelectionMode(QTableWidget::NoSelection);
  tblStandard->setShowGrid(false);
  tblStandard->setEditTriggers(QTableWidget::NoEditTriggers);
  tblStandard->setMinimumHeight(100);
  layStandard->addWidget(tblStandard);

  QStringList headStdLabels;
  headStdLabels << tr("Id") << tr("Name") << tr("Version") << tr("Load") << tr("Enable") << tr("Description");
  tblStandard->setHorizontalHeaderLabels(headStdLabels);
  tblStandard->verticalHeader()->hide();
  tblStandard->setWordWrap(false);

  QGroupBox* boxProtocol = new QGroupBox(tr("Protocol Plugins"), this);
  QVBoxLayout* layProtocol = new QVBoxLayout(boxProtocol);

  tblProtocol = new QTableWidget(0, 5);
  tblProtocol->setSelectionMode(QTableWidget::NoSelection);
  tblProtocol->setShowGrid(false);
  tblProtocol->setEditTriggers(QTableWidget::NoEditTriggers);
  tblProtocol->setMinimumHeight(100);
  layProtocol->addWidget(tblProtocol);

  QStringList headProtoLabels;
  headProtoLabels << tr("Id") << tr("Name") << tr("Version") << tr("Load") << tr("Description");
  tblProtocol->setHorizontalHeaderLabels(headProtoLabels);
  tblProtocol->verticalHeader()->hide();
  tblProtocol->setWordWrap(false);

  // Connect all the signals now
  connect(tblProtocol, SIGNAL(itemChanged(QTableWidgetItem*)), SLOT(slot_protocol(QTableWidgetItem*)));
  connect(tblStandard, SIGNAL(itemChanged(QTableWidgetItem*)), SLOT(slot_standard(QTableWidgetItem*)));
  connect(tblStandard, SIGNAL(cellDoubleClicked(int, int)), SLOT(slot_stdConfig(int, int)));

  QDialogButtonBox* buttons = new QDialogButtonBox();

  QPushButton* btnRefresh = new QPushButton(tr("Refresh"));
  buttons->addButton(btnRefresh, QDialogButtonBox::ActionRole);
  connect(btnRefresh, SIGNAL(clicked()), SLOT(slot_refresh()));

  QPushButton* btnDone = new QPushButton(tr("Done"));
  buttons->addButton(btnDone, QDialogButtonBox::RejectRole);
  connect(btnDone, SIGNAL(clicked()), SLOT(close()));

  QVBoxLayout* layWindow = new QVBoxLayout(this);
  layWindow->addWidget(boxStandard);
  layWindow->addWidget(boxProtocol);
  layWindow->addWidget(buttons);

  slot_refresh();

  resize(500, 400);

  show();
}

PluginDlg::~PluginDlg()
{
  myInstance = NULL;
}

Licq::GeneralPlugin::Ptr PluginDlg::getGeneralPlugin(int id)
{
  Licq::GeneralPluginsList plugins;
  gPluginManager.getGeneralPluginsList(plugins);
  BOOST_FOREACH(Licq::GeneralPlugin::Ptr plugin, plugins)
  {
    if (plugin->id() == id)
      return plugin;
  }
  return Licq::GeneralPlugin::Ptr();
}

Licq::ProtocolPlugin::Ptr PluginDlg::getProtocolPlugin(int id)
{
  Licq::ProtocolPluginsList plugins;
  gPluginManager.getProtocolPluginsList(plugins);
  BOOST_FOREACH(Licq::ProtocolPlugin::Ptr plugin, plugins)
  {
    if (plugin->id() == id)
      return plugin;
  }
  return Licq::ProtocolPlugin::Ptr();
}

void PluginDlg::slot_standard(QTableWidgetItem* item)
{
  // Signal can be raised for multiple reasons so first check if this is cell with a checkbox.
  // Also, if it is a checkbox but is not added to the list, it is also a signal that should be ignored.
  if(mapCheckboxCache.contains(item) == false)
    return;

  // Next check that the state has actually changed
  bool state = (item->checkState() == Qt::Checked);
  if(state == mapCheckboxCache[item])
    return;

  int nRow = tblStandard->row(item);
  int nCol = tblStandard->column(item);
  int index = tblStandard->item(nRow, 0)->text().toInt();
  Licq::GeneralPlugin::Ptr plugin = getGeneralPlugin(index);

  if (nCol == 3)
  {
    //Load or Unload
    if (state == true)
    {
      QString pluginName = tblStandard->item(nRow, 1)->text();
      gPluginManager.startGeneralPlugin(pluginName.toLatin1().constData(), 0, NULL);
    }
    else if (plugin.get() != NULL)
    {
      plugin->shutdown();
    }
  }
  else if (nCol == 4 && plugin.get() != NULL)
  {
    //Enable or Disable
    if (state == true)
      plugin->enable();
    else
      plugin->disable();
  }

  // Update cached state
  mapCheckboxCache[item] = state;

  // Schedule an update of the plugin list
  QTimer::singleShot(1000, this, SLOT(slot_refresh()));
}


void PluginDlg::slot_protocol(QTableWidgetItem* item)
{
  // Signal can be raised for multiple reasons so first check if this is cell with a checkbox.
  // Also, if it is a checkbox but is not added to the list, it is also a signal that should be ignored.
  if(mapCheckboxCache.contains(item) == false)
    return;

  // Next check that the state has actually changed
  bool state = (item->checkState() == Qt::Checked);
  if(state == mapCheckboxCache[item])
    return;

  int nRow = tblProtocol->row(item);
  int nCol = tblProtocol->column(item);
  int index = tblProtocol->item(nRow, 0)->text().toInt();
  Licq::ProtocolPlugin::Ptr plugin = getProtocolPlugin(index);

  if (nCol == 3)
  {
    //Load or Unload
    if (state == true)
    {
      QString pluginName = tblProtocol->item(nRow, 1)->text();
      gPluginManager.startProtocolPlugin(pluginName.toLatin1().constData());
    }
    else if (plugin.get() != NULL)
    {
      unsigned long protocolId = plugin->protocolId();

      // Daemon doesn't notify when plugins are unloaded
      // so tell mainwin directly from here
      gMainWindow->slot_pluginUnloaded(protocolId);

      plugin->shutdown();
    }
  }

  // Update cached state
  mapCheckboxCache[item] = state;

  // Schedule an update of the plugin list
  QTimer::singleShot(1000, this, SLOT(slot_refresh()));
}

void PluginDlg::slot_stdConfig(int nRow, int /* nCol */)
{
  int pluginIndex = tblStandard->item(nRow, 0)->text().toUShort();
  Licq::GeneralPlugin::Ptr plugin = getGeneralPlugin(pluginIndex);
  if (plugin.get() == NULL)
    return;

  if (plugin->configFile().empty())
  {
    InformUser(this, tr("Plugin %1 has no configuration file").arg(plugin->name().c_str()));
    return;
  }

  QString f;
  f.sprintf("%s%s", Licq::gDaemon.baseDir().c_str(), plugin->configFile().c_str());
  new EditFileDlg(f);
}

void PluginDlg::slot_refresh()
{
  // Clear the tables
  tblStandard->clearContents();
  tblStandard->setRowCount(0);
  tblProtocol->clearContents();
  tblProtocol->setRowCount(0);
  mapCheckboxCache.clear();

  // Load up the standard loaded plugin info
  Licq::GeneralPluginsList plugins;
  gPluginManager.getGeneralPluginsList(plugins);
  int i = 0;
  BOOST_FOREACH(Licq::GeneralPlugin::Ptr plugin, plugins)
  {
    tblStandard->setRowCount(i+1);
    tblStandard->setItem(i, 0, new QTableWidgetItem(QString::number(static_cast<int>(plugin->id()))));
    tblStandard->setItem(i, 1, new QTableWidgetItem(QString(plugin->name().c_str())));
    tblStandard->setItem(i, 2, new QTableWidgetItem(QString(plugin->version().c_str())));
    QTableWidgetItem* chkLoad = new QTableWidgetItem("");
    chkLoad->setFlags(chkLoad->flags() | Qt::ItemIsUserCheckable);
    QTableWidgetItem* chkEnable = new QTableWidgetItem("");
    chkEnable->setFlags(chkLoad->flags() | Qt::ItemIsUserCheckable);
    chkLoad->setCheckState(Qt::Checked);
    bool bEnabled = plugin->isEnabled();
    chkEnable->setCheckState((bEnabled ? Qt::Checked : Qt::Unchecked));
    tblStandard->setItem(i, 3, chkLoad);
    tblStandard->setItem(i, 4, chkEnable);
    tblStandard->setItem(i, 5, new QTableWidgetItem(QString(plugin->description().c_str())));
    mapCheckboxCache[chkLoad] = true;
    mapCheckboxCache[chkEnable] = bEnabled;
    i++;
  }

  // Load up the standard unloaded plugin info
  list<string> unloadedPlugins;
  gPluginManager.getAvailableGeneralPlugins(unloadedPlugins, false);
  BOOST_FOREACH(string plugin, unloadedPlugins)
  {
    tblStandard->setRowCount(i+1);
    tblStandard->setItem(i, 0, new QTableWidgetItem("*"));
    tblStandard->setItem(i, 1, new QTableWidgetItem(plugin.c_str()));
    tblStandard->setItem(i, 2, new QTableWidgetItem(""));
    QTableWidgetItem* chkLoad = new QTableWidgetItem("");
    chkLoad->setFlags(chkLoad->flags() | Qt::ItemIsUserCheckable);
    chkLoad->setCheckState(Qt::Unchecked);
    QTableWidgetItem* chkEnable = new QTableWidgetItem("");
    chkEnable->setFlags(chkLoad->flags() | Qt::ItemIsUserCheckable);
    chkEnable->setCheckState(Qt::Unchecked);
    tblStandard->setItem(i, 3, chkLoad);
    tblStandard->setItem(i, 4, chkEnable);
    tblStandard->setItem(i, 5, new QTableWidgetItem(tr("(Unloaded)")));
    mapCheckboxCache[chkLoad] = false;
    mapCheckboxCache[chkEnable] = false;
    i++;
  }

  // Load up the protocol plugin info now
  Licq::ProtocolPluginsList protocols;
  gPluginManager.getProtocolPluginsList(protocols);
  i = 0;
  BOOST_FOREACH(Licq::ProtocolPlugin::Ptr protocol, protocols)
  {
    if (protocol->name() == "ICQ")
      continue;

    tblProtocol->setRowCount(i+1);
    tblProtocol->setItem(i, 0, new QTableWidgetItem(QString::number(static_cast<int>(protocol->id()))));
    tblProtocol->setItem(i, 1, new QTableWidgetItem(QString(protocol->name().c_str())));
    tblProtocol->setItem(i, 2, new QTableWidgetItem(QString(protocol->version().c_str())));
    QTableWidgetItem* chkLoad = new QTableWidgetItem("");
    chkLoad->setFlags(chkLoad->flags() | Qt::ItemIsUserCheckable);
    chkLoad->setCheckState(Qt::Checked);
    tblProtocol->setItem(i, 3, chkLoad);
    tblProtocol->setItem(i, 4, new QTableWidgetItem(""));
    mapCheckboxCache[chkLoad] = true;
    i++;
  }

  list<string> unloadedProtocols;
  gPluginManager.getAvailableProtocolPlugins(unloadedProtocols, false);
  BOOST_FOREACH(string protocol, unloadedProtocols)
  {
    tblProtocol->setRowCount(i+1);
    tblProtocol->setItem(i, 0, new QTableWidgetItem("*"));
    tblProtocol->setItem(i, 1, new QTableWidgetItem(protocol.c_str()));
    tblProtocol->setItem(i, 2, new QTableWidgetItem(""));
    QTableWidgetItem* chkLoad = new QTableWidgetItem("");
    chkLoad->setFlags(chkLoad->flags() | Qt::ItemIsUserCheckable);
    chkLoad->setCheckState(Qt::Unchecked);
    tblProtocol->setItem(i, 3, chkLoad);
    tblProtocol->setItem(i, 4, new QTableWidgetItem(tr("(Unloaded)")));
    mapCheckboxCache[chkLoad] = false;
    i++;
  }

  tblStandard->resizeRowsToContents();
  tblProtocol->resizeRowsToContents();

  tblStandard->resizeColumnsToContents();
  tblProtocol->resizeColumnsToContents();
}
