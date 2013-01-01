/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2013 Licq developers <licq-dev@googlegroups.com>
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

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/usermanager.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/protocolmanager.h>

#include "config/iconmanager.h"
#include "core/messagebox.h"
#include "helpers/support.h"
#include "widgets/skinnablelabel.h"
#include "widgets/specialspinbox.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::OwnerEditDlg */

OwnerEditDlg::OwnerEditDlg(unsigned long ppid, QWidget* parent)
  : QDialog(parent),
    myPpid(ppid),
    myNewOwner(true),
    mySetStatus(Licq::User::OfflineStatus)
{
  init();
  show();
}

OwnerEditDlg::OwnerEditDlg(const Licq::UserId& ownerId, unsigned setStatus, const QString& autoMessage, QWidget* parent)
  : QDialog(parent),
    myOwnerId(ownerId),
    myPpid(ownerId.protocolId()),
    myNewOwner(false),
    mySetStatus(setStatus),
    myAutoMessage(autoMessage)
{
  init();

  {
    Licq::OwnerReadGuard o(myOwnerId);
    if (!o.isLocked())
    {
      close();
      return;
    }

    edtId->setText(o->accountId().c_str());
    edtId->setEnabled(false);
    edtPassword->setText(QString::fromLocal8Bit(o->password().c_str()));
    chkSave->setChecked(o->SavePassword());
    myHostEdit->setText(QString::fromLocal8Bit(o->serverHost().c_str()));
    myPortSpin->setValue(o->serverPort());
  }

  show();
}

void OwnerEditDlg::init()
{
  Support::setWidgetProps(this, "OwnerEdit");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("Edit Account"));

  QGridLayout* lay = new QGridLayout(this);
  lay->setColumnStretch(2, 2);
  lay->setColumnMinimumWidth(1, 8);

  SkinnableLabel* protocolName = new SkinnableLabel();

  edtId = new QLineEdit();
  connect(edtId, SIGNAL(returnPressed()), SLOT(slot_ok()));

  edtPassword = new QLineEdit();
  edtPassword->setEchoMode(QLineEdit::Password);
  connect(edtPassword, SIGNAL(returnPressed()), SLOT(slot_ok()));

  myHostEdit = new QLineEdit();
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
  myHostEdit->setPlaceholderText(tr("Protocol default"));
#endif
  myPortSpin = new SpecialSpinBox(0, 0xffff, tr("Auto"));
  myPortSpin->setValue(0);

  int i = 0;
  QLabel* lbl;

#define ADDWIDGET(name, widget) \
  lbl = new QLabel(name); \
  lbl->setBuddy(widget); \
  lay->addWidget(lbl, i, 0); \
  lay->addWidget(widget, i++, 2)

  ADDWIDGET(tr("Protocol:"), protocolName);
  ADDWIDGET(tr("&User ID:"), edtId);
  ADDWIDGET(tr("&Password:"), edtPassword);

  chkSave = new QCheckBox(tr("&Save Password"));
  lay->addWidget(chkSave, i++, 0, 1, 3);

  ADDWIDGET(tr("S&erver:"), myHostEdit);
  ADDWIDGET(tr("P&ort:"), myPortSpin);

#undef ADDWIDGET

  lay->setRowStretch(i++, 2);

  QDialogButtonBox* buttons = new QDialogButtonBox();
  buttons->addButton(QDialogButtonBox::Ok);
  buttons->addButton(QDialogButtonBox::Cancel);
  connect(buttons, SIGNAL(accepted()), SLOT(slot_ok()));
  connect(buttons, SIGNAL(rejected()), SLOT(close()));
  lay->addWidget(buttons, i++, 0, 1, 3);


  Licq::ProtocolPlugin::Ptr protocol = Licq::gPluginManager.getProtocolPlugin(myPpid);
  if (protocol.get() != NULL)
    protocolName->setText(protocol->name().c_str());
  protocolName->setPrependPixmap(IconManager::instance()->iconForProtocol(myPpid));
}

void OwnerEditDlg::slot_ok()
{
  if (myNewOwner)
  {
    QString id = edtId->text();
    if (id.isEmpty())
    {
      InformUser(this, tr("User ID field cannot be empty."));
      return;
    }

    myOwnerId = Licq::UserId(myPpid, id.toLocal8Bit().constData());
    Licq::gUserManager.addOwner(myOwnerId);
  }

  QString pwd = edtPassword->text();

  {
    Licq::OwnerWriteGuard o(myOwnerId);
    if (!o.isLocked())
      return;

    o->setPassword(pwd.toLocal8Bit().constData());
    o->SetSavePassword(chkSave->isChecked());
    o->setServer(myHostEdit->text().toLocal8Bit().constData(), myPortSpin->value());
    o->save(Licq::Owner::SaveOwnerInfo);
  }

  if (mySetStatus != Licq::User::OfflineStatus)
    Licq::gProtocolManager.setStatus(myOwnerId, mySetStatus,
        (myAutoMessage.isNull() ? Licq::gProtocolManager.KeepAutoResponse : myAutoMessage.toUtf8().constData()));

  close();
}
