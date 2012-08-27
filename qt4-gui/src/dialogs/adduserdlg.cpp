/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2012 Licq developers <licq-dev@googlegroups.com>
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

#include <licq/contactlist/usermanager.h>
#include <licq/icq/icq.h>

#include "config/contactlist.h"
#include "contactlist/contactlist.h"
#include "helpers/support.h"

#include "widgets/groupcombobox.h"
#include "widgets/protocombobox.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::AddUserDlg */

AddUserDlg::AddUserDlg(const Licq::UserId& userId, QWidget* parent)
  : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
  Support::setWidgetProps(this, "AddUserDialog");
  setWindowTitle(tr("Licq - Add User"));
  setAttribute(Qt::WA_DeleteOnClose, true);

  QGridLayout* layDialog = new QGridLayout(this);

  QLabel* lblProtocol = new QLabel(tr("&Protocol:"));
  myProtocol = new ProtoComboBox(ProtoComboBox::FilterOwnersOnly);
  myProtocol->setCurrentPpid(userId.protocolId());
  lblProtocol->setBuddy(myProtocol);

  unsigned line = 0;

  layDialog->addWidget(lblProtocol, line, 0);
  layDialog->addWidget(myProtocol, line++, 1);

  QLabel* lblGroup = new QLabel(tr("&Group:"));
  myGroup = new GroupComboBox();
  lblGroup->setBuddy(myGroup);

  // Get current active group and set as default
  if (Config::ContactList::instance()->groupId() < ContactListModel::SystemGroupOffset)
    myGroup->setCurrentGroupId(Config::ContactList::instance()->groupId());

  layDialog->addWidget(lblGroup, line, 0);
  layDialog->addWidget(myGroup, line++, 1);

  QLabel* lblId = new QLabel(tr("&User ID:"));
  QString accountId = userId.accountId().c_str();
  myId = new QLineEdit();
  if (!accountId.isEmpty())
    myId->setText(accountId);
  connect(myId, SIGNAL(returnPressed()), SLOT(ok()));
  lblId->setBuddy(myId);

  layDialog->addWidget(lblId, line, 0);
  layDialog->addWidget(myId, line++, 1);

  myNotify = new QCheckBox(tr("&Notify user"));
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
  QString accountId = myId->text().trimmed();
  Licq::UserId userId(accountId.toLatin1().constData(), myProtocol->currentPpid());
  int group = myGroup->currentGroupId();
  bool notify = myNotify->isChecked();
  bool added = false;

  if (userId.isValid())
    added = Licq::gUserManager.addUser(userId, true, true, group);

  if (added && notify)
    gLicqDaemon->icqAlertUser(userId);

  close();
}
