// -*- c-basic-offset: 2 -*-
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or   
    (at your option) any later version.                              
                                       
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
    GNU General Public License for more details.                 
                                                
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software      
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qlistview.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qdir.h>
#include <qstring.h>
#include <qstringlist.h>

#include "plugindlg.h"
#include "ewidgets.h"
#include "editfile.h"

#include "licq_icqd.h"

PluginDlg::PluginDlg()
  : QWidget(0, "PluginDialog", WDestructiveClose)
{
  QGroupBox *lbox = new QGroupBox(tr("Loaded"), this);

  lstLoaded = new QListView(lbox);
  lstLoaded->addColumn(tr("Id"));
  lstLoaded->addColumn(tr("Name"));
  lstLoaded->addColumn(tr("Version"));
  lstLoaded->addColumn(tr("Status"));
  lstLoaded->addColumn(tr("Description"));
  lstLoaded->setAllColumnsShowFocus (true);

  QPushButton *btnEnable = new QPushButton(tr("Enable"), lbox);
  connect(btnEnable, SIGNAL(clicked()), this, SLOT(slot_enable()));
  QPushButton *btnDisable = new QPushButton(tr("Disable"), lbox);
  connect(btnDisable, SIGNAL(clicked()), this, SLOT(slot_disable()));
  QPushButton *btnUnload = new QPushButton(tr("Unload"), lbox);
  connect(btnUnload, SIGNAL(clicked()), this, SLOT(slot_unload()));
  QPushButton *btnDetails = new QPushButton(tr("Details"), lbox);
  connect(btnDetails, SIGNAL(clicked()), this, SLOT(slot_details()));
  QPushButton *btnConfig = new QPushButton(tr("Configure"), lbox);
  connect(btnConfig, SIGNAL(clicked()), this, SLOT(slot_config()));
  int bw = 75;
  bw = QMAX(bw, btnEnable->sizeHint().width());
  bw = QMAX(bw, btnDisable->sizeHint().width());
  bw = QMAX(bw, btnUnload->sizeHint().width());
  bw = QMAX(bw, btnDetails->sizeHint().width());
  bw = QMAX(bw, btnConfig->sizeHint().width());

  QGroupBox *abox = new QGroupBox(tr("Available"), this);

  lstAvailable = new QListBox(abox);

  QPushButton *btnLoad = new QPushButton(tr("Load"), abox);
  connect(btnLoad, SIGNAL(clicked()), this, SLOT(slot_load()));
  bw = QMAX(bw, btnLoad->sizeHint().width());

  QPushButton *btnRefresh = new QPushButton(tr("Refresh"), this);
  connect(btnRefresh, SIGNAL(clicked()), this, SLOT(slot_refresh()));
  bw = QMAX(bw, btnRefresh->sizeHint().width());
  QPushButton *btnDone = new QPushButton(tr("Done"), this);
  connect(btnDone, SIGNAL(clicked()), this, SLOT(close()));
  bw = QMAX(bw, btnDone->sizeHint().width());

  btnDetails->setFixedWidth(bw);
  btnEnable->setFixedWidth(bw);
  btnDisable->setFixedWidth(bw);
  btnUnload->setFixedWidth(bw);
  btnConfig->setFixedWidth(bw);
  btnLoad->setFixedWidth(bw);
  btnDone->setFixedWidth(bw);

  QVBoxLayout *lay = new QVBoxLayout(this, 10, 5);

  QVBoxLayout *llay = new QVBoxLayout(lbox, 20);
  llay->addWidget(lstLoaded);
  QHBoxLayout *blay = new QHBoxLayout;
  blay->addStretch(1);
  blay->addWidget(btnEnable);
  blay->addStretch(1);
  blay->addWidget(btnDisable);
  blay->addStretch(1);
  blay->addWidget(btnUnload);
  blay->addStretch(1);
  llay->addLayout(blay);
  blay = new QHBoxLayout;
  blay->addStretch(1);
  blay->addWidget(btnDetails);
  blay->addStretch(1);
  blay->addWidget(btnConfig);
  blay->addStretch(1);
  llay->addLayout(blay);
  lay->addWidget(lbox);
  lay->addSpacing(15);

  QVBoxLayout *alay = new QVBoxLayout(abox, 20);
  alay->addWidget(lstAvailable);
  alay->addWidget(btnLoad);
  lay->addWidget(abox);
  lay->addSpacing(15);
  blay = new QHBoxLayout;
  blay->addStretch(1);
  blay->addWidget(btnRefresh);
  blay->addStretch(1);
  blay->addWidget(btnDone);
  blay->addStretch(1);
  lay->addLayout(blay);

  slot_refresh();

  show();
}


void PluginDlg::slot_load()
{
  if (lstAvailable->currentItem() == -1) return;

  char *sz[] = { "licq", NULL };
  gLicqDaemon->PluginLoad(lstAvailable->text(lstAvailable->currentItem()).latin1(), 1, sz);

  slot_refresh();
}


void PluginDlg::slot_details()
{
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
}


void PluginDlg::slot_enable()
{
  if (lstLoaded->currentItem() == NULL) return;
  gLicqDaemon->PluginEnable(lstLoaded->currentItem()->text(0).toUShort());

  slot_refresh();
}


void PluginDlg::slot_disable()
{
  if (lstLoaded->currentItem() == NULL) return;
  gLicqDaemon->PluginDisable(lstLoaded->currentItem()->text(0).toUShort());

  slot_refresh();
}

void PluginDlg::slot_unload()
{
  if (lstLoaded->currentItem() == NULL) return;
  gLicqDaemon->PluginShutdown(lstLoaded->currentItem()->text(0).toUShort());

  slot_refresh();
}


void PluginDlg::slot_config()
{
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
  // Load up the plugin info
  PluginsList l;
  PluginsListIter it;
  gLicqDaemon->PluginList(l);
  lstLoaded->clear();
  for (it = l.begin(); it != l.end(); it++)
  {
    (void) new QListViewItem(lstLoaded, QString::number((int)(*it)->Id()),
                     QString((*it)->Name()),
                     QString((*it)->Version()),
                     QString((*it)->Status()),
                     QString((*it)->Description()));
  }

  lstAvailable->clear();
  QDir d(LIB_DIR, "licq_*.so", QDir::Name, QDir::Files | QDir::Readable);
  QStringList s = d.entryList();
  QStringList::Iterator sit;
  for (sit = s.begin(); sit != s.end(); sit++)
  {
    (*sit).remove(0, 5);
    (*sit).truncate((*sit).length() - 3);
  }
  lstAvailable->insertStringList(s);
}


#include "plugindlg.moc"
