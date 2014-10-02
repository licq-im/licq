/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2011 Licq developers
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

#include "refusedlg.h"

#include "config.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include <licq/contactlist/user.h>

#include "helpers/support.h"

#include "widgets/mledit.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::RefuseDlg */

RefuseDlg::RefuseDlg(const Licq::UserId& userId, const QString& t, QWidget* parent)
   : QDialog(parent)
{
  Support::setWidgetProps(this, "RefuseDialog");
  setModal(true);

  QVBoxLayout* lay = new QVBoxLayout(this);

  {
    Licq::UserReadGuard u(userId);
    QLabel* lbl = new QLabel(tr("Refusal message for %1 with ").arg(t) +
        QString::fromUtf8(u->getAlias().c_str()) + ":");
    lay->addWidget(lbl);
  }

  mleRefuseMsg = new MLEdit(true);
  mleRefuseMsg->setSizeHintLines(5);
  lay->addWidget(mleRefuseMsg);

  QDialogButtonBox* buttons = new QDialogButtonBox();
  lay->addWidget(buttons);

  QPushButton* btnRefuse = new QPushButton(tr("Refuse"));
  buttons->addButton(btnRefuse, QDialogButtonBox::AcceptRole);
  connect( btnRefuse, SIGNAL(clicked()), SLOT(accept()) );

  QPushButton* btnCancel = new QPushButton(tr("Cancel"));
  buttons->addButton(btnCancel, QDialogButtonBox::RejectRole);
  connect( btnCancel, SIGNAL(clicked()), SLOT(reject()) );

  setWindowTitle(tr("Licq - %1 Refusal").arg(t));
}

QString RefuseDlg::RefuseMessage()
{
  return mleRefuseMsg->toPlainText();
}
