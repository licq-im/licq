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

#include <licq_events.h>
#include <licq_user.h>

#include "editgrp.h"
#include "ewidgets.h"
#include "gui-strings.h"
#include "licq_user.h"
#include "sigman.h"

EditGrpDlg::EditGrpDlg(CSignalManager* signalManager, QWidget *parent)
  : QWidget(parent, "EditGroupDialog")
{
  setCaption(tr("Licq - Edit Groups"));
  QGridLayout *lay = new QGridLayout(this, 2, 1, 8, 10);
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
  vlay->addWidget(btnAdd);
  vlay->addWidget(btnRemove);
  vlay->addWidget(btnUp);
  vlay->addWidget(btnDown);
  vlay->addWidget(btnEdit);

  edtName = new QLineEdit(grpGroups);
  edtName->setEnabled(false);
  glay->addMultiCellWidget(edtName, 3, 3, 0, 2);

  QHBoxLayout *hlay = new QHBoxLayout;
  hlay->addWidget(QWhatsThis::whatsThisButton(this), 0, AlignLeft);
  hlay->addSpacing(20);
  btnSave = new QPushButton(tr("&Save"), this);
  btnSave->setEnabled(false);
  btnSave->setMinimumWidth(75);
  QWhatsThis::add(btnSave, tr("Save the name of a group being modified."));
  hlay->addWidget(btnSave, 0, AlignRight);
  btnDone = new QPushButton(tr("&Done"), this);
  btnDone->setMinimumWidth(75);
  hlay->addWidget(btnDone, 0, AlignRight);
  lay->addLayout(hlay, 1, 0);

  RefreshList();
  connect(signalManager, SIGNAL(signal_updatedList(CICQSignal*)),
      this, SLOT(listUpdated(CICQSignal*)));

  connect(btnAdd, SIGNAL(clicked()), this, SLOT(slot_add()));
  connect(btnRemove, SIGNAL(clicked()), this, SLOT(slot_remove()));
  connect(btnUp, SIGNAL(clicked()), this, SLOT(slot_up()));
  connect(btnDown, SIGNAL(clicked()), this, SLOT(slot_down()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(btnDone, SIGNAL(clicked()), this, SLOT(close()));
  connect(edtName, SIGNAL(returnPressed()), this, SLOT(slot_editok()));
  connect(btnSave, SIGNAL(clicked()), this, SLOT(slot_editok()));
}

unsigned short EditGrpDlg::currentGroupId() const
{
  if (lstGroups->currentItem() == -1)
    return 0;

  return myGroupIds[lstGroups->currentItem()];
}

void EditGrpDlg::setCurrentGroupId(unsigned short groupId)
{
  for (unsigned i = 0; i < lstGroups->count(); ++i)
    if (myGroupIds[i] == groupId)
    {
      lstGroups->setCurrentItem(i);
      break;
    }
}

void EditGrpDlg::RefreshList()
{
  unsigned short groupId = currentGroupId();
  lstGroups->clear();
  myGroupIds.clear();

  FOR_EACH_GROUP_START_SORTED(LOCK_R)
  {
    QString name = QString::fromLocal8Bit(pGroup->name().c_str());
    lstGroups->insertItem(name);
    myGroupIds.push_back(pGroup->id());
  }
  FOR_EACH_GROUP_END

  setCurrentGroupId(groupId);
}

void EditGrpDlg::listUpdated(CICQSignal* sig)
{
  switch (sig->SubSignal())
  {
    case LIST_GROUP_ADDED:
    case LIST_GROUP_REMOVED:
    case LIST_GROUP_CHANGED:
    case LIST_GROUP_REORDERED:

    case LIST_INVALIDATE:
      if (btnSave->isEnabled())
        slot_editcancel();
      RefreshList();
      break;
  }
}

void EditGrpDlg::slot_add()
{
  // Don't add group until user has had a chance to set a name for it
  myEditGroupId = 0;
  lstGroups->setCurrentItem(-1);

  btnSave->setEnabled(true);
  btnDone->setEnabled(false);
  edtName->setEnabled(true);
  edtName->setText(tr("noname"));
  edtName->setFocus();
  btnEdit->setText(tr("Cancel"));
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_editcancel()));
  lstGroups->setEnabled(false);
  btnSave->setDefault(true);
}

void EditGrpDlg::slot_remove()
{
  unsigned short groupId = currentGroupId();
  if (groupId == 0)
    return;

  QString warning(tr("Are you sure you want to remove\n"
                     "the group '%1'?").arg(lstGroups->currentText()));

  if(QueryUser(this, warning, tr("Ok"), tr("Cancel"))) {
    gUserManager.RemoveGroup(groupId);
    RefreshList();
  }
}

void EditGrpDlg::moveGroup(int delta)
{
  unsigned short groupId = currentGroupId();
  if (groupId == 0)
    return;

  LicqGroup* group = gUserManager.FetchGroup(groupId, LOCK_R);
  if (group == NULL)
    return;
  unsigned short oldSortIndex = group->sortIndex();
  gUserManager.DropGroup(group);

  if (delta + oldSortIndex < 0)
    return;

  gUserManager.ModifyGroupSorting(groupId, oldSortIndex + delta);
  RefreshList();
}

void EditGrpDlg::slot_up()
{
  moveGroup(-1);
}

void EditGrpDlg::slot_down()
{
  moveGroup(1);
}

void EditGrpDlg::slot_edit()
{
  myEditGroupId = currentGroupId();
  if (myEditGroupId == 0)
    return;

  btnSave->setEnabled(true);
  btnDone->setEnabled(false);
  edtName->setEnabled(true);
  edtName->setText(lstGroups->currentText());
  edtName->setFocus();
  btnEdit->setText(tr("Cancel"));
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_editcancel()));
  lstGroups->setEnabled(false);
  btnSave->setDefault(true);
}

void EditGrpDlg::slot_editok()
{
  if (myEditGroupId == 0)
    myEditGroupId = gUserManager.AddGroup(edtName->text().local8Bit().data());
  else
    gUserManager.RenameGroup(myEditGroupId, edtName->text().local8Bit().data());
  RefreshList();
  setCurrentGroupId(myEditGroupId);

  btnSave->setDefault(false);
  lstGroups->setEnabled(true);
  btnEdit->setText(tr("Edit Name"));
  edtName->clear();
  edtName->setEnabled(false);
  btnSave->setEnabled(false);
  btnDone->setEnabled(true);
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_editok()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
}

void EditGrpDlg::slot_editcancel()
{
  btnSave->setDefault(false);
  lstGroups->setEnabled(true);
  btnEdit->setText(tr("Edit Name"));
  edtName->clear();
  edtName->setEnabled(false);
  btnSave->setEnabled(false);
  btnDone->setEnabled(true);
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_editcancel()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
}

#include "editgrp.moc"
