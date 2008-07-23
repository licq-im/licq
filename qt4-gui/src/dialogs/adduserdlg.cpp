// -*- c-basic-offset: 2 -*-
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

#include "adduserdlg.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include <licq_icqd.h>

#include "helpers/support.h"

#include "widgets/groupcombobox.h"
#include "widgets/protocombobox.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::AddUserDlg */

AddUserDlg::AddUserDlg(QString id, unsigned long ppid, QWidget* parent)
  : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
  Support::setWidgetProps(this, "AddUserDialog");
  setWindowTitle(tr("Licq - Add user"));
  setAttribute(Qt::WA_DeleteOnClose, true);

  QGridLayout* layDialog = new QGridLayout(this);

  QLabel* lblProtocol = new QLabel(tr("&Protocol:"));
  myProtocol = new ProtoComboBox();
  myProtocol->setCurrentPpid(ppid);
  lblProtocol->setBuddy(myProtocol);

  unsigned line = 0;

  layDialog->addWidget(lblProtocol, line, 0);
  layDialog->addWidget(myProtocol, line++, 1);

  QLabel* lblGroup = new QLabel(tr("&Group:"));
  myGroup = new GroupComboBox();
  lblGroup->setBuddy(myGroup);

  layDialog->addWidget(lblGroup, line, 0);
  layDialog->addWidget(myGroup, line++, 1);

  QLabel* lblId = new QLabel(tr("New &User ID:"));
  myId = new QLineEdit();
  if (!id.isEmpty())
    myId->setText(id);
  connect(myId, SIGNAL(returnPressed()), SLOT(ok()));
  lblId->setBuddy(myId);

  layDialog->addWidget(lblId, line, 0);
  layDialog->addWidget(myId, line++, 1);

  myNotify = new QCheckBox(tr("&Notify User"));
  myNotify->setChecked(true);
  layDialog->addWidget(myNotify, line++, 0, 1, 2);

  QDialogButtonBox* buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel);
  connect(buttons, SIGNAL(accepted()), SLOT(ok()));
  connect(buttons, SIGNAL(rejected()), SLOT(close()));

  layDialog->addWidget(buttons, line++, 0, 1, 2);

  myId->setFocus();
  show();
}

void AddUserDlg::ok()
{
  QByteArray id = myId->text().trimmed().toLatin1();
  unsigned long ppid = myProtocol->currentPpid();
  unsigned short group = myGroup->currentGroupId();
  bool notify = myNotify->isChecked();
  bool added = false;

  if (!id.isEmpty())
  {
    const ICQUser* u = gUserManager.FetchUser(id, ppid, LOCK_R);

    if (u == NULL)
      added = gLicqDaemon->AddUserToList(id, ppid, true, false, group);
    else
    {
      bool notInList = u->NotInList();
      gUserManager.DropUser(u);

      if (notInList)
      {
        gUserManager.SetUserInGroup(id, ppid, GROUPS_USER, group, true, true);
        ICQUser* user = gUserManager.FetchUser(id, ppid, LOCK_W);
        user->SetPermanent();
        gUserManager.DropUser(user);
        added = true;
      }
    }
  }

  if (added && notify)
    gLicqDaemon->icqAlertUser(id, ppid);

  close();
}
