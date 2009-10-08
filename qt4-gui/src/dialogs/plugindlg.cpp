// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#include <algorithm>
#include <list>
#include <string>

#include <QDialogButtonBox>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include <licq_icqd.h>

#include "core/mainwin.h"
#include "core/messagebox.h"

#include "helpers/support.h"

#include "editfiledlg.h"

using std::string;
using std::list;
using std::find;
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

  if (nCol == 3)
  {
    //Load or Unload
    if (state == true)
    {
      char* sz[] = { strdup("licq"), NULL };
      QString plugin = tblStandard->item(nRow, 1)->text();
      gLicqDaemon->PluginLoad(plugin.toLatin1(), 1, sz);
      free(sz[0]);
    }
    else
    {
      gLicqDaemon->PluginShutdown(index);
    }
  }
  else if (nCol == 4)
  {
    //Enable or Disable
    if (state == true)
      gLicqDaemon->PluginEnable(index);
    else
      gLicqDaemon->PluginDisable(index);
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

  if (nCol == 3)
  {
    //Load or Unload
    if (state == true)
    {
      QString plugin = tblProtocol->item(nRow, 1)->text();
      gLicqDaemon->ProtoPluginLoad(plugin.toLatin1());
    }
    else
    {
      unsigned long nPPID = 0;
      ProtoPluginsList l;
      ProtoPluginsListIter it;
      gLicqDaemon->ProtoPluginList(l);
      for (it = l.begin(); it != l.end(); it++)
      {
        if ((*it)->Id() == index)
        {
          nPPID = (*it)->PPID();
          break;
        }
      }

      // Daemon doesn't notify when plugins are unloaded
      // so tell mainwin directly from here
      gMainWindow->slot_pluginUnloaded(nPPID);

      gLicqDaemon->ProtoPluginShutdown(index);
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

  PluginsList l;
  PluginsListIter it;
  gLicqDaemon->PluginList(l);
  for (it = l.begin(); it != l.end(); it++)
  {
    if ((*it)->Id() == pluginIndex)
      break;
  }
  if (it == l.end()) return;

  if ((*it)->ConfigFile() == NULL)
  {
    InformUser(this, tr("Plugin %1 has no configuration file").arg((*it)->Name()));
    return;
  }

  QString f;
  f.sprintf("%s%s", BASE_DIR, (*it)->ConfigFile());
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

  list< string > lLoadedSPlugins;
  list< string > lLoadedPPlugins;

  // Load up the standard loaded plugin info
  PluginsList l;
  PluginsListIter it;
  int i = 0;
  gLicqDaemon->PluginList(l);
  for (it = l.begin(); it != l.end(); it++)
  {
    lLoadedSPlugins.push_back((*it)->LibName());

    tblStandard->setRowCount(i+1);
    tblStandard->setItem(i, 0, new QTableWidgetItem(QString::number(static_cast<int>((*it)->Id()))));
    tblStandard->setItem(i, 1, new QTableWidgetItem(QString((*it)->Name())));
    tblStandard->setItem(i, 2, new QTableWidgetItem(QString((*it)->Version())));
    QTableWidgetItem* chkLoad = new QTableWidgetItem("");
    chkLoad->setFlags(chkLoad->flags() | Qt::ItemIsUserCheckable);
    QTableWidgetItem* chkEnable = new QTableWidgetItem("");
    chkEnable->setFlags(chkLoad->flags() | Qt::ItemIsUserCheckable);
    chkLoad->setCheckState(Qt::Checked);
    QString strStatus((*it)->Status());
    bool bEnabled = (strStatus.indexOf("enable", 0, Qt::CaseInsensitive) != -1) ||
                    (strStatus.indexOf("running", 0, Qt::CaseInsensitive) != -1);
    chkEnable->setCheckState((bEnabled ? Qt::Checked : Qt::Unchecked));
    tblStandard->setItem(i, 3, chkLoad);
    tblStandard->setItem(i, 4, chkEnable);
    tblStandard->setItem(i, 5, new QTableWidgetItem(QString((*it)->Description())));
    mapCheckboxCache[chkLoad] = true;
    mapCheckboxCache[chkEnable] = bEnabled;
    i++;
  }

  // Load up the standard unloaded plugin info
  QDir d(LIB_DIR, "licq_*.so", QDir::Name, QDir::Files | QDir::Readable);
  QStringList s = d.entryList();
  QStringList::Iterator sit;
  for (sit = s.begin(); sit != s.end(); sit++)
  {
    (*sit).remove(0, 5);
    (*sit).truncate((*sit).length() - 3);
    if (::find(lLoadedSPlugins.begin(), lLoadedSPlugins.end(),
               (*sit).toAscii().constData()) != lLoadedSPlugins.end())
      continue;

    tblStandard->setRowCount(i+1);
    tblStandard->setItem(i, 0, new QTableWidgetItem("*"));
    tblStandard->setItem(i, 1, new QTableWidgetItem(*sit));
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
  ProtoPluginsList p_l;
  ProtoPluginsListIter p_it;
  i = 0;
  gLicqDaemon->ProtoPluginList(p_l);
  for (p_it = p_l.begin(); p_it != p_l.end(); p_it++)
  {
    lLoadedPPlugins.push_back((*p_it)->LibName());
    if (strcmp((*p_it)->Name(), "Licq") == 0)
      continue;

    tblProtocol->setRowCount(i+1);
    tblProtocol->setItem(i, 0, new QTableWidgetItem(QString::number(static_cast<int>((*p_it)->Id()))));
    tblProtocol->setItem(i, 1, new QTableWidgetItem(QString((*p_it)->Name())));
    tblProtocol->setItem(i, 2, new QTableWidgetItem(QString((*p_it)->Version())));
    QTableWidgetItem* chkLoad = new QTableWidgetItem("");
    chkLoad->setFlags(chkLoad->flags() | Qt::ItemIsUserCheckable);
    chkLoad->setCheckState(Qt::Checked);
    tblProtocol->setItem(i, 3, chkLoad);
    tblProtocol->setItem(i, 4, new QTableWidgetItem(""));
    mapCheckboxCache[chkLoad] = true;
    i++;
  }

  QDir d2(LIB_DIR, "protocol_*.so", QDir::Name, QDir::Files | QDir::Readable);
  s = d2.entryList();
  for (sit = s.begin(); sit != s.end(); sit++)
  {
    (*sit).remove(0, 9);
    (*sit).truncate((*sit).length() - 3);
    if (::find(lLoadedPPlugins.begin(), lLoadedPPlugins.end(),
               (*sit).toAscii().constData()) != lLoadedPPlugins.end())
      continue;

    tblProtocol->setRowCount(i+1);
    tblProtocol->setItem(i, 0, new QTableWidgetItem("*"));
    tblProtocol->setItem(i, 1, new QTableWidgetItem(*sit));
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
