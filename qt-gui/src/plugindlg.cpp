#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qlistview.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qgroupbox.h>

#include "plugindlg.h"

#include "icqd.h"

PluginDlg::PluginDlg(CICQDaemon *d) : QWidget(0, 0)
{
  licqDaemon = d;

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
  int bw = 75;
  bw = QMAX(bw, btnEnable->sizeHint().width());
  bw = QMAX(bw, btnDisable->sizeHint().width());
  bw = QMAX(bw, btnUnload->sizeHint().width());

  QGroupBox *abox = new QGroupBox(tr("Available"), this);

  lstAvailable = new QListView(abox);
  lstAvailable->addColumn(tr("Name"));
  lstLoaded->setAllColumnsShowFocus (true);

  QPushButton *btnLoad = new QPushButton(tr("Load"), abox);
  connect(btnLoad, SIGNAL(clicked()), this, SLOT(slot_load()));
  bw = QMAX(bw, btnLoad->sizeHint().width());

  QPushButton *btnRefresh = new QPushButton(tr("Refresh"), this);
  connect(btnRefresh, SIGNAL(clicked()), this, SLOT(slot_refresh()));
  bw = QMAX(bw, btnRefresh->sizeHint().width());
  QPushButton *btnDone = new QPushButton(tr("Done"), this);
  connect(btnDone, SIGNAL(clicked()), this, SLOT(hide()));
  bw = QMAX(bw, btnDone->sizeHint().width());

  btnEnable->setFixedWidth(bw);
  btnDisable->setFixedWidth(bw);
  btnUnload->setFixedWidth(bw);
  btnLoad->setFixedWidth(bw);
  btnDone->setFixedWidth(bw);

  QVBoxLayout *lay = new QVBoxLayout(this, 10, 5);

  QVBoxLayout *llay = new QVBoxLayout(lbox, 20, 5);
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
  lay->addWidget(lbox);
  lay->addSpacing(15);

  QVBoxLayout *alay = new QVBoxLayout(abox, 20, 5);
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


void PluginDlg::slot_load() {}


void PluginDlg::slot_enable()
{
  if (lstLoaded->currentItem() == NULL) return;
  licqDaemon->PluginEnable(lstLoaded->currentItem()->text(0).toUShort());
}


void PluginDlg::slot_disable()
{
  if (lstLoaded->currentItem() == NULL) return;
  licqDaemon->PluginDisable(lstLoaded->currentItem()->text(0).toUShort());
}

void PluginDlg::slot_unload()
{
  if (lstLoaded->currentItem() == NULL) return;
  licqDaemon->PluginShutdown(lstLoaded->currentItem()->text(0).toUShort());
}


void PluginDlg::slot_refresh()
{
  // Load up the plugin info
  PluginsList l;
  PluginsListIter it;
  licqDaemon->PluginList(l);
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
  //...

}


void PluginDlg::hide()
{
  QWidget::hide();
  delete this;
}


#include "moc/moc_plugindlg.h"
