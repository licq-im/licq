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

#include <qdir.h>
#include <qgroupbox.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvbox.h>

#include "skinbrowser.h"
#include "skin.h"
#include "mainwin.h"
#include "licq_constants.h"
#include "licq_log.h"
#include "editfile.h"

SkinBrowserDlg::SkinBrowserDlg(CMainWindow *_mainwin, QWidget *parent)
  : QWidget(parent, "SkinBrowserDialog")
{
  mainwin = _mainwin;

  QBoxLayout* t_lay = new QVBoxLayout(this, 8, 4);

  grpSkins = new QGroupBox(1, Vertical, tr("Skins"), this);
  t_lay->addWidget(grpSkins);

  lstSkins = new QListView(grpSkins);
  lstSkins->addColumn(tr("Name"), 180);
  lstSkins->setMinimumWidth(195);
#if QT_VERSION >= 210
  lstSkins->setShowSortIndicator(true);
#else
  lstSkins->setMinimumSize(lstSkins->sizeHint());
#endif

  QVBox* btn1 = new QVBox(grpSkins);
  btnApplySkin = new QPushButton(tr("&Apply"), btn1);
  btnEditSkin = new QPushButton(tr("&Edit"), btn1);

  grpIcons = new QGroupBox(1, Vertical, tr("Icons"), this);
  t_lay->addWidget(grpIcons);
  lstIcons = new QListView(grpIcons);
  lstIcons->addColumn(tr("Name"), 180);
  lstIcons->setMinimumWidth(195);
#if QT_VERSION >= 210
  lstIcons->setShowSortIndicator(true);
#else
  lstIcons->setMinimumSize(lstIcons->sizeHint());
#endif

  QVBox* btn2 = new QVBox(grpIcons);
  btnApplyIcons = new QPushButton(tr("A&pply"), btn2);

  QBoxLayout* lay = new QHBoxLayout(t_lay);
  lay->addStretch(1);
  btnDone = new QPushButton(tr("&Done"), this);
  btnDone->setMinimumWidth(75);
  btnDone->setDefault(true);
  lay->addWidget(btnDone);

  setCaption(tr("Licq Skin Browser"));

  // Load up the available packs
  QString szDir;
  szDir.sprintf("%s%s", SHARE_DIR, QTGUI_DIR);
  QDir dPlugins(szDir, "skin.*", QDir::Name | QDir::IgnoreCase, QDir::Dirs);
  if (!dPlugins.count())
  {
    gLog.Error("%sError reading qt-gui directory %s.\n", L_ERRORxSTR, szDir.latin1());
    (void) new QListViewItem(lstSkins, tr("Error"));
    lstSkins->setEnabled(false);
  }
  else
  {
    QStringList::Iterator it;
    QStringList lst = dPlugins.entryList();
    for (it = lst.begin(); it != lst.end(); ++it)
      (void) new QListViewItem(lstSkins, (*it).mid(5));
  }
  QDir dIcons(szDir, "icons.*", QDir::Name | QDir::IgnoreCase, QDir::Dirs);
  if (!dIcons.count())
  {
    gLog.Error("%sError reading qt-gui directory %s.\n", L_ERRORxSTR, szDir.latin1());
    (void) new QListViewItem(lstIcons, tr("Error"));
    lstSkins->setEnabled(false);
  }
  else
  {
    QStringList::Iterator it;
    QStringList lst = dIcons.entryList();
    for (it = lst.begin(); it != lst.end(); ++it)
      (void) new QListViewItem(lstIcons, (*it).mid(6));
  }

  connect(btnApplySkin, SIGNAL(clicked()), this, SLOT(slot_applyskin()));
  connect(btnEditSkin, SIGNAL(clicked()), this, SLOT(slot_editskin()));
  connect(btnApplyIcons, SIGNAL(clicked()), this, SLOT(slot_applyicons()));
  connect(btnDone, SIGNAL(clicked()), this, SLOT(close()));
}


void SkinBrowserDlg::slot_applyskin()
{
  if (!lstSkins->currentItem()) return;
  mainwin->ApplySkin(lstSkins->currentItem()->text(0).local8Bit());
}


void SkinBrowserDlg::slot_applyicons()
{
  if (!lstIcons->currentItem()) return;
  mainwin->ApplyIcons(lstIcons->currentItem()->text(0).local8Bit());
}


void SkinBrowserDlg::slot_editskin()
{
  if (!lstSkins->currentItem()) return;
  QString f;
  f.sprintf("%sqt-gui/skin.%s/%s.skin", SHARE_DIR,
            QFile::encodeName(lstSkins->currentItem()->text(0)).data(),
            QFile::encodeName(lstSkins->currentItem()->text(0)).data());
  (void) new EditFileDlg(f);
}

#include "skinbrowser.moc"
