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

#include <qlistbox.h>
#include <qlineedit.h>
#include <qwhatsthis.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtoolbutton.h>

#include "editgrp.h"
#include "ewidgets.h"
#include "user.h"

EditGrpDlg::EditGrpDlg(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  QGridLayout *lay = new QGridLayout(this, 2, 1, 15, 10);
  grpGroups = new QGroupBox(tr("Groups"), this);
  lay->addWidget(grpGroups, 0, 0);

  QGridLayout *glay = new QGridLayout(grpGroups, 4, 3, 20, 5);
  lstGroups = new QListBox(grpGroups);
  glay->addMultiCellWidget(lstGroups, 0, 0, 0, 1);

  QVBoxLayout *vlay = new QVBoxLayout(glay);
  btnAdd = new QPushButton(tr("Add"), grpGroups);
  btnRemove = new QPushButton(tr("Remove"), grpGroups);
  btnUp = new QPushButton(tr("Shift Up"), grpGroups);
  btnDown = new QPushButton(tr("Shift Down"), grpGroups);
  btnEdit = new QPushButton(tr("Edit Name"), grpGroups);
  QWhatsThis::add(btnEdit, tr("Edit group name (hit enter to save)."));
  btnDefault = new QPushButton(tr("Default"), grpGroups);
  QWhatsThis::add(btnDefault, tr("The default group to start up in."));
  btnNewUser = new QPushButton(tr("New Users"), grpGroups);
  QWhatsThis::add(btnNewUser, tr("The group to which new users will be automatically added."));
  vlay->addWidget(btnAdd);
  vlay->addWidget(btnRemove);
  vlay->addWidget(btnUp);
  vlay->addWidget(btnDown);
  vlay->addWidget(btnEdit);
  vlay->addWidget(btnDefault);
  vlay->addWidget(btnNewUser);
  glay->addMultiCell(vlay, 0, 0, 2, 2);

  glay->addWidget(new QLabel(tr("Default:"), grpGroups), 1, 0);
  nfoDefault = new CInfoField(grpGroups, true);
  glay->addMultiCellWidget(nfoDefault, 1, 1, 1, 2);
  glay->addWidget(new QLabel(tr("New User:"), grpGroups), 2, 0);
  nfoNewUser = new CInfoField(grpGroups, true);
  glay->addMultiCellWidget(nfoNewUser, 2, 2, 1, 2);

  edtName = new QLineEdit(grpGroups);
  edtName->setEnabled(false);
  glay->addMultiCellWidget(edtName, 3, 3, 0, 2);

  QHBoxLayout *hlay = new QHBoxLayout;
  hlay->addWidget(QWhatsThis::whatsThisButton(this), 0, AlignLeft);
  hlay->addSpacing(20);
  btnDone = new QPushButton(tr("Done"), this);
  hlay->addWidget(btnDone, 0, AlignRight);
  lay->addLayout(hlay, 1, 0);

  RefreshList();

  connect(btnAdd, SIGNAL(clicked()), this, SLOT(slot_add()));
  connect(btnRemove, SIGNAL(clicked()), this, SLOT(slot_remove()));
  connect(btnUp, SIGNAL(clicked()), this, SLOT(slot_up()));
  connect(btnDown, SIGNAL(clicked()), this, SLOT(slot_down()));
  connect(btnDefault, SIGNAL(clicked()), this, SLOT(slot_default()));
  connect(btnNewUser, SIGNAL(clicked()), this, SLOT(slot_newuser()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(btnDone, SIGNAL(clicked()), this, SLOT(hide()));
  connect(edtName, SIGNAL(returnPressed()), this, SLOT(slot_editok()));
}


void EditGrpDlg::RefreshList()
{
  lstGroups->clear();
  lstGroups->insertItem(tr("All Users"));
  if (gUserManager.DefaultGroup() == 0)
    nfoDefault->setText(tr("All Users"));
  if (gUserManager.NewUserGroup() == 0)
    nfoNewUser->setText(tr("All Users"));
  GroupList *g = gUserManager.LockGroupList(LOCK_R);
  for (unsigned short i = 0; i < g->size(); i++)
  {
    lstGroups->insertItem(QString::fromLocal8Bit((*g)[i]));
    if (i + 1 == gUserManager.DefaultGroup())
      nfoDefault->setText(QString::fromLocal8Bit((*g)[i]));
    if (i + 1== gUserManager.NewUserGroup())
      nfoNewUser->setText(QString::fromLocal8Bit((*g)[i]));
  }
  gUserManager.UnlockGroupList();
}

void EditGrpDlg::slot_add()
{
  gUserManager.AddGroup(strdup(tr("noname").local8Bit()));
  RefreshList();
  emit (signal_updateGroups());
}


void EditGrpDlg::slot_remove()
{
  int n = lstGroups->currentItem() - 1;
  if (n < 0) return;
  GroupList *g = gUserManager.LockGroupList(LOCK_R);
  QString warning(tr("Are you sure you want to remove\nthe group '") +
                  QString::fromLocal8Bit((*g)[n]) + "'?");
  gUserManager.UnlockGroupList();
  if(!QueryUser(this, warning, tr("Ok"), tr("Cancel")));
    return;
  gUserManager.RemoveGroup(n + 1);
  RefreshList();
  emit (signal_updateGroups());
}


void EditGrpDlg::slot_up()
{
  int n = lstGroups->currentItem() - 1;
  if (n <= 0) return;
  gUserManager.SwapGroups(n + 1, n);
  RefreshList();
  emit (signal_updateGroups());
}


void EditGrpDlg::slot_down()
{
  int n = lstGroups->currentItem() - 1;
  if (n < 0 /* || n == max */) return;
  gUserManager.SwapGroups(n + 1, n + 2);
  RefreshList();
  emit (signal_updateGroups());
}


void EditGrpDlg::slot_default()
{
  if (QueryUser(this, tr("Clear or Set?"), tr("Clear"), tr("Set")))
  {
    gUserManager.SetDefaultGroup(0);
  }
  else
  {
    int n = lstGroups->currentItem();
    if (n == -1) return;
    gUserManager.SetDefaultGroup(n);
  }
  RefreshList();
  //emit (signal_updateGroups());
}


void EditGrpDlg::slot_newuser()
{
  if (QueryUser(this, tr("Clear or Set?"), tr("Clear"), tr("Set")))
  {
    gUserManager.SetNewUserGroup(0);
  }
  else
  {
    int n = lstGroups->currentItem();
    if (n == -1 ) return;
    gUserManager.SetNewUserGroup(n);
  }
  RefreshList();
  //emit (signal_updateGroups());
}


void EditGrpDlg::slot_edit()
{
  int n = lstGroups->currentItem() - 1;
  if (n < 0) return;
  edtName->setEnabled(true);
  GroupList *g = gUserManager.LockGroupList(LOCK_R);
  edtName->setText(QString::fromLocal8Bit((*g)[n]));
  gUserManager.UnlockGroupList();
  m_nEditGrp = n + 1;
  btnEdit->setText(tr("Cancel"));
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_editcancel()));
  lstGroups->setEnabled(false);
}


void EditGrpDlg::slot_editok()
{
  gUserManager.RenameGroup(m_nEditGrp, edtName->text().local8Bit());
  RefreshList();
  emit (signal_updateGroups());

  lstGroups->setEnabled(true);
  btnEdit->setText(tr("Edit"));
  edtName->clear();
  edtName->setEnabled(false);
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_editok()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
}


void EditGrpDlg::slot_editcancel()
{
  lstGroups->setEnabled(true);
  btnEdit->setText(tr("Edit"));
  edtName->clear();
  edtName->setEnabled(false);
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_editcancel()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
}

void EditGrpDlg::hide()
{
   QWidget::hide();
   delete this;
}


#include "editgrp.moc"
