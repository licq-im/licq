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

#include "gpgkeyselect.h"

#include "config.h"

#include <cstring>
#include <gpgme.h>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq_user.h>
#include <licq_events.h>

#include "core/mainwin.h"

#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::GPGKeySelect */
/* TRANSLATOR LicqQtGui::KeyView */

GPGKeySelect::GPGKeySelect(QString id, unsigned long ppid, QWidget* parent)
  : QDialog(parent),
    szId(id),
    nPPID(ppid)
{
  if (id.isNull() || !nPPID)
    return;

  setAttribute(Qt::WA_DeleteOnClose, true);
  Support::setWidgetProps(this, "GPGKeySelectDialog");

  const ICQUser* u = gUserManager.FetchUser(szId.toLatin1(), nPPID, LOCK_R);
  if (u == NULL)
    return;

  setWindowTitle(tr("Select GPG Key for user %1")
      .arg(QString::fromUtf8(u->GetAlias())));

  QVBoxLayout* top_lay = new QVBoxLayout(this);

  top_lay->addWidget(new QLabel(tr("Select a GPG key for user %1.")
        .arg(QString::fromUtf8(u->GetAlias()))));
  if (strcmp(u->GPGKey(), "") == 0)
    top_lay->addWidget(new QLabel(tr("Current key: No key selected")));
  else
    top_lay->addWidget(new QLabel(tr("Current key: %1")
          .arg(QString::fromLocal8Bit(u->GPGKey()))));

  useGPG = new QCheckBox(tr("Use GPG Encryption"));
  useGPG->setChecked(u->UseGPG() || strcmp(u->GPGKey(), "") == 0);
  top_lay->addWidget(useGPG);

  // Filter
  QHBoxLayout* filterLayout = new QHBoxLayout();
  top_lay->addLayout(filterLayout);
  filterLayout->addWidget(new QLabel(tr("Filter:")));
  QLineEdit* filterText = new QLineEdit();
  filterText->setFocus();
  connect(filterText, SIGNAL(textChanged(const QString&)),
      SLOT(filterTextChanged(const QString&)));
  filterLayout->addWidget(filterText);

  gUserManager.DropUser(u);

  // public keys
  keySelect = new KeyView(szId, nPPID);
  top_lay->addWidget(keySelect);
  connect(keySelect, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
      SLOT(slot_doubleClicked(QTreeWidgetItem*, int)));

  QDialogButtonBox* buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel |
      QDialogButtonBox::Discard);

  QPushButton* btnNoKey = buttons->button(QDialogButtonBox::Discard);
  btnNoKey->setText(tr("&No Key"));

  connect(buttons, SIGNAL(accepted()), SLOT(slot_ok()));
  connect(buttons, SIGNAL(rejected()), SLOT(slotCancel()));
  connect(btnNoKey, SIGNAL(clicked()), SLOT(slotNoKey()));

  top_lay->addWidget(buttons);

  show();
}

GPGKeySelect::~GPGKeySelect()
{
  emit signal_done();
}

void GPGKeySelect::filterTextChanged(const QString& str)
{
  for (int i = 0; i < keySelect->topLevelItemCount(); ++i)
  {
    QTreeWidgetItem* item = keySelect->topLevelItem(i);
    item->setHidden(!item->text(0).contains(str, Qt::CaseInsensitive) &&
        !item->text(1).contains(str, Qt::CaseInsensitive) &&
        !item->text(2).contains(str, Qt::CaseInsensitive));
  }
}

void GPGKeySelect::slot_doubleClicked(QTreeWidgetItem* item, int /* column */)
{
  if ( item )
    slot_ok();
}

void GPGKeySelect::slot_ok()
{
  QTreeWidgetItem* curItem = keySelect->currentItem();

  if (curItem != NULL)
  {
    if (curItem->parent() != NULL)
      curItem = curItem->parent();

    ICQUser* u = gUserManager.FetchUser(szId.toLatin1(), nPPID, LOCK_W);
    if (u != NULL)
    {
      u->SetGPGKey(curItem->text(2).toAscii());
      u->SetUseGPG(useGPG->isChecked());
      gUserManager.DropUser(u);
      updateIcon();
    }
  }

  close();
}

void GPGKeySelect::updateIcon()
{
  CICQSignal s(SIGNAL_UPDATExUSER, USER_GENERAL, szId.toLatin1(), nPPID);
  gMainWindow->slot_updatedUser(&s);
  return;
}

void GPGKeySelect::slotNoKey()
{
  ICQUser* u = gUserManager.FetchUser(szId.toLatin1(), nPPID, LOCK_W);
  if ( u )
  {
    u->SetGPGKey( "" );
    gUserManager.DropUser( u );
    updateIcon();
  }

  close();
};

void GPGKeySelect::slotCancel()
{
  close();
}

gpgme_ctx_t mCtx;
gpgme_key_t key;


KeyView::KeyView(QString id, unsigned long ppid, QWidget* parent)
  : QTreeWidget(parent),
    szId(id),
    nPPID(ppid)
{
  header()->setClickable(false);
  QStringList headers;
  headers << tr("Name") << tr("EMail") << tr("ID");
  setHeaderLabels(headers);

  setAllColumnsShowFocus(true);

  initKeyList();

  setRootIsDecorated(true);
}

void KeyView::resizeEvent(QResizeEvent* event)
{
  QTreeWidget::resizeEvent(event);

  unsigned short totalWidth = 0;
  int nNumCols = columnCount();
  for (int i = 0; i < nNumCols - 1; i++)
    totalWidth += columnWidth(i);

  int newWidth = width() - totalWidth - 2;
  if (newWidth <= 0)
  {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setColumnWidth(nNumCols - 1, 2);
  }
  else
  {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setColumnWidth(nNumCols - 1, newWidth);
  }
}

void KeyView::testViewItem(QTreeWidgetItem* item, const ICQUser* u)
{
  int val = 0;
  for (int i = 0; i < 2; ++i)
  {
    if (item->text(i).contains(u->GetFirstName(), Qt::CaseInsensitive))
      val++;
    if (item->text(i).contains(u->GetLastName(), Qt::CaseInsensitive))
      val++;
    if (item->text(i).contains(u->GetAlias(), Qt::CaseInsensitive))
      val++;
    if (item->text(i).contains(u->GetEmailPrimary(), Qt::CaseInsensitive))
      val++;
  }

  if (item->text(2).contains(u->GPGKey(), Qt::CaseInsensitive))
    val += 10;

  if (val > maxItemVal)
  {
    maxItemVal = val;
    maxItem = item;
  }
}

void KeyView::initKeyList()
{
  gpgme_new(&mCtx);

  const ICQUser* u = gUserManager.FetchUser(szId.toLatin1(), nPPID, LOCK_R);
  maxItemVal = -1;
  maxItem = NULL;

  int err = gpgme_op_keylist_start(mCtx, NULL, 0);

  while (!err)
  {
    err = gpgme_op_keylist_next(mCtx, &key);
    if (err)
      break;
    gpgme_user_id_t uid = key->uids;
    if (uid && key->can_encrypt && key->subkeys)
    {
      QStringList fColumns;
      fColumns << QString::fromUtf8(uid->name);
      fColumns << QString::fromUtf8(uid->email);
      fColumns << QString(key->subkeys->keyid).right(8);
      QTreeWidgetItem* f = new QTreeWidgetItem(this, fColumns);
      if (u)
        testViewItem(f, u);
      uid = uid->next;
      while (uid)
      {
        QStringList gColumns;
        gColumns << QString::fromUtf8(uid->name);
        gColumns << QString::fromUtf8(uid->email);
        QTreeWidgetItem* g = new QTreeWidgetItem(f, gColumns);
        if (u)
          testViewItem(g, u);
        uid = uid->next;
      }
    }
    gpgme_key_release(key);
  }

  if (u)
    gUserManager.DropUser(u);

  gpgme_release(mCtx);
  if (maxItem)
    setCurrentItem(maxItem);
}
