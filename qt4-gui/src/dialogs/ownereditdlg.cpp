/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Licq developers
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

#include "ownereditdlg.h"

#include "config.h"

#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <licq_icqd.h>
#include <licq_user.h>

#include "config/iconmanager.h"

#include "core/messagebox.h"

#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::OwnerEditDlg */

OwnerEditDlg::OwnerEditDlg(unsigned long ppid, QWidget* parent)
  : QDialog(parent)
{
  Support::setWidgetProps(this, "OwnerEdit");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("Edit Account"));

  IconManager* iconman = IconManager::instance();

  QGridLayout* lay = new QGridLayout(this);
  lay->setColumnStretch(2, 2);
  lay->setColumnMinimumWidth(1, 8);

  // Protocol
  lay->addWidget(new QLabel(tr("Protocol:")), 0, 0);
  cmbProtocol = new QComboBox();
  lay->addWidget(cmbProtocol, 0, 2);

  // User ID
  lay->addWidget(new QLabel(tr("User ID:")), 1, 0);
  edtId = new QLineEdit();
  edtId->setMinimumWidth(edtId->sizeHint().width()*2);
  lay->addWidget(edtId, 1, 2);

  // Password
  lay->addWidget(new QLabel(tr("Password:")), 2, 0);
  edtPassword = new QLineEdit();
  edtPassword->setEchoMode(QLineEdit::Password);
  lay->addWidget(edtPassword, 2, 2);

  // Save Password
  chkSave = new QCheckBox(tr("Save Password"));
  lay->addWidget(chkSave, 3, 0, 1, 3);

  QString id;
  // Fill the combo list now
  FOR_EACH_PROTO_PLUGIN_START(gLicqDaemon)
    unsigned long curPpid = (*_ppit)->PPID();
    if (ppid != 0) // Modifying a user
    {
      ICQOwner* o = gUserManager.FetchOwner(curPpid, LOCK_R);
      if (o == NULL)
        id = QString::null;
      else
      {
        id = o->IdString();
        gUserManager.DropOwner(curPpid);
      }

      cmbProtocol->addItem(
          iconman->iconForStatus(ICQ_STATUS_ONLINE, id.toLatin1(), curPpid),
          (*_ppit)->Name(), QString::number(curPpid));

      // Check if this is the current protocol
      if (ppid == curPpid)
        cmbProtocol->setCurrentIndex(cmbProtocol->count() - 1);
    }
    else // Adding a user
    {
      ICQOwner* o = gUserManager.FetchOwner(curPpid, LOCK_R);
      if (o == NULL)
        cmbProtocol->addItem(
            iconman->iconForStatus(ICQ_STATUS_ONLINE, "0", curPpid),
            (*_ppit)->Name(), QString::number(curPpid));
      else
        gUserManager.DropOwner(curPpid);
    }
  FOR_EACH_PROTO_PLUGIN_END

  // Set the fields
  if (ppid != 0)
  {
    ICQOwner* o = gUserManager.FetchOwner(ppid, LOCK_R);
    if (o)
    {
      edtId->setText(o->IdString());
      edtPassword->setText(o->Password());
      chkSave->setChecked(o->SavePassword());
      gUserManager.DropOwner(ppid);
    }

    cmbProtocol->setEnabled(false);
  }
  else
  {
    if (cmbProtocol->count() == 0)
    {
      InformUser(this, tr("Currently only one account per protocol is supported."));
      close();
      return;
    }
  }

  lay->setRowStretch(4, 1);

  QDialogButtonBox* buttons = new QDialogButtonBox();
  lay->addWidget(buttons, 5, 0, 1, 3);

  btnOk = new QPushButton(tr("&Ok"));
  buttons->addButton(btnOk, QDialogButtonBox::AcceptRole);

  btnCancel = new QPushButton(tr("&Cancel"));
  buttons->addButton(btnCancel, QDialogButtonBox::RejectRole);

  connect(btnOk, SIGNAL(clicked()), SLOT(slot_ok()));
  connect(edtId, SIGNAL(returnPressed()), SLOT(slot_ok()));
  connect(edtPassword, SIGNAL(returnPressed()), SLOT(slot_ok()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(close()));

  // Set Tab Order
  setTabOrder(edtId, edtPassword);
  setTabOrder(edtPassword, cmbProtocol);
  setTabOrder(cmbProtocol, btnOk);
  setTabOrder(btnOk, btnCancel);

  show();
}

void OwnerEditDlg::slot_ok()
{
  QString szUser = edtId->text();
  QString szPassword;
  if (!edtPassword->text().isEmpty())
    szPassword = edtPassword->text();
  unsigned long nPPID = cmbProtocol->itemData(cmbProtocol->currentIndex()).toString().toULong();

  ICQOwner* o = gUserManager.FetchOwner(nPPID, LOCK_W);
  if (o != NULL)
  {
    o->SetId(szUser.toLatin1());
  }
  else
  {
    gUserManager.AddOwner(szUser.toLatin1(), nPPID);
    o = gUserManager.FetchOwner(nPPID, LOCK_W);
  }
  if (!szPassword.isNull())
    o->SetPassword(szPassword.toLatin1());

  o->SetSavePassword(chkSave->isChecked());

  gUserManager.DropOwner(nPPID);
  gLicqDaemon->SaveConf();

  close();
}

