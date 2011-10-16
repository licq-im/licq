/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2005-2011 Licq developers
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

#include "gpgkeymanager.h"

#include "config.h"

#include <boost/foreach.hpp>

#include <QDialogButtonBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/pluginsignal.h>

#include "contactlist/contactlist.h"
#include "core/messagebox.h"

#include "helpers/support.h"

#include "gpgkeyselect.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::GPGKeyManager */
/* TRANSLATOR LicqQtGui::KeyListItem */

struct luser
{
  Licq::UserId userId;
  QString alias;
};

bool compare_luser(const struct luser& left, const struct luser& right)
{
  return QString::compare(left.alias, right.alias, Qt::CaseInsensitive) <= 0;
}

GPGKeyManager::GPGKeyManager(QWidget* parent)
  : QDialog(parent)
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  Support::setWidgetProps(this, "GPGKeyManager");
  setWindowTitle(tr("Licq - GPG Key Manager"));

  QVBoxLayout* lay_main = new QVBoxLayout(this);

  lst_keyList = new KeyList();
  lst_keyList->setAllColumnsShowFocus(true);
  QStringList headers;
  headers << tr("User") << tr("Active") << tr("Key ID");
  lst_keyList->setHeaderLabels(headers);
  connect(lst_keyList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
      SLOT(slot_doubleClicked(QTreeWidgetItem*)));
  lay_main->addWidget(lst_keyList);
  connect(lst_keyList, SIGNAL(itemSelectionChanged()), SLOT(keySelectionChanged()));

  myUsersMenu = new QMenu(this);
  connect(myUsersMenu, SIGNAL(aboutToShow()), SLOT(showAddMenu()));
  connect(myUsersMenu, SIGNAL(triggered(QAction*)), SLOT(addUser(QAction*)));

  QDialogButtonBox* buttons = new QDialogButtonBox();
  lay_main->addWidget(buttons);

  QPushButton* addButton = buttons->addButton(tr("&Add"), QDialogButtonBox::ActionRole);
  addButton->setMenu(myUsersMenu);

#define BUTTON(btn, role, name, slot) \
  btn = buttons->addButton(name, QDialogButtonBox::role); \
  connect(btn, SIGNAL(clicked()), SLOT(slot()))

  BUTTON(myEditButton, ActionRole, tr("&Edit..."), slot_edit);
  BUTTON(myRemoveButton, ActionRole, tr("&Remove"), slot_remove);

#undef BUTTON
  buttons->addButton(QDialogButtonBox::Close);
  connect(buttons, SIGNAL(rejected()), SLOT(close()));

  initKeyList();
  keySelectionChanged();
  show();
}

void GPGKeyManager::slot_edit()
{
  slot_doubleClicked(lst_keyList->currentItem());
}

void GPGKeyManager::slot_doubleClicked(QTreeWidgetItem* item)
{
  if (item != NULL)
    dynamic_cast<KeyListItem*>(item)->edit();
}

void GPGKeyManager::showAddMenu()
{
  // Remove existing entries in menu
  myUsersMenu->clear();

  QList<luser> list;

  {
    Licq::UserListGuard userList;
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      Licq::UserReadGuard u(user);

      if (!u->gpgKey().empty())
        continue;

      luser tmp;
      tmp.userId = u->id();
      tmp.alias = QString::fromUtf8(u->getAlias().c_str());
      list.append(tmp);
    }
  }

  qSort(list.begin(), list.end(), compare_luser);

  for (int i = 0; i < list.count(); i++)
    myUsersMenu->addAction(list.at(i).alias)->setData(QVariant::fromValue(list.at(i).userId));
}

void GPGKeyManager::addUser(QAction* res)
{
  lst_keyList->editUser(res->data().value<Licq::UserId>());
}

void GPGKeyManager::slot_remove()
{
  KeyListItem* item = (KeyListItem*)lst_keyList->currentItem();
  if (item != NULL)
  {
    if (QueryYesNo(this, tr("Do you want to remove the GPG key binding for the user %1?\n"
            "The key isn't deleted from your keyring.")
          .arg(item->text(0))))
    {
      item->unsetKey();
      delete item;
      lst_keyList->resizeColumnsToContents();
    }
  }
}

void GPGKeyManager::initKeyList()
{
  Licq::UserListGuard userList;
  BOOST_FOREACH(const Licq::User* user, **userList)
  {
    Licq::UserReadGuard u(user);

    if (!u->gpgKey().empty())
      new KeyListItem(lst_keyList, *u);
  }

  lst_keyList->resizeColumnsToContents();
}

void GPGKeyManager::keySelectionChanged()
{
  bool hasSelection = !lst_keyList->selectedItems().isEmpty();
  myEditButton->setEnabled(hasSelection);
  myRemoveButton->setEnabled(hasSelection);
}

// THE KEYLIST
KeyList::KeyList(QWidget* parent)
  : QTreeWidget(parent)
{
  setAcceptDrops(true);
  setRootIsDecorated(false);
}

void KeyList::editUser(const Licq::UserId& userId)
{
  KeyListItem* item = NULL;
  bool found = false;

  for (int i = 0; i < topLevelItemCount(); ++i)
  {
    item = dynamic_cast<KeyListItem*>(topLevelItem(i));

    if (item->getUserId() == userId)
    {
      found = true;
      break;
    }
  }

  if (!found)
  {
    Licq::UserReadGuard u(userId);
    if (!u.isLocked())
      return;
    item = new KeyListItem(this, *u);
    resizeColumnsToContents();
  }

  item->edit();
};

void KeyList::dragEnterEvent(QDragEnterEvent* event)
{
  if (event->mimeData()->hasText())
    event->acceptProposedAction();
}

void KeyList::dropEvent(QDropEvent* event)
{
  if (!event->mimeData()->hasText())
    return;

  QString text = event->mimeData()->text();

  if (text.length() <= 4)
    return;

  unsigned long nPPID = 0;

  {
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(Licq::Owner* owner, **ownerList)
    {
      unsigned long ppid = owner->protocolId();
      if (text.startsWith(Licq::protocolId_toString(ppid).c_str()))
      {
        nPPID = ppid;
        break;
      }
    }
  }

  if (nPPID == 0)
    return;

  editUser(Licq::UserId(text.mid(4).toLatin1().constData(), nPPID));
}

void KeyList::resizeEvent(QResizeEvent* e)
{
  QTreeWidget::resizeEvent(e);

  int totalWidth = 0;
  int nNumCols = columnCount();
  for (int i = 1; i < nNumCols; ++i)
    totalWidth += columnWidth(i);

  int newWidth = width() - totalWidth - 2;
  if (newWidth <= 0)
  {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setColumnWidth(0, 2);
  }
  else
  {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setColumnWidth(0, newWidth);
  }
}

void KeyList::resizeColumnsToContents()
{
  for (int i = 0; i < columnCount(); i++)
    resizeColumnToContents(i);
}

// KEYLISTITEM
KeyListItem::KeyListItem(QTreeWidget* parent, const Licq::User* u)
  : QTreeWidgetItem(parent),
    myUserId(u->id()),
    keySelect(NULL)
{
  updateText(u);
}

void KeyListItem::updateText(const Licq::User* u)
{
  setText(0, QString::fromUtf8(u->getAlias().c_str()));
  setText(1, u->UseGPG() ? tr("Yes") : tr("No"));
  setText(2, u->gpgKey().c_str());
}

void KeyListItem::edit()
{
  if (keySelect == NULL)
  {
    keySelect = new GPGKeySelect(myUserId);
    connect(keySelect, SIGNAL(signal_done()), SLOT(slot_done()));
  }
}

void KeyListItem::slot_done()
{
  Licq::UserReadGuard u(myUserId);
  keySelect = NULL;

  if (u.isLocked())
  {
    if (u->gpgKey().empty())
      delete this;
    else
      updateText(*u);
    dynamic_cast<KeyList*>(treeWidget())->resizeColumnsToContents();
  }
}

void KeyListItem::unsetKey()
{
  {
    Licq::UserWriteGuard u(myUserId);
    if (u.isLocked())
    {
      u->SetUseGPG(false);
      u->setGpgKey("");
    }
  }

  // Notify all plugins (including ourselves)
  Licq::gUserManager.notifyUserUpdated(myUserId, Licq::PluginSignal::UserSecurity);
}
