/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq developers
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

#include "addgroupdlg.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include <licq/contactlist/group.h>
#include <licq/contactlist/usermanager.h>

#include "config/contactlist.h"
#include "contactlist/contactlist.h"
#include "helpers/support.h"
#include "widgets/groupcombobox.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::AddGroupDlg */

AddGroupDlg::AddGroupDlg(QWidget* parent)
  : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
  Support::setWidgetProps(this, "AddGroupDialog");
  setWindowTitle(tr("Licq - Add group"));
  setAttribute(Qt::WA_DeleteOnClose, true);

  QGridLayout* layDialog = new QGridLayout(this);

  QLabel* nameLabel = new QLabel(tr("New &Group Name:"));
  myNameEdit = new QLineEdit();
  connect(myNameEdit, SIGNAL(returnPressed()), SLOT(ok()));
  nameLabel->setBuddy(myNameEdit);

  layDialog->addWidget(nameLabel, 0, 0);
  layDialog->addWidget(myNameEdit, 0, 1);

  QLabel* positionLabel = new QLabel(tr("&Position:"));
  myPositionCombo = new GroupComboBox(true);
  myPositionCombo->setCurrentIndex(myPositionCombo->count() - 1);
  positionLabel->setBuddy(myPositionCombo);

  // Get current active group and set as default
  if (Config::ContactList::instance()->groupId() < ContactListModel::SystemGroupOffset)
    myPositionCombo->setCurrentGroupId(Config::ContactList::instance()->groupId());

  layDialog->addWidget(positionLabel, 1, 0);
  layDialog->addWidget(myPositionCombo, 1, 1);

  QDialogButtonBox* buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel);
  connect(buttons, SIGNAL(accepted()), SLOT(ok()));
  connect(buttons, SIGNAL(rejected()), SLOT(close()));

  layDialog->addWidget(buttons, 2, 0, 1, 2);

  myNameEdit->setFocus();
  show();
}

void AddGroupDlg::ok()
{
  QString name = myNameEdit->text().trimmed();
  if (name.isEmpty())
    return;

  int groupId = Licq::gUserManager.AddGroup(name.toLatin1().constData());

  if (groupId != 0)
  {
    int afterGroupId = myPositionCombo->currentGroupId();
    int sortIndex = -1;
    if (afterGroupId == -1)
    {
      sortIndex = 0;
    }
    else
    {
      Licq::GroupReadGuard g(afterGroupId);
      if (g.isLocked())
        sortIndex = g->sortIndex() + 1;
    }
    if (sortIndex != -1)
      Licq::gUserManager.ModifyGroupSorting(groupId, sortIndex);
  }

  close();
}
