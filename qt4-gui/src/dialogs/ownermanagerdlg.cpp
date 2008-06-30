/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2006 Licq developers
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

#include "ownermanagerdlg.h"

#include "config.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QStringList>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <licq_icqd.h>
#include <licq_user.h>

#include "config/iconmanager.h"

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/messagebox.h"

#include "helpers/support.h"

#include "ownereditdlg.h"
#include "registeruser.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::OwnerManagerDlg */

OwnerManagerDlg* OwnerManagerDlg::myInstance = NULL;

void OwnerManagerDlg::showOwnerManagerDlg()
{
  if (myInstance == NULL)
    myInstance = new OwnerManagerDlg();
  else
    myInstance->raise();
}

OwnerManagerDlg::OwnerManagerDlg(QWidget* parent)
  : QDialog(parent),
    registerUserDlg(NULL)
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  Support::setWidgetProps(this, "AccountDialog");
  setWindowTitle(tr("Licq - Account Manager"));

  QVBoxLayout* toplay = new QVBoxLayout(this);

  // Add the list box
  ownerView = new QTreeWidget();
  QStringList headers;
  headers << tr("Protocol") << tr("User ID");
  ownerView->setHeaderLabels(headers);
  ownerView->setIndentation(0);
  toplay->addWidget(ownerView);

  // Add the buttons now
  QDialogButtonBox* buttons = new QDialogButtonBox();
  toplay->addWidget(buttons);

  addButton = new QPushButton(tr("&Add"));
  buttons->addButton(addButton, QDialogButtonBox::ActionRole);

  registerButton = new QPushButton(tr("&Register"));
  buttons->addButton(registerButton, QDialogButtonBox::ActionRole);

  modifyButton = new QPushButton(tr("&Modify"));
  buttons->addButton(modifyButton, QDialogButtonBox::ActionRole);

  removeButton = new QPushButton(tr("D&elete"));
  buttons->addButton(removeButton, QDialogButtonBox::ActionRole);

  closeButton = new QPushButton(tr("&Done"));
  buttons->addButton(closeButton, QDialogButtonBox::RejectRole);

  // Connect all the signals
  connect(ownerView, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
      SLOT(listClicked(QTreeWidgetItem*)));
  connect(ownerView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
      SLOT(modifyOwner(QTreeWidgetItem*, int)));
  connect(addButton, SIGNAL(clicked()), SLOT(addOwner()));
  connect(registerButton, SIGNAL(clicked()), SLOT(registerOwner()));
  connect(modifyButton, SIGNAL(clicked()), SLOT(modifyOwner()));
  connect(removeButton, SIGNAL(clicked()), SLOT(removeOwner()));
  connect(closeButton, SIGNAL(clicked()), SLOT(close()));

  // Add the owners to the list now
  updateOwners();

  // Show information to the user
  if (gUserManager.NumOwners() == 0)
  {
    InformUser(this, tr("From the Account Manager dialog you are able to add"
        " and register your accounts.\n"
        "Currently, only one account per protocol is supported, but this will"
        " be changed in future versions."));
  }

  show();
}

OwnerManagerDlg::~OwnerManagerDlg()
{
  myInstance = NULL;
}

void OwnerManagerDlg::updateOwners()
{
  ownerView->clear();

  if (gUserManager.NumOwners() != 0)
  {
    QString id, proto;
    unsigned long ppid = 0;

    IconManager* iconman = IconManager::instance();

    FOR_EACH_OWNER_START(LOCK_R)
      id = pOwner->IdString();
      ppid = pOwner->PPID();
      proto = gLicqDaemon->ProtoPluginName(ppid);

      QTreeWidgetItem* item = new QTreeWidgetItem(ownerView);
      item->setIcon(0, iconman->iconForStatus(ICQ_STATUS_ONLINE, id.toLatin1(), ppid));
      item->setText(0, proto.isNull() ? tr("(Invalid Protocol)") : proto);
      item->setData(0, Qt::UserRole, QString::number(ppid));
      item->setText(1, id.isNull() ? tr("(Invalid ID)") : id);
    FOR_EACH_OWNER_END
  }

  ownerView->resizeColumnToContents(0);
  ownerView->resizeColumnToContents(1);
  ownerView->sortByColumn(0, Qt::AscendingOrder);

  modifyButton->setEnabled(false);
  removeButton->setEnabled(false);
}

void OwnerManagerDlg::listClicked(QTreeWidgetItem* item)
{
  modifyButton->setEnabled(item != NULL);
  removeButton->setEnabled(item != NULL);
}

void OwnerManagerDlg::addOwner()
{
  OwnerEditDlg* d = new OwnerEditDlg(0, this);
  connect(d, SIGNAL(destroyed()), SLOT(updateOwners()));
}

void OwnerManagerDlg::registerOwner()
{
  if (!gUserManager.OwnerId(LICQ_PPID).empty())
  {
    QString buf = tr("You are currently registered as\n"
        "UIN (User ID): %1\n"
        "Base Directory: %2\n"
        "Rerun licq with the -b option to select a new\n"
        "base directory and then register a new user.")
        .arg(gUserManager.OwnerId(LICQ_PPID).c_str()).arg(BASE_DIR);
    InformUser(this, buf);
    return;
  }

  if (registerUserDlg != 0)
    registerUserDlg->raise();
  else
  {
    registerUserDlg = new RegisterUserDlg(this);
    connect(registerUserDlg, SIGNAL(signal_done(bool, QString, unsigned long)),
        SLOT(registerDone(bool, QString, unsigned long)));
  }
}

void OwnerManagerDlg::registerDone(bool success, QString newId, unsigned long newPpid)
{
  registerUserDlg = 0;

  if (success)
  {
    updateOwners();
    LicqGui::instance()->showInfoDialog(mnuUserGeneral, newId, newPpid);
  }
}

void OwnerManagerDlg::modifyOwner()
{
  modifyOwner(ownerView->currentItem());
}

void OwnerManagerDlg::modifyOwner(QTreeWidgetItem* item, int /* column */)
{
  if (item == NULL)
    return;

  OwnerEditDlg* editDlg = new OwnerEditDlg(
      item->data(0, Qt::UserRole).toString().toULong(), this);
  connect(editDlg, SIGNAL(destroyed()), SLOT(updateOwners()));
}

void OwnerManagerDlg::removeOwner()
{
  QTreeWidgetItem* item = ownerView->currentItem();
  if (item == NULL)
    return;

  gUserManager.RemoveOwner(item->data(0, Qt::UserRole).toString().toULong());
  gLicqDaemon->SaveConf();
  updateOwners();
}
