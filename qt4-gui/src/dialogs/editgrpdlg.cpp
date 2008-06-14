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

#include <licq_events.h>
#include <licq_user.h>

#include "core/licqgui.h"
#include "core/mainwin.h"
#include "core/messagebox.h"
#include "core/signalmanager.h"

#include "helpers/licqstrings.h"
#include "helpers/support.h"

#include "widgets/infofield.h"

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
  glay->addWidget(lstGroups, 0, 0, 1, 2);

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
  BUTTON(btnNewUser, tr("Set New Users"), slot_newuser);
#undef BUTTON

  btnEdit->setToolTip(tr("Edit group name (hit enter to save)."));
  btnNewUser->setToolTip(
      tr("The group to which new users will be automatically added.\n"
        "All new users will be in the local system group New Users,\n"
        "but for server side storage will also be stored in the specified group."));

  glay->addLayout(vlay, 0, 2);

  glay->addWidget(new QLabel(tr("New User:"), grpGroups), 1, 0);
  nfoNewUser = new InfoField(true);
  nfoNewUser->setToolTip(btnNewUser->toolTip());
  glay->addWidget(nfoNewUser, 1, 1, 1, 2);

  edtName = new QLineEdit(grpGroups);
  edtName->setEnabled(false);
  connect(edtName, SIGNAL(returnPressed()), SLOT(slot_editok()));
  glay->addWidget(edtName, 2, 0, 1, 2);

  btnSave = new QPushButton(tr("&Save"));
  btnSave->setEnabled(false);
  btnSave->setToolTip(tr("Save the name of a group being modified."));
  connect(btnSave, SIGNAL(clicked()), SLOT(slot_editok()));
  glay->addWidget(btnSave, 2, 2);

  QDialogButtonBox* buttons = new QDialogButtonBox();
  connect(buttons, SIGNAL(rejected()), SLOT(close()));
  btnDone = buttons->addButton(QDialogButtonBox::Close);
  btnDone->setText(tr("&Done"));
  lay->addWidget(buttons);

  RefreshList();
  connect(LicqGui::instance()->signalManager(),
      SIGNAL(updatedList(CICQSignal*)), SLOT(listUpdated(CICQSignal*)));

  show();
}

unsigned short EditGrpDlg::currentGroupId() const
{
  if (lstGroups->currentItem() == NULL)
    return 0;

  unsigned short groupId = lstGroups->currentItem()->data(Qt::UserRole).toUInt();
  return groupId;
}

void EditGrpDlg::setCurrentGroupId(unsigned short groupId)
{
  for (int i = 0; i < lstGroups->count(); ++i)
    if (lstGroups->item(i)->data(Qt::UserRole).toUInt() == groupId)
    {
      lstGroups->setCurrentRow(i);
      break;
    }
}

void EditGrpDlg::RefreshList()
{
  unsigned short groupId = currentGroupId();
  lstGroups->clear();

  const QString allUsers = LicqStrings::getSystemGroupName(GROUP_ALL_USERS);
  QListWidgetItem* item = new QListWidgetItem(allUsers, lstGroups);
  item->setData(Qt::UserRole, 0);

  if (gUserManager.NewUserGroup() == GROUP_ALL_USERS)
    nfoNewUser->setText(allUsers);

  FOR_EACH_GROUP_START_SORTED(LOCK_R)
  {
    QString name = QString::fromLocal8Bit(pGroup->name().c_str());
    item = new QListWidgetItem(name, lstGroups);
    item->setData(Qt::UserRole, pGroup->id());

    if (gUserManager.NewUserGroup() == pGroup->id())
      nfoNewUser->setText(name);
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
  btnEdit->setText(tr("Cancel"));
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(btnEdit, SIGNAL(clicked()), SLOT(slot_editcancel()));
  lstGroups->setEnabled(false);
  btnSave->setDefault(true);
}

void EditGrpDlg::slot_remove()
{
  unsigned short groupId = currentGroupId();
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

void EditGrpDlg::slot_newuser()
{
  gUserManager.SetNewUserGroup(currentGroupId());
  RefreshList();
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
