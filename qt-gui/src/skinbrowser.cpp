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
#include <qlistbox.h>
#include <qpushbutton.h>

#include "skinbrowser.h"
#include "skin.h"
#include "mainwin.h"
#include "constants.h"
#include "log.h"

SkinBrowserDlg::SkinBrowserDlg(CMainWindow *_mainwin, QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  mainwin = _mainwin;

  grpSkins = new QGroupBox(tr("Skins"), this);
  lstSkins = new QListBox(grpSkins);
  btnApplySkin = new QPushButton(tr("Apply"), grpSkins);
  grpIcons = new QGroupBox(tr("Icons"), this);
  lstIcons = new QListBox(grpIcons);
  btnApplyIcons = new QPushButton(tr("Apply"), grpIcons);
  btnDone = new QPushButton(tr("Done"), this);
  setCaption(tr("Licq Skin Browser"));

  // Load up the available packs
  QString szDir;
  szDir.sprintf("%s%s", SHARE_DIR, QTGUI_DIR);
  QDir dPlugins(szDir, "skin.*", QDir::Name | QDir::IgnoreCase, QDir::Dirs);
  if (!dPlugins.count())
  {
    gLog.Error("%sError reading qt-gui directory %s.\n", L_ERRORxSTR, szDir.latin1());
    lstSkins->insertItem(tr("ERROR"));
    lstSkins->setEnabled(false);
  }
  else
  {
    QStringList::Iterator it;
    QStringList lst = dPlugins.entryList();
    for (it = lst.begin(); it != lst.end(); ++it)
      lstSkins->insertItem((*it).mid(5));
  }
  QDir dIcons(szDir, "icons.*", QDir::Name | QDir::IgnoreCase, QDir::Dirs);
  if (!dIcons.count())
  {
    gLog.Error("%sError reading qt-gui directory %s.\n", L_ERRORxSTR, szDir.latin1());
    lstSkins->insertItem(tr("ERROR"));
    lstSkins->setEnabled(false);
  }
  else
  {
    QStringList::Iterator it;
    QStringList lst = dIcons.entryList();
    for (it = lst.begin(); it != lst.end(); ++it)
        lstIcons->insertItem((*it).mid(6));
  }
  setGeometry(mainwin->x() - 100, mainwin->y() + 100, 300,
              150 + (lstSkins->count() > lstIcons->count() ? lstSkins->count() : lstIcons->count()) * 15);

  connect(btnApplySkin, SIGNAL(clicked()), this, SLOT(slot_applyskin()));
  connect(btnApplyIcons, SIGNAL(clicked()), this, SLOT(slot_applyicons()));
  connect(btnDone, SIGNAL(clicked()), this, SLOT(hide()));
}


void SkinBrowserDlg::slot_applyskin()
{
  if (lstSkins->currentItem() == -1) return;
  mainwin->ApplySkin(lstSkins->text(lstSkins->currentItem()));
}


void SkinBrowserDlg::slot_applyicons()
{
  if (lstIcons->currentItem() == -1) return;
  mainwin->ApplyIcons(lstIcons->text(lstIcons->currentItem()));
}


void SkinBrowserDlg::hide()
{
   QWidget::hide();
   delete this;
}


void SkinBrowserDlg::resizeEvent (QResizeEvent *)
{
  grpSkins->setGeometry(10, 10, width() / 2 - 20, height() - 60);
  lstSkins->setGeometry(10, 20, grpSkins->width() - 20, grpSkins->height() - 70);
  btnApplySkin->setGeometry(grpSkins->width() / 2 - 50, grpSkins->height() - 40, 100, 30);
  grpIcons->setGeometry(width() / 2 + 5, 10, width() / 2 - 20, height() - 60);
  lstIcons->setGeometry(10, 20, grpIcons->width() - 20, grpIcons->height() - 70);
  btnApplyIcons->setGeometry(grpIcons->width() / 2 - 50, grpIcons->height() - 40, 100, 30);
  btnDone->setGeometry(width() / 2 - 50, height() - 40, 100, 30);
}

#include "moc/moc_skinbrowser.h"
