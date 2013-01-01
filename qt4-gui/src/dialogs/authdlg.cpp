/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "authdlg.h"

#include "config.h"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq/contactlist/usermanager.h>
#include <licq/userid.h>
#include <licq/protocolmanager.h>

#include "helpers/support.h"

#include "widgets/mledit.h"
#include "widgets/ownercombobox.h"

using Licq::gProtocolManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::AuthDlg */

AuthDlg::AuthDlg(enum AuthDlgType type, const Licq::UserId& userId, QWidget* parent)
  : QDialog(parent),
    myType(type),
    myUserId(userId)
{
  Support::setWidgetProps(this, "AuthDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);

  QString messageTitle;
  switch (myType)
  {
    default:
    case RequestAuth:
      setWindowTitle(tr("Licq - Request Authorization"));
      messageTitle = tr("Request");
      break;
    case GrantAuth:
      setWindowTitle(tr("Licq - Grant Authorization"));
      messageTitle = tr("Response");
      break;
    case RefuseAuth:
      setWindowTitle(tr("Licq - Refuse Authorization"));
      messageTitle = tr("Response");
      break;
  }

  QVBoxLayout* dialogLayout = new QVBoxLayout(this);
  QHBoxLayout* userIdLayout = new QHBoxLayout();

  QLabel* ownerLabel = new QLabel(this);
  ownerLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  ownerLabel->setText(tr("&Account:"));
  myOwnerCombo = new OwnerComboBox();
  ownerLabel->setBuddy(myOwnerCombo);
  userIdLayout->addWidget(ownerLabel);
  userIdLayout->addWidget(myOwnerCombo);

  QLabel* accountIdLabel = new QLabel(this);
  accountIdLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  accountIdLabel->setText(tr("&User ID:"));
  myAccountIdEdit = new QLineEdit(this);
  myAccountIdEdit->setMinimumWidth(90);
  accountIdLabel->setBuddy(myAccountIdEdit);
  connect(myAccountIdEdit, SIGNAL(returnPressed()), SLOT(send()) );
  userIdLayout->addWidget(accountIdLabel);
  userIdLayout->addWidget(myAccountIdEdit);

  dialogLayout->addLayout(userIdLayout);
  dialogLayout->addSpacing(6);

  QGroupBox* messageBox = new QGroupBox(messageTitle, this);
  dialogLayout->addWidget(messageBox);
  dialogLayout->setStretchFactor(messageBox, 2);

  QVBoxLayout* messageLayout = new QVBoxLayout(messageBox);
  myMessageEdit = new MLEdit(true);
  myMessageEdit->setSizeHintLines(5);
  messageLayout->addWidget(myMessageEdit);

  QDialogButtonBox* buttons = new QDialogButtonBox();
  QPushButton* okButton = buttons->addButton(QDialogButtonBox::Ok);
  QPushButton* cancelButton = buttons->addButton(QDialogButtonBox::Cancel);

  connect(myMessageEdit, SIGNAL(ctrlEnterPressed()), this, SLOT(send()));
  connect(okButton, SIGNAL(clicked()), SLOT(send()) );
  connect(cancelButton, SIGNAL(clicked()), SLOT(close()) );

  dialogLayout->addWidget(buttons);

  if (userId.isValid())
  {
    myOwnerCombo->setCurrentOwnerId(userId.ownerId());
    myOwnerCombo->setEnabled(false);
    myAccountIdEdit->setText(userId.accountId().c_str());
    myAccountIdEdit->setEnabled(false);
    myMessageEdit->setFocus();
  }
  else
  {
    myOwnerCombo->setFocus();
  }

  show();
}

void AuthDlg::send()
{
  Licq::UserId userId = myUserId;
  if (!userId.isValid())
    userId = Licq::UserId(myOwnerCombo->currentOwnerId(), myAccountIdEdit->text().toUtf8().constData());

  if (userId.isValid())
  {
    QByteArray messageText = myMessageEdit->toPlainText().toUtf8();

    switch (myType)
    {
      default:
      case RequestAuth:
        gProtocolManager.requestAuthorization(userId, messageText.constData());
        break;
      case GrantAuth:
        gProtocolManager.authorizeReply(userId, true, messageText.constData());
        break;
      case RefuseAuth:
        gProtocolManager.authorizeReply(userId, false, messageText.constData());
        break;
    }

    close();
  }
}
