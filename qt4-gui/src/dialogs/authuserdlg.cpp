// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#include "authuserdlg.h"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextCodec>
#include <QVBoxLayout>

#include <licq_icqd.h>
#include <licq_user.h>

#include "helpers/support.h"
#include "helpers/usercodec.h"

#include "widgets/mledit.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::AuthUserDlg */

AuthUserDlg::AuthUserDlg(const UserId& userId, bool grant, QWidget* parent)
  : QDialog(parent),
    myUserId(userId),
    myGrant(grant)
{
  Support::setWidgetProps(this, "AuthUserDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("Licq - %1 Authorization")
      .arg(myGrant ? tr("Grant") : tr("Refuse")));

  QVBoxLayout* toplay = new QVBoxLayout(this);

  QLabel* lblUin = new QLabel();
  lblUin->setAlignment(Qt::AlignCenter);
  if (!USERID_ISVALID(myUserId))
  {
    lblUin->setText(tr("User Id:"));
    myUin = new QLineEdit();
    connect(myUin, SIGNAL(returnPressed()), SLOT(ok()));
    QHBoxLayout* lay = new QHBoxLayout();
    lay->addWidget(lblUin);
    lay->addWidget(myUin);
    toplay->addLayout(lay);
  }
  else
  {
    myUin = NULL;
    toplay->addWidget(lblUin);
    QString userName = LicqUser::getUserAccountId(myUserId).c_str();
    const LicqUser* u = gUserManager.fetchUser(myUserId);
    if (u != NULL)
    {
       userName = QString("%1 (%2)").arg(QString::fromUtf8(u->GetAlias())).arg(u->accountId().c_str());
       gUserManager.DropUser(u);
    }

    lblUin->setText(tr("%1 authorization to %2")
        .arg(myGrant ? tr("Grant") : tr("Refuse"))
        .arg(userName));
  }

  QGroupBox* grpResponse = new QGroupBox(tr("Response"));
  toplay->addWidget(grpResponse);
  toplay->setStretchFactor(grpResponse, 2);

  QVBoxLayout* layResponse = new QVBoxLayout(grpResponse);
  myResponse = new MLEdit(true);
  myResponse->setSizeHintLines(5);
  connect(myResponse, SIGNAL(ctrlEnterPressed()), SLOT(ok()));
  layResponse->addWidget(myResponse);

  QDialogButtonBox* buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel);
  connect(buttons, SIGNAL(accepted()), SLOT(ok()));
  connect(buttons, SIGNAL(rejected()), SLOT(close()));

  toplay->addWidget(buttons);

  if (!USERID_ISVALID(myUserId))
    myUin->setFocus();
  else
    myResponse->setFocus();

  show();
}

void AuthUserDlg::ok()
{
  if (myUin != NULL && myUin->text().trimmed().isEmpty())
    return;

  if (!USERID_ISVALID(myUserId))
    myUserId = LicqUser::makeUserId(myUin->text().trimmed().toLatin1().data(), LICQ_PPID);

  if (USERID_ISVALID(myUserId))
  {
    const QTextCodec* codec = UserCodec::codecForUserId(myUserId);
    if (myGrant)
      gLicqDaemon->authorizeGrant(myUserId, codec->fromUnicode(myResponse->toPlainText()).data());
    else
      gLicqDaemon->authorizeRefuse(myUserId, codec->fromUnicode(myResponse->toPlainText()).data());
    close();
  }
}
