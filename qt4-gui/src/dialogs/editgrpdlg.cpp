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

#include "editgrpdlg.h"

#include "config.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq_events.h>
#include <licq_user.h>

#include "core/licqgui.h"
#include "core/messagebox.h"
#include "core/signalmanager.h"

#include "helpers/licqstrings.h"
#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::EditGrpDlg */

EditGrpDlg::EditGrpDlg(QWidget* parent)
  : QDialog(parent)
{
  Support::setWidgetProps(this, "EditGroupDialog");
  setWindowTitle(tr("Licq - Edit Groups"));

  QVBoxLayout* lay = new QVBoxLayout(this);
  grpGroups = new QGroupBox(tr("Groups"));
  lay->addWidget(grpGroups);

  QGridLayout* glay = new QGridLayout(grpGroups);
  lstGroups = new QListWidget(grpGroups);
  glay->addWidget(lstGroups, 0, 0);

  QVBoxLayout* vlay = new QVBoxLayout();
#define BUTTON(var, name, slot) \
  var = new QPushButton(name, grpGroups); \
  connect(var, SIGNAL(clicked()), SLOT(slot())); \
  vlay->addWidget(var)

  BUTTON(btnAdd, tr("Add"), slot_add);
  BUTTON(btnRemove, tr("Remove"), slot_remove);
  BUTTON(btnUp, tr("Shift Up"), slot_up);
  BUTTON(btnDown, tr("Shift Down"), slot_down);
  BUTTON(btnEdit, tr("Edit Name"), slot_edit);
#undef BUTTON
  vlay->addStretch(1);

  btnEdit->setToolTip(tr("Edit group name (hit enter to save)."));

  glay->addLayout(vlay, 0, 1);

  edtName = new QLineEdit(grpGroups);
  edtName->setEnabled(false);
  connect(edtName, SIGNAL(returnPressed()), SLOT(slot_editok()));
  glay->addWidget(edtName, 1, 0);

  btnSave = new QPushButton(tr("&Save"));
  btnSave->setEnabled(false);
  btnSave->setToolTip(tr("Save the name of a group being modified."));
  connect(btnSave, SIGNAL(clicked()), SLOT(slot_editok()));
  glay->addWidget(btnSave, 1, 1);

  QDialogButtonBox* buttons = new QDialogButtonBox();
  connect(buttons, SIGNAL(rejected()), SLOT(close()));
  btnDone = buttons->addButton(QDialogButtonBox::Close);
  btnDone->setText(tr("&Done"));
  lay->addWidget(buttons);

  RefreshList();
  connect(LicqGui::instance()->signalManager(),
      SIGNAL(updatedList(unsigned long, int, const UserId&)),
      SLOT(listUpdated(unsigned long)));

  show();
}

int EditGrpDlg::currentGroupId() const
{
  if (lstGroups->currentItem() == NULL)
    return 0;

  return lstGroups->currentItem()->data(Qt::UserRole).toInt();
}

void EditGrpDlg::setCurrentGroupId(int groupId)
{
  for (int i = 0; i < lstGroups->count(); ++i)
    if (lstGroups->item(i)->data(Qt::UserRole).toInt() == groupId)
    {
      lstGroups->setCurrentRow(i);
      break;
    }
}

void EditGrpDlg::RefreshList()
{
  int groupId = currentGroupId();
  lstGroups->clear();

  FOR_EACH_GROUP_START_SORTED(LOCK_R)
  {
    QString name = QString::fromLocal8Bit(pGroup->name().c_str());
    QListWidgetItem* item = new QListWidgetItem(name, lstGroups);
    item->setData(Qt::UserRole, pGroup->id());
  }
  FOR_EACH_GROUP_END

  setCurrentGroupId(groupId);
}

void EditGrpDlg::listUpdated(unsigned long subSignal)
{
  switch (subSignal)
  {
    case LIST_GROUP_ADDED:
    case LIST_GROUP_REMOVED:
    case LIST_GROUP_CHANGED:
    case LIST_GROUP_REORDERED:

    case LIST_INVALIDATE:
      if (btnSave->isEnabled()) // we are editing the group name
        slot_editcancel();
      RefreshList();
      break;
  }
}

void EditGrpDlg::slot_add()
{
  // Don't add group until user has had a chance to set a name for it
  myEditGroupId = 0;
  lstGroups->setCurrentRow(-1);

  btnSave->setEnabled(true);
  btnDone->setEnabled(false);
  edtName->setEnabled(true);

  edtName->setText(tr("noname"));
  edtName->setFocus();
  edtName->selectAll();
  btnEdit->setText(tr("Cancel"));
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(btnEdit, SIGNAL(clicked()), SLOT(slot_editcancel()));
  lstGroups->setEnabled(false);
  btnSave->setDefault(true);
}

void EditGrpDlg::slot_remove()
{
  int groupId = currentGroupId();
  if (groupId == 0)
    return;

  QString warning(tr("Are you sure you want to remove\n"
                     "the group '%1'?").arg(lstGroups->currentItem()->text()));

  if (QueryYesNo(this, warning))
  {
    gUserManager.RemoveGroup(groupId);
    RefreshList();
  }
}

void EditGrpDlg::moveGroup(int delta)
{
  int groupId = currentGroupId();
  if (groupId == 0)
    return;

  LicqGroup* group = gUserManager.FetchGroup(groupId, LOCK_R);
  if (group == NULL)
    return;
  int oldSortIndex = group->sortIndex();
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

  edtName->setText(lstGroups->currentItem()->text());
  edtName->setFocus();
  btnEdit->setText(tr("Cancel"));
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(btnEdit, SIGNAL(clicked()), SLOT(slot_editcancel()));
  lstGroups->setEnabled(false);
  btnSave->setDefault(true);
}

void EditGrpDlg::slot_editok()
{
  if (myEditGroupId == 0)
    myEditGroupId = gUserManager.AddGroup(edtName->text().toLocal8Bit().data());
  else
    gUserManager.RenameGroup(myEditGroupId, edtName->text().toLocal8Bit().data());
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
  connect(btnEdit, SIGNAL(clicked()), SLOT(slot_edit()));
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
  connect(btnEdit, SIGNAL(clicked()), SLOT(slot_edit()));
}
