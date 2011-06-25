/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2011 Licq developers
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

#include <boost/foreach.hpp>

#include <QAction>
#include <QDialogButtonBox>
#include <QMenu>
#include <QPushButton>
#include <QStringList>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/plugin/pluginmanager.h>

#include "config/iconmanager.h"

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/messagebox.h"
#include "core/signalmanager.h"

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

  myAddMenu = new QMenu(this);

  myAddButton = new QPushButton(tr("&Add"));
  myAddButton->setMenu(myAddMenu);
  buttons->addButton(myAddButton, QDialogButtonBox::ActionRole);

  registerButton = new QPushButton(tr("&Register..."));
  buttons->addButton(registerButton, QDialogButtonBox::ActionRole);

  modifyButton = new QPushButton(tr("&Modify..."));
  buttons->addButton(modifyButton, QDialogButtonBox::ActionRole);

  removeButton = new QPushButton(tr("D&elete..."));
  buttons->addButton(removeButton, QDialogButtonBox::ActionRole);

  closeButton = new QPushButton(tr("&Done"));
  buttons->addButton(closeButton, QDialogButtonBox::RejectRole);

  // Connect all the signals
  connect(ownerView, SIGNAL(itemSelectionChanged()), SLOT(listSelectionChanged()));
  connect(ownerView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
      SLOT(modifyOwner(QTreeWidgetItem*, int)));
  connect(registerButton, SIGNAL(clicked()), SLOT(registerOwner()));
  connect(modifyButton, SIGNAL(clicked()), SLOT(modifyOwner()));
  connect(removeButton, SIGNAL(clicked()), SLOT(removeOwner()));
  connect(closeButton, SIGNAL(clicked()), SLOT(close()));
  connect(gGuiSignalManager, SIGNAL(ownerAdded(const Licq::UserId&)), SLOT(updateOwners()));
  connect(gGuiSignalManager, SIGNAL(ownerRemoved(const Licq::UserId&)), SLOT(updateOwners()));
  connect(gGuiSignalManager, SIGNAL(protocolPlugin(unsigned long)), SLOT(updateProtocols()));

  // Add the owners to the list now
  updateOwners();
  listSelectionChanged();

  // Show information to the user
  if (Licq::gUserManager.NumOwners() == 0)
  {
    InformUser(this, tr("From the Account Manager dialog you can add your accounts or register a new account.\n"
        "Note that only one account per protocol is supported in Licq."));
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

  if (Licq::gUserManager.NumOwners() != 0)
  {
    IconManager* iconman = IconManager::instance();

    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(const Licq::Owner* owner, **ownerList)
    {
      QString id = owner->accountId().c_str();
      unsigned long ppid = owner->protocolId();
      QString proto;
      Licq::ProtocolPlugin::Ptr protocol = Licq::gPluginManager.getProtocolPlugin(ppid);
      if (protocol.get() != NULL)
        proto = protocol->name().c_str();

      QTreeWidgetItem* item = new QTreeWidgetItem(ownerView);
      item->setIcon(0, iconman->iconForStatus(Licq::User::OnlineStatus, owner->id()));
      item->setText(0, proto.isNull() ? tr("(Invalid Protocol)") : proto);
      item->setData(0, Qt::UserRole, QString::number(ppid));
      item->setText(1, id.isNull() ? tr("(Invalid ID)") : id);
    }
  }

  ownerView->resizeColumnToContents(0);
  ownerView->resizeColumnToContents(1);
  ownerView->sortByColumn(0, Qt::AscendingOrder);

  updateProtocols();
}

void OwnerManagerDlg::updateProtocols()
{
  bool enableAdd = false;
  bool enableRegister = false;
  myAddMenu->clear();

  Licq::ProtocolPluginsList protocols;
  Licq::gPluginManager.getProtocolPluginsList(protocols);
  BOOST_FOREACH(Licq::ProtocolPlugin::Ptr protocol, protocols)
  {
    unsigned long ppid = protocol->protocolId();
    Licq::UserId userId = Licq::gUserManager.ownerUserId(ppid);

    if (ppid == LICQ_PPID)
      // ICQ protocol found, allow registering if there is no owner
      enableRegister = !userId.isValid();

    if (userId.isValid())
      // Owner exists, don't allow adding another
      continue;

    enableAdd = true;

    QAction* a = myAddMenu->addAction(QString::fromLocal8Bit(protocol->name().c_str()) + "...", this, SLOT(addOwner()));
    a->setIcon(IconManager::instance()->iconForProtocol(ppid));
    a->setData(QString::number(ppid));
  }

  myAddButton->setEnabled(enableAdd);
  registerButton->setEnabled(enableRegister);
}

void OwnerManagerDlg::listSelectionChanged()
{
  bool hasSelection = !ownerView->selectedItems().isEmpty();

  modifyButton->setEnabled(hasSelection);
  removeButton->setEnabled(hasSelection);
}

void OwnerManagerDlg::addOwner()
{
  QAction* a = qobject_cast<QAction*>(sender());
  if (a == NULL)
    return;

  unsigned long ppid = a->data().toUInt();
  new OwnerEditDlg(ppid, this);
}

void OwnerManagerDlg::registerOwner()
{
  Licq::UserId oldOwnerId = Licq::gUserManager.ownerUserId(LICQ_PPID);
  if (oldOwnerId.isValid())
  {
    QString buf = tr("You are currently registered as\n"
        "UIN (User ID): %1\n"
        "Base Directory: %2\n"
        "Rerun licq with the -b option to select a new\n"
        "base directory and then register a new user.")
        .arg(oldOwnerId.accountId().c_str()).arg(Licq::gDaemon.baseDir().c_str());
    InformUser(this, buf);
    return;
  }

  if (registerUserDlg != 0)
    registerUserDlg->raise();
  else
  {
    registerUserDlg = new RegisterUserDlg(this);
    connect(registerUserDlg, SIGNAL(signal_done(bool, const Licq::UserId&)),
        SLOT(registerDone(bool, const Licq::UserId&)));
  }
}

void OwnerManagerDlg::registerDone(bool success, const Licq::UserId& userId)
{
  registerUserDlg = 0;

  if (success)
  {
    gLicqGui->showInfoDialog(mnuUserGeneral, userId);
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

  if (!QueryYesNo(this, tr("Do you really want to remove account %1?").arg(item->text(1))))
    return;

  Licq::gUserManager.RemoveOwner(item->data(0, Qt::UserRole).toString().toULong());
  Licq::gDaemon.SaveConf();
}
