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

#include "gpgkeyselect.h"

#include "config.h"

#include <cstring>
#include <memory>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/gpghelper.h>
#include <licq/pluginsignal.h>


#include "helpers/support.h"

using namespace std;

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::GPGKeySelect */
/* TRANSLATOR LicqQtGui::KeyView */

GPGKeySelect::GPGKeySelect(const Licq::UserId& userId, QWidget* parent)
  : QDialog(parent),
    myUserId(userId)
{
  if (!userId.isValid())
    return;

  setAttribute(Qt::WA_DeleteOnClose, true);
  Support::setWidgetProps(this, "GPGKeySelectDialog");

  QVBoxLayout* top_lay = new QVBoxLayout(this);

  {
    Licq::UserReadGuard u(myUserId);
    if (!u.isLocked())
      return;

    setWindowTitle(tr("Select GPG Key for user %1")
        .arg(QString::fromUtf8(u->getAlias().c_str())));

    top_lay->addWidget(new QLabel(tr("Select a GPG key for user %1.")
        .arg(QString::fromUtf8(u->getAlias().c_str()))));
    if (u->gpgKey().empty())
      top_lay->addWidget(new QLabel(tr("Current key: No key selected")));
    else
      top_lay->addWidget(new QLabel(tr("Current key: %1")
            .arg(QString::fromLocal8Bit(u->gpgKey().c_str()))));

    useGPG = new QCheckBox(tr("Use GPG Encryption"));
    useGPG->setChecked(u->UseGPG() || u->gpgKey().empty());
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
  }

  // public keys
  keySelect = new KeyView(myUserId);
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

    {
      Licq::UserWriteGuard u(myUserId);
      if (u.isLocked())
      {
        u->setGpgKey(curItem->text(2).toAscii().data());
        u->SetUseGPG(useGPG->isChecked());
      }
    }

    // Notify all plugins (including ourselves)
    Licq::gUserManager.notifyUserUpdated(myUserId, Licq::PluginSignal::UserSecurity);
  }

  close();
}

void GPGKeySelect::slotNoKey()
{
  {
    Licq::UserWriteGuard u(myUserId);
    if (u.isLocked())
      u->setGpgKey("");
  }

  // Notify all plugins (including ourselves)
  Licq::gUserManager.notifyUserUpdated(myUserId, Licq::PluginSignal::UserSecurity);

  close();
};

void GPGKeySelect::slotCancel()
{
  close();
}


KeyView::KeyView(const Licq::UserId& userId, QWidget* parent)
  : QTreeWidget(parent),
    myUserId(userId)
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

  int totalWidth = 0;
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

void KeyView::testViewItem(QTreeWidgetItem* item, const Licq::User* u)
{
  int val = 0;
  for (int i = 0; i < 2; ++i)
  {
    if (item->text(i).contains(u->getFirstName().c_str(), Qt::CaseInsensitive))
      val++;
    if (item->text(i).contains(u->getLastName().c_str(), Qt::CaseInsensitive))
      val++;
    if (item->text(i).contains(u->getAlias().c_str(), Qt::CaseInsensitive))
      val++;
    if (item->text(i).contains(u->getEmail().c_str(), Qt::CaseInsensitive))
      val++;
  }

  if (item->text(2).contains(u->gpgKey().c_str(), Qt::CaseInsensitive))
    val += 10;

  if (val > maxItemVal)
  {
    maxItemVal = val;
    maxItem = item;
  }
}

void KeyView::initKeyList()
{
  Licq::UserReadGuard u(myUserId);
  maxItemVal = -1;
  maxItem = NULL;

  auto_ptr<list<Licq::GpgKey> > keyList(Licq::gGpgHelper.getKeyList());
  list<Licq::GpgKey>::const_iterator i;
  for (i = keyList->begin(); i != keyList->end(); ++i)
  {
    // There shouldn't be any key without a user id in list, but make just in case
    if (i->uids.empty())
      continue;

    // First user id is primary uid
    list<Licq::GpgUid>::const_iterator uid = i->uids.begin();

    QStringList cols;
    cols << QString::fromUtf8(uid->name.c_str());
    cols << QString::fromUtf8(uid->email.c_str());
    cols << QString(i->keyid.c_str()).right(8);
    QTreeWidgetItem* keyItem = new QTreeWidgetItem(this, cols);
    if (u.isLocked())
      testViewItem(keyItem, *u);

    ++uid;
    for ( ;uid != i->uids.end(); ++uid)
    {
      cols.clear();
      cols << QString::fromUtf8(uid->name.c_str());
      cols << QString::fromUtf8(uid->email.c_str());
      QTreeWidgetItem* uidItem = new QTreeWidgetItem(keyItem, cols);
      if (u.isLocked())
        testViewItem(uidItem, *u);
    }
  }

  if (maxItem)
    setCurrentItem(maxItem);
}
