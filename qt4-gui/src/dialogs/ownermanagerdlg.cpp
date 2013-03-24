/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2013 Licq developers <licq-dev@googlegroups.com>
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
#include <QHeaderView>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/plugin/pluginmanager.h>

#include "config/iconmanager.h"

#include "contactlist/contactlist.h"

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/messagebox.h"
#include "core/signalmanager.h"

#include "helpers/support.h"
#include "userdlg/userdlg.h"

#include "ownereditdlg.h"

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
    myPendingAdd(false),
    myPendingRegister(false)
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  Support::setWidgetProps(this, "AccountDialog");
  setWindowTitle(tr("Licq - Account Manager"));

  QVBoxLayout* toplay = new QVBoxLayout(this);

  // Add the list box
  myOwnerView = new QTreeWidget();
  myOwnerView->header()->hide();
  myOwnerView->setItemsExpandable(false);
  toplay->addWidget(myOwnerView);

  // Add the buttons now
  QDialogButtonBox* buttons = new QDialogButtonBox();
  toplay->addWidget(buttons);

  myAddButton = new QPushButton(tr("&Add..."));
  buttons->addButton(myAddButton, QDialogButtonBox::ActionRole);

  myRegisterButton = new QPushButton(tr("&Register..."));
  buttons->addButton(myRegisterButton, QDialogButtonBox::ActionRole);

  myModifyButton = new QPushButton(tr("&Modify..."));
  buttons->addButton(myModifyButton, QDialogButtonBox::ActionRole);

  myRemoveButton = new QPushButton(tr("R&emove..."));
  buttons->addButton(myRemoveButton, QDialogButtonBox::ActionRole);

  buttons->addButton(QDialogButtonBox::Close);

  // Connect all the signals
  connect(myOwnerView, SIGNAL(itemSelectionChanged()), SLOT(listSelectionChanged()));
  connect(myOwnerView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
      SLOT(itemDoubleClicked(QTreeWidgetItem*, int)));
  connect(myAddButton, SIGNAL(clicked()), SLOT(addPressed()));
  connect(myRegisterButton, SIGNAL(clicked()), SLOT(registerPressed()));
  connect(myModifyButton, SIGNAL(clicked()), SLOT(modify()));
  connect(myRemoveButton, SIGNAL(clicked()), SLOT(remove()));
  connect(buttons, SIGNAL(rejected()), SLOT(close()));
  connect(gGuiSignalManager, SIGNAL(ownerAdded(const Licq::UserId&)), SLOT(updateList()));
  connect(gGuiSignalManager, SIGNAL(ownerRemoved(const Licq::UserId&)), SLOT(updateList()));
  connect(gGuiSignalManager, SIGNAL(protocolPluginLoaded(unsigned long)),
      SLOT(protocolLoaded(unsigned long)));
  connect(gGuiSignalManager, SIGNAL(protocolPluginUnloaded(unsigned long)),
      SLOT(updateList()));
  connect(gGuiSignalManager, SIGNAL(updatedStatus(const Licq::UserId&)),
      SLOT(updateList()));

  // Add the protocals and owners to the list
  updateList();

  // Show information to the user
  if (Licq::gUserManager.NumOwners() == 0)
  {
    InformUser(this, tr("From the Account Manager dialog you can add your "
                        "accounts or register a new account."));
  }

  show();
}

OwnerManagerDlg::~OwnerManagerDlg()
{
  myInstance = NULL;
}

void OwnerManagerDlg::updateList()
{
  myOwnerView->clear();

  IconManager* iconman = IconManager::instance();

  // Get currently active protocols
  Licq::ProtocolPluginsList protocols;
  Licq::gPluginManager.getProtocolPluginsList(protocols);
  BOOST_FOREACH(Licq::ProtocolPlugin::Ptr protocol, protocols)
  {
    unsigned long ppid = protocol->protocolId();

    QTreeWidgetItem* protoItem = new QTreeWidgetItem(myOwnerView);
    protoItem->setIcon(0, iconman->iconForProtocol(ppid));
    protoItem->setText(0, QString(tr("%1 (Version: %2)")).arg(protocol->name().c_str())
        .arg(protocol->version().c_str()));
    protoItem->setData(0, Qt::UserRole, (unsigned int)ppid);

    Licq::OwnerListGuard ownerList(ppid);
    BOOST_FOREACH(const Licq::Owner* o, **ownerList)
    {
      Licq::OwnerReadGuard owner(o);

      QTreeWidgetItem* ownerItem = new QTreeWidgetItem(protoItem);
      ownerItem->setIcon(0, iconman->iconForStatus(owner->status(), owner->id()));
      ownerItem->setText(0, QString("%1 (%2)")
          .arg(QString::fromUtf8(owner->id().accountId().c_str()))
          .arg(owner->statusString(true, false).c_str()));
      ownerItem->setData(0, Qt::UserRole, QVariant::fromValue(owner->id()));
      ownerItem->setData(0, Qt::UserRole+1, owner->status());
    }
  }

  std::list<std::string> unloadedProtocols;
  Licq::gPluginManager.getAvailableProtocolPlugins(unloadedProtocols, false);
  BOOST_FOREACH(std::string protocol, unloadedProtocols)
  {
    // Guess protocol id based on name (default is same as ICQ)
    unsigned long ppid = ICQ_PPID;
    if (protocol == "msn")
      ppid = MSN_PPID;
    else if (protocol == "jabber")
      ppid = JABBER_PPID;

    QTreeWidgetItem* protoItem = new QTreeWidgetItem(myOwnerView);
    protoItem->setIcon(0, iconman->iconForProtocol(ppid, Licq::User::OfflineStatus));
    protoItem->setText(0, QString(tr("%1 (Not loaded)").arg(protocol.c_str())));
    protoItem->setData(0, Qt::UserRole, protocol.c_str());
  }

  myOwnerView->expandAll();
  myOwnerView->sortByColumn(0, Qt::AscendingOrder);

  listSelectionChanged();
}

void OwnerManagerDlg::listSelectionChanged()
{
  if (myOwnerView->selectedItems().isEmpty())
  {
    myAddButton->setEnabled(false);
    myRegisterButton->setEnabled(false);
    myModifyButton->setEnabled(false);
    myRemoveButton->setEnabled(false);
    return;
  }

  const QTreeWidgetItem* item = myOwnerView->selectedItems().front();
  QVariant data = item->data(0, Qt::UserRole);
  bool hasChildren = (item->childCount() > 0);
  switch (data.type())
  {
    case QVariant::String: // data is name of unloaded protocol
      myAddButton->setEnabled(true);
      myRegisterButton->setEnabled(data.toString() == "icq" || data.toString() == "msn");
      myModifyButton->setEnabled(false);
      myRemoveButton->setEnabled(false);
      break;
    case QVariant::UInt: // data is id of loaded protocol
    {
      unsigned long protocolId = data.toUInt();
      Licq::ProtocolPlugin::Ptr plugin = Licq::gPluginManager.getProtocolPlugin(protocolId);
      bool mayAdd = (!hasChildren || (plugin->capabilities() & Licq::ProtocolPlugin::CanMultipleOwners));
      myAddButton->setEnabled(mayAdd);
      myRegisterButton->setEnabled(mayAdd && (protocolId == ICQ_PPID || protocolId == MSN_PPID));
      myModifyButton->setEnabled(false);
      myRemoveButton->setEnabled(!hasChildren);
      break;
    }
    default: // data is id of owner
      myAddButton->setEnabled(false);
      myRegisterButton->setEnabled(false);
      myModifyButton->setEnabled(true);
      myRemoveButton->setEnabled(item->data(0, Qt::UserRole+1).toUInt() == Licq::Owner::OfflineStatus);
  }
}

void OwnerManagerDlg::protocolLoaded(unsigned long protocolId)
{
  updateList();

  // We don't have a good way of matching the name of an unloaded protocol
  // with the new ppid so just assume the first one we get is the right one.
  if (myPendingAdd)
  {
    myPendingAdd = false;
    addOwner(protocolId);
  }
  if (myPendingRegister)
  {
    myPendingRegister = false;
    registerOwner(protocolId);
  }
}

void OwnerManagerDlg::addPressed()
{
  const QTreeWidgetItem* item = myOwnerView->currentItem();
  if (item == NULL)
    return;
  QVariant data = item->data(0, Qt::UserRole);

  if (data.type() == QVariant::String)
  {
    // Protocol needs to be loaded before owner can be added
    myPendingAdd = true;
    Licq::gPluginManager.startProtocolPlugin(data.toString().toLatin1().constData());
  }
  else
  {
    // Protocol is already loaded
    addOwner(data.toUInt());
  }
}

void OwnerManagerDlg::addOwner(unsigned long protocolId)
{
  new OwnerEditDlg(protocolId, this);
}

void OwnerManagerDlg::registerPressed()
{
  const QTreeWidgetItem* item = myOwnerView->currentItem();
  if (item == NULL)
    return;
  QVariant data = item->data(0, Qt::UserRole);

  if (data.type() == QVariant::String)
  {
    // Protocol needs to be loaded before owner can be registered
    myPendingRegister = true;
    Licq::gPluginManager.startProtocolPlugin(data.toString().toLatin1().constData());
  }
  else
  {
    // Protocol is already loaded
    registerOwner(data.toUInt());
  }
}

void OwnerManagerDlg::registerOwner(unsigned long protocolId)
{
  switch (protocolId)
  {
    case ICQ_PPID:
      gLicqGui->viewUrl("https://www.icq.com/join");
      break;
    case MSN_PPID:
      gLicqGui->viewUrl("https://signup.live.com/signup.aspx");
      break;
  }
}

void OwnerManagerDlg::modify()
{
  const QTreeWidgetItem* item = myOwnerView->currentItem();
  if (item == NULL)
    return;
  Licq::UserId ownerId = item->data(0, Qt::UserRole).value<Licq::UserId>();

  UserDlg::showDialog(ownerId, UserDlg::OwnerPage);
}

void OwnerManagerDlg::itemDoubleClicked(QTreeWidgetItem* item, int /* column */)
{
  if (item == NULL)
    return;
  QVariant data = item->data(0, Qt::UserRole);
  switch (data.type())
  {
    case QVariant::String: // Unloaded protocol - Load it
      Licq::gPluginManager.startProtocolPlugin(data.toString().toLatin1().constData());
      break;
    case QVariant::UInt: // Loaded protocol - Add owner
      if (item->childCount() == 0)
        new OwnerEditDlg(data.toUInt(), this);
      break;
    default: // Owner - Modify
      Licq::UserId ownerId = data.value<Licq::UserId>();
      UserDlg::showDialog(ownerId, UserDlg::OwnerPage);
  }
}

void OwnerManagerDlg::remove()
{
  const QTreeWidgetItem* item = myOwnerView->currentItem();
  if (item == NULL)
    return;
  QVariant data = item->data(0, Qt::UserRole);

  if (data.type() == QVariant::UInt)
  {
    // This is a protocol, unload it
    Licq::ProtocolPlugin::Ptr plugin = Licq::gPluginManager.getProtocolPlugin(data.toUInt());
    Licq::gPluginManager.unloadProtocolPlugin(plugin);
  }
  else
  {
    // Remove owner after checking with user
    if (!QueryYesNo(this, tr("Do you really want to remove account %1?").arg(item->text(1))))
      return;

    Licq::UserId ownerId = data.value<Licq::UserId>();
    Licq::gUserManager.removeOwner(ownerId);
  }
}
