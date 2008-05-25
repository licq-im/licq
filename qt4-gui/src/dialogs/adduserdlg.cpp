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

  layDialog->addWidget(lblProtocol, 0, 0);
  layDialog->addWidget(myProtocol, 0, 1);

  QLabel* lblUin = new QLabel(tr("&New User ID:"));
  myUin = new QLineEdit();
  if (!id.isEmpty())
    myUin->setText(id);
  connect(myUin, SIGNAL(returnPressed()), SLOT(ok()));
  lblUin->setBuddy(myUin);

  layDialog->addWidget(lblUin, 1, 0);
  layDialog->addWidget(myUin, 1, 1);

  myNotify = new QCheckBox(tr("Notify User"));
  myNotify->setChecked(true);
  layDialog->addWidget(myNotify, 2, 0, 1, 2);

  QDialogButtonBox* buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel);
  connect(buttons, SIGNAL(accepted()), SLOT(ok()));
  connect(buttons, SIGNAL(rejected()), SLOT(close()));

  layDialog->addWidget(buttons, 3, 0, 1, 2);

  myUin->setFocus();
  show();
}

void AddUserDlg::ok()
{
  QString uin = myUin->text();

  if (!uin.isEmpty())
    gLicqDaemon->AddUserToList(uin.toLatin1(), myProtocol->currentPpid(),
        myNotify->isChecked());

  close();
}
