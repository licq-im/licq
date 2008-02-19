/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2005-2006 Licq developers
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

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq_events.h>
#include <licq_icqd.h>
#include <licq_user.h>
extern char *PPIDSTRING(unsigned long);

#include "core/mainwin.h"

#include "helpers/support.h"

#include "gpgkeyselect.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::GPGKeyManager */
/* TRANSLATOR LicqQtGui::KeyListItem */

struct luser
{
  QString szId;
  unsigned long nPPID;
  QString alias;
};

bool compare_luser(const struct luser& left, const struct luser& right)
{
  return left.alias < right.alias;
}

GPGKeyManager::GPGKeyManager(QWidget* parent)
  : QDialog(parent)
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  Support::setWidgetProps(this, "GPGKeyManager");
  setWindowTitle(tr("Licq GPG Key Manager"));

  QVBoxLayout* lay_main = new QVBoxLayout(this);

  // GPG Passphrase box
  QGroupBox* box_passphrase = new QGroupBox(tr("GPG Passphrase"));
  lay_main->addWidget( box_passphrase );

  QHBoxLayout* lay_passphrase = new QHBoxLayout(box_passphrase);

  lbl_passphrase = new QLabel(tr("No passphrase set"));
  lay_passphrase->addWidget(lbl_passphrase);
  QPushButton* btn_setPass = new QPushButton(tr("&Set GPG Passphrase"));
  connect(btn_setPass, SIGNAL(clicked()), SLOT(slot_setPassphrase()));
  lay_passphrase->addWidget(btn_setPass);

  // GPG User/Key list
  QHBoxLayout* lay_hbox = new QHBoxLayout();
  lay_main->addLayout(lay_hbox);
  QGroupBox* box_keys = new QGroupBox(tr("User Keys"));
  lay_hbox->addWidget(box_keys, 1);
  QVBoxLayout* lay_keys = new QVBoxLayout(box_keys);

  QVBoxLayout* lay_buttonBox = new QVBoxLayout();
  lay_hbox->addLayout(lay_buttonBox, 0);
  QPushButton* btn_add = new QPushButton(tr("&Add"));
  QPushButton* btn_edit = new QPushButton(tr("&Edit"));
  QPushButton* btn_remove = new QPushButton(tr("&Remove"));
  connect(btn_add, SIGNAL(clicked()), SLOT(slot_add()));
  connect(btn_edit, SIGNAL(clicked()), SLOT(slot_edit()));
  connect(btn_remove, SIGNAL(clicked()), SLOT(slot_remove()));
  lbl_dragndrop = new QLabel(tr("Drag&Drop user to add to list."));
  lbl_dragndrop->setWordWrap(true);

  // create the keylist
  lst_keyList = new KeyList();
  lst_keyList->setAllColumnsShowFocus(true);
  QStringList headers;
  headers << tr("User") << tr("Active") << tr("Key ID");
  lst_keyList->setHeaderLabels(headers);
  connect(lst_keyList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
      SLOT(slot_doubleClicked(QTreeWidgetItem*)));
  lay_keys->addWidget(lst_keyList);

  lay_buttonBox->addWidget(btn_add);
  lay_buttonBox->addWidget(btn_edit);
  lay_buttonBox->addWidget(btn_remove);
  lay_buttonBox->addWidget(lbl_dragndrop);
  lay_buttonBox->addStretch();

  // Close
  QDialogButtonBox* buttons = new QDialogButtonBox();
  lay_main->addWidget(buttons);

  QPushButton* btn_close = buttons->addButton(QDialogButtonBox::Close);
  btn_close->setText(tr("&Close"));
  connect(btn_close, SIGNAL(clicked()), SLOT(slot_close()));

  initKeyList();

  show();
}

GPGKeyManager::~GPGKeyManager()
{}

void GPGKeyManager::slot_setPassphrase()
{
  QMessageBox::information( this, tr("Set Passphrase"), tr("Not yet implemented. Use licq_gpg.conf.") );
}

void GPGKeyManager::slot_close()
{
  close();
}

void GPGKeyManager::slot_edit()
{
  QTreeWidgetItem* item = lst_keyList->currentItem();
  if (item )
    ( (KeyListItem*)item )->edit();
}

void GPGKeyManager::slot_doubleClicked(QTreeWidgetItem* item)
{
  if (item )
    ( (KeyListItem*)item )->edit();
}

void GPGKeyManager::slot_add()
{
  QMenu popupMenu;
  QList<luser> list;

  FOR_EACH_USER_START(LOCK_R)
  {
    if (strcmp( pUser->GPGKey(), "") == 0)
    {
      luser tmp;
      tmp.szId = pUser->IdString();
      tmp.nPPID = pUser->PPID();
      tmp.alias = QString::fromUtf8(pUser->GetAlias());
      list.append(tmp);
    }
  }
  FOR_EACH_USER_END

  qSort(list.begin(), list.end(), compare_luser);

  for (int i = 0; i < list.count(); i++)
    popupMenu.addAction(list.at(i).alias)->setData(i);

  QAction* res = popupMenu.exec(QCursor::pos());
  if (res == NULL)
    return;
  const luser* tmp = &list.at(res->data().toInt());
  if (tmp == NULL)
    return;

  ICQUser* u = gUserManager.FetchUser(tmp->szId.toLatin1(), tmp->nPPID, LOCK_R);
  if ( u )
  {
    editUser( u );
    gUserManager.DropUser(u);
  }
}

void GPGKeyManager::editUser(ICQUser* u)
{
  for (int i = 0; i < lst_keyList->topLevelItemCount(); ++i)
  {
    KeyListItem* item = dynamic_cast<KeyListItem*>(lst_keyList->topLevelItem(i));

    if (item->getszId() == u->IdString() && item->getnPPID() == u->PPID())
    {
      item->edit();
      return;
    }
  }

  (new KeyListItem(lst_keyList, u))->edit();
};

void GPGKeyManager::slot_remove()
{
  KeyListItem* item = (KeyListItem*)lst_keyList->currentItem();
  if ( item )
  {
    if ( QMessageBox::question( this, tr("Remove GPG key"), tr("Do you want to remove the GPG key? The key isn't deleted from your keyring."), QMessageBox::Yes, QMessageBox::No )==QMessageBox::Yes )
    {
      item->unsetKey();
      delete item;
    };
  }
}

void GPGKeyManager::initKeyList()
{
  FOR_EACH_USER_START(LOCK_R)
  {
    if ( strcmp( pUser->GPGKey(), "" )!= 0 )
    {
      new KeyListItem( lst_keyList, pUser );
    }
  }
  FOR_EACH_USER_END
}



// THE KEYLIST
KeyList::KeyList(QWidget* parent)
  : QTreeWidget(parent)
{
  setAcceptDrops(true);
}

KeyList::~KeyList()
{
}

void KeyList::dragEnterEvent(QDragEnterEvent* event)
{
  if (event->mimeData()->hasText())
    event->acceptProposedAction();
}

void KeyList::dragMoveEvent(QDragMoveEvent* /* event */)
{
  // Do nothing, just overload function so base class won't interfere
}

void KeyList::dropEvent(QDropEvent* event)
{
  if (!event->mimeData()->hasText())
    return;
  QString text = event->mimeData()->text();

  unsigned long nPPID = 0;
  FOR_EACH_PROTO_PLUGIN_START(gLicqDaemon)
  {
    if (text.startsWith(PPIDSTRING((*_ppit)->PPID())))
    {
      nPPID = (*_ppit)->PPID();
      break;
    }
  }
  FOR_EACH_PROTO_PLUGIN_END;

  if (nPPID == 0 || text.length() <= 4)
    return;

  QString szId = text.mid(4);

  ICQUser* u = gUserManager.FetchUser(szId.toLatin1(), nPPID, LOCK_R);

  if (u != NULL)
  {
    bool found = false;
    for (int i = 0; i < topLevelItemCount(); ++i)
    {
      KeyListItem* item = dynamic_cast<KeyListItem*>(topLevelItem(i));
      if (item->getszId() == szId && item->getnPPID() == nPPID)
      {
        item->edit();
        found = true;
        break;
      }
    }
    if (!found)
      (new KeyListItem(this, u))->edit();
    gUserManager.DropUser(u);
  }
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

// KEYLISTITEM
KeyListItem::KeyListItem(QTreeWidget* parent, ICQUser* u)
  : QTreeWidgetItem(parent),
    szId(u->IdString()),
    nPPID(u->PPID()),
    keySelect(NULL)
{
  updateText( u );
}

KeyListItem::~KeyListItem()
{
}

void KeyListItem::updateText(ICQUser* u)
{
  setText( 0, QString::fromUtf8(u->GetAlias()) );
  setText( 1, u->UseGPG() ? tr("Yes") : tr("No") );
  setText( 2, u->GPGKey() );
}

void KeyListItem::edit()
{
  if ( !keySelect )
  {
    keySelect = new GPGKeySelect(szId, nPPID);
    connect(keySelect, SIGNAL(signal_done()), SLOT(slot_done()));
  }
}

void KeyListItem::slot_done()
{
  ICQUser* u = gUserManager.FetchUser(szId.toLatin1(), nPPID, LOCK_R);
  keySelect = NULL;

  if ( u )
  {
    if ( strcmp(u->GPGKey(),"")==0 )
      delete this;
    else
      updateText( u );
    gUserManager.DropUser(u);
  }
}

void KeyListItem::unsetKey()
{
  ICQUser* u = gUserManager.FetchUser(szId.toLatin1(), nPPID, LOCK_W);

  if (u != NULL)
  {
    u->SetUseGPG(false);
    u->SetGPGKey("");
    gUserManager.DropUser(u);
    CICQSignal s(SIGNAL_UPDATExUSER, USER_GENERAL, szId.toLatin1(), nPPID);
    gMainWindow->slot_updatedUser(&s);
  }
}
