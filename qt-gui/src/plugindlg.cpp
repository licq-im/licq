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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qtable.h>
#include <qheader.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qdir.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtimer.h>

#include <algorithm>
#include <list>
#include <string>

#include "plugindlg.h"
#include "ewidgets.h"
#include "editfile.h"

#include "licq_icqd.h"

using std::string;
using std::list;
using std::find;

PluginDlg::PluginDlg()
  : QWidget(0, "PluginDialog", WDestructiveClose)
{
  QGroupBox *sbox = new QGroupBox(tr("Standard Plugins"), this);

  tblStandard = new QTable(1, 6, sbox);
  tblStandard->setSelectionMode(QTable::NoSelection);
  tblStandard->setShowGrid(false);
  tblStandard->setColumnReadOnly(0, true);
  tblStandard->setColumnReadOnly(1, true);
  tblStandard->setColumnReadOnly(2, true);
  tblStandard->setColumnReadOnly(5, true);
  tblStandard->setMinimumHeight(100);

  QHeader *headerStd = tblStandard->horizontalHeader();
  headerStd->setLabel(0, tr("Id"), 20);
  headerStd->setLabel(1, tr("Name"));
  headerStd->setLabel(2, tr("Version"), 50);
  headerStd->setLabel(3, tr("Load"), 32);
  headerStd->setLabel(4, tr("Enable"), 40);
  headerStd->setLabel(5, tr("Description"));

  QGroupBox *pbox = new QGroupBox(tr("Protocol Plugins"), this);

  tblProtocol = new QTable(1, 5, pbox);
  tblProtocol->setSelectionMode(QTable::NoSelection);
  tblProtocol->setShowGrid(false);
  tblProtocol->setColumnReadOnly(0, true);
  tblProtocol->setColumnReadOnly(1, true);
  tblProtocol->setColumnReadOnly(2, true);
  tblProtocol->setColumnReadOnly(4, true);
  tblProtocol->setMinimumHeight(100);

  QHeader *headerPrt = tblProtocol->horizontalHeader();
  headerPrt->setLabel(0, tr("Id"), 20);
  headerPrt->setLabel(1, tr("Name"));
  headerPrt->setLabel(2, tr("Version"), 50);
  headerPrt->setLabel(3, tr("Load"), 32);
  headerPrt->setLabel(4, tr("Description"));
 
  // Connect all the signals now
  connect(tblStandard, SIGNAL(valueChanged(int, int)), this, SLOT(slot_standard(int, int)));
  connect(tblStandard, SIGNAL(doubleClicked(int, int, int, const QPoint &)),
    this, SLOT(slot_stdConfig(int, int, int, const QPoint &)));
  connect(tblProtocol, SIGNAL(valueChanged(int, int)), this, SLOT(slot_protocol(int, int)));
  QPushButton *btnRefresh = new QPushButton(tr("Refresh"), this);
  connect(btnRefresh, SIGNAL(clicked()), this, SLOT(slot_refresh()));
  QPushButton *btnDone = new QPushButton(tr("Done"), this);
  connect(btnDone, SIGNAL(clicked()), this, SLOT(close()));

  QVBoxLayout *lay = new QVBoxLayout(this, 10, 5);
  
  QVBoxLayout *slay = new QVBoxLayout(sbox, 15);
  slay->addWidget(tblStandard);
  lay->addWidget(sbox);
  
  QVBoxLayout *play = new QVBoxLayout(pbox, 15);
  play->addWidget(tblProtocol);
  lay->addWidget(pbox);
  
  QHBoxLayout *hlay = new QHBoxLayout;
  hlay->addStretch(1);
  hlay->addWidget(btnRefresh);
  hlay->addStretch(1);
  hlay->addWidget(btnDone);
  hlay->addStretch(1);
  lay->addLayout(hlay);
  
  slot_refresh();

  show();
}

PluginDlg::~PluginDlg()
{
  emit signal_done();
}


void PluginDlg::slot_standard(int nRow, int nCol)
{
  if (nCol == 3)
  {
    //Load or Unload
    QCheckTableItem *chkLoad = dynamic_cast<QCheckTableItem *>(tblStandard->item(nRow, 3));
    if (chkLoad->isChecked())
    {
      char *sz[] = { strdup("licq"), NULL };
      QString plugin = tblStandard->text(nRow, 1);
      gLicqDaemon->PluginLoad(plugin.latin1(), 1, sz);
      free(sz[0]);
    }
    else
      gLicqDaemon->PluginShutdown(tblStandard->text(nRow, 0).toUShort());
      
    QTimer::singleShot(1000, this, SLOT(slot_refresh()));
  }
  else if (nCol == 4)
  {
    //Enable or Disable
    QCheckTableItem *chkEnable = dynamic_cast<QCheckTableItem *>(tblStandard->item(nRow, 4));
    if (chkEnable->isChecked())
      gLicqDaemon->PluginEnable(tblStandard->text(nRow, 0).toUShort());
    else
      gLicqDaemon->PluginDisable(tblStandard->text(nRow, 0).toUShort());
    
    QTimer::singleShot(1000, this, SLOT(slot_refresh()));
  }
}


void PluginDlg::slot_protocol(int nRow, int nCol)
{
  if (nCol == 3)
  {
    //Load or Unload
    QCheckTableItem *chkLoad = dynamic_cast<QCheckTableItem *>(tblProtocol->item(nRow, 3));
    if (chkLoad->isChecked())
    {
      QString plugin = tblProtocol->text(nRow, 1);   
      gLicqDaemon->ProtoPluginLoad(plugin.latin1());
    }
    else
    {
      unsigned long nPPID = 0;
      ProtoPluginsList l;
      ProtoPluginsListIter it;
      gLicqDaemon->ProtoPluginList(l);
      for (it = l.begin(); it != l.end(); it++)
      {
        if ((*it)->Id() == tblProtocol->text(nRow, 0).toUShort())
        {
          nPPID = (*it)->PPID();
          break;
        }
      }
      
      emit pluginUnloaded(nPPID);
      gLicqDaemon->ProtoPluginShutdown(tblProtocol->text(nRow, 0).toUShort());
    }
    
    QTimer::singleShot(1000, this, SLOT(slot_refresh()));
  }
}


void PluginDlg::slot_details()
{
#if 0
  if (lstLoaded->currentItem() == NULL) return;
  PluginsList l;
  PluginsListIter it;
  gLicqDaemon->PluginList(l);
  for (it = l.begin(); it != l.end(); it++)
  {
    if ((*it)->Id() == lstLoaded->currentItem()->text(0).toUShort())
      break;
  }
  if (it == l.end()) return;

  InformUser(this, tr("Licq Plugin %1 %2\n").arg( (*it)->Name() )
      .arg( (*it)->Version() ) + QString((*it)->Usage()));
#endif
}


void PluginDlg::slot_stdConfig(int nRow, int /* nCol */, int, const QPoint &)
{
  PluginsList l;
  PluginsListIter it;
  gLicqDaemon->PluginList(l);
  for (it = l.begin(); it != l.end(); it++)
  {
    if ((*it)->Id() == tblStandard->text(nRow, 0).toUShort())
      break;
  }
  if (it == l.end()) return;

  if ((*it)->ConfigFile() == NULL)
  {
    InformUser(this, tr("Plugin %1 has no configuration file").arg((*it)->Name()));
    return;
  }

  QString f;
  f.sprintf("%s/%s", BASE_DIR, (*it)->ConfigFile());
  (void) new EditFileDlg(f);  
}

void PluginDlg::slot_refresh()
{
  // Clear the tables
  while (tblStandard->numRows())
    tblStandard->removeRow(tblStandard->numRows()-1);
  while (tblProtocol->numRows())
    tblProtocol->removeRow(tblProtocol->numRows()-1);
    
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
    
    tblStandard->insertRows(i);
    tblStandard->setText(i, 0, QString::number((int)(*it)->Id()));
    tblStandard->setText(i, 1, QString((*it)->Name()));
    tblStandard->setText(i, 2, QString((*it)->Version()));
    QCheckTableItem *chkLoad = new QCheckTableItem(tblStandard, "");
    QCheckTableItem *chkEnable = new QCheckTableItem(tblStandard, "");
    chkLoad->setChecked(true);
    QString strStatus((*it)->Status());
    bool bEnabled = (strStatus.find("enable", 0, false) != -1) ||
                    (strStatus.find("running", 0, false) != -1);
    chkEnable->setChecked(bEnabled);
    tblStandard->setItem(i, 3, chkLoad);
    tblStandard->setItem(i, 4, chkEnable);
    tblStandard->setText(i, 5, QString((*it)->Description()));
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
    if (::find(lLoadedSPlugins.begin(), lLoadedSPlugins.end(), (*sit).latin1()) != lLoadedSPlugins.end())
      continue;
          
    tblStandard->insertRows(i);
    tblStandard->setText(i, 0, "*");
    tblStandard->setText(i, 1, *sit);
    tblStandard->setText(i, 2, "");
    QCheckTableItem *chkLoad = new QCheckTableItem(tblStandard, "");
    QCheckTableItem *chkEnable = new QCheckTableItem(tblStandard, "");
    tblStandard->setItem(i, 3, chkLoad);
    tblStandard->setItem(i, 4, chkEnable);
    tblStandard->setText(i, 5, tr("(Unloaded)"));
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
    if (strcmp((*p_it)->Name(), "Licq") == 0) continue;
    tblProtocol->insertRows(i);
    tblProtocol->setText(i, 0, QString::number((int)(*p_it)->Id()));
    tblProtocol->setText(i, 1, QString((*p_it)->Name()));
    tblProtocol->setText(i, 2, QString((*p_it)->Version()));
    QCheckTableItem *chkLoad = new QCheckTableItem(tblProtocol, "");
    chkLoad->setChecked(true);
    tblProtocol->setItem(i, 3, chkLoad);
    tblProtocol->setText(i, 4, "");
    i++;
  }
  
  QDir d2(LIB_DIR, "protocol_*.so", QDir::Name, QDir::Files | QDir::Readable);
  s = d2.entryList();
  for (sit = s.begin(); sit != s.end(); sit++)
  {
    (*sit).remove(0, 9);
    (*sit).truncate((*sit).length() - 3);
    if (::find(lLoadedPPlugins.begin(), lLoadedPPlugins.end(), (*sit).latin1()) != lLoadedPPlugins.end())
      continue;  
               
    tblProtocol->insertRows(i);
    tblProtocol->setText(i, 0, "*");
    tblProtocol->setText(i, 1, *sit);
    tblProtocol->setText(i, 2, "");
    QCheckTableItem *chkLoad = new QCheckTableItem(tblProtocol, "");
    tblProtocol->setItem(i, 3, chkLoad);
    tblProtocol->setText(i, 4, tr("(Unloaded)"));
    i++;    
  }
}


#include "plugindlg.moc"
