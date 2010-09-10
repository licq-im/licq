// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#include "groupdlg.h"

#include "config.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq/contactlist/group.h>
#include <licq/oneventmanager.h>

#include "helpers/support.h"
#include "settings/oneventbox.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::GroupDlg */

GroupDlg::GroupDlg(int groupId, QWidget* parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
    myGroupId(groupId)
{
  Support::setWidgetProps(this, "GroupDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);

  {
    Licq::GroupReadGuard group(myGroupId);
    if (group.isLocked())
      setWindowTitle(tr("Licq - Group ") + group->name().c_str());
  }

  QVBoxLayout* top_lay = new QVBoxLayout(this);
  myOnEventBox = new OnEventBox(false);
  top_lay->addWidget(myOnEventBox);

  QDialogButtonBox* buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel |
      QDialogButtonBox::Apply);
  top_lay->addWidget(buttons);

  connect(buttons, SIGNAL(accepted()), SLOT(ok()));
  connect(buttons, SIGNAL(rejected()), SLOT(close()));
  connect(buttons->button(QDialogButtonBox::Apply),
      SIGNAL(clicked()), SLOT(apply()));

  // Get onevents data for group
  Licq::OnEventData* effectiveData = Licq::gOnEventManager.getEffectiveGroup(myGroupId);
  const Licq::OnEventData* groupData = Licq::gOnEventManager.lockGroup(myGroupId);
  myOnEventBox->load(effectiveData, groupData);
  Licq::gOnEventManager.unlock(groupData);
  Licq::gOnEventManager.dropEffective(effectiveData);

  show();
}

void GroupDlg::ok()
{
  apply();
  close();
}

void GroupDlg::apply()
{
  // Save onevent settings
  Licq::OnEventData* groupData = Licq::gOnEventManager.lockGroup(myGroupId, true);
  myOnEventBox->apply(groupData);
  Licq::gOnEventManager.unlock(groupData, true);
}
