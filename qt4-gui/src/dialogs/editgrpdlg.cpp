/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#include "editgrpdlg.h"

#include "config.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq_user.h>

#include "core/mainwin.h"
#include "core/messagebox.h"

#include "helpers/licqstrings.h"

#include "widgets/infofield.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::EditGrpDlg */

EditGrpDlg::EditGrpDlg(QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle(tr("Licq - Edit Groups"));
  setObjectName("EditGroupDialog");

  QVBoxLayout* lay = new QVBoxLayout(this);
  grpGroups = new QGroupBox(tr("Groups"));
  lay->addWidget(grpGroups);

  QGridLayout* glay = new QGridLayout(grpGroups);
  lstGroups = new QListWidget(grpGroups);
  glay->addWidget(lstGroups, 0, 0, 1, 2);

  QVBoxLayout* vlay = new QVBoxLayout();
  btnAdd = new QPushButton(tr("Add"), grpGroups);
  btnRemove = new QPushButton(tr("Remove"), grpGroups);
  btnUp = new QPushButton(tr("Shift Up"), grpGroups);
  btnDown = new QPushButton(tr("Shift Down"), grpGroups);
  btnEdit = new QPushButton(tr("Edit Name"), grpGroups);
  btnEdit->setToolTip(tr("Edit group name (hit enter to save)."));
  btnDefault = new QPushButton(tr("Set Default"), grpGroups);
  btnDefault->setToolTip(tr("The default group to start up in."));
  btnNewUser = new QPushButton(tr("Set New Users"), grpGroups);
  btnNewUser->setToolTip(
        tr("The group to which new users will be automatically added.  "
           "All new users will be in the local system group New Users "
           "but for server side storage will also be stored in the "
           "specified group."));
  vlay->addWidget(btnAdd);
  vlay->addWidget(btnRemove);
  vlay->addWidget(btnUp);
  vlay->addWidget(btnDown);
  vlay->addWidget(btnEdit);
  vlay->addWidget(btnDefault);
  vlay->addWidget(btnNewUser);
  glay->addLayout(vlay, 0, 2);

  glay->addWidget(new QLabel(tr("Default:"), grpGroups), 1, 0);
  nfoDefault = new InfoField(true);
  nfoDefault->setToolTip(btnDefault->toolTip());
  glay->addWidget(nfoDefault, 1, 1, 1, 2);
  glay->addWidget(new QLabel(tr("New User:"), grpGroups), 2, 0);
  nfoNewUser = new InfoField(true);
  nfoNewUser->setToolTip(btnNewUser->toolTip());
  glay->addWidget(nfoNewUser, 2, 1, 1, 2);

  edtName = new QLineEdit(grpGroups);
  edtName->setEnabled(false);
  glay->addWidget(edtName, 3, 0, 1, 2);

  btnSave = new QPushButton(tr("&Save"));
  btnSave->setEnabled(false);
  btnSave->setToolTip(tr("Save the name of a group being modified."));
  glay->addWidget(btnSave, 3, 2);

  QDialogButtonBox* buttons = new QDialogButtonBox();
  lay->addWidget(buttons);

  btnDone = buttons->addButton(QDialogButtonBox::Close);
  btnDone->setText(tr("&Done"));

  RefreshList();

  connect(btnAdd, SIGNAL(clicked()), SLOT(slot_add()));
  connect(btnRemove, SIGNAL(clicked()), SLOT(slot_remove()));
  connect(btnUp, SIGNAL(clicked()), SLOT(slot_up()));
  connect(btnDown, SIGNAL(clicked()), SLOT(slot_down()));
  connect(btnDefault, SIGNAL(clicked()), SLOT(slot_default()));
  connect(btnNewUser, SIGNAL(clicked()), SLOT(slot_newuser()));
  connect(btnEdit, SIGNAL(clicked()), SLOT(slot_edit()));
  connect(btnDone, SIGNAL(clicked()), SLOT(close()));
  connect(edtName, SIGNAL(returnPressed()), SLOT(slot_editok()));
  connect(btnSave, SIGNAL(clicked()), SLOT(slot_editok()));

  show();
}

void EditGrpDlg::RefreshList()
{
  lstGroups->clear();

  const QString allUsers = LicqStrings::getSystemGroupName(GROUP_ALL_USERS);

  lstGroups->addItem(allUsers);

  if (gUserManager.DefaultGroup() == GROUP_ALL_USERS)
    nfoDefault->setText(allUsers);

  if (gUserManager.NewUserGroup() == GROUP_ALL_USERS)
    nfoNewUser->setText(allUsers);

  GroupList* g = gUserManager.LockGroupList(LOCK_R);
  for (unsigned short i = 0; i < g->size(); i++)
  {
    lstGroups->addItem(QString::fromLocal8Bit((*g)[i]));

    if (gUserManager.DefaultGroup() == i + 1)
      nfoDefault->setText(QString::fromLocal8Bit((*g)[i]));

    if (gUserManager.NewUserGroup() == i + 1)
      nfoNewUser->setText(QString::fromLocal8Bit((*g)[i]));
  }
  gUserManager.UnlockGroupList();
}

void EditGrpDlg::slot_add()
{
  gUserManager.AddGroup(strdup(tr("noname").toLocal8Bit()));
  RefreshList();
  lstGroups->setCurrentRow(lstGroups->count()-1);
  slot_edit();
}


void EditGrpDlg::slot_remove()
{
  int n = lstGroups->currentRow();
  // don't allow the default group #0 "All Users" to be deleted
  // don't try to delete if there is no current Item (currentItem() == -1)
  if (n < 1) return;

  GroupList* g = gUserManager.LockGroupList(LOCK_R);
  QString warning(tr("Are you sure you want to remove\n"
                     "the group '%1'?").arg(QString::fromLocal8Bit((*g)[n-1])));
  gUserManager.UnlockGroupList();

  if (QueryYesNo(this, warning))
  {
    gUserManager.RemoveGroup(n);
    RefreshList();
    lstGroups->setCurrentRow(n - 1);
  }
}


void EditGrpDlg::slot_up()
{
  int n = lstGroups->currentRow() - 1;
  if (n <= 0) return;
  gUserManager.SwapGroups(n + 1, n);
  RefreshList();
  lstGroups->setCurrentRow(n);
}


void EditGrpDlg::slot_down()
{
  int n = lstGroups->currentRow() - 1;
  if (n < 0 /* || n == max */) return;
  gUserManager.SwapGroups(n + 1, n + 2);
  RefreshList();
  if (n + 2 >= int(lstGroups->count()))
    lstGroups->setCurrentRow(lstGroups->count() - 1);
  else
    lstGroups->setCurrentRow(n + 2);
}


void EditGrpDlg::slot_default()
{
  int n = lstGroups->currentRow();
  if (n == -1) return;
  gUserManager.SetDefaultGroup(n);
  RefreshList();
  lstGroups->setCurrentRow(n);
}

void EditGrpDlg::slot_newuser()
{
  int n = lstGroups->currentRow();
  if (n == -1 ) return;
  gUserManager.SetNewUserGroup(n);
  RefreshList();
  lstGroups->setCurrentRow(n);
}

void EditGrpDlg::slot_edit()
{
  int n = lstGroups->currentRow() - 1;
  if (n < 0) return;
  btnSave->setEnabled(true);
  btnDone->setEnabled(false);
  edtName->setEnabled(true);
  GroupList* g = gUserManager.LockGroupList(LOCK_R);
  edtName->setText(QString::fromLocal8Bit((*g)[n]));
  edtName->setFocus();
  gUserManager.UnlockGroupList();
  m_nEditGrp = n + 1;
  btnEdit->setText(tr("Cancel"));
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(btnEdit, SIGNAL(clicked()), SLOT(slot_editcancel()));
  lstGroups->setEnabled(false);
}


void EditGrpDlg::slot_editok()
{
  int n = lstGroups->currentRow();
  gUserManager.RenameGroup(m_nEditGrp, edtName->text().toLocal8Bit());
  RefreshList();

  lstGroups->setEnabled(true);
  btnEdit->setText(tr("Edit Name"));
  edtName->clear();
  edtName->setEnabled(false);
  btnSave->setEnabled(false);
  btnDone->setEnabled(true);
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_editok()));
  connect(btnEdit, SIGNAL(clicked()), SLOT(slot_edit()));
  lstGroups->setCurrentRow(n);
}


void EditGrpDlg::slot_editcancel()
{
  lstGroups->setEnabled(true);
  btnEdit->setText(tr("Edit Name"));
  edtName->clear();
  edtName->setEnabled(false);
  btnSave->setEnabled(false);
  btnDone->setEnabled(true);
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_editcancel()));
  connect(btnEdit, SIGNAL(clicked()), SLOT(slot_edit()));
}
